---
layout: post
title: mysql es数据同步
tags:
- go-language
categories: go-language
description: mysql es数据同步
---

本章我们主要记录一下使用go-mysql-elasticsearch的使用。go-mysql-elasticsearch是一个用于实现MySQL与ES之间自动数据同步的服务，首先其使用```mysqldump```来抓取一次原始数据，之后使用binlog来实现增量的数据同步。


<!-- more -->

## 1. 环境的搭建

### 1.1 mysqldump的安装

mysqldump存在于```mysql-community-client ```中，这里我们只需要安装mysql-communit-client即可，具体请参看mysql的安装。

### 1.2 go-mysql-elasticsearch的编译

1) 安装Go开发环境(1.9+)，并设置GOPATH

2) 下载go-mysql-elasticsearch

执行如下命令下载go-mysql-elasticsearch安装包：
<pre>
# echo $GOPATH
/root/workspace/gowork
# cd /root/workspace/gowork/src
# go get github.com/siddontang/go-mysql-elasticsearch
package github.com/siddontang/go-mysql-elasticsearch: no Go files in /root/workspace/gowork/src/github.com/siddontang/go-mysql-elasticsearch
# tree
.
└── github.com
    └── siddontang
        └── go-mysql-elasticsearch
            ├── clear_vendor.sh
            ├── cmd
            │   └── go-mysql-elasticsearch
            │       └── main.go
            ├── Dockerfile
            ├── elastic
            │   ├── client.go
            │   └── client_test.go
            ├── etc
            │   └── river.toml
            ├── go.mod
            ├── go.sum
            ├── LICENSE
            ├── Makefile
            ├── README.md
            └── river
                ├── config.go
                ├── master.go
                ├── river_extra_test.go
                ├── river.go
                ├── river_test.go
                ├── rule.go
                ├── status.go
                └── sync.go

8 directories, 19 files
</pre>

3) 编译go-mysql-elasticsearch
<pre>
# cd $GOPATH/src/github.com/siddontang/go-mysql-elasticsearch
# make
GO111MODULE=on go build -o bin/go-mysql-elasticsearch ./cmd/go-mysql-elasticsearch
go: finding github.com/siddontang/go v0.0.0-20180604090527-bdc77568d726
go: finding github.com/siddontang/go-log v0.0.0-20180807004314-8d05993dda07
go: finding github.com/siddontang/go-mysql v0.0.0-20190303113352-670f74e8daf5
go: finding github.com/BurntSushi/toml v0.3.1
go: finding github.com/juju/errors v0.0.0-20190207033735-e65537c515d7
go: finding github.com/shopspring/decimal v0.0.0-20180709203117-cd690d0c9e24
go: finding github.com/pingcap/errors v0.11.0
go: finding github.com/satori/go.uuid v1.2.0
go: downloading github.com/juju/errors v0.0.0-20190207033735-e65537c515d7
go: downloading github.com/siddontang/go-mysql v0.0.0-20190303113352-670f74e8daf5
go: downloading github.com/BurntSushi/toml v0.3.1
go: downloading github.com/siddontang/go v0.0.0-20180604090527-bdc77568d726
go: downloading github.com/siddontang/go-log v0.0.0-20180807004314-8d05993dda07
go: extracting github.com/juju/errors v0.0.0-20190207033735-e65537c515d7
go: extracting github.com/siddontang/go-log v0.0.0-20180807004314-8d05993dda07
go: extracting github.com/BurntSushi/toml v0.3.1
go: extracting github.com/siddontang/go v0.0.0-20180604090527-bdc77568d726
go: extracting github.com/siddontang/go-mysql v0.0.0-20190303113352-670f74e8daf5
go: downloading github.com/shopspring/decimal v0.0.0-20180709203117-cd690d0c9e24
go: downloading github.com/pingcap/errors v0.11.0
go: downloading github.com/satori/go.uuid v1.2.0
go: extracting github.com/pingcap/errors v0.11.0
go: extracting github.com/satori/go.uuid v1.2.0
go: extracting github.com/shopspring/decimal v0.0.0-20180709203117-cd690d0c9e24
# ls bin
go-mysql-elasticsearch
</pre>
说明： 上面使用go1.1.1新功能module来编译go-mysql-elasticsearch，其会用到```go.mod```文件来指定编译过程中需要导入哪些packge。然后在编译时就会自动下载对应的package，并解压到适当的目录（注意，这里解压时可能会对文件名做一些转换）

4) go-mysql-elasticsearch的启动
<pre>
# ./bin/go-mysql-elasticsearch -config=./etc/river.toml
</pre>
上面看到，go-mysql-elasticsearch的启动方式很简单，这里我们主要需要关注```配置文件```的写法，留作稍后讲解。

5） 所需权限

运行go-mysql-elasticsearch进行日志同步时，要求访问数据库的用户具有如下权限：
<pre>
# GRANT REPLICATION CLIENT ON *.* TO 'repl'@'%';
# GRANT REPLICATION SLAVE ON *.* TO 'repl'@'%';       
# GRANT SELECT ON *.* TO 'repl'@'%';
# GRANT RELOAD ON *.* TO 'repl'@'%';
</pre>

## 2. go-mysql-elasticsearch注意事项

在使用go-mysql-elasticsearch时需要注意如下一些事项：

* 所支持的MySQL版本小于8.0

* 所支持的ES版本小于6.0

* mysql binlog的格式必须为row格式

* MySQL binlog_row_image必须为```FULL```格式，假如使用```MINIMAL```或```noblob```的话，则在更新主键数据时有可能会造成一些字段的丢失。MariaDB只支持full row image格式。

* 在运行过程中修改表结构

* 需要同步的MySQL表要有主键(Primary Key)，当前已经支持了复合主键。例如，主键是(a,b)，那么我们将会使用```a:b```来作为key。主键数据将会作为Es中的'id'。

* 首先需要在Es中创建好相应的关联映射，因为通常默认的映射将不能够实现精确的查找

* mysqldump必须与go-mysql-elasticsearch运行在同一个节点。假如没有mysqldump的话，则go-mysql-elasticsearch将只会尝试同步binlog，而不会原来的初始数据

* 通常不建议再一个SQL语句中改变太多的行


## 3. go-mysql-elasticsearch配置文件

在具体介绍介绍配置文件(.toml)之前，我们先看看go-mysql-elasticsearch中自带的示例配置etc/river.toml:
{% highlight string %}
# MySQL address, user and password
# user must have replication privilege in MySQL.
my_addr = "127.0.0.1:3306"
my_user = "root"
my_pass = ""
my_charset = "utf8"

# Set true when elasticsearch use https
#es_https = false
# Elasticsearch address
es_addr = "127.0.0.1:9200"
# Elasticsearch user and password, maybe set by shield, nginx, or x-pack
es_user = ""
es_pass = ""

# Path to store data, like master.info, if not set or empty,
# we must use this to support breakpoint resume syncing. 
# TODO: support other storage, like etcd. 
data_dir = "./var"

# Inner Http status address
stat_addr = "127.0.0.1:12800"

# pseudo server id like a slave 
server_id = 1001

# mysql or mariadb
flavor = "mysql"

# mysqldump execution path
# if not set or empty, ignore mysqldump.
mysqldump = "mysqldump"

# if we have no privilege to use mysqldump with --master-data,
# we must skip it.
#skip_master_data = false

# minimal items to be inserted in one bulk
bulk_size = 128

# force flush the pending requests if we don't have enough items >= bulk_size
flush_bulk_time = "200ms"

# Ignore table without primary key
skip_no_pk_table = false

# MySQL data source
[[source]]
schema = "test"

# Only below tables will be synced into Elasticsearch.
# "t_[0-9]{4}" is a wildcard table format, you can use it if you have many sub tables, like table_0000 - table_1023
# I don't think it is necessary to sync all tables in a database.
tables = ["t", "t_[0-9]{4}", "tfield", "tfilter"]

# Below is for special rule mapping

# Very simple example
# 
# desc t;
# +-------+--------------+------+-----+---------+-------+
# | Field | Type         | Null | Key | Default | Extra |
# +-------+--------------+------+-----+---------+-------+
# | id    | int(11)      | NO   | PRI | NULL    |       |
# | name  | varchar(256) | YES  |     | NULL    |       |
# +-------+--------------+------+-----+---------+-------+
# 
# The table `t` will be synced to ES index `test` and type `t`.
[[rule]]
schema = "test"
table = "t"
index = "test"
type = "t"

# Wildcard table rule, the wildcard table must be in source tables 
# All tables which match the wildcard format will be synced to ES index `test` and type `t`.
# In this example, all tables must have same schema with above table `t`;
[[rule]]
schema = "test"
table = "t_[0-9]{4}"
index = "test"
type = "t"

# Simple field rule 
#
# desc tfield;
# +----------+--------------+------+-----+---------+-------+
# | Field    | Type         | Null | Key | Default | Extra |
# +----------+--------------+------+-----+---------+-------+
# | id       | int(11)      | NO   | PRI | NULL    |       |
# | tags     | varchar(256) | YES  |     | NULL    |       |
# | keywords | varchar(256) | YES  |     | NULL    |       |
# +----------+--------------+------+-----+---------+-------+
#
[[rule]]
schema = "test"
table = "tfield"
index = "test"
type = "tfield"

[rule.field]
# Map column `id` to ES field `es_id`
id="es_id"
# Map column `tags` to ES field `es_tags` with array type 
tags="es_tags,list"
# Map column `keywords` to ES with array type
keywords=",list"

# Filter rule 
#
# desc tfilter;
# +-------+--------------+------+-----+---------+-------+
# | Field | Type         | Null | Key | Default | Extra |
# +-------+--------------+------+-----+---------+-------+
# | id    | int(11)      | NO   | PRI | NULL    |       |
# | c1    | int(11)      | YES  |     | 0       |       |
# | c2    | int(11)      | YES  |     | 0       |       |
# | name  | varchar(256) | YES  |     | NULL    |       |
# +-------+--------------+------+-----+---------+-------+
#
[[rule]]
schema = "test"
table = "tfilter"
index = "test"
type = "tfilter"

# Only sync following columns
filter = ["id", "name"]

# id rule
#
# desc tid_[0-9]{4};
# +----------+--------------+------+-----+---------+-------+
# | Field    | Type         | Null | Key | Default | Extra |
# +----------+--------------+------+-----+---------+-------+
# | id       | int(11)      | NO   | PRI | NULL    |       |
# | tag      | varchar(256) | YES  |     | NULL    |       |
# | desc     | varchar(256) | YES  |     | NULL    |       |
# +----------+--------------+------+-----+---------+-------+
#
[[rule]]
schema = "test"
table = "tid_[0-9]{4}"
index = "test"
type = "t"
# The es doc's id will be `id`:`tag`
# It is useful for merge muliple table into one type while theses tables have same PK 
id = ["id", "tag"]
{% endhighlight %}
通过上面我们看到，基本上可分为```全局```、```[[source]]```、```[[rule]]```三种配置节点。下面我们就分别介绍一下。

### 3.1 全局配置

全局配置主要有如下配置项：

* my_addr： MySQL地址

* my_user: 连接MySQL的用户，通常要求具有复制(replication)权限

* my_pass: 连接MySQL用户对应的密码

* my_charset: 连接所使用的字符编码

* es_https: 假如elasticsearch使用https的话，请将此选项设置为true

* es_addr: es的地址

* es_user: es对应的用户

* es_pass: es对应用户的密码

* data_dir: 用户存放数据的目录，比如存放master.info信息。我们通常需要使用此目录来保存同步进程，以支持在同步中断后的恢复。

* stat_addr: 内部的http状态地址，我们可以通过此地址来查看当前的同步状态信息

* server_id: 一个pseudo server id

* flavor: 用于指定使用的是mysql还是mariadb数据库

* mysqldump： 用于指定mysqldump的执行路径。假如没有设置，或设置为空，则会忽略mysqldump

* skip_master_data: 假如我们没有权限来使用```mysqldump --master-data```时，我们必须将本选项设置为true，以进行跳过

* bulk_size: 一个bulk可以插入的最少条目数(item)

* flush_bulk_time: 假如当前我们并没有足够条目(item)，则在flush_bulk_time后会强制刷新所挂起的请求

* skip_no_pk_table： 跳过没有主键的数据库表

### 3.2 Source配置
在go-mysql-elasticsearch中，你必须在source节点中配置你需要将哪些数据库表同步到elasticsearch。source配置节点的基本格式如下：
{% highlight string %}
[[source]]
schema = "test"
tables = ["t1", t2]

[[source]]
schema = "test_1"
tables = ["t3", t4]
{% endhighlight %}
其中```schema```用于指定数据库名，而```tables```用于指定所要同步的表。

假如你想同步一个数据库中的所有表，那么你可以使用通配符：
<pre>
[[source]]
schema = "test"
tables = ["*"]

# When using an asterisk, it is not allowed to sync multiple tables
# tables = ["*", "table"]
</pre>


### 3.3 Rule配置
默认情况下，go-mysql-elasticsearch将会使用mysql的表名(table name)来作为es的索引及类型名称，使用mysql数据库表的字段名作为es的字段名。例如，有一个数据库表```blog```，则在ES中默认对应的index及type名都为```blog```，假如数据库表中的一个字段为```title```，则在es中默认对应的字段名也为```title```。

>注意： go-mysql-elasticsearch会使用小写的名称来作为ES的index及type名称。例如，你有一个数据库表，其名称为BLOG，则在es中对应的index及type名为blog

```Rule```可以让你改变这种名称映射规则，rule节点的配置格式如下：
{% highlight string %}
[[rule]]
schema = "test"
table = "t1"
index = "t"
type = "t"
parent = "parent_id"
id = ["id"]

    [rule.field]
    mysql = "title"
    elastic = "my_title"
{% endhighlight %}
在上面的例子中，我们使用新的index及type名称```t```来替换默认的```t1```，使用字段名```my_title```来替换```title```。

###### 3.3.1 Rule字段类型
为了将mysql中的一列映射成为Es中的另外一种类型，你可以通过如下方式来定义字段类型：
{% highlight string %}
[[rule]]
schema = "test"
table = "t1"
index = "t"
type = "t"

    [rule.field]
    // This will map column title to elastic search my_title
    title="my_title"

    // This will map column title to elastic search my_title and use array type
    title="my_title,list"

    // This will map column title to elastic search title and use array type
    title=",list"

    // If the created_time field type is "int", and you want to convert it to "date" type in es, you can do it as below
    created_time=",date"
{% endhighlight %}
上面的修正符```list```会把一个类似于```a,b,c```这样的mysql字符串转换成es的数组类型```{"a","b","c"}```，假如在es上你需要通过这些字段来进行过滤(filter)的话，这通常很有用处。

###### 3.3.2 Wildcard table
因为go-mysql-elasticsearch只允许你设置哪些mysql表需要进行同步，但假如有一个很大的数据库表被拆分成了很多子表的话，例如table_0000,table_0001,...,table_1023，那么如果我们要在一个rule中列出这些表，将会十分困难。

幸运的是go-mysql-elasticsearch支持表通配符：
{% highlight string %}
[[source]]
schema = "test"
tables = ["test_river_[0-9]{4}"]

[[rule]]
schema = "test"
table = "test_river_[0-9]{4}"
index = "river"
type = "river"
{% endhighlight %}
```test_river_[0-9]{4}```是一个表通配的定义，其代表着*test_river_0000*到*test_river_9999*，同时在```rule```节点设置的table也应该要相同。

在上面的例子中，假如你有1024个子表，则所有的这些子表都会被同步到es的river索引中。

###### 3.3.3 Parent-Child关系
当前已不使用，这里不做介绍。

###### 3.3.4 Filter fields
你可以使用filter来指定需要同步哪些字段：
{% highlight string %}
[[rule]]
schema = "test"
table = "tfilter"
index = "test"
type = "tfilter"

# Only sync following columns
filter = ["id", "name"]
{% endhighlight %}
在上面的例子中，我们只会同步MySQL数据库表的```id```和```name```列到Es。

###### 3.3.5 忽略没有主键(PK)的表
当你同步一个没有主键的table时，你会看到如下错误信息：
<pre>
schema.table must have a PK for a column
</pre>
你可以在配置文件中类忽略这些表：
{% highlight string %}
# Ignore table without a primary key
skip_no_pk_table = true
{% endhighlight %}


###### 3.3.6 Elasticsearch Pipeline
你可以使用Ingest Node Pipeline来预处理文档，比如进行Json字符串解码，合并字段（field)等等。
{% highlight string %}
[[rule]]
schema = "test"
table = "t1"
index = "t"
type = "_doc"

# pipeline id
pipeline = "my-pipeline-id"
{% endhighlight %}
注意： 你需要手动创建pipeline，并且Es版本应该>=5.0

## 4. 为什么不使用其他rivers
尽管也有一些其他的MySQL-Es同步方案，比如elasticsearch-river-jdbc、elasticsearch-river-mysql，但我仍使用Go来开发了go-mysql-elasticsearch，这是为什么？ 

* 定制化： 我想要可以指定哪些表可以进行同步，可以指定index及type的名称，甚至es中field的名称

* 使用binlog来实现增量更新，并且当服务重启的时候可以从上一次同步中断点进行恢复

* 作为一个通用的同步框架，不仅仅适用于MySQL-Elasticsearch之间的同步，稍加修改也可以实现MySQL-Redis、MySQL-Memcached之间的同步

* 支持数据库表通配，我们可能有很多类似于table_0000-table_1023这样的子表，但是共用同一个elasticsearch的index及type

<br />
<br />
**[参看]：**

1. [mysql数据实时同步到elasticsearch](https://www.jianshu.com/p/db9c9108c09a)

2. [Git go-mysql-elasticsearch](https://github.com/siddontang/go-mysql-elasticsearch)

3. [Go1.1.1新功能module的介绍及使用](https://blog.csdn.net/benben_2015/article/details/82227338)

4. [安全考虑，binlog_row_image建议尽量使用FULL](https://blog.csdn.net/actiontech/article/details/81701362)

5. [MySQL 5.7贴心参数之binlog_row_image](https://www.cnblogs.com/gomysql/p/6155160.html)

6. [Creating a project with Go modules](http://www.jetbrains.com/help/go/create-a-project-with-vgo-integration.html)

<br />
<br />
<br />

