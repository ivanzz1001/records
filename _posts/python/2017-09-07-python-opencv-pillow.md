---
layout: post
title: Python图像处理库Pillow入门
tags:
- python
categories: python
description: Python图像处理库Pillow入门
---

Pillow是Python里的图像处理库（PIL: Python Image Library)，提供了广泛的文件格式支持，强大的图像处理能力，主要包括图像存储、图像显示、格式转换以及基本的图像处理操作等。

<!-- more -->


本文我们简单介绍一下Pillow的安装及使用方法。


## 1. Pillow的安装
安装方法很简单，直接通过如下命令即可完成安装：
{% highlight string %}
pip install Pillow
{% endhighlight %}


## 2. 使用Image类
PIL最重要的类是 Image class，你可以通过多种方法创建这个类的实例。可以从文件加载图像，或者从scratch创建：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

image = PIL.Image.open("D:\\ImageNet\output.png")

print(image.format,image.size,image.mode)

image.show("output.png")
{% endhighlight %}
format这个属性标识了图像来源。如果图像不是从文件读取，则它的值就是None。 size属性是一个二元tuple，包含width和height（宽度和高度，单位都是px)。mode属性定义了图像bands的数量和名称，以及像素类型和深度。常见的modes有"L"(luminance)标识灰度图像，"RGB"表示真彩色图像，"CMYK"表示出版图像。



## 3. 读写图像
PIL 模块支持大量图片格式。在使用Image模块的open()函数从磁盘读取文件,你不需要知道文件格式就能打开它，这个库能够根据文件内容自动确定文件格式。要保存文件，使用Image类的save()方法。保存文件的时候文件名就变得很重要了。除非你指定格式，否则这个库将会以文件名的扩展名的扩展名作为格式保存。
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

try:
    image = PIL.Image.open("D:\\ImageNet\\output.png")
    image.save("D:\\ImageNet\\test_output.jpg")
except IOError:
    print("save file error")
{% endhighlight %}

## 4. 创建缩略图
缩略图是网络开发或图像软件预览常用的一种基本技术，使用Python的Pillow图像库可以很方便的建立缩略图，如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image
import glob
import os


# create thumbnail
size = (128,128)

for infile in glob.glob("D:\\ImageNet\\*.jpg"):
    f, ext = os.path.splitext(infile)
    image = PIL.Image.open(infile)
    image.thumbnail(size,PIL.Image.ANTIALIAS)
    image.save(f+".thumbnail","JPEG")
{% endhighlight %}
上段代码对ImageNet下的jpg图片文件全部创建缩略图，并保存。 glob模块是一种智能化的文件名匹配技术，在批量图像处理中经常会用到。

注意： Pillow库不会直接解码或者加载图像栅格数据。当你打开一个文件，只会读取文件头信息用来确定格式、颜色模式、大小等等，文件的剩余部分不会主动处理。这意味着打开一个图像文件的操作十分快速，跟图片大小和压缩方式无关。

## 5. 图像的剪切、粘贴与合并操作
Image类包含的方法允许你操作图像部分选区，PIL.Image.Image.crop()方法获取图像的一个子矩形选区，然后可以对这个选区进行处理并粘贴回原图。
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")

box = (250,100,450,300)
region = image.crop(box)

region = region.transpose(PIL.Image.ROTATE_180)
image.paste(region, box)

image.show("RotateRegion")
{% endhighlight %}

矩形选区有一个4元元组定义，分别表示左、上、右、下的坐标。这个库以左上角为坐标原点，单位是px，所以上诉代码复制了一个 200×200 pixels 的矩形选区。这个选区现在可以被处理并且粘贴到原图。

当你粘贴矩形选区的时候必须保证尺寸一致。此外，矩形选区不能在图像外。然而你不必保证矩形选区和原图的颜色模式一致，因为矩形选区会被自动转换颜色。

## 6. 分离和合并颜色通道
对于多通道图像，有时候在处理时希望能够分别对每个通道处理，处理完成后重新合成多通道，在Pillow中，很简单，如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")
r,g,b = image.split()
im = PIL.Image.merge("RGB", (r,g,b))
{% endhighlight %}
对于split()函数，如果是单通道的，则返回其本身，否则，返回各个通道


## 7. 几何变换
对图像进行几何变换是一种基本处理，在Pillow中包括resize( )和rotate( )，如用法如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")

img_resize = image.resize((128,128))
img_rotate = image.rotate(45)           # degree conter-clockwise
img_rotate.show()
{% endhighlight %}

其中，resize()函数的参数是一个新图像大小的元祖，而rotate( )则需要输入顺时针的旋转角度。在Pillow中，对于一些常见的旋转作了专门的定义：
<pre>
out = image.transpose(Image.FLIP_LEFT_RIGHT)
out = image.transpose(Image.FLIP_TOP_BOTTOM)
out = image.transpose(Image.ROTATE_90)
out = image.transpose(Image.ROTATE_180)
out = image.transpose(Image.ROTATE_270)
</pre>

## 8. 颜色空间变换
在处理图像时，根据需要进行颜色空间的转换，如将彩色转换为灰度：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")

cmyk = image.convert("CMYK")
gray = image.convert("L")
{% endhighlight %}

## 9. 图像滤波
图像滤波在ImageFilter模块中，在该模块中，预先定义了很多增强滤波器，可以通过filter()函数使用，预定义滤波器包括：

* BLUR
* CONTOUR
* DETAIL
* EDGE_ENHANCE
* EDGE_ENHANCE_MOR
* EMBOSS
* FIND_EDGES
* SMOOTH
* SMOOTH_MORE
* SHARPEN

其中BLUR就是均值滤波，CONTOUR找轮廓，FIND_EDGES边缘检测，使用该模块时，需先导入，使用方法如下：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image
import PIL.ImageFilter

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")


outF = image.filter(PIL.ImageFilter.DETAIL)
conF = image.filter(PIL.ImageFilter.CONTOUR)
edgeF = image.filter(PIL.ImageFilter.FIND_EDGES)
image.show()
outF.show()
conF.show()
edgeF.show()
{% endhighlight %}

除此以外，ImageFilter模块还包括一些扩展性强的滤波器：
<pre>
滤波器名称： class PIL.ImageFilter.GaussianBlur(radius=2)
描述： Gaussian blur filter.
参数说明： radius – Blur radius.


滤波器名称： class PIL.ImageFilter.UnsharpMask(radius=2, percent=150, threshold=3)
描述： Unsharp mask filter.
      See Wikipedia’s entry on digital unsharp masking for an explanation of the parameters.

</pre>
更多详细内容可以参考：[PIL/ImageFilter](http://pillow-cn.readthedocs.org/en/latest/_modules/PIL/ImageFilter.html#GaussianBlur)


## 10. 图像增强
图像增强也是图像预处理中的一个基本技术，Pillow中的图像增强函数主要在ImageEnhance模块下，通过该模块可以调节图像的颜色、对比度和饱和度和锐化等：
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image
import PIL.ImageEnhance

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")


imgEH = PIL.ImageEnhance.Contrast(image)
imgEH.enhance(1.3).show("30% more contrast")

imgCH = PIL.ImageEnhance.Color(image)
imgCH.enhance(1.3).show("30% more color")
{% endhighlight %}
图像增强：

<pre>
class PIL.ImageEnhance.Color(image)
Adjust image color balance.
This class can be used to adjust the colour balance of an image, in a manner similar to the controls on a colour
TV set. An enhancement factor of 0.0 gives a black and white image. A factor of 1.0 gives the original image.

class PIL.ImageEnhance.Contrast(image)
Adjust image contrast.
This class can be used to control the contrast of an image, similar to the contrast control on a TV set. 
An enhancement factor of 0.0 gives a solid grey image. A factor of 1.0 gives the original image.

class PIL.ImageEnhance.Brightness(image)
Adjust image brightness.
This class can be used to control the brighntess of an image. An enhancement factor of 0.0 gives a black image. 
A factor of 1.0 gives the original image.

class PIL.ImageEnhance.Sharpness(image)
Adjust image sharpness.
This class can be used to adjust the sharpness of an image. An enhancement factor of 0.0 gives a blurred image, a factor of 1.0 gives the original image, and a factor of 2.0 gives a sharpened image.
</pre>

图像增强的详细内容可以参考：[PIL/ImageEnhance](http://pillow-cn.readthedocs.org/en/latest/_modules/PIL/ImageEnhance.html#Color)


## 11. 图像的数组操作
图像用Image.open的形式加载进来后是一个PIL的图像对象，为了方便用Numpy进一步操作，需要用array()转化为数组。
{% highlight string %}
# -*- coding: utf-8 -*-


import PIL.Image
import numpy

image = PIL.Image.open("D:\\ImageNet\\timg2.jpg")


# 转化为数组
img_array = numpy.array(image)

# 获取图像参数
height,width = img_array.shape[0:2]

# 访问像素[行，列，channel]
value = img_array[0,0,0]

for i in range(height):
    for j in range(100):
        img_array[i,j,0] = 0xFF
        img_array[i,j,1] = 0x0
        img_array[i,j,2] = 0x0


pil_im2 = PIL.Image.fromarray(img_array)
pil_im2.show("rebuild from array")
{% endhighlight %}



## 12. OpenCV与Pillow示例

下面给出一个OpenCV与Pillow协同工作的例子：
{% highlight string %}
# -*- coding: utf-8 -*-

import cv2
import numpy
from PIL import Image
from PIL import ImageFilter
from PIL import ImageEnhance



image = Image.open("D:\\ImageNet\\card1.jpg")
outF = image.filter(ImageFilter.DETAIL)

enhanced = ImageEnhance.Color(outF)
enhanced_color = enhanced.enhance(1.3)

sharpness = ImageEnhance.Sharpness(enhanced_color)
sharpnessImg = sharpness.enhance(1.3)


img = numpy.array(sharpnessImg)



shape = (img.shape[0],img.shape[1])
newImage = numpy.ndarray(shape,img.dtype)


for i in range(img.shape[0]):
    for j in range(img.shape[1]):
        newImage[i,j] = 0.11 * img[i,j][0] + 0.59 * img[i,j][1] + 0.30 * img[i,j][2]


#ret2, th2 = cv2.threshold(newImage, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

cv2.namedWindow("NewImageWeightAver")
cv2.imshow("NewImageWeightAver",newImage)
cv2.waitKey(0)

cv2.destroyAllWindows()
{% endhighlight %}


<br />
<br />
**[参考]:**

1. [Python图像处理库Pillow入门](http://python.jobbole.com/84956/)

2. [Pillow和Numpy的图像基本操作](http://blog.csdn.net/jenny1000000/article/details/62897710) 

3. [Pillow Github](https://github.com/python-pillow/Pillow)

4. [Pillow官网](http://pillow.readthedocs.io/en/latest/)


<br />
<br />
<br />

