---
layout: post
title: linux中cut命令的用法
tags:
- LinuxOps
categories: linuxOps
description: linux中cut命令的用法
---


本文主要记录一下Linux中cut命令的用法。

<!-- more -->

## 1. cut命令使用说明
<pre>
Usage: cut OPTION... [FILE]...
</pre>

cut命令用于选出```FILE```文件中每一行的某些部分,然后打印到标准输出。

cut命令支持的参数列表：
<pre>
 -b, --bytes=LIST           选择指定位置处(字节处）的数据
 -c, --characters=LIST      选择指定字符处的数据
 -d, --delimiter=DELIM      使用DELIM作为字段分隔符，而不是采用默认的TAB
 -f, --fields=LIST          只选择指定field出的内容;同时也会打印出不包含delimiter的行（除非指定了-s选项)

 -n                         搭配-b选项使用： 表示不要分割多字节字符。如果字符的最后一个字节落在由 -b 标志的 List 参数指示的范
                            围之内，该字符将被写出；否则，该字符将被排除。
 --complement               补全选中的字节、字符或域
 -s, --only-delimited       不打印没有包含分界符的行
 --output-delimiter=STRING  使用STRING作为输出分隔符
</pre>

可以通过如下的方式来指定范围：
<pre>
 N     第N个字节/字符/field， 索引从1开始
 N-    从第N个字节/字符/field开始，一直到行尾
 N-M   从第N到第M(included)个字节/字符/field
 -M    从第1到第M(included)字节/字符/field
</pre>

## 2. 用法举例
有如下两个文件:

1) test1.txt
{% highlight string %}
abcd	efg
hijk	lmn

opqr	st
uvwx	yz
{% endhighlight %}

注意： 上面以TAB键分割，中间还有一行空行

2) test2.txt
{% highlight string %}
星期一
星期二
星期三

星期四
星期五
星期六
星期天
{% endhighlight %}


**1） 剪切单个字节**
<pre>
# cut -b 1 test1.txt
a
h

o
u
</pre>

**2) 剪切多个字节**
<pre>
//剪切1、3、5个字节
# cut -b 1,3,5 test1.txt
ac
hj

oq
uw

//剪切1-6字节
# cut -b 1-6 test1.txt
abcd    e
hijk    l

opqr    s
uvwx    y
</pre>

**3) 剪切字符**
{% highlight string %}
# cut -b 2 test2.txt
�
�
�

�
�
�
�
{% endhighlight %}
这里我们看到出现乱码，因为```-b```只是针对字节进行裁剪，对一个汉字进行字节裁剪，得到的结果必然是乱码，若想使用```-b```命令对字节进行裁剪，那么则需要使用```-n```选项，此选项的作用是取消分割多字节字符。
{% highlight string %}
[root@localhost test]# cut -nb 3 test2.txt
一
二
三

四
五
六
天
{% endhighlight %}

**4) 字符剪切(-c)**
{% highlight string %}
# cut -c 3 test2.txt
一
二
三

四
五
六
天
{% endhighlight %}

**5) 剪切指定域**

上面的```-b```、```-c```只是针对于格式固定的数据中剪切，但是对于一些格式不固定的，就没有办法获取到我们想要的数据，因此便有了```-f ```域的概念
{% highlight string %}
# cat /etc/passwd | head -n 3
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
daemon:x:2:2:daemon:/sbin:/sbin/nologin
{% endhighlight %}

将上面的第一个```:```前面的字符给剪切出来，那么我们就可以使用```-d```命令，指定其分割符为```：```然后再选取第一个域内的内容即可:
<pre>
# cat /etc/passwd | head -n 3 | cut -d : -f 1
root
bin
daemon

# cat /etc/passwd | head -n 3 | cut -d : -f 7
/bin/bash
/sbin/nologin
/sbin/nologin
</pre>

**6) 剪切出IP地址**
{% highlight string %}
# ifconfig | grep -w "inet"
        inet 172.18.0.1  netmask 255.255.0.0  broadcast 172.18.255.255
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        inet 192.168.69.128  netmask 255.255.255.0  broadcast 192.168.69.255
        inet 127.0.0.1  netmask 255.0.0.0
        inet 192.168.122.1  netmask 255.255.255.0  broadcast 192.168.122.255

# ifconfig | grep -w "inet" | tr -s ' ' | cut -d ' ' -f 3,5,7 --output-delimiter="    "
172.18.0.1    255.255.0.0    172.18.255.255
172.17.0.1    255.255.0.0    172.17.255.255
192.168.69.128    255.255.255.0    192.168.69.255
127.0.0.1    255.0.0.0
192.168.122.1    255.255.255.0    192.168.122.255
{% endhighlight %}
注意： 如果你的文件分隔符恐怕不止一个空格的话，此时若要用cut，则必须先用命令```tr -s ' '```将多余的空格剔除。

<br />
<br />
**[参看]:**

1. [linux cut用法](http://blog.csdn.net/u011003120/article/details/52190187)


<br />
<br />
<br />


