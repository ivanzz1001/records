---
layout: post
title: mysql数据库基础（二）
tags:
- database
categories: database
description: mysql数据库基础
---



本文讲述一下mysql数据库的一些基础知识及相关操作。当前的数据库版本为：```5.7.22 MySQL Community Server (GPL)```


<!-- more -->

## 1. 数据库的创建与删除

* 创建数据库

登录MySQL数据库以后，我们可以通过```CREATE```命令创建数据库，语法如下：
{% highlight string %}
CREATE DATABASE <dbname> 
{% endhighlight %}

如下我们创建一个名称为```test```的数据库：
{% highlight string %}
mysql> create database test;
Query OK, 1 row affected (0.04 sec)

mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| app                |
| mysql              |
| performance_schema |
| sys                |
| test               |
+--------------------+
6 rows in set (0.00 sec)
{% endhighlight %}

* 删除数据库

登录MySQL数据库后，我们可以通过```DROP```命令删除数据库，语法如下：
{% highlight string %}
DROP DATABASE <dbname>
{% endhighlight %}
如下我们删除```test```数据库：
{% highlight string %}
mysql> drop database test;
Query OK, 0 rows affected (0.08 sec)
{% endhighlight %}

* 选择数据库

使用```USE```命令选择数据库：
{% highlight string %}
USE <dbname>
{% endhighlight %}


## 2. MySQL数据类型
MySQL数据类型整体上可以分为如下几大类：

* 数值类型

* 日期和时间类型

* 字符串类型

* 空间(Spatial)类型

这里我们只介绍前面3种，最后一种暂不做介绍。

### 2.1 数值类型

在如下介绍的```数值类型```(Numeric Type), 符号```M```用于指示该数值类型的显示宽度。

<pre>
注： 关于显示宽度，只是告诉MySQL数据库，我们对该字段宽度通常为指定的宽度。但是这不是强制性的，如果不等于该宽度也是可以的（一般不能
    超过数值类型中mysql规定的最大值255)
</pre>



## 2. 表操作

**1) 导出整个数据库表结构**
{% highlight string %}
# mysqldump -hhostname -uusername -ppassword -d databasename >> databasename.sql
{% endhighlight %}






<br />
<br />
**[参看]**:

1. [MySQL基础](https://www.toutiao.com/a6543580080638001668/)

2. [mySQL](https://baike.baidu.com/item/mySQL/471251?fr=aladdin)

3. [MySQL用户权限(Host,User,Password)管理(mysql.user)](https://blog.csdn.net/typa01_kk/article/details/49126365)

4. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)


<br />
<br />
<br />

