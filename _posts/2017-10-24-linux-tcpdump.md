---
layout: post
title: Linux tcpdump的使用
tags:
- LinuxOps
categories: linux
description: Linux tcpdump的使用
---


tcpdump工具包含很多选项，但是很多选项并不常用。本文简要的介绍一下tcpdump的使用，要详细了解请参看[官网](http://www.tcpdump.org/manpages/tcpdump.1.html)


<!-- more -->


## 1. 基本用法

如下给出tcpdump使用的基本方法：
<pre>
tcpdump [ -AbdDefhHIJKlLnNOpqRStuUvxX ] [ -B buffer_size ] [ -c count ]
       [ -C file_size ] [ -G rotate_seconds ] [ -F file ]
       [ -i interface ] [ -j tstamp_type ] [ -m module ] [ -M secret ]
       [ -P in|out|inout ]
       [ -r file ] [ -V file ] [ -s snaplen ] [ -T type ] [ -w file ]
       [ -W filecount ]
       [ -E spi@ipaddr algo:secret,...  ]
       [ -y datalinktype ] [ -z postrotate-command ] [ -Z user ]
       [ expression ]
</pre>

## 2. Description

Tcpdump会打印出一个网络接口上匹配布尔表达式```expression```的网络包的内容，打印出的内容前面通常都包含时间戳信息。在使用时也可以通过```-w```选项将抓取到的数据包写到一个文件以供后续分析；或者通过```-r```选项从保存的文件中读取一个数据包，而不是从网络接口来抓包； 也可以通过```-V```选项来读取保存有数据包的文件列表。在任何情况下只有匹配```expression```的数据包才会被处理。

假如不指定```-c```选项的话，tcpdump将会一直抓取数据包，除非被```SIGINT```或```SIGTERM```信号中断（通常情况下可以通过control-C来产生SIGINT信号，通过kill命令来产生SIGTERM信号）。如果指定```-c```选项的话，则会在收到```SIGINT```信号，或收到```SIGTERM```信号，或收到指定数量的数据包时tcpdump就会停止抓取。

当tcpdump完成数据包的抓取时，其通常会报告如下一些信息：
{% highlight string %}

{% endhighlight %}




<br />
<br />
<br />





