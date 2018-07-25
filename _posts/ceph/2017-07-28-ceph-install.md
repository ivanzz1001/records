---
layout: post
title: ceph源代码编译
tags:
- ceph
categories: ceph
description: ceph源代码编译
---

本文我们注意介绍一些ceph hammer版本的编译。具体安装环境如下(ceph编译时需要耗费极大量内存，建议内存至少4G以上)：

<!-- more -->
<pre>
[root@ceph001-node1 /]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.1.1503 (Core) 
Release:        7.1.1503
Codename:       Core

[root@ceph001-node1 /]# uname -a
Linux ceph001-node1 3.10.0-229.el7.x86_64 #1 SMP Fri Mar 6 11:36:42 
UTC 2015 x86_64 x86_64 x86_64 GNU/Linux
</pre>





## 获取ceph hammer安装源文件
ceph源代码：https://github.com/ceph/ceph
{% highlight string %}
yum install git
git clone -b hammer https://github.com/ceph/ceph.git
{% endhighlight %}

## 安装ceph

1） 查看安装步骤

在下载下来的安装源文件目录下查看安装说明：
<pre>
[root@localhost ceph]# cat INSTALL 
Installation Instructions
*************************

When pulling from git, use the --recursive option to include sub-modules:

$ git clone --recursive https://github.com/ceph/ceph.git

And then build the configure script with:

$ ./autogen.sh

Then the usual:

$ ./configure
$ make

Note that if the FUSE library is not found, the user-space fuse client
will not be built.

If you are doing development, you may want to do

$ CXXFLAGS="-g -pg" ./configure

or similar to avoid the default (-g -O2), which includes optimizations
(-O2).
</pre>


2） 安装依赖项
{% highlight string %}
./install-deps.sh
{% endhighlight %}

3） 执行```./autogen.sh```

在执行过程中提示没有libtoolize,及gcc,g++等工具，这里执行安装：
{% highlight string %}
yum install libtool
yum install gcc
yum install gcc-c++
{% endhighlight %}

4) 执行```./configure```

如果不想要依赖于google-perftools，请使用```./configure --without-tcmalloc```

在执行过程中提示需要安装对应的依赖库：
{% highlight string %}
yum install snappy-devel
yum install leveldb-devel
yum install libuuid-devel
yum install libblkid-devel
yum install libudev-devel
yum install  keyutils-libs-devel
yum install cryptopp-devel
yum install fuse-devel
yum install libatomic_ops-devel
yum install libaio-devel
yum install xfsprogs-devel
yum install boost-devel
yum install libedit-devel
yum install expat-devel
{% endhighlight %}

5) 执行```make```进行编译



<br />
<br />

**参考**:

1. [指定ceph版本下载](https://github.com/ceph/ceph/releases?after=v12.2.4)

<br />
<br />
<br />

