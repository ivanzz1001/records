---
layout: post
title: Linux文件统计
tags:
- LinuxOps
categories: linuxOps
description: Linux文件统计
---

本章我们主要介绍一下Linux中的文件统计。主要包括两方面的内容：

* 统计文件个数

* 统计文件所占空间


<!-- more -->

## 1. 统计文件个数

* 统计某一目录下的文件个数（不包括目录及子目录下的文件）
<pre>
# ls -l nginx-1.10.3
total 672
drwxr-xr-x 6 1001 1001    326 Mar 19 00:49 auto
-rw-r--r-- 1 1001 1001 265299 Jan 31  2017 CHANGES
-rw-r--r-- 1 1001 1001 404694 Jan 31  2017 CHANGES.ru
drwxr-xr-x 2 1001 1001    168 Mar 19 00:49 conf
-rwxr-xr-x 1 1001 1001   2481 Jan 31  2017 configure
drwxr-xr-x 4 1001 1001     72 Mar 19 00:49 contrib
drwxr-xr-x 2 1001 1001     40 Mar 19 00:49 html
-rw-r--r-- 1 1001 1001   1397 Jan 31  2017 LICENSE
-rw-r--r-- 1 root root    337 Mar 22 02:22 Makefile
drwxr-xr-x 2 1001 1001     21 Mar 19 00:49 man
drwxr-xr-x 4 root root    138 Mar 22 02:22 objs
-rw-r--r-- 1 1001 1001     49 Jan 31  2017 README
drwxr-xr-x 9 1001 1001     91 Mar 19 00:49 src

# ls -l nginx-1.10.3 | grep "^-" | wc -l
6
</pre>

* 统计某一目录下的文件个数（包括子目录下的文件)
<pre>
# ls -lR ./nginx-1.10.3 |grep "^-"|wc -l
394

# find ./nginx-1.10.3 -type f | wc -l
394
</pre>

* 统计某一目录下的文件夹个数（包括子目录下的文件夹）
<pre>
# ls -lR ./nginx-1.10.3 | grep "^d" | wc -l
55

//说明： 通过如下find查找，包括./nginx-1.10.3这个目录本身
# find ./nginx-1.10.3 -type d | wc -l		
56
</pre>


## 2. 统计文件所占空间

```du命令```用来查看目录或文件所占用的磁盘空间的大小，常用的选项有：
{% highlight string %}
-h     以人类可读的方式来显示文件大小

-a     显示目录占用的磁盘空间大小，还要显示其下目录和文件占用磁盘空间的大小

-s     显示目录占用的磁盘空间大小，不要显示其子目录和文件占用的磁盘空间大小

-c     显示 '整个' 目录或文件所占用的磁盘空间大小，还要统计它们的总和

--apparent-size: 显示目录或文件自身的大小

-l     统计硬链接占用磁盘空间的大小

-L     统计符号链接所指向的文件占用的磁盘空间大小
{% endhighlight %}

### 2.1 示例

* 查看某一目录总共占用的容量，而不是单独列出各子项占用的容量
<pre>
# du -sh ./nginx-1.10.3
6.2M    ./nginx-1.10.3
</pre>


* 查看某一目录下一级子文件和子目录占用的磁盘容量
<pre>
# du -lh --max-depth=1 ./nginx-1.10.3
408K    ./nginx-1.10.3/auto
36K     ./nginx-1.10.3/conf
76K     ./nginx-1.10.3/contrib
4.9M    ./nginx-1.10.3/src
8.0K    ./nginx-1.10.3/html
8.0K    ./nginx-1.10.3/man
76K     ./nginx-1.10.3/objs
6.2M    ./nginx-1.10.3
</pre>


* 统计当前文件夹（目录）大小，并按文件大小排序
<pre>
# du -sh * | sort -h
172K    nginx-hello-world-module
596K    zlib-1.2.11.tar.gz
892K    nginx-1.10.3.tar.gz
2.0M    pcre-8.40.tar.gz
3.7M    zlib-1.2.11
6.2M    nginx-1.10.3
12M     pcre-8.40
</pre>

## 3. 查看各分区的使用情况 
我们可以使用```df -h```命令来查看各分区的使用情况：
<pre>
# df -h
</pre>




<br />
<br />

**[参看]**

1. [Linux下用ls和du命令查看文件以及文件夹大小](https://www.cnblogs.com/xueqiuqiu/p/7635722.html)

<br />
<br />
<br />


