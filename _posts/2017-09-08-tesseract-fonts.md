---
layout: post
title: Tesseract训练字体
tags:
- ocr
categories: ocr
description: tesseract训练字体
---


我们可以用不同字体(fonts)文本来生成图片，然后再用这些生成的图片来进行tesseract训练。在进行训练的宿主机上必须有这些字体。

所需要的字体被定义在[training/language-specific.sh](https://github.com/tesseract-ocr/tesseract/blob/master/training/language-specific.sh)文件中。而更多的字体可以在[langdata/font_properties](https://github.com/tesseract-ocr/langdata/blob/master/font_properties)找到。假如你添加字体到第一个文件中（或是通过命令行参数的形式来显示指定），那么你也必须将它们添加到第二个文件。


<!-- more -->
参看:

1. [Fonts for Tesseract training](https://github.com/tesseract-ocr/tesseract/wiki/Fonts)



## 1. 查找字体

为了找到当前宿主系统上已经安装并且可以正确显示训练文本(training text)字体，我们可以使用如下的命令（请根据自己实际情况更改语言代码和目录位置）。```fontslist.txt```将会提供可用于training/language-specific.sh脚本的字体列表：
<pre>
# text2image --find_fonts \
--fonts_dir /usr/share/fonts \
--text ../langdata/hin/hin.training_text \
--min_coverage .9  \
--outputbase ../langdata/hin/hin \
|& grep raw | sed -e 's/ :.*/" \\/g'  | sed -e 's/^/  "/' >../langdata/hin/fontslist.txt
</pre>
```(注：上述命令不适用于Fraktur字体，因为它也会识别所有的Latin字体）```

如下我们生成当前系统上可用于中文训练的字体列表：
{% highlight string %}
# mkdir workspace
# cd workspace
# mkdir -p results/chi_sim


# gitclone https://github.com/tesseract-ocr/langdata.git  
(或者此处我们可以提前下载好我们需要的中文训练数据，放于langdata文件夹)

# /opt/tesseract4.0/bin/text2image --find_fonts \
--fonts_dir /usr/share/fonts \
--text ./langdata/chi_sim/chi_sim.training_text \
--min_coverage .9  \
--outputbase ./results/chi_sim/chi_sim\
|& grep raw | sed -e 's/ :.*/" \\/g'  | sed -e 's/^/  "/' >./results/chi_sim/fontslist.txt
{% endhighlight %}

执行完上述命令后，我们生成：
<pre>
[root@localhost workspace]# ls results/chi_sim/
chi_sim.AR_PL_UMing_CN_Semi-Light.tif      chi_sim.AR_PL_UMing_TW_Semi-Light.tif  chi_sim.WenQuanYi_Zen_Hei_Medium.tif        fontslist.txt
chi_sim.AR_PL_UMing_HK_Semi-Light.tif      chi_sim.WenQuanYi_Micro_Hei_Mono.tif   chi_sim.WenQuanYi_Zen_Hei_Mono_Medium.tif
chi_sim.AR_PL_UMing_TW_MBE_Semi-Light.tif  chi_sim.WenQuanYi_Micro_Hei.tif        chi_sim.WenQuanYi_Zen_Hei_Sharp_Medium.tif

[root@localhost workspace]# cat results/chi_sim/fontslist.txt 
  "AR PL UMing CN Semi-Light" \
  "AR PL UMing HK Semi-Light" \
  "AR PL UMing TW MBE Semi-Light" \
  "AR PL UMing TW Semi-Light" \
  "WenQuanYi Micro Hei" \
  "WenQuanYi Micro Hei Mono" \
  "WenQuanYi Zen Hei Medium" \
  "WenQuanYi Zen Hei Mono Medium" \
  "WenQuanYi Zen Hei Sharp Medium" \
</pre>


## 2. 字体的安装

在Centos上一般我们可以通过如下方式找到字体，然后安装：
<pre>
[root@localhost workspace]# yum search fonts | grep Chinese
wqy-microhei-fonts.noarch : Compact Chinese fonts derived from Droid
                                      : Simplified Chinese
                                        : Traditional Chinese
cjkuni-ukai-fonts.noarch : Chinese Unicode TrueType font in Kai face
cjkuni-uming-fonts.noarch : Chinese Unicode TrueType font in Ming face
ghostscript-chinese-zh_CN.noarch : Ghostscript Simplified Chinese fonts
ghostscript-chinese-zh_TW.noarch : Ghostscript Traditional Chinese fonts
google-noto-sans-simplified-chinese-fonts.noarch : Sans Simplified Chinese font
google-noto-sans-traditional-chinese-fonts.noarch : Sans Traditional Chinese
</pre>
这里我们安装当前机器上尚未安装的几个：
<pre>
# yum install cjkuni-ukai-fonts.noarch ghostscript-chinese-zh_CN.noarch google-noto-sans-simplified-chinese-fonts.noarch
</pre>


我们在WIN7上有大量的中文字体，但是很多时候我们不能通过上述方法进行安装，下面我们介绍如何安装WIN7上的一些字体。

(1) 安装字体管理工具
<pre>
# yum install fontconfig mkfontscale
</pre>

(2) 把WIN7上的一些字体拷贝到Linux上

Win7上的字体在C:\Windows\Fonts目录。 我们可以在Linux上创建相应的目录,例如：
<pre>
# mkdir -p /usr/share/fonts/chinese
</pre>
此处我们将C:\Windows/Fonts\simsun.ttc及C:\Windows\Fonts\simfang.ttf拷贝到上述目录

执行如下命令更改权限：
<pre>
# chmod -R 755 /usr/share/fonts/chinese
</pre>

(3) 建立字体缓存

执行如下命令建立字体缓存：
<pre>
# cd /usr/share/fonts/chinese
# mkfontscale
# mkfontdir
# fc-cache -fv
</pre>

(4) 查看字体
<pre>
[root@localhost chinese]# fc-list | grep SimSun
/usr/share/fonts/chinese/simsun.ttc: NSimSun,新宋体:style=Regular
/usr/share/fonts/chinese/simsun.ttc: SimSun,宋体:style=Regular
[root@localhost chinese]# 
[root@localhost chinese]# fc-list | grep FangSong
/usr/share/fonts/chinese/simfang.ttf: FangSong,仿宋:style=Regular,Normal,obyčejné,Standard,Κανονικά,
Normaali,Normál,Normale,Standaard,Normalny,Обычный,Normálne,Navadno,Arrunta
</pre>

(5) 使字体生效

系统操作系统，是字体生效。


## 3. 查看当前可用字体

执行如下命令查看当期系统可用字体：
<pre>
# /opt/tesseract4.0/bin/text2image --fonts_dir /usr/share/fonts --list_available_fonts
</pre>



<br />
<br />
<br />

