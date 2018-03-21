---
layout: post
title: linux操作系统中locate命令的使用
tags:
- LinuxOps
categories: LinuxOps
description: linux操作系统中locate命令的使用
---

本章我们主要谈一谈Linux操作系统下locate命令的使用.



<!-- more -->


## 1. locate命令

locate命令用于在文件系统中通过名字来查找文件。

### 1.1 简述

```locate```命令用于查找文件或目录。locate命令要比```find -name```快得多，原因在于它不搜索具体目录，而是搜索一个数据库/var/lib/mlocate/mlocate.db。这个数据库中含有本地所有文件信息。Linux系统自动创建这个数据库，并且每天自动更新一次。因此，我们在用whereis 和 locate查找文件时，有时会找到已经被删除的数据；或者刚刚建立的文件，却无法查找到，原因就是因为数据库文件没有被更新。为了避免这种情况，可以在使用locate之前，先使用```updatedb```命令，手动更新数据库。整个locate工作其实是由4部分组成的：

* /usr/bin/updatedb: 主要用来更新数据库，通过crontab自动完成的

* /usr/bin/locate: 查询文件位置

* /etc/updatedb.conf: updatedb的配置文件

* /var/lib/mlocate/mlocate.db: 存放文件信息的文件

 
```locate```会读取由updatedb准备好的一个或多个数据库，然后将满足匹配```PATTERN```的文件写到标准输出，每行一个文件名。假如并未指定```--regex```选项，则```PATTERN```可以包含通配符。假如```PATTERN```中并未包含任何通配符，则locate命令以```*PATTERN*```模式进行查找。

默认情况下，locate命令并不会检查数据库中的文件是否仍然存在，也不会报告在上一次更新数据库之后产生的文件。

### 1.2 用法
<pre>
locate [OPTION]... [PATTERN]...
</pre>

### 1.3 选项
{% highlight string %}
-A, --all               打印所有匹配PATTERNs的文件
-b, --basename          只名称与PATTERNs匹配的文件（例如/test/test.txt，则basename为test.txt)
-c, --count             只打印匹配到的文件数目，而不是文件名本身
-d, --database DBPATH   用DBPATH来代替默认的数据库。DBPATH是一个以：（冒号）分割的数据库名称列表
-e, --existing          只打印在调用locate时存在的文件
-L, --follow            当和-e选项一起使用时，如果该文件是一个软链接文件，则会获取到该软链接对应的实际文件（此为locate命令默认行文）。
-h, --help              打印帮助文档信息
-i, --ignore-case       当进行文件模式匹配时，忽略大小写
-l, --limit, -n LIMIT   当成功匹配到LIMIT数量的文件时，退出
-P, --nofollow, -H      当和-e选项一起使用时，如果该文件是一个软链接文件，此时不跟随链接的实际文件
-0, --null              输出时以ASCII NUL作为分隔符
-S, --statistics        获得每一个搜索数据库的统计信息
-q, --quiet             当在读取和处理数据库时，遇到错误也不打印相关信息
-r, --regexp REGEXP     查找一个基本的模式匹配REGEXP。此选项情况下，不能出现PATTERNs
-V, --version           打印locate的版本信息和license信息
-w, --wholename         匹配完整路径名（默认）
{% endhighlight %}
例如查找一个文件名称为```NAME（并不是*NAME*)```的文件,可以使用如下：
{% highlight string %}
ff
{% endhighlight %}


### 1.4 示例





<br />
<br />
**[参看]:**

1. [Linux下which、whereis、locate、find 命令的区别](http://blog.chinaunix.net/uid-20554039-id-3035417.html)

2. [每天一个linux命令:locate](https://www.cnblogs.com/xqzt/p/5426666.html)

3. [Linux 命令（文件和目录管理 - locate）](http://blog.csdn.net/liang19890820/article/details/53285624)
<br />
<br />
<br />





