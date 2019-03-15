---
layout: post
title: uniq命令的使用
tags:
- LinuxOps
categories: linuxOps
description: uniq命令的使用
---

本节主要介绍一下uniq命令的使用。

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux sz-oss-01.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>


<!-- more -->


## 1. uniq命令
```uniq```命令用于报告或删除重复的行。其基本语法如下：
<pre>
uniq [OPTION]... [INPUT [OUTPUT]]
</pre>

### 1.1 相关选项
```uniq```命令用于过滤相邻的匹配行，并将结果输出到标准输出。下面介绍一下常用选项：
{% highlight string %}
1) -c, --count: 在每列旁边显示该行重复出现的次数

2) -d, --repeated: 仅显示重复出现的行，即出现次数>=2的行，且只打印一次

3) -D, --all-repeated[=METHOD]: 仅显示重复的行，即出现次数>=2的行，且打印重复行的所有行。其中 METHOD 表示对重复行集合的分隔方式，有三种取值，分别为none、prepend和separate。
                                其中none表示不进行分隔，为默认选项，uniq -D等同于uniq --all-repeated=none；prepend表示在每一个重复行集合前面插入一个空行；separate表示在
                                每个重复行集合间插入一个空行

3) -f, --skip-fields=N: 忽略前N个字段。字段由空白字符（空格符、Tab）分隔。

4) -i, --ignore-case: 当进行比较的时候忽略大小写

5) -s, --skip-chars=N: 在比较时忽略前面N个字符

6） -u, --unique： 只打印不重复的行

7) -w, --check-chars=N: 一行中比较的字符数不超过N个字符
{% endhighlight %}


## 2. 使用示例
1） **对无序文件去重无效**
<pre>
# cat testfile.txt 
hello
world
friend
hello
world
hello

# uniq ./testfile.txt 
hello
world
friend
hello
world
hello
</pre>
上面我们看到，直接删除未经排序的文件，将会发现没有任何行被删除。

2) **uniq结合sort命令，对排序文件进行去重**
{% highlight string %}
# cat testfile.txt 
hello
world
friend
hello
world
hello
# cat testfile.txt | sort | uniq
friend
hello
world
{% endhighlight %}

3) **排序之后删除重复行，同时在行首位置输出该行重复的次数**
{% highlight string %}
# cat testfile.txt
hello
world
friend
hello
world
hello
# sort testfile.txt | uniq -c
      1 friend
      3 hello
      2 world
{% endhighlight %}

4) **仅显示存在重复的行，并在行首显示该行重复的次数**
<pre>
# cat testfile.txt
hello
world
friend
hello
world
hello
# sort testfile.txt | uniq -dc
      3 hello
      2 world
</pre>

5) **仅显示不重复的行**
{% highlight string %}
# cat testfile.txt 
hello
world
friend
hello
world
hello
# sort testfile.txt | uniq -u
friend
{% endhighlight %}

6) **仅显示重复的行，且显示重复行的所有行**
<pre>
# cat testfile.txt 
hello
world
friend
hello
world
hello
# sort testfile.txt | uniq --all-repeated=prepend

hello
hello
hello

world
world
# sort testfile.txt | uniq -D
hello
hello
hello
world
world
</pre>

7) **通过-w选项比较前N个字符**

uniq默认是比较相邻行的所有内容来判断是否重复，我们可以通过选项```-w```或```--check-chars=N```指定比较前N个字符。比如我们有如下内容的文件test.txt：
<pre>
# cat test.txt
apple
application
api
# uniq -w3 -D test.txt
apple
application
</pre>

8) **统计行数**
{% highlight string %}
# last | awk '{S[$3]++} {for(a in S){print a}}' | sort | uniq -c | sort -rn
    125 10.133.150.6
    123 10.133.146.59
    121 10.133.146.60
    117 10.133.147.43
    116 10.133.150.83
    115 10.133.146.55
    110 172.28.144.47
     97 10.133.150.85
     96 172.28.144.18
     94 10.133.146.43
     68 10.133.146.47
     64 10.133.150.111
     53 10.133.143.106
     45 10.133.146.67
     41 10.133.143.116
     29 boot
     27 10.133.143.107
      9 10.133.143.122
      2 
      1 Tue
{% endhighlight %}

如下我们只显示重复的内容：
<pre>
# last | awk '{S[$3]++} {for(a in S){print a}}' | sort | uniq -d

10.133.143.106
10.133.143.107
10.133.143.116
10.133.143.122
10.133.146.43
10.133.146.47
10.133.146.55
10.133.146.59
10.133.146.60
10.133.146.67
10.133.147.43
10.133.150.111
10.133.150.6
10.133.150.83
10.133.150.85
172.28.144.18
172.28.144.47
boot
</pre>

如下我们只显示出现一次的行：
{% highlight string %}
# last | awk '{S[$3]++} {for(a in S){print a}}' | sort | uniq -u
Tue
{% endhighlight %}


<br />
<br />

**[参看]**


1. [Linux uniq命令详解](https://www.cnblogs.com/ftl1012/p/uniq.html)

2. [linux uniq命令](https://blog.csdn.net/technologyleader/article/details/81941410)

3. [uniq命令](http://man.linuxde.net/uniq)
<br />
<br />
<br />


