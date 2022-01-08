---
layout: post
title: MongoDB数据库与集合
tags:
- database
categories: database
description:  MongoDB数据库与集合
---

本章介绍以下MongoDB数据库与collections相关内容，在此做个记录。


<!-- more -->



## 1. Databases and Collections

1) **Overview**

MongoDB将数据记录作为documents方式来进行存储（特别是[BSON documents](https://docs.mongodb.com/manual/core/document/#std-label-bson-document-format))，集中起来保存在一个[collections](https://docs.mongodb.com/manual/reference/glossary/#std-term-collection)中。一个数据库可以一个或多个documents collections。

2） **Databases**

在MongoDB中，database可以保存一个或多个documents集合。在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中，要选择一个数据库的话，可以使用```use <db>```语句，参看如下示例：
{% highlight string %}
>>> use myDB
{% endhighlight %}

* Create a Database

假如一个数据库不存在的话，则当你第一次存入数据时，MongoDB就会自动创建。因此，你可以switch到一个不存在的database，然后再mongosh中执行如下下命令：
{% highlight string %}
>>> use myNewDB

>>> db.myNewCollection1.insertOne( { x: 1 } )
{% endhighlight %}
上面的insertOne()操作会同时创建```myNewDB```数据库与```myNewCollection1```集合（假如相应数据库和集合不存在的话）。请确保database与collection的名称遵循MongoDB [Naming Restrictions](https://docs.mongodb.com/manual/reference/limits/#std-label-restrictions-on-db-names)。

3） **Collections**

MongoDB会将documents存放再collections中。collections与关系型数据库中的tables类似。

![mongodb-documents](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-collection.bakedsvg.svg)

* Create a Collection

假如一个collection不存在，MongoDB会在你第一次存入数据时创建该collection:
{% highlight string %}
db.myNewCollection2.insertOne( { x: 1 } )
db.myNewCollection3.createIndex( { y: 1 } )
{% endhighlight %}

上面insertOne()与createIndex()操作都会创建对应的collection(假如不存在的话）。请确保collection名称符合MongoDB [Naming Restrictions](https://docs.mongodb.com/manual/reference/limits/#std-label-restrictions-on-db-names)。

* Explicit Creation

MongoDB提供了db.createCollection()方法来显式创建一个collection，该方法有许多选项，比如设置maximum size或者documentation校验规则。假如你不指定这些选项的话，你不必显式的创建collection，因为MongoDB会在第一次存入数据时自动帮你创建。

要修改collection选项的话，请参看[collMod](https://docs.mongodb.com/manual/reference/command/collMod/#mongodb-dbcommand-dbcmd.collMod)

* Document Validation

>New in version 3.2.

默认情况下，collection并不要求其documents都具有相同的schema。比如，单个collection中的documents并不需要含有相同的fields，documents间相同field的数据类型也可以不同。

从MongoDB 3.2版本开始，在对collection进行更新与插入操作时可以强制进行[document validation rules](https://docs.mongodb.com/manual/core/schema-validation/)校验。想要了解更多细节，请参看[Schema Validation](https://docs.mongodb.com/manual/core/schema-validation/)。

* Modifying Document Structure

如果要改变collection中documents的结构，比如添加新的fields、移除已存在的fields、修改field值为新类型、更新documents为一个新的结构。

* Unique Identifiers

> New in version 3.6
> Note: featureCompatibilityVersion 在3.6及之后版本中必须被设置

collections会被指定一个不可变的UUID值。collection的UUID在replica set所有成员间保持一致。

要获取一个collection的UUID，可以执行```listCollections```命令或者db.getCollectionInfos(方法。

### 4.1 Views
MongoDB视图是一个可查询的object，其内容由[aggregation pipeline](https://docs.mongodb.com/manual/core/aggregation-pipeline/#std-label-aggregation-pipeline)在其他collections或views上生成。MongoDB并不会持久化视图内容到硬盘上。视图的内容会在客户端执行查询时根据需要进行计算。MongoDB可以控制客户端是否有查询视图的权限。MongoDB禁止在视图上执行写操作。

例如，通过视图你可以实现：

* 在雇员信息集中创建视图，并排除具体的雇员隐私信息。这样应用程序就可以通过查询视图，但获取不到雇员的隐私信息。

* 在摄像头数据集上创建视图，并添加一些经过计算的fields及metrics。应用程序可以使用简单的查询操作来查询数据

* 联合```库存```(inventory)与```历史订单```(order history)两个collection来创建一个视图。应用程序可以查询joined的数据，而不需要了解底层复杂的pipeline。

当客户端执行[视图查询](https://docs.mongodb.com/manual/core/views/#std-label-views-supported-operations)时，MongoDB会将客户端查询追加到底层的pipeline，然后将相应的结果返回给客户端。MongoDB也可能会返回的combined pipeline上运用[aggregation pipeline optimizations](https://docs.mongodb.com/manual/core/aggregation-pipeline-optimization/)。

###### 4.1.1 Create View

要创建或定义一个视图，可采用如下方法：

* 使用db.createCollection()方法或者create命令
{% highlight string %}
db.createCollection(
  "<viewName>",
  {
    "viewOn" : "<source>",
    "pipeline" : [<pipeline>],
    "collation" : { <collation> }
  }
)
{% endhighlight %}

* 使用db.createView()方法
{% highlight string %}
db.createView(
  "<viewName>",
  "<source>",
  [<pipeline>],
  {
    "collation" : { <collation> }
  }
)
{% endhighlight %}
>Note: 你必须在与source collection相同的数据库上创建视图
> 另外，在视图定义中pipeline不能包含$out及$merge stage。

###### 4.1.2 Behavior

在视图上可以执行如下行为：

1） **Read Only**

视图是只读的，不允许在视图上执行写操作，否则会报告相应的错误。

可以在视图上执行如下读取操作：

* db.collection.find()
* db.collection.findOne()
* db.collection.aggregate()
* db.collection.countDocuments()
* db.collection.estimatedDocumentCount()
* db.collection.count()
* db.collection.distinct()







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

8. [mongoDB数据库命令](https://docs.mongodb.com/manual/reference/command/)

9. [Role-Based Access Control](https://docs.mongodb.com/manual/core/authorization/#std-label-authorization)

10. [mongodb method](https://docs.mongodb.com/manual/reference/method/)

<br />
<br />
<br />

