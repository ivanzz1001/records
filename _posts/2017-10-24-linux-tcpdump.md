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


## 3. OPTIONS
* ```-A```: 以ASCII码的形式打印每一个数据包（去除链路层头部信息）。这一般可以很方便的用于抓取网页。

* ```-B``` *buffer_size*:

* ```--buffer-size=```*buffer_size*: 设置操作系统抓取缓存空间大小，单位为KiB(1024Bytes)


* ```-c```: 指定抓取的数据包个数

* ```-d```: 以容易阅读的形式,在标准输出上打印出编排过的包匹配码, 随后tcpdump停止

* ```-e```: 在dump出来的每一行上打印出链路层头部信息。例如，针对以太网协议或是IEEE 802.11协议可以打印MAC地址

* ```-F file```: 使用file文件作为filter expression的输入，此时会忽略命令行上传入表达式。

* ```-i``` *interface*:
* ```--interface=```*interface*: 指定在那个网络接口上进行监听。假如未指定，tcpdump会查询当前配置的编号最低的网络接口（不包括loopback接口），例如可能选取为eth0。 在Linux2.2以后版本系统中，本参数可以被设置为```any```,这使得tcpdump可以监听所有网卡接口。

* ```-n```: 不要将地址（例如：主机地址或端口）转换成名称

* ```--print```: 向控制台打印出已经解析的包（即使在有```-w```选项的情况下）

* ```-q```: Quick(quiet?) 输出。打印更少的协议信息，因此输出行会相应更短。

* ```-T``` *type*: 强制匹配```expression```的数据包解析称为type指定的类型。

* ```-t```: 在每一个输出行中不要打印时间戳

* ```-tt```: 打印时间戳（从January 1, 1970, 00:00:00, UTC到现在的秒数 及 一秒的小数部分）

* ```-v```: 




<br />
<br />
<br />





