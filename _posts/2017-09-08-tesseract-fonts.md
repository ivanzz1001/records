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






<br />
<br />
<br />

