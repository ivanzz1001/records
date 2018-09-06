---
layout: post
title: mysql数据库基础（一）
tags:
- database
categories: database
description: mysql数据库基础
---



本文讲述一下mysql数据库的一些基础知识及相关操作。当前的数据库版本为：```5.7.22 MySQL Community Server (GPL)```


<!-- more -->

## 1. MySQL数据库
MySQL数据库是一个```关系型数据库管理系统```，由瑞典MySQL AB公司开发，目前属于Oracle旗下产品。MySQL是最流行的关系型数据库管理系统之一，在WEB应用等方面，MySQL是最好的RDBMS(Relational Database Managerment System)应用软件。

在```MySQL5.0```之前，默认采用```MyISAM```作为存储引擎，具有较高的查询和插入速度，但不支持事务； 从```MySQL5.5```起，采用```InnoDB```作为默认的存储引擎，支持ACID事务，支持行级锁定。

## 2. MySQL管理
## 2.1 MySQL的启动与关闭
我们可以通过如下的命令来检查MySQL服务器是否启动：
<pre>
# ps -ef | grep mysqld
mysql      2299      1  0 Aug10 ?        00:27:55 /usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid
</pre>
通过上面我们看到，MySQL已经启动。如果mysql未启动，我们可以采用如下的方式来启动：
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: active (running) since Tue 2018-05-15 18:03:45 CST; 5s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 76866 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 76786 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 76868 (mysqld)
   Memory: 315.0M
   CGroup: /system.slice/mysqld.service
           └─76868 /usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid

May 15 18:03:40 localhost.localdomain systemd[1]: Starting MySQL Server...
May 15 18:03:40 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532622].
May 15 18:03:40 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532841].
May 15 18:03:40 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532622].
May 15 18:03:40 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532841].
May 15 18:03:42 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532622].
May 15 18:03:42 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532841].
May 15 18:03:42 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532622].
May 15 18:03:42 localhost.localdomain mysqld_pre_systemd[76786]: Full path required for exclude: net:[4026532841].
May 15 18:03:45 localhost.localdomain systemd[1]: Started MySQL Server.
{% endhighlight %}


## 2.2 查询MySQL版本号
查询mysql版本号的方法有多种。

**1） 在可登录Mysql控制台时** 

在控制台登录mysql时，可以看到版本号：
{% highlight string %}
# mysql -uroot -ptestAa@123
mysql: [Warning] Using a password on the command line interface can be insecure.
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 6
Server version: 5.7.22 MySQL Community Server (GPL)
{% endhighlight %} 

也可以通过```status```命令来查看:
{% highlight string %}
mysql> status
--------------
mysql  Ver 14.14 Distrib 5.7.22, for Linux (x86_64) using  EditLine wrapper

Connection id:          6
Current database:       mysql
Current user:           root@localhost
SSL:                    Not in use
Current pager:          stdout
Using outfile:          ''
Using delimiter:        ;
Server version:         5.7.22 MySQL Community Server (GPL)
Protocol version:       10
Connection:             Localhost via UNIX socket
Server characterset:    latin1
Db     characterset:    latin1
Client characterset:    utf8
Conn.  characterset:    utf8
UNIX socket:            /var/lib/mysql/mysql.sock
Uptime:                 25 days 21 hours 49 min 25 sec

Threads: 1  Questions: 56  Slow queries: 0  Opens: 136  Flush tables: 1  Open tables: 129  Queries per second avg: 0.000
--------------
{% endhighlight %}

另外也可以通过系统函数```version()```来查询：
{% highlight string %}
mysql> select version();
+-----------+
| version() |
+-----------+
| 5.7.22    |
+-----------+
1 row in set (0.02 sec)
{% endhighlight %}

**2) 在不可登录MySQL控制台时**

我们可以通过如下命令查看：
{% highlight string %}
# mysql --help | grep Distrib
mysql  Ver 14.14 Distrib 5.7.22, for Linux (x86_64) using  EditLine wrapper
{% endhighlight %}

也可以通过查看安装包版本来查看：
{% highlight string %}
# rpm -qa | grep mysql 
mysql-community-client-5.7.22-1.el7.x86_64
mysql-community-common-5.7.22-1.el7.x86_64
mysql-community-devel-5.7.22-1.el7.x86_64
mysql-community-libs-5.7.22-1.el7.x86_64
mysql80-community-release-el7-1.noarch
mysql-community-server-5.7.22-1.el7.x86_64
{% endhighlight %}


## 2.3 MySQL用户设置

**1) 添加用户**

如果需要添加MySQL用户，你只需要在MySQL数据库中的user表添加新用户即可。以下为添加用户的实例，用户名为```guest```，密码为```guest123```，并授权用户可进行```SELECT```、```INSERT```和```UPDATE```操作权限：
{% highlight string %}
# mysql -uroot -ptestAa@123
mysql: [Warning] Using a password on the command line interface can be insecure.
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 6
Server version: 5.7.22 MySQL Community Server (GPL)

Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> use mysql;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed

mysql> INSERT INTO user (host, user, authentication_string, ssl_cipher,x509_issuer, x509_subject, select_priv, insert_priv, update_priv) 
    ->  VALUES ('localhost', 'guest', PASSWORD('guestAa@123'), "", "", "", 'Y', 'Y', 'Y');
Query OK, 1 row affected, 1 warning (0.02 sec)

mysql> FLUSH PRIVILEGES;
Query OK, 0 rows affected (0.12 sec)

mysql> SELECT host, user, authentication_string FROM user WHERE user='guest';
+-----------+-------+-------------------------------------------+
| host      | user  | authentication_string                     |
+-----------+-------+-------------------------------------------+
| localhost | guest | *8096F2F3F8E1A3F95F9A25697E14A21A4F0311D2 |
+-----------+-------+-------------------------------------------+
{% endhighlight %}

这里注意在添加完用户之后需要执行```FLUSH PRIVILEGES;```语句，这个命令执行后会重新载入授权表。如果不使用该命令，你就无法使用新创建的用户来连接mysql服务器，除非你重启mysql服务器。

你可以在创建用户时，为用户指定权限，在对应的权限列中，在插入语句中设置为```'Y'```即可，用户权限列表如下：
<pre>
Select_priv： 用户可以通过SELECT命令选择数据。
Insert_priv： 用户可以通过INSERT命令插入数据;
Update_priv： 用户可以通过UPDATE命令修改现有数据;
Delete_priv： 用户可以通过DELETE命令删除现有数据;
Create_priv： 用户可以创建新的数据库和表;
Drop_priv： 用户可以删除现有数据库和表;
Reload_priv： 用户可以执行刷新和重新加载MySQL所用各种内部缓存的特定命令,包括日志、权限、主机、查询和表;重新加载权限表;
Shutdown_priv： 用户可以关闭MySQL服务器;在将此权限提供给root账户之外的任何用户时,都应当非常谨慎;
Process_priv： 用户可以通过SHOW PROCESSLIST命令查看其他用户的进程;服务器管理;
File_priv： 用户可以执行SELECT INTO OUTFILE和LOAD DATA INFILE命令;加载服务器上的文件;
Grant_priv： 用户可以将已经授予给该用户自己的权限再授予其他用户(任何用户赋予全部已有权限);
References_priv: 目前只是某些未来功能的占位符；现在没有作用;
Index_priv： 用户可以创建和删除表索引;用索引查询表;
Alter_priv： 用户可以重命名和修改表结构;
Show_db_priv： 用户可以查看服务器上所有数据库的名字,包括用户拥有足够访问权限的数据库;可以考虑对所有用户禁用这个权限,除非有特别不可抗拒的原因;
Super_priv： 用户可以执行某些强大的管理功能,例如通过KILL命令删除用户进程,使用SET GLOBAL修改全局MySQL变量,执行关于复制和日志的各种命令;超级权限;
Create_tmp_table_priv： 用户可以创建临时表;
Lock_tables_priv： 用户可以使用LOCK TABLES命令阻止对表的访问/修改;
Execute_priv： 用户可以执行存储过程;此权限只在MySQL 5.0及更高版本中有意义;
Repl_slave_priv： 用户可以读取用于维护复制数据库环境的二进制日志文件;此用户位于主系统中,有利于主机和客户机之间的通信;主服务器管理;
Repl_client_priv： 用户可以确定复制从服务器和主服务器的位置;从服务器管理;
Create_view_priv： 用户可以创建视图;此权限只在MySQL 5.0及更高版本中有意义;
Show_view_priv： 用户可以查看视图或了解视图如何执行;此权限只在MySQL 5.0及更高版本中有意义;
Create_routine_priv： 用户可以更改或放弃存储过程和函数;此权限是在MySQL 5.0中引入的;
Alter_routine_priv： 用户可以修改或删除存储函数及函数;此权限是在MySQL 5.0中引入的;
Create_user_priv： 用户可以执行CREATE USER命令,这个命令用于创建新的MySQL账户;
Event_priv： 用户能否创建、修改和删除事件;这个权限是MySQL 5.1.6新增的;
Trigger_priv： 用户能否创建和删除触发器,这个权限是MySQL 5.1.6新增的;
Create_tablespace_priv： 创建表空间
</pre>

另外一种添加用户的方法为通过MySQL的```GRANT```命令，以下命令会给指定数据库```TUTORIALS```添加用户```zara```，密码为```zaraAa@123```:
{% highlight string %}
mysql> GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP ON TUTORIALS.* TO 'zara'@'localhost' IDENTIFIED BY 'zaraAa@123';
Query OK, 0 rows affected, 1 warning (0.05 sec)

mysql> FLUSH PRIVILEGES;
Query OK, 0 rows affected (0.12 sec)
{% endhighlight %}

另外一种添加用户的方法为通过SQL的 GRANT 命令，以下命令会给指定数据库TUTORIALS添加用户 zara ，密码为 zara123 。


**2) 删除用户**


我们可以通过如下的方式删除上面添加的两个用户```zara```和```guest```:
{% highlight string %}
mysql> use mysql;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
mysql> DELETE FROM `user` WHERE Host='localhost' AND User='guest';
Query OK, 1 row affected (0.02 sec)

mysql> DELETE FROM `user` WHERE Host='localhost' AND User='zara';
Query OK, 1 row affected (0.00 sec)

mysql> FLUSH PRIVILEGES;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

### 2.4 MySQL配置文件
MySQL的配置文件为```/etc/my.cnf```，一般情况下，不需要修改该配置文件，该文件默认配置如下：
{% highlight string %}
[mysqld]
datadir=/var/lib/mysql
socket=/var/lib/mysql/mysql.sock

# Disabling symbolic-links is recommended to prevent assorted security risks
symbolic-links=0

log-error=/var/log/mysqld.log
pid-file=/var/run/mysqld/mysqld.pid
{% endhighlight %}

### 2.5 管理MySQL的命令
下面列出了使用MySQL数据库过程中常用的命令：

* USE <dbname>: 用于选择要操作的数据库
{% highlight string %}
mysql> use app;
Database changed
{% endhighlight %}

* SHOW databases: 列出MySQL数据库管理系统的数据库列表
{% highlight string %}
mysql> SHOW databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| app                |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
5 rows in set (0.00 sec)
{% endhighlight %}

*  SHOW tables: 用于显示指定数据库的所有表，使用该命令前需要使用```use```命令来选择要操作的数据库
{% highlight string %}
mysql> use app;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
mysql> show tables;
+---------------+
| Tables_in_app |
+---------------+
| appinfo       |
| bucketinfo    |
+---------------+
2 rows in set (0.00 sec)
{% endhighlight %}

* SHOW COLUMNS FROM <table>: 显示数据表的属性，属性类型，主键信息，是否为NULL，默认值等其他信息
{% highlight string %}
mysql> show columns from appinfo;
+-----------------+---------------------+------+-----+---------+----------------+
| Field           | Type                | Null | Key | Default | Extra          |
+-----------------+---------------------+------+-----+---------+----------------+
| seqid           | bigint(20) unsigned | NO   | PRI | NULL    | auto_increment |
| appid           | varchar(64)         | NO   | UNI | NULL    |                |
| appkey          | varchar(64)         | NO   |     | NULL    |                |
| projectid       | varchar(64)         | NO   | MUL | NULL    |                |
| activity        | tinyint(4)          | NO   |     | NULL    |                |
| isshow          | tinyint(4)          | NO   |     | NULL    |                |
| create_uid      | varchar(128)        | NO   |     | NULL    |                |
| last_modify_uid | varchar(128)        | NO   |     | NULL    |                |
| create_ts       | bigint(20)          | NO   |     | NULL    |                |
| last_modify_ts  | bigint(20)          | NO   |     | NULL    |                |
+-----------------+---------------------+------+-----+---------+----------------+
10 rows in set (0.01 sec)
{% endhighlight %}

* SHOW CREATE TABLE <table>: 查看创建数据库表的语句
{% highlight string %}
mysql> show create table appinfo \G
*************************** 1. row ***************************
       Table: appinfo
Create Table: CREATE TABLE `appinfo` (
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `appid` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `appkey` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `projectid` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `activity` tinyint(4) NOT NULL,
  `isshow` tinyint(4) NOT NULL,
  `create_uid` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  `last_modify_uid` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  `create_ts` bigint(20) NOT NULL,
  `last_modify_ts` bigint(20) NOT NULL,
  PRIMARY KEY (`seqid`),
  UNIQUE KEY `key_appid` (`appid`),
  KEY `key_project` (`projectid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci
1 row in set (0.00 sec)
{% endhighlight %}

* SHOW INDEX FROM <table>: 显示数据表的详细索引信息，包括```PRIMARY KEY```(主键)
{% highlight string %}
mysql> show index from appinfo;
+---------+------------+-------------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+
| Table   | Non_unique | Key_name    | Seq_in_index | Column_name | Collation | Cardinality | Sub_part | Packed | Null | Index_type | Comment | Index_comment |
+---------+------------+-------------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+
| appinfo |          0 | PRIMARY     |            1 | seqid       | A         |           0 |     NULL | NULL   |      | BTREE      |         |               |
| appinfo |          0 | key_appid   |            1 | appid       | A         |           0 |     NULL | NULL   |      | BTREE      |         |               |
| appinfo |          1 | key_project |            1 | projectid   | A         |           0 |     NULL | NULL   |      | BTREE      |         |               |
+---------+------------+-------------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+
3 rows in set (0.01 sec)
{% endhighlight %}

* SHOW TABLE STATUS LIKE[FROM <dbname>] [LIKE 'pattern'] \G: 该命令将输出MySQL数据库管理系统的性能及统计信息
{% highlight string %}
mysql> SHOW TABLE STATUS from app \G
*************************** 1. row ***************************
           Name: appinfo
         Engine: InnoDB
        Version: 10
     Row_format: Dynamic
           Rows: 0
 Avg_row_length: 0
    Data_length: 16384
Max_data_length: 0
   Index_length: 32768
      Data_free: 0
 Auto_increment: 1
    Create_time: 2018-05-16 16:10:02
    Update_time: NULL
     Check_time: NULL
      Collation: utf8_unicode_ci
       Checksum: NULL
 Create_options: 
        Comment: 
*************************** 2. row ***************************
           Name: bucketinfo
         Engine: InnoDB
        Version: 10
     Row_format: Dynamic
           Rows: 0
 Avg_row_length: 0
    Data_length: 16384
Max_data_length: 0
   Index_length: 32768
      Data_free: 0
 Auto_increment: 1
    Create_time: 2018-05-16 16:10:02
    Update_time: NULL
     Check_time: NULL
      Collation: utf8_unicode_ci
       Checksum: NULL
 Create_options: 
        Comment: 
2 rows in set (0.00 sec)
{% endhighlight %}







<br />
<br />
**[参看]**:

1. [MySQL基础](https://www.toutiao.com/a6543580080638001668/)

2. [mySQL](https://baike.baidu.com/item/mySQL/471251?fr=aladdin)

3. [MySQL用户权限(Host,User,Password)管理(mysql.user)](https://blog.csdn.net/typa01_kk/article/details/49126365)

4. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)


<br />
<br />
<br />

