---
layout: post
title: gdb如何确定内存释放(转)
tags:
- LinuxOps
categories: linux
description: linux调试
---

文章转自[gdb如何确定内存已经释放](https://blog.csdn.net/weixin_36356040/article/details/112432311)，在此做一个记录，防止原文丢失，并便于后续查阅。

<!-- more -->

## 1. 问题现场
为了更好地实现对项目地管理，我们将组内一个项目迁移到```MDP框架```(基于Spring Boot)，随后我们就发现系统会频繁报出Swap区域使用量过高的异常。

笔者被叫去排查原因，发现配置了4G堆内内存，但实际使用的物理内存竟然高达7G，确实不正常。

JVM参数配置情况如下：
<pre>
-XX:MetaspaceSize=256M -XX:MaxMetaspaceSize=256M -XX:+AlwaysPreTouch -XX:ReservedCodeCacheSize=128m -XX:InitialCodeCacheSize=128m, -Xss512k -Xmx4g -Xms4g,-XX:+UseG1GC -XX:G1HeapRegionSize=4M
</pre>

实际使用的物理内存如下图所示(top命令显式的内存情况)：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure1.jfif)


## 2. 排查过程

### 2.1 使用Java层面的工具定位内存区域

堆内内存、Code区域或者使用```unsafe.allocateMemory```和```DirectByteBuffer```申请的堆外内存.

笔者在项目中添加JVM参数```-XX:NativeMemoryTracking=detail```，然后重启项目，使用命令
<pre>
# jcmd pid VM.native_memory detail
</pre>
查看到的内存分布如下（jcmd显示的内存情况）：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure2.jfif)

发现命令显示的committed的内存小于物理内存，因为```jcmd```命令显示的内存包含堆内内存、Code区域、通过unsafe.allocateMemory和DirectByteBuffer申请的内存，但是不包括其他Native Code(C代码）申请的堆外内存。所以猜测是使用Native Code申请内存导致的问题。

为了防止误判，笔者使用了pmap查看内存分布，发现大量的64M的地址；而这些地址不在```jcmd```命令所给出的地址空间里面，基本上判定就是这些64M的内存所导致。

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure3.jfif)

### 2.2 使用系统层面的工具定位堆外内存
因为笔者已经基本上确定是Native Code所引起，而Java层面的工具不便于排查此类问题，只能使用系统层面的工具去定位问题。

1） **使用gperftools**

我们首先使用了```gperftools```去定位问题。gperftools的使用方法可以参考[gperftools wiki](https://github.com/gperftools/gperftools/wiki)。监控得到如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure4.jfif)

从上图可以看出：使用malloc()申请的内存最高到3G之后就释放了，之后始终维持在700M-800M。笔者第一反应是：难道Native Code中没有使用malloc()申请，直接使用mmap/brk申请的？

>注： gperftools的原理就是使用动态链接的方式替换了操作系统默认的内存分配器(glibc)

2） **使用strace去追踪系统调用**

因为使用gperftools没有追踪到这些内存，于是直接使用命令:
<pre>
# strace -f -e "brk,mmap,munmap" -p pid
</pre>
追踪向OS申请内存请求，但是并没有发现可疑内存申请。strace监控如下图所示：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure5.jfif)

3) **使用gdb去dump可疑内存**

因为使用strace没有追踪到可疑内存申请，于是想着看看内存中的情况。就是直接使用:
<pre>
# gdb -pid pid
</pre>
进入GDB之后，然后使用命令
<pre>
# dump memory mem.bin startAddress endAddressdump
</pre>
将指定内存段的数据dump出来，其中```startAddress```和```endAddress```可以从```/proc/pid/smaps```中查找。然后使用```strings mem.bin```查看dump的内容，如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure6.jfif)

从内容上来看，像是解压后的JAR包信息。读取JAR包信息应该是在项目启动的时候，那么在项目启动之后使用strace作用就不是很大了。所以应该在项目启动的时候使用strace，而不是启动完成之后。

4） **再次，项目启动时使用strace去追踪系统调用**

项目启动使用strace追踪系统调用，发现确实申请了很多64M的内存空间，截图如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure7.jfif)

使用该mmap申请的地址空间在pmap对应如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure8.jfif)


5） **最后，使用jstack去查看对应的线程**

最后，我们使用jstack去查看对应的线程。因为strace命令中已经显示申请内存的线程ID。直接使用命令jstack pid去查看线程栈，找到对应的线程栈(注意10进制和16进制转换)如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure9.jfif)

这里基本上就可以看出问题来了：MCC（美团统一配置中心）使用了```Reflections```进行扫包，底层使用了Spring Boot去加载JAR。因为解压JAR使用了```Inflater```类，需要用到堆外内存，然后使用```Btrace```去追踪这个类，栈如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure10.jfif)


然后查看使用MCC的地方，发现没有配置扫包路径，默认是扫描所有的包。于是修改代码，配置扫包路径，发布上线后内存问题解决。


## 3. 为什么堆外内存没有释放掉呢？

问题虽然已经解决了，但是有几个疑问：

* 为什么使用旧的框架没有问题？

* 为什么堆外内存没有释放？

* 为什么内存大小都是64M，JAR大小不可能这么大，而且都是一样大？

* 为什么gperftools最终显示使用的内存大小是700M左右，解压包真的没有使用malloc()申请内存吗？

带着疑问，笔者直接看了一下Spring Boot Loader那一块的源码。发现Spring Boot对于Java JDK的```InflaterInputStream```进行了包装并且使用了```Inflater```，而Inflater本身用于解压JAR包时需要用到堆外内存。而包装之后的类```ZipInflaterInputStream```没有释放Inflater持有的堆外内存。于是笔者以为找到了原因，立马向Spring Boot社区反馈了这个Bug。但是反馈之后，笔者就发现Inflater这个对象本身实现了finalize()方法，这个方法中有调用释放堆外内存的逻辑。也就是说Spring Boot依赖于GC释放堆外内存。

笔者使用```jmap```查看堆内对象时，发现已经基本上没有Inflater这个对象了。于是就怀疑GC的时候，没有调用finalize()。带着这样的怀疑，笔者把Inflater进行包装在Spring Boot Loader里面替换成自己包装的Inflater，在finalize()进行打点监控，结果finalize()方法确实被调用了。于是笔者又去看了Inflater对应的C代码，发现初始化的时候使用了malloc()申请内存，end的时候也调用了free()去释放内存。

此刻，笔者只能怀疑free()的时候没有真正释放内存，便把Spring Boot包装的```InflaterInputStream```替换成JAVA JDK自带的，发现替换之后，内存问题也得以解决了。

这时，再反过来看gperftools的内存分布情况，发现使用Spring Boot时，内存一直在增加，突然某个点内存使用下降了好多（使用量直接由3G降为700M左右）。这个点应该就是GC引起的，内存应该释放了，但是在操作系统层面并没有看到内存变化，那是不是没有释放到操作系统，被内存分配器持有了呢？

继续探究，发现系统默认的内存分配器(glibc 2.12版本）和使用gperftools内存地址分布差别很明显，2.5G地址使用```smaps```发现它是属于Native Stack。内存地址分布如下（gperftools显示的内存地址分布）：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure11.jfif)

到此，基本上可以确定是内存分配器在捣鬼。搜索了一下glibc 64M，发现glibc从2.11开始对每个线程引入内存池(64位机器大小就是64M内存)，原文如下（glib内存池说明）：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure12.jfif)

按照文中所说去修改```MALLOC_ARENA_MAX```环境变量，发现没什么效果。查看tcmalloc(gperftools使用的内存分配器)也使用了内存池方式。

为了验证是内存池搞的鬼，笔者就简单写个不带内存池的内存分配器。使用命令gcc zjbmalloc.c -fPIC -shared -o zjbmalloc.so生成动态库，然后使用export LD_PRELOAD=zjbmalloc.so替换掉glibc的内存分配器。其中代码Demo如下：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure13.jfif)

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure14.jfif)

通过在自定义分配器当中埋点可以发现其实程序启动之后应用实际申请的堆外内存始终在700M-800M之间，gperftools监控显示内存使用量也是在700M-800M左右。但是从操作系统角度来看进程占用的内存差别很大(这里只是监控堆外内存)。

笔者做了一下测试，使用不同分配器进行不同程度的扫包，占用的内存如下(内存测试对比)：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure15.jfif)

为什么自定义的malloc()申请800M，最终占用的物理内存在1.7G呢？

因为自定义内存分配器采用的是mmap分配内存，mmap分配内存按需向上取整到整数个页，所以存在着巨大的空间浪费。通过监控发现最终申请的页面数目在536k个左右，那实际上向系统申请的内存等于512k * 4k(pagesize) = 2G。为什么这个数据大于1.7G呢？

因为操作系统采取的是延迟分配的方式，通过mmap向系统申请内存的时候，系统仅仅返回内存地址并没有分配真实的物理内存。只有在真正使用的时候，系统产生一个缺页中断，然后再分配实际的物理Page。

## 4. 总结

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure16.jfif)


整个内存分配的流程如上图所示。MCC扫包的默认配置是扫描所有的JAR包。在扫描包的时候，Spring Boot不会主动去释放堆外内存，导致在扫描阶段，堆外内存占用量一直持续飙升。当发生GC的时候，Spring Boot依赖于finalize()机制去释放了堆外内存；但是glibc为了性能考虑，并没有真正把内存归返到操作系统，而是留下来放入内存池了，导致应用层以为发生了“内存泄漏”。所以修改MCC的配置路径为特定的JAR包，问题解决。笔者在发表这篇文章时，发现Spring Boot的最新版本(2.0.5.RELEASE)已经做了修改，在ZipInflaterInputStream主动释放了堆外内存不再依赖GC；所以Spring Boot升级到最新版本，这个问题也可以得到解决。



<br />
<br />
**[参看]:**



1. [gdb如何确定内存 已经释放](https://blog.csdn.net/weixin_35235724/article/details/112423823)

<br />
<br />
<br />





