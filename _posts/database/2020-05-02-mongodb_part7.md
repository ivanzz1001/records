---
layout: post
title: MongoDB CRUD操作
tags:
- database
categories: database
description:  MongoDB CRUD操作
---


MongoDB CRUD操作即创建(create)、读取(read)、更新(update)、删除(delete) documents。本文我们介绍一下这些操作，在此做个记录，以便后续查阅。


<!-- more -->

## 1. CRUD操作

### 1.1 Create操作
创建(create)或插入(insert)操作可以添加新[documents](https://docs.mongodb.com/manual/core/document/#std-label-bson-document-format)到一个[collection](https://docs.mongodb.com/manual/core/databases-and-collections/#std-label-collections)。假如collection当前不存在，那么插入操作会自动的创建该collection。

MongoDB提供了如下的方法来插入documents到collection:

* [db.collection.insertOne()](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#mongodb-method-db.collection.insertOne)*New in version 3.2*

* [db.collection.insertMany()](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#mongodb-method-db.collection.insertMany)*New in version 3.2*

在MongoDB中，插入操作的目标(target)只能是一个collection。

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-mongodb-insertOne.bakedsvg.svg)MongoDB针对```单个```(single)document的写操作均为原子性([atomic](https://docs.mongodb.com/manual/core/write-operations-atomicity/))的。

更多示例，请参看[Insert Documents](https://docs.mongodb.com/manual/tutorial/insert-documents/)。

### 1.2 读操作
读(Read)操作可以从collection中获取documents，比如从collection中查询documents。MongoDB提供如下的方法来从collection中读取documents:

* [db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)

可以指定[query filters or criteria](https://docs.mongodb.com/manual/tutorial/query-documents/#std-label-read-operations-query-argument)来标识需要返回哪些documents:

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-mongodb-find.bakedsvg.svg)


更多示例，请参看:

* [Query Documents](https://docs.mongodb.com/manual/tutorial/query-documents/)
* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)
* [Query an Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)
* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)

### 1.3 更新操作
更新(update)操作用于修改collection中已存在的documents。MongoDB提供了如下的方法来更新collection中documents:

* [db.collection.updateOne()](https://docs.mongodb.com/manual/reference/method/db.collection.updateOne/#mongodb-method-db.collection.updateOne) *New in version 3.2*
* [db.collection.updateMany()](https://docs.mongodb.com/manual/reference/method/db.collection.updateMany/#mongodb-method-db.collection.updateMany) *New in version 3.2*
* [db.collection.replaceOne()](https://docs.mongodb.com/manual/reference/method/db.collection.replaceOne/#mongodb-method-db.collection.replaceOne) *New in version 3.2*

MongoDB中，更新操作的目标(target)只能是一个collection。MongoDB针对```单个```(single)document的所有写操作均为原子性([atomic](https://docs.mongodb.com/manual/core/write-operations-atomicity/))的。

在执行更新操作时可以指定criterial, filters来更新特定的documents。这些[filters](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)与读操作的filters具有完全一样的语法。

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-mongodb-updateMany.bakedsvg.svg)

更多示例，请参看[Update Documents](https://docs.mongodb.com/manual/tutorial/update-documents/)

### 1.4 删除操作
删除(delete)操作可以从collection中移除documents。MongoDB提供了如下的方法来从一个collection中删除documents:

* [db.collection.deleteOne()](https://docs.mongodb.com/manual/reference/method/db.collection.deleteOne/#mongodb-method-db.collection.deleteOne) *New in version 3.2*
* [db.collection.deleteMany()](https://docs.mongodb.com/manual/reference/method/db.collection.deleteMany/#mongodb-method-db.collection.deleteMany) *New in version 3.2*

MongoDB中，删除操作的目标(target)只能是一个collection。MongoDB针对```单个```(single)document的所有写操作均为原子性([atomic](https://docs.mongodb.com/manual/core/write-operations-atomicity/))的。

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-mongodb-deleteMany.bakedsvg.svg)


更多示例，请参看[Delete Documents](https://docs.mongodb.com/manual/tutorial/remove-documents/)

### 1.5 Bulk Write
MongoDB提供了批量写操作。更多详细细节，请参看[Bulk Write Operations](https://docs.mongodb.com/manual/core/bulk-write-operations/)

## 2. Insert Documents








<br />
<br />
**[参看]**:

1. [mongodb官网](https://www.mongodb.com/)

2. [mongodb method](https://docs.mongodb.com/manual/reference/method/)

3. [mongodb command](https://docs.mongodb.com/manual/reference/command/find/)

4. [MongoDB Projection](https://blog.csdn.net/weixin_43031412/article/details/97632341)

5. [mongodb菜鸟教程](https://www.runoob.com/mongodb/mongodb-query.html)

6. [了解 MongoDB 看这一篇就够了](http://blog.itpub.net/31556440/viewspace-2672431/)


<br />
<br />
<br />

