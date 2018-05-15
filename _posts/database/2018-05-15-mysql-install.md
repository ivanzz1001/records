---
layout: post
title: mysql安装及简单使用
tags:
- database
categories: database
description: mysql安装及简单使用
---



本文简要记录一下Mysql的安装及简单使用。具体的安装环境如下：


<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 


# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
当前我们使用较多的版本是： mysql5.6、mysql5.7

## 1. Mysql在线安装

CentOS7默认数据库是mariadb，其是mysql的一个分支。这里首先检查当前操作系统是否有mariadb，如有的话需要进行卸载：
<pre>
# rpm -qa | grep mariadb
mariadb-libs-5.5.52-1.el7.x86_64

# yum remove mariadb-libs-5.5.52-1.el7.x86_64

//(如果卸载不掉，请用如下方式卸载)
# rpm -ev mariadb-libs-5.5.52-1.el7.x86_64
</pre>


## 1.1 安装mysql官方yum源
在官网```https://dev.mysql.com/downloads/repo/yum/```可以找到最新的mysql yum源，最新的yum源一般会包括当前最新版的mysql,也会包含一些相对旧的常用的mysql。这里我们下载的mysql80-community-release-el7-1.noarch.rpm中包含所需要的mysql5.6与mysql5.7版本：
<pre>
# wget https://repo.mysql.com//mysql80-community-release-el7-1.noarch.rpm

# yum localinstall ./mysql80-community-release-el7-1.noarch.rpm
</pre>
安装完成后我们可以在如下目录看到:
<pre>
# ls -al /etc/yum.repos.d/mysql-community
mysql-community.repo         mysql-community-source.repo 
</pre>

也可以通过如下命令查看mysql yum源是否安装成功：
<pre>
# yum repolist enabled | grep "mysql.*-community.*"
mysql-connectors-community/x86_64 MySQL Connectors Community                  51
mysql-tools-community/x86_64      MySQL Tools Community                       63
mysql80-community/x86_64          MySQL 8.0 Community Server                  17
</pre>

接下来我们修改```mysql-community.repo```源，改变默认安装的mysql版本。比如我们需要默认安装mysql5.7， 则把5.7版本对应的``enable```字段改为1，其他版本的改为0：
{% highlight string %}
# cat /etc/yum.repos.d/mysql-community.repo 
# Enable to use MySQL 5.5
[mysql55-community]
name=MySQL 5.5 Community Server
baseurl=http://repo.mysql.com/yum/mysql-5.5-community/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

# Enable to use MySQL 5.6
[mysql56-community]
name=MySQL 5.6 Community Server
baseurl=http://repo.mysql.com/yum/mysql-5.6-community/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

# Enable to use MySQL 5.7
[mysql57-community]
name=MySQL 5.7 Community Server
baseurl=http://repo.mysql.com/yum/mysql-5.7-community/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql80-community]
name=MySQL 8.0 Community Server
baseurl=http://repo.mysql.com/yum/mysql-8.0-community/el/7/$basearch/
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql-connectors-community]
name=MySQL Connectors Community
baseurl=http://repo.mysql.com/yum/mysql-connectors-community/el/7/$basearch/
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql-tools-community]
name=MySQL Tools Community
baseurl=http://repo.mysql.com/yum/mysql-tools-community/el/7/$basearch/
enabled=1
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql-tools-preview]
name=MySQL Tools Preview
baseurl=http://repo.mysql.com/yum/mysql-tools-preview/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql-cluster-7.5-community]
name=MySQL Cluster 7.5 Community
baseurl=http://repo.mysql.com/yum/mysql-cluster-7.5-community/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql

[mysql-cluster-7.6-community]
name=MySQL Cluster 7.6 Community
baseurl=http://repo.mysql.com/yum/mysql-cluster-7.6-community/el/7/$basearch/
enabled=0
gpgcheck=1
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-mysql
{% endhighlight %}
这里我们将```mysql80-community```段的enable字段改为0； ```mysql57-community```段的enable字段改为1。

### 1.2 安装mysql5.7

上面我们改完```mysql-community.repo```后就可以直接执行如下的命令安装```mysql5.7```了：
<pre>
# yum install mysql-community-server
Dependencies Resolved

===========================================================================================================================================================================
 Package                                         Arch                            Version                                  Repository                                  Size
===========================================================================================================================================================================
Installing:
 mysql-community-server                          x86_64                          5.7.22-1.el7                             mysql57-community                          165 M
Installing for dependencies:
 mysql-community-client                          x86_64                          5.7.22-1.el7                             mysql57-community                           24 M
 mysql-community-common                          x86_64                          5.7.22-1.el7                             mysql57-community                          274 k
 mysql-community-libs                            x86_64                          5.7.22-1.el7                             mysql57-community                          2.1 M

Transaction Summary
===========================================================================================================================================================================
Install  1 Package (+3 Dependent packages)

</pre>

接着我们再安装一个mysql客户端开发所用到的lib库：
<pre>
# yum install mysql-devel
mysql-community-devel-5.7.22-1.el7.x86_64.rpm                                                                                                       | 3.6 MB  00:00:11     
Running transaction check
Running transaction test
Transaction test succeeded
Running transaction
  Installing : mysql-community-devel-5.7.22-1.el7.x86_64                                                                                                               1/1 
  Verifying  : mysql-community-devel-5.7.22-1.el7.x86_64                                                                                                               1/1 

Installed:
  mysql-community-devel.x86_64 0:5.7.22-1.el7 
</pre>

### 1.3 配置mysql

安装完mysql后有如下默认配置文件：

* ```配置文件```: /etc/my.cnf

* ```日志文件```: /var/log/var/log/mysqld.log 

* ```服务启动脚本```: /usr/lib/systemd/system/mysqld.service 

* ```socket文件```: /var/run/mysqld/mysqld.pid

/etc/my.cnf配置文件：
{% highlight string %}
# For advice on how to change settings please see
# http://dev.mysql.com/doc/refman/5.7/en/server-configuration-defaults.html

[mysqld]
#
# Remove leading # and set to the amount of RAM for the most important data
# cache in MySQL. Start at 70% of total RAM for dedicated server, else 10%.
# innodb_buffer_pool_size = 128M
#
# Remove leading # to turn on a very important data integrity option: logging
# changes to the binary log between backups.
# log_bin
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

### 1.4 启动mysql

1) 执行如下的命令设置mysql开机启动
<pre>
# systemctl enable mysqld
# systemctl daemon-reload
</pre>

2) 执行如下命令启动mysql服务
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


3) 找出mysql初始启动密码

不过采用上面方式安装的mysql，第一次启动使用的是一个随机的密码，我们通过如下方法找出：
<pre>
# grep "password" /var/log/mysqld.log 
2018-05-15T10:03:43.142527Z 1 [Note] A temporary password is generated for root@localhost: E?*RxdrDq6Ta
</pre>

4) 登录重置密码

我们使用上述密码登录mysql，然后对密码进行重置：
{% highlight string %}
# mysql -uroot -p E?*RxdrDq6Ta
Enter password: 
ERROR 1049 (42000): Unknown database 'E?*RxdrDq6Ta'
[root@localhost mysql]# mysql -uroot -pE?*RxdrDq6Ta
mysql: [Warning] Using a password on the command line interface can be insecure.
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 3
Server version: 5.7.22

Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql>
{% endhighlight %}
我们登录之后，暂时不能做任何事情，因为MySQL默认必须修改密码之后才能操作数据库：
<pre>
//下面我们将密码重置为testAa@123 (注： 当前mysql对密码强度有要求，必须为大小写字母+数组）
# ALTER USER 'root'@'localhost' IDENTIFIED BY 'testAa@123';

//或通过如下命令修改用户密码
# set password for 'root'@'localhost'=password('testAa@123');

//执行如下刷新权限
# flush privileges;
</pre>


5) 为root账户授予远程访问的权限
<pre>
# GRANT ALL PRIVILEGES ON *.* TO 'root'@'%' IDENTIFIED BY 'testAa@123' WITH GRANT OPTION;
# flush privileges;
</pre>

也可以专门新建一个账户用于远程访问：
<pre>
# create user 'test_user'@'%' identified by '123456';
# GRANT ALL PRIVILEGES ON *.* TO 'test_user'@'%' IDENTIFIED BY 'testAa@123' WITH GRANT OPTION;
# flush privileges;

// 也可以通过如下指定要授予的权限
# grant select, insert, update, delete on *.* to 'test_user'@'localhost' identified by 'testAa@123';
# flush privileges;
</pre>

6) 修改mysql默认字符编码

首先采用下面的命令查看当前默认字符编码：
{% highlight string %}
# show variables like '%character%';
+--------------------------+----------------------------+
| Variable_name            | Value                      |
+--------------------------+----------------------------+
| character_set_client     | utf8                       |
| character_set_connection | utf8                       |
| character_set_database   | latin1                     |
| character_set_filesystem | binary                     |
| character_set_results    | utf8                       |
| character_set_server     | latin1                     |
| character_set_system     | utf8                       |
| character_sets_dir       | /usr/share/mysql/charsets/ |
+--------------------------+----------------------------+
8 rows in set (0.00 sec)


//也可通过如下方式查看
# \s;
--------------
mysql  Ver 14.14 Distrib 5.7.22, for Linux (x86_64) using  EditLine wrapper

Connection id:          4
Current database:
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
Uptime:                 50 min 47 sec

Threads: 1  Questions: 19  Slow queries: 0  Opens: 119  Flush tables: 1  Open tables: 112  Queries per second avg: 0.006
--------------

ERROR: 
No query specified
{% endhighlight %}
关于mysql字符编码的设置，我们这里暂时不详细介绍。这里暂时可以不用修改，但是下面我们给出修改相应字符的方法：
<pre>
# SET character_set_client = utf8;
# SET character_set_results = utf8;
# SET character_set_connection = utf8;
</pre>

### 1.4 开放3306端口

这里我们可以直接通过如下方法关闭防火墙与SeLinux:
<pre>
systemctl stop firewalld.service 
systemctl disable firewalld.service 
setenforce 0 
sed -i 's/SELINUX=enforcing/SELINUX=disabled/' /etc/selinux/config 
</pre>

也可以通过如下方法单独开放3306端口：
<pre>
//centos查询端口是不是开放的
# firewall-cmd --permanent --query-port=3306/tcp

//添加对外开放端口
# firewall-cmd --permanent --add-port=3306/tcp

//重启防火墙
# firewall-cmd –reload
</pre>

### 1.5 测试远程连接mysql

1） telnet 3306端口

首先telnet该端口:
{% highlight string %}
# telnet <mysql-ip> 3306
{% endhighlight %}

2) 通过Navicat for MySQL进行测试连接


### 1.6 程序测试

上面我们安装了```mysql-devel```，我们这里用其来对刚搭建的mysql进行测试。 编写```test_mysql.c```文件如下：
{% highlight string %}

{% endhighlight %}
编译运行：
<pre>
# whereis mysql
mysql: /usr/bin/mysql /usr/lib64/mysql /usr/include/mysql /usr/share/mysql /usr/share/man/man1/mysql.1.gz

# gcc -o test_mysql test_mysql.c -L/usr/lib64/mysql -lmysqlclient
# ./test_mysql
MYSQL tables in mysql database:
columns_priv
db
engine_cost
event
func
general_log
gtid_executed
help_category
help_keyword
help_relation
help_topic
innodb_index_stats
innodb_table_stats
ndb_binlog_index
plugin
proc
procs_priv
proxies_priv
server_cost
servers
slave_master_info
slave_relay_log_info
slave_worker_info
slow_log
tables_priv
time_zone
time_zone_leap_second
time_zone_name
time_zone_transition
time_zone_transition_type
user
finish
</pre>




<br />
<br />

**[参看】**

1. [CentOS7安装MySQL](https://www.cnblogs.com/bigbrotherer/p/7241845.html)

2. [CentOS7 64位下MySQL5.7安装与配置（YUM）](https://www.linuxidc.com/Linux/2016-09/135288.htm)

3. [Centos7.4安装并配置Mysql5.7](https://blog.csdn.net/zl570932980/article/details/78934601)

4. [设置MySQL的字符编码](https://www.cnblogs.com/xbq8080/p/6572133.html)

5. [Mysql中各种与字符编码集（character_set）有关的变量含义](https://www.cnblogs.com/JMLiu/p/8313204.html)

6. [MySQL字符集编码解析](https://www.jianshu.com/p/96ee5b2adef3)

7. [MySQL字符编码设置方法](http://www.jb51.net/article/85337.htm)

8. [MySQL字符集详解](https://www.cnblogs.com/wcwen1990/p/6917109.html)


<br />
<br />
<br />

