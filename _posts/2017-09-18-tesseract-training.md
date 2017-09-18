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

要训练Tesseract4.0.0，你不需要有任何神经网络的背景，但是了解神经网络有助于你理解一些训练选项的差异。在你深入研究训练流程之前，建议阅读[Implementation introduction](https://github.com/tesseract-ocr/tesseract/wiki/NeuralNetsInTesseract4.00)和[ImproveQuality](https://github.com/tesseract-ocr/tesseract/wiki/ImproveQuality)等相关内容。








<br />
<br />
<br />

