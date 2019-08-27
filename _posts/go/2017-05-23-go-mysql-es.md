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

### 3.3 Rule配置






<br />
<br />
**[参看]：**

1. [mysql数据实时同步到elasticsearch](https://www.jianshu.com/p/db9c9108c09a)

2. [Git go-mysql-elasticsearch](https://github.com/siddontang/go-mysql-elasticsearch)

3. [Go1.1.1新功能module的介绍及使用](https://blog.csdn.net/benben_2015/article/details/82227338)

4. [安全考虑，binlog_row_image建议尽量使用FULL](https://blog.csdn.net/actiontech/article/details/81701362)

5. [MySQL 5.7贴心参数之binlog_row_image](https://www.cnblogs.com/gomysql/p/6155160.html)

<br />
<br />
<br />

