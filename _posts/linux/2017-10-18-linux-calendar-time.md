---
layout: post
title: 日历时间
tags:
- LinuxOps
categories: linux
description: 日历时间
---


日历时间，是用“从一个标准时间点到此时的时间经过的秒数”来表示的时间。日历时间（Calendar Time)是通过time_t数据类型来表示的，用time_t表示的时间(日历时间）是从一个时间点（例如：1970年1月1日0时0分0秒）到此时的秒数。



<!-- more -->


## 1. 日历时间
 
这个标准时间点对不同的编译器来说会有所不同，但对一个编译系统来说，这个标准时间点是不变的，该编译系统中的时间对应的日历时间都通过该标准时间点来衡量，所以可以说日历时间是“相对时间”，但是无论你在哪一个时区，在同一时刻对同一个标准时间点来说，日历时间都是一样的。在C语言中通过time函数获得日历时间。

日历时间(Calendar Time)是通过time_t数据类型来表示的，用time_t表示的时间（日历时间）是从一个时间点(例如:1970年1月1日0时0分0秒）到此时的秒数。

time_t实际上是长整形，到未来的某一天，从一个时间点（一般是1970年1月1日0时0分0秒）到那时的秒数（即日历时间）超出了长整型所能表示的数的范围怎么办？ 对time_t数据类型的值来说，它所表示的时间不能晚于2038年1月18日19时14分07秒。为了能够表示更久远的时间，一些编译器厂商引入了64位甚至更长的整型数来保存日历时间。比如微软在Visual C++中采用了__time64_t数据类型来保存日历时间，并通过_time64()函数来获得日历时间（而不是通过使用32位字的time()函数），这样就可以通过该数据类型保存3001年1月1日0时0分0秒（不包括该时间点）之前的时间。


## 2. 程序案例
{% highlight string %}
#include <time.h>
#include <stdio.h>
#include <dos.h>

int main(void)
{
	time_t t;
	t = time(NULL);
	printf("The number of seconds since January 1, 1970 is %ld\n",t);
	return 0;
}
{% endhighlight %}

用time()函数结合其他函数（如：localtime、gmtime、asctime、ctime）可以获得当前系统时间或是标准时间。具体用法参见这几个函数。









<br />
<br />

**[参看]:**

1. [日历时间](https://baike.baidu.com/item/%E6%97%A5%E5%8E%86%E6%97%B6%E9%97%B4/2763820?fr=aladdin)


<br />
<br />
<br />


