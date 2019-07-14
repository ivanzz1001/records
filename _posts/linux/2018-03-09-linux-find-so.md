---
layout: post
title: linux中动态链接库的查找
tags:
- LinuxOps
categories: linux
description: linux中动态链接库的查找
---

本章我们主要谈一谈Linux操作系统下动态链接库的查找。



<!-- more -->

## 1. 动态链接库查找
学习到了一个阶段之后，就需要不断的总结、沉淀、清零，然后才能继续"上路"。回想起自己当年刚接触Linux时，不管是用源码包安装程序，还是在程序运行时出现的和动态链接库的各种恩恩怨怨，心里那真一个难受。那时候脑袋里曾经也犯过嘀咕，为啥Linux不弄成Windows那样呢？ 装个软件那个麻烦不说，连运行软件都那么恼火。如果那样的话就不叫Linux了。借用小米公司CEO雷军一句话： 小米，为发烧而生。我认为： Linux，为真理而在。特别是为那些喜欢折腾，热衷技术背后原理和实现细节的人们而生。

说到和动态链接库查找路径相关的问题，总体上课归结为两类：

1） 通过源代码编译程序时出现找不到某个依赖包的问题，而如果此时你恰好已经按照它的要求确确实实、千真万确、天地良心地把依赖库给装好了，它还给你耍混、犯二，有一股折腾不死人不偿命的劲儿，那让人真是气不打一处来。如果Linux此时有头有脸，你是不是想抽它丫两大嘴巴；

2） 就是在运行程序的时候，明明把那个程序需要的依赖包都已经安装的妥妥的了，可运行的时候人家就告诉你说*error while loading shared libraries: libxxx.so.y; can't open shared object file: No such file or directory*，任凭你怎么折腾都没用。此时，你要是心想“撤吧，哥们，Linux太欺负人了，不带这么玩儿的”，那你就大错特错了，只要你抱着“美好的事情总会发生”和“办法永远比问题多”的信念坚持下去，你就一定会成功。话的意思有点自欺欺人，精神鸦片的味道在里面，但确实是这么个理儿。

上面两类问题最大的原因就是，你没弄明白它们的机制和原理。你看到的只是现象，当年学马克思主义哲学的时候，老师怎么教导我们的？要透过现象看本质。如果你把上述两种应用的原理搞清了，那问题不就自然而然的迎刃而解了么。下面咱就一一探讨一下这两个问题，以便对新进学习Linux的朋友起一个参考资料的作用。

### 2. 程序编译时动态库的查找
通过源代码包安装程序时，主要用到了```三大步```策略： configure、make、make install。出问题最多的就是在configure阶段，很多初学者由于不知道configure的那么多参数该怎么用，所以往往为了省事，一句简单的“./configure”下去，百分之八九十都能成功，可问题往往就出在剩下的百分之十几上面了。则让我们又一次相信了，小概率事件的发生对事情的影响是多么的深远。在安装的```configure```阶段，为了检测安装环境是否满足，通常情况下都是通过一个叫做```pkg-config```的工具来检测它需要依赖的动态链接库是否存在，这个工具我们在前面相关博文已经介绍过了。```pkg-config```通常情况都是位于```/usr/bin```目录下，是个可执行程序。在configure阶段，通常都会用```pkg-config```来判断所依赖的动态库是否存在。现在的问题是，这个工具是如何判断的呢？它的依据是什么？当这两个问题弄明白了，真相也就大白了。

一般当我们安装完某个程序后，如果它提供了动态库的功能，在源码中都会有一个或多个以```pc```结尾的文件，当执行完make install后这些pc文件拷贝到*$(prefix)/lib/pkgconfig/*这个目录里，这里的```prefix```就是我们在configure阶段时通过配置参数```--prefix```指定的，缺省情况这个值就是*/usr/local*，所以这些```pc```文件最终会被拷贝到*/usr/local/lib/pkgconfig/*目录下。可能有人会问，	这些pc文件有啥用呢？我们随便打开一个来瞧瞧：
<pre>
# cat /usr/local/lib/pkgconfig/librtmp.pc
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
incdir=${prefix}/include


Name: librtmp
Description: RTMP implementation
Version: v2.3
Requires: libssl,libcrypto
URL: http://rtmpdump.mplayerhq.hu
Libs: -L${libdir} -lrtmp -lz
Cflags: -I${incdir}
</pre>
跟我们configure阶段相关的主要集中在```Libs```和```Cflags```两项上面。如果你此时再执行下面这两条命令，就完全明白了：
<pre>
# pkg-config --cflags librtmp
-I/usr/local/include
# pkg-config --libs librtmp
-L/usr/local/lib -lrtmp -lz -lssl -lcrypto
</pre>
也就是说，```pkg-config```把我们以前需要在Makefile里指定编译和链接时所需要用到的参数从手工硬编码的模式变成了自动完成，节约了多少跨平台移植的兼容性问题，我们是不是需要感谢人家十八辈子祖宗！ 假如说，我们将要编译的软件包依赖```librtmp```这个动态库，那么此时在我们系统上这个检测就算通过了。当然这只是第一步，检测过了并不一定兼容，这里我们只讨论能不能找到依赖库的问题，兼容性问题那都不是个事儿。人家要啥版本你好生伺候就是了，这个没得商量，最好也不要商量，童叟无欺，不然后果严重。好了，如果说找不到某个库怎么办？前提是你确确实实已经安装了它需要的库，不用多想，原因只有一个，```pkg-config```找不到与这个库对应的```pc```文件。为什么会找不到呢，原有有2点：

* ```pkg-config```搜索了所有它认为合适的目录都没有找到这个库对应的```pc```文件的下落；

* 这个库在发布时根本就没有提供它的pc文件

这里我们严重“抗议、鄙视+抵制”第二种情况的软件包，而且也尽量不要用它，一个出来混都不自报家门的家伙，肯定也好不到哪里去。那么，现在的问题就只剩下一个了：```pkg-config```的查找路径是哪里？

```pkg-config```较老的版本里，缺省情况下会到*/usr/lib/pkgconfig*、*/usr/local/lib/pkgconfig*、*/usr/share/pkgconfig*等目录下去搜索```pc```文件。据我所知在```0.23```及之后的版本里```pkg-config```的源码里已经没有关于缺省搜索路径的任何硬编码的成分了，至于具体从哪个版本开始我也没去追究，还有望知道的朋友分享一下。取而代之的是，当你看```pkg-config```的man手册时会有下面一段话：
<pre>
pkg-config retrieves information about packages from special metadata files. 
These files are  named  after the  package,  with  the extension .pc.

By default, pkg-config looks in the directory prefix/lib/pkgconfig for these files; 
it will also look in the colon-separated (on Windows, semicolon-separated) list of 
directories specified by the PKG_CONFIG_PATH environment variable.
</pre>
以及如下这点补充：
<pre>
PKG_CONFIG_PATH
    A colon-separated (on Windows, semicolon-separated) list of directories to search  for  .pc
    files. The  default directory will always be searched after searching the path; the default 
    is libdir/pkg-config:datadir/pkgconfig where libdir is the libdir where pkg-config and  datadir
    is  the  datadir where pkg-config was installed.
</pre>
上面提到的```prefix```、```libdir```和```datadir```，就是安装```pkg-config```时被设定好的，具体情况是：

1） 如果你是通过```yum```或```rpm```包安装的
<pre>
prefix=/usr
libdir=${prefix}/lib
datadir=${prefix}/share
</pre>

2) 如果你是通过源代码包安装的，且没有指定```prefix```的值（指定的情况同1）
<pre>
prefix=/usr/local
libdir=${prefix}/lib
datadir=${prefix}/share
</pre>
```pkg-config```在查找对应软件包的信息时的缺省搜索路径已经很清楚了。如果你的软件包对应的pc文件都不在这两个目录下时，```pkg-config```肯定找不到。原因既然都已经找到了，那解决办法也就多种多样了：

* 方案1：我们可以在安装我们那个被依赖的软件包时，在configure阶段用```--prefix```参数把安装目录指定到```/usr```目录下；

* 方案2： 也可以按照上面说的，通过一个名叫```PKG_CONFIG_PATH```的环境变量来向```pkg-config```指明自己的pc文件所在的路径，不过要注意的是```PKG_CONFIG_PATH```所指定的路径优先级比较高，```pkg-config```会先进行搜索，完了之后采取搜索缺省路径。

上述两种方案，前者的优点是以后再通过源码安装软件时少了不少麻烦，缺点是用户自己的软件包和系统软件混到一起不方便管理。所以实际使用中，后者用的要多一些：

**方案2**在实际操作中有两种实现方式：

1） 针对没有root权限的情况，大多数情况都是执行：
<pre>
export PKG_CONFIG_PATH=/your/local/path:$PKG_CONFIG_PATH
</pre>
然后在configure时就绝对没有问题了。

2） 在用户的home目录下的```.bash_profile```文件里或系统文件*/etc/profile*的末尾添加上面一行也行，都可以

至此，动态库查找问题的第一种情况就彻底解决了。想了解pc文件的更多细节，可以参考[pkg-config指南](http://people.freedesktop.org/~dbn/pkg-config-guide.html)，想学习```pkg-config```工具更多用法的朋友建议参看man手册。


## 3. 程序运行时动态库的查找

我们在程序运行时经常会遇到：
{% highlight string %}
libxxx.so.y => not found
{% endhighlight %}

这种情况我们可以通过```ldconfig```方式来解决，也是99%的场合下的解决办法，但那是针对有root权限的用户的解决办法。没有root权限运行软件时，Linux也为我们提供了一个名为```LD_LIBRARY_PATH```的环境变量来解决运行时动态库查找路径的解决方案。同样地，由这个环境变量所指定的路径会被装载器*/lib/ld-2.12.so*优先查找，然后才是动态库缓存文件*/etc/ld.so.cache*，风采瞬间就被```LD_LIBRARY_PATH```给抢完了，*/etc/ld.so.cache*表示很不高兴。针对```LD_LIBRARY_PATH```环境变量这种情况，绝对是临时不能再临时的解决方案了，如果只是测试用，用export像解决```PKG_CONFIG_PATH```一样的方式干净利索就行了，千万不要在实际生产上线的运维环境里把*export LD_LIBRARY_PATH=...*添加到```.bash_profile```或者*/etc/profile*里。

其实```PKG_CONFIG_PATH```和```LD_LIBRARY_PATH```经常被很多人误用，特别是新手们在解决问题时，也不分青红皂白，逮着了就一顿狂export，根据实际场合，运气好了说不定问题还真就解决，点儿背了折腾一天半宿也是白忙活。其实要是留点心，还是挺容易明白的。

```PKG_CONFIG_PATH```从字面意思上翻译，就是*“软件包的配置路径”*，这不很明显了么，编译软件时如果出现找不到所依赖的动态链接库时就都全靠*PKG_CONFIG_PATH*了。

```LD_LIBRARY_PATH```也很直白了*“装载器的库路径”*，LD是Loader的简写。在Linux中系统启动一个程序的过程就交```装载```，一个程序要执行时它或多或少的会依赖一些动态库（静态编译的除外）。当你用*“ldd 可执行程序名”*查看一个软件启动时所依赖的动态库，如果输出项有"libxxx.so.y=> not found"一项，你这个软件100%运行不起来。

不信我们来做个试验：
{% highlight string %}
# echo $LD_LIBRARY_PATH    //没有进行配置

# ldd /usr/local/bin/ffmpeg
        linux-gate.so.1 =>  (0x00914000)
        libavdevice.so.54 => /usr/local/lib/libavdevice.so.54 (0x007d0000)
        libavfilter.so.3 => /usr/local/lib/libavfilter.so.3 (0x001f3000)
        libavformat.so.54 => /usr/local/lib/libavformat.so.54 (0x002b5000)
        libavcodec.so.54 => /usr/local/lib/libavcodec.so.54 (0xb68dd000)
        libpostproc.so.52 => /usr/local/lib/libpostproc.so.52 (0x0083c000)
        libswresample.so.0 => /usr/local/lib/libswresample.so.0 (0x00a91000)
        libswscale.so.2 => /usr/local/lib/libswscale.so.2 (0x00d80000)
        libavutil.so.52 => /usr/local/lib/libavutil.so.52 (0x001a7000)
        libm.so.6 => /lib/libm.so.6 (0x0058b000)
        libpthread.so.0 => /lib/libpthread.so.0 (0x001d7000)
        libc.so.6 => /lib/libc.so.6 (0x005e2000)
        libasound.so.2 => /lib/libasound.so.2 (0x00ec5000)
        libdc1394.so.22 => /usr/local/lib/libdc1394.so.22 (0x00116000)
        librt.so.1 => /lib/librt.so.1 (0x00184000)
        libfreetype.so => /usr/local/lib/libfreetype.so (0x00411000)
        libass.so.4 => /usr/local/lib/libass.so.4 (0x0091a000)
        libssl.so.1.0.0 => /usr/local/lib/libssl.so.1.0.0 (0x0048c000)
        libcrypto.so.1.0.0 => /usr/local/lib/libcrypto.so.1.0.0 (0x00aa8000)
        librtmp.so.0 => /usr/local/lib/librtmp.so.0 (0x009dd000)
        libz.so.1 => /lib/libz.so.1 (0x0018d000)
        libx264.so.132 => /usr/local/lib/libx264.so.132 (0x00fb1000)
        libvorbisenc.so.2 => /usr/local/lib/libvorbisenc.so.2 (0x0194d000)
        libvorbis.so.0 => /usr/local/lib/libvorbis.so.0 (0x004e5000)
        libvo-aacenc.so.0 => /usr/local/lib/libvo-aacenc.so.0 (0x00799000)
        libtwolame.so.0 => /usr/local/lib/libtwolame.so.0 (0x0050d000)
        libtheoraenc.so.1 => /usr/local/lib/libtheoraenc.so.1 (0x0052d000)
        libtheoradec.so.1 => /usr/local/lib/libtheoradec.so.1 (0x00779000)
        libspeex.so.1 => /usr/local/lib/libspeex.so.1 (0x00c94000)
        libmp3lame.so.0 => /usr/local/lib/libmp3lame.so.0 (0x0088c000)
        libfaac.so.0 => /usr/local/lib/libfaac.so.0 (0x00573000)
        /lib/ld-linux.so.2 (0x005c2000)
        libdl.so.2 => /lib/libdl.so.2 (0x001a1000)
        libraw1394.so.11 => /usr/local/lib/libraw1394.so.11 (0x005b5000)
        libfribidi.so.0 => /usr/local/lib/libfribidi.so.0 (0x007b5000)
        libfontconfig.so.1 => /usr/local/lib/libfontconfig.so.1 (0x007ea000)
        libogg.so.0 => /usr/local/lib/libogg.so.0 (0x00583000)
        libexpat.so.1 => /lib/libexpat.so.1 (0x00933000)
{% endhighlight %}
在上面我的系统里没有设置```LD_LIBRARY_PATH```环境变量。现在我们把ffmpeg所依赖的一个库```libmp3lame.so.0```从*/usr/local/lib*下移动到*/opt*目录里，并执行ldconfig，让```libmp3lame.so.0```彻底从*/etc/ld.so.cache*里面消失。其实*libmp3lame.so.0*只是*libmp3lame.so.0.0.0*的一个符号链接，我们真正需要移动的是后者，完了之后再执行*ldd /usr/lacal/bin/ffmpeg*时结果如下：
{% highlight string %}
# ldd /usr/local/bin/ffmpeg
        linux-gate.so.1 =>  (0x00249000)
        libavdevice.so.54 => /usr/local/lib/libavdevice.so.54 (0x00e12000)
        libavfilter.so.3 => /usr/local/lib/libavfilter.so.3 (0x00ccd000)
        libavformat.so.54 => /usr/local/lib/libavformat.so.54 (0x00891000)
        libavcodec.so.54 => /usr/local/lib/libavcodec.so.54 (0xb6877000)
        libpostproc.so.52 => /usr/local/lib/libpostproc.so.52 (0x001a6000)
        libswresample.so.0 => /usr/local/lib/libswresample.so.0 (0x00b8f000)
        libswscale.so.2 => /usr/local/lib/libswscale.so.2 (0x0024a000)
        libavutil.so.52 => /usr/local/lib/libavutil.so.52 (0x005d7000)
        libm.so.6 => /lib/libm.so.6 (0x007ad000)
        libpthread.so.0 => /lib/libpthread.so.0 (0x001f6000)
        libc.so.6 => /lib/libc.so.6 (0x0029f000)
        libasound.so.2 => /lib/libasound.so.2 (0x00604000)
        libdc1394.so.22 => /usr/local/lib/libdc1394.so.22 (0x00436000)
        librt.so.1 => /lib/librt.so.1 (0x00a06000)
        libfreetype.so => /usr/local/lib/libfreetype.so (0x0052d000)
        libass.so.4 => /usr/local/lib/libass.so.4 (0x00211000)
        libssl.so.1.0.0 => /usr/local/lib/libssl.so.1.0.0 (0x00eed000)
        libcrypto.so.1.0.0 => /usr/local/lib/libcrypto.so.1.0.0 (0x00f46000)
        librtmp.so.0 => /usr/local/lib/librtmp.so.0 (0x004b9000)
        libz.so.1 => /lib/libz.so.1 (0x0022a000)
        libx264.so.132 => /usr/local/lib/libx264.so.132 (0x0765d000)
        libvorbisenc.so.2 => /usr/local/lib/libvorbisenc.so.2 (0x00a0f000)
        libvorbis.so.0 => /usr/local/lib/libvorbis.so.0 (0x004ce000)
        libvo-aacenc.so.0 => /usr/local/lib/libvo-aacenc.so.0 (0x005a8000)
        libtwolame.so.0 => /usr/local/lib/libtwolame.so.0 (0x006f0000)
        libtheoraenc.so.1 => /usr/local/lib/libtheoraenc.so.1 (0x00710000)
        libtheoradec.so.1 => /usr/local/lib/libtheoradec.so.1 (0x00756000)
        libspeex.so.1 => /usr/local/lib/libspeex.so.1 (0x00770000)
        libmp3lame.so.0 => not found)
        libfaac.so.0 => /usr/local/lib/libfaac.so.0 (0x004a4000)
        /lib/ld-linux.so.2 (0x0050d000)
        libdl.so.2 => /lib/libdl.so.2 (0x0023e000)
        libraw1394.so.11 => /usr/local/lib/libraw1394.so.11 (0x004f6000)
        libfribidi.so.0 => /usr/local/lib/libfribidi.so.0 (0x0078a000)
        libfontconfig.so.1 => /usr/local/lib/libfontconfig.so.1 (0x007d7000)
        libogg.so.0 => /usr/local/lib/libogg.so.0 (0x00243000)
        libexpat.so.1 => /lib/libexpat.so.1 (0x00806000)

# ffmpeg --help
ffmpeg: error while loading shared libraries: libmp3lame.so.0: cannot open shared 
object file: No such file or directory  //此时ffmpeg当然运行不起来
{% endhighlight %}
我们来试试```LD_LIBRARY_PATH```，看看好使不：
{% highlight string %}
# export LD_LIBRARY_PATH=/opt:$LD_LIBRARY_PATH
#
# ldd /usr/local/bin/ffmpeg
        linux-gate.so.1 =>  (0x00136000)
        libavdevice.so.54 => /usr/local/lib/libavdevice.so.54 (0x00552000)
        libavfilter.so.3 => /usr/local/lib/libavfilter.so.3 (0x00655000)
        libavformat.so.54 => /usr/local/lib/libavformat.so.54 (0x00243000)
        libavcodec.so.54 => /usr/local/lib/libavcodec.so.54 (0xb68a7000)
        libpostproc.so.52 => /usr/local/lib/libpostproc.so.52 (0x00137000)
        libswresample.so.0 => /usr/local/lib/libswresample.so.0 (0x00187000)
        libswscale.so.2 => /usr/local/lib/libswscale.so.2 (0x0047e000)
        libavutil.so.52 => /usr/local/lib/libavutil.so.52 (0x00a9d000)
        libm.so.6 => /lib/libm.so.6 (0x00af9000)
        libpthread.so.0 => /lib/libpthread.so.0 (0x00823000)
        libc.so.6 => /lib/libc.so.6 (0x0083e000)
        libasound.so.2 => /lib/libasound.so.2 (0x0055f000)
        libdc1394.so.22 => /usr/local/lib/libdc1394.so.22 (0x0019e000)
        librt.so.1 => /lib/librt.so.1 (0x00b3c000)
        libfreetype.so => /usr/local/lib/libfreetype.so (0x0039f000)
        libass.so.4 => /usr/local/lib/libass.so.4 (0x00f67000)
        libssl.so.1.0.0 => /usr/local/lib/libssl.so.1.0.0 (0x00cb3000)
        libcrypto.so.1.0.0 => /usr/local/lib/libcrypto.so.1.0.0 (0x00d0c000)
        librtmp.so.0 => /usr/local/lib/librtmp.so.0 (0x0020c000)
        libz.so.1 => /lib/libz.so.1 (0x00c77000)
        libx264.so.132 => /usr/local/lib/libx264.so.132 (0x00f80000)
        libvorbisenc.so.2 => /usr/local/lib/libvorbisenc.so.2 (0x07c66000)
        libvorbis.so.0 => /usr/local/lib/libvorbis.so.0 (0x0041a000)
        libvo-aacenc.so.0 => /usr/local/lib/libvo-aacenc.so.0 (0x0076c000)
        libtwolame.so.0 => /usr/local/lib/libtwolame.so.0 (0x004fe000)
        libtheoraenc.so.1 => /usr/local/lib/libtheoraenc.so.1 (0x00717000)
        libtheoradec.so.1 => /usr/local/lib/libtheoradec.so.1 (0x00f0c000)
        libspeex.so.1 => /usr/local/lib/libspeex.so.1 (0x00221000)
        libmp3lame.so.0 => not found           //纳尼？？！！！
        libfaac.so.0 => /usr/local/lib/libfaac.so.0 (0x00124000)
        /lib/ld-linux.so.2 (0x00bad000)
        libdl.so.2 => /lib/libdl.so.2 (0x0023b000)
        libraw1394.so.11 => /usr/local/lib/libraw1394.so.11 (0x007b6000)
        libfribidi.so.0 => /usr/local/lib/libfribidi.so.0 (0x00442000)
        libfontconfig.so.1 => /usr/local/lib/libfontconfig.so.1 (0x0051e000)
        libogg.so.0 => /usr/local/lib/libogg.so.0 (0x009f7000)
        libexpat.so.1 => /lib/libexpat.so.1 (0x00b60000)
{% endhighlight %}
还记得上面提到了软链接么，*libmp3lame.so.0*就是*libmp3lame.so.0.0.0*的软链接，这是动态库的命名规范的一种公约，我们只要在*/opt*目录下建立一个名为*libmp3lame.so.0*的到*/opt/libmp3lame.so.0.0.0*的软链接就OK了：
{% highlight string %}
# ls
libmp3lame.so.0.0.0
# ln -s libmp3lame.so.0.0.0 libmp3lame.so.0
# ll
total 316
lrwxrwxrwx. 1 root root     19 Dec  7 23:27 libmp3lame.so.0 -> libmp3lame.so.0.0.0
-rwxr-xr-x. 1 root root 321228 Dec  7 23:25 libmp3lame.so.0.0.0

# ldd /usr/local/bin/ffmpeg
        linux-gate.so.1 =>  (0x00cc4000)
        libavdevice.so.54 => /usr/local/lib/libavdevice.so.54 (0x00577000)
        libavfilter.so.3 => /usr/local/lib/libavfilter.so.3 (0x00e3f000)
        libavformat.so.54 => /usr/local/lib/libavformat.so.54 (0x00202000)
        libavcodec.so.54 => /usr/local/lib/libavcodec.so.54 (0x00f01000)
        libpostproc.so.52 => /usr/local/lib/libpostproc.so.52 (0x00170000)
        libswresample.so.0 => /usr/local/lib/libswresample.so.0 (0x00750000)
        libswscale.so.2 => /usr/local/lib/libswscale.so.2 (0x0035e000)
        libavutil.so.52 => /usr/local/lib/libavutil.so.52 (0x005ba000)
        libm.so.6 => /lib/libm.so.6 (0x00452000)
        libpthread.so.0 => /lib/libpthread.so.0 (0x001c0000)
        libc.so.6 => /lib/libc.so.6 (0x008c2000)
        libasound.so.2 => /lib/libasound.so.2 (0x0047c000)
        libdc1394.so.22 => /usr/local/lib/libdc1394.so.22 (0x003d6000)
        librt.so.1 => /lib/librt.so.1 (0x00db3000)
        libfreetype.so => /usr/local/lib/libfreetype.so (0x00a80000)
        libass.so.4 => /usr/local/lib/libass.so.4 (0x001db000)
        libssl.so.1.0.0 => /usr/local/lib/libssl.so.1.0.0 (0x005e7000)
        libcrypto.so.1.0.0 => /usr/local/lib/libcrypto.so.1.0.0 (0x00afb000)
        librtmp.so.0 => /usr/local/lib/librtmp.so.0 (0x00584000)
        libz.so.1 => /lib/libz.so.1 (0x00599000)
        libx264.so.132 => /usr/local/lib/libx264.so.132 (0x02bc9000)
        libvorbisenc.so.2 => /usr/local/lib/libvorbisenc.so.2 (0x05ccd000)
        libvorbis.so.0 => /usr/local/lib/libvorbis.so.0 (0x00640000)
        libvo-aacenc.so.0 => /usr/local/lib/libvo-aacenc.so.0 (0x00834000)
        libtwolame.so.0 => /usr/local/lib/libtwolame.so.0 (0x00668000)
        libtheoraenc.so.1 => /usr/local/lib/libtheoraenc.so.1 (0x00688000)
        libtheoradec.so.1 => /usr/local/lib/libtheoradec.so.1 (0x006ce000)
        libspeex.so.1 => /usr/local/lib/libspeex.so.1 (0x00815000)
        libmp3lame.so.0 => /opt/libmp3lame.so.0 (0x00767000)   //终于圆满了:)
        libfaac.so.0 => /usr/local/lib/libfaac.so.0 (0x006e8000)
        /lib/ld-linux.so.2 (0x003b6000)
        libdl.so.2 => /lib/libdl.so.2 (0x001f4000)
        libraw1394.so.11 => /usr/local/lib/libraw1394.so.11 (0x00444000)
        libfribidi.so.0 => /usr/local/lib/libfribidi.so.0 (0x006f8000)
        libfontconfig.so.1 => /usr/local/lib/libfontconfig.so.1 (0x00710000)
        libogg.so.0 => /usr/local/lib/libogg.so.0 (0x001f9000)
        libexpat.so.1 => /lib/libexpat.so.1 (0x007e3000)
{% endhighlight %}
所以说，针对路径库查找的种种问题，无非就这么两大类，关键是找对原因，对症下药，方能药到病除。




<br />
<br />
**[参看]:**

1. [谈谈Linux下动态库查找路径的问题](http://blog.chinaunix.net/uid-23069658-id-4028681.html)

2. [linux ldconfig命令](https://blog.csdn.net/winycg/article/details/80572735)

3. [一堂课玩转rpm包的制作](http://blog.chinaunix.net/uid-23069658-id-3944462.html)



<br />
<br />
<br />





