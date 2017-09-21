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
* .tr文件被替换成了.lstmf数据文件
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

训练数据都是通过使用如下的命令来创建的：
{% highlight string %}
training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata --output_dir ~/tesstutorial/engtrain
{% endhighlight %}

上面生成LSTM训练数据的命令与产生base Tesseract训练数据的命令是相同的。要想训练一个通用目的的基于LSTM的OCR引擎，这肯定是不够的，但还是可以作为一个很好的学习例子。

执行如下命令针对```Impact```字体产生一份参考数据：
{% highlight string %}
training/tesstrain.sh --fonts_dir /usr/share/fonts --lang eng --linedata_only \
  --noextract_font_properties --langdata_dir ../langdata \
  --tessdata_dir ./tessdata \
  --fontlist "Impact Condensed" --output_dir ~/tesstutorial/engeval
{% endhighlight %}

我们在下面讲述tune的时候就会用到该数据。


```注--fontlist后接多个字体：````
<pre>
# training/tesstrain.sh --fontlist "fontname 1" "fontname 2"
</pre>


## 10. Tutorial Guide to lstmtraining





<br />
<br />
<br />

