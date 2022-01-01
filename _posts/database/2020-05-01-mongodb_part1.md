---
layout: post
title: MongoDB的安装及使用(1)
tags:
- database
categories: database
description:  MongoDB的安装及使用
---

如下是MongoDB官网对自身的介绍：
>MongoDB was founded in 2007 by Dwight Merriman, Eliot Horowitz and Kevin Ryan – the team behind DoubleClick.
>
>At the Internet advertising company DoubleClick (now owned by Google), the team developed and used many custom data stores to work around the shortcomings of existing databases. The business served 400,000 ads per second, but often struggled with both scalability and agility. Frustrated, the team was inspired to create a database that tackled the challenges it faced at DoubleClick.
>
>This was when MongoDB was born.


<!-- more -->

## 1. MongoDB产品矩阵
MongoDB公司目前主要有如下产品矩阵：

* Atlas

* Enterprise Advanced

* Community Edition

* Realm

下面我们分别对其进行简单介绍。

1） **Atlas**

MongoDB Atlas是由公司创始团队所开发的一款多云数据库服务(multi-cloud database service)。当我们需要在（不同供应商的）云服务器上构建弹性、高效的全球应用时，Atlas可以帮助我们简化数据库的部署和管理。如下图所示：

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/atlas-plp-hero.svg)

>More: https://docs.atlas.mongodb.com/

2) **Enterprise Advanced**

企业版提供了一系列的产品(products)和服务(services)来提高MongoDB数据的安全性、性能，以及帮助用户更好的管理MongoDB数据库。

* Database Management

在数据库管理方面，提供了相应的管理工具来简化MongoDB数据库的运维。主要包括： Automation、Monitoring、Optimization、Backups等

* Data and Business Protection 

提供先进的访问控制(access control)和数据安全特性来保护数据库。可以很容易的将已存在的安全设施、工具等与MongoDB继承。主要包括： Auditing、Encryption、Authentication、Commercial License

* Support and Services 

购买了MongoDB企业版的客户，可以获得MongoDB公司的技术支持与服务。

3) **Community Edition**

MongoDB社区版分布式数据库实现了灵活的文档数据模型(document date model)，并支持ad-hoc查询、二级索引、实时聚合等功能，从而为数据的访问与分析提供有力的支撑。

4） **Realm**

MongoDB Realm是一个开发平台，主要用于支持现代数据驱动的应用程序。可以使用Realm来构建mobile、web、desktop、以及IOT应用。

## 2. MongoDB介绍
欢迎使用MongoDB 5.0手册！MongoDB是一个文档数据库，旨在简化开发和扩展(scaling)。手册(manual)会介绍MongoDB关键概念(key concepts)、查询语言，并提供MongoDB操作及管理上的一些思考与流程。

MongoDB提供了本地部署与云(cloud-hosted)部署两种选项：

* 对于本地部署(locally hosted deployments), MongoDB又分为```社区版```与```企业版```
    * MongoDB社区版是[开源并免费使用](https://github.com/mongodb/mongo)的版本
	* MongoDB企业版作为MongoDB Enterprise Advanced订阅的一部分提供，并对MongoDB的部署提供全面的支持。此外，MongoDB企业版也增加了一些特性的支持，比如LDAP以及Kerberos支持、磁盘加密和审计。
	
* MongoDB Atlas是云中托管的 MongoDB Enterprise 服务选项，无需安装开销，并提供免费层级来开始使用。

### 2.1 文档数据库(Document Database)
在MongoDB中一条记录(record)就是一个文档(document)，其是由field/value对所组成的一个数据结构。MongoDB documents与JSON Objects类似。values字段可以为其他documents、arrays或arrays of documents。如下所示：

![mongodb-documents](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-document.bakedsvg.svg)

使用documents具有如下优点：

* 在许多编程语言中，documents对应于原生数据类型(native data types)

* 内嵌documents和数组可以降低昂贵连接(join)的需求

* 动态schema可以更好的支持多种形态

1） **集合/视图/按需实例化视图**

MongoDB将文档存储在集合中。集合类似于关系数据库中的表。另外，对于集合(collections)，MongoDB还支持：

* Read-only Views(Staring in MongoDB 3.4)

* On-Demand Materialized Views (Starting in MongoDB 4.2)

### 2.2 Key Features 
1) **High Performance¶**

MongoDB提供了高性能的数据持久化能力。特别是：

* 支持内嵌数据模型以降低数据库系统IO活动

* 通过索引实现高效的数据查询，并且索引的keys可以来自于内嵌documents与arrays

2) **Rich Query Language**

MongoDB提供了丰富的查询语言(query language)以支持读写操作(CRUD)，另外还支持如下：

* [Data Aggregation](https://docs.mongodb.com/manual/core/aggregation-pipeline/)

* [Text Search](https://docs.mongodb.com/manual/text-search/)和[Geospatial Queries](https://docs.mongodb.com/manual/tutorial/geospatial-tutorial/)

>注：请参看如下
> [SQL to MongoDB Mapping Chart](https://docs.mongodb.com/manual/reference/sql-comparison/)
> [SQL to Aggregation Mapping Chart](https://docs.mongodb.com/manual/reference/sql-aggregation-comparison/)

3) **High Availability**

MongoDB中实现的复制设施，称为[replica set](https://docs.mongodb.com/manual/replication/)，实现了：

* automatic failover

* data redundancy

一个```replica set```是一组维护相同数据集的MongoDB服务器，提供数据冗余，并增强数据的可用性。

4） **Horizontal Scalability**

MongoDB在核心功能层面提供水平扩展能力：

* 通过Sharding方式将数据存放到集群中

* 从3.4版本开始，MongoDB支持基于shard key的方式来创建zone。在一个平衡的集群中，MongoDB将zone所覆盖的读取和写入操作仅定向到zone中对应的分片。

5) **Support for Multiple Storage Engines**

MongoDB支持多存储引擎:

* [WiredTiger Storage Engine](https://docs.mongodb.com/manual/core/wiredtiger/) (including support for [Encryption at Rest](https://docs.mongodb.com/manual/core/security-encryption-at-rest/))

* [In-Memory Storage Engine](https://docs.mongodb.com/manual/core/inmemory/)

另外，MongoDB提供可插拔的存储引擎API以支持第三方开发新的存储引擎。





<br />
<br />
**[参看]**:

1. [mongodb官网](https://www.mongodb.com/)

2. [mongodb软件包官方下载](https://www.mongodb.com/try/download/community)

3. [mongodb官方文档](https://docs.mongodb.com/)

4. [mongodb中文网](https://www.mongodb.org.cn/)

5. [mongodb github](https://github.com/mongodb)

6. [MongoDB公司介绍](https://www.mongodb.com/en/company)

7. [MongoDB中文手册](https://docs.mongoing.com/)

<br />
<br />
<br />

