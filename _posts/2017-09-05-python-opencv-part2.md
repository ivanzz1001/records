---
layout: post
title: （转）Python下OpenCV的使用 -- 二值化
tags:
- python
- OpenCV
categories: python
description: Python下OpenCV的使用
---

图像的阈值处理一般使得图像的像素值更单一、图像更简单。阈值可以分为全局性质的阈值，也可以分为局部性质的阈值，可以是单阈值的也可以是多阈值的。当然阈值越多是越复杂的。下面将介绍opencv下的三种阈值方法：

* 简单阈值
* 自适应阈值
* Otsu’s二值化

<!-- more -->


## 1. 图像二值化介绍

定义：图像的二值化，就是将图像上的像素点的灰度值设置为0或255，也就是将整个图像呈现出明显的只有黑和白的视觉效果。

一幅图像包括目标物体、背景还有噪声，要想从多值的数字图像中直接提取出目标物体，常用的方法就是设定一个阈值，用T将图像的数据分成两部分：大于T的像素群和小于T的像素群。这是研究灰度变换的最特殊的方法，称为图像的二值化(Binarization)。



## 2. 简单阈值
简单阈值当然是最简单的，选取一个全局阈值，然后就把整幅图像分成了非黑即白的二值图像了。Python-OpenCV提供了阈值函数:
{% highlight string %}
threshold(src, thresh, maxval, type, dst=None)
{% endhighlight %}
* src: 指原图像，原图像应该是灰度图
* thresh： 用来对像素值进行分类的阈值
* maxval: 指当像素值高于（有时是小于）阈值时应该被赋予的新的像素值
* type: 指不同的阈值方法。 

该函数有两个返回值，第一个retval(得到的阈值值（在后面一个方法中会用到))，第二个就是阈值化后的图像。

（注意：使用cv2.THRESH_OTSU、cv2.THRESH_TRIANGLE时,所对应的图像必须是灰度图像。
{% highlight string %}
# -*- coding: utf-8 -*-

import numpy
import cv2
import matplotlib.pyplot



print(cv2.__version__)

# read image, support bmp,jpg,png,tiff format
img = cv2.imread("D:\\timg1.jpg",cv2.IMREAD_GRAYSCALE)

ret,thresh1 = cv2.threshold(img,127,255,cv2.THRESH_BINARY)
ret,thresh2 = cv2.threshold(img,127,255,cv2.THRESH_BINARY_INV)

ret,thresh3 = cv2.threshold(img,127,255,cv2.THRESH_MASK)
ret,thresh4 = cv2.threshold(img,127,255,cv2.THRESH_OTSU)  

ret,thresh5 = cv2.threshold(img,127,255,cv2.THRESH_TOZERO)
ret,thresh6 = cv2.threshold(img,127,255,cv2.THRESH_TOZERO_INV)

ret,thresh7 = cv2.threshold(img,127,255,cv2.THRESH_TRIANGLE)
ret,thresh8 = cv2.threshold(img,127,255,cv2.THRESH_TRUNC)


titles = ["IMG","BINARY","BINARY_INV","MASK","OTSU","TOZERO","TOZERO_INV","TRIANGLE","TRUNC"]
images = [img,thresh1,thresh2,thresh3,thresh4,thresh5,thresh6,thresh7,thresh8]
#titles = ["IMG","BINARY","BINARY_INV","MASK","TOZERO","TOZERO_INV","TRUNC"]
#images = [img,thresh1,thresh2,thresh3,thresh5,thresh6,thresh8]

for i in range(9):
    matplotlib.pyplot.subplot(3,3,i+1)
    matplotlib.pyplot.imshow(images[i],"gray")
    matplotlib.pyplot.title(titles[i])
    matplotlib.pyplot.xticks([])
    matplotlib.pyplot.yticks([])

matplotlib.pyplot.show()
{% endhighlight %}















<br />
<br />
<br />

