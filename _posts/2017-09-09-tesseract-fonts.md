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
<pre>
[root@localhost tesseract-master]# ls /usr/share/fonts/chinese/
arial.ttf               ARIALUNI.TTF  msyhbd.ttf  simfang.ttf  simkai.ttf  simsun.ttc  STFANGSO.TTF  STSONG.TTF   STXINWEI.TTF
Arial-Unicode-Bold.ttf  ariblk.ttf    msyh.ttf    simhei.ttf   SIMLI.TTF   SIMYOU.TTF  STKAITI.TTF   STXIHEI.TTF  STZHONGS.TTF
</pre>

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
[root@localhost tesseract-master]# /opt/tesseract4.0/bin/text2image --fonts_dir /usr/share/fonts --list_available_fonts
  0: AR PL UKai CN
  1: AR PL UKai HK
  2: AR PL UKai TW
  3: AR PL UKai TW MBE
  4: AR PL UMing CN Semi-Light
  5: AR PL UMing HK Semi-Light
  6: AR PL UMing TW MBE Semi-Light
  7: AR PL UMing TW Semi-Light
  8: Abyssinica SIL
  9: Arial Unicode MS
 10: Caladea
 11: Caladea Bold
 12: Caladea Bold Italic
 13: Caladea Italic
 14: Cantarell
 15: Cantarell Bold
 16: Carlito
 17: Carlito Bold
 18: Carlito Bold Italic
 19: Carlito Italic
 20: Century Schoolbook L Bold
 21: Century Schoolbook L Bold Italic
 22: Century Schoolbook L Italic
 23: Century Schoolbook L Medium
 24: Clean
 25: DejaVu Sans
 26: DejaVu Sans Bold
 27: DejaVu Sans Bold Oblique
 28: DejaVu Sans Bold Oblique Semi-Condensed
 29: DejaVu Sans Bold Semi-Condensed
 30: DejaVu Sans Mono
 31: DejaVu Sans Mono Bold
 32: DejaVu Sans Mono Bold Oblique
 33: DejaVu Sans Mono Oblique
 34: DejaVu Sans Oblique
 35: DejaVu Sans Oblique Semi-Condensed
 36: DejaVu Sans Semi-Condensed
 37: DejaVu Sans Ultra-Light
 38: DejaVu Serif
 39: DejaVu Serif Bold
 40: DejaVu Serif Bold Italic
 41: DejaVu Serif Bold Italic Semi-Condensed
 42: DejaVu Serif Bold Semi-Condensed
 43: DejaVu Serif Italic
 44: DejaVu Serif Italic Semi-Condensed
 45: DejaVu Serif Semi-Condensed
 46: Dingbats
 47: FangSong
 48: Fixed
 49: FreeMono
 50: FreeMono Bold
 51: FreeMono Bold Italic
 52: FreeMono Italic
 53: FreeSans
 54: FreeSans Italic
 55: FreeSans Semi-Bold
 56: FreeSans Semi-Bold Italic
 57: FreeSerif
 58: FreeSerif Bold
 59: FreeSerif Bold Italic
 60: FreeSerif Italic
 61: Jomolhari
 62: KaiTi
 63: Khmer OS
 64: Khmer OS Content
 65: Khmer OS System
 66: LKLUG
 67: LiSu
 68: Liberation Mono
 69: Liberation Mono Bold
 70: Liberation Mono Bold Italic
 71: Liberation Mono Italic
 72: Liberation Sans
 73: Liberation Sans Bold
 74: Liberation Sans Bold Italic
 75: Liberation Sans Italic
 76: Liberation Serif
 77: Liberation Serif Bold
 78: Liberation Serif Bold Italic
 79: Liberation Serif Italic
 80: Lohit Assamese
 81: Lohit Bengali
 82: Lohit Devanagari
 83: Lohit Gujarati
 84: Lohit Kannada
 85: Lohit Malayalam
 86: Lohit Marathi
 87: Lohit Nepali
 88: Lohit Oriya
 89: Lohit Punjabi
 90: Lohit Tamil
 91: Lohit Telugu
 92: Madan2
 93: Meera
 94: Microsoft YaHei
 95: Microsoft YaHei Bold
 96: NSimSun
 97: NanumGothic
 98: NanumGothic Bold
 99: NanumGothic Semi-Bold
100: Nimbus Mono L
101: Nimbus Mono L Bold
102: Nimbus Mono L Bold Oblique
103: Nimbus Mono L Oblique
104: Nimbus Roman No9 L
105: Nimbus Roman No9 L Bold
106: Nimbus Roman No9 L Bold Italic
107: Nimbus Roman No9 L Italic
108: Nimbus Sans L
109: Nimbus Sans L Bold
110: Nimbus Sans L Bold Condensed
111: Nimbus Sans L Bold Italic
112: Nimbus Sans L Bold Italic Condensed
113: Nimbus Sans L Condensed
114: Nimbus Sans L Italic
115: Nimbus Sans L Italic Condensed
116: Noto Sans SC
117: Noto Sans SC Bold
118: Noto Sans SC Heavy
119: Noto Sans SC Medium
120: Noto Sans SC Medium
121: Noto Sans SC Semi-Light
122: Noto Sans SC Semi-Light
123: Nuosu SIL
124: Open Sans
125: Open Sans Bold
126: Open Sans Bold Italic
127: Open Sans Italic
128: Open Sans Semi-Bold
129: Open Sans Semi-Bold Italic
130: Open Sans Semi-Light
131: Open Sans Semi-Light Italic
132: Open Sans Ultra-Bold
133: Open Sans Ultra-Bold Italic
134: OpenSymbol
135: Overpass
136: Overpass Bold
137: Overpass Bold Italic
138: Overpass Italic
139: Overpass Semi-Light
140: Overpass Semi-Light Italic
141: Overpass Ultra-Light
142: Overpass Ultra-Light Italic
143: PT Sans
144: PT Sans Bold
145: PT Sans Bold Italic
146: PT Sans Italic
147: PT Sans Narrow Bold Condensed
148: PT Sans Narrow Condensed
149: Padauk
150: Padauk Bold
151: PakType Naskh Basic
152: STFangsong
153: STIX
154: STIX Bold
155: STIX Bold Italic
156: STIX Italic
157: STKaiti
158: STSong
159: STXihei
160: STXinwei
161: STZhongsong
162: SimHei
163: SimSun
164: Standard Symbols L
165: URW Bookman L
166: URW Bookman L Bold
167: URW Bookman L Bold Italic
168: URW Bookman L Italic
169: URW Chancery L Medium Italic
170: URW Gothic L Book
171: URW Gothic L Book Oblique
172: URW Gothic L Semi-Bold
173: URW Gothic L Semi-Bold Oblique
174: URW Palladio L Bold
175: URW Palladio L Bold Italic
176: URW Palladio L Italic
177: URW Palladio L Medium
178: Utopia
179: Utopia Bold
180: Utopia Bold Italic
181: Utopia Italic
182: VL Gothic
183: Waree
184: Waree Bold
185: Waree Bold Oblique
186: Waree Oblique
187: WenQuanYi Micro Hei
188: WenQuanYi Micro Hei Mono
189: WenQuanYi Zen Hei Medium
190: WenQuanYi Zen Hei Mono Medium
191: WenQuanYi Zen Hei Sharp Medium
192: YouYuan
</pre>



<br />
<br />
<br />

