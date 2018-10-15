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

* 采用基于statement的复制时，INSERT...SELECT所需要的行级锁比```row-based replication```要更大

* Update语句可能需要扫描整个表(因为WHERE子句可能并没有使用索引），这样就会导致采用基于statement的复制比```row-based```复制需要更大的行级锁

* 针对InnoDB，使用```AUTO_INCREMENT```的```INSERT```语句会阻塞其他```非冲突```(nonconflicting)INSERT语句

* 对于更复杂的SQL语句，被复制到slave的SQL语句在行被更新或插入之前必须被再次评估(evaluated)。对于```row-based```复制来说，slave只需要修改受影响的行数据，并不需要执行整个SQL语句；

* 假如复制到slave上的SQL语句解析到相应的错误，特别是在复杂SQL语句的情况下，```statement-based```复制可能会随着时间的推移缓慢的增加受影响行的错误，即可能会不断的造成错误积累

* 在使用statement-based replication时，普通的存储函数在调用```NOW()```时一般可以相同的结果。但是如果是在调用存储过程中使用NOW()则可能会出现问题；

* Deterministic UDFs must be applied on the slaves.

* 在master及slave上表(table)定义必须要相同


### 1.3 row-based复制的优点

* 所有的更改都可以被复制。这是最安全的复制形式

* 该技术与大多数其他的数据库管理系统一致

* 对于如下语句在master上需要更少的行级锁，因此可以获得更高的并发性能
<pre>
1) INSERT...SELECT
2) 带有AUTO_INCREMENT子句的INSERT语句
3）带有WHERE条件的UPDATE或DELETE语句
</pre>

* 对于任何的```INSERT```、```UPDATE```或```DELETE```语句，slave都需要更少的行级锁

### 1.4 row-based复制的缺点
* row-based复制会有更多的数据需要被记录到日志中。如果要复制一个DML statement(比如UPDATE或DELETE语句），```statement-based```复制只需要将这些语句写入到binlog中；相反，```row-based```复制会将所有需要更改的行都写入到binlog中。假如前述DML语句修改了很多行的话，```row-based```复制则会产生更多的日志。此外，rollback语句也同样会产生较多的日志数据。这就意味着从backup处获取和重载数据也会花费较多的时间。另外，binlog会被锁住更长时间来写数据，这可能会导致并发问题。可以考虑使用```binlog_row_image=minimal```来降低对效率方面的影响。

* 对于会产生大量```BLOB```值的user defined functions(UDF)，采用row-based复制会花费更长的时间。

* 你并不能在slave上看到从master处接收到了何种statements，并且是如何执行的。然而，你可以通过使用```mysqlbinlog```工具来查看什么数据被改变了（需要加上```--base64-output=DECODE-ROWS```和```--verbose```选项）

* 对于使用```MyISAM```存储引擎的表来说，采用```row-based```复制时执行```INSERT```语句需要比```statement-based```复制更强的锁。这就意味着使用```row-based```复制时不支持在```MyISAM```上并发的插入数据

## 2. row-based日志和复制的用法
MySQL可以使用```statement-based```日志(SBL)、```row-based```日志(RBL)或者```mix-format```日志。具体使用哪一种格式的binlog会影响到日志的大小及效率。因此到底选择```row-based```复制(RBR)还是```statement-based```复制(SBR)取决于你的应用程序及环境。本章描述了使用```row-based```格式日志时的一些常见问题，然后再会讨论一些使用```row-based```复制的最佳实践。

* 针对临时表(temporary table)的row-based复制： 当使用```row-based```格式的复制时，并不会复制临时表。而当使用```mix```格式的复制时，针对临时表的```安全语句```(safe statement)都会采用statement-based格式来记录。

 使用```row-based```格式的日志时，临时表并不会被复制，并且也没有这个需要。另外，因为临时表只能够被创建该临时表的线程所访问，因此复制临时表几乎没有任何实际的意义。虽然```statement-based```复制能够复制临时表，但其实也没什么实际意义。

* 非事务表(nontransactional tables)的```row-based```日志及同步： 当有很多行数据受到影响，这一系列的更改会被分割成多个事件；当遇到事件提交，所有的这些事件都会被写到binlog中。当在slave上执行时，会在所有相关的表上加上锁，然后相应的行会被批量的修改（关于这种批量修改的效率，取决于表所采用的存储引擎）。

* binlog的大小及延迟： ```row-based```日志会将所有被修改的行写入到binlog中，因此对应的日志很可能会比较快的膨胀。这种迅速增长很可能会影响到slave的同步时间。在用户应用程序中应该要了解到这种潜在的延时。

* 读取binlog: ```mysqlbinlog```会使用```BINLOG```语法来显示```row-based```事件。会使用一个base-64编码的字符串来显示，因此具体代表的含义可能并不是那么明显。当在使用```mysqlbinlog```时加上```--base64-output=DECODE-ROWS```与```--verbose```选项时，则可以转换成比较便于人阅读的格式。当日志是以```row-based```格式记录时，在遇到数据库崩溃，需要从slave进行数据恢复时，你就可以采用此方法来进行。

* binlog执行错误及```slave_exec_mode```: 当slave_exec_mode为```IDEMPOTENT```时， 当遇到一些原始数据丢失导致```row-based```日志中的一些更改事件可能并不能执行成功，这时MySQL并不会产生相应的错误，复制也不会失败。这就意味着这些更新也许并不会在slave上生效，因此slave与master此时将不再同步了。当slave_exec_mode为```IDEMPOTENT```时，由于延迟及```row-based```非事务表的使用，这会使得主从之间的偏离变得更大。

* 不支持基于server-id的过滤

* 数据库级别(Database-level)的复制选项： 依据所采用的日志格式的不同（row-based/statement-based)，选项```--replicate-do-db```、```--replicate-ignore-db```与```--replicate-rewrite-db```可能会产生不一样的效果。因此，建议避免使用数据库级别的选项，转而使用表级别的选项```--replicate-do-table```与```--replicate-ignore-table```.

* row-based日志、非事务表、和已停止的slave: 当使用row-based日志时，假如在slave线程正在更新一个非事务表时，slave就被停止了，这种情况下slave数据库就可能会进入一种非一致性状态。由于这样的原因，建议使用事务性存储引擎（例如InnoDB)来对所有表做```row-based```复制。在关闭MySQL服务器之前，使用```STOP SLAVE```或者```STOP SLAVE SQL_THREAD```可以防止此类事件的发生。不管采用的是哪一种存储引擎或日志格式，都建议通过此方法来关闭MySQL Server。

## 3. 识别binlog中安全与非安全的statement
在MySQL复制中，所谓statementde ```安全性```就是指当使用```statement-based```格式的日志时，该statement所造成的影响能否被正确的复制。假如能够被正确的复制，则说明是安全的；否则，则被认为是不安全的。

一般来说，假如一条statement执行的结果是确定性的，则可以认为该statement是安全的，否则就认为是不安全的。然而对于许多非确定性的函数我们并不认为其是不安全的。另外对于执行结果为浮点数的数学函数，因为这一般依赖于计算机硬件，因此我们认为是不安全的。

下面我们简单介绍一下对于安全性与非安全性statement的处理：依据该statement本身的安全性和所采用的binlog格式，同一个statement可能会做不同的处理：

* 当使用```row-based```格式日志时，一般来说所谓的```安全```与```非安全```statement是没有差别的

* 当使用```mix```格式日志时，针对哪些```非安全```statement则会采用row-based格式的日志；对于哪些被认为是```安全```的statement，则会采用statement-based格式的日志；

* 当使用```statement-based```格式的日志时，对于哪些被认为是```非安全```的statement，在记录日志时会添加上相应的警告信息，而对于哪些被认为是```安全```的statement则进行直接的日志记录。








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

