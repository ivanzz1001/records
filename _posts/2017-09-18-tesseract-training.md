---
layout: post
title: 训练Tesseract4.0
tags:
- ocr
categories: ocr
description: 训练Tesseract4.0
---

从官网下载已经训练好的中文训练数据，发现效果比较差。这里结合官方教程及自己的实践记录下如何训练Tesseract4.0.0。本文共分成10个部分来进行讲解。


<!-- more -->
<br />
<br />

参看: 

1. [Training Tesseract4.0.0](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract-4.00)



## 1. 介绍
Tesseract4.0.0包含了一个新的基于神经网络的识别引擎，该引擎在(document image文档图像）识别的精确性上相比以前的版本有了一个显著的提高。而这肯定也需要更加强大的计算机计算能力。

神经网络需要多得多的训练数据，并且训练也比原来的Tesseract会慢很多。针对基于Latin的语言，当前已经存在的模型数据被训练过:
<pre>
about 400000 textlines spanning about 4500 fonts.
</pre>
而对于其他脚本，并没有这么多的字体可用，但是它们也被训练了大体相同数量的textlines。与以往几分钟到几个小时的训练时长不同，*训练Tesseract4.0.0需要花费几天到几个礼拜*。即使当前官网已经提供了新的训练好的数据，但也许你会发现这并不能很好的适用于特定的一些情况，因此这里你可能想要自己重新的来训练它。

在训练时有很多不同的选项：

* 微调(Fine tune): 从已经训练好的语言开始，针对一些特定的额外数据进行训练。这种情况通常适用于你当前要解决的问题与已经训练好的数据类似，但是在一些微小的地方稍有不同，比如一个不常用的字体。这个时候可以只训练很小的一部分数据。
* 从神经网络中去掉顶层（或随机若干层），然后使用新的数据来重新训练顶层。在微调(Fine tune)效果不是很好的时候，这通常是下一个最好的选项。在训练一个全新的语言或脚本的时候，假如与原来语言或脚本很相似的话，则去掉顶层之后仍然还可以正常的工作。
* 重新训练。这是一项艰巨的任务，除非你针对特定的问题（环境）拥有一份很具有代表性的、高效的大规模训练集。假如没有的话，你通常并不能训练出一个实际上更好的数据。


虽然上面的一些选项看起来有些不同，但是这个训练步骤几乎相同。

从Tesseract4.0.0开始，老的识别引擎仍然存在，并且也可以被训练。但是它已经属于过时状态，除非后续后更好的素材，否则在将来的发布版本中很有可能会被去掉。

## 2. 预备工作

要训练Tesseract4.0.0，你不需要有任何神经网络的背景，但是了解神经网络有助于你理解一些训练选项的差异。在你深入研究训练流程之前，建议阅读[Implementation introduction](https://github.com/tesseract-ocr/tesseract/wiki/NeuralNetsInTesseract4.00) 和 [ImproveQuality](https://github.com/tesseract-ocr/tesseract/wiki/ImproveQuality)等相关内容。


## 3. 安装必要库

从Tesseract3.03开始，我们需要一些额外的库来构建训练工具：
{% highlight string %}
sudo apt-get install libicu-dev
sudo apt-get install libpango1.0-dev
sudo apt-get install libcairo2-dev
{% endhighlight %}
这一步，可能会由于库版本原因，我们需要手动安装。具体安装方法我们前面已经介绍过了。

## 4. 构建训练工具

从Tesseract3.03版本开始，假如你是通过源代码的方式来编译生成Tesseract的话，你需要单独执行命令来编译安装训练工具。一旦上面的这些库已经被安装，在Tesseract的源码目录执行如下命令：
{% highlight string %}
make
make training
sudo make training-install
{% endhighlight %}

可以执行如下命令来编译```ScrollView.java```:
{% highlight string %}
make ScrollView.jar
export SCROLLVIEW_PATH=$PWD/java
{% endhighlight %}
注： 我们上面没有特别执行安装命令```make install-jars```来进行安装，可以认为我们的安装目录就是```Tesseract源码目录/java```,并且我们通过export命令将该目录导出到```SCROLLVIEW_PATH```环境变量中。

## 5. 所需硬-软件环境

训练Tesseract4.0, 最好（但非必须）是在多核（4核）CPU上，并且拥有OpenMP及Intel Intrinsics以支持```SSE/AVX```扩展。基本上只要有足够的内存就可以运行，而CPU性能越高，则运行越快。并不需要（也不支持）GPU。内存的控制可以通过命令行选项```--max_image_MB```来指定，但是建议至少需要1GB的内存。


## 6. 训练文本需求
对于基于Latin的语言，当前已存在的模型已经训练了大概4500种字体，400000个textline。而对于其他的脚本，并没有这么多的字体可用，但是它们也被训练了大体相等数量的textline。


## 7. 训练流程概况
Tesseract4.0.0的整个训练流程与Tesseract3.04的训练流程，从概念上来说都是相同的：

* 准备训练文本
* 将训练文本变成 image + box文件（如果你已有image文件的话，只需要手动生成box文件）
* 生成unicharset文件（可以部分指定，比如手工创建）
* 用unicharset文件及可选的字典数据创建初始traineddata文件。
* 运行tesseract来处理image + box文件以生成一个数据集合
* 在数据集合上进行训练
* 合并数据文件

主要的不同有：

* box文件只需要是textline级别。这样从image文件生成训练数据会更容易
* .tr文件被替换成了 .lstmf 数据文件
* Fonts可以并且应该自由的混合在一起，而不是分开
* 组合步骤（mftraining,cntraining,shapeclustering)被替换成了一个单独的慢速lsmtraining步骤

Tesseract4.0的训练不能做到像Tesseract3.04那么自动化，主要原因在于：

* 慢速lsmtraining步骤并不能够很好的从中间开始运行，但是在停止之后它可以重新开始训练。并且在训练结束之后很难自动的进行报告
* 有很多选项来指定如何训练神经网络
* 新训练的语言模型、unicharset 与 base Tesseract的语言模型、unicharset可能不一样
* 针对神经网络Tesseract，并不需要一个相同语言的base Tesseract

```Creating the training data```流程我们下文会进行介绍，接着是```Tutorial guide to lstmtraining```会介绍主要的训练过程。我们都会通过命令行方式进行演示，至少在Linux平台，你只需要拷贝对应的命令行到terminal执行即可。为了是```tesstrain.sh```脚本正常工作，你需要将```training```及```api```目录设置到PATH环境变量中；或者执行make install。

## 8. Tesseract训练涉及文件

和base Tesseract类似，完整的LSTM模型和其需要的所有数据都被打包在```traineddata```文件中。不像是base Tesseract那样，Tesseract4.0在训练时必须提供一个初始```traineddata```文件，并且必须事先建立好。该文件包括：

* Config file提供控制参数
* ```Unicharset``` 定义字符集
* ```Unicharcompress```又称为recoder，用于将unicharset映射到具体的编码以供神经网络识别器使用。
* 标点符号模式集，用于模式化匹配在一个词前后允许出现的标点符号
* Word集. 系统语言模型的词表
* 数集。 用于模式化匹配Number

上面 红色字体 标明的元素是必须要提供的。 而其他一些元素是可选的，但是假如它们被提供了的话，标点符号模式集也必须要提供。有一个新的工具```combine_lang_data```用于构建初始```traineddata```,这需要以```unicharset```和可选的wordlist作为输入。

在训练过程中，训练器会写checkpoint文件，这是神经网络训练器的一个标配行为。这就允许根据需要中途停止、重启训练。任何的checkpoint都可以通过命令行标签```--stop_training```被转换成一个全功能的```traineddata```用于识别。

在训练过程中，当获得更好的训练结果的时候训练器也会周期性的写checkpoint文件。

你也可以修改神经网络，然后对其中的部分进行训练；我们也可以通过```--continue_from```选项指定一个已存在的checkpoint文件来对特定的训练数据进行微调。

假设我们通过```--continue_from```选项从一个checkpoint文件启动训练，并且通过```--traineddata```选项改变了unicharset的话，此时我们必须通过```--old_traineddata```选项来指定对应的traineddata。这允许训练器在训练过程中进行对应的字符映射。

## 9. Creating Training Data
与base Tesseract类似，我们可以通过字体自己构建出一些训练数据；也可以从已经存在的图像来构建训练数据。对于任何一种情况，我们都是需要有tiff/box文件，除非是box文件只需要覆盖textline而不是一个个单独的字符。

有两种方式来格式化一个box文件：

**Box File Format - First Option**

在这种格式下，在box文件中的每一行都匹配tiff 图像中的一个```character```。

在一系列line之后必须插入一个特殊的line以标明一个end-of-line

**Box File Format - Second Option(NOT YET IMPLEMENTED)**

未实现，暂不介绍。


<br />


如下的指令都是针对通过字体来构建训练数据，因此你首选必须要安装所需要的字体。

运行```tesstrain.sh```脚本的过程与base Tesseract类似。使用```--linedata_only```选项来进行LSTM训练。

值得注意的是，拥有更多的训练文本和页面有利于训练结果的准确性，因为神经网络并不能够泛化，需要训练类似于它们将运行的东西。假如目标域(target domain)被严格的限制，则所有需要更多训练数据的严重警告都可能不适用，但是网络规范可能需要修改。

```training data```都是通过使用如下的命令来创建的：
{% highlight string %}
training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata --output_dir ~/tesstutorial/engtrain
{% endhighlight %}

上面生成LSTM训练数据的命令与产生base Tesseract训练数据的命令是相同的。要想训练一个通用目的的基于LSTM的OCR引擎，这肯定是不够的，但还是可以作为一个很好的学习例子。

执行如下命令针对```Impact```字体产生一份```eval data```（当前宿主机上似乎没有```Impact```,我们可以```DejaVu Serif```代替)：
{% highlight string %}
training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata \
  --fontlist "Impact Condensed" --output_dir ~/tesstutorial/engeval
{% endhighlight %}

我们在下面讲述tune的时候就会用到该数据。


下面我们我们针对中文，生成tiff/box文件：
{% highlight string %}
# rm -rf /tmp/tmp.*                                  //clear the directory
# rm -rf results/chi_sim/*                
# rm -rf tesstutorial/chi_simtrain/*      
# rm -rf tesstutorial/chi_simeval/*       
# rm -rf tesstutorial/engtrain/*
# rm -rf tesstutorial/engeval/*

# mkdir -p results/chi_sim
# mkdir -p tesstutorial/chi_simtrain
# mkdir -p tesstutorial/chi_simeval
# mkdir -p tesstutorial/engtrain
# mkdir -p tesstutorial/engeval


# cd tesseract-master/
# training/text2image --find_fonts \
--fonts_dir /usr/share/fonts \
--text ../langdata/chi_sim/chi_sim.training_text \
--min_coverage .9  \
--outputbase ../results/chi_sim/chi_sim\
|& grep raw | sed -e 's/ :.*/" \\/g'  | sed -e 's/^/  "/' >../results/chi_sim/fontslist.txt

# cat ../results/chi_sim/fontslist.txt
  "AR PL UKai CN" \
  "AR PL UKai HK" \
  "AR PL UKai TW" \
  "AR PL UKai TW MBE" \
  "AR PL UMing CN Semi-Light" \
  "AR PL UMing HK Semi-Light" \
  "AR PL UMing TW MBE Semi-Light" \
  "AR PL UMing TW Semi-Light" \
  "Arial Unicode MS" \
  "FangSong" \
  "KaiTi" \
  "LiSu" \
  "Microsoft YaHei" \
  "Microsoft YaHei Bold" \
  "NSimSun" \
  "Noto Sans SC" \
  "Noto Sans SC Bold" \
  "Noto Sans SC Heavy" \
  "Noto Sans SC Medium" \
  "Noto Sans SC Medium" \
  "Noto Sans SC Semi-Light" \
  "Noto Sans SC Semi-Light" \
  "STFangsong" \
  "STKaiti" \
  "STSong" \
  "STXihei" \
  "STXinwei" \
  "STZhongsong" \
  "SimHei" \
  "SimSun" \
  "WenQuanYi Micro Hei" \
  "WenQuanYi Micro Hei Mono" \
  "WenQuanYi Zen Hei Medium" \
  "WenQuanYi Zen Hei Mono Medium" \
  "WenQuanYi Zen Hei Sharp Medium" \
  "YouYuan" \


//我们看到上面有些行重复，执行如下命令去除重复
# gawk '!a[$0]++' ../results/chi_sim/fontslist.txt 
  "AR PL UKai CN" \
  "AR PL UKai HK" \
  "AR PL UKai TW" \
  "AR PL UKai TW MBE" \
  "AR PL UMing CN Semi-Light" \
  "AR PL UMing HK Semi-Light" \
  "AR PL UMing TW MBE Semi-Light" \
  "AR PL UMing TW Semi-Light" \
  "Arial Unicode MS" \
  "FangSong" \
  "KaiTi" \
  "LiSu" \
  "Microsoft YaHei" \
  "Microsoft YaHei Bold" \
  "NSimSun" \
  "Noto Sans SC" \
  "Noto Sans SC Bold" \
  "Noto Sans SC Heavy" \
  "Noto Sans SC Medium" \
  "Noto Sans SC Semi-Light" \
  "STFangsong" \
  "STKaiti" \
  "STSong" \
  "STXihei" \
  "STXinwei" \
  "STZhongsong" \
  "SimHei" \
  "SimSun" \
  "WenQuanYi Micro Hei" \
  "WenQuanYi Micro Hei Mono" \
  "WenQuanYi Zen Hei Medium" \
  "WenQuanYi Zen Hei Mono Medium" \
  "WenQuanYi Zen Hei Sharp Medium" \
  "YouYuan" \

//产生training data
# training/tesstrain.sh --fonts_dir /usr/share/fonts --lang chi_sim --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata \
  --exposures "0" \
  --output_dir ../tesstutorial/chi_simtrain \
  --overwrite


//产生eval data
# training/tesstrain.sh --fonts_dir /usr/share/fonts --lang chi_sim --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata \
  --exposures "0" \
  --fontlist "AR PL UKai CN" \
  "AR PL UKai HK" \
  "AR PL UKai TW" \
  "AR PL UKai TW MBE" \
  "AR PL UMing CN Semi-Light" \
  "AR PL UMing HK Semi-Light" \
  "AR PL UMing TW MBE Semi-Light" \
  "AR PL UMing TW Semi-Light" \
  "Arial Unicode MS" \
  "FangSong" \
  "KaiTi" \
  "LiSu" \
  "Microsoft YaHei" \
  "Microsoft YaHei Bold" \
  "NSimSun" \
  "Noto Sans SC" \
  "Noto Sans SC Bold" \
  "Noto Sans SC Heavy" \
  "Noto Sans SC Medium" \
  "Noto Sans SC Semi-Light" \
  "STFangsong" \
  "STKaiti" \
  "STSong" \
  "STXihei" \
  "STXinwei" \
  "STZhongsong" \
  "SimHei" \
  "SimSun" \
  "WenQuanYi Micro Hei" \
  "WenQuanYi Micro Hei Mono" \
  "WenQuanYi Zen Hei Medium" \
  "WenQuanYi Zen Hei Mono Medium" \
  "WenQuanYi Zen Hei Sharp Medium" \
  "YouYuan" \
  --output_dir ../tesstutorial/chi_simeval \
  --overwrite


//删除数据
# rm -rf ../tesstutorial/chi_simtrain/*
# rm -rf ../tesstutorial/chi_simeval/*
# rm -rf /tmp/tmp.*
{% endhighlight %}

执行后生成如下```.lstmf```和```unicharset```文件：
<pre>
[root@localhost tesseract]# ls ../tesstutorial/chieval/chi_sim
chi_sim/                                           chi_sim.LiSu.exp0.lstmf                            chi_sim.STKaiti.exp0.lstmf
chi_sim.Arial_Unicode_MS.exp0.lstmf                chi_sim.Microsoft_YaHei_Bold.exp0.lstmf            chi_sim.STSong.exp0.lstmf
chi_sim.AR_PL_UKai_CN.exp0.lstmf                   chi_sim.Microsoft_YaHei.exp0.lstmf                 chi_sim.STXihei.exp0.lstmf
chi_sim.AR_PL_UKai_HK.exp0.lstmf                   chi_sim.Noto_Sans_SC_Bold.exp0.lstmf               chi_sim.STXinwei.exp0.lstmf
chi_sim.AR_PL_UKai_TW.exp0.lstmf                   chi_sim.Noto_Sans_SC.exp0.lstmf                    chi_sim.STZhongsong.exp0.lstmf
chi_sim.AR_PL_UKai_TW_MBE.exp0.lstmf               chi_sim.Noto_Sans_SC_Heavy.exp0.lstmf              chi_sim.training_files.txt
chi_sim.AR_PL_UMing_CN_Semi-Light.exp0.lstmf       chi_sim.Noto_Sans_SC_Medium.exp0.lstmf             chi_sim.WenQuanYi_Micro_Hei.exp0.lstmf
chi_sim.AR_PL_UMing_HK_Semi-Light.exp0.lstmf       chi_sim.Noto_Sans_SC_Semi-Light.exp0.lstmf         chi_sim.WenQuanYi_Micro_Hei_Mono.exp0.lstmf
chi_sim.AR_PL_UMing_TW_MBE_Semi-Light.exp0.lstmf   chi_sim.NSimSun.exp0.lstmf                         chi_sim.WenQuanYi_Zen_Hei_Medium.exp0.lstmf
chi_sim.AR_PL_UMing_TW_Semi-Light.exp0.lstmf       chi_sim.SimHei.exp0.lstmf                          chi_sim.WenQuanYi_Zen_Hei_Mono_Medium.exp0.lstmf
chi_sim.FangSong.exp0.lstmf                        chi_sim.SimSun.exp0.lstmf                          chi_sim.WenQuanYi_Zen_Hei_Sharp_Medium.exp0.lstmf
chi_sim.KaiTi.exp0.lstmf                           chi_sim.STFangsong.exp0.lstmf                      chi_sim.YouYuan.exp0.lstmf
</pre>




## 10. Tutorial Guide to lstmtraining

### 10.1 创建初始Traineddata
*(注意： 这是一个新步骤)*

作为```unicharset``` 和 ```script_dir```的替代，当前```lstmtraining```需要在命令行传入一个```traineddata```文件，以获得lstm针对某种语言训练的所有相关信息。```traineddata```所需要的文件如下：
<pre>
1. lstm-unicharset (必须）
2. lstm-recoder （必须）

(3个dawg文件,可选)
3. lstm-punc-dawg
4. lstm-word-dawg
5. lstm-number-dawg

(1个配置文件,可选)
6. config file
</pre>
除了上述这些，并不需要其他组件，假如存在的话也会被忽略，并不会被使用。

并没有工具可以直接的来创建```lstm-recoder```。然而tesseract4.0有一个新的工具```combine_lang_model```,它接受如下文件作为输入：

* input_unicharset
* script_dir(script_dir指向langdata目录）
* word list文件(可选）

以```input_unicharset```作为输入，通过```combine_lang_model```这个工具可以创建出lstm-recoder和所有的dawgs。假若也提供了wordlist的话，都会将这些打包进traineddata文件中。

下面给出一个示例：
{% highlight string %}
 combine_lang_model    \
 --input_unicharset  ../tesstutorial/sanskrit2003/san/san.unicharset  \
 --script_dir "../langdata"   \
 --words "../langdata/san/san.wordlist" \
 --numbers "../langdata/san/san.numbers"   \
 --puncs "../langdata/san/san.punc" \
 --output_dir ../tesstutorial/sanskrit2003   \
 --lang "san"     --pass_through_recoder \
 --version_str "4.0.0alpha-20170816 sanskrit2003"
{% endhighlight %}

下面针对中文我们生成lstm-recoder:
{% highlight string %}
# training/combine_lang_model --input_unicharset ../tesstutorial/chieval/chi_sim/chi_sim.unicharset \
 --script_dir ../langdata \
 --words ../langdata/chi_sim/chi_sim.wordlist \
 --numbers ../langdata/chi_sim/chi_sim.numbers \
 --puncs ../langdata/chi_sim/chi_sim.punc \
 --output_dir ../tesstutorial/chieval \
 --lang chi_sim \
 --version_str "4.0.0alpha chi_sim"
{% endhighlight %}

### 10.2 LSTMTraining命令行

`lstmtraining`是一个用于训练神经网络的多功能工具。下表描述了它的命令行选项：

|        Flag          |       type         |     Default   |                  Explanation                |
|:---------------------|:-------------------|:--------------|:--------------------------------------------|
|      traineddata     |      string        |      none     |初始traineddata文件路径，该路径下包含unicharset、recorder与可选的语言模型|
|      net_spec        |      string        |      none     |指定神经网络的拓扑结构                          |
|      model_output    |      string        |      none     |产生的model/checkpoints文件的存放目录           |
|      max_image_MB    |      int           |      6000     |缓存图片所使用的最大内存                        |
| sequential_training  |      bool          |      false    |按sequential training设置为true，默认是采用round-robin来处理训练数据|
|      net_mode        |      int           |      192      |来自于network.h头文件中NetworkFlags枚举所定义的Flag。可选值有：128 for Adam optimization instead of momentum; 64 to allow different layers to have their own learning rates, discovered automatically.|
|perfect_sample_delay  |     int            |       0       |When the network gets good, only backprop a perfect sample after this many imperfect samples have been seen since the last perfect sample was allowed through.|
|   debug_interval     |     int            |       0       |假若设置为非0值，则每隔指定间隔显示可视化调试信息   |
|   weight_range       |     double         |       0.1     |用于初始化权重的随机值区间                       |
|   momentum           |     double         |       0.5     |Momentum for alpha smoothing gradients.       |
|   adam_beta          |     double         |      0.999    |Smoothing factor squared gradients in ADAM algorithm.|
|   max_iterations     |     int            |        0      |在达到max_iterations的训练次数后停止停止训练      |
|   target_error_rate  |     double         |      0.01     |假如平均错误率高于本值时停止训练                  |
|   continue_from      |     string         |      none     |前一个checkpoint的路径，可以通过该checkpoint来继续进行训练或者fine tune|
|   stop_training      |     bool           |      false    |将--continue_from指定的checkpoint转换成一个识别模型|
|   convert_to_int     |     bool           |      false    |With stop_training, convert to 8-bit integer for greater speed, with slightly less accuracy.|
|   append_index       |     int            |      -1       |Cut the head off the network at the given index and append --net_spec network in place of the cut off part.|
|   train_listfile     |     string         |      none     |Filename of a file listing training data files.|
|   eval_listfile      |     string         |      none     |Filename of a file listing evaluation data files to be used in evaluating the model independently of the training data.|

上述的大部分flag都可以采用默认值，其中一些flag只需要在下面例句的特定操作下才需要。这里我们首先对一些相对复杂的flags进行一个详细的解释。


参看: [梯度下降优化算法概述](http://blog.csdn.net/u014421266/article/details/50637415)   


**1) Unicharset Compression-recoding**

LSTMs在顺序性学习的时候很高效，但是在状态数太多的时候就会严重降低速度。根据经验来看，让LSTM学习一个长sequence比学习一个短的sequence更有优势，因此对于一些复杂的脚本（Han,Hangul,Indic脚本），更好的做法是将其中的每一个符号以少量的classes重新编码为一个短sequence，而不是采用大量的classes。 ```combine_lang_model```命令默认采用了此特性。它会将每一个```Han```字符编码为1~5的变长码，```Hangul```使用Jamo编码变成长度为3的编码序列，其的脚本则采用它们的Unicode组件序列。为了充分使用本特性，我们应该为combine_lang_model添加```--pass_through_recoder```这一flag。

<br />

**2) Randomized Training Data and sequential_training**

<br />

**3) Model output**

训练器周期性的将checkpoints写入到```--model_output```所指定的目录。因此可以在任何时刻停止训练，然后我们可以根据这些checkpoints从停止处重启训练。要强制开启一个全新的训练的话，可以使用```--model_output```设置一个新的目录，或者将原来目录中的所有文件删除。

<br />


**4) Net Mode and Optimization**

<br />

**5) Perfect Sample Delay**

在训练时，并不需要在一些很"简单"的样本上浪费大量的时间，但是神经网络需要能够处理它们，因此可以允许在训练时丢弃一些太高频的简单样本。```--perfect_sample_delay```参数的作用是：如果从上一个perfect sample之后，后面一直都是perfect sample的话，则会丢弃其中的一些perfect sample，直到遇到一些imperfect sample。当前的默认值0表示采用所有样本。在实际使用过程中，本选项貌似效果不明显。假如允许长时间的训练的话，设置为0可以得到最好的效果。

<br />

**6) Debug Interval and Visual Debugging**

```--debug_interval```选项默认值为0，trainer每100次循环输出一个progress报告。

```--debug_interval```设置为-1，表示trainer每一次循环都输出一个详细的文本调试信息

对于```--debug_interval > 0```，trainer会在神经网络层上显示多屏的调试信息。对于```--debug_interval 1```这一特例，在进行下一次循环之前会等待LSTMForward窗口上的一次点击。而对于其他值则会按指定的频率打印信息。


```注意：```设置--debug_interval > 0的话，必须要编译ScrollView.jar和其他的一些训练工具。请参看[Building the Training Tools](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract-4.00#building-the-training-tools)


调试的文本信息包括：truth text, the recognized text, the iteration number, the training sample id (file and page) and the mean value of several error metrics.


可视化调试信息包括：

每一个神经网络层的前、后向窗口。对于大部分信息可能都是无意义的垃圾数据，但是```Output/Output-back```和```ConvNL```窗口还是值得查看。




## 11. Training From Scratch

如下的例子展示了"training from scratch"命令行的使用方法。用上述命令行尝试的默认训练数据来执行下面的命令：
{% highlight string %}
mkdir -p ~/tesstutorial/engoutput
training/lstmtraining --debug_interval 100 \
  --traineddata ~/tesstutorial/engtrain/eng/eng.traineddata \
  --net_spec '[1,36,0,1 Ct3,3,16 Mp3,3 Lfys48 Lfx96 Lrx96 Lfx256 O1c111]' \
  --model_output ~/tesstutorial/engoutput/base --learning_rate 20e-4 \
  --train_listfile ~/tesstutorial/engtrain/eng.training_files.txt \
  --eval_listfile ~/tesstutorial/engeval/eng.training_files.txt \
  --max_iterations 5000 &>~/tesstutorial/engoutput/basetrain.log
{% endhighlight %}

这里我们针对中文：
{% highlight string %}
# mkdir -p ../tesstutorial/chi_simoutput
# training/lstmtraining --debug_interval 100 \
  --traineddata ../tesstutorial/chi_simtrain/chi_sim/chi_sim.traineddata \
  --net_spec '[1,36,0,1 Ct3,3,16 Mp3,3 Lfys48 Lfx96 Lrx96 Lfx256 O1c111]' \
  --model_output ../tesstutorial/chi_simoutput/base --learning_rate 20e-4 \
  --train_listfile ../tesstutorial/chi_simtrain/chi_sim.training_files.txt \
  --eval_listfile ../tesstutorial/chi_simeval/chi_sim.training_files.txt \
  --max_iterations 5000 &>../tesstutorial/chi_simoutput/basetrain.log
{% endhighlight %}


在另一个单独的窗口，我们可以使用如下命令来观察日志文件：
{% highlight string %}
tail -f ../tesstutorial/chi_simoutput/basetrain.log
{% endhighlight %}

(假如你以前看过本学习手册，你也许会注意到其中的一些数字已经发生了改变。这是由于产生的神经网络略小的缘故，并且由于增加了ADAM优化器，使得有一个更高的学习效率）。


在训练到600次的时候，空白会开始显示在```CTC Output```窗口，并且在1300次的时候图像中会出现空格，然后在```LSTMForward窗口```开始出现绿色的行。


值得注意的是，我们上面训练的engine所训练的数据量与原来遗留的Tesseract engine所训练的数据量是一样的，但是对于其他字体的精确性也许会更差。我们可以通过如下的命令运行针对```Impact Condensed```字体进行一个独立的测试：
{% highlight string %}
# training/lstmeval --model ~/tesstutorial/engoutput/base_checkpoint \
  --traineddata ../tesstutorial/engtrain/eng/eng.traineddata \
  --eval_listfile ../tesstutorial/engeval/eng.training_files.txt
{% endhighlight %}

结果很可能为85%的字符错误率。




## 12. Fine Tuning for Impact

Fine Tuning是用已经存在的模型针对新的数据进行训练的过程，这一过程并不会改变神经网络的任何部分，尽管你可以添加字符到字符集中（请参看：[Fine Tuning for ± a few characters](https://github.com/tesseract-ocr/tesseract/wiki/TrainingTesseract-4.00#fine-tuning-for-%C2%B1-a-few-characters)）

{% highlight string %}
# training/lstmtraining --model_output /path/to/output [--max_image_MB 6000] \
  --continue_from /path/to/existing/model \
  --traineddata /path/to/original/traineddata \
  [--perfect_sample_delay 0] [--debug_interval 0] \
  [--max_iterations 0] [--target_error_rate 0.01] \
  --train_listfile /path/to/list/of/filenames.txt
{% endhighlight %}

注意：```--continue_from```可以指定为一个checkpoint，或者是一个recognition model，即使它们的文件格式是不同的。checkpoint文件在```--model_output```目录下以```checkpoint```结束的文件。而一个```recognition model```则可以从一个已存在的traineddata文件中使用combine_tessdata解压得到。值得注意的是也需要提供原始的训练文件，因为它包含有unicharset和recoder. 下面是针对我们前面训练好的模型，对于```Impact```字体的一个Fine Tuning训练：
{% highlight string %}
mkdir -p ~/tesstutorial/impact_from_small
training/lstmtraining --model_output ~/tesstutorial/impact_from_small/impact \
  --continue_from ~/tesstutorial/engoutput/base_checkpoint \
  --traineddata ~/tesstutorial/engtrain/eng/eng.traineddata \
  --train_listfile ~/tesstutorial/engeval/eng.training_files.txt \
  --max_iterations 1200
{% endhighlight %}

训练到100遍的时候，character/word的错误率为22.36%/50%，而当训练到1200遍的时候错误率则下降到0.3%/1.2%。现在我们可以进行一个测试：
{% highlight string %}
# training/lstmeval --model ~/tesstutorial/impact_from_small/impact_checkpoint \
  --traineddata ~/tesstutorial/engtrain/eng/eng.traineddata \
  --eval_listfile ~/tesstutorial/engeval/eng.training_files.txt
{% endhighlight %}

这似乎显示了一个更好的结果，character/word的错误率为0.0086%/0.057%，这是因为训练平均已经超过了1000遍，并且训练器已经进行了提高。但是这并不代表整个```Impace```字体，因为只是测试了训练数据。

<br />

如下有一个小的例子，Fine Tuning的目的就是使用在一个已存在的fully-trained模型上面：
{% highlight string %}
# mkdir -p ~/tesstutorial/impact_from_full

# training/combine_tessdata -e tessdata/best/eng.traineddata \
  ~/tesstutorial/impact_from_full/eng.lstm

# training/lstmtraining --model_output ~/tesstutorial/impact_from_full/impact \
  --continue_from ~/tesstutorial/impact_from_full/eng.lstm \
  --traineddata tessdata/best/eng.traineddata \
  --train_listfile ~/tesstutorial/engeval/eng.training_files.txt \
  --max_iterations 400
{% endhighlight %}
训练到100遍的时候，它有一个1.35%/4.56% char/word的错误率，而当训练到400遍的时候会下降到0.533%/1.633%的错误率。我们通过如下的命令进行测试：
{% highlight string %}
# training/lstmeval --model ~/tesstutorial/impact_from_full/impact_checkpoint \
  --traineddata tessdata/best/eng.traineddata \
  --eval_listfile ~/tesstutorial/engeval/eng.training_files.txt
{% endhighlight %}
再一次我们获得了一个较好的结果：char的错误率为0.017%，word的错误率为0.120%。而更有意思的是经过上面的训练之后，其对其他字体的影响。下面我们在```base training```上来测试一个例子：
{% highlight string %}
# training/lstmeval --model ~/tesstutorial/impact_from_full/impact_checkpoint \
  --traineddata tessdata/best/eng.traineddata \
  --eval_listfile ~/tesstutorial/engtrain/eng.training_files.txt
{% endhighlight %}

这时我们发现char的错误率为0.25548592，word的错误率为0.82523491.

这看起来效果更差了，尽管针对eval训练集可以达到一个接近为0的错误率并且只训练了400遍。```注意:```如果训练遍数超过400次的话，则针对base training会有一个更高的错误率。

总结：预先训练好的模型可以针对一个小的数据集被重新fine-tuned或者adapted，而对原来的通用的精确性不会造成太大的损坏。Fine-tuning是很重要的，但是应该避免针对某一小的数据集进行过度训练，否则可能会降低对整体数据集的识别率。


## 13. Fine Tuning for ± a few characters

```New Feature.```我们可以添加一些新的字符到字符集中，然后使用fine tuning来训练它们，而不需要对一个大规模的数据进行训练。

训练需要有新的unicharset/recoder，language models(可选），和原来的含有unicharset/recoder的traineddata文件。
{% highlight string %}
# training/lstmtraining --model_output /path/to/output [--max_image_MB 6000] \
  --continue_from /path/to/existing/model \
  --traineddata /path/to/traineddata/with/new/unicharset \
  --old_traineddata /path/to/existing/traineddata \
  [--perfect_sample_delay 0] [--debug_interval 0] \
  [--max_iterations 0] [--target_error_rate 0.01] \
  --train_listfile /path/to/list/of/filenames.txt
{% endhighlight %}

下面我们尝试添加```plus-minus符号(±)```到已经存在的英文模型当中。修改```langdata/eng/eng.training_text```文件让其包含一些```±```符号。我这里插入了14个，如下所示：
<pre>
# grep ± ../langdata/eng/eng.training_text
alkoxy of LEAVES ±1.84% by Buying curved RESISTANCE MARKED Your (Vol. SPANIEL
TRAVELED ±85¢ , reliable Events THOUSANDS TRADITIONS. ANTI-US Bedroom Leadership
Inc. with DESIGNS self; ball changed. MANHATTAN Harvey's ±1.31 POPSET Os—C(11)
VOLVO abdomen, ±65°C, AEROMEXICO SUMMONER = (1961) About WASHING Missouri
PATENTSCOPE® # © HOME SECOND HAI Business most COLETTI, ±14¢ Flujo Gilbert
Dresdner Yesterday's Dilated SYSTEMS Your FOUR ±90° Gogol PARTIALLY BOARDS ﬁrm
Email ACTUAL QUEENSLAND Carl's Unruly ±8.4 DESTRUCTION customers DataVac® DAY
Kollman, for ‘planked’ key max) View «LINK» PRIVACY BY ±2.96% Ask! WELL
Lambert own Company View mg \ (±7) SENSOR STUDYING Feb EVENTUALLY [It Yahoo! Tv
United by #DEFINE Rebel PERFORMED ±500Gb Oliver Forums Many | ©2003-2008 Used OF
Avoidance Moosejaw pm* ±18 note: PROBE Jailbroken RAISE Fountains Write Goods (±6)
Oberﬂachen source.” CULTURED CUTTING Home 06-13-2008, § ±44.01189673355 €
netting Bookmark of WE MORE) STRENGTH IDENTICAL ±2? activity PROPERTY MAINTAINED
</pre>

```注：±可以通过alt+0177输入```

现在使用下面的命令产生新的```training```和```eval```数据：
{% highlight string %}
# training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata --output_dir ~/tesstutorial/trainplusminus

# training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata \
  --fontlist "Impact Condensed" --output_dir ~/tesstutorial/evalplusminus
{% endhighlight %}

在新的训练数据上运行fine tuning。这需要更多的训练遍数，因为针对新的目标字符其拥有更少的样本数：
{% highlight string %}
# training/combine_tessdata -e tessdata/best/eng.traineddata \
  ~/tesstutorial/trainplusminus/eng.lstm

# training/lstmtraining --model_output ~/tesstutorial/trainplusminus/plusminus \
  --continue_from ~/tesstutorial/trainplusminus/eng.lstm \
  --traineddata ~/tesstutorial/trainplusminus/eng/eng.traineddata \
  --old_traineddata tessdata/best/eng.traineddata \
  --train_listfile ~/tesstutorial/trainplusminus/eng.training_files.txt \
  --max_iterations 3600
{% endhighlight %}





<br />
<br />
<br />

