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
>Note: 假如collection当前不存在，insert操作会创建对应的collection

###### Insert a Single Document
[db.collection.insertOne()](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#mongodb-method-db.collection.insertOne)可以将单个document插入到collection。

如下的示例插入一个新的document到```inventory```这个collection中。假如document未指定```_id```字段的话，MongoDB会添加一个```_id```字段，并赋予一个ObjectId值。参看[Insert Behavior](https://docs.mongodb.com/manual/tutorial/insert-documents/#std-label-write-op-insert-behavior)。

{% highlight string %}
db.inventory.insertOne(
   { item: "canvas", qty: 100, tags: ["cotton"], size: { h: 28, w: 35.5, uom: "cm" } }
)
{% endhighlight %}
[insertOne()](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#mongodb-method-db.collection.insertOne)方法会返回新插入的document的```_id```字段值。要查看返回的document示例，请参看[ db.collection.insertOne() reference](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#std-label-insertOne-examples)

如果要查询刚刚插入的document，执行[query the collection](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)
{% highlight string %}
db.inventory.find( { item: "canvas" } )
{% endhighlight %}

###### Insert Multiple Documents 

> New in version 3.2.

[db.collection.insertMany()](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#mongodb-method-db.collection.insertMany)可以插入多个document到collection中。可以传递一个document数组作为参数。

如下的示例插入3个documents到```inventory```集合中。假如document并未指定```_id```字段的话，那么MongoDB会帮助为每个document生成一个```_id```字段，值为ObjectId类型。参看[Insert Behavior](https://docs.mongodb.com/manual/tutorial/insert-documents/#std-label-write-op-insert-behavior)。

{% highlight string %}
db.inventory.insertMany([
   { item: "journal", qty: 25, tags: ["blank", "red"], size: { h: 14, w: 21, uom: "cm" } },
   { item: "mat", qty: 85, tags: ["gray"], size: { h: 27.9, w: 35.5, uom: "cm" } },
   { item: "mousepad", qty: 25, tags: ["gel", "blue"], size: { h: 19, w: 22.85, uom: "cm" } }
])
{% endhighlight %}

[insertMany()](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#mongodb-method-db.collection.insertMany)会返回一个document，里面包含```新插入```的documents的```_id```。参看[reference](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#std-label-insertMany-examples)以了解相关示例。

要查询所插入的documents的话，执行[query the collection](https://docs.mongodb.com/manual/tutorial/query-documents/#std-label-read-operations-query-document)。

{% highlight string %}
db.inventory.find( {} )
{% endhighlight %}

###### Insert Behavior

1) **Collection Creation**

假如对应的collection不存在的话，insert操作会创建对应的collection。

2） **_id字段**

在MongoDB中，collection中的每一个document都需要一个唯一的```_id```字段来作为primary key。假如所插入的document未指定```_id```字段的话，MongoDB driver会自动为```_id```字段生成一个[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#std-label-objectid)。

当执行update操作时，如果[upsert: true](https://docs.mongodb.com/manual/reference/method/db.collection.update/#std-label-upsert-parameter)被设置的话，当执行的结果是插入document的话，也同样会生成```_id```字段。

3) **Atomicity**

在MongoDB中，对于```单个```document来说所有的写操作都是原子性。更多关于MongoDB及原子操作，请参看[Atomicity and Transactions](https://docs.mongodb.com/manual/core/write-operations-atomicity/)。

4） **Write Acknowledgement**

对于数据库的写操作，我们可以指定MongoDB的ack响应级别。参看[Write Concern](https://docs.mongodb.com/manual/reference/write-concern/)


### 2.1 Insert Methods
MongoDB提供了如下的方法来向collection中插入documents:

* [db.collection.insertOne()](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#mongodb-method-db.collection.insertOne) 插入一个document到collection

* [db.collection.insertMany()](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#mongodb-method-db.collection.insertMany) 插入多个document到collection

###### 2.2.1 Additional Methods for Inserts

如下的方法也可以插入documents到collection:

* [db.collection.updateOne()](https://docs.mongodb.com/manual/reference/method/db.collection.updateOne/#mongodb-method-db.collection.updateOne) when used with the upsert: true option.
* [db.collection.updateMany()](https://docs.mongodb.com/manual/reference/method/db.collection.updateMany/#mongodb-method-db.collection.updateMany) when used with the upsert: true option.
* [db.collection.findAndModify()](https://docs.mongodb.com/manual/reference/method/db.collection.findAndModify/#mongodb-method-db.collection.findAndModify) when used with the upsert: true option.
* [db.collection.findOneAndUpdate()](https://docs.mongodb.com/manual/reference/method/db.collection.findOneAndUpdate/#mongodb-method-db.collection.findOneAndUpdate) when used with the upsert: true option.
* [db.collection.findOneAndReplace()](https://docs.mongodb.com/manual/reference/method/db.collection.findOneAndReplace/#mongodb-method-db.collection.findOneAndReplace) when used with the upsert: true option.
* [db.collection.bulkWrite()](https://docs.mongodb.com/manual/reference/method/db.collection.bulkWrite/#mongodb-method-db.collection.bulkWrite).

## 3. Query Documents
本节我们提供一些使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法的示例来演示查询操作。例子使用的collection为```inventory```。首先我们向集合中插入一些数据，执行如下：
{% highlight string %}
db.inventory.insertMany([
   { item: "journal", qty: 25, size: { h: 14, w: 21, uom: "cm" }, status: "A" },
   { item: "notebook", qty: 50, size: { h: 8.5, w: 11, uom: "in" }, status: "A" },
   { item: "paper", qty: 100, size: { h: 8.5, w: 11, uom: "in" }, status: "D" },
   { item: "planner", qty: 75, size: { h: 22.85, w: 30, uom: "cm" }, status: "D" },
   { item: "postcard", qty: 45, size: { h: 10, w: 15.25, uom: "cm" }, status: "A" }
]);
{% endhighlight %}

###### Select All Documents in a Collection
要查询collection中所有的documents，可以向find()方法传递一个empty document作为查询过滤器。查询过滤器(query filter)决定了查询准则：
{% highlight string %}
db.inventory.find( {} )
{% endhighlight %}
上面的操作对应于如下SQL语句：
<pre>
SELECT * FROM inventory
</pre>
更多关于find()方法的语法信息，请参看[find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)

###### Specify Equality Condition
要指定```相等条件```(equality conditions)，在[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)中使用```<field>:<value>```:
{% highlight string %}
{ <field1>: <value1>, ... }
{% endhighlight %}

下面的例子从```inventory```这个collection中查询```status```等于```D```的记录：
{% highlight string %}
db.inventory.find( { status: "D" } )
{% endhighlight %}
该操作对应于如下的SQL语句：
<pre>
SELECT * FROM inventory WHERE status = "D"
</pre>

###### Specify Conditions Using Query Operators
一个[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)可以通过如下形式来使用[query operators](https://docs.mongodb.com/manual/reference/operator/query/#std-label-query-selectors):
{% highlight string %}
{ <field1>: { <operator1>: <value1> }, ... }
{% endhighlight%}
下面的列子获取```inventory```这个collection中所有```status```等于```"A"```或```D```的记录：
{% highlight string %}
db.inventory.find( { status: { $in: [ "A", "D" ] } } )
{% endhighlight %}

>Note: 上面的例子中尽管可以使用[$or](https://docs.mongodb.com/manual/reference/operator/query/or/#mongodb-query-op.-or) operator，但是对于同一字段进行```相等```比较时，建议采用[$in](https://docs.mongodb.com/manual/reference/operator/query/in/#mongodb-query-op.-in)。

该操作对应于如下SQL语句：
{% highlight string %}
SELECT * FROM inventory WHERE status in ("A", "D")
{% endhighlight %}

参看[Query and Projection Operators](https://docs.mongodb.com/manual/reference/operator/query/)以了解MongoDB query operators的完整列表。

###### Specify ```AND``` Conditionsicons

一个复合查询可以指定collection documents中的多个字段作为查询条件。我们可以使用```AND```来连接所有的查询条件。

如下的示例查询```inventory```这个collection中所有```status```为```"A"```且```qty```小于等于30的记录：
{% highlight string %}
db.inventory.find( { status: "A", qty: { $lt: 30 } } )
{% endhighlight %}

上述语句等价于如下的SQL语句：
{% highlight string %}
SELECT * FROM inventory WHERE status = "A" AND qty < 30
{% endhighlight %}

更多比较操作符，请参看[comparison operators](https://docs.mongodb.com/manual/reference/operator/query-comparison/#std-label-query-selectors-comparison)。

###### Specify OR Conditions

在进行一个符合查询时，我们可以使用[$or](https://docs.mongodb.com/manual/reference/operator/query/or/#mongodb-query-op.-or)操作符来多个查询条件，只要collection中的documents满足其中一个条件，即匹配成功。

如下的示例查询```inventory```这个collection中```status```为```"A"```或```qty```小于等于30的数据记录：
{% highlight string %}
db.inventory.find( { $or: [ { status: "A" }, { qty: { $lt: 30 } } ] } )
{% endhighlight %}

上面的查询语句等价于如下SQL语句：
{% highlight string %}
SELECT * FROM inventory WHERE status = "A" OR qty < 30
{% endhighlight %}

>Note: Queries which use [comparison operators](https://docs.mongodb.com/manual/reference/operator/query-comparison/#std-label-query-selectors-comparison) are subject to [Type Bracketing](https://docs.mongodb.com/manual/reference/method/db.collection.find/#std-label-type-bracketing).

###### Specify AND as well as OR Conditions

如下的例子中，查询```inventroy```这个collection中```status```为```"A"```，且```qty```小于等于30或```item```以字符```p```开头的数据记录：
{% highlight string %}
db.inventory.find( {
     status: "A",
     $or: [ { qty: { $lt: 30 } }, { item: /^p/ } ]
} )
{% endhighlight %}

上面的语句对应于如下SQL查询语句：
{% highlight string %}
SELECT * FROM inventory WHERE status = "A" AND ( qty < 30 OR item LIKE "p%")
{% endhighlight %}

>Note: MongoDB支持正则表达式[$regex](https://docs.mongodb.com/manual/reference/operator/query/regex/#mongodb-query-op.-regex)查询，查询某字段匹配某个pattern


###### Additional Query Tutorials

对于其他的查询示例，请参看:

* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)
* [Query an Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)
* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)
* [Project Fields to Return from Query](https://docs.mongodb.com/manual/tutorial/project-fields-from-query-results/)
* [Query for Null or Missing Fields](https://docs.mongodb.com/manual/tutorial/query-for-null-fields/)

###### Behavior

1) **Cursor**

[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)会为所匹配的documents返回一个[cursor](https://docs.mongodb.com/manual/tutorial/iterate-a-cursor/)。

2） **Read Isolation**

>New in version 3.2

如果从[replica set](https://docs.mongodb.com/manual/replication/)或[replica set shards](https://docs.mongodb.com/manual/sharding/)中执行查询的话，允许客户端选择读取的隔离级别。更详细信息，请参看[Read Concern](https://docs.mongodb.com/manual/reference/read-concern/)。

### 3.1 Query on Embedded/Nested Documents
如下提供一个示例，在mongosh中使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法查询内嵌documents。我们使用```inventory```这个collection。首先向其中填充如下数据：
{% highlight string %}
db.inventory.insertMany( [
   { item: "journal", qty: 25, size: { h: 14, w: 21, uom: "cm" }, status: "A" },
   { item: "notebook", qty: 50, size: { h: 8.5, w: 11, uom: "in" }, status: "A" },
   { item: "paper", qty: 100, size: { h: 8.5, w: 11, uom: "in" }, status: "D" },
   { item: "planner", qty: 75, size: { h: 22.85, w: 30, uom: "cm" }, status: "D" },
   { item: "postcard", qty: 45, size: { h: 10, w: 15.25, uom: "cm" }, status: "A" }
]);
{% endhighlight %}

###### Match an Embedded/Nested Document
要查询内嵌document中某个字段，使用[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter) ```{ <field>:<value>}```，其中```<value>```为所要匹配的document.

下面的例子查询```size```字段等于```{ h: 14, w: 21, uom: "cm" }```的document:
{% highlight string %}
db.inventory.find( { size: { h: 14, w: 21, uom: "cm" } } )
{% endhighlight %}

对一个内嵌document做```Equality```匹配的话，要求```严格```(exact)的完全匹配，包括其中的字段顺序。例如，如下的查询将匹配不上```inventory```这个collection中的任何document:
{% highlight string %}
db.inventory.find(  { size: { w: 21, h: 14, uom: "cm" } }  )
{% endhighlight %}

###### Query on Nested Field




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

