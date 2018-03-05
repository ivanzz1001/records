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

cv2.namedWindow("NewImageMax")
cv2.imshow("NewImageMax",newImage)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

**(3) 平均值法**

将彩色图像中的三分量亮度求平均得到一个灰度值。
<pre>
F(i,j) = (R(i,j) + G(i,j) + B(i,j)) /3
</pre>


代码示例如下：
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
       newImage[i,j] = (int(image[i,j][0]) + int(image[i,j][1]) + int(image[i,j][2])) / 3

cv2.namedWindow("NewImageAver")
cv2.imshow("NewImageAver",newImage)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

**(4) 加权平均法**

根据重要性及其他指标，将三个分量以不同的权值进行加权平均。由于人眼对绿色的敏感最高，对蓝色敏感最低，因此按下式对RGB分量进行加权平均能得到较合理的灰度图像。
<pre>
F(i,j) = 0.30R(i,j) + 0.59G(i,j) + 0.11B(i,j))
</pre>

代码示例如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg4.png")
print(image.shape)

b,g,r = cv2.split(image)   # the order is not r,g,b
#print(b.shape)
#print(type(b))
#print(b.dtype)
#print(image.dtype)

shape = (image.shape[0],image.shape[1])
newImage = numpy.ndarray(shape,image.dtype)


for i in range(image.shape[0]):
    for j in range(image.shape[1]):
        newImage[i,j] = 0.11 * image[i,j][0] + 0.59 * image[i,j][1] + 0.30 * image[i,j][2]

cv2.namedWindow("NewImageWeightAver")
cv2.imshow("NewImageWeightAver",newImage)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

我们可以调整上面的分量，比如:
<pre>
newImage[i, j] = 0.10 * image[i, j][0] + 0.65 * image[i, j][1] + 0.25 * image[i, j][2]
</pre>

上面的公式可以看出绿色（G 分量）所占的比重比较大，所以有时候也会直接取G 分量进行灰度化

```注意： cv2.split()函数分割出来的颜色三个分量顺序是BGR```


<br />
<br />

**[参看]:**

1. [Python-OpenCV 处理图像（七）：图像灰度化处理](https://segmentfault.com/a/1190000003755100)

2. [图像灰度化的三种方法及matlab,c++,python实现](http://blog.csdn.net/what_lei/article/details/48681903)



<br />
<br />
<br />

