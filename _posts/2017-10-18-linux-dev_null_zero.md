---
layout: post
title: Linux /dev/null和/dev/zero简介及对比
tags:
- LinuxOps
categories: linux
description: Linux /dev/null和/dev/zero简介及对比
---


本章我们主要介绍Linux下的两个特殊文件---/dev/null和/dev/zero.


<!-- more -->


## 1. 概念

* /dev/null: 在类Unix系统中，/dev/null，或称为空设备，是一个特殊的设备文件。它丢弃一切写入其中的数据（但报告写入操作成功），读取它则会立即得到一个EOF。在程序员行话尤其是Unix行话中，/dev/null被称为位桶(bit bucket)或者黑洞(black hole)。空设备常被用于丢弃不必要的的输出流，或作为用于输入流的空文件。这些操作通常由重定向完成。

* /dev/zero: 在类Unix操作系统中，/dev/zero是一个特殊的文件，当你读它的时候它提供无限的空字符（NULL, ASCII NUL, 0x00)。其中的一个典型用法是**用它提供的字符流来覆盖信息**；另一个常见的用法是**产生一个特定大小的空白文件**。 BSD就是通过mmap把/dev/zero映射到虚拟地址空间来实现共享内存的。可以使用mmap将/dev/zero映射到一个虚拟的内存空间，这个操作的效果等同于使用一段匿名内存（没有和任何文件关联）

## 2. /dev/null的日常使用
把/dev/null看作“黑洞”，它等价于一个只写文件，并且所有写入它的内容都会永远丢失，而尝试从它哪儿读取内容则什么也读不到。然而，/dev/null对命令行和脚本都非常的有用。

我们都知道```cat $filename```会输出filename对应的文件内容（输出到标准输出）。而使用```cat $filename > /dev/null```则不会得到任何信息，因为我们将本来该通过标准输出显示的文件信息重定向到了/dev/null中。参看如下示例：
<pre>
# cat << EOF >> devnull_test.txt
> test /dev/null ....
> EOF
# cat devnull_test.txt >/dev/null
# cat devnull_test.txt 1>/dev/null      //将标准输出重定向到/dev/null


# cat abcd.txt >/dev/null				//abcd.txt并不存在
cat: abcd.txt: No such file or directory
# cat abcd.txt 2>/dev/null              //将标准错误重定向到/dev/null中
</pre>

有时候我们可能并不想要看到任何输出，我们只想看这条命令是不是运行正常，那么我们可以同时禁止标准输出和标准错误输出。例如：
<pre>
# cat devnull_test.txt 2>/dev/null
test /dev/null ....
# cat devnull_test.txt >/dev/null 2>/dev/null 
# echo $?
0

# cat abcd.txt 2>/dev/null                             //abcd.txt并不存在
# cat abcd.txt >/dev/null 2>/dev/null
# echo $?
1
</pre>

有时候，我们需要删除一些文件的内容而不删除文件本身(这个方法可以用来删除日志文件，在我的Debian笔记本上我给 /var 盘配的空间有些过小，有时候就需要手动使用这个操作来清空日志):
<pre>
# cat /dev/null >/var/log/messages
</pre>

## 3. /dev/zero的日常使用
















<br />
<br />

**[参看]:**

1. [Linux 下的两个特殊的文件 -- /dev/null 和 /dev/zero 简介及对比](http://blog.csdn.net/pi9nc/article/details/18257593)


<br />
<br />
<br />


