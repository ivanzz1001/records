---
layout: post
title: （转）Python下OpenCV的使用 -- 基本操作
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


## 5. 图像平移
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)


rows,cols = image.shape[:2]


M = numpy.float32([[1,0,100],[0,1,50]])
dst = cv2.warpAffine(image,M,(cols,rows))

cv2.imshow("image",dst)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}


## 6. 图像旋转
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)



rows,cols = image.shape[:2]

M = cv2.getRotationMatrix2D((cols/2,rows/2),90,1)
dst = cv2.warpAffine(image,M,(cols,rows))


cv2.imshow("image",dst)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}

## 7. 仿射变换
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy
import matplotlib.pyplot

image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)

rows,cols = image.shape[:2]

pts1 = numpy.float32([[50,50],[200,50],[50,200]])
pts2 = numpy.float32([[10,100],[200,50],[100,250]])

M = cv2.getAffineTransform(pts1,pts2)

dst = cv2.warpAffine(image,M,(cols,rows))


cv2.imshow("image",dst)
cv2.waitKey(0)

cv2.destroyAllWindows()

{% endhighlight %}

## 8. 通道的拆分、合并处理
对于一张图片的 R、G、B 通道，我们可以很方便的使用 OpenCV 获取并分离或者合并：

(这是将图像灰度化处理的一种方式)
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy



image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)

b,g,r = cv2.split(image)
img = cv2.merge((b,g,r))

cv2.imshow("image",img)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}


## 9. 图片添加边距
{% highlight string %}
# -*- coding: utf-8 -*-


import cv2
import numpy



image = cv2.imread("D:\\ImageNet\\timg2.jpg")
print(image.shape)

BLUE = [255,0,0]

replicate = cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_REPLICATE)
reflect = cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_REFLECT)
reflect101 = cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_REFLECT_101)
wrap = cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_WRAP)
constant= cv2.copyMakeBorder(image,10,10,10,10,cv2.BORDER_CONSTANT,value=BLUE)


#cv2.imshow("image",replicate)
cv2.imshow("image",reflect)
#cv2.imshow("image",reflect101)
#cv2.imshow("image",wrap)
#cv2.imshow("image",constant)
#cv2.imshow("image",image)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}



<br />
<br />

**[参看]**

1. [Python-OpenCV 处理图像（一）：基本操作](https://segmentfault.com/a/1190000003742422)



<br />
<br />
<br />

