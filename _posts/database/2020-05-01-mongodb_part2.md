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

### 1.1 Views
MongoDB视图是一个可查询的object，其内容由[aggregation pipeline](https://docs.mongodb.com/manual/core/aggregation-pipeline/#std-label-aggregation-pipeline)在其他collections或views上生成。MongoDB并不会持久化视图内容到硬盘上。视图的内容会在客户端执行查询时根据需要进行计算。MongoDB可以控制客户端是否有查询视图的权限。MongoDB禁止在视图上执行写操作。

例如，通过视图你可以实现：

* 在雇员信息集中创建视图，并排除具体的雇员隐私信息。这样应用程序就可以通过查询视图，但获取不到雇员的隐私信息。

* 在摄像头数据集上创建视图，并添加一些经过计算的fields及metrics。应用程序可以使用简单的查询操作来查询数据

* 联合```库存```(inventory)与```历史订单```(order history)两个collection来创建一个视图。应用程序可以查询joined的数据，而不需要了解底层复杂的pipeline。

当客户端执行[视图查询](https://docs.mongodb.com/manual/core/views/#std-label-views-supported-operations)时，MongoDB会将客户端查询追加到底层的pipeline，然后将相应的结果返回给客户端。MongoDB也可能会返回的combined pipeline上运用[aggregation pipeline optimizations](https://docs.mongodb.com/manual/core/aggregation-pipeline-optimization/)。

###### 1.1.1 Create View

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

###### 1.1.2 Behavior

在视图上可以执行如下行为：

1） **Read Only**

视图是只读的，不允许在视图上执行写操作，否则会报告相应的错误。

可以在视图上执行如下读取操作：

* [db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)
* db.collection.findOne()
* db.collection.aggregate()
* db.collection.countDocuments()
* db.collection.estimatedDocumentCount()
* db.collection.count()
* db.collection.distinct()

2) **Index Use and Sort Operations**

* 视图(Views)会使用底层Collection的索引

* 由于索引是在底层collection上，因此不能直接在视图(view)上创建、删除、重建索引，也不能在视图(views)上获得索引列表信息；

* 从MongoDB 4.4开始，当在视图(view)上执行[find命令](https://docs.mongodb.com/manual/reference/command/find/)时，可以指定按```$natural```排序。在之前老的版本，是不支持在视图上按```$natural```排序的。

* 对于blocking sort以及blocking group操作来说，视图的底层聚合 pipeline有100MB的内存限制。从MongoDB 4.4开始，当使用find命令在视图上执行blocking sort和blocking group操作时，可以使用``` allowDiskUse: true```选项，以允许MongoDB使用临时文件。

>注：在MongoDB 4.4之前，只允许在```aggregate```命令上使用allowDiskUse选项


3) **Projection Restrictions**

在视图上执行find()操作时，并不支持如下的projection操作符:

* [$](https://docs.mongodb.com/manual/reference/operator/projection/positional/#mongodb-projection-proj.-)

* [$elemMatch](https://docs.mongodb.com/manual/reference/operator/projection/elemMatch/#mongodb-projection-proj.-elemMatch)

* [$slice](https://docs.mongodb.com/manual/reference/operator/projection/slice/#mongodb-projection-proj.-slice)

* [$meta](https://docs.mongodb.com/manual/reference/operator/aggregation/meta/#mongodb-expression-exp.-meta)

4) **Immutable Name**

创建视图之后，我们不能对视图进行重命名。

5） **View Creation**

* 在进行读取操作(read operations)的时候会根据需要进行视图计算，MongoDB会将视图(view)的读取操作作为底层聚合管道(aggregation pipeline)的一部分。因此，视图并不支持如下操作：

  * [db.collection.mapReduce()](https://docs.mongodb.com/manual/reference/method/db.collection.mapReduce/#mongodb-method-db.collection.mapReduce)

  * ```$text```操作符，这是由于$text操作只在aggregation的第一个stage有效

  * ```$geoNear``` pipeline stage

* 假如使用了聚合管道(aggregation pipeline)来创建视图，这会抑制```_id```字段，因此在视图中的document没有```_id```字段；

当你在视图(views)上执行查询时：

* db.collection.find()方法中的filter、projection、sort、skip、limit等操作会被转化为等价的[aggregation pipeline stages](https://docs.mongodb.com/manual/reference/operator/aggregation-pipeline/#std-label-aggregation-pipeline-operator-reference)

* 转化后的aggregation pipeline stages会被添加到视图的[aggregation pipeline](https://docs.mongodb.com/manual/core/aggregation-pipeline/#std-label-aggregation-pipeline)。这并不会修改视图底层的pipeline（视图底层的pipeline是在创建视图时设置的）

* [Aggregation pipeline optimizer](https://docs.mongodb.com/manual/core/aggregation-pipeline-optimization/)会重新调整aggregation pipeline stages，以提高性能。这并不会影响查询结果。

6） **Sharded View**

假如底层的集合(collection)做了shard的话，那么我们认为视图(view)也是做了shard。因此，在执行[$lookup](https://docs.mongodb.com/manual/reference/operator/aggregation/lookup/#mongodb-pipeline-pipe.-lookup)和[$graphLookup](https://docs.mongodb.com/manual/reference/operator/aggregation/graphLookup/#mongodb-pipeline-pipe.-graphLookup)操作时不能为```from```字段指定一个sharded view。


7） **Views and Collation**

* 你可以在创建视图时指定一个默认的[collation](https://docs.mongodb.com/manual/reference/collation/)。假如不指定collation，则视图的默认collation为simple binary comparison collator。即，视图并不会继承集合的默认collation

* 对于在视图上执行字符串的比较，会采用视图默认的collation。如果一个操作试图修改或覆盖view的默认collation的话，将会失败。

* 假如是基于另外一个view来创建视图的话，所指定的collation必须与源view的collation一致。

* 假如一个聚合操作涉及到多个视图(view)，比如```$lookup```或者```$graphLookup```，那么要求这些视图具有相同的collation。

8) **Public View Definition**

用于列出(list) collection的操作，比如[db.getCollectionInfo()](https://docs.mongodb.com/manual/reference/method/db.getCollectionInfos/#mongodb-method-db.getCollectionInfos)、[db.getCollectionNames()](https://docs.mongodb.com/manual/reference/method/db.getCollectionNames/#mongodb-method-db.getCollectionNames)，都会在对应的输出中包含相应的视图信息。

>IMPORTANT:
>
> 视图的定义是public的，在视图上执行db.getCollectionInfos()、explain等操作时输出将会含有视图定义时对应的pipeline。因此，避免在视图定义时直接引用一些敏感的字段和值。

###### 1.1.3 Drop a view
要删除一个视图的话，可以在视图上执行[ db.collection.drop()](https://docs.mongodb.com/manual/reference/method/db.collection.drop/#mongodb-method-db.collection.drop)方法。

###### 1.1.4 Modify a View
你可以通过删除，然后再重建的方式来修改视图，也可以通过[collMod](https://docs.mongodb.com/manual/reference/command/collMod/#mongodb-dbcommand-dbcmd.collMod)命令来修改视图。

###### 1.1.5 Supported Operations

如下的操作提供对视图的支持（上文提到可能会有一些限制）：

<pre>
  Commands                    Methods
=================================================================
                         db.createCollection()  
  create 
                         db.createView()

-----------------------------------------------------------------
  collMod

------------------------------------------------------------------
                         db.getCollection()
                         db.getCollectionInfos()
                         db.getCollectionNames()
 
------------------------------------------------------------------
                         db.collection.aggregate()
                         db.collection.find()
  find                   db.collection.findOne()
  distinct               db.collection.countDocuments()
  count                  db.collection.estimatedDocumentCount()
                         db.collection.count()
                         db.collection.distinct()
</pre>

### 1.2 On-Demand Materialized Views

从4.2版本开始，MongoDB对于[aggregation pipeline](https://docs.mongodb.com/manual/core/aggregation-pipeline/)添加了[$merge stage](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#mongodb-pipeline-pipe.-merge)。该stage可以将pipeline的结果合并到一个已存在的collection上，而不是完全的替换该collection。此功能允许用户创建on-demand materialized views，当对应的pipeline运行时视图也会得到更新。


###### 1.2.1 示例
假设在2019年1月末，```bakesales```这个collection所含有的销售信息如下：
{% highlight string %}
db.bakesales.insertMany( [
   { date: new ISODate("2018-12-01"), item: "Cake - Chocolate", quantity: 2, amount: new NumberDecimal("60") },
   { date: new ISODate("2018-12-02"), item: "Cake - Peanut Butter", quantity: 5, amount: new NumberDecimal("90") },
   { date: new ISODate("2018-12-02"), item: "Cake - Red Velvet", quantity: 10, amount: new NumberDecimal("200") },
   { date: new ISODate("2018-12-04"), item: "Cookies - Chocolate Chip", quantity: 20, amount: new NumberDecimal("80") },
   { date: new ISODate("2018-12-04"), item: "Cake - Peanut Butter", quantity: 1, amount: new NumberDecimal("16") },
   { date: new ISODate("2018-12-05"), item: "Pie - Key Lime", quantity: 3, amount: new NumberDecimal("60") },
   { date: new ISODate("2019-01-25"), item: "Cake - Chocolate", quantity: 2, amount: new NumberDecimal("60") },
   { date: new ISODate("2019-01-25"), item: "Cake - Peanut Butter", quantity: 1, amount: new NumberDecimal("16") },
   { date: new ISODate("2019-01-26"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") },
   { date: new ISODate("2019-01-26"), item: "Cookies - Chocolate Chip", quantity: 12, amount: new NumberDecimal("48") },
   { date: new ISODate("2019-01-26"), item: "Cake - Carrot", quantity: 2, amount: new NumberDecimal("36") },
   { date: new ISODate("2019-01-26"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") },
   { date: new ISODate("2019-01-27"), item: "Pie - Chocolate Cream", quantity: 1, amount: new NumberDecimal("20") },
   { date: new ISODate("2019-01-27"), item: "Cake - Peanut Butter", quantity: 5, amount: new NumberDecimal("80") },
   { date: new ISODate("2019-01-27"), item: "Tarts - Apple", quantity: 3, amount: new NumberDecimal("12") },
   { date: new ISODate("2019-01-27"), item: "Cookies - Chocolate Chip", quantity: 12, amount: new NumberDecimal("48") },
   { date: new ISODate("2019-01-27"), item: "Cake - Carrot", quantity: 5, amount: new NumberDecimal("36") },
   { date: new ISODate("2019-01-27"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") },
   { date: new ISODate("2019-01-28"), item: "Cookies - Chocolate Chip", quantity: 20, amount: new NumberDecimal("80") },
   { date: new ISODate("2019-01-28"), item: "Pie - Key Lime", quantity: 3, amount: new NumberDecimal("60") },
   { date: new ISODate("2019-01-28"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") },
] );
{% endhighlight %}

1) **定义On-Demand Materialized View**

如下的```updateMonthlySales```函数定义了```monthlybakesales ```生成视图，其包含有每个月的销售信息统计。在下面的例子中，函数接受一个```日期```参数来更新从哪一天开始的月销售额信息：
{% highlight string %}
updateMonthlySales = function(startDate) {
   db.bakesales.aggregate( [
      { $match: { date: { $gte: startDate } } },
      { $group: { _id: { $dateToString: { format: "%Y-%m", date: "$date" } }, sales_quantity: { $sum: "$quantity"}, sales_amount: { $sum: "$amount" } } },
      { $merge: { into: "monthlybakesales", whenMatched: "replace" } }
   ] );
};
{% endhighlight %}

* ```$match```阶段过滤大于等于startDate的销售数据

* ```$group```阶段按月为单位对销售数据进行分组。此阶段产生的输出形式如下：
{% highlight string %}
{ "_id" : "<YYYY-mm>", "sales_quantity" : <num>, "sales_amount" : <NumberDecimal> }
{% endhighlight %}


* ```$merge```阶段将将对应的文档输出信息写入到```monthlybakesales```集合中。基于```_id```字段(对于unshared collections，默认为```_id```字段），此阶段会检查聚合(aggregation)结果中的document是否匹配集合中已存在的document:

  * 当有一个匹配的话（比如对应year-month的document已经存在），则本阶段会使用聚合中的document来替换原来老的document;

  * 当并没有相匹配的，则此阶段会插入一个新的document到集合中

2) **Perform Initial Run**

初次运行时，你可以传递```new ISODate("1970-01-01")```:
{% highlight string %}
updateMonthlySales(new ISODate("1970-01-01"));
{% endhighlight %}

在初始运行之后，```monthlybakesales```包含如下documents。比如执行db.monthlybakesales.find().sort( { _id: 1 } )查询：
{% highlight string %}
{ "_id" : "2018-12", "sales_quantity" : 41, "sales_amount" : NumberDecimal("506") }
{ "_id" : "2019-01", "sales_quantity" : 86, "sales_amount" : NumberDecimal("896") }
{% endhighlight %}

3) **Refresh Materialized View**

假设在2019年2月的第一周，```bakesales```集合发生了改变，有新的销售信息更新。特别是增加了1月和2月的销售额：
{% highlight string %}
db.bakesales.insertMany( [
   { date: new ISODate("2019-01-28"), item: "Cake - Chocolate", quantity: 3, amount: new NumberDecimal("90") },
   { date: new ISODate("2019-01-28"), item: "Cake - Peanut Butter", quantity: 2, amount: new NumberDecimal("32") },
   { date: new ISODate("2019-01-30"), item: "Cake - Red Velvet", quantity: 1, amount: new NumberDecimal("20") },
   { date: new ISODate("2019-01-30"), item: "Cookies - Chocolate Chip", quantity: 6, amount: new NumberDecimal("24") },
   { date: new ISODate("2019-01-31"), item: "Pie - Key Lime", quantity: 2, amount: new NumberDecimal("40") },
   { date: new ISODate("2019-01-31"), item: "Pie - Banana Cream", quantity: 2, amount: new NumberDecimal("40") },
   { date: new ISODate("2019-02-01"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") },
   { date: new ISODate("2019-02-01"), item: "Tarts - Apple", quantity: 2, amount: new NumberDecimal("8") },
   { date: new ISODate("2019-02-02"), item: "Cake - Chocolate", quantity: 2, amount: new NumberDecimal("60") },
   { date: new ISODate("2019-02-02"), item: "Cake - Peanut Butter", quantity: 1, amount: new NumberDecimal("16") },
   { date: new ISODate("2019-02-03"), item: "Cake - Red Velvet", quantity: 5, amount: new NumberDecimal("100") }
] )
{% endhighlight %}
这时，如果要刷新```monthlybakesales```在1月和2月的数据的话，可以再次执行updateMonthlySales()函数以返回aggregation pipeline，函数的起始时间可设置为```new ISODate("2019-01-01")```:
{% highlight string %}
updateMonthlySales(new ISODate("2019-01-01"));
{% endhighlight %}
执行完成之后，```monthlybakesales```的内容就得到了更新，以反映bakesales集合中最近的数据。执行db.monthlybakesales.find().sort( { _id: 1 } )返回的结果为：
{% highlight string %}
{ "_id" : "2018-12", "sales_quantity" : 41, "sales_amount" : NumberDecimal("506") }
{ "_id" : "2019-01", "sales_quantity" : 102, "sales_amount" : NumberDecimal("1142") }
{ "_id" : "2019-02", "sales_quantity" : 15, "sales_amount" : NumberDecimal("284") }
{% endhighlight %}


###### 1.2.2 Additional Information

```$merge```阶段(stage):

* 可以将结果输出到相同或不同的数据库；

* 假如输出集合(collection)不存在的话，此阶段会自动创建对应的集合；

* 可以将结果合并(insert new documents, merge documents, replace documents, keep existing documents, fail the operation, process documents with a custom update pipeline)到一个已存在的集合中

* 可以将结果输出到一个sharded collection中

关于[$merge](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#mongodb-pipeline-pipe.-merge)，可查看如下更多内容：

* More information on [$merge](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#mongodb-pipeline-pipe.-merge) and available options

* Example: [On-Demand Materialized View: Initial Creation](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#std-label-merge-mat-view-init-creation)

* Example: [On-Demand Materialized View: Update/Replace Data](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#std-label-merge-mat-view-refresh)

* Example: [Only Insert New Data](https://docs.mongodb.com/manual/reference/operator/aggregation/merge/#std-label-merge-mat-view-insert-only)


### 1.3 Capped Collections

###### 1.3.1 Overview
[capped collections](https://docs.mongodb.com/manual/reference/glossary/#std-term-capped-collection)是一种固定大小的collection，其支持

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

