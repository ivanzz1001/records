---
layout: post
title: mysql主从复制(一)
tags:
- database
categories: database
description: mysql数据库基础
---


复制使得一个MySQL server(master)的数据可以被复制到一个或多个其他的MySQL Server中（slave)。复制默认情况下是异步进行的，因此从服务器(Slaves)不需要永久的连接上主服务器(master)以接受更新。依据相应的配置，你可以复制所有的数据库，也可以复制所指定的数据库，甚至是指定数据库中的某一个表。使用MySQL主从复制有如下优势：

* 集群化解决方案 -- 可以将负载分摊到众多的从服务器以提高性能。在这样的环境下，所有的写操作和更新操作都必须在主服务器上，而读一般在一个或多个从服务器上。

* 提高数据的安全性 -- 因为数据会被复制到从服务器上，并且从服务器可以暂停复制进程，然后我们就可以在不中断主服务器的情况下对从服务器执行```backup```操作。


* 统计分析 --- 即时的数据可以在主服务器上被创建，我们可以在从服务器上对数据进行分析而并不影响主服务器(master)的性能

* 远距离数据分发 

MySQL的复制特性允许支持单向的异步复制： 一个MySQL Server作为主服务器，另外可以有一个或多个作为从服务器。这与```NDB集群```的同步复制是相反的。

可以有很多种方式来建立两个服务器之间的同步，但是最好的方式取决于数据的呈现方式以及所使用的存储引擎。有两种核心的复制格式： 一种是Statement Based Replication(SBR)，会复制整个SQL语句； 另一种是Row Based Replication(RBR),只会复制所改变的数据行。当然你可以可以使用第三种方式：Mixed Based Replication(MBR)。关于不同的复制模式，我们后面会再进行详细介绍。

从MySQL5.6.5版本起(包括5.6.5版本)，支持基于全局事务标识(Global Transaction identifiers, GTIDs)的```事务复制```。当使用这种类型复制的时候，并不需要直接的使用日志文件，这极大的简化了很多常见的复制任务。因为采用```GTIDs```复制本身就是事务性质的，提交到master上的事务会同步应用到slave上。



<!-- more -->

## 1. MySQL复制的配置
MySQL不同服务器之间的复制是基于二进制日志（binary logging)机制来完成的。MySQL master实例将对数据的更新和修改作为```events```写入到二进制日志文件中。该二进制日志文件根据数据库记录到的修改以不同的日志格式来进行存储。MySQL slave服务器会被配置为从Master读取二进制日志文件，然后在本地执行该日志文件所记录的数据库操作事件。

在本场景下，MySQL master处于```dumb```状态。一旦binary logging功能启用后，所有的语句都会记录到二进制日志文件中。每一个slave都会接受master日志的一份完整拷贝，由从服务器决定需要执行日志中的哪些事件。假如从服务器没有进行配置的话，则二进制日志文件中的所有事件均会在Slave上执行。假如有需要，你可以配置slave只执行某些数据库或表上的事件（注： 对于master服务器，所有的日志均会被记录，并不能选择只记录其中的某一些）。

每一个Slave都会记录当前所读日志文件的信息： 所读日志文件的名称、以及读取偏移量。这就意味着可以有多个从服务器同时连接到master，然后执行该日志文件中的不同部分。因为是由从服务器控制着读取进程，因此不同的slave服务器连接上master或者断开与master的连接均不会影响master服务器的操作。

在进行master-slave MySQL服务器配置时，每一个服务器都必须要有一个唯一ID(通过```server-id```选项来进行配置），另外每一个从服务器都必须配置master服务器的host name，日志文件名称，以及日志文件偏移量。这可以在Slave机器上通过```CHANGE MASTER TO```来进行修改。配置完成后相应的配置信息会被记录到slave的相应目录，这可能是一个文件或一个数据表。


本节我们主要会介绍```如何建立和配置MySQL复制所需要的环境```,包括创建一个新的复制环境的step-by-step操作指令。主要包括以下内容：

* MySQL主从复制配置相关手册

* MySQL日志记录格式。主要会介绍```statement based replication (SBR)``` 和 ```row-based replication (RBR)```两种。而第三种混合格式MBR会同时结合前两种的优点。

* 详细介绍不同配置选项和变量对复制所产生的影响

* MySQL复制进程的管理及监控（一般来说，一旦主从复制建立好之后只需少量的管理及监控）


### 1.1 建立MySQL主从复制
本章会介绍如何建立一个完整的MySQL Server复制。有很多种不同的方法来建立MySQL Server之间的复制，具体要采用哪一种方式取决于master数据库中是否已有数据以及你的习惯。

在建立MySQL Server复制的时候，有一些基本的任务要完成：

* 在Master主机上，你必须启用```二进制日志```(binary logging)，并且配置一个唯一的```server ID```。这可能需要重启MySQL Server服务器

* 在每一个你需要连接Master的Slave主机上，也需要配置一个唯一的```server ID```。这可能需要重启MySQL Server服务器

* （可选）在master上创建一个单独的用户，这样slave在复制读取master的二进制日志时，就可以使用该用户身份完成与Master的认证

* 在创建一个```数据快照```(data snapshot)或者启动复制进程之前，你需要在master上记录二进制日志的位置(position)。你需要通过这些信息来配置从机(slave)，这样slave就知道从何处开始执行二进制日志文件中的事件。

* 假如master主机上已经拥有数据，并且你想要将数据同步到```从机```(slave)上，那么你需要创建一个```数据快照```(data snapshot)。有多种不同的方法来创建数据库快照，具体要采用哪一种取决于数据库的大小以及数据库文件的存放位置。你可以使用```mysqldump```工具或者通过直接拷贝文件来创建数据库快照。

* 配置slave与master连接， 比如master的主机名(host name)，登录信息，以及二进制日志文件的名称及偏移。

除去上面的基本配置以外，需要根据自身场景来配置MySQL主从复制：

* 以全新的方式建立master与slave之间的数据复制（master与slave主机上均没有历史数据）

* 从一个已存在的MySQL Server来建立一个新的master

* 添加slave到一个已存在的复制环境中

这里我们在介绍管理MySQL复制服务器之前，会先介绍如下三个部分的内容:

* 控制Master服务器的SQL语句

* 控制Slave服务器的SQL语句

* 复制和二进制日志选项及变量



## 2. 控制Master服务器的SQL语句
在本章我们会介绍管理MySQL master服务器的SQL语句。我们先介绍如下几个```SHOW```语句的使用：

**1) SHOW BINARY LOGS**

本语句用于列出MySQL Server上的二进制日志文件(binary log)。
{% highlight string %}
mysql> SHOW BINARY LOGS;
ERROR 1381 (HY000): You are not using binary logging
{% endhighlight %}
上面显示我们当前并未启用```二进制日志```功能。注意执行此命令需要有超级用户权限或者```REPLICATION CLIENT```权限。

**2) SHOW BINLOG EVENTS**

语法如下：
{% highlight string %}
SHOW BINLOG EVENTS
	[IN 'log_name']
	[FROM pos]
	[LIMIT [offset,] row_count]
{% endhighlight %}
用于显示一个二进制日志文件中的事件。假如你并未指定```log_name```的话，则会显示第一个二进制日志文件。```LIMIT```子句与SELECT语法中的LIMIT相同。
<pre>
注： 如果我们在使用SHOW BINLOG EVENTS时不添加LIMIT子句的话，则可能会耗费大量的时间及资源，因为SQL服务器会返回给客户端二进制日志文件中的所有内容（这
包括SQL Server修改数据的所有SQL语句）。另外，这里我们介绍一个工具mysqlbinlog，我们可以使用该工具来将二进制日志文件保存为一个文本文件，后续我们就可以
对该文本文件进行检查及分析。 
</pre>
下面我们执行上述语句：
{% highlight string %}
mysql> SHOW BINLOG EVENTS LIMIT 10;
Empty set (0.00 sec)
{% endhighlight %}

```SHOW BINLOG EVENTS```会显示二进制日志文件中每一个事件的如下字段：

* Log_name: 日志文件的名称

* Pos： 事件发生的位置

* Event_type: 事件的类型

* Server_id: 产生事件的MySQL Server ID。

* End_log_pos: 下一个事件的起始位置，等于```Pos```+sizeof(event)

* Info: 关于该事件类型的更详细的信息。信息的格式取决于事件的类型

**3） SHOW MASTER STATUS**

本语句用于显示master主机的```二进制日志```文件的状态信息。执行本语句时需要```SUPER```或者```REPLICATION CLIENT```权限。参看如下示例：
{% highlight string %}
mysql> SHOW MASTER STATUS\G
*************************** 1. row ***************************
File: master-bin.000002
Position: 1307
Binlog_Do_DB: test
Binlog_Ignore_DB: manual, mysql
Executed_Gtid_Set: 3E11FA47-71CA-11E1-9E33-C80AA9429562:1-5
1 row in set (0.00 sec)
{% endhighlight %}
当使用全局的```transaction IDs```时，该列表明在Master上已经执行了```GTID```所表示的事务集。这与```gtid_executed```系统变量的值是一样的。

**4) SHOW SLAVE HOSTS**

本语句用于列出注册在对应master下的slave主机列表。注意```SHOW SLAVE HOSTS```命令应该在master机器上执行。该语句会列出连接到master上的所有从机信息。例如：
{% highlight string %}
mysql> SHOW SLAVE HOSTS;
+------------+-----------+------+-----------+--------------------------------------+
| Server_id | Host | Port | Master_id | Slave_UUID |
+------------+-----------+------+-----------+--------------------------------------+
| 192168010 | iconnect2 | 3306 | 192168011 | 14cb6624-7f93-11e0-b2c0-c80aa9429562 |
| 1921680101 | athena | 3306 | 192168011 | 07af4990-f41f-11df-a566-7ac56fdaf645 |
+------------+-----------+------+-----------+--------------------------------------+
{% endhighlight %}

* Server_id: 标志slave服务器的唯一ID值，通常会配置在MySQL slave服务器的配置文件中，或者使用```--server-id=value```选项进行配置。

* Host: slave服务器的主机名称，通常可以使用```--report-host```选项来指定。这样就可以指定与操作系统名称不一样的主机名。

* User： 通过```--report-user```选项指定的slave用户名。注意：只有在master服务器以```--show-slave-auth-info```选项启动时才会打印本列信息。

* Password: 通过```--report-password```选项指定的slave密码。注意： 只有在master服务器以```--show-slave-auth-info```选项启动时才会打印本列信息。

* Port: 指定master服务器所监听的端口

* Master_id: 指定master服务器的唯一ID。

* Slave_UUID: slave服务器的全局唯一ID，通常在slave服务器的```auto.cnf```文件中指定。

**5） 查询当前日志配置相关信息**

可以通过如下命令查询当前日志配置相关信息：
{% highlight string %}
mysql> show variables like 'log_%';
+----------------------------------------+------------------------------------+
| Variable_name                          | Value                              |
+----------------------------------------+------------------------------------+
| log_bin                                | ON                                 |
| log_bin_basename                       | /var/lib/mysql/master-logbin       |
| log_bin_index                          | /var/lib/mysql/master-logbin.index |
| log_bin_trust_function_creators        | OFF                                |
| log_bin_use_v1_row_events              | OFF                                |
| log_builtin_as_identified_by_password  | OFF                                |
| log_error                              | /var/log/mysqld.log                |
| log_error_verbosity                    | 3                                  |
| log_output                             | FILE                               |
| log_queries_not_using_indexes          | OFF                                |
| log_slave_updates                      | OFF                                |
| log_slow_admin_statements              | OFF                                |
| log_slow_slave_statements              | OFF                                |
| log_statements_unsafe_for_binlog       | ON                                 |
| log_syslog                             | OFF                                |
| log_syslog_facility                    | daemon                             |
| log_syslog_include_pid                 | ON                                 |
| log_syslog_tag                         |                                    |
| log_throttle_queries_not_using_indexes | 0                                  |
| log_timestamps                         | UTC                                |
| log_warnings                           | 2                                  |
+----------------------------------------+------------------------------------+
21 rows in set (0.00 sec)
{% endhighlight %}

**6) 查询当前binlog的格式**
{% highlight string %}
mysql> show variables like 'binlog_format';
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| binlog_format | ROW   |
+---------------+-------+
1 row in set (0.00 sec)
{% endhighlight %}
 

### 2.1 PURGE BINARY LOGS语法
这里我们先说明一下查看MySQL数据库文件存放位置的方法：
{% highlight string %}
mysql> show global variables like "%datadir%";
+---------------+-----------------+
| Variable_name | Value           |
+---------------+-----------------+
| datadir       | /var/lib/mysql/ |
+---------------+-----------------+
1 row in set (0.01 sec)
{% endhighlight %}
上面我们看到MySQL的数据存放目录为```/var/lib/mysql```。

PURGE BINARY LOGS语法如下：
{% highlight string %}
PURGE { BINARY | MASTER } LOGS
	{ TO 'log_name' | BEFORE datetime_expr }
{% endhighlight %}

MySQL ```binary log```是由一系列记录MySQL数据修改信息的文件组成： ```多个```二进制日志文件 + ```1个```索引文件组成（请参看后续的```Binary Log```介绍）。

```PURGE BINARY LOGS```会删除所有指定日期之前或者```索引文件```中指定log_name之前的日志。上面```BINARY```与```MASTER```同义。所删除的日志文件会从索引文件中移除。参看如下例子(注意下面语句的执行需要在启用```--log-bin```选项的情况下才有效)：
{% highlight string %}
PURGE BINARY LOGS TO 'mysql-bin.010';
PURGE BINARY LOGS BEFORE '2008-04-02 22:46:26';
{% endhighlight %}

上面语句分别表示： 

* 删除```mysql-bin.010```之前的日志（不包括mysql-bin.010本身）

* 删除```2008-04-02 22:46:26```时间点之前的日志

PURGE语句中```BEFORE```变量```datetime_expr```必须为DATETIME类型的值（即格式必须为：```YYYY-MM-DD hh:mm:ss```)。该语句即使在slave正在复制时执行仍是安全的，这样在执行时就并不需要停止master。假如当前有一个处于active状态slave正在读取一个你尝试删除的日志，那么上述语句并不会删除该文件及其之后的文件，而只会删除更早期的文件。然而，假如有一个slave并未连接上，碰巧你执行上述命令把该slave上次读取的日志文件删除了，那么slave在连接上之后将不能再进行复制。

为了安全的删除删除日志文件，通常我们遵循如下的步骤：

* 在每一个slave机器上，执行```SHOW SLAVE STATUS```用于检查其当前正在读取哪一个日志文件；

* 在master机器上，执行```SHOW BINARY LOGS```以获得当前的日志文件列表；

* 找出所有slave服务器所读取的最早期的日志文件，则比该日志文件更早期的可以被删除；

* 对将要被删除的日志文件做一个备份（建议做好备份）

* Purge所有比target文件更早期的日志文件；

此外，你也可以使用```expire_logs_days```系统变量来自动的淘汰过期的日志文件。假如你当前正在使用MySQL主从复制，那么设置的过期时间应该要大于复制的最大延迟时间。

```PURGE BINARY LOGS TO```与```PURGE BINARY LOGS BEFORE```在实际的文件已经被删除，但是日志索引文件中仍存在相应记录的情况下会执行出错。要处理这种错误，你需要手动编辑```.index```索引文件，并确保索引文件中指定的日志文件确实存在。

### 2.2 RESET MASTER语法
{% highlight string %}
RESET MASTER
{% endhighlight %}
用于删除索引文件中所列出的所有日志文件，并将索引文件清空，然后创建一个新的```二进制日志```(binary log)文件。

从```MySQL5.6.5```版本开始，也会清空系统变量```gtid_purged```与```gtid_executed```，执行完后会将这两个变量的值设置为空字符串(empty string)。该语句通常在master第一次启动的时候会执行。
<pre>
RESET MASTER与PURGE BINARY LOGS的不同主要反应在如下两个方面：

1) RESET MASTER会移除索引文件中列出的所有二进制文件，只会留下一个空的后缀名称为.000001的二进制日志文件。而PURGE BINARY LOGS并不会重置日志文件序号；

2) RESET MASTER一般不会在主从复制过程中执行。如果在主从复制过程中执行RESET MASTER的话，会引起未知后果；而执行PURGE BINRY LOGS通常被认为是安全的。
</pre>

在你最初建立主从复制的时候，执行```RESET MASTER```还是很有用处的。通常在执行时你需要按照如下步骤：

* 启动master与slave，并开始进行主从复制；

* 在master上进行一些查询测试；

* 检查查询是否已经被复制到了slave上；

* 当复制正常运行之后，在slave上执行```STOP SLAVE```，然后再执行```RESET SLAVE```,接着再检查slave上一些不必要的数据是否已经被删除；

* 在master上执行```RESET MASTER```来清除相应的查询测试；

在完成了上面的测试，确保复制工作正常之后，将相应的测试数据清除，然后就可以启动正式的主从复制了。

### 2.3 设置sql_log_bin
{% highlight string %}
SET sql_log_bin = {OFF|ON}
{% endhighlight %}
```sql_log_bin```变量用于控制当前session是否启用二进制日志(binary log)（此处假设binary log本身已经启用了）。在默认情况下值为```ON```:
{% highlight string %}
mysql> show variables like 'sql_log_bin';
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| sql_log_bin   | ON    |
+---------------+-------+
1 row in set (0.00 sec)
{% endhighlight %}

如果要启用或者禁止当前session的二进制日志功能，则可以将该变量设置为```ON```或者```OFF```。通过将当前session的```sql_log_bin```设置为```OFF```，这样就可以临时的对当前会话进制二进制日志功能，从而避免本会话在master上做的修改被同步到slave上。

执行修改会话系统变量的操作是一个```受限```的操作，需要有相应的权限。另外需要注意的是，我们并不能在一个事务(transaction)或者子查询(subquery)中执行```sql_log_bin```的设置。

如果将```sql_log_bin```设置为```OFF```，那么将会阻止二进制日志文件中的事务被分配```GTID```。这样假如你正通过```GTID```来进行复制，即使后序你再启用了sql_log_bin，前面的日志中的事务仍然会被丢失。

## 3. 控制Slave服务器的SQL语句

本节主要会讨论管理slave服务器的SQL语句。这里我们先介绍如下几个```SHOW```语句的使用：

**1） SHOW SLAVE STATUS**

本语句用于显示slave线程的必要参数信息。执行本语句时需要```SUPER```权限或者```REPLICATION CLIENT```权限。例如：
{% highlight string %}
mysql> SHOW SLAVE STATUS\G
*************************** 1. row ***************************
Slave_IO_State: Waiting for master to send event
Master_Host: localhost
Master_User: root
Master_Port: 13000
Connect_Retry: 60
Master_Log_File: master-bin.000002
Read_Master_Log_Pos: 1307
Relay_Log_File: slave-relay-bin.000003
Relay_Log_Pos: 1508
Relay_Master_Log_File: master-bin.000002
Slave_IO_Running: Yes
Slave_SQL_Running: Yes
Replicate_Do_DB:
Replicate_Ignore_DB:
Replicate_Do_Table:
Replicate_Ignore_Table:
Replicate_Wild_Do_Table:
Replicate_Wild_Ignore_Table:
Last_Errno: 0
Last_Error:
Skip_Counter: 0
Exec_Master_Log_Pos: 1307
Relay_Log_Space: 1858
Until_Condition: None
Until_Log_File:
Until_Log_Pos: 0
Master_SSL_Allowed: No
Master_SSL_CA_File:
Master_SSL_CA_Path:
Master_SSL_Cert:
Master_SSL_Cipher:
Master_SSL_Key:
Seconds_Behind_Master: 0
Master_SSL_Verify_Server_Cert: No
Last_IO_Errno: 0
Last_IO_Error:
Last_SQL_Errno: 0
Last_SQL_Error:
Replicate_Ignore_Server_Ids:
Master_Server_Id: 1
Master_UUID: 3e11fa47-71ca-11e1-9e33-c80aa9429562
Master_Info_File: /var/mysqld.2/data/master.info
SQL_Delay: 0
SQL_Remaining_Delay: NULL
Slave_SQL_Running_State: Reading event from the relay log
Master_Retry_Count: 10
Master_Bind:
Last_IO_Error_Timestamp:
Last_SQL_Error_Timestamp:
Master_SSL_Crl:
Master_SSL_Crlpath:
Retrieved_Gtid_Set: 3e11fa47-71ca-11e1-9e33-c80aa9429562:1-5
Executed_Gtid_Set: 3e11fa47-71ca-11e1-9e33-c80aa9429562:1-5
Auto_Position: 1
{% endhighlight %}
这里我们对其中的一些字段做简单的介绍：

* Slave_IO_State: 用于指明当前slave线程的状态： 尝试连接master、等待来自master的事件、重连master等

* Master_Host: 显示当前slave所连接的master的主机名称；


* Master_Log_File，Read_Master_Log_Pos: 记录了IO thread读到的当前master binlog文 件和位置， 对应master的binlog文件和位置。

* Relay_Log_File，Relay_Log_Pos: 记录了SQL thread执行到relay log的那个文件和位置，对应的是slave上的relay log文件和位置。

* Relay_Master_Log_File，Exec_Master_Log_Pos: 记录的是SQL thread执行到master binlog的文件和位置，对应的master上binlog的文件和位置。
<pre>
说明： Relay log file与binary log file类似。从服务器I/O线程将主服务器的二进制日志读取过来记录到从服务器本地文件，
然后SQL线程会读取relay-log日志的内容并应用到从服务器，从而使从服务器和主服务器的数据保持一致
</pre>

* Slave_SQL_Running: 显示SQL线程是否正在运行；

* Relay_Log_Space: 所有已存在的relay日志所占用的空间大小；

* Seconds_Behind_Master： 本字段用于表明slave延迟了多少时间（单位：秒）。当slave当前正在处理更新的时候，本字段用于指明当前处理事件的slave本地时间戳与该事件在master产生时的时间戳之间的延时； 如果当前slave并未在处理更新事件，那么本字段为0。

实际上，本字段是用于表明当前slave的SQL线程与IO线程之间的时间延迟。假如在master与slave之间的网络连接良好，则slave的IO线程与master很接近，在此种情况下本字段可以很好的估算出slave的SQL线程与master之间的延时； 而假如slave与master之间的网速较低，则slave的SQL线程由于低速的IO线程可能经常会处于挂起状态，这时候```Seconds_Behind_Master```的值可能经常会显示为0，而实际这种情况下slave与master之间延迟很大。换句话说，本字段只在```网络良好```的情况下有效。

从MySQL5.6.9起，假如slave的SQL线程并没有运行，或者SQL线程当前已经处理完成了所有的relay log且此时slave的IO线程并未运行的话，此字段的值会被设置为NULL。

* Replicate_Ignore_Server_Ids: 在MySQL5.6版本中，你可以将一个slave通过```CHANGE MASTER TO```语句的```IGNORE_SERVER_IDS```选项忽略0个或多个master。默认情况下本字段的值为空，而在一些环形复制或者multi-master复制的情形下，通常会设置忽略某些master。```Replicate_Ignore_Server_Ids```在不为空的时候显示样式如下：
{% highlight string %}
Replicate_Ignore_Server_Ids: 2, 6, 9
{% endhighlight %}

注意```Replicate_Ignore_Server_Ids```是在IO线程层面来进行过滤的（而不是在SQL线程层面），这就意味着被过滤掉的数据并不会写入到relay log中。而```--replicate-do-table```这样的选项是在SQL线程层面来进行过滤的。

* Master_Server_Id: master的server ID 值；

* Master_UUID: master的uuid值；

* Master_Info_File: 用于```master.info```文件的位置；

* SQL_Delay: slave的SQL线程必须落后与master多少秒；

**2） SHOW RELAYLOG EVENTS语法**
{% highlight string %}
SHOW RELAYLOG EVENTS
	[IN 'log_name']
	[FROM pos]
	[LIMIT [offset,] row_count]
{% endhighlight %}
用于显示一个slave服务器的relay log中的事件，假如在使用时并未指定```log_name```，则会显示relay log索引文件中指明的第一个relay log。本语句只能在slave主机上执行，在master主机上执行无效。```LIMIT```子句与SELECT语法中的LIMIT相同。
<pre>
注： 如果我们在使用SHOW RELAYLOG EVENTS时不添加LIMIT子句的话，则可能会耗费大量的时间及资源，因为SQL服务器会返回给客户端二进制日志文件中的所有内容（这
包括slave所接受到的所有修改数据的语句）。另外，这里我们介绍一个工具mysqlbinlog，我们可以使用该工具来将二进制日志文件保存为一个文本文件，后续我们就可以
对该文本文件进行检查及分析。 
</pre>

```SHOW RELAYLOG EVENTS```会显示二进制日志文件中每一个事件的如下字段：

* Log_name: 日志文件的名称

* Pos： 事件发生的位置

* Event_type: 事件的类型

* Server_id: 产生事件的MySQL Server ID。

* End_log_pos: master的binlog中的End_log_pos值

* Info: 关于该事件类型的更详细的信息。信息的格式取决于事件的类型

### 3.1 CHANGE MASTER TO语法
{% highlight string %}
CHANGE MASTER TO option [, option] ...
option:
	MASTER_BIND = 'interface_name'
	| MASTER_HOST = 'host_name'
	| MASTER_USER = 'user_name'
	| MASTER_PASSWORD = 'password'
	| MASTER_PORT = port_num
	| MASTER_CONNECT_RETRY = interval
	| MASTER_RETRY_COUNT = count
	| MASTER_DELAY = interval
	| MASTER_HEARTBEAT_PERIOD = interval
	| MASTER_LOG_FILE = 'master_log_name'	
	| MASTER_LOG_POS = master_log_pos
	| MASTER_AUTO_POSITION = {0|1}
	| RELAY_LOG_FILE = 'relay_log_name'
	| RELAY_LOG_POS = relay_log_pos
	| MASTER_SSL = {0|1}
	| MASTER_SSL_CA = 'ca_file_name'
	| MASTER_SSL_CAPATH = 'ca_directory_name'
	| MASTER_SSL_CERT = 'cert_file_name'
	| MASTER_SSL_CRL = 'crl_file_name'
	| MASTER_SSL_CRLPATH = 'crl_directory_name'
	| MASTER_SSL_KEY = 'key_file_name'
	| MASTER_SSL_CIPHER = 'cipher_list'
	| MASTER_SSL_VERIFY_SERVER_CERT = {0|1}
	| IGNORE_SERVER_IDS = (server_id_list)

server_id_list:
[server_id [, server_id] ... ]
{% endhighlight %}
```CHANGE MASTER TO```用于修改slave服务器的相关参数： 连接master服务器时的参数、读取master binlog的参数、读取slave relaylog的参数。执行本指令需要有```SUPER```权限。

要使用```CHANGE MASTER TO```，你必须要首先停止slave的复制进程（使用```STOP SLAVE```)。此外，从MySQL5.6.11版本起，```gtid_next```也需要被设置为```AUTOMATIC```。在使用时，未指定的选项```一般情况下```将保持为默认值(除几个例外）。 例如，假如连接到MySQL Master的密码已经被修改，可以执行如下的语句来告诉slave新的密码：
{% highlight string %}
STOP SLAVE; -- if replication was running
CHANGE MASTER TO MASTER_PASSWORD='new3cret';
START SLAVE; -- if you want to restart replication
{% endhighlight %}

下面我们对其中的一些选项做一个简单的说明：

* MASTER_HOST,MASTER_PORT: 用于指明所要连接的master的主机名、端口号。假如你指定了```MASTER_HOST```与```MASTER_PORT```选项，则此时slave会假定当前的master与以前的master是不一样了。在这种情况下，原来老的master log文件名及position将被认为是不可再用了。因此在设置了```MASTER_HOST```与```MASTER_PORT```选项的情况下，如果你并未显示的指定```MASTER_LOG_FILE```与```MASTER_LOG_POS```，则此时他们的值将被重置为```MASTER_LOG_FILE=''```与```MASTER_LOG_POS=4```。

* MASTER_LOG_FILE，MASTER_LOG_POS: 用于指定slave的IO线程下次应该从哪个binlog文件的哪个位置开始读取。

* MASTER_AUTO_POSITION: 一般用于基于```GTID```的复制，因此不能搭配MASTER_LOG_FILE与MASTER_LOG_POS一起使用

<br />
```CHANGE MASTER TO```通常用在你当前已经有一个master的snapshot，并且拥有从master新建snapshot时开始的binlog日志的情况下来创建一个slave。此时slave可以用该snapshot来加载数据，加载完成之后再来同步master，此时你可以在slave主机上运行：
<pre>
CHANGE MASTER TO MASTER_LOG_FILE='log_name', MASTER_LOG_POS=log_pos
</pre>
然后slave会从这一点开始读取master binlog文件。如下的例子演示了```修改master server并从指定文件的pos位置进行复制```:
{% highlight string %}
CHANGE MASTER TO
	MASTER_HOST='master2.example.com',
	MASTER_USER='replication',
	MASTER_PASSWORD='bigs3cret',
	MASTER_PORT=3306,
	MASTER_LOG_FILE='master2-bin.001',
	MASTER_LOG_POS=4,
	MASTER_CONNECT_RETRY=10;
{% endhighlight %}

下面的一个例子给出了一种较少用到的情况： 当slave已经有了relay log文件时，由于某种原因你想要再次执行。此时不必保证slave与master连通，你只需要执行```CHANGE MASTER TO```来启动SQL线程：
{% highlight string %}
CHANGE MASTER TO
RELAY_LOG_FILE='slave-relay-bin.006',
RELAY_LOG_POS=4025;
{% endhighlight %}

### 3.2 MASTER_POS_WAIT()语法
{% highlight string %}
SELECT MASTER_POS_WAIT('master_log_file', master_log_pos [, timeout])
{% endhighlight %}
```MASTER_POS_WAIT```其实是一个函数，主要用于确保slave已经读取并执行到master binlog的指定位置。

### 3.3 RESET SLAVE语法
{% highlight string %}
RESET SLAVE [ALL]
{% endhighlight %}
```REST SLAVE```使得slave清除掉master binlog中的复制位置。本语句的主要目的是为了清空复制环境以获得一个干净的初始状态：清除master信息以及relaylog信息，删除所有的relaylog文件，并建立一个全新的relaylog文件。也会将通过```CHANGE MASTER TO```语句的```MASTER_DELAY```选项设置的值置为0。但是```RESET SLAVE```并不会改变```gtid_executed```及```gtid_purged```的值。要执行```RESET SLAVE```指令，你必须首先要停止slave的复制线程，因此在执行本指令前一般要先执行```STOP SLAVE```。


### 3.4 设置全局sql_slave_skip_counter语法
{% highlight string %}
SET GLOBAL sql_slave_skip_counter = N
{% endhighlight %}
该语句用于指示跳过接下来的```N```个事件，该语句在进行复制恢复的时候很有用： 假如在恢复过程中出现了若干条语句执行错误，则可以使用本语句进行跳过。


### 3.5 START SLAVE语法
{% highlight string %}
START SLAVE [thread_types] [until_option] [connection_options]

thread_types:
	[thread_type [, thread_type] ... ]

thread_type:
	IO_THREAD | SQL_THREAD

until_option:
	UNTIL { {SQL_BEFORE_GTIDS | SQL_AFTER_GTIDS} = gtid_set
		| MASTER_LOG_FILE = 'log_name', MASTER_LOG_POS = log_pos
		| RELAY_LOG_FILE = 'log_name', RELAY_LOG_POS = log_pos
		| SQL_AFTER_MTS_GAPS }

connection_options:
	[USER='user_name'] [PASSWORD='user_pass'] [DEFAULT_AUTH='plugin_name'] [PLUGIN_DIR='plugin_dir']

gtid_set:
	uuid_set [, uuid_set] ...
	| ''

uuid_set:
	uuid:interval[:interval]...

uuid:
	hhhhhhhh-hhhh-hhhh-hhhh-hhhhhhhhhhhh

h:
	[0-9,A-F]

interval:
	n[-n]
(n >= 1)
{% endhighlight %}
假如在执行```START SLAVE```时并不指定```thread_type```选项，则同时会启动IO线程与SQL线程，其中IO线程负责从master读取事件并将读取到的事件存放到```relaylog```中，SQL线程负责从relaylog中读取事件并执行。执行```START SLAVE```需要有```SUPER```权限。

假如```START SLAVE```启动线程成功的话，则不会返回任何错误消息。然而即使启动成功，假如之后连接master失败，又或者读取binlog失败的话，slave还是会停止，而此时```START SLAVE```并不会产生任何警告信息。你必须通过查看slave的错误日志消息来查看slave线程的状态，或者通过```SHOW SLAVE STATUS```来获得相应的状态。


从MySQL5.6.6版本起，```START SLAVE ... UTIL```支持两个额外的```GTID```选项。这两个选项后面都可以跟随一个或多个```gtid_set```作为参数。当并未指定```thread_type```选项时，```START SLAVE UNTIL SQL_BEFORE_GTIDS```会使得slave的SQL线程在处理到```gtid_set```集合中某一个事件时就停止； 而```START SLAVE UNTIL SQL_AFTER_GTIDS```会使得slave的两个线程会处理到```gtid_set```集合中的最后一个事件时停止。```SQL_THREAD```以及```IO_THREAD```都支持```SQL_BEFORE_GTIDS```与```SQL_AFTER_GTIDS```。例如：
<pre>
START SLAVE SQL_THREAD UNTIL SQL_BEFORE_GTIDS =3E11FA47-71CA-11E1-9E33-C80AA9429562:11-56
</pre>
上面会使得slave的SQL线程在处理到server_uuid为```3E11FA47-71CA-11E1-9E33-C80AA9429562```的master序号```11```的事件时就会停止。换句话说，序号是```10```(包括10）之前的事件slave都会进行处理。
<pre>
START SLAVE SQL_THREAD UNTIL SQL_AFTER_GTIDS = 3E11FA47-71CA-11E1-9E33-C80AA9429562:11-56
</pre>
上面会使得slave的SQL线程处理server_uuid为```3E11FA47-71CA-11E1-9E33-C80AA9429562```的master的所有事务，这包括```10```(包括是10）之前的事件，也包括```11```至```56```这个区间的事件，处理完成之后slave将会停止不再处理任何其他的事件。换句话说，slave的SQL线程处理的最后一个事件序号为56。

另外从MySQL5.6.6版本开始，还支持```START SLAVE UNTIL SQL_AFTER_MTS_GAPS```。该语句会导致以```多SQL线程```运行模式的slave在relaylog中找不到更多```gaps```时就会停止。该语句可以搭配```SQL_THREAD```选项来使用，但是不能搭配```IO_THREAD```选项。```START_SLAVE UNTIL SQL_AFTER_MTS_GAPS```应该用在slave在```多线程```模式遇到失败情况，需要转成```单线程```运行模式的情形中。下面的例子演示了一个处于```failed```状态以```多线程```模式的slave转换成```单线程```运行模式：
{% highlight string %}
START SLAVE UNTIL SQL_AFTER_MTS_GAPS;
SET @@GLOBAL.slave_parallel_workers = 0;
START SLAVE SQL_THREAD;
{% endhighlight %}
假如你在```relay_log_recovery```启用的情况下以```多线程```模式运行slave失败，则你必须要在执行```CHANGE MASTER TO```语句执行之前先执行```START SLAVE UNTIL SQL_AFTER_MTS_GAPS```。
<pre>
注： 你一般可以通过SHOW PROCESSLIST来查看START SLAVE以及CHANGE MASTER TO的相关执行状态
</pre>

```START SLAVE```在SQL线程以及IO线程启动完成之后，就会打印出相应的信息。然而，IO线程虽然启动，但是可能还没有连接上master。正是由于这个原因，如果使用```SHOW SLAVE STATUS```显示```SLAVE_SQL_RUNNING=YES```，也并不能保证slave启动一定成功，很有可能只是IO线程启动成功，但还没有真正连接上master。

### 3.5 STOP SLAVE语法
{% highlight string %}
STOP SLAVE [thread_types]

thread_types:
	[thread_type [, thread_type] ... ]

thread_type: IO_THREAD | SQL_THREAD
{% endhighlight %}

用于停止slave线程。在执行```STOP SLAVE```时需要有```SUPER```权限。一般我们是在停止slave服务器之前执行```STOP SLAVE```命令。






<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)

5. [查看mysql主从配置的状态及修正 slave不启动问题](https://blog.csdn.net/ahzxj2012/article/details/54017969)


<br />
<br />
<br />

