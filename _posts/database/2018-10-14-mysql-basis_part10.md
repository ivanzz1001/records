---
layout: post
title: mysql主从复制(三)
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
如下的一些statement被认为是不安全的：

* 包含系统函数的statement: 因为这些函数在不同的主机上可能会返回不同的值。这些函数主要包括
<pre>
FOUND_ROWS()、GET_LOCK()、IS_FREE_LOCK()、IS_USED_LOCK()、LOAD_FILE()、MASTER_POS_WAIT()、
PASSWORD()、RAND()、RELEASE_LOCK()、ROW_COUNT()、SESSION_USER()、SLEEP()、SYSDATE()、
SYSTEM_USER()、USER()、UUID()、UUID_SHORT()
</pre>

* 有一些```非确定性```函数是```安全的```。尽管这些函数是非确定性的，但它们在日志记录和复制时，我们认为是安全的。这些函数主要包括
<pre>
CONNECTION_ID()、CURDATE()、CURRENT_DATE()、CURRENT_TIME()、CURRENT_TIMESTAMP()、CURTIME()、LAST_INSERT_ID()、
LOCALTIME()、LOCALTIMESTAMP()、NOW()、UNIX_TIMESTAMP()、UTC_DATE()、UTC_TIME()、UTC_TIMESTAMP()
</pre>


* 对系统变量的引用： 使用```statement-based```复制时，对于大部分系统变量来说都不能被正确的复制

* UDFS: 这里因为我们并不能控制```UDFs```的具体实现，因此这里我们认为是不安全的

* 自动触发或更新带有```AUTO_INCREMENT```列表： 这一般在复制时认为是不安全的，因为有可能在master与slave上执行更新的顺序会不一样。另外，对于列为```AUTO_INCREMENT```的复合主键，假如该列并不是复合主键的第一列，那么在执行```INSERT```插入时我们也认为是不安全的。

* INSERT DELAYED语句： 该statement被认为是不安全的，这是因为在执行插入时中间有可能会并发的执行其他的语句。

* 在复合primary key或者unique key的表上执行```INSERT ... ON DUPLICATE KEY UPDATE```被认为是不安全的：因为这取决于存储引擎按什么样的顺序来检索这些key，不同的存储引擎检索顺序有可能是不一样的，因此最后更新的行也有可能会不一样。

* 使用带LIMIT字段的更新： 因为获取行数据的顺序是未定的，因此我们认为这种更新是不安全的

* 访问或引用log表： 因为系统日志表在master与slave上有可能是不同的

* LOAD DATA INFILE语句： 该语句被认为是不安全的，假如```binlog_format=mix```时该语句会在日志中以```row-based```格式来记录。
<pre>
注意： 当binlog_format=statements时，LOAD DATA INFILE并不会产生相应的警告信息，这一点与其他的非安全语句不同
</pre>



## 4. 基于GTID的复制
本章会简单介绍一下基于GTID(Global Transaction Identifiers)的复制，GTID是在```MySQL5.6.5```版本开始引入的。当使用GTIDs时，每一个在原始服务器上提交的事件都可以被标识及跟踪，并将该提交的事件应用到slave上。这就意味着当使用GTIDs的时候，并没有必要参看binlog文件及相应的位置偏移就可以将相应的事件直接同步到slave，这极大的简化了相应任务的执行。因为```GTID-based```复制是完全```transaction-based```，因此可以很容易就可以知道主从是否一致； 只要相应的事件已经提交到了master，则肯定也提交到了slave，因此主从之间的一致性是可以保证的。你可以使用带GTIDs的```statement-based```复制或者```row-based```复制，但是我们建议使用```row-based```复制。

本章我们主要会介绍一下如下方面的内容：

* GTIDs是被定义和创建的，在MySQL Server中是如何展示的

* 建立和启动基于GTID复制(GTID-based replication)的步骤

* 使用GTIDs时，如何增加新的复制服务器

* 使用基于GTIDs的复制时的相关限制

* 禁用GTIDs的相关步骤


### 4.1 GTIDs概要
一个```global transaction identifier```(GTID)就是一个唯一的标识符，当每一个事务提交到master时，就会创建一个GTID，并将该GTID与该事件相关联。该标识符不仅仅在master上是唯一的，而且在整个复制集群中都是唯一的。GTID与事务之间是一一对应的关系。

一个GTID是由两部分组成，中间以冒号分割：
<pre>
GTID = source_id:transaction_id
</pre>

其中```source_id```用于标识产生该标识的源服务器。通常情况下，会采用MySQL服务器的```server_uuid```来标识source_id。```transaction_id```是一个用于标示提交到源服务器事务的序号，其通常是按提交顺序来决定的； 例如，第一个被提交的事务其```transaction_id```为1，那么第10个被提交到该服务器的事务其```transaction_id```就为10。注意，一个事务的```transaction_id```是不可能为0的。例如：第23个提交到server_uuid为```3E11FA47-71CA-11E1-9E33-C80AA9429562```的MySQL服务器的事务，其GTID如下
<pre>
3E11FA47-71CA-11E1-9E33-C80AA9429562:23
</pre>

在一些输出（如```SHOW SLAVE STATUS```)及binlog中都以这样的格式来表示GTIDs。我们也可以通过如下的命令:
<pre>
mysqlbinlog --base64-output=DECODE-ROWS
</pre>
来查看日志文件中的GTIDs。

另外，对于在执行如```SHOW MASTER STATUS```或者```SHOW SLAVE STATUS```命令时，相应的输出会将来自于相同server的GTIDs合并到一起来显示，例如：
<pre>
3E11FA47-71CA-11E1-9E33-C80AA9429562:1-5
</pre>
注意，从MySQL5.6.6版本开始，我们也会采用该格式来指定```START SLAVE```选项```SQL_BEFORE_GTIDS```或```SQL_AFTER_GTID```的参数。

<br />
**1) GTID SETs**

```GTID```集合的表示方法如下：
{% highlight string %}
gtid_set:
	uuid_set [, uuid_set] ...
	| ''

uuid_set:
	uuid:interval[:interval]...

uuid:
	hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh

h:
	[0-9|A-F]

interval:
	n[-n]

	(n >= 1)
{% endhighlight %}

**2) GTID的生成及生命周期**

```GTID```的生成及生命周期包含如下一些步骤：

* 1) 事务是在master上被提交和执行的： 一个事务的GTID是通过使用master的server_uuid和当前未被使用的最小非0事务序列号组成的。GTID会被写入到binlog中，后面跟着的就是该事务本身

* 2） 在binlog中的数据被传送到slave，并存储到slave的relaylog后，slave就会读取该事务的GTID并将该值写入到```gtid_next```系统变量中。这就告诉了slave下一个要记录到日志的事务所采用的GTID
<pre>
说明： slave在会话上下文中设置gtid_next
</pre>

* 3) slave会通过其自身的binlog检查并确保该GTID并未被使用来记录事务。当且仅当该GTID未被使用，slave就会将该GTID写入到自己的binlog中并执行该事务。在处理事务之前，首先检查该事务对应的GTID，这样就可以保证该事务以前并未在slave上执行过，而且还可以确保并没有其他的session读取了该事务。换句话说，多个客户端不允许并发执行同一个事务。

* 4） 因为gtid_next不为空(empty)，因此slave本身并不会尝试为该事务产生一个GTID。

### 4.2 使用GTID来建立主从复制
本节会描述一下如何配置和启动```GTID-based```复制。这里介绍的是```cold start```情形下的相应步骤。所谓```cold start```就是指第一次启动replication master（也包括停止后的重新启动）。对于一个在运行过程中的master，如果要启动基于GTID的复制，请参看下一节```使用GTID来进行系统恢复及水平扩展```。

如下介绍最简单的基于GTID复制场景（即只有一个master和一个slave）下的相关步骤：

1) 假如replication当前已经运行，我们可以将master与slave都设置为只读状态，通过这样来完成主从之间的同步；

2） 停止master与slave服务器

3） 重新启动master与slave服务器，并同时开启GTID、binlog、以及```slave update logging```功能，禁用带GTID的```statement-based```复制功能。另外，master与slave服务器都只能以```read-only```模式启动，并且不要开启slave的SQL线程和IO线程。在本章后面的例子中我们会描述```mysqld```启动时的一些必要选项。

4） 为slave指定master作为其复制源，并且使用```auto-position```模式。在本节的后面我们也会给出相应的例子

5） 建立一个新的备份。原先旧的不带GTID功能的binlog在开启GTID功能后并不能被使用了，因此在这里我们需要重新建立备份。

6） 启动slave，然后在master与slave上禁用```read-only```模式，这样就可以使得它们可以被更新

<br />
在如下的例子中，我们已经采用```经典```的```file-based```复制协议（statement-based、row-based、mix-based都是基于文件的复制协议）建立起了主从。注： 下面的操作大部分都需要```SUPER```权限，因此建议使用root权限来进行操作

**1）同步服务器**

首先使master与slave都处于```read-only```状态，我们可以通过在master与slave上执行如下命令： 
{% highlight string %}
mysql> SET @@global.read_only = ON;ze
{% endhighlight %}
然后等待所有处于```ongoing```状态的事务进行提交或回滚。然后，等待slave同步上master。这里```确保```我们在进行下一步操作之前完成MySQL slave对master的同步。

假如你采用binlog来做除replication之外的其他的事情，例如用来做某个点的即时备份或恢复，请等待所有这些不带```GTID```的binlog都处理完成。
<pre>
注意：对于具有不带GTID功能事务功能的binlog日志，我们并不能将这些日志用在带GTID功能的服务器上。因此，在处理之前，必须
确保在主从复制拓扑结构的任何地方都没有这种日志。
</pre>

**2） 停止master与slave服务器**

使用```mysqladmin```工具停止master与slave服务器：
<pre>
# mysqladmin -uusername -p shutdown
</pre>

**3) 重启master与slave服务器，同时启用GTIDs功能** 

为了使binlog拥有```global transaction identifiers```(gtid)，我们需要在启动master与slave时：使用GTID模式，启用binlog功能，启用slave update logging功能，禁用不安全的statement-based复制功能。另外，你需要以```read-only```模式来启动，以防止相应的服务器再做更新操作。这就意味着在启动mysql时至少需要包含如下选项：
{% highlight string %}
//master
# mysqld_safe --gtid_mode=ON --log-bin --log-slave-updates --enforce-gtid-consistency &

//slave
# mysqld_safe --gtid_mode=ON --log-bin --log-slave-updates --enforce-gtid-consistency --skip-slave-start &
{% endhighlight %}
注意： 上面需要根据自身情况，以决定是否要添加额外的启动选项


**4）配置slave指向的master**

我们需要配置slave以master作为复制源，并使用```GTID-based auto-position```以替代```file-based position```。在master上执行```CHANGE MASTER TO```语句，并使用```MASTER_AUTO_POSITION```选项以告诉slave每一个事务都会由GTID标识：
{% highlight string %}
mysql> CHANGE MASTER TO
> MASTER_HOST = host,
> MASTER_PORT = port,
> MASTER_USER = user,
> MASTER_PASSWORD = password,
> MASTER_AUTO_POSITION = 1;
{% endhighlight %}
说明： 假如你在上面```步骤1)```已经进行了这些配置，则可以忽略这一步的修改。

在这一步骤中，我们并没有指定```MASTER_LOG_FILE```选项，也没有指定```MASTER_LOG_POS```选项，因为我们这里指定了```MASTER_AUTO_POSITION=1```，因此不能指定这些选项。如果我们同时指定，则会产生相应的错误。

**5） 重新建立新的backup**

那些在你启用```GTID```功能以前所做的备份在当前已经不能再被使用。你必须在这一时刻重新创建备份。例如，你可以在你需要备份的服务器上执行```FLUSH LOGS```。然后你就可以显式的来执行备份。

**6） 启动slave，并禁止read-only模式**

通过如下的命令来启动slave:
{% highlight string %}
mysql> START SLAVE;
{% endhighlight %}
然后在master上执行如下命令以禁用```read_only```:
{% highlight string %}
mysql> SET @@global.read_only = OFF;
{% endhighlight %}
到此```GTID-based```复制就已经开始运行了，你就可以在master开始进行相应的读写操作了。

### 4.3 使用GTID来进行系统恢复及水平扩展

从```MySQL5.6.9```版本起，当使用带GTID功能的复制时，我们可以有多种方式来水平扩充slave，或者在master失败的情况下将一个slave提升为master。在本节，我们主要讨论如下4个方面的内容：

* 简单复制(Simple replication)

* 拷贝数据或事务到slave

* 注入空事务（insjecting empty transactions)

* 排除gtid_purged的事务

在MySQL的主从复制中添加```GTID```的主要目的是为了简化复制数据流(replication data flow)的管理以及对系统错误的恢复。在数据库更新的时候GTIDs扮演了一个核心角色： 服务器会自动的跳过那些拥有gtid且以前被处理过的事务。这对于```自动化的复制位置```以及正确的错误恢复来说至关重要。

组成一个事务的```标识```(identifier)和```事件集合```都会被记录到binlog中。这在从一个已有数据的服务器向另一个新的服务器复制数据时存在一些挑战。为了在新的服务器上重新产生这些标识(identifier)，需要将这些标识从老的服务器拷贝到新的服务器，并且保证标识与实际事件(event)的一一对应， 而这是在系统需要做恢复或切换的情况下，要求迅速的将某一个slave提升为master时所必要的。

**1） 简单复制**

这是最简单的方法来在新的服务器上重新产生标识符(identifier)和事务(transaction): 你只需要简单的将该新的服务器变成```拥有完整历史执行记录的master```的slave即可。并且在master与slave上同时启用```GTID```功能。请参看上一节```使用GTID来建立主从复制```.

一旦复制被启动，新的服务器就可以从master拷贝整个binlog，这样就可以获得所有的GTID信息。

本方法是最简单和有效的，但是需要slave从master读取binlog。但这在有的时候，slave服务器需要花费大量的时间来进行同步，因此该方法并不适合快速的错误恢复或从backup中恢复。本节后面会说明如何避免通过从master拷贝binlog文件数据来拉取整个执行的历史记录到新的服务器。

**2) 拷贝数据和事务到slave**

重新执行binlog中的所有事务的历史记录很可能是一个耗时的工作，这是在建立新的slave的主要瓶颈。为了解决需要同步整个历史记录这一问题，需要将master上的```数据快照```、binlog、以及```全局事务信息```导入到slave中。在slave开始正式进行同步之前(IO线程与SQL线程工作），slave会通过binlog处理完遗留的事务。

此种方法有多种变体，这些变体之间的主要不同表现在: 数据dump方式、binlog中的事务传送到slave的方式。参看下表：

![db-mysql](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_gtid.jpg)

该方法通常可以使得新的slave服务器马上就可以工作；只有那些在我们创建snapshot或者dump文件时提交的事务，slave才需要进行同步。这就意味着slave服务器的可用性也并不是瞬时连续的，而是需要一个相对短的时间来完成slave与master的同步。

通常提前拷贝binlog文件到slave会比slave直接同步master的整个事务执行记录耗时更少。然而考虑到文件大小及其他一些方面的原因，通过这样移动文件到slave有时也并不是那么优雅。下面剩余的两种方法使用其他的一些方式来将master上的历史transaction传送到新的slave上。

**3）注入空事务(empty transactions)**

master的全局(global)变量```gtid_executed```包含了所有在master上执行过的事务集。这里我们不必像以前那样在添加新的slave服务器时需要创建data snapshot并且拷贝binlog文件，只需要用到master上```gtid_executed```的相关内容。在添加新的slave到复制链之前，我们获取到master的```gtid_executed```，然后需要在新添加的server上为```gtid_executed```集中的每一个GTID提交一个```空事务```(empty transaction)，例如：
{% highlight string %}
SET GTID_NEXT='aaa-bbb-ccc-ddd:N';
BEGIN;
COMMIT;
SET GTID_NEXT='AUTOMATIC';
{% endhighlight %}
一旦所有的```事务标识```(GTID)都通过空事务被恢复，那么你必须```flush```并```purge```slave的binlog。例如：
{% highlight string %}
FLUSH LOGS;
PURGE BINARY LOGS TO 'master-bin.00000N';
{% endhighlight %}
我们进行上面操作的主要目的是为了防止该新建的服务器在后序提升为master时产生错误的transaction，从而干扰整个复制流（这里```FLUSH LOGS```语句会强制创建一个新的binlog文件；```PURGE BINARY LOGS```会去除掉所有空事务,但是保留所对应的identifier）。

该方法创建了一个新的服务器，本质上其实是一个snapshot，但是现在其已经可以成为master了，因为其binlog历史记录已经覆盖了master复制流（即其已经追上了master)，本方法其实有些类似于我们下面介绍的第4种方法。

**4) 排除gtid_purged的事务**

master的全局(global)```gtid_purged```变量包含了所有从master binlog中所```purged```事务。正如我们前面所讨论的(方法3： 注入空事务)，你可以获取```gtid_executed```相关内容。而与前面方法不同的是，这里我们并不需要提交空事务(empty transaction)或者执行```PURGE BINARY LOGS```，我们可以基于```gtid_executed```的值来直接设置slave的```gtid_purged```。
<pre>
注意： 在MySQL5.6.9之前，gtid_purged并不能被设置。
</pre>

如前面介绍的```注入空事务```那样，本方法所创建的server其实也是一个snapshot，但是现在其也可以成为master了，因为其binlog历史记录已经覆盖了master复制流（即其已经追上了master)。


### 4.4 基于GTID复制的限制
因为基于```GTID```的复制依赖于```transaction```，因此当使用该类型复制时MySQL的有一些特性是不支持的。本节主要会介绍一下基于```GTID```复制的一些限制。

**1） 涉及到非事务存储引擎(nontransactional storage engines)的更新**

当使用GTID时，对于那些非事务存储引擎表(例如MyISAM)的更新不能像事务存储引擎表(例如InnoDB)的更新那样来处理。限制主要是因为假如在同一个事务中更新```非事务存储引擎表```的同时夹杂了更新```事务存储引擎表```，这样就可能会导致针对同一个事务会有多个```GTID```被指定。这种情况还可能发生在master与slave对同一个表采用不同的存储引擎的情况下： 其中一个支持事务，而另一个不支持事务

在前面提到的任何一种情况，GTID与transaction的一一对应关系遭到了破坏，因此```GTID-based```复制可能并不能正常的工作。

**2） CREATE TABLE ... SELECT语句**

对于```statement-based```复制来说，```CREATE TABLE ... SELECT```是不安全的。当使用```row-based```复制时 ，该语句实际上是会被记录为```两个```单独的事件： 一个是用于创建表，另一个用于将数据从原表中插入到新表。当该语句在一个事务中被执行时，在有一些情况下这两个事件可能会接收到同一个```transaction identifier```，这就意味着第一个插入事件会被slave忽略掉。因此，```CREATE TABLE ... SELECT```在使用```GTID-based```复制时是不支持的。

**3） 临时表**

当使用GTID时（即在MySQL启动时指定了```--enforce-gtid-consistency```选项）不支持```CREATE TEMPORARY TABLE```与```DROP TEMPORARY TABLE```处于一个事务中。但是假如在使用GTID的情况下，autocommit设置为1，并且不在事务中则可以使用这些语句。


**4） 阻止执行不支持的语句**

为了阻止执行那些会导致```GTID-based```复制失败的语句，当启用GTID时，所有的服务器都必须以```--enforce-gtid-consistency```选项启动。这会导致前面所介绍的一些不支持的语句在执行时产生错误。

当使用GTID时，不支持```sql_slave_skip_counter```。假如你需要跳过事务，请使用master的```gtid_executed```值。

**5) GTID模式与mysqldump**

从MySQL5.6.9版本起，可以将mysqldump导出的dump文件导入到以```GTID```模式运行的MySQL Server中。


**6） GTID模式以及mysql_upgrade**

在```MySQL5.6.7```版本之前，```mysql_upgrade```在以```--write-binlog=OFF```选项运行时是不能够连接上一个正在以```GTID```模式运行的MySQL Server的。否则的话，MySQL Server必须要重启，并指定```--gtid-mode=OFF```选项（即关闭GTID功能），然后再以```--gtid_mode=ON```选项重启```mysql_upgrade```。而从```MySQL5.6.7```版本开始，```mysql_upgrade```默认是以```--write-binlog=OFF```启动。



### 4.5 禁止GTID事务
假如你想要从支持```GTID```功能的```MySQL5.6```版本降级到不支持```GTID```功能的其他MySQL版本，你必须在降级之前参照如下的步骤禁止```GTID```功能。在MySQL5.6版本中，为了禁止GTID，你必须使相应的server处于offline状态。


**1)** 在所有slave上，通过运行如下的命令禁用auto-position
{% highlight string %}
mysql> STOP SLAVE;

mysql> CHANGE MASTER TO MASTER_AUTO_POSITION = 0, MASTER_LOG_FILE = file, \
MASTER_LOG_POS = position;

mysql> START SLAVE;
{% endhighlight %}

**2)** 在所有服务器上（master及slave），通过运行如下的命令停止数据更新
{% highlight string %}
SET @@GLOBAL.READ_ONLY = ON;
{% endhighlight %}

**3)** 等待所有正在进行中的```事务```提交或回滚。然后，等待一段时间，确保任何存在于binlog中的事务都被复制到了所有的slave中。注意： 在进行下一步之前请确保所有的更新都已经处理完成。

假如你使用binlog来做除```复制```(replication)以外的其他事情，例如用于备份或数据恢复，那么请等到所有这些任务完成以不再需要这些带有GTID的老的binlog时为止。理想状态下，等待服务器```purge```所有的binlog，以及所有的已存在的backup操作过期。
<pre>
注意： 对于那些包含GTID事务的binlog文件，其不能被用于GTID功能被禁用的服务器上。因此在处理之前，必须确保
在整个拓扑结构中已经不存在GTID事务
</pre>

**4）** 在所有的server上，通过```mysqladmin```工具关闭MySQL
<pre>
# mysqladmin -uusername -p shutdown
</pre>

**5)** 在所有server的配置文件```my.cnf```中设置如下两个选项
<pre>
gtid-mode=OFF
enforce_gtid_consistency=OFF
</pre>

**6)** 以```read-only```模式重启mysql（可以使用```mysqld_safe```或者```mysqld```启动脚本，并指定```--read_only=ON```选项）。以read-only模式启动server，以防止可能的更新操作。

**7）** 此时，再重新备份数据库以供日后使用。以前已存在的带GTID功能的备份在当前禁用```GTID```功能的情况下是不能再使用了。例如，你可以创建备份的机器上执行```FLUSH LOGS```。然后你就可以显示的手动执行备份或者等待你配置的定时备份完成。

**8）** 在每一个server上，通过运行如下语句重新启用```update```功能
{% highlight string %}
SET @@GLOBAL.READ_ONLY = OFF;
{% endhighlight %}

到此为止，假如你想要降级到更老版本MySQL，就可以采用通常的方法来开始降级了。











<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)

5. [MySQL 设置基于GTID的复制](http://blog.51cto.com/13540167/2086045)

6. [MySQL 在线开启/关闭GTID](https://blog.csdn.net/jslink_l/article/details/54574066)


<br />
<br />
<br />

