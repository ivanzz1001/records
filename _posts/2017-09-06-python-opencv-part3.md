---
layout: post
title: （转）Python下OpenCV的使用 -- 二值化
tags:
- python
- OpenCV
categories: python
description: Python下OpenCV的使用
---

本节我们介绍一下Python-OpenCV处理图像的基本操作。

<!-- more -->

<br />
<br />


## 1. 图片读写和显示操作
安装好OpenCV之后，首先尝试加载一张最简单的图片并显示出来，代码示例如下：
{% highlight string %}
import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg4.png")
print(image.shape)
cv2.imshow("image",image)


k = cv2.waitKey(0)
if k == ord('s'):                  # wait for 's' key to save and exit
    cv2.imwrite("D:\\ImageNet\\output1.png",image)


cv2.destroyAllWindows()
{% endhighlight %}


## 2. 获取图片属性
示例代码如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg4.png")
print(image.shape)


if image.shape[2] == 2:
    print("Gray Image")
elif image.shape[2] == 3:
    print("RGB Image")

print(image.size)

print(image.dtype)
{% endhighlight %}


## 3. 输出文本

在处理图片的时候，我们经常会需要把一些信息直接以文字的形式输出在图片上，下面的代码将实现这个效果：

{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)


# def putText(img, text, org, fontFace, fontScale, color, thickness=None, lineType=None, bottomLeftOrigin=None):
cv2.putText(image, "I Iove you!", (20,40), 0, 0.8, (0,0,255),2)

cv2.imshow("image",image)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}


![python-opencv-puttext](https://ivanzz1001.github.io/records/assets/img/python/python-opencv-puttext.png)


## 4. 图片缩放

下面的例子将实现缩放图片并保存，这个在使用 OpenCV 做图像处理的时候都是很常用的操作：
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)


# def resize(src, dsize, dst=None, fx=None, fy=None, interpolation=None): # real signature unknown; restored from __doc__

#res = cv2.resize(image,None,fx=2, fy=2, interpolation = cv2.INTER_CUBIC)

# or

height, width = image.shape[:2]
res = cv2.resize(image,(2*width, 2*height), interpolation = cv2.INTER_CUBIC)

cv2.imshow("image",res)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}


## 5. 








<br />
<br />
<br />

