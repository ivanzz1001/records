---
layout: post
title: tesseract的安装
tags:
- python
categories: python
description: tesseract的安装
---

在做图片文字提取时，会使用到tesseract开源软件。这里简要介绍tesseract的安装。这里我们会分别介绍Linux与Windows平台的安装方法。

<!-- more -->
参看:

1. [tesseract官网](https://github.com/tesseract-ocr/)

2. [python使用tesseract-ocr完成验证码识别](http://blog.csdn.net/a349458532/article/details/51490291)

3. [ CentOS7下编译安装tesseract-ocr](http://blog.csdn.net/strugglerookie/article/details/71606540)

## 1. Linux上安装tesseract

可以直接安装已编译好的二进制安装包，也可以通过源代码来编译安装。下面我们通过两节来分别介绍。

### 1.1 二进制包安装tesseract
参看:https://github.com/tesseract-ocr/tesseract/wiki

在很多Linux发行版本下面都有已编译好的Tesseract。安装包的名字通常叫做```tesseract```或者```tesseract-ocr```,你可以通过相应发行版本Linux的包安装工具来查询。此外，tesseract训练好的语言包也通常可以通过Linux发行版本的包管理工具来从repositories获得。但是假如你不能通过这种方式下载到这些训练好的数据（比如<=3.02 或者 tesseract最新训练包），你就需要到其他地方去下载，然后将其拷贝到```tessdata```目录，通常是/usr/share/tesseract-ocr/tessdata或者/usr/share/tessdata。

如下给出一个示例：

1) 安装环境

当前我们安装环境是Centos7.3.1:

如下我们是在Centos7.01:下通过二进制包安装tesseract:
{% highlight string %}
[root@localhost tesseract-3.05.01]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

[root@localhost tesseract-3.05.01]# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 
UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

{% endhighlight %}

2） 安装tesseract

通过yum包管理工具进行安装：
{% highlight string %}
# yum search tesseract

# yum install tesseract.x86_64
{% endhighlight %}

3) 安装pytesseract
{% highlight string %}
[root@localhost tesseract-3.05.01]# pip3 search pytesseract
pytesseract (0.1.7)  - Python-tesseract is a python wrapper for google's Tesseract-OCR

[root@localhost tesseract-3.05.01]# pip3 install pytesseract
{% endhighlight %}

4) 安装pillow
{% highlight string %}
# pip3 install Pillow
{% endhighlight %}


### 1.2 源代码编译安装tesseract

当前(2017-9-9)[github](https://github.com/tesseract-ocr/tesseract)最新版本tesseract为4.0.0版本，还未稳定,git clone下来尝试了一下编译也有比较大问题。当前的稳定版本是3.05.01，因此我们就采用该版本来安装。
<pre>
The latest stable version is 3.05.01, released on June 1, 2017. Latest source code for 3.05 is available from 3.05 branch on github.
</pre>

参看：https://github.com/tesseract-ocr/tesseract/wiki/Compiling


(1) 安装环境

当前我们安装环境是Centos7.3.1:

如下我们是在Centos7.01:下通过二进制包安装tesseract:
{% highlight string %}
[root@localhost tesseract-3.05.01]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

[root@localhost tesseract-3.05.01]# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 
UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

{% endhighlight %}

(2) 安装依赖项
{% highlight string %}
# yum install autoconf
# yum install automake
# yum install libtool
# yum install pkgconfig.x86_64

# yum install libpng12-devel.x86_64
# yum install libjpeg-devel
# yum install libtiff-devel.x86_64
# yum install zlib-devel.x86_64
{% endhighlight %}

这里在安装时也安装训练工具所依赖的库：
{% highlight string %}
# yum install libicu-devel.x86_64
# yum install pango-devel.x86_64
# yum install cairo-devel.x86_64
{% endhighlight %}


(3) 安装Leptonica

Leptonica主要用于图像处理和图像分析，这里我们需要安装的版本>=1.74。

参看：http://www.leptonica.org/download.html
{% highlight string %}
# wget http://www.leptonica.org/source/leptonica-1.74.4.tar.gz
# tar -zxvf leptonica-1.74.4.tar.gz
# cd leptonica-1.74.4

# ./configure
# make
# make install

# make uninstall  #卸载可执行
{% endhighlight %}
这里安装完成之后，默认会安装到/usr/local/lib目录。请执行如下命令：
<pre>
[root@localhost tesseract-src]# pkg-config --list-all | grep lept
lept                      leptonica - An open source C library for efficient image processing and image analysis operations
</pre>
如果没有出现类似如上信息，请将leptconica的库所对应的lept.pc目录添加到PKG_CONFIG_PATH环境变量，否则后面编译tesseract会有问题.
<pre>
# cd /usr/local/lib/pkgconfig/
# ls
lept.pc  libevent.pc  libevent_pthreads.pc  msgpack.pc  

# export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig/  #此方法仅针对当前窗口有效
</pre>
请参看我的另一篇博文：[Linux中pkg-config的使用](https://ivanzz1001.github.io/records/post/linux/2017/09/08/linux-pkg-config)



(4) 安装tesseract
{% highlight string %}
# wget https://github.com/tesseract-ocr/tesseract/archive/3.05.01.tar.gz

# tar -zxvf 3.05.01.tar.gz 

# ./autogen.sh
# ./configure



You can now build and install tesseract by running:
# make
# sudo make install


Training tools can be build and installed (after building of tesseract) with:
# make training
# sudo make training-install


# make uninstall  #卸载可执行
{% endhighlight %}

按如上方式一般能够成功的安装上tesseract：
<pre>
[root@localhost pkgconfig]# tesseract -v
tesseract 3.05.01
 leptonica-1.74.4
  libpng 1.5.13 : libtiff 4.0.3 : zlib 1.2.7
</pre>








<br />
<br />
<br />

