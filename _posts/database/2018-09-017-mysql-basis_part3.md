---
layout: post
title: mysql数据库基础（三）
tags:
- database
categories: database
description: mysql数据库基础
---


本章包含两个方面的内容：

* MySQL服务器模式

* MySQL字符集与校对集


<!-- more -->

## 1. MySQL服务器模式
不同的MySQL客户端可以通过不同的模式操作MySQL Server。DBA可以设置一个全局模式，而每个应用程序可以根据需要为相应的会话设置不同的模式。

MySQL操作模式会影响到SQL的语法和相应的SQL语句的校验。

### 1.1 设置SQL模式
默认情况下SQL的模式是```NO_ENGINE_SUBSTITUTION```。如果要在MySQL Server启动的时候就设置好相应的SQL模式的话，可以使用```--sql-mode=<modes>```命令行选项来进行设置，也可以在MySQL配置文件中通过```sql-mode=<modes>```来进行配置。```<modes>```是由一系列由空格分隔的不同的模式组成。如果要清除SQL模式，则只需要在启动时传递```--sql-mode=""```或者在配置文件中配置```sql-mode=""```。

如果需要在运行时更改SQL模式，那么可以设置```全局```与```会话``` sql_mode系统变量：
<pre>
SET GLOBAL sql_mode = 'modes';
SET SESSION sql_mode = 'modes';
</pre>
对于设置全局变量，需要有```SUPER```权限，并且会影响到之后连接的所有客户端。对于设置```会话```变量，则只会影响到当前客户端。每个客户端都可以改变该会话的sql_mode。

可以通过如下命令来获取当前```全局```及```会话```sql_mode:
<pre>
SELECT @@GLOBAL.sql_mode;
SELECT @@SESSION.sql_mode;
</pre>

### 1.2 最重要的SQL模式
SQL有很多模式，下面我们介绍几种常用的重要的SQL模式：

* ANSI: 该模式会改变相应的语法和操作行为，以使最接近标准的SQL。它是一种特殊的```组合模式```(combination modes)。

* STRICT_TRANS_TABLES: 假如一个值并不能插入到一个“事务表”中，那么中断该语句的执行。

* TRANDITIONAL： 使MySQL接近于传统的SQL数据库系统。简单的描述即为“在插入错误的值到一列时直接返回错误，而不是警告”。

## 2. MySQL字符集与校验集



<br />
<br />
**[参看]**:

1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)


<br />
<br />
<br />

