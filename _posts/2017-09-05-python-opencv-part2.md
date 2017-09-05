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

（注意：使用```cv2.THRESH_OTSU```、```cv2.THRESH_TRIANGLE```时,所对应的图像必须是灰度图像)
{% highlight string %}
# -*- coding: utf-8 -*-

import numpy
import cv2
import matplotlib.pyplot



print(cv2.__version__)

# read image, support bmp,jpg,png,tiff format
img = cv2.imread("D:\\timg2.jpg",cv2.IMREAD_GRAYSCALE)

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

![python-opencv-threshold](https://ivanzz1001.github.io/records/assets/img/python/python-opencv-threshold.png)

可以看到这里把阈值设置成了127，对于BINARY方法，当图像中的灰度值大于127的重置像素值为255.


## 3. 自适应阈值

前面看到简单阈值是一种全局性的阈值，只需要规定一个阈值值，整个图像和这个阈值比较。而自适应阈值可以看成一种局部性的阈值，通过规定一个区域大小，比较这个点与区域大小里面像素点的平均值（或者其他特征）的大小关系确定这个像素点是属于黑或者白（如果是二值情况）。使用的函数为：
{% highlight string %}
adaptiveThreshold(src, maxValue, adaptiveMethod, thresholdType, blockSize, C, dst=None)
{% endhighlight %}

* src: 指原图像，原图像应该是灰度图
* maxValue: 指当像素值高于（有时是小于）阈值时应该被赋予的新的像素值
* adaptiveMethod: 指CV_ADAPTIVE_THRESH_MEAN_C 或 CV_ADAPTIVE_THRESH_GAUSSIAN_C
* thresholdType: 指取阈值类型，必须是CV_THRESH_BINARY 或者 CV_THRESH_BINARY_INV
* blockSize: 指用来计算阈值的像素领域大小：3,5,7,...
* C: 指与方法有关的参数。阈值等于均值或者加权值减去这个常数（为0相当于阈值 就是求得领域内均值或者加权值） 

这种方法理论上得到的效果更好，相当于在动态自适应的调整属于自己像素点的阈值，而不是整幅图像都用一个阈值。
{% highlight string %}
# -*- coding: utf-8 -*-

import numpy
import cv2
import matplotlib.pyplot



print(cv2.__version__)

# read image, support bmp,jpg,png,tiff format
img = cv2.imread("D:\\timg2.jpg",cv2.IMREAD_GRAYSCALE)

ret, threshold1 = cv2.threshold(img,127,255,cv2.THRESH_BINARY)
threshold2 = cv2.adaptiveThreshold(img,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,7,2)
threshold3 = cv2.adaptiveThreshold(img,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,7,2)

images = [img,threshold1,threshold2,threshold3]

matplotlib.pyplot.figure()
for i in range(4):
    matplotlib.pyplot.subplot(2,2,i+1)
    matplotlib.pyplot.imshow(images[i],"gray")


matplotlib.pyplot.show()
{% endhighlight %}

![python-opencv-adaptive](https://ivanzz1001.github.io/records/assets/img/python/python-opencv-adaptive.png)

可以看到上述窗口大小使用为11，当窗口越小的时候，得到的图像越细。想象一下，如果把窗口设置足够大以后（不能超过图像大小），那么得到的结果可能和第二幅图像相同了。

{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy 
import matplotlib.pyplot


print(cv2.__version__)


img = cv2.imread("D:\\timg2.jpg")
GrayImage=cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

# 中值滤波
GrayImage= cv2.medianBlur(GrayImage,3)
ret,th1 = cv2.threshold(GrayImage,127,255,cv2.THRESH_BINARY)

#3 为Block size, 5为param1值
th2 = cv2.adaptiveThreshold(GrayImage,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,3,5)
th3 = cv2.adaptiveThreshold(GrayImage,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,3,5)

titles = ['Gray Image', 'Global Thresholding (v = 127)', 'Adaptive Mean Thresholding', 'Adaptive Gaussian Thresholding']
images = [GrayImage, th1, th2, th3]


for i in range(4):
    matplotlib.pyplot.subplot(2,2,i+1)
    matplotlib.pyplot.imshow(images[i],'gray')
    matplotlib.pyplot.title(titles[i])
    matplotlib.pyplot.xticks([])
    matplotlib.pyplot.yticks([])

matplotlib.pyplot.show()
{% endhighlight %}

![python-opencv-adaptive2](https://ivanzz1001.github.io/records/assets/img/python/python-opencv-adaptive2.png)


## 4. Otsu’s二值化











<br />
<br />
参看：

1) [http://blog.csdn.net/jjddss/article/details/72841141](http://blog.csdn.net/jjddss/article/details/72841141)

2) [http://blog.csdn.net/what_lei/article/details/49159655](http://blog.csdn.net/what_lei/article/details/49159655)



<br />
<br />
<br />

