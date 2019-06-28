---
layout: post
title: mysql主从复制(二)
tags:
- database
categories: database
description: mysql数据库基础
---


在上一章，我们介绍了一下MySQL主从复制的一些前导知识，本章我们就介绍一下主从复制具体的配置步骤。当前操作系统环境是：
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 


# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
数据库版本是：
<pre>
# mysql --version
mysql  Ver 14.14 Distrib 5.7.22, for Linux (x86_64) using  EditLine wrapper
</pre>

这里我们有两台虚拟机```192.168.79.129```与```192.168.79.128```，其中：

* master为192.168.79.129， server id为1921681001

* slave为192.168.79.128, server id为1921681002


<!-- more -->


## 1. 建立MySQL主从复制

### 1.1 Replication Master的配置
在replication master上，首先你必须启用```二进制日志```(binary log)并建立建立一个唯一的```server id```。

在master上必须启用binlog功能，因为这是主从复制的基础。此外还必须为master分配一个唯一的server id，分配的范围是[1,2^32-1]。要配置binlog及server id,首先关闭MySQL服务器，然后编辑```my.cnf```文件或者```my.ini```文件，在```[mysqld]```段落下增加```log-bin```及```server-id```选项。例如：

**1） 关闭mysql服务**
{% highlight string %}
# systemctl list-units | grep mysql
mysqld.service                                                                                        loaded active running   MySQL Server
# systemctl stop mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: inactive (dead) since Thu 2018-10-11 15:55:30 CST; 5s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 2597 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 893 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 2670 (code=exited, status=0/SUCCESS)

Oct 11 15:31:49 localhost.localdomain systemd[1]: Starting MySQL Server...
Oct 11 15:32:22 localhost.localdomain systemd[1]: Started MySQL Server.
Oct 11 15:55:29 localhost.localdomain systemd[1]: Stopping MySQL Server...
Oct 11 15:55:30 localhost.localdomain systemd[1]: Stopped MySQL Server.
{% endhighlight %}

**2) 修改配置文件**

修改```/etc/my.cnf```配置文件如下：
{% highlight string %}
[root@localhost ~]# vi /etc/my.cnf
[mysqld]
#
# Remove leading # and set to the amount of RAM for the most important data
# cache in MySQL. Start at 70% of total RAM for dedicated server, else 10%.
# innodb_buffer_pool_size = 128M
#
# Remove leading # to turn on a very important data integrity option: logging
# changes to the binary log between backups.
log-bin=master-logbin
server-id=168079129

#
# Remove leading # to set options mainly useful for reporting servers.
# The server defaults are faster for transactions and fast SELECTs.
# Adjust sizes as needed, experiment to find the optimal values.
# join_buffer_size = 128M
# sort_buffer_size = 2M
# read_rnd_buffer_size = 2M
datadir=/var/lib/mysql
socket=/var/lib/mysql/mysql.sock

# Disabling symbolic-links is recommended to prevent assorted security risks
symbolic-links=0

log-error=/var/log/mysqld.log
pid-file=/var/run/mysqld/mysqld.pid
{% endhighlight %}
上面我们将```log-bin```配置为```master-logbin```；将```server-id```配置为```1921681001````。

**3) 重启MySQL**
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: active (running) since Thu 2018-10-11 16:08:36 CST; 7s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 19228 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 19202 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 19231 (mysqld)
   Memory: 192.9M
   CGroup: /system.slice/mysqld.service
           └─19231 /usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid

Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026533331].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026532755].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026533235].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026532947].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026532851].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026533139].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026533043].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026533331].
Oct 11 16:08:35 localhost.localdomain mysqld_pre_systemd[19202]: Full path required for exclude: net:[4026532755].
Oct 11 16:08:36 localhost.localdomain systemd[1]: Started MySQL Server.
{% endhighlight %}

注：

* 假如你并未指定```server-id```,又或者将其值设为了```0```，那么master会拒绝任何slave的连接；

* 在为了最大程度的保证数据的一致性和可靠性（针对InnoDB引擎），你可以在master的配置文件my.cnf中设置```innodb_flush_log_at_trx_commit=1```，并设置```sync_binlog=1```。

* 在master上，确保不要启用```skip-networking```。因为假如networking被禁止了的话，slave将不能够和master进行通信


###  1.2 Replication Slave配置

在建立replication slave时，你首先也必须得指定一个唯一的```server id```。假如当前你并未指定```server id```又或者指定的```server id```与master的```server id```冲突，那么你必须重新指定.指定步骤如下：


**1) 关闭MySQL服务**
{% highlight string %}
# systemctl list-units | grep mysql
mysqld.service                                                                                        loaded active running   MySQL Server
# systemctl stop mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: inactive (dead) since Thu 2018-10-11 15:55:30 CST; 5s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 2597 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 893 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 2670 (code=exited, status=0/SUCCESS)

Oct 11 15:31:49 localhost.localdomain systemd[1]: Starting MySQL Server...
Oct 11 15:32:22 localhost.localdomain systemd[1]: Started MySQL Server.
Oct 11 15:55:29 localhost.localdomain systemd[1]: Stopping MySQL Server...
Oct 11 15:55:30 localhost.localdomain systemd[1]: Stopped MySQL Server.
{% endhighlight %}

**2) 修改配置文件**

修改```/etc/my.cnf```配置文件中的```[mysqld]```段，添加```server-id```选项：
{% highlight string %}
[mysqld]
server-id=168079128
{% endhighlight %}

上面我们将```server-id```配置为```1921681002```。修改完成之后重启mysql服务:
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
{% endhighlight %}

假如你要建立多个```slaves```，每一个```slave```都必须有一个唯一的```server-id```（不能与master或其他slave相冲突)。


对于建立slave服务，你可以不用启用binlog功能。然而，假如你在slave上也启用了binlog功能，你可以用slave的binlog来做数据备份以及恢复，也可以通过该slave来构建更为复杂的复制拓扑(replication topology)。例如将本slave作为其他slave的master。
<pre>
说明： 假如你并未指定slave的server id，或者将其指定为0， 则slave会拒绝连接到master。
</pre>


### 1.3 创建replication用户
每一个slave都需要使用MySQL用户名、密码来连接到master，因此必须在master上创建好相应的账户。任何具有```REPLICATION SLAVE```权限的账户都可以用于复制操作。当然你也可以为每一个不同的slave分别创建不同的访问账户，或者所有slave都共用同一个账户。

当然你也可以不必为复制特别的创建一个账户，但是你应该清楚用于复制的账户信息会被明文的保存在```master信息库文件```或者表（table)中。因此为了避免不必要的用户信息泄露，最好还是创建一个只具有```REPLICATION SLAVE```权限的账户。

要创建一个账户，使用```CREATE USER```语句，并通过```GRANT```语句为用户分配复制权限(*REPLICATION SLAVE*)。例如我们下面在master上创建一个名称为```repl```的账户用于复制：
{% highlight string %}
mysql> CREATE USER 'repl'@'%' IDENTIFIED BY 'replAa@123';
mysql> GRANT REPLICATION SLAVE ON *.* TO 'repl'@'%';
mysql> FLUSH PRIVILEGES; 

mysql> select Host, User, Repl_slave_priv from mysql.user;
+-----------+---------------+-----------------+
| Host      | User          | Repl_slave_priv |
+-----------+---------------+-----------------+
| localhost | root          | Y               |
| localhost | mysql.session | N               |
| localhost | mysql.sys     | N               |
| %         | root          | Y               |
| %         | test_user     | Y               |
| %         | repl          | Y               |
+-----------+---------------+-----------------+
6 rows in set (0.00 sec)
{% endhighlight %}

### 1.4  获得master binlog复制点
为了配置slave的复制进程能够从一个正确的```点```(point)开始复制，你需要了解master当前binlog的相关偏移。
<pre>
注： 使用'FLUSH TABLES WITH READ LOCK' 会阻塞InnoDB类型表的提交操作(commit operation)被阻塞。
</pre>

假如在master上当前已经存在一些数据，在slave开启复制进程之前你想要先把这些数据同步到slave，则首先你必须在master主机上停止处理SQL，然后再获取当前master的binlog偏移并导出对应的日志数据，之后你才能够恢复master以继续处理SQL请求。假如你并未停止master执行相应的SQL语句，那么导出的```数据```和master```状态信息```很可能并不匹配，这样就会导致后续slave数据库的不一致性甚至导致slave数据库崩溃。

假如你打算停止master以创建一个```数据快照```(data snapshot)，则可以跳过本步骤，直接在创建数据快照之后拷贝一份对应的binlog索引即可。在这种情况下，master在重启之后会创建一个新的binlog文件，slave就可以将这个新的binlog文件作为起始复制点。

为了获得master的binlog起始复制点，你可以按如下的步骤进行操作：

**1)** 通过命令行客户端启动一个新的session连接上master，然后通过执行```FLUSH TABLES WITH READ LOCK```将所有的```表```(tables)及阻塞的写操作都flush到日志文件中
{% highlight string %}
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}
**注意**:在执行完上面的语句之后，并不要退出对应的客户端连接，这样read lock将会一直维持， 否则对应的lock就会释放。

**2)** 在master另外一个session(即另一个客户端连接）上，使用```SHOW MASTER STATUS```语句来获得当前binlog文件的名称及位置(position)
{% highlight string %}
mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000002 |      747 |    test      |  manual,mysql    |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}
上面```File```这一列显示了binlog的文件名称，```Position```这一列显示了在对应日志文件中的偏移。在本例子中，binlog文件的名称为```master-logbin.000002```，position为747。请记录好这些信息，在后续建立slave时需要用到。这些信息指明了slave应该从这个```偏移点```来开启后续的复制进程。

假如master在之前的运行过程中并未开启binlog功能，则通过执行```SHOW MASTER STATUS```或```mysqldump --master-data```后显示的binlog文件名称及position均为空。在这种情况下，后续如果你要指定slave的复制点的话，则日志文件名称指定为空字符串```''```, 偏移指定为```4```。

<br />
通过上面的步骤就可以知道slave的起始复制点，然后就可以在这一点开启我们的日志复制进程。

假如在slave开启复制前，你有一些历史数据需要被同步，那么需要维持上面```步骤1)```的读锁，然后执行```1.5 使用mysqldump创建数据快照```或者```1.6 使用raw数据文件创建数据快照```。这里```read lock```的主要目的是为了防止将历史数据拷贝到slave的过程中，master上相应的数据又做了改变。


我们可以通过如下的命令查看binlog存放的位置：
{% highlight string %}
mysql> show variables like '%log_bin%';
+---------------------------------+------------------------------------+
| Variable_name                   | Value                              |
+---------------------------------+------------------------------------+
| log_bin                         | ON                                 |
| log_bin_basename                | /var/lib/mysql/master-logbin       |
| log_bin_index                   | /var/lib/mysql/master-logbin.index |
| log_bin_trust_function_creators | OFF                                |
| log_bin_use_v1_row_events       | OFF                                |
| sql_log_bin                     | ON                                 |
+---------------------------------+------------------------------------+
6 rows in set (0.00 sec)

# ls /var/lib/mysql/
app         ca.pem           ib_buffer_pool  ib_logfile1           master-logbin.000002  mysql.sock          private_key.pem  server-key.pem
auto.cnf    client-cert.pem  ibdata1         ibtmp1                master-logbin.index   mysql.sock.lock     public_key.pem   sys
ca-key.pem  client-key.pem   ib_logfile0     master-logbin.000001  mysql                 performance_schema  server-cert.pem  test

# cat /var/lib/mysql/master-logbin.index 
./master-logbin.000001
./master-logbin.000002
{% endhighlight %}
一般来说，在刚安装好MySQL后对应的数据目录如下：
<pre>
# ls /var/lib/mysql/
auto.cnf    ca.pem           client-key.pem  ibdata1      ib_logfile1  mysql       mysql.sock.lock     private_key.pem  server-cert.pem  sys
ca-key.pem  client-cert.pem  ib_buffer_pool  ib_logfile0  ibtmp1       mysql.sock  performance_schema  public_key.pem   server-key.pem
</pre>


### 1.5 使用mysqldump创建数据快照

在已存在的master数据库中创建数据快照(snapshot)的其中一种方法就是使用```mysqldump```工具来dump出所有你想要复制的数据库。一旦dump出来了所有的数据，然后你就可以将这些数据导入到slave，之后再开启复制进程。

下面的例子会将所有的数据库都dump到一个名称为```dumpdb.db```文件中，在dump时采用了```--master-data```选项，这样就会自动的加上```CHANGE MASTER TO```语句，从而在导入slave后可以开启复制进程：
{% highlight string %}
# mysqldump -hlocalhost -uroot -ptestAa@123 --all-databases --master-data > dbdump.db
mysqldump: [Warning] Using a password on the command line interface can be insecure.

# more dbdump.db 
-- MySQL dump 10.13  Distrib 5.7.22, for Linux (x86_64)
--
-- Host: localhost    Database: 
-- ------------------------------------------------------
-- Server version       5.7.22-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Position to start replication or point-in-time recovery from
--

CHANGE MASTER TO MASTER_LOG_FILE='master-logbin.000002', MASTER_LOG_POS=747;

--
-- Current Database: `app`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `app` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `app`;
{% endhighlight %}
从上面我们看到在开始添加上了```CHANGE MASTER TO```，后面就是各个数据库、表的相应数据。

假如你在导出数据时不使用```--master-data```选项，那么你必须在执行```mysqldump```前在另一个session中手动执行```FLUSH TABLES WITH READ LOCK```，等到数据完成之后退出该session以解除对表的读锁或直接再另外开一个session执行```UNLOCK TABLE```来解锁。并且你还需要通过执行```SHOW MASTER STATUS```来确定当前导出的数据快照所对应的binlog偏移，然后在后续启动slave时执行```CHANGE MASTER TO```。

当然你也可以只导出指定的数据库，此种情况下请记住在slave处也必须要**过滤**掉那些你并不想要复制的数据库。例如：
{% highlight string %}
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

# mysqldump -hlocalhost -uroot -ptestAa@123  --master-data --databases app test> selected.db
mysqldump: [Warning] Using a password on the command line interface can be insecure.

# more selected.db 
-- MySQL dump 10.13  Distrib 5.7.22, for Linux (x86_64)
--
-- Host: localhost    Database: app
-- ------------------------------------------------------
-- Server version       5.7.22-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Position to start replication or point-in-time recovery from
--

CHANGE MASTER TO MASTER_LOG_FILE='master-logbin.000002', MASTER_LOG_POS=747;

--
-- Current Database: `app`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `app` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `app`;

--
-- Table structure for table `appinfo`
--
{% endhighlight %}
上面只导出了```app```以及```test```这两个数据库。我们在后续建立slave时，就需要忽略其他的数据库。


之后，我们就可以将上面导出的数据拷贝到slave上，然后导入到slave数据库中：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dbdump.db
{% endhighlight %}


### 1.6 使用Raw数据文件创建数据快照
假如你的数据库本身比较庞大，拷贝```raw```数据文件会比采用mysqldump导出然后再导入slave来的更为高效。这项技术跳过了在插入数据时可能需要大量的更新索引从而造成的低效（因为mysqldump导出的其实是一条条SQL语句，在slave上导入时其实是执行相应SQL语句的过程，这也包括数据的插入）。

使用本方法时，如果用于存放数据的表采用了复杂的缓存(cache)和日志算法(logging algorithms)的话，则需要一些额外的步骤来产生一个```即时```的snapshot: 即时在你执行了```FLUSH TABLES WITH READ LOCK;```，初始的拷贝命令还是可能会遗漏掉cache信息和日志更新信息。另外，假如master与slave的```ft_stopword_file```, ```ft_min_word_len```或者```ft_max_word_len```的值不同，且所拷贝的数据表具有全文索引的话，本方法也存在一定的缺陷不是很可靠。


假如你当前使用的是```InnoDB```表，你可以使用```MySQL企业版备份组件```中的```mysqlbackup```命令来产生一致性的数据快照。该命令同样会记录快照所对应的binlog以及偏移。```MySQL企业版备份组件```是一个商业化收费套件，这里我们不进行介绍。

上面介绍的两种方式都存在一定的不足，其实我们可以使用```冷备份```(cold backup)来获得```InnoDB```类型表的可靠数据快照： 即在```slow shutdown```MySQL Server之后来拷贝数据。

而如果要创建```MyISAM```类型表的```原始数据快照```(raw data snapshot)，你可以使用标准的```cp```或```copy```命令工具进行直接拷贝，或者使用```scp```或```rsync```进行远程拷贝。

在创建```Raw```数据文件快照时，通常我们不需要用到如下文件：

* 有关```mysql```数据库的文件

* master info repository文件

* master的binlog文件（注：一般binlog的索引文件我们还是需要的）

* 任何的relay日志文件

例如master数据目录下一般有如下文件：
{% highlight string %}
# ls /var/lib/mysql
app         ca.pem           ib_buffer_pool  ib_logfile1           master-logbin.000002  mysql            performance_schema  server-cert.pem  test
auto.cnf    client-cert.pem  ibdata1         ibtmp1                master-logbin.000003  mysql.sock       private_key.pem     server-key.pem
ca-key.pem  client-key.pem   ib_logfile0     master-logbin.000001  master-logbin.index   mysql.sock.lock  public_key.pem      sys
{% endhighlight %}

我们一般只需要打包如下文件：
<pre>
app		ib_logfile0		ib_logfile1		ibdata1		test
</pre>



如果要```确保```原始数据(raw data)的一致性，我们可以在创建快照的过程中关闭master服务器，具体步骤如下：

**1） 获得读锁和master的状态**
{% highlight string %}
//1. session1连接上获取读锁
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)


//2. session连接上获得master的状态
mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000002 |      747 |              |                  |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}

**2) 关闭master server**

在另一个会话窗口执行如下命令来关闭master server:
{% highlight string %}
# mysqladmin -hlocalhost -uroot -ptestAa@123 shutdown
mysqladmin: [Warning] Using a password on the command line interface can be insecure.
{% endhighlight %}


**3) 拷贝MySQL数据文件**

通常我们可以直接将数据打包即可，例如：
{% highlight string %}
# tar -zcvf /var/lib/mysql/ ./db.tar.gz
{% endhighlight %}

**4) 重启master服务器**
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
{% endhighlight %}


假如我们使用的并不是```InnoDB```类型的表，则你可以在不需要关闭master的情况下来创建快照。具体参看如下步骤：

**1） 获得读锁和master的状态**
{% highlight string %}
//1. session1连接上获取读锁
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)


//2. session连接上获得master的状态
mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000002 |      747 |              |                  |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}

**2) 拷贝MySQL数据文件**

通常我们可以直接将数据打包即可，例如：
{% highlight string %}
# tar -zcvf /var/lib/mysql/ ./db.tar.gz
{% endhighlight %}


**3） 解除读锁**
{% highlight string %}
mysql> UNLOCK TABLES;
{% endhighlight %}

<br />

在我们通过上述方法创建了数据快照后，将相应的数据快照拷贝到slave上。

### 1.7 全新环境创建master/slave复制
最简单和直接的方法是在一个全新的环境下来创建master和slave复制。请参看如下步骤：

* 配置master

* 启动mysql master

* 创建一个用于复制的用户(user)

* 获得master的状态信息(master status)，或者创建snapshot时的mysql binlog索引文件

* 在master上释放读锁
{% highlight string %}
mysql> UNLOCK TABLES;
{% endhighlight %}

* 更新slave的配置

* 执行```CHANGE MASTER TO```语句来设置master复制服务器的配置（请参看```1.10 slave主机上设置master配置```)

<br />
我们在每一个slave上执行上述步骤就可以建立起master-slave之间的复制。

上面的步骤显示了在一个全新的环境下建立主从复制，因此并没有任何历史数据需要进行交换与导入。后面如果我们建立好复制环境后，需要将来自另外一个服务器的数据导入到master，那么我们可以执行如下命令：
{% highlight string %}
# mysql -h master < fulldb.dump
{% endhighlight %}
这样在执行完成后master上的数据将会自动同步到slave上。

### 1.8 已有历史数据环境创建复制
当已有历史数据的情况下，你需要决定在slave开始复制数据之前，如何最好的将master中的历史数据导入到slave中。基本的操作步骤如下：

**1)** 在master上创建一个用于复制的用户(user)

**2)** 假如master当前并未设置server-id或并未开启binlog功能，则需要关闭master服务器来进行设置；假如你需要关闭mysql master服务器的话， 正好可以利用这个机会来获取数据库快照。此时你需要首先获得master状态(参看```1.4 获取master二进制日志复制点```)，然后关闭master，再修改配置，最后通过```raw数据文件```来创建数据库快照(参看```1.6 使用Raw数据文件创建快照```)

**3)** 假如master当前已经设置好了server-id并开启了binlog功能，首先获取到master状态(参看```1.4 获取master二进制日志复制点```),然后使用```mysqldump```来创建数据库快照（参看```1.5 使用mysqldump创建数据快照```)或者使用raw数据来创建快照(参看```1.6 使用Raw数据文件创建快照```)

**4)** 更新slave配置

**5)** 导入快照数据到slave。 这里根据你创建数据快照的方式不同，采用不同的方式导入

* 采用mysqldump创建的数据快照

a) 使用```--skip-slave-start```选项来启动slave，这样复制进程并不会被启动。可以通过修改```my.cnf```配置文件，在```[mysqld]```段落下添加```skip-slave-start```:
{% highlight string %}
[mysqld]
server-id=148

skip-slave-start
{% endhighlight %}

b) 执行如下命令将dump文件导入到slave中
{% highlight string %}
# mysql -hlocalhost -uroot -ptestAa@123 < fulldb.dump 
{% endhighlight %}

* 采用raw数据文件创建的数据快照

a) 将快照数据解压到从机目录
<pre>
# tar -zxvf dumpdb.tar.gz -C /var/lib/mysql
# chown -R mysql:mysql /var/lib/mysql/*
</pre>
注意请确保解压后的文件slave有相应的权限来访问这些数据(可以参看文件原来的权限）。

b) 使用```--skip-slave-start```选项来启动slave，这样复制进程并不会被启动。可以通过修改```my.cnf```配置文件，在```[mysqld]```段落下添加```skip-slave-start```:
{% highlight string %}
[mysqld]
server-id=148

skip-slave-start
{% endhighlight %}

**6）** 配置slave从master的指定复制点开始复制。即需要告诉slave从master的哪一个binlog的哪一个偏移开始复制数据。（参看```1.10 slave主机上设置master配置```)

**7)** 启动slave线程
{% highlight string %}
mysql> START SLAVE;
{% endhighlight %}

<br />
在执行完上述这些步骤之后，slave就会连接到master，并在指定的复制点开始数据同步。假如你忘记了设置master的server-id，则master会拒绝slave的连接； 而假如你忘记了设置slave的server-id，则则slave的错误日志中可以获得如下日志信息（一般为 /var/log/mysqld.log)：
{% highlight string %}
Warning: You should set server-id to a non-0 value if master_host
is set; we will force server id to 2, but this MySQL server will
not act as a slave.
{% endhighlight %}

slave会使用其所存储的```master info repository```来跟踪其当前已经处理到master binlog的什么位置。该repository可以是一个文件，也可以是一个table，具体是什么取决于```--master-info-repository```选项。假如master以```--master-info-repository=FILE```形式运行，则你可以在slave的数据目录下找到两个名称分别为```master.info```与```relay-log.info```的文件；而如果配置为```TABLE```，那么相应的信息会存放在```mysql.master_slave_info```表中。在任何情况下，你都不应该修改这些文件的内容，即使在了解该文件各个字段的含义时，也不建议直接修改，如果确实要修改的话，可以通过```CHANGE MASTER TO```语句来进行。

### 1.9 添加slave到已存在的复制环境中
你可以在不停止```master```的情况下将另外一个slave添加到已存在的复制环境中。要实现这样的功能，你可以通过拷贝已存在的slave的数据目录来建立该新的slave，并且给该新建的slave一个新的server-id。

要拷贝一个已存在的slave，请按如下步骤执行：

**1)** 停止已存在的slave，并记录slave的状态信息，特别是master binlog文件与relay log文件的偏移。你可以通过如下的命令来查看：
{% highlight string %}
mysql> STOP SLAVE;
mysql> SHOW SLAVE STATUS\G
{% endhighlight %}

**2)** 关闭已存在的slave
{% highlight string %}
# mysqladmin shutdown
{% endhighlight %}

**3)** 拷贝已存在slave的数据目录到新的slave主机上，包括日志文件以及relay log文件。

<br />
说明：
<pre>
1） 在拷贝之前，需要确认已存在的slave上所有相关的信息都已经存放到对应的数据目录文件中。例如，对于InnoDB系统表空间、undo表空间、和redo日志也许会存放在另外位置，
 InnoDB表空间文件与file-per-table tablespaces也许都会放在其他的目录。slave的binlog与relaylog也可能有它们自己的独立目录。你需要通过查看系统变量来确定对应的这些文件
 存放在什么地方，然后把所有这些文件拷贝到新的从机上。

2） 在拷贝的时候，假如master info repository信息是以文件方式存放的，则对应的文件也应该拷贝过去； 如果是以table方式存放的，则对应的表存放在数据目录中。

3） 在拷贝完成之后，从拷贝目录删除掉auto.cnf，这样新建的slave将会以一个不同的uuid来启动。
</pre>

在添加新的复制slave时经常遇到的类似如下这样的警告和错误信息：
{% highlight string %}
071118 16:44:10 [Warning] Neither --relay-log nor --relay-log-index were used; so
replication may break when this MySQL server acts as a slave and has his hostname
changed!! Please use '--relay-log=new_slave_hostname-relay-bin' to avoid this problem.
071118 16:44:10 [ERROR] Failed to open the relay log './old_slave_hostname-relay-bin.003525'
(relay_log_pos 22940879)
071118 16:44:10 [ERROR] Could not find target log during relay log initialization
071118 16:44:10 [ERROR] Failed to initialize the master info structure
{% endhighlight %}
这种情形一般发生在未指定```--relay-log```选项的情况下，因为relaylog一般以```host name```来命名。同样，在未指定```--relay-log-index```的情况下，也可能会出现这样的问题。因此，如果遇到这样的问题，我们一般通过添加```--relay-log```或```--relay-log-index```这样的选项来解决。假如在有些MySQL版本中没有这样的选项，请使用如下两个：
<pre>
existing_slave_hostname-relay-bin
existing_slave_hostname-relay-bin.index
</pre>
另外，假如你在尝试了如下4)、5）、6）、7）步骤之后仍遇到前面的问题，请按如下a)、b）、c)步骤来解决
<pre>
a) 假如当前你还未在新创建的slave上执行'STOP SLAVE', 则执行STOP SLAVE； 假如你已经重新启动了已存在的slave，则在已存在的slave上也同样执行STOP SLAVE;

b) 拷贝已存在slave的relaylog索引文件到新的slave数据目录中，并替换原来老的relaylog索引文件；

c) 执行如下4)、5)、6)、7)步骤
</pre>

**4)** 当拷贝完成，重启已存在的slave

**5）** 在新创建的slave上，修改配置文件，并给该新创建的slave指定一个唯一的server id。

**6）** 启动新创建的slave服务器，启动时指定```--skip-slave-start```选项，使得复制进程不会启动。执行```SHOW SLAVE STATUS```语句以确定```新建的slave```与```已存在的slave```相比较配饰是否正确。也需要再次检查server id与UUID是否唯一。

**7）** 通过执行```START SLAVE```语句来启动新建的slave
{% highlight string %}
mysql> START SLAVE;
{% endhighlight %}

到此为止，新建的slave就会用其对应的master info repository信息来开始同步进程。


### 1.10 slave主机上设置master配置
要建立master slave之间的复制，你必须要告诉slave必要的连接信息。你可以在slave上执行如下的命令来完成：
{% highlight string %}
mysql> CHANGE MASTER TO
-> MASTER_HOST='master_host_name',
-> MASTER_USER='replication_user_name',
-> MASTER_PASSWORD='replication_password',
-> MASTER_LOG_FILE='recorded_log_file_name',
-> MASTER_LOG_POS=recorded_log_position;
{% endhighlight %}
```CHANGE MASTER TO```语句还有很多其他的选项。例如能够使用SSL建立安全的复制。

注意：
<pre>
主从复制并不能使用unix socket来进行。你必须要确保slave能够通过TCP/IP来连接到master。
</pre>



<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)



<br />
<br />
<br />

