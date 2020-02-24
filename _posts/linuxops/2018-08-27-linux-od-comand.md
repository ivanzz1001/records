---
layout: post
title: Linux中od命令的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux中od命令的使用
---

本文主要讲述一下Linux中dd命令的使用。


<!-- more -->


## 1. od命令
od命令用于将指定文件内容以八进制、十进制、十六进制、浮点格式或ASCII编码字符方式显示。通常用于显示或查看文件中不能直接显示在终端的字符。od命令系统默认的显示方式是八进制，名称源于Octal Dump。

常见的文件为文本文件和二进制文件。od命令主要用来查看保存在二进制文件中的值，按照指定格式解释文件中的数据并输出，不管是IEEE754格式的浮点数还是ASCII码，od命令都能按照需求输出它们的值。

### 1.1 命令格式
<pre>
# od [OPTION]... [FILE]...
</pre>
其中命令选项主要有：
{% highlight string %}
-A, --address-radix=RADIX
         在输出中以何种格式来表示地址偏移，其中RADIX的可选值有[doxn]。d表示Decimal,
         o表示Octal，x表示Hex，n表示None

-j, --skip-bytes=BYTES
         表示跳过开头的BYTES个字节

-N, --read-bytes=BYTES
         表示只dump BYTES个字节

-S BYTES, --strings[=BYTES]
         输出长度不小于指定字节数的字符串

-w[BYTES], --width[=BYTES]
         设置每行显示的字节数，od默认每行显示32字节

-t, --format=TYPE
         指定输出格式，格式包括a、c、d、f、o、u和x，各含义如下：
         1) a: named character, ignoring high-order bit
         2) c: select printable characters or backslash escapes
         3) d[SIZE]：十进制，正负数都包含，SIZE字节组成一个十进制整数；
         4) f[SIZE]：浮点，SIZE字节组成一个浮点数；
         5) o[SIZE]：八进制，SIZE字节组成一个八进制数；
         6) u[SIZE]：无符号十进制，只包含正数，SIZE字节组成一个无符号十进制整数；
         7) x[SIZE]：十六进制，SIZE字节为单位以十六进制输出，即输出时一列包含SIZE字节


此外，我们还有一些简写的方式来输出相应格式：
-a     same as -t a,  select named characters, ignoring high-order bit

-b     same as -t o1, select octal bytes

-c     same as -t c,  select printable characters or backslash escapes

-d     same as -t u2, select unsigned decimal 2-byte units

-f     same as -t fF, select floats

-i     same as -t dI, select decimal ints

-l     same as -t dL, select decimal longs

-o     same as -t o2, select octal 2-byte units

-s     same as -t d2, select decimal 2-byte units

-x     same as -t x2, select hexadecimal 2-byte units
{% endhighlight %}

### 1.2 使用实例
首先我们先准备一个tmp文件：
<pre>
# echo abcdef g >tmp
# cat tmp
abcdef g
</pre>

1) **使用单字节八进制解释进行输出**
<pre>
# od -b ./tmp
0000000 141 142 143 144 145 146 040 147 012
0000011
</pre>
注意： 左侧的默认地址格式为八进制表示

2) **使用ascii码进行输出**
<pre>
# od -c ./tmp
0000000   a   b   c   d   e   f       g  \n
0000011
</pre>
注意，其中包括转义字符

3) **使用单字节十进制进行解释**
<pre>
# od -t d1 ./tmp
0000000   97   98   99  100  101  102   32  103   10
0000011
</pre>

4) **设置地址格式为十进制**
<pre>
# od -A d -c ./tmp
0000000   a   b   c   d   e   f       g  \n
0000009
</pre>

5) **设置地址格式为十六进制**
<pre>
# od -A x -c ./tmp
000000   a   b   c   d   e   f       g  \n
000009
</pre>

6) **跳过开始的2个字节**
<pre>
# od -j 2 -c ./tmp
0000002   c   d   e   f       g  \n
0000011
</pre>

7) **跳过开始的两个字节，并且仅输出两个字节**
<pre>
# od -N 2 -j 2 -c ./tmp
0000002   c   d
0000004
</pre>

8) **每行仅输出1个字节**
<pre>
# od -w1 -c tmp
0000000   a
0000001   b
0000002   c
0000003   d
0000004   e
0000005   f
0000006    
0000007   g
0000010  \n
0000011
</pre>

9) **每行输出两个字节**
<pre>
# od -w2 -c tmp
0000000   a   b
0000002   c   d
0000004   e   f
0000006       g
0000010  \n
0000011
</pre>

10) **每行输出3个字节，并使用八进制单字节进行解释**
<pre>
# od -w3 -b tmp
0000000 141 142 143
0000003 144 145 146
0000006 040 147 012
0000011
</pre>

## 2. xxd命令
这里顺带再介绍一个xxd命令，该命令以十六进制方式显示文件，例如：
<pre>
# xxd ./tmp
00000000: 6162 6364 6566 2067 0a                   abcdef g.
</pre>

<br />
<br />

**[参看]**

1. [od命令](http://man.linuxde.net/od)


2. [Linux od命令详细介绍及用法实例](https://www.jb51.net/article/102421.htm)

<br />
<br />
<br />


