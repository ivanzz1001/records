---
layout: post
title: MYSQL数据库查看被锁状态以及解锁
tags:
- database
categories: database
description: MYSQL数据库查看被锁状态以及解锁
---


本文记录以下如何查看MySQL数据库被锁状态以及解锁方法。


<!-- more -->


## 1. MySQL数据库查看被锁状态及解锁

前言：在信息时代，数据时最重要的,数据库一般都存在数据库的表中，但当表被锁的时候，数据无法读或者写，造成数据的丢失,进而产生严重的后果...  

### 1.1 查看MySQL数据库被锁状态

查询mysql哪些表正处于被锁状态，有如下两种方式：

1） **方式1**

{% highlight string %}
# show OPEN TABLES where In_use > 0;
{% endhighlight %}

2) **方式2**

如下命令查看数据库中表的状态，是否被锁:
{% highlight string %}
# SHOW PROCESSLIST;
{% endhighlight %}


### 1.2 解锁锁定的表
<pre>
# kill id
</pre>

<br />
<br />
**[参看]**:

1. [MYSQL数据库查看被锁状态以及解锁](https://www.cnblogs.com/dengzhangkun/p/3670822.html)



<br />
<br />
<br />

