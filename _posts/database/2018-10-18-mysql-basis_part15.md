---
layout: post
title: MySQL主从复制示例
tags:
- database
categories: database
description: MySQL主从复制示例
---


在前面的章节中我们介绍了MySQL主从复制的一些基础知识，这里我们给出相应的示例。这里我们有两台虚拟机```192.168.79.129```与```192.168.79.128```，其中：

* master为192.168.79.129， server id为1921681001

* slave为192.168.79.128, server id为1921681002


<!-- more -->


## 1. Replication Master配置

1） **关闭mysql服务**
<pre>
# systemctl list-units | grep mysql
mysqld.service                                                                                        loaded active running   MySQL Server
# systemctl stop mysqld
# systemctl status mysqld
</pre>

2) **修改配置文件**

修改mysql配置文件```/etc/my.cnf```:
{% highlight string %}
# For advice on how to change settings please see
# http://dev.mysql.com/doc/refman/5.7/en/server-configuration-defaults.html
[mysqld]
sql_mode = NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION
skip-character-set-client-handshake = true
character_set_server = utf8mb4
collation_server = utf8mb4_general_ci
init_connect = 'SET NAMES utf8mb4'
lower_case_table_names = 1
server-id = 1921681001
datadir = /var/lib/mysql
socket = /var/lib/mysql/mysql.sock
log-error = /var/log/mysqld.log
pid-file = /var/run/mysqld/mysqld.pid
slow_query_log = 1
long_query_time = 1
log-bin = master-logbin
binlog_format = ROW
expire_logs_days = 7
symbolic-links = 0
skip-name-resolve = 1
back_log = 600
max_connections = 15000
max_connect_errors = 6000
open_files_limit = 65535
table_open_cache = 2048
table_definition_cache = 2048
table_open_cache_instances = 64
max_allowed_packet = 64M
binlog_cache_size = 4M
max_binlog_size = 1G
max_binlog_cache_size = 2G
max_heap_table_size = 96M
tmp_table_size = 96M
read_buffer_size = 8M
read_rnd_buffer_size = 16M
sort_buffer_size = 16M
join_buffer_size = 16M
thread_cache_size = 64
thread_stack = 512K
query_cache_size = 0
query_cache_type = 0
query_cache_limit = 8M
key_buffer_size = 32M
ft_min_word_len = 4
transaction_isolation = REPEATABLE-READ
performance_schema = 1
explicit_defaults_for_timestamp = true
skip-external-locking
default-storage-engine = InnoDB
innodb_autoinc_lock_mode = 2
innodb_locks_unsafe_for_binlog = 0
innodb_rollback_on_timeout = 1
innodb_buffer_pool_size = 6144M
innodb_buffer_pool_instances = 8
innodb_buffer_pool_load_at_startup = 1
innodb_buffer_pool_dump_at_shutdown = 1
innodb_flush_method = O_DIRECT
innodb_page_cleaners = 4
innodb_file_per_table = 1
innodb_open_files = 500
innodb_write_io_threads = 8
innodb_read_io_threads = 8
innodb_io_capacity = 4000
innodb_io_capacity = 8000
innodb_buffer_pool_dump_pct = 40
innodb_thread_concurrency = 0
innodb_flush_log_at_trx_commit = 2
innodb_log_buffer_size = 32M
innodb_log_file_size = 128M
innodb_log_files_in_group = 3
innodb_max_dirty_pages_pct = 80
innodb_lock_wait_timeout = 120 
innodb_spin_wait_delay = 30
innodb_file_format = Barracuda
innodb_purge_threads = 4
innodb_print_all_deadlocks = 1
bulk_insert_buffer_size = 64M
myisam_sort_buffer_size = 128M
myisam_max_sort_file_size = 10G
myisam_repair_threads = 1
interactive_timeout = 28800
wait_timeout = 28800
ignore-db-dir = lost+found
ignore-db-dir = zabbix.history
ignore-db-dir = zabbix.trends
log_slow_slave_statements = 1
sync_binlog = 1
master_info_repository = TABLE
relay_log_info_repository = TABLE
log_slave_updates
relay_log_recovery = 1
relay_log_purge = 1
innodb_monitor_enable = "module_innodb"
innodb_monitor_enable = "module_server"
innodb_monitor_enable = "module_dml"
innodb_monitor_enable = "module_ddl"
innodb_monitor_enable = "module_trx"
innodb_monitor_enable = "module_os"
innodb_monitor_enable = "module_purge"
innodb_monitor_enable = "module_log"
innodb_monitor_enable = "module_lock"
innodb_monitor_enable = "module_buffer"
innodb_monitor_enable = "module_index"
innodb_monitor_enable = "module_ibuf_system"
innodb_monitor_enable = "module_buffer_page"
innodb_monitor_enable = "module_adaptive_hash"
[mysqld_safe]
pid-file = /run/mysqld/mysqld.pid
syslog
[mysqldump]
quick
max_allowed_packet = 64M
[mysqladmin]
socket=/var/lib/mysql/mysql.sock
[client]
!includedir /etc/my.cnf.d
{% endhighlight %}
注： 在master上我们还可以在配置文件```/etc/my.cnf```的```[mysqld]```段中明确指定需要记录哪些数据库操作的日志，以及忽略哪些数据库操作的日志
<pre>
[mysqld]
binlog_do_db = test1       #数据库白名单列表,二进制日志记录的数据库,即需要同步的库
binlog_do_db = test2
binlog_do_db = test3

binlog_ignore_db = igdb1   #数据库黑名单列表, 二进制日志中忽略的数据库
binlog_ignore_db = igdb2
</pre>

3) **重启master服务**
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: active (running) since Fri 2019-06-28 17:14:15 CST; 13s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 68552 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 68528 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 68555 (mysqld)
   Memory: 1.0G
   CGroup: /system.slice/mysqld.service
           └─68555 /usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid
{% endhighlight %}

## 2. Replication Slave配置

1) **关闭mysql服务**
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

2) **修改配置文件**

修改mysql配置文件：
{% highlight string %}
# For advice on how to change settings please see
# http://dev.mysql.com/doc/refman/5.7/en/server-configuration-defaults.html

[mysqld]
read_only=1
sql_mode = NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION
skip-character-set-client-handshake = true
character_set_server = utf8mb4
collation_server = utf8mb4_general_ci
init_connect = 'SET NAMES utf8mb4'
lower_case_table_names = 1
server-id = 1921681002
datadir = /var/lib/mysql
socket = /var/lib/mysql/mysql.sock
log-error = /var/log/mysqld.log
pid-file = /var/run/mysqld/mysqld.pid
slow_query_log = 1
long_query_time = 1
#log-bin
#binlog_format = ROW
expire_logs_days = 7
symbolic-links = 0
skip-name-resolve = 1
back_log = 600
max_connections = 1000
max_connect_errors = 6000
open_files_limit = 65535
table_open_cache = 2048
table_definition_cache = 2048
table_open_cache_instances = 64
max_allowed_packet = 64M
binlog_cache_size = 4M
max_binlog_size = 1G
max_binlog_cache_size = 2G
max_heap_table_size = 96M
tmp_table_size = 96M
read_buffer_size = 8M
read_rnd_buffer_size = 16M
sort_buffer_size = 16M
join_buffer_size = 16M
thread_cache_size = 64
thread_stack = 512K
query_cache_size = 0
query_cache_type = 0
query_cache_limit = 8M
key_buffer_size = 32M
ft_min_word_len = 4
transaction_isolation = REPEATABLE-READ
performance_schema = 1
explicit_defaults_for_timestamp = true
skip-external-locking
default-storage-engine = InnoDB
innodb_autoinc_lock_mode = 2
innodb_locks_unsafe_for_binlog = 0
innodb_rollback_on_timeout = 1
#innodb_buffer_pool_size = 6144M
innodb_buffer_pool_size = 5760M
innodb_buffer_pool_instances = 8
innodb_buffer_pool_load_at_startup = 1
innodb_buffer_pool_dump_at_shutdown = 1
innodb_flush_method = O_DIRECT
innodb_page_cleaners = 4
innodb_file_per_table = 1
innodb_open_files = 500
innodb_write_io_threads = 8
innodb_read_io_threads = 8
innodb_io_capacity = 4000
innodb_io_capacity = 8000
innodb_buffer_pool_dump_pct = 40
innodb_thread_concurrency = 0
innodb_flush_log_at_trx_commit = 2
innodb_log_buffer_size = 32M
innodb_log_file_size = 128M
innodb_log_files_in_group = 3
innodb_max_dirty_pages_pct = 80
innodb_lock_wait_timeout = 120 
innodb_spin_wait_delay = 30
innodb_file_format = Barracuda
innodb_purge_threads = 4
innodb_print_all_deadlocks = 1
bulk_insert_buffer_size = 64M
myisam_sort_buffer_size = 128M
myisam_max_sort_file_size = 10G
myisam_repair_threads = 1
interactive_timeout = 28800
wait_timeout = 28800
ignore-db-dir = lost+found
ignore-db-dir = zabbix.history
ignore-db-dir = zabbix.trends
log_slow_slave_statements = 1
sync_binlog = 1
master_info_repository = TABLE
relay_log_info_repository = TABLE
log_slave_updates
relay_log_recovery = 1
relay_log_purge = 1
innodb_monitor_enable = "module_innodb"
innodb_monitor_enable = "module_server"
innodb_monitor_enable = "module_dml"
innodb_monitor_enable = "module_ddl"
innodb_monitor_enable = "module_trx"
innodb_monitor_enable = "module_os"
innodb_monitor_enable = "module_purge"
innodb_monitor_enable = "module_log"
innodb_monitor_enable = "module_lock"
innodb_monitor_enable = "module_buffer"
innodb_monitor_enable = "module_index"
innodb_monitor_enable = "module_ibuf_system"
innodb_monitor_enable = "module_buffer_page"
innodb_monitor_enable = "module_adaptive_hash"
[mysqld_safe]
pid-file = /run/mysqld/mysqld.pid
syslog
[mysqldump]
quick
max_allowed_packet = 64M
[mysqladmin]
socket=/var/lib/mysql/mysql.sock
[client]
!includedir /etc/my.cnf.d
{% endhighlight %}
注： 从服务器SQL Thread在Replay中继日志中的事件时仅读取于特定数据库相关的事件，并应用于本地. (但是浪费I/O ,浪费带宽) 推荐使用从节点复制过滤相关设置项。我们可以在配置文件```/etc/my.cnf```的```[mysqld]```段中明确指定只复制哪些数据库，同时忽略哪些数据库：
<pre>
replicate_do_db = test1                 #复制库的白名单. 设定需要复制的数据库（多数据库使用逗号隔开或重复设置多行）
replicate_ingore_db = test2             #复制库的黑名单. 设定需要忽略的复制数据库 （多数据库使用逗号隔开或重复设置多行）
replicate_do_table = test1.uw           #复制表的白名单. 设定需要复制的表（多数据库使用逗号隔开或重复设置多行）
relicate_ingore_table=test.user         #复制表的黑名单. 设定需要忽略的复制的表（多数据库使用逗号隔开或重复设置多行）

replicate_wild_do_table = test.%        #同replication-do-table功能一样，但是可以通配符. 更高级别的应用，通配符,应用到哪一类表的。
replicate_wild_ignore_table=mysql.%     #同replication-ignore-table功能一样，但是可以加通配符.
</pre>
当某一个数据库在master中存在，而在slave中不存在时，会出现sql错误，这时候可以排除或者从库手动导入主库数据库。

3） **重启mysql**

{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
{% endhighlight %}


## 3. 创建replication用户

在master上执行如下语句创建replication用户：
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

## 4. 获得master binlog复制点
在master上执行如下操作：

1) 通过命令行客户端启动一个新的session连接上master，然后通过执行FLUSH TABLES WITH READ LOCK将所有的表(tables)及阻塞的写操作都flush到日志文件中
{% highlight string %}
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}
注意:在执行完上面的语句之后，并不要退出对应的客户端连接，这样read lock将会一直维持， 否则对应的lock就会释放。

2) 在master另外一个session(即另一个客户端连接）上，使用SHOW MASTER STATUS语句来获得当前binlog文件的名称及位置(position)
{% highlight string %}
mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000013 |      154 |              |                  |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}

注意此时仍暂时保持mysql处于```READ LOCK```状态。

## 5. master上创建数据快照

执行如下命令导出要同步的数据库：
{% highlight string %}
# mysqldump -hlocalhost -uroot -ptestAa@123  --master-data --databases app test> selected.db
mysqldump: [Warning] Using a password on the command line interface can be insecure.
[root@localhost ~]# more selected.db 
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

CHANGE MASTER TO MASTER_LOG_FILE='master-logbin.000013', MASTER_LOG_POS=154;

--
-- Current Database: `app`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `app` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `app`;

--
-- Table structure for table `appinfo`
--

DROP TABLE IF EXISTS `appinfo`;
{% endhighlight %}


## 6. 导入数据到slave
将master上创建的数据快照拷贝到slave上，执行如下命令将快照数据导入到slave中：
{% highlight string %}
# mysql -uroot -ptestAa@123 < selected.db
{% endhighlight %}

执行后查看是否导入成功：
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
6 rows in set (0.01 sec)
{% endhighlight %}

注：此时可以解除master上面的```READ LOCK```，执行如下命令解除
{% highlight string %}
mysql> UNLOCK TABLES;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

## 7. slave启动主从同步
如下命令在slave上执行。

**1） 执行同步前，要先关闭slave**
{% highlight string %}
mysql> stop slave;
Query OK, 0 rows affected, 1 warning (0.01 sec)
{% endhighlight %}


2) **指定所要同步的master**
{% highlight string %}
change  master to master_host='192.168.79.129',master_user='repl',master_password='replAa@123',master_log_file='master-logbin.000013',master_log_pos=154;
{% endhighlight %}

注意上面这些信息在前面都已经获取到了，一定要保持前后一致。

3) **启动slave同步**
{% highlight string %}
mysql> start slave;
Query OK, 0 rows affected (0.01 sec)
{% endhighlight %}

4) **查看当前同步状态**
{% highlight string %}
mysql> show slave status \G;
*************************** 1. row ***************************
               Slave_IO_State: Waiting for master to send event
                  Master_Host: 192.168.79.129
                  Master_User: repl
                  Master_Port: 3306
                Connect_Retry: 60
              Master_Log_File: master-logbin.000013
          Read_Master_Log_Pos: 154
               Relay_Log_File: localhost-relay-bin.000002
                Relay_Log_Pos: 324
        Relay_Master_Log_File: master-logbin.000013
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
          Exec_Master_Log_Pos: 154
              Relay_Log_Space: 535
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
             Master_Server_Id: 1921681001
                  Master_UUID: 3fc0b929-5827-11e8-a831-000c296f14dc
             Master_Info_File: mysql.slave_master_info
                    SQL_Delay: 0
          SQL_Remaining_Delay: NULL
      Slave_SQL_Running_State: Slave has read all relay log; waiting for more updates
           Master_Retry_Count: 86400
                  Master_Bind: 
      Last_IO_Error_Timestamp: 
     Last_SQL_Error_Timestamp: 
               Master_SSL_Crl: 
           Master_SSL_Crlpath: 
           Retrieved_Gtid_Set: 
            Executed_Gtid_Set: 
                Auto_Position: 0
         Replicate_Rewrite_DB: 
                 Channel_Name: 
           Master_TLS_Version: 
1 row in set (0.00 sec)

ERROR: 
No query specified
{% endhighlight %}

## 8. 测试验证
我们在```master```的```test.student```表中插入一条数据：
{% highlight string %}
mysql> insert student(stuid, stuname) values(10000, "test10000");
Query OK, 1 row affected (0.01 sec)

mysql> select * from student;
+-------+-----------+
| stuid | stuname   |
+-------+-----------+
|  1001 | ivan1001  |
|  1002 | 郭晋安    |
|  1003 | 杨怡      |
|  1004 | 林文龙    |
|  1005 | 周丽淇    |
|  1006 | 天堂哥    |
|  1007 | 欢喜哥    |
| 10000 | test10000 |
+-------+-----------+
8 rows in set (0.01 sec)
{% endhighlight %}
在```slave```上查看插入的数据是否同步过来：
{% highlight string %}
mysql> use test;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
mysql> select * from student;
+-------+-----------+
| stuid | stuname   |
+-------+-----------+
|  1001 | ivan1001  |
|  1002 | 郭晋安    |
|  1003 | 杨怡      |
|  1004 | 林文龙    |
|  1005 | 周丽淇    |
|  1006 | 天堂哥    |
|  1007 | 欢喜哥    |
| 10000 | test10000 |
+-------+-----------+
8 rows in set (0.00 sec)
{% endhighlight %}
这里可以看到数据库同步成功。


<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)

5. [MySQL 设置基于GTID的复制](http://blog.51cto.com/13540167/2086045)

6. [MySQL 在线开启/关闭GTID](https://blog.csdn.net/jslink_l/article/details/54574066)

7. [mysql在线开启或禁用GTID模式](http://www.cnblogs.com/magmell/p/9223556.html)

8. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

9. [Mysql主从同步](https://www.cnblogs.com/kevingrace/p/6256603.html)

<br />
<br />
<br />

