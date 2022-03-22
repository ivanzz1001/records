---
layout: post
title: MongoDB的安装
tags:
- database
categories: database
description:  MongoDB的安装
---

MongoDB server有两个版本： Community版本和Enterprise版本

本章我们介绍MongoDB的安装。

* 将当前部署环境升级到MongoDB 5.0，请参看[Upgrade Procedures](https://docs.mongodb.com/manual/release-notes/5.0/#std-label-5.0-upgrade)

* 为当前版本升级最新版本，请参看[Upgrade to the Latest Revision of MongoDB ](https://docs.mongodb.com/manual/tutorial/upgrade-revision/)

<!-- more -->

## 1. 相关参考文档说明

1） MongoDB安装手册

如下我们介绍MongoDB社区版(Conmunity Edition)在Linux平台上安装的一些参考手册：

* [Install MongoDB Community Edition on Red Hat or CentOS](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-red-hat/)
* [Install MongoDB Community Edition on Ubuntu](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-ubuntu/)
* [Install MongoDB Community Edition on Debian](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-debian/)
* [Install MongoDB Community Edition on SUSE](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-suse/)
* [Install MongoDB Community Edition on Amazon Linux](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-amazon/)

2) **更新社区版本到企业版本**

* [Upgrade to MongoDB Enterprise (Standalone)](https://docs.mongodb.com/manual/tutorial/upgrade-to-enterprise-standalone/)
* [Upgrade to MongoDB Enterprise (Replica Set)](https://docs.mongodb.com/manual/tutorial/upgrade-to-enterprise-replica-set/)
* [Upgrade to MongoDB Enterprise (Sharded Cluster)](https://docs.mongodb.com/manual/tutorial/upgrade-to-enterprise-sharded-cluster/)


## 2. 在RedHat或Centos上安装MongoDB社区版

由于

本节我们介绍使用yum包管理器在Red Hat Enterprise Linux、Centos Linux、Oracle Linux上安装MongoDB 5.0社区版。

当前操作系统环境为：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core)
</pre>


### 2.1 Install MongoDB Community Edition

1) **配置yum源**

创建/etc/yum.repos.d/mongodb-org-5.0.repo文件如下：
{% highlight string %}
[mongodb-org-5.0]
name=MongoDB Repository
baseurl=https://repo.mongodb.org/yum/redhat/$releasever/mongodb-org/5.0/x86_64/
gpgcheck=1
enabled=1
gpgkey=https://www.mongodb.org/static/pgp/server-5.0.asc
{% endhighlight %}

我们也可以直接从[MongoDB repository](https://repo.mongodb.org/yum/redhat/)仓库中下载对应的rpm包来安装。

对于MongoDB 5.0之前的版本，```奇数```版本表示开发版(development releases)。从MongoDB 5.1版本开始，MongoDB有一个季度版本。

2） **安装MongoDB包**

安装最新版本的MongoDB，请执行如下命令：
{% highlight string %}
# sudo yum install -y mongodb-org

...
Installed:
  mongodb-org.x86_64 0:5.0.5-1.el7                                                                                                                          

Dependency Installed:
  mongodb-database-tools.x86_64 0:100.5.1-1                   mongodb-mongosh.x86_64 0:1.1.9-1.el7          mongodb-org-database.x86_64 0:5.0.5-1.el7      
  mongodb-org-database-tools-extra.x86_64 0:5.0.5-1.el7       mongodb-org-mongos.x86_64 0:5.0.5-1.el7       mongodb-org-server.x86_64 0:5.0.5-1.el7        
  mongodb-org-shell.x86_64 0:5.0.5-1.el7                      mongodb-org-tools.x86_64 0:5.0.5-1.el7       

Complete!
{% endhighlight %}
另外，如果要安装一个特定版本的MongoDB的话，请单独指定每一个组件的名称及对应的版本号，如下所示：
{% highlight string %}
# sudo yum install -y mongodb-org-5.0.5 mongodb-org-database-5.0.5 mongodb-org-server-5.0.5 mongodb-org-shell-5.0.5 mongodb-org-mongos-5.0.5 mongodb-org-tools-5.0.5
{% endhighlight %}

然而使用上面命令安装的时候，如果MongoDB有一个更新的版本可用，那么yum会自动的更新到最新版本。为了防止此种情况出现，我们必须钉住(pin)相应的包。要钉住一个package，在/etc/yum.conf中添加如下```exclude```指令：
{% highlight string %}
exclude=mongodb-org,mongodb-org-database,mongodb-org-server,mongodb-org-shell,mongodb-org-mongos,mongodb-org-tools
{% endhighlight %}

安装完成后，我们可以看看默认的/etc/mongod.conf配置文件：
{% highlight string %}
# mongod.conf

# for documentation of all options, see:
#   http://docs.mongodb.org/manual/reference/configuration-options/

# where to write logging data.
systemLog:
  destination: file
  logAppend: true
  path: /var/log/mongodb/mongod.log

# Where and how to store data.
storage:
  dbPath: /var/lib/mongo
  journal:
    enabled: true
#  engine:
#  wiredTiger:

# how the process runs
processManagement:
  fork: true  # fork and run in background
  pidFilePath: /var/run/mongodb/mongod.pid  # location of pidfile
  timeZoneInfo: /usr/share/zoneinfo

# network interfaces
net:
  port: 27017
  bindIp: 127.0.0.1  # Enter 0.0.0.0,:: to bind to all IPv4 and IPv6 addresses or, alternatively, use the net.bindIpAll setting.


#security:

#operationProfiling:

#replication:

#sharding:

## Enterprise-Only Options

#auditLog:

#snmp:
{% endhighlight %}

### 2.2 运行MongoDB社区版

###### 2.2.1 先决条件
1) **ulimit**

大多数Unix-like操作系统都会限制一个进程可使用的系统资源。这些限制可能会对MongoDB的操作产生负面影响，在安装时我们可能需要适当的调整。请参看[UNIX ulimit](https://docs.mongodb.com/manual/reference/ulimit/)以了解更详细的设置。

>注：从MongoDB 4.4版本开始，假如ulimit的值低于64000时，在启动的时候会报告相应的错误。

这里，我们以root用户启动，调整root用户下ulimit值如下；
<pre>
# ulimit -n 64000
# ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 14985
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 64000
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 14985
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
</pre>

###### 2.2.2 设置目录
1） **使用默认目录**

默认情况下，MongoDB会使用```mongod```账户来运行，并使用如下的默认目录：

* /var/lib/mongo(数据目录）
* /var/log/mongodb(日志目录）

通过yum安装时，包管理器会自动这两个默认目录。并且这两个目录的owner及group都是```mongod```。如下：
<pre>
# ls -al /var/lib/mongo
total 4
drwxr-xr-x.  2 mongod mongod    6 Dec  2 08:58 .
drwxr-xr-x. 55 root   root   4096 Jan 19 05:23 ..

# ls -al /var/log/mongodb
total 4
drwxr-xr-x.  2 mongod mongod   24 Jan 19 05:23 .
drwxr-xr-x. 21 root   root   4096 Jan 19 05:23 ..
-rw-r-----.  1 mongod mongod    0 Dec  2 08:58 mongod.log
</pre>

2) **使用非默认默认**

要使用非默认的数据目录及日志目录的话：

2.1) 创建新的目录

2.2) 修改配置文件/etc/mongod.conf的如下字段

  * storage.dbPath指定一个新的目录（比如： /some/data/directory）
  * systemLog.path指定一个新的日志目录（比如：/some/log/directory/mongod.log）

2.3) 确保运行mongodb的用户有对应目录的访问权限
{%highlight string %}
# sudo chown -R mongod:mongod <directory>
{% endhighlight %}
假如你想更改运行MongoDB进程的用户，你必须给新用户访问这些目录的权限。

2.4) 配置SELinux

这里我们直接关闭防火墙（更高级的配置请参看[Configure SeLinux](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-red-hat/)）：
<pre>
# systemctl stop firewalld.service 
# systemctl disable firewalld.service 
# setenforce 0 
# sed -i 's/SELINUX=enforcing/SELINUX=disabled/' /etc/selinux/config 
</pre>

###### 2.2.3 运行
遵循如下的步骤来运行MongoDB。如下的这些指令假设你使用的是默认的配置。

1） **Init system**

要运行和管理```mongod```进程，你需要使用到操作系统内置的[init system](https://docs.mongodb.com/manual/reference/glossary/#std-term-init-system)。当前新版的Linux操作系统都倾向于使用```systemd```(用systemctl命令）；而老版本的Linux系统使用的是```System V init```(用service命令)

假如你不确定你当前的操作系统使用的是哪一种init system的话，执行如下命令：
<pre>
# ps --no-headers -o comm 1
systemd
</pre>

这里我们当前Centos 7.0使用的是systemd。

2） **启动MongoDB**

执行如下命令启动[mongod](https://docs.mongodb.com/manual/reference/program/mongod/#mongodb-binary-bin.mongod)进程：
<pre>
# sudo systemctl start mongod
# ps -ef | grep mongod
mongod     4746      1  5 06:09 ?        00:00:00 /usr/bin/mongod -f /etc/mongod.conf
</pre>

假如在启动mongod过程中遇到如下错误：
{% highlight string %}
Failed to start mongod.service: Unit mongod.service not found.
{% endhighlight %}
执行如下命令：
<pre>
# sudo systemctl daemon-reload
# sudo systemctl start mongod
</pre>

mongoDB启动后，我们来看看其所使用的端口：
<pre>
# netstat -nlp | grep mongo
tcp        0      0 127.0.0.1:27017         0.0.0.0:*               LISTEN      4746/mongod         
unix  2      [ ACC ]     STREAM     LISTENING     44414    4746/mongod          /tmp/mongodb-27017.sock
</pre>

3) **检查MongoDB是否启动成功**

可以使用如下命令检查mongodb是否启动成功：
<pre>
# sudo systemctl status mongod
● mongod.service - MongoDB Database Server
   Loaded: loaded (/usr/lib/systemd/system/mongod.service; enabled; vendor preset: disabled)
   Active: active (running) since Wed 2022-01-19 06:09:39 PST; 2min 44s ago
     Docs: https://docs.mongodb.org/manual
  Process: 4743 ExecStart=/usr/bin/mongod $OPTIONS (code=exited, status=0/SUCCESS)
  Process: 4738 ExecStartPre=/usr/bin/chmod 0755 /var/run/mongodb (code=exited, status=0/SUCCESS)
  Process: 4735 ExecStartPre=/usr/bin/chown mongod:mongod /var/run/mongodb (code=exited, status=0/SUCCESS)
  Process: 4733 ExecStartPre=/usr/bin/mkdir -p /var/run/mongodb (code=exited, status=0/SUCCESS)
 Main PID: 4746 (mongod)
   CGroup: /system.slice/mongod.service
           └─4746 /usr/bin/mongod -f /etc/mongod.conf

Jan 19 06:09:38 localhost.localdomain systemd[1]: Starting MongoDB Database Server...
Jan 19 06:09:38 localhost.localdomain mongod[4743]: about to fork child process, waiting until server is ready for connections.
Jan 19 06:09:38 localhost.localdomain mongod[4743]: forked process: 4746
Jan 19 06:09:39 localhost.localdomain systemd[1]: Started MongoDB Database Server.
</pre>

可以使用如下命令来设置MongoDB开机启动：
<pre>
# sudo systemctl enable mongod
</pre>

4) **关闭MongoDB**

执行如下命令来关闭MongoDB:
<pre>
# sudo systemctl stop mongod
</pre>

5) **重启MongoDB**

执行如下命令来重启MongoDB:
<pre>
# sudo systemctl restart mongod
</pre>
我们可以查看/var/log/mongodb/mongod.log文件来了解mongod进行运行的一些重要信息。


6) **Begin using MongoDB**

在与[mongod](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)运行的同一台主机上，启动一个[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)。我们可以直接不添加任何选项启动mongosh，这样mongosh会自动连接本机的27017端口。

{% highlight string %}
# mongosh 
# mongosh --host 127.0.0.1 --port 27017
Current Mongosh Log ID: 61e82c6da95d3d0cd54cf7b8
Connecting to:          mongodb://127.0.0.1:27017/?directConnection=true&serverSelectionTimeoutMS=2000&appName=mongosh+1.1.9
Using MongoDB:          5.0.5
Using Mongosh:          1.1.9

For mongosh info see: https://docs.mongodb.com/mongodb-shell/

------
   The server generated these startup warnings when booting:
   2022-01-19T06:09:39.109-08:00: Access control is not enabled for the database. Read and write access to data and configuration is unrestricted
   2022-01-19T06:09:39.110-08:00: /sys/kernel/mm/transparent_hugepage/enabled is 'always'. We suggest setting it to 'never'
   2022-01-19T06:09:39.110-08:00: /sys/kernel/mm/transparent_hugepage/defrag is 'always'. We suggest setting it to 'never'
------

 test> 
{% endhighlight %}

与了解更多mongosh相关内容，请参看：

* [mongosh documentation](https://docs.mongodb.com/mongodb-shell/)

* [ Start Developing with MongoDB](https://api.mongodb.com/)

### 2.3 卸载MongoDB社区版
要完整的从操作系统中卸载MongoDB的话，你必须删除掉MongoDB应用程序本身、配置文件、以及包含数据和日志的目录。如下我们介绍一下必要的步骤。

1） **停止MongoDB**

执行如下命令停止MongoDB:
{% highlight string %}
# sudo service mongod stop
{% endhighlight %}

2) **移除安装包**

执行如下命令移除mongoDB安装包：
{% highlight string %}
# sudo yum erase $(rpm -qa | grep mongodb-org)
{% endhighlight %}

3) **删除数据目录**

执行如下命令删除数据目录和日志目录：
<pre>
# sudo rm -r /var/log/mongodb
# sudo rm -r /var/lib/mongo
</pre>

### 2.4 Additional Information

1) **Localhost Binding by Default**

默认情况下，MongoDB启动时bindIp会被设置为127.0.0.1，即被绑定到本地网络接口。这意味着```mongod```只能接受本地连接，远程客户端将不能够连接到```mongod```，并且mongod也不能够初始化一个[replica set](https://docs.mongodb.com/manual/reference/glossary/#std-term-replica-set)。除非我们在```mongod```启动时绑定到一个对外网卡上。

我们可以有两种方法配置：

* 修改MongoDB配置文件中的```bindIp```

* 通过命令行参数```--bind_ip```

>WARNING: 在你绑定到一个公有IP地址之前，确保unauthorized access不会影响到集群的安全。完整的安全列表建议，请参看[Security Checklist](https://docs.mongodb.com/manual/administration/security-checklist/)。至少，我们应该考虑[enabling authentication](https://docs.mongodb.com/manual/administration/security-checklist/#std-label-checklist-auth)以及[hardening network infrastructure](https://docs.mongodb.com/manual/core/security-hardening/)

更多关于bindIp的知识，请参看[IP Binding](https://docs.mongodb.com/manual/core/security-mongodb-configuration/)

2) **MongoDB Community Edition Packages**

MongoDB社区版有自己专用的仓库(repository)，官方支持如下一些安装包：

* mongodb-org： 是一个metapackage，用于自动的安装如下的一些组件(component packages)

* mongodb-org-database: 是一个metapackage，用于自动的安装如下组件

  * mongodb-org-server: 含有mongod守护进程，相关的初始化脚本，以及一个配置文件(/etc/mongod.conf)。你可以使用初始化脚本，以默认配置文件的方式来启动mongod
  * mongodb-org-mongos: 包含mongos守护进程
  * mongodb-org-shell: 包含经典mongo shell

* mongodb-mongosh: 包含MongoDB Shell(mongosh)

* mongodb-org-tools: 是一个metapackage，用于自动安装如下组件

  * mongodb-database-tools: 包含如下一些数据库工具
   
> [mongodump](https://docs.mongodb.com/database-tools/mongodump/#mongodb-binary-bin.mongodump)
> 
> [mongorestore](https://docs.mongodb.com/database-tools/mongorestore/#mongodb-binary-bin.mongorestore)
> 
>[bsondump](https://docs.mongodb.com/database-tools/bsondump/#mongodb-binary-bin.bsondump)
>
>[mongoimport](https://docs.mongodb.com/database-tools/mongoimport/#mongodb-binary-bin.mongoimport)
>
>[mongoexport](https://docs.mongodb.com/database-tools/mongoexport/#mongodb-binary-bin.mongoexport)
>
>[mongostat](https://docs.mongodb.com/database-tools/mongostat/#mongodb-binary-bin.mongostat)
>
>[mongotop](https://docs.mongodb.com/database-tools/mongotop/#mongodb-binary-bin.mongotop)
>
>[mongofiles](https://docs.mongodb.com/database-tools/mongofiles/#mongodb-binary-bin.mongofiles)

  * mongodb-org-database-tools-extra: 包含[install_compass](https://docs.mongodb.com/manual/reference/program/install_compass/#std-label-install-compass)脚本


## 3. 使用.tgz压缩包来安装MongoDB
本节我们介绍如何使用一个下载的```.tgz```压缩包来在Red Hat Enterprise Linux、Centos Linux或Oracle Linux上装MongoDB 5.0 Community Edition.

### 3.1 Considerations
1) **MongoDB Shell, mongosh**

当你使用```.tgz```压缩包来安装MongoDB时，你需要遵循[mongosh installation instructions](https://docs.mongodb.com/mongodb-shell/install/)来单独的下载和安装[mongosh](https://docs.mongodb.com/mongodb-shell/)


### 3.2 Install MongoDB Community Edition

###### 3.2.1 Prerequisites

首先需要安装MongoDB Community Edition的一些依赖：
<pre>
# sudo yum install libcurl openssl xz-libs
</pre>

###### 3.2.2 Procedure
遵循如下的步骤，使用```.tgz```来安装MongoDB社区版。



1） **Download the tarball**

在你安装了上述必要的依赖包之后，可以到[MongoDB Download Center](https://www.mongodb.com/try/download/community?tck=docs_server)下载对应的社区版本：

1. Version下拉列表, 选择对应的MongoDB版本

2. Platform下拉列表, 选择对应的操作系统和架构

3. Package下拉列表，选择tgz

这里我们执行如下命令下载mongodb-linux-x86_64-rhel70-5.0.5.tgz:
<pre>
# mkdir -p /opt/mongodb
# cd -p /opt/mongodb
# wget https://fastdl.mongodb.org/linux/mongodb-linux-x86_64-rhel70-5.0.5.tgz
</pre>

2) **解压安装包**

这里使用如下命令解压安装包：
<pre>
# tar -zxvf mongodb-linux-x86_64-rhel70-5.0.5.tgz
</pre>

3) **配置环境变量**

这里我们修改/etc/profile，添加如下配置：
<pre>
export MONGO_HOME = /opt/mongodb
export PATH = $PATH:$MONGO_HOME
</pre>

然后执行如下命令生效：
<pre>
# source /etc/profile
</pre>

> 注：我们使用软链接将其链接到/usr/local/bin目录也可以
> 
> sudo ln -s /path/to/the/mongodb-directory/bin/* /usr/local/bin/

4) **安装MongoDB Shell(mongosh)**

这里我们下载对应版本的mongodb shell安装包来进行安装：
<pre>
# mkdir /opt/mongosh
# wget https://fastdl.mongodb.org/linux/mongodb-shell-linux-x86_64-rhel70-5.0.5.tgz
# tar -zxvf mongodb-shell-linux-x86_64-rhel70-5.0.5.tgz
</pre>
解压安装完成后配置环境变量，修改/etc/profile:
<pre>
export MONGOSH_HOME=/opt/mongosh/bin/
export PATH = $PATH:$MONGOSH_HOME
</pre>

### 3.3 Run MongoDB Community Edition

###### 3.3.1 Prerequisites 

1.1) **ulimit**

关于ulimit配置，请参看上文。

###### 3.3.2 Directory Paths

这里我们创建如下目录：
<pre>
# sudo mkdir -p /var/lib/mongo
# sudo mkdir -p /var/log/mongodb
</pre>
	
默认情况下，MongoDB使用```mongod```用户来运行，因此我们要创建mongod用户和组，并更改对应目录的所有者和所属组：
<pre>
# sudo chown -R mongod:mongod /var/lib/mongo
# sudo chown -R mongod:mongod /var/log/mongodb
</pre>

###### 3.3.3 Procedure
1) **创建数据目录和日志目录**

<pre>
# sudo mkdir -p /var/lib/mongo
# sudo mkdir -p /var/log/mongodb
</pre>

假如你打算自己启动MongoDB的话，你需要确保有访问对应目录的权限：
<pre>
sudo chown `whoami` /var/lib/mongo     # Or substitute another user
sudo chown `whoami` /var/log/mongodb   # Or substitute another user
</pre>

2) **运行mongodb**

执行如下命令运行mongodb:
<pre>
# mongod --dbpath /var/lib/mongo --logpath /var/log/mongodb/mongod.log --fork
</pre>

3) **检查mongodb是否启动成功**

查看启动日志/var/log/mongod/mongod.log，看mongodb是否启动成功。如果出现如下信息表示启动正常：
{% highlight string %}
[initandlisten] waiting for connections on port 27017
{% endhighlight %}

4) **使用mongodb**

使用mongosh来连接MongoDB:
<pre>
# mongosh 
</pre>

<br />
<br />
**[参看]**:

1. [mongodb官网](https://www.mongodb.com/)

2. [mongodb manual](https://docs.mongodb.com/manual/)

2. [mongodb method](https://docs.mongodb.com/manual/reference/method/)

3. [mongodb command](https://docs.mongodb.com/manual/reference/command/find/)

4. [MongoDB Projection](https://blog.csdn.net/weixin_43031412/article/details/97632341)

5. [mongodb菜鸟教程](https://www.runoob.com/mongodb/mongodb-query.html)

6. [了解 MongoDB 看这一篇就够了](http://blog.itpub.net/31556440/viewspace-2672431/)


<br />
<br />
<br />

