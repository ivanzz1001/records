---
layout: post
title: tesseract的安装
tags:
- ocr
categories: ocr
description: tesseract的安装
---

在做图片文字提取时，会使用到tesseract开源软件。这里简要介绍tesseract的安装，需要安装两个部分：tesseract引擎、训练好的语言数据。这里我们会分别介绍Linux与Windows平台的安装方法。

<!-- more -->
参看:

1. [tesseract官网](https://github.com/tesseract-ocr/)



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
# wget http://www6.atomicorp.com/channels/atomic/centos/7/x86_64/RPMS/autoconf-archive-2012.09.08-4.el7.art.noarch.rpm
# rpm -ivh autoconf-archive-2012.09.08-4.el7.art.noarch.rpm --nosignature  --nodigest
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


## 2. windows上安装tesseract


## 3. 下载训练语言包

在tesseract官网的langdata工程下有很多语言包，我们可以将这些语言包下载下来，然后自己通过这些语言包来进行训练，来获得我们自己的训练数据。
{% highlight string %}
# git clone https://github.com/tesseract-ocr/langdata.git
{% endhighlight %}
语言包下载下来之后，把里面自己所需要的语言复制到/usr/local/share/tessdata目录下。

参看：https://github.com/tesseract-ocr/langdata

## 4. 下载已经训练好的数据

在tesseract官网的tessdata工程下有很多已经为我们训练好的各种语言的数据，我们如果不想自己训练，就可以直接采用。由于整个工程训练数据比较大，我们可以只下载自己所需要的训练数据即可。

```(注意：我们这里采用的是3.05.01版本的tesseract，应该要采用对应版本的训练好的数据)```
{% highlight string %}
# git clone https://github.com/tesseract-ocr/tessdata.git     # 这里我们全部下载
{% endhighlight %}

下载下来之后，把自己所需要的训练数据复制到/usr/local/share/tessdata目录下。


## 5. 测试

在这一节，我们首先会讲述一下tesseract的基本用法，然后给出一些简单的文字识别示例，对与更复杂的测试，我们后续会再分章节进行讲解。

### 5.1 tesseract基本用法
tesseract是一个命令行程序，其基本用法如下：
{% highlight string %}
# tesseract imagename outputbase [-l lang] [-psm pagesegmode] [configfile...]
{% endhighlight %}

因此，我们可以通过如下命令从myscan.png中识别文字，然后将结果保存到out.txt中：
{% highlight string %}
# tesseract myscan.png out.txt
{% endhighlight %}

默认情况下，tesseract识别的语言是英文，我们可以通过如下方式来执行识别其他语言，比如德语：
{% highlight string %}
# tesseract myscan.png out -l deu
{% endhighlight %}

也可以指定同时识别多种语言，比如：英语 和 德语
{% highlight string %}
# tesseract myscan.png out -l eng+deu
{% endhighlight %}

tesseract也包括一个hOCR模式，它可以根据每个词的坐标产生一个特殊的HTML文件。而这可以通过使用[Hocr2PDF工具](https://exactcode.com/opensource/exactimage/)产生一个可搜索的pdf文件。要想使用该功能，可以添加```hocr```配置选项，例如：
{% highlight string %}
# tesseract myscan.png out hocr
{% endhighlight %}


对于版本>=3.03的tesseract，我们也可以直接通过如下方式产生一个可搜索的pdf文件：
{% highlight string %}
# tesseract myscan.png out pdf
{% endhighlight %}


### 5.2 测试识别英文

这里我们只是简单测试一下对英文的识别。现有如下图片：

![myscan](/records/assets/img/tesseract/tesseract-english-1.png)

执行如下命令进行识别：
<pre>
[root@localhost workspace]# tesseract myscan.png out
Tesseract Open Source OCR Engine v3.05.01 with Leptonica
Warning. Invalid resolution 0 dpi. Using 70 instead.

[root@localhost workspace]# cat out.txt 
Tesserzcl is a commandrline program, so ﬁrst open a (ermina‘ or command prompt. The command
I: used m m:

teSsEr-act imagsname outputbasE [71 lang] [rpsm pagEsEgmude] [(0nfigfile.
</pre>

从上面的识别结果来看，效果还不甚理想。

### 5.3 识别中文

这里我们只是简单测试一下对中文的识别。现有如下图片：

![myscan](/records/assets/img/tesseract/tesseract-chinese-1.png)


执行如下命令进行识别：
<pre>
[root@localhost workspace]# tesseract tesseract-chinese-1.png out -l chi_sim
Tesseract Open Source OCR Engine v3.05.01 with Leptonica
Warning. Invalid resolution 0 dpi. Using 70 instead.
[root@localhost workspace]# cat out.txt 
责朐十/\大L义耒 ' 习近平总书记丕丑系列重要讲话中又寸全面从严蒙合竟提出很多新
理含'涵盖责朐思惧建设 组织健设、 1乍风建设、 反腐倡廉建章良租制度建设'概茅舌
起耒有十个方面'形成了完 含体系'成为全面从严蒙合竞朐思想统领. 党建
网微平台趟丧题锢学习.

</pre>





<br />
<br />
<br />

