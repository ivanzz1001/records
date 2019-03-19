---
layout: post
title: MySQL服务器模式及相关内置变量
tags:
- database
categories: database
description: mysql数据库基础
---


本章我们主要包含两部分的内容：

* MySQL服务器模式

* MySQL内置变量


<!-- more -->


## 1. MySQL服务器模式
不同的MySQL客户端可以通过不同的模式操作MySQL Server。DBA可以设置一个全局模式，而每个应用程序可以根据需要为相应的会话设置不同的模式。

MySQL操作模式会影响到SQL的语法和相应的SQL语句的校验。

### 1.1 设置SQL模式
默认情况下SQL的模式是```NO_ENGINE_SUBSTITUTION```。如果要在MySQL Server启动的时候就设置好相应的SQL模式的话，可以使用```--sql-mode=<modes>```命令行选项来进行设置，也可以在MySQL配置文件中通过```sql-mode=<modes>```来进行配置。```<modes>```是由一系列由空格分隔的不同的模式组成。如果要清除SQL模式，则只需要在启动时传递```--sql-mode=""```或者在配置文件中配置```sql-mode=""```。

如果需要在运行时更改SQL模式，那么可以设置```全局```与```会话``` sql_mode系统变量：
<pre>
SET GLOBAL sql_mode = 'modes';
SET SESSION sql_mode = 'modes';
</pre>
对于设置全局变量，需要有```SUPER```权限，并且会影响到之后连接的所有客户端。对于设置```会话```变量，则只会影响到当前客户端。每个客户端都可以改变该会话的sql_mode。

可以通过如下命令来获取当前```全局```及```会话```sql_mode:
<pre>
SELECT @@GLOBAL.sql_mode;
SELECT @@SESSION.sql_mode;
</pre>

### 1.2 最重要的SQL模式
SQL有很多模式，下面我们介绍几种常用的重要的SQL模式：

* ANSI: 该模式会改变相应的语法和操作行为，以使最接近标准的SQL。它是一种特殊的```组合模式```(combination modes)。

* STRICT_TRANS_TABLES: 假如一个值并不能插入到一个“事务表”中，那么中断该语句的执行。

* TRANDITIONAL： 使MySQL接近于传统的SQL数据库系统。简单的描述即为“在插入错误的值到一列时直接返回错误，而不是警告”。

## 2. 获取服务器元数据
MySQL有很多元数据，这里我们列出几个常用的：

* **SELECT VERSION()**: 服务器版本信息

* **SELECT DATABASE()**: 当前数据库名(或者返回为NULL)

* **SELECT USER()**: 当前用户名

* **SHOW STATUS**: 服务器状态

* **SHOW VARIABLES**: 获取当前会话的配置变量



## 3. 修改MySQL系统变量
下面以设置MySQL 系统变量```wait_timeout```为例。

### 3.1 设置全局变量

**1） 修改参数文件，然后重启MySQL**
{% highlight string %}
# vi /etc/my.cnf

[mysqld]

wait_timeout=10

# service mysqld restart
{% endhighlight %}
此种方法太过生硬，并且要重启MySQL，一般不推荐。


**2) 在命令行通过SET来设置，然后再修改参数文件**

如果要修改全局变量，必须要显示指定```GLOBAL```或者```@@global.```，同时必须要有```SUPER```权限：
{% highlight string %}
mysql> SET GLOBAL wait_timeout=10;

or

mysql> set @@global.wait_timeout=10;
{% endhighlight %}
然后通过下面的命令查看设置是否成功：
{% highlight string %}
然后查看设置是否成功:

mysql> SELECT @@global.wait_timeout=10;

or

mysql> SHOW GLOBAL variables like 'wait_timeout';
+---------------+-------+

| Variable_name | Value |

+---------------+-------+

| wait_timeout  | 10    | 

+---------------+-------+
{% endhighlight %}
如果查询时使用的是```SHOW VARIABLES```的话，会发现设置并没有生效，除非重新登录再查看。这是因为使用```SHOW VARIABLES```的话就等同于使用```SHOW SESSION VARIABLES```，查询的是会话变量，只有使用```SHOW GLOBAL VARIABLES```查询的才是全局变量。如果仅仅想修改会话变量的话，可以使用类似```SET wait_timeout=10;```或```SET SESSION wait_timeout=10;````这样的语法。

当前只修改了正在运行的MySQL实例参数，但下次重启mysqld又会回到默认值，所以别忘了修改参数文件：
{% highlight string %}
# vi /etc/my.cnf

[mysqld]

wait_timeout=10
{% endhighlight %}


### 3.2 修改会话变量
如果要修改会话变量值，可以指定```SESSION```或者```LOCAL```关键字，或者通过```@@session```、```@@local```、```@@```限定符， 又或者不加任何关键字与限定符。例如：
{% highlight string %}
SET SESSION wait_timeout = 10;

or

SET LOCAL wait_timeout = 10;

or

SET @@session.wait_timeout = 10;

or

SET @@local.wait_timeout = 10;

or

SET @@wait_timetout = 10;

or 

SET wait_timeout = 10;
{% endhighlight %} 
然后查看设置是否成功：
{% highlight string %}
mysql> select @@wait_timeout;

or

mysql> select @@session.wait_timeout;

or

mysql> select @@local.wait_timeout;

or

mysql> show variables like 'wait_timeout';

or

mysql> show local variables like 'wait_timeout';

or

mysql> show session variables like 'wait_timeout';

+---------------+-------+

| Variable_name | Value |

+---------------+-------+

| wait_timeout  | 10    | 

+---------------+-------+
{% endhighlight %}

另外，如果要将一个全局系统变量设置为MySQL编译时的默认值，或者将一个session系统变量设置为当前的全局值，可以将该变量的值设置为```DEFAULT```。例如：
{% highlight string %}
SET @@session.max_join_size = DEFAULT;
SET @@session.max_join_size = @@global.max_join_size;
{% endhighlight %}



<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

