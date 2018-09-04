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
mysql> show create table catalogue_index_0 \G
*************************** 1. row ***************************
       Table: catalogue_index_0
Create Table: CREATE TABLE `catalogue_index_0` (
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `fileid` varchar(24) COLLATE utf8_unicode_ci NOT NULL,
  `md5` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `filetype` int(2) NOT NULL,
  `parentid` varchar(72) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `filename` varchar(512) COLLATE utf8_unicode_ci NOT NULL,
  `filesize` bigint(20) DEFAULT '0',
  `userid` varchar(64) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `deletetag` int(1) NOT NULL,
  `create_ts` bigint(20) NOT NULL,
  `modify_ts` bigint(20) NOT NULL,
  `upUid` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`seqid`),
  KEY `key_parentid` (`parentid`),
  KEY `key_fileid` (`fileid`)
) ENGINE=InnoDB AUTO_INCREMENT=269 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci
1 row in set (0.00 sec)
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

