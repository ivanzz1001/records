---
layout: post
title: variable-precision SWAR算法
tags:
- data-structure
categories: data-structure
description:variable-precision SWAR算法
---


本章主要介绍一下variable-precision SWAR算法的实现原理，之后再对redis中相关```位计数```操作的实现做简单说明。


<!-- more -->

## 1. variable-precision SWAR算法

```variable-precision``` SWAR算法通常用于统计一个数组中非0位的数量，数学上被称为```计算汉明重量```（Hamming Weight)。

因为汉明重量经常被用于信息论、编码理论和密码学，所以研究人员针对计算汉明重量开发了多种不同的算法，一些处理器甚至直接带有计算汉明重量的指令，而对于不具备这种特殊指令的普通处理器来说，目前已知效率最好的通用算法为variable-precision SWAR算法，该算法通过一系列位移和位运算操作，可以在常数时间内计算多个字节的汉明重量，并且不需要使用额外的内存。


### 1.1 算法实现
下面给出的是一个处理```32位```长度位数的variable-precision SWAR算法实现：
{% highlight string %}
//计算32位二进制的汉明重量
uint32_t swar(uint32_t i)
{
	i = (i & 0x55555555) + ((i >> 1) & 0x55555555);			//步骤1
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);			//步骤2
	i = (i & 0x0F0F0F0F) + ((i >> 4) & 0x0F0F0F0F);			//步骤3
	i = (i * (0x01010101) >> 24);							//步骤4

	return i;
}
{% endhighlight %}


### 1.2 算法分析
我们对上面算法的实现进行分析。首先看上面几个特殊的16进制数：

![ds-bit](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_variable_precision_swar.jpg)

下面我们分析每一个步骤：

1） **步骤1**
{% highlight string %}
i = (i & 0x55555555) + ((i >> 1) & 0x55555555);
{% endhighlight %}
其中:

* **(i & 0x55555555)**得到```j```，```j```的偶数位全为0，奇数位与```i```的奇数位相对应； 

* **(i >> 1) & 0x55555555)**得到```q```，```q```的偶数位全为0，奇数位与```i```的偶数位相对应。

通过上面**(i & 0x55555555) + ((i >> 1) & 0x55555555)**，即```j+q```得到```z```，```z```第1位和第2位的二进制表示(00或01或10)就是i的第1、2位中值为1的数量。这样```z```中每两位一组的二进制记录了```i```中对应的两位上的非0的个数。

2） **步骤2**
{% highlight string %}
i = (i & 0x33333333) + ((i >> 2) & 0x33333333);	
{% endhighlight %}
该步中的```i```即上一步中得到的```z```，如果该步骤中得到的结果是```y```，那么y中每四位一组的二进制表示记录了i(原始数据）中对应的四位上的非0的个数。

3）**步骤3**
{% highlight string %}
{% endhighlight %}






<br />
<br />

1. [variable-precision SWAR算法“计算汉明重量”](https://blog.csdn.net/u010320108/article/details/60878085)

<br />
<br />


