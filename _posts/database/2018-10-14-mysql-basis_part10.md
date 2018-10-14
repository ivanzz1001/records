---
layout: post
title: mysql数据库基础（十）
tags:
- database
categories: database
description: mysql数据库基础
---


本章我们主要介绍一下复制格式(Replication Formats).

MySQL的主从复制实现的主要原理是： master服务器记录相应的更新信息到binlog，slave服务器从master服务器读取binlog并在本地重新执行binlog中事件，从而使得master与slave达到一致的状态。根据事件的类型不同，master会以不同的格式将日志写到binlog中。主从复制过程中，也采用与master binlog中事件相对应的格式来进行处理，binlog格式与replication所采用的格式的之间的映射如下：

* 当使用```statement-based``` binlog时，master会将对应的SQL语句写到binlog中。此种情况下```主从复制```是通过在slave上执行相同的SQL语句来进行的。这种以标准MySQL语法的binlog来进行的复制，被称为```statement-based replication```(SBR)。MySQL5.1.4之前一般均采用此格式。

* 当使用```row-based```的日志格式时，master在binlog中记录相应的事件用于表明MySQL表的每一行是如何改变的。采用此种格式时，主从复制是通过slave从master拷贝表中每一行的更改。这被称为```row-base replication```(RBR)。

* 你也可以配置MySQL使用混合的日志格式(SBR与RBR)，这被称为```mix-format logging```。当使用```混合格式日志```时，在默认情况下是用```statement-based```日志，对于一些特定的SQL，会根据当前表的存储引擎的情况自动的转换为```row-based```日志。使用混合日志格式的复制通常被称为MBR。

<br />
MySQL5.6版本，默认采用的是statement-based日志格式。

当使用```MIX```格式日志时，binlog具体使用的是哪一种依赖于表的存储引擎以及所执行的SQL语句。MySQL所采用的日志格式是受```binlog_format```系统变量所控制的。该变量可以在session层级设定也可以在global层级设定。如果要设置全局变量，你可以在mysql的配置文件的```[mysqld]```段落设置```binlog-format```为```ROW```或者```STATEMENT```或```MIXED```。通常情况下我们不要在MySQL运行过程中更改binlog的格式。




<!-- more -->

## 1. Statement-based与Row-based复制优缺点
每一种binlog格式都有各自的优点与缺点。对于大部分用户来说，采用```MIX```格式能够获得最好的数据完整性和性能。然而，假如你想充分的利用```statement-based```日志格式或```row-based```日志格式的相关特性来处理一些任务的话，本章后面会教详细的说明各自的优缺点。

* statement-based复制优点

* statement-base复制缺点

* row-based复制优点

* row-based复制缺点

### 1.1 statement-based复制优点
采用基于statement的复制具有如下优点：

* 技术成熟

* 写入到log文件的数据较少。当一次性更新或删除多行数据的时候，存储的日志文件所占用的空间会很少。则也意味着slave从master读取相应的日志数据会更快。

* log文件包含数据更改的SQL语句，因此可以在后边进行审计

### 1.2 statement-based复制缺点

**1）对于SBR复制来说，Statements是不安全的**

在使用statement-based复制时，并不是所有修改数据（比如INSERT、DELETE、UPDATE与REPLACE语句）的语句都可以被复制。任何非决定性的行为都难于被复制。例如下面的一些DML(Data Modification Language)语句：

* 依赖于UDF(user-defined functions)或存储过程的非决定性语句。因为这些语句返回的结果依赖于UDF或存储过程或相应的传入参数。

* 使用带```LIMIT```子句的```DELETE```或```UPDATE```时，如果没有采用```ORDER BY```做进一步限制的话，这样的语句处理结果可能不唯一。

* 使用如下函数的语句在采用```statement-based```复制时不能被正确的复制
<pre>
LOAD_FILE()

UUID(),UUID_SHORT()

USER()

FOUND_ROWS()

SYSDATE()(注：除非master与slave启动时添加了--sysdate-is-now选项)

GET_LOCK()

IS_FREE_LOCK()

IS_USED_LOCK()

MASTER_POS_WAIT()

RAND()

RELEASE_LOCK()

SLEEP()

VERSION()
</pre>
对于所有其他的函数一般都能够在采用```statement-based```复制时被正确的复制，则包括```NOW()```这样的函数。对于那些使用```statement-base replication```不能被正确复制的函数通常都会在记录日志时增加如下的警告信息：
{% highlight string %}
[Warning] Statement is not safe to log in statement format.
{% endhighlight %}







<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)

5. [](https://blog.csdn.net/ahzxj2012/article/details/54017969)


<br />
<br />
<br />

