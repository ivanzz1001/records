---
layout: post
title: mysql数据库基本操作
tags:
- database
categories: database
description: mysql数据库基本操作
---



本文记录一下mysql数据库的基本操作。


<!-- more -->


## 1. 表操作



**1) 导出mysql建表语句**
{% highlight string %}
mysql> show create table test_1\G;
*************************** 1. row ***************************
       Table: token_1
Create Table: CREATE TABLE `test_1` (
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `token` varchar(36) NOT NULL,
  `appid` varchar(64) NOT NULL,
  `expire` bigint(20) NOT NULL,
  `acl` varchar(64) NOT NULL,
  `ak` varchar(64) NOT NULL,
  `sk` varchar(64) NOT NULL,
  `create_ts` bigint(20) NOT NULL COMMENT '创建时间',
  PRIMARY KEY (`seqid`),
  UNIQUE KEY `token` (`token`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8
1 row in set (0.00 sec)

ERROR: 
No query specified
{% endhighlight %}
上面导出一个```test_1```数据表的建表过程。

**2) 导出整个数据库表结构**
{% highlight string %}
# mysqldump -hhostname -uusername -ppassword -d databasename >> databasename.sql
{% endhighlight %}



<br />
<br />
**[参看]**:

1. [MySQL基础](https://www.toutiao.com/a6543580080638001668/)

<br />
<br />
<br />

