---
layout: post
title: Linux中ldconfig的使用
tags:
- LinuxOps
categories: linux
description: Linux中ldconfig的使用
---


本文我们介绍一下Linux系统中ldconfig命令的使用。

<!-- more -->

## 1. ldconfig命令
```/sbin/ldconfig```命令用于配置```动态链接器```在运行时的绑定，其会为如下三种共享链接库创建必要的links以及cache:

* */etc/ld.so.conf*文件中所列目录下的共享库

* 受信任目录下的共享库(/lib、/usr/lib)

* 在命令行通过```ldconfig -f```选项指定的共享库

缓存文件默认为*/etc/ld.so.cache*,此文件保存已排好序的动态链接库名字列表，程序连接的时候首先从这个文件里边查找，然后再到ld.so.conf的路径里边去详细找。这就是为什么修改了ld.so.conf要重新运行一下ldconfig的原因。

cache会被运行时链接器(ld.so或者ld-linux.so)所使用。当需要更新链接时，ldconfig会检查共享库的文件名和头部信息以决定采用哪个版本。

ldconfig会根据所使用的C库(C libs)来尝试推导到底采用哪一种执行库(比如： libc5、libc6或者glibc)，之后在运行时来链接该库。因此，当编译动态链接库时，明智的做法是通过```-lc```来指定所链接的C库（不要具体指定到底是哪一种libc5、libc6或glibc）。

值得注意的是，其中有一些现存库并没有包含足够的信息来推导它们的类型。因此，*/etc/ld.so.conf*文件也支持显式的指定到底使用哪一种类型的lib库，但这通常只用于那些不能自动推导的可执行库(ELF libs)。配置格式为```dirname=TYPE```，其中```TYPE```可以是libc4、libc5或libc6。运行ldconfig命令通常要求具有超级用户权限，这是因为可能需要对一些root用户的目录和文件具有写权限。

1) **ldconfig选项**
<pre>
   选项                                   描述
------------------------------------------------------------------------------------------------------------------
 -v            Verbose模式，会打印当前的版本号、ldconfig命令所扫描的每一个目录的名称，以及所创建的任何链接。用于覆盖quiet模式.

 -n            只处理在命令行上所指定的目录。并不处理受信任的目录(/lib和/usr/lib)，也不处理/etc/ld.so.conf中所指定的目录。
               隐含了-N的含义

 -N            不要重建cache。除非同时指定了-X选项，否则链接(links)依然会被更新

 -X            不要更新链接。除非同时指定了-N选项，否则cache依然会被重建

-f conf        使用conf以代替/etc/ld.so.conf

-C cache       使用cache以代替/etc/ld.so.cache

-r root        此选项改变应用程序的根目录为ROOT（是调用chroot函数实现的)。选择此项时，系统默认的配置文件/etc/ld.so.conf，
               实际对应的为ROOT/etc/ld.so.conf。如用ldconfig -r /usr/zzz时，打开配置文件/etc/ld.so.conf，实际打开的是
               /usr/zzz/etc/ld.so.conf文件。用此选项可以大大增加动态链接库管理的灵活性

-I            通常情况下，ldconfig搜索动态链接库时将自动建立动态链接库的链接。选择此项时，进入专家模式，需要手动设置链接。
              一般用户不用此项

-p            打印当前cache中保存的目录以及候选lib库
</pre>

2) **ldconfig所涉文件**
<pre>
    文件                                  描述
------------------------------------------------------------------------------------------------------------
/lib/ld.so                   运行时的链接器与加载器

/etc/ld.so.conf              以冒号、空格、tab、换行符或者逗号分割的目录列表，用于指定搜索路径

/etc/ld.so.cache             /etc/ld.so.conf所指定目录下动态链接库名字列表（已排序）
</pre>

## 2. ldconfig使用说明

在使用```ldconfig```时有几个需要注意的地方：

* 往*/lib*、*/usr/lib*目录里添加东西，是不用修改*/etc/ld.so.conf*的，但是完成之后需要调用一下ldconfig，不然这个library可能会找不到

* 想往*/lib*、*/usr/lib*以外的其他目录添加东西的时候，一定要修改*/etc/ld.so.conf*，然后再调用```ldconfig```，不然也会找不到。比如我们安装了一个mysql到*/usr/local/mysql*，```mysql```有一大堆library在*/usr/local/mysql/lib*下面，这时就需要在*/etc/ld.so.conf*下面添加一行: */usr/local/mysql/lib*，保存过后再执行```ldconfig```命令，新的Library才能在程序运行时被找到

* 如果想在上面两个目录以外放lib，但是又不想再*/etc/ld.so.conf*中添加东西（或者没有权限加东西）。那也可以，就是export一个全局变量```LD_LIBRARY_PATH```，然后运行程序的时候就会去这个目录中找library。一般来讲这只是一种临时解决方案，在没有权限或临时需要的时候使用。

* ldconfig做的这些东西跟程序```运行时```有关，与```编译时```没有任何一点关系。编译的时候还是该加```-L```就得加，不要混淆了

* 总之，就是不管做了什么关于library的变动后，最好都ldconfig一下，不然可能会遇到一些意想不到的结果。执行该命令，不会花太多时间，但是后面可能会省事很多





<br />
<br />

**[参看]:**

1. [pkg-config官网](https://www.freedesktop.org/wiki/Software/pkg-config/)

2. [ldconfig命令](http://man.linuxde.net/ldconfig)

3. [ldconfig命令用法笔记](https://blog.csdn.net/philosophyatmath/article/details/51094619)

4. [PKG_CONFIG_PATH变量 与 ld.so.conf 文件](http://www.cnblogs.com/s_agapo/archive/2012/04/24/2468925.html)

5. [Linux下运行时链接库的路径顺序](https://blog.csdn.net/npu_wy/article/details/38642191)
<br />
<br />
<br />





