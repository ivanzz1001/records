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


## 3. go-mysql-elasticsearch配置文件



<br />
<br />
**[参看]：**

1. [mysql数据实时同步到elasticsearch](https://www.jianshu.com/p/db9c9108c09a)

2. [Git go-mysql-elasticsearch](https://github.com/siddontang/go-mysql-elasticsearch)

3. [Go1.1.1新功能module的介绍及使用](https://blog.csdn.net/benben_2015/article/details/82227338)

<br />
<br />
<br />

