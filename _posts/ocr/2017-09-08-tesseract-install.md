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

当前(2017-9-9)[github](https://github.com/tesseract-ocr/tesseract)最新版本tesseract为4.0.0版本。我们可以同时安装当前最新稳定版本3.05.1和最新版本4.0.0。这里以安装tesseract 4.0.0版本为例：

参看：https://github.com/tesseract-ocr/tesseract/wiki/Compiling


**(1) 安装环境**

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

**(2) 安装依赖项**
{% highlight string %}
# yum install autoconf
# yum install automake
# wget http://www6.atomicorp.com/channels/atomic/centos/7/x86_64/RPMS/autoconf-archive-2012.09.08-4.el7.art.noarch.rpm
# rpm -ivh autoconf-archive-2012.09.08-4.el7.art.noarch.rpm --nosignature  --nodigest
# yum install libtool
# yum install pkgconfig.x86_64

# yum install libpng12-devel.x86_64


# yum install libjpeg-devel
或通过源代码安装,通过源代码安装我们需要为libjpeg建立pc文件（请见后文）
# wget http://www.ijg.org/files/jpegsrc.v8d.tar.gz
# tar -zxvf jpegsrc.v8d.tar.gz
# cd jpeg-8d/
# ./configure --enable-shared
# make
# make install



# yum install libtiff-devel.x86_64
# yum install zlib-devel.x86_64
{% endhighlight %}

手动安装libjpeg，可能没有自动生成libjpeg.pc文件，我们手动建立：
{% highlight string %}
prefix=/usr/local
exec_prefix=${prefix}
libdir=${prefix}/lib
includedir=${prefix}/include

Name: libjpeg
Description: JPEG image codec
Version: 
Libs: -L${libdir} -ljpeg
Cflags: -I${includedir}
{% endhighlight %}
请根据libjpeg的实际安装位置，调整上述代码然后保存为libjpeg.pc，然后再将该文件拷贝到pkg-config能找到的位置，比如本人机器上保存在/usr/local/lib/pkgconfig/libjpeg.pc。

<br />


这里在安装时也安装训练工具所依赖的库(tesseract 4.0.0版本需要最新libicu-devel，需手动安装)：
{% highlight string %}
# wget http://download.icu-project.org/files/icu4c/58.2/icu4c-58_2-src.tgz
# tar -zxvf icu4c-58_2-src.tgz
# cd icu/source
# chmod +x runConfigureICU configure install-sh
# ./runConfigureICU Linux/gcc --with-library-bits=64
# make
# make check
# make install


# yum install pango-devel.x86_64
# yum install cairo-devel.x86_64
{% endhighlight %}


**(3) 安装Leptonica**

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



**(4) 安装tesseract**
{% highlight string %}
# wget https://github.com/tesseract-ocr/tesseract/archive/master.zip

# mkdir -p /opt/tesseract4.0

# unzip master.zip 

# ./autogen.sh
# ./configure --prefix=/opt/tesseract4.0 --disable-graphics



You can now build and install tesseract by running:
# make
# sudo make install


Training tools can be build and installed (after building of tesseract) with:
# make training
# sudo make training-install


It is also useful, but not required, to build ScrollView.jar:
# make ScrollView.jar
# export SCROLLVIEW_PATH=$PWD/java



# make uninstall  #卸载可执行
# make training-uninstall #卸载可执行
{% endhighlight %}

按如上方式一般能够成功的安装上tesseract：
<pre>
[root@localhost tesseract-master]# ls /opt/tesseract4.0/
bin  include  lib  share

[root@localhost tesseract-master]# /opt/tesseract4.0/bin/tesseract -v
tesseract 4.00.00alpha
 leptonica-1.74.4
  libpng 1.5.13 : libtiff 4.0.3 : zlib 1.2.7

 Found AVX2
 Found AVX
 Found SSE
</pre>

一般情况下tessdata_dir目录为/opt/tesseract4.0/share/tessdata，如果tessdata目录在其他地方，我们可以通过```TESSDATA_PREFIX```来进行设置。假设我们在/opt/tess_best/tessdata目录下存放训练数据的话，我们可以将```TESSDATA_PREFIX```设置为/opt/tess_best.

## 2. windows上安装tesseract

略

## 3. 下载训练语言包

在tesseract官网的langdata工程下有很多语言包，我们可以将这些语言包下载下来，然后自己通过这些语言包来进行训练，来获得我们自己的训练数据。
{% highlight string %}
# git clone https://github.com/tesseract-ocr/langdata.git
{% endhighlight %}
语言包下载下来之后，把里面自己所需要的语言复制到安装目录下的share/tessdata目录。例如我们上面把tesseract安装到了/opt/tesseract4.0目录下，则这里我们可以将所需要的语言包复制到/opt/tesseract4.0/share/tessdata目录下。

参看：https://github.com/tesseract-ocr/langdata

## 4. 下载已经训练好的数据

在tesseract官网的tessdata工程下有很多已经为我们训练好的各种语言的数据，我们如果不想自己训练，就可以直接采用。由于整个工程训练数据比较大，我们可以只下载自己所需要的训练数据即可。
{% highlight string %}
# git clone https://github.com/tesseract-ocr/tessdata.git        # 这里我们全部下载(for general purpose)

# git clone https://github.com/tesseract-ocr/tessdata_best.git   #(best for lstm)
{% endhighlight %}

下载下来之后，把自己所需要的训练数据复制到安装目录下的share/tessdata目录。例如我们上面把tesseract安装到了/opt/tesseract4.0目录下，则这里我们可以将所需要的语言包复制到/opt/tesseract4.0/share/tessdata目录下。

不过因为tessdata(tessdata_best)数据太大，针对中文我们一般只需要如下几个文件：
<pre>
[root@localhost tessdata_best]# ls
chi_sim.traineddata  chi_sim_vert.traineddata  eng.traineddata  ori.traineddata  osd.traineddata
</pre>

我们可以通过如下方式来下载：
{% highlight string %}
# mkdir tessdata && cd tessdata
# wget https://github.com/tesseract-ocr/tessdata/raw/master/chi_sim.traineddata
# wget https://github.com/tesseract-ocr/tessdata/raw/master/eng.traineddata
# wget https://github.com/tesseract-ocr/tessdata/raw/master/ori.traineddata
# wget https://github.com/tesseract-ocr/tessdata/raw/master/osd.traineddata

# mkdir tessdata_best && cd tessdata_best                           # (best for lstm)
# wget https://github.com/tesseract-ocr/tessdata_best/raw/master/chi_sim.traineddata
# wget https://github.com/tesseract-ocr/tessdata_best/raw/master/chi_sim_vert.traineddata
# wget https://github.com/tesseract-ocr/tessdata_best/raw/master/eng.traineddata
# wget https://github.com/tesseract-ocr/tessdata_best/raw/master/ori.traineddata
# wget https://github.com/tesseract-ocr/tessdata_best/raw/master/osd.traineddata
{% endhighlight %}



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

<br />

**说明：**
<pre>
tesseract官网有很多训练好的语言包版本，tesseract中有些命令参数必须结合对应的语言包版本才能使用。

比如当我们使用 --oem 2模式时（即 Tesseract + LSTM模式），就必须配合 LSTM + lang models 类型的语言包.
</pre>

<br />

### 5.2 测试识别英文

这里我们只是简单测试一下对英文的识别。现有如下图片：

![myscan](/records/assets/img/tesseract/tesseract-english-1.png)

执行如下命令进行识别：
{% highlight string %}
[root@localhost workspace]# /opt/tesseract4.0/bin/tesseract tesseract-english-1.png out
Tesseract Open Source OCR Engine v4.00.00alpha with Leptonica
Warning. Invalid resolution 0 dpi. Using 70 instead.
Estimating resolution as 129

[root@localhost workspace]# cat out.txt 
Tesseract is a command-line program, so first open a terminal or command prompt. The command
is used like thi

tesseract imagename outputbase [-1 lang] [-psm pagesegnode] [configfile.

{% endhighlight %}

从上面的识别结果来看，效果还OK。

### 5.3 识别中文

这里我们只是简单测试一下对中文的识别。现有如下图片：

![myscan](/records/assets/img/tesseract/tesseract-chinese-1.png)


执行如下命令进行识别：
{% highlight string %}
[root@localhost workspace]# /opt/tesseract4.0/bin/tesseract tesseract-chinese-1.png out -l chi_sim
Tesseract Open Source OCR Engine v4.00.00alpha with Leptonica
Warning. Invalid resolution 0 dpi. Using 70 instead.
Estimating resolution as 219

[root@localhost workspace]# cat out.txt 
党 的 十 八大 以 来 ， 习 近 平 总 书记 在 系列 重要 讲话 中 对 全 面 从 严 治 党 提出 很 多 新
理念 ， 涵 盖 党 的 思想 建设 、 组 织 建 设 、 作 风 建 设 、 反 腐 倡 廉 建设 和 制度 建设 ， 概 括
起 来 有 十 个 方面 ， 形 成 了 完 念 体系 ， 成 为 全 面 从 严 治 党 的 思想 统领 。 党 建
网 微 平台 邀 您 一 起 学 习 。

{% endhighlight %}


```如上出现相应的警告信息，原因暂时未明。```


## 6. tesseract文件介绍

tesseract字符提取时会用到很多文件，这里做一个介绍。

### 6.1 特殊文件
* osd.traineddata： 主要用于方向和脚本检测
* equ.traineddata： 主要用于数学、等式检测


### 6.2 tesseract4.0.0更新的文件
当使用```best```目录中的新模型时，tesseract只支持基于LSTM的OCR引擎，而legacy引擎是不支持这些文件的。因此，当tesseract的oem模式选为0或2时，是不支持```best```目录中的新模型的。

可以用如下命令来检测文字方向及所用语言：
<pre>
./tesseract4 pic/tesseract-chinese-1.png stdout -l osd --psm 0 --oem 0
</pre>
```这里osd必须为支持legacy引擎的osd.traineddata文件```




<br />
<br />
**[参考]:**

1. [Tesseract-OCR识别中文与训练字库实例](http://www.cnblogs.com/wzben/p/5930538.html)

2. [tesseract-ocr 第四课 如何训练新语言](http://www.codeweblog.com/tesseract-ocr-%E7%AC%AC%E5%9B%9B%E8%AF%BE-%E5%A6%82%E4%BD%95%E8%AE%AD%E7%BB%83%E6%96%B0%E8%AF%AD%E8%A8%80/)

3. [使用Tesseract训练lang文件并OCR识别集装箱号](http://www.jianshu.com/p/5f847d8089ce)
<br />
<br />
<br />

