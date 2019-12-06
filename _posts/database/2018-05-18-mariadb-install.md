---
layout: post
title: mariadb的安装及简单使用
tags:
- database
categories: database
description: mariadb的安装及简单使用
---



本文简要记录一下MariaDB的安装及简单使用。具体的安装环境如下：


<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 


# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
当前我们使用较多的版本是： MariaDB 10.3.7






## 2. 离线安装mariadb

### 2.1 下载安装包

我们可以到[mariadb官网](https://downloads.mariadb.org/)去下载对应的安装包，这里我们下载*mariadb-10.3.7-linux-systemd-x86_64.tar.gz*:
<pre>
# wget http://ftp.hosteurope.de/mirror/archive.mariadb.org//mariadb-10.3.7/bintar-linux-systemd-x86_64/mariadb-10.3.7-linux-systemd-x86_64.tar.gz
</pre>

下载完成后，解压到*/usr/local/*目录：
<pre>
# sudo tar -zxvf mariadb-10.3.7-linux-systemd-x86_64.tar.gz -C /usr/local
# cd /usr/local/mariadb-10.3.7-linux-systemd-x86_64/
# ls
bin      COPYING.thirdparty  data  EXCEPTIONS-CLIENT  INSTALL-BINARY  man         README.md     scripts  sql-bench
COPYING  CREDITS             docs  include            lib             mysql-test  README-wsrep  share    support-files
</pre>

### 2.2 查看安装说明
这里我们查看上面的```INSTALL-BINARY```：
{% highlight string %}

   A more detailed version of the preceding description for
   installing a binary distribution follows:

    1. Add a login user and group for mysqld to run as:
shell> groupadd mysql
shell> useradd -g mysql mysql
       These commands add the mysql group and the mysql user. The
       syntax for useradd and groupadd may differ slightly on
       different versions of Unix, or they may have different names
       such as adduser and addgroup.
       You might want to call the user and group something else
       instead of mysql. If so, substitute the appropriate name in
       the following steps.

    2. Pick the directory under which you want to unpack the
       distribution and change location into it. In the following
       example, we unpack the distribution under /usr/local. (The
       instructions, therefore, assume that you have permission to
       create files and directories in /usr/local. If that directory
       is protected, you must perform the installation as root.)
shell> cd /usr/local

    3. Obtain a distribution file using the instructions in Section
       2.1.3, "How to Get MariaDB." For a given release, binary
       distributions for all platforms are built from the same MariaDB
       source distribution.

    4. Unpack the distribution, which creates the installation
       directory. Then create a symbolic link to that directory:
shell> gunzip < /path/to/mysql-VERSION-OS.tar.gz | tar xvf -
shell> ln -s full-path-to-mysql-VERSION-OS mysql
       The tar command creates a directory named mysql-VERSION-OS.
       The ln command makes a symbolic link to that directory. This
       lets you refer more easily to the installation directory as
       /usr/local/mysql.
       With GNU tar, no separate invocation of gunzip is necessary.
       You can replace the first line with the following alternative
       command to uncompress and extract the distribution:
shell> tar zxvf /path/to/mysql-VERSION-OS.tar.gz

    5. Change location into the installation directory:
shell> cd mysql
       You will find several files and subdirectories in the mysql
       directory. The most important for installation purposes are
       the bin and scripts subdirectories:

          + The bin directory contains client programs and the
            server. You should add the full path name of this
            directory to your PATH environment variable so that your
            shell finds the MariaDB programs properly. See Section
            2.14, "Environment Variables."

          + The scripts directory contains the mysql_install_db
            script used to initialize the mysql database containing
            the grant tables that store the server access
            permissions.

    6. Ensure that the distribution contents are accessible to mysql.
       If you unpacked the distribution as mysql, no further action
       is required. If you unpacked the distribution as root, its
       contents will be owned by root. Change its ownership to mysql
       by executing the following commands as root in the
       installation directory:
shell> chown -R mysql .
shell> chgrp -R mysql .
       The first command changes the owner attribute of the files to
       the mysql user. The second changes the group attribute to the
       mysql group.

    7. If you have not installed MariaDB before, you must create the
       MariaDB data directory and initialize the grant tables:
shell> scripts/mysql_install_db --user=mysql
       If you run the command as root, include the --user option as
       shown. If you run the command while logged in as that user,
       you can omit the --user option.
       The command should create the data directory and its contents
       with mysql as the owner.
       After creating or updating the grant tables, you need to
       restart the server manually.

    8. Most of the MariaDB installation can be owned by root if you
       like. The exception is that the data directory must be owned
       by mysql. To accomplish this, run the following commands as
       root in the installation directory:

shell> chown -R root .
shell> chown -R mysql data

    9. If you want MariaDB to start automatically when you boot your
       machine, you can copy support-files/mysql.server to the
       location where your system has its startup files. More
       information can be found in the support-files/mysql.server
       script itself and in Section 2.13.1.2, "Starting and Stopping
       MariaDB Automatically."
   10. You can set up new accounts using the bin/mysql_setpermission
       script if you install the DBI and DBD::mysql Perl modules. See
       Section 4.6.14, "mysql_setpermission --- Interactively Set
       Permissions in Grant Tables." For Perl module installation
       instructions, see Section 2.15, "Perl Installation Notes."
   11. If you would like to use mysqlaccess and have the MariaDB
       distribution in some nonstandard location, you must change the
       location where mysqlaccess expects to find the mysql client.
       Edit the bin/mysqlaccess script at approximately line 18.
       Search for a line that looks like this:
$MYSQL     = '/usr/local/bin/mysql';    # path to mysql executable
       Change the path to reflect the location where mysql actually
       is stored on your system. If you do not do this, a Broken pipe
       error will occur when you run mysqlaccess.

   After everything has been unpacked and installed, you should test
   your distribution. To start the MariaDB server, use the following
   command:
shell> bin/mysqld_safe --user=mysql &

   If you run the command as root, you must use the --user option as
   shown. The value of the option is the name of the login account
   that you created in the first step to use for running the server.
   If you run the command while logged in as mysql, you can omit the
   --user option.

   If the command fails immediately and prints mysqld ended, you can
   find some information in the host_name.err file in the data
   directory.

   More information about mysqld_safe is given in Section 4.3.2,
   "mysqld_safe --- MySQL Server Startup Script."
{% endhighlight %}

### 2.3 安装MariaDB
1) 为mysqld创建一个登录用户及用户组
<pre>
# sudo groupadd mysql
# sudo useradd -g mysql mysql
</pre>

2) 为解压的*mariadb-10.3.7-linux-systemd-x86_64*创建一个软链接，方便管理
{% highlight string %}
# cd /usr/local
# ls -al mariadb-10.3.7-linux-systemd-x86_64
# ln -sf mariadb-10.3.7-linux-systemd-x86_64 mysql
# ls -al mysql
lrwxrwxrwx 1 root root 36 Dec  6 14:15 mysql -> mariadb-10.3.7-linux-systemd-x86_64/
{% endhighlight %}

3) 更改mysql安装目录的访问权限

通常我们要求以```mysql```的身份来执行安装目录下的相关文件，因此这里我们更改相关目录的访问权限：
<pre>
# cd /usr/local/mysql
# sudo chown -R mysql .
# sudo chgrp -R mysql .
# chmod -R 755 /usr/local/mysql/
</pre>

4) 创建数据存放目录

通常情况下，mysql的数据存放目录为*/var/lib/mysql*，在这里我们将存放数据的目录更改为*/apps/dbdat/mariadb/*:
<pre>
# mkdir -p /apps/dbdat/mariadb/
# sudo chown -R mysql:mysql /data/
</pre>

5) 初始化数据库

执行如下命令初始化数据库：
<pre>
# pwd
/usr/local/mysql
# sudo scripts/mysql_install_db --user=mysql --datadir=/apps/dbdat/mariadb/
Installing MariaDB/MySQL system tables in '/apps/dbdat/mariadb/' ...
OK

To start mysqld at boot time you have to copy
support-files/mysql.server to the right place for your system

PLEASE REMEMBER TO SET A PASSWORD FOR THE MariaDB root USER !
To do so, start the server, then issue the following commands:

'./bin/mysqladmin' -u root password 'new-password'
'./bin/mysqladmin' -u root -h mvxl73483 password 'new-password'

Alternatively you can run:
'./bin/mysql_secure_installation'

which will also give you the option of removing the test
databases and anonymous user created by default.  This is
strongly recommended for production servers.

See the MariaDB Knowledgebase at http://mariadb.com/kb or the
MySQL manual for more instructions.

You can start the MariaDB daemon with:
cd '.' ; ./bin/mysqld_safe --datadir='/apps/dbdat/mariadb/'

You can test the MariaDB daemon with mysql-test-run.pl
cd './mysql-test' ; perl mysql-test-run.pl

Please report any problems at http://mariadb.org/jira

The latest information about MariaDB is available at http://mariadb.org/.
You can find additional information about the MySQL part at:
http://dev.mysql.com
Consider joining MariaDB's strong and vibrant community:
https://mariadb.org/get-involved/
</pre>

6) 将*/usr/local/mysql/bin*添加到PATH环境变量

这里修改*/etc/profile*文件，将mysql安装路径添加到PATH中，然后执行```source /etc/profile```，执行完成后：

<pre>
# echo $PATH
/usr/lib64/ccache:/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin:/user/local/bin/:/apps/.local/bin:/apps/bin:/usr/local/mysql/bin
</pre>

7) 准备配置文件

在/usr/local/mysql/support-files/下的配置文件模板，已经配置好的部分参数，分别用于不同的环境，这里说明一下：

* my-small.cnf 这个是为小型数据库或者个人测试使用的，不能用于生产环境

* my-medium.cnf 这个适用于中等规模的数据库，比如个人项目或者小型企业项目中，

* my-large.cnf 一般用于专门提供SQL服务的服务器中，即专门运行数据库服务的主机，配置要求要更高一些，适用于生产环境

* my-huge.cnf 用于企业级服务器中的数据库服务，一般更多用于生产环境使用

所以根据以上几个文件，如果个人使用或者测试，那么可以使用前两个模板；企业服务器或者64G以上的高配置服务器可以使用后面两个模板，另外也可以根据自己的需求来加大参数和扩充配置获得更好的性能。但很不幸的是，我们当前下载的安装包中没有这些文件，这里我们直接在*/etc/*目录下创建```my.cnf```，如下：
<pre>
# The MariaDB server
[mysqld]
port=3306
socket=/var/lib/mysql/mysql.sock
basedir=/usr/local/mysql
datadir=/apps/dbdat/mariadb/
innodb_file_per_table=on

[mysqld_safe]
log-error=/var/log/mariadb/mariadb.log
pid-file=/var/run/mariadb/mariadb.pid

[mysql]
default-character-set=utf8
socket=/var/lib/mysql/mysql.sock

[client]
default-character-set=utf8
socket=/var/lib/mysql/mysql.sock
</pre>

8) 创建log-error、pid-file、socket文件目录
<pre>
# sudo mkdir -p /var/log/mariadb/
# sudo touch /var/log/mariadb/mariadb.log
# sudo chown mysql /var/log/mariadb/mariadb.log

# sudo mkdir -p /var/run/mariadb/
# sudo chown -R mysql:mysql /var/run/mariadb/

# sudo mkdir -p /var/lib/mysql/
# sudo chown -R mysql:mysql /var/lib/mysql
</pre>

9) 创建mariadb自启动脚本，并加入开机启动

<pre>
# sudo cp /usr/local/mysql/support-files/systemd/mariadb.service  /usr/lib/systemd/system/

# sudo systemctl enable mariadb
# sudo systemctl start mariadb
# sudo systemctl stop mariadb
</pre>

10) MySQL的安全设置
<pre>
# sudo ./bin/mysql_secure_installation 

NOTE: RUNNING ALL PARTS OF THIS SCRIPT IS RECOMMENDED FOR ALL MariaDB
      SERVERS IN PRODUCTION USE!  PLEASE READ EACH STEP CAREFULLY!

In order to log into MariaDB to secure it, we'll need the current
password for the root user.  If you've just installed MariaDB, and
you haven't set the root password yet, the password will be blank,
so you should just press enter here.

Enter current password for root (enter for none):   //直接回车，默认为空
OK, successfully used password, moving on...

Setting the root password ensures that nobody can log into the MariaDB
root user without the proper authorisation.

Set root password? [Y/n] y     //是否设置MySQL管理员root的密码，y设置，输入2次
New password: 
Re-enter new password: 
Password updated successfully!
Reloading privilege tables..
 ... Success!


By default, a MariaDB installation has an anonymous user, allowing anyone
to log into MariaDB without having to have a user account created for
them.  This is intended only for testing, and to make the installation
go a bit smoother.  You should remove them before moving into a
production environment.

Remove anonymous users? [Y/n] y    //是否删除匿名账户 y删除
 ... Success!

Normally, root should only be allowed to connect from 'localhost'.  This
ensures that someone cannot guess at the root password from the network.

Disallow root login remotely? [Y/n] n   //是否不允许root用户远程登陆，n不禁用
 ... skipping.

By default, MariaDB comes with a database named 'test' that anyone can
access.  This is also intended only for testing, and should be removed
before moving into a production environment.

Remove test database and access to it? [Y/n] y   //是否删除test测试数据库，y删除
 - Dropping test database...
 ... Success!
 - Removing privileges on test database...
 ... Success!

Reloading the privilege tables will ensure that all changes made so far
will take effect immediately.

Reload privilege tables now? [Y/n] y   //重新加载可用的数据库表  y 是
 ... Success!

Cleaning up...

All done!  If you've completed all of the above steps, your MariaDB
installation should now be secure.

Thanks for using MariaDB!
</pre>


## 2.4 修改root密码

默认情况下，新安装的 mariadb 的密码为空，在shell终端直接输入 mysql 就能登陆数据库。如果是刚安装第一次使用，请使用 mysql_secure_installation 命令初始化，请参看上一节*步骤10) MySQL的安全设置*。

### 2.4.1 已知root密码
在我们知道root密码的情况下，如果要修改root密码，主要有如下两种方法：

1) 直接在shell命令行使用 mysqladm 命令修改
<pre>
# mysqladmin -uroot -poldpassword password newpassword
</pre>

2) 登录数据库修改密码

首先执行如下命令登录：
<pre>
# mysql -uroot -p
</pre>

接着执行如下命令进行修改：
{% highlight string %}
//更新 mysql 库中 user 表的字段：
MariaDB [(none)]> use mysql;  
MariaDB [mysql]> UPDATE user SET password=password('newpassword') WHERE user='root';  
MariaDB [mysql]> flush privileges;  
MariaDB [mysql]> exit;
{% endhighlight %}

这里作为测试，我们将密码修改为```root123```。

### 2.4.2 未知root密码
如果是忘记了 root 密码，则需要以跳过授权的方式启动 mariadb 来修改密码。

1) 关闭mariadb服务
<pre>
# sudo systemctl stop mariadb
# ps -ef | grep mysql
</pre>

2) 使用跳过授权的方式启动 mariadb
<pre>
# mysqld_safe --skip-grant-tables &
[1] 1441
# 170531 02:10:28 mysqld_safe Logging to '/var/log/mariadb/mariadb.log'.
170531 02:10:28 mysqld_safe Starting mysqld daemon with databases from /var/lib/mysql

# ps -ef | grep 1441
root      1441   966  0 02:10 pts/0    00:00:00 /bin/sh /usr/bin/mysqld_safe --skip-grant-tables
mysql     1584  1441  0 02:10 pts/0    00:00:00 /usr/libexec/mysqld --basedir=/usr --datadir=/var/lib/mysql --plugin-dir=/usr/lib64/mysql/plugin --user=mysql --skip-grant-tables --log-error=/var/log/mariadb/mariadb.log --pid-file=/var/run/mariadb/mariadb.pid --socket=/var/lib/mysql/mysql.sock
</pre>

3) 当跳过授权启动时，可以不需要密码直接登陆数据库。登陆更新密码即可
{% highlight string %}
# mysql
MariaDB [(none)]> use mysql;  
MariaDB [mysql]> UPDATE user SET password=password('newpassword') WHERE user='root';  
MariaDB [mysql]> flush privileges;   
MariaDB [mysql]> exit; 
{% endhighlight %}

更新密码后，在跳过授权启动时也不能空密码直接登陆了

4) 关闭跳过授权启动的进程
<pre>
# ps -ef | grep mysql
# sudo kill -9 1441 
# sudo kill -9 1584
</pre>

5） 正常启动 mariadb
<pre>
# sudo systemctl start mariadb
</pre>



<br />
<br />

**[参看】**

1. [MariaDB安装与使用](https://www.cnblogs.com/oukele/p/9590965.html)

2. [Centos7之MariaDB数据库安装教程](https://www.jianshu.com/p/85ad52c88399)

3. [MariaDB三种方法安装及多实例实现](https://blog.51cto.com/13695854/2127892)

4. [MariaDB安装配置](https://www.jianshu.com/p/cf898ddcc0fa)

5. [CentOS7安装MariaDB数据库](https://jingyan.baidu.com/article/67662997be3a7b54d51b8437.html)

6. [CentOS使用rpm离线安装mariadb](https://www.4spaces.org/centos-7-install-mariadb-offline/)

7. [centos7 安装mariadb](https://blog.51cto.com/bajiebushizhu/2126062)

8. [centos7安装mariadb10](https://my.oschina.net/u/222749/blog/2221406)

9. [MySQL/MariaDB的Root密码重置教程](https://www.jb51.net/article/147122.htm)

10. [在CentOS7中安装MariaDB10.3](https://blog.csdn.net/zbljz98/article/details/80462241)

11. [CentOS7.x安装mariadb-10.3](https://www.cnblogs.com/miaocbin/p/11451754.html)

12. [三种方式安装mariadb-10.3.18](https://www.cnblogs.com/ysuwangqiang/p/11766470.html)

13. [CentOS7.4安装Mariadb10.3.11操作实录](https://blog.csdn.net/lovemyth27/article/details/84326281)

14. [CentOS7安装通用二进制格式MariaDB 10.2.8](https://www.linuxidc.com/Linux/2017-10/147343.htm)

15. [Mariadb二进制安装和配置说明](https://www.centos.bz/2017/12/mariadb%E4%BA%8C%E8%BF%9B%E5%88%B6%E5%AE%89%E8%A3%85%E5%92%8C%E9%85%8D%E7%BD%AE%E8%AF%B4%E6%98%8E/)

16. [Mariadb修改root密码](https://www.cnblogs.com/keithtt/p/6922378.html)
<br />
<br />
<br />

