---
layout: post
title: Python环境搭建之OpenCV
tags:
- python
categories: python
description: Python环境搭建之OpenCV
---

本文主要讲述在python3.6环境下如何搭建OpenCV开发环境。

<!-- more -->

## 1. OpenCV介绍
Open Source Computer Vision Library.OpenCV于1999年由Intel建立，如今由Willow Garage提供支持。OpenCV是一个基于BSD许可（开源）发行的跨平台计算机视觉库，可以运行在Linux、Windows、MacOS操作系统上。它轻量级而且高效——由一系列 C 函数和少量C++类构成，同时提供了Python、Ruby、MATLAB等语言的接口，实现了图像处理和计算机视觉方面的很多通用算法。

简而言之，通过openCV可实现计算机图像、视频的编辑。广泛应用于图像识别、运动跟踪、机器视觉等领域。

## 2. 安装OpenCV

这里我们是在Win7 Python3.6.2(64bit)的环境下安装OpenCV：
{% highlight string %}
C:\Users\Administrator>python
Python 3.6.2 (v3.6.2:5fd33b5, Jul  8 2017, 04:57:36) [MSC v.1900 64 bit (AMD
 on win32
Type "help", "copyright", "credits" or "license" for more information.
>>>
{% endhighlight %}

OpenCV依赖于numpy、matplotlib、opencv-python三个包。这里我们首先介绍一下python3.6下安装numpy + scipy + matplotlib，注意这里我们安装的numpy为numpy+mkl，因为scipy需要依赖于numpy + mkl。


### 2.1 安装 numpy + scipy + matplotlib
参看：http://blog.csdn.net/yxpyxp12341234/article/details/70436854

**(1) 安装**

直接通过如下命令安装时容易出现问题：
{% highlight string %}
pip install numpy
pip install scipy
pip install matplotlib
{% endhighlight %}


此时，我们需要到 ```http://www.lfd.uci.edu/~gohlke/pythonlibs/``` 下载对应的安装包。这里我们根据自己的环境，下载安装包如下：
<pre>
numpy‑1.13.1+mkl‑cp36‑cp36m‑win_amd64.whl

scipy-0.19.1-cp36-cp36m-win_amd64.whl
</pre> 


下载好后安装```numpy‑1.13.1+mkl‑cp36‑cp36m‑win_amd64.whl```,提示文件名错误：
{% highlight string %}
C:\Users\Administrator>pip install E:\numpy‑1.13.1+mkl‑cp36‑cp36m‑win_amd64.whl
numpy‑1.13.1+mkl‑cp36‑cp36m‑win_amd64.whl is not a valid wheel filename.
{% endhighlight %}


此时，我们需要将将文件名更改一下，变成```numpy-1.12.1-cp36-none-win_amd64.whl```,然后执行如下命令进行安装即可：
{% highlight string %}
pip install E:\numpy-1.12.1-cp36-none-win_amd64.whl
pip install E:\scipy-0.19.1-cp36-cp36m-win_amd64.whl
pip install matplotlib                        # matplotlib直接安装即可
{% endhighlight %}



**(2) 程序验证**

编写如下程序进行验证：
{% highlight string %}
# -*- coding: utf-8 -*-

import numpy
import matplotlib.pyplot

def f(t):
    return numpy.exp(-t)*numpy.cos(2*numpy.pi*t)

t1 = numpy.arange(0.0, 5.0, 0.1)
t2 = numpy.arange(0.0, 5.0, 0.2)

matplotlib.pyplot.figure(1)
matplotlib.pyplot.subplot(211)
matplotlib.pyplot.plot(t1, f(t1), 'bo', t2, f(t2), 'k')

matplotlib.pyplot.subplot(212)
matplotlib.pyplot.plot(t2, numpy.cos(2*numpy.pi*t2), "r--")

matplotlib.pyplot.show()
{% endhighlight %} 

程序运行结果如下：

![numpy_matplotlib](https://ivanzz1001.github.io/records/assets/img/python/numpy_matplotlib.png)










<br />
<br />
<br />

