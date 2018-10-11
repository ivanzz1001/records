---
layout: post
title: mysql数据库基础（九）
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

上面我们将```server-id```配置为```1921681002````。假如你要建立多个master，则每个master的server id都不能重复。

对于slave服务，你可以不用启用binlog功能。然而，假如你在slave上也启用了binlog功能，你可以用slave的binlog来做数据备份以及恢复，也可以通过该slave来构建更为复杂的复制拓扑： 例如将本slave作为其他slave的master。
<pre>
说明： 假如你并未指定slave的server id，或者将其指定为0， 则slave会拒绝连接到master。
</pre>


### 1.3 创建replication用户
每一个slave都需要使用MySQL用户名、密码来连接到master，因此必须在master上创建好相应的账户。任何具有```REPLICATION SLAVE```权限的账户都可以用于复制操作。当然你也可以为每一个不同的slave分别创建不同的访问账户，或者所有slave都共用同一个账户。

当然你也可以不必为复制特别的创建一个账户，但是你应该清楚用于复制的账户信息会被明文的保存在```repository```文件中。因此为了避免不必要的用户信息泄露，最好还是创建一个只具有```REPLICATION SLAVE```权限的账户。

要创建一个账户，使用```CREATE USER```语句，并通过```GRANT```语句为用户分配复制权限。例如我们下面在master上创建一个名称为```repl```的账户用于复制：
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

假如在master上当前已经存在一些数据，在slave开启复制进程之前你想要先把这些数据同步到slave，则首先你必须在master主机上停止处理SQL，然后再获取master的binlog偏移并导出对应的日志数据，之后你才能够恢复master以继续处理SQL请求。假如你并未停止master执行相应的SQL语句，那么导出的```数据```和master```状态信息```很可能并不匹配，这样就会导致后续slave数据库的不一致性甚至导致slave数据库崩溃。

假如你打算停止master以创建一个```数据快照```(data snapshot)，则可以跳过本步骤，直接在创建数据快照之后拷贝一份对应的binlog索引即可。在这种情况下，master在重启之后会创建一个新的binlog文件，slave就可以将这个新的binlog文件作为起始复制点。

为了获得master的binlog起始复制点，你可以按如下的步骤进行操作：

**1)** 通过命令行客户端启动一个新的session连接上master，然后通过执行```FLUSH TABLES WITH READ LOCK```将所有的```表```(tables)及阻塞的写操作都flush到日志文件中
{% highlight string %}
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}
注意在执行完上面的语句之后，并不要退出对应的客户端连接，这样read lock将会一直维持， 否则对应的lock就会释放。

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
上面```File```这一列显示了binlog的文件名称，```Position```这一列显示了在对应日志文件中的偏移。在本例子中，binlog文件的名称为```master-logbin.000002```，position为747。在后续配置slave时这些信息，其指明了slave应该从这个```偏移点```来开启后续的复制进程。

假如master在之前的运行过程中并未开启binlog功能，则通过执行```SHOW MASTER STATUS```或```mysqldump --master-data```后显示的binlog文件名称及position均为空。在这种情况下，后续如果你要指定slave的复制点的话，则日志文件名称指定为```''```, 偏移指定为```4```。

<br />
通过上面的步骤就可以知道slave的起始复制点，然后就可以在这一点开启我们的日志复制进程。

假如在slave开启复制前，你有一些历史数据需要被同步，那么需要维持上面```步骤1)```的读锁，然后执行```1.5 使用mysqldump创建数据快照```或者```1.6 使用raw文件来创建数据快照```。这里```read lock```的主要目的是为了防止将历史数据拷贝到slave的过程中，master上相应的数据又做了改变。


### 1.5 使用mysqldump创建数据快照

创建master中已有数据库快照的其中一种方法就是使用```mysqldump```工具来dump出所有你想要复制的数据库。一旦dump出来了所有的数据，然后你可以将这些数据导入到slave，之后再开启复制进程。















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

