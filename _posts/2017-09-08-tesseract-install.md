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









<br />
<br />
<br />

