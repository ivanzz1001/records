---
layout: post
title: （转）Python下OpenCV的使用 -- 图像灰度化
tags:
- python
- OpenCV
categories: python
description: Python下OpenCV的使用
---

```灰度化处理```就是将一幅彩色图像转化为灰度图像的过程。彩色图像分为R、G、B三个分量，分别显示出红绿蓝等各种颜色，灰度化就是使彩色的R、G、B分量相等的过程。灰度值最大的像素点比较亮(像素值最大为255，为白色）；反之比较暗（像素最小为0，为黑色）。

<!-- more -->

灰度数字图像是每个像素只有一个采样颜色的图像，这类图像通常显示为从最暗黑色到最亮白色的灰度。灰度图像与黑白图像不同，在计算机图像领域中黑白图像只有黑白两种颜色，灰度图像在黑色与白色之间还有许多级的颜色深度。

在RGB模型中，如果R=G=B时，则彩色表示一种灰度颜色，其中R=G=B的值叫灰度值.




## 1. 图像灰度化算法

图像灰度化算法主要有4种，下面分别介绍。

**(1) 分量法**

将彩色图像中的三分量的亮度作为三个灰度图像的灰度值，可根据应用需要选取一种灰度图像。
<pre>
F1(i,j) = R(i,j)

F2(i,j) = G(i,j)

F3(i,j) = B(i,j)
</pre>

代码示例：
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg4.png")
print(image.shape)
b,g,r = cv2.split(image)   # the order is not r,g,b

cv2.namedWindow("Image")
cv2.imshow("Image",image)

cv2.namedWindow("ImageR")
cv2.imshow("ImageR",r)

cv2.namedWindow("ImageG")
cv2.imshow("ImageG",g)

cv2.namedWindow("ImageB")
cv2.imshow("ImageB",b)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

**(2) 最大值法**

将彩色图像中的三分量亮度最大值作为灰度图的灰度值。
<pre>
F(i,j) = max(R(i,j),G(i,j),B(i,j))
</pre>

代码示例：
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg4.png")
print(image.shape)

#b,g,r = cv2.split(image)   # the order is not r,g,b
#print(b.shape)
#print(type(b))
#print(b.dtype)
#print(image.dtype)

shape = (image.shape[0],image.shape[1])
newImage = numpy.ndarray(shape,image.dtype)


for i in range(image.shape[0]):
    for j in range(image.shape[1]):
       newImage[i,j] = max(image[i,j][0],image[i,j][1],image[i,j][2])

cv2.namedWindow("NewImage")
cv2.imshow("NewImage",newImage)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

** (3) 平均值法**

将彩色图像中的三分量亮度求平均得到一个灰度值。
<pre>
F(i,j) = (R(i,j) + G(i,j) + B(i,j)) /3
</pre>








<br />
<br />
<br />

