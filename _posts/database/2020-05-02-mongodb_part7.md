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

如果要使用内嵌document中某个字段作为查询条件来查询的话，使用[dot notation](https://docs.mongodb.com/manual/reference/glossary/#std-term-dot-notation)(```"field.nestedField"```)。

>Note: 当使用dot notation来执行查询的话，查询字段必须在双引号内

1） **Specify Equality Match on a Nested Field**

下面的例子查询```size```中的内嵌字段```uom```等于```in```的所有document:
{% highlight string %}
db.inventory.find( { "size.uom": "in" } )
{% endhighlight %}

2) **Specify Match using Query Operator**

一个[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)可以使用[query operators](https://docs.mongodb.com/manual/reference/operator/query/#std-label-query-selectors)来指定查询条件，形式如下：
{% highlight string %}
{ <field1>: { <operator1>: <value1> }, ... }
{% endhighlight %}

下面的示例使用less than operator ([$lt](https://docs.mongodb.com/manual/reference/operator/query/lt/#mongodb-query-op.-lt))在```size```的内嵌字段```h```上做查询：
{% highlight string %}
db.inventory.find( { "size.h": { $lt: 15 } } )
{% endhighlight %}

3) **Specify AND Condition**

如下查询内嵌字段```h```小于15，内嵌字段```uom```等于```in```，且```status```等于```D```的documents:
{% highlight string %}
db.inventory.find( { "size.h": { $lt: 15 }, "size.uom": "in", status: "D" } )
{% endhighlight %}

###### Additional Query Tutorials
其他查询示例，请参看：

* [Query Documents](https://docs.mongodb.com/manual/tutorial/query-documents/)
* [Query an Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)
* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)

### 3.2 Query an Array

本节我们提供一些示例，使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法在数组字段上做查询。例子使用```inventory```这个collection。下面我们先向该collection填充一些数据：
{% highlight string %}
db.inventory.insertMany([
   { item: "journal", qty: 25, tags: ["blank", "red"], dim_cm: [ 14, 21 ] },
   { item: "notebook", qty: 50, tags: ["red", "blank"], dim_cm: [ 14, 21 ] },
   { item: "paper", qty: 100, tags: ["red", "blank", "plain"], dim_cm: [ 14, 21 ] },
   { item: "planner", qty: 75, tags: ["blank", "red"], dim_cm: [ 22.85, 30 ] },
   { item: "postcard", qty: 45, tags: ["blue"], dim_cm: [ 10, 15.25 ] }
]);
{% endhighlight %}

###### Match an Array
要在一个数组上指定equality condition，使用query document ```{ <field>: <value>}```，其中```<value>```与数组严格匹配，包含数组顺序也要匹配。

如下的例子查询的字段```tags```的值为数组，且按顺序值为```red```和```blank```的document:
{% highlight string %}
db.inventory.find( { tags: ["red", "blank"] } )
{% endhighlight %}

另外，假如你仅仅只想查询一个数组中含有```red```及```blank```元素（不关心顺序，也不关心是否有其他元素），请使用[$all](https://docs.mongodb.com/manual/reference/operator/query/all/#mongodb-query-op.-all) operator。

{% highlight string %}
db.inventory.find( { tags: { $all: ["red", "blank"] } } )
{% endhighlight %}

###### Query an Array for an Element
如果要查询数组中```至少```包含某一指定值，使用filter ```{<field>: <value>}```，其中```<value>```为数组元素的值。

如下的例子查询```tags```字段至少含有```red```元素的documents:
{% highlight string %}
db.inventory.find( { tags: "red" } )
{% endhighlight %}

如果要在数组字段的elements上指定查询条件的话，可以在[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)上使用[query operators](https://docs.mongodb.com/manual/reference/operator/query/#std-label-query-selectors):
{% highlight string %}
{ <array field>: { <operator1>: <value1>, ... } }
{% endhighlight %}

例如，下面的操作查询```dim_cm```数组中至少包含一个元素，该元素的值大于25:
{% highlight string %}
db.inventory.find( { dim_cm: { $gt: 25 } } )
{% endhighlight %}


###### Specify Multiple Conditions for Array Elements

当需要在数组元素上指定复合条件的话，你可以为```单个```数组元素指定查询条件，也可以为```多个```数组元素指定查询条件.


1) **Query an Array with Compound Filter Conditions on the Array Elements**

下面的例子查询```dim_cm```数组中元素满足匹配条件的documents。例如，数组中有一个元素大于15且有另一个元素小于20， 或者某一个元素同时满足这两个条件的documents:
{% highlight string %}
db.inventory.find( { dim_cm: { $gt: 15, $lt: 20 } } )
{% endhighlight %}

2) **Query for an Array Element that Meets Multiple Criteria**

使用[$elemMatch](https://docs.mongodb.com/manual/reference/operator/query/elemMatch/#mongodb-query-op.-elemMatch)操作符来为数组中的元素指定多个准则(criteria)，要求数组中至少有一个元素匹配所有准则。

如下的示例查询```dim_cm```数组中至少有一个元素同时满足：大于([$gt](https://docs.mongodb.com/manual/reference/operator/query/gt/#mongodb-query-op.-gt))22，且小于([$lt](https://docs.mongodb.com/manual/reference/operator/query/lt/#mongodb-query-op.-lt))30

{% highlight string %}
db.inventory.find( { dim_cm: { $elemMatch: { $gt: 22, $lt: 30 } } } )
{% endhighlight %}

3) **Query for an Element by the Array Index Position**

使用[dot notation](https://docs.mongodb.com/manual/reference/glossary/#std-term-dot-notation)，你可以为某一特定index上的数组元素指定查询条件。数组下表从0开始索引。

>Note: 当使用dot notation执行查询时，所查询的field(包括内嵌field)必须在双引号中

下面的示例查询```dim_cm```数组中第二个元素大于25的所有documents:
{% highlight string %}
db.inventory.find( { "dim_cm.1": { $gt: 25 } } )
{% endhighlight %}

4) **Query an Array by Array Length**

使用[$size](https://docs.mongodb.com/manual/reference/operator/query/size/#mongodb-query-op.-size)操作符来查询一个数组中数组元素的个数。例如，下面查询```tags```数组有3个元素的documents:
{% highlight string %}
db.inventory.find( { "tags": { $size: 3 } } )
{% endhighlight %}

###### Additional Query Tutorials

更多查询示例，请参看：

* [Query Documents](https://docs.mongodb.com/manual/tutorial/query-documents/)
* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)
* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)

### 3.3 Query an Array of Embedded Documents

本节提供一些示例，使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法查询数组元素为```内嵌```(nested) documents的记录。例子基于```inventory```这个collection，这里我们首先向该collection填充一些数据：
{% highlight string %}
db.inventory.insertMany( [
   { item: "journal", instock: [ { warehouse: "A", qty: 5 }, { warehouse: "C", qty: 15 } ] },
   { item: "notebook", instock: [ { warehouse: "C", qty: 5 } ] },
   { item: "paper", instock: [ { warehouse: "A", qty: 60 }, { warehouse: "B", qty: 15 } ] },
   { item: "planner", instock: [ { warehouse: "A", qty: 40 }, { warehouse: "B", qty: 5 } ] },
   { item: "postcard", instock: [ { warehouse: "B", qty: 15 }, { warehouse: "C", qty: 35 } ] }
]);
{% endhighlight %}

###### Query for a Document Nested in an Array
下面的例子查询```instock```数组中匹配指定条件的documents:
{% highlight string %}
db.inventory.find( { "instock": { warehouse: "A", qty: 5 } } )
{% endhighlight %}

对整个内嵌(embeded/nested)document的Equality匹配要求对指定的document严格匹配，包括字段的顺序。例如，如下的查询将匹配不上```inventory```这个collection中的任何document:
{% highlight string %}
db.inventory.find( { "instock": { qty: 5, warehouse: "A" } } )
{% endhighlight %}

###### Specify a Query Condition on a Field in an Array of Documents

1) **Specify a Query Condition on a Field Embedded in an Array of Documents**

假如你不知道某一document在内嵌数组中的索引位置，那么可以使用```.```(dot)将将数组字段的名称(array-field-name)与内嵌document字段名称(nested-document-field-name)连接起来。

如下的示例查询```instock```数组至少有一个内嵌元素含有```qty```字段，且该字段的值小于等于20：
{% highlight string %}
db.inventory.find( { 'instock.qty': { $lte: 20 } } )
{% endhighlight %}

2) **Use the Array Index to Query for a Field in the Embedded Document**

使用[dot notation](https://docs.mongodb.com/manual/reference/glossary/#std-term-dot-notation)，你可以为数组中某个索引(position)位置上的内嵌document字段指定查询条件。数组的索引从0开始。

>Note: 当使用dot notation时，字段(field)与索引(index)必须在双引号中

下面的例子查询```instock```数组中第一个document元素含有内嵌```qtr```字段，并且该字段的值小于等于20:
{% highlight string %}
db.inventory.find( { 'instock.0.qty': { $lte: 20 } } )
{% endhighlight %}

###### Specify Multiple Conditions for Array of Documents
当需要对一个数组内嵌document的多个field指定查询条件时，你可以为内嵌的```单个```document指定多个匹配条件，也可以为```多个```document指定多个匹配条件。

1） **A Single Nested Document Meets Multiple Query Conditions on Nested Fields**

使用[$elemMatch](https://docs.mongodb.com/manual/reference/operator/query/elemMatch/#mongodb-query-op.-elemMatch) operator为数组中的内嵌document指定多个```准则```(criteria)，要求数组中至少有一个内嵌document匹配所有准则。

下面的示例查询```instock```数组中至少有一个内嵌document同时含有字段```qty```等于5，且含有字段```warehouse```等于```A```:
{% highlight string %}
db.inventory.find( { "instock": { $elemMatch: { qty: 5, warehouse: "A" } } } )
{% endhighlight %}

2) **Combination of Elements Satisfies the Criteria**

假如对于数组字段(field)执行复合条件查询时```不使用```[$elemMatch](https://docs.mongodb.com/manual/reference/operator/query/elemMatch/#mongodb-query-op.-elemMatch)的话，则只要数组中的内嵌document的任何组合(可以多个内嵌documents组合在一起）满足查询条件即匹配成功。

例如，下面查询```instock```数组中任何内嵌document含有```qty```字段大于10，以及数组中任何内嵌document(没必要是同一个内嵌document)含有```qty```字段小于等于20:
{% highlight string %}
db.inventory.find( { "instock.qty": { $gt: 10,  $lte: 20 } } )
{% endhighlight %}

下面的例子查询```instock```数组中至少一个内嵌document含有字段```qty```等于5，且至少一个内嵌document(没必要是同一内嵌document)含有字段```warehouse```等于```A```:
{% highlight string %}
db.inventory.find( { "instock.qty": 5, "instock.warehouse": "A" } )
{% endhighlight %}

###### Additional Query Tutorials

更多查询示例，请参看:

* [Query an Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)
* [Query Documents](https://docs.mongodb.com/manual/tutorial/query-documents/)
* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)


### 3.4 Project Fields to Return from Query
>Note: projection这里可以翻译为“字段筛选”

默认情况下，在MongoDB中执行查询会返回所匹配的documents中的所有字段。为了限制返回给应用程序的数据量，我们可以使用一个[projection](https://docs.mongodb.com/manual/reference/glossary/#std-term-projection) document来指定(specify)或限制(restrict)返回的字段(field)。

本节提供一些查询示例，使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法执行查询，并返回指定的字段。例子基于```inventory```这个collection，这里我们首先向该collection填充一些数据：
{% highlight string %}
db.inventory.insertMany( [
  { item: "journal", status: "A", size: { h: 14, w: 21, uom: "cm" }, instock: [ { warehouse: "A", qty: 5 } ] },
  { item: "notebook", status: "A",  size: { h: 8.5, w: 11, uom: "in" }, instock: [ { warehouse: "C", qty: 5 } ] },
  { item: "paper", status: "D", size: { h: 8.5, w: 11, uom: "in" }, instock: [ { warehouse: "A", qty: 60 } ] },
  { item: "planner", status: "D", size: { h: 22.85, w: 30, uom: "cm" }, instock: [ { warehouse: "A", qty: 40 } ] },
  { item: "postcard", status: "A", size: { h: 10, w: 15.25, uom: "cm" }, instock: [ { warehouse: "B", qty: 15 }, { warehouse: "C", qty: 35 } ] }
]);
{% endhighlight %}


###### Return All Fields in Matching Documents
假如不指定一个[projection](https://docs.mongodb.com/manual/reference/glossary/#std-term-projection) document的话，则[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)会返回匹配字段的所有元素。

下面的示例返回```inventory```这个collection中```status```等于```"A"```的document的所有字段：
{% highlight string %}
db.inventory.find( { status: "A" } )
{% endhighlight %}
上面的查询语句等价于如下SQL语句：
<pre>
SELECT * from inventory WHERE status = "A"
</pre>

###### Return the Specified Fields and the ```_id``` Field Only
可以通过projection显式的指定返回哪些字段，方法是将返回的```<field>```设置为1。如下的操作返回匹配查询条件的所有documents。在返回的结果集中，只包含```item```及```status```字段，以及默认情况下的```_id```字段。
{% highlight string %}
db.inventory.find( { status: "A" }, { item: 1, status: 1 } )
{% endhighlight %}

上述查询语句对应于如下SQL语句：
<pre>
SELECT _id, item, status from inventory WHERE status = "A"
</pre>

###### Suppress ```_id``` Field
我们可以在projection中将```_id```字段设置为0，从而返回结果中不含有```_id```字段：
{% highlight string %}
db.inventory.find( { status: "A" }, { item: 1, status: 1, _id: 0 } )
{% endhighlight %}

例子中查询语句对应于如下SQL语句：
<pre>
SELECT item, status from inventory WHERE status = "A"
</pre>

>Note: 除了```_id```字段外，你不能在projection document中同时使用inclusion与exclusion

###### Return All But the Excluded Fields
我们可以使用projection来排除返回某些字段。下面的示例会返回除```status```及```instock```字段外的所有其他字段：
{% highlight string %}
db.inventory.find( { status: "A" }, { status: 0, instock: 0 } )
{% endhighlight %}


>Note: 除了```_id```字段外，你不能在projection document中同时使用inclusion与exclusion


###### Return Specific Fields in Embedded Documents

我们可以返回内嵌document的指定字段。可以在projection document中使用[dot noation](https://docs.mongodb.com/manual/core/document/#std-label-document-dot-notation)来引用指定的内嵌字段。

如下的示例返回：

* ```_id```字段（默认会返回)
* ```item```字段
* ```status```字段
* ```size```中的```uom```字段

{% highlight string %}
db.inventory.find(
   { status: "A" },
   { item: 1, status: 1, "size.uom": 1 }
)
{% endhighlight%}

从MongoDB 4.4版本开始，针对内嵌字段我们也可以使用内嵌形式，例如：
<pre>
{ item: 1, status: 1, size: { uom: 1 } }
</pre>

###### Suppress Specific Fields in Embedded Documents
我们可以抑制内嵌document的指定字段。使用[dot notation](https://docs.mongodb.com/manual/core/document/#std-label-document-dot-notation)来引用对应的字段，并将对应的字段值设置为0.

下面的例子指定一个projection，排除返回```size```document中的```uom```字段，而其他的字段均返回：
{% highlight string %}
db.inventory.find(
   { status: "A" },
   { "size.uom": 0 }
)
{% endhighlight %}

从MongoDB 4.4版本开始，针对内嵌字段我们也可以使用内嵌形式，例如：
<pre>
{ item: 1, status: 1, size: { uom: 0 } }
</pre>

###### Projection on Embedded Documents in an Array

使用[dot notation](https://docs.mongodb.com/manual/core/document/#std-label-document-dot-notation)数组中内嵌document的指定字段。

如下的例子返回指定字段：


* ```_id```字段（默认会返回)
* ```item```字段
* ```status```字段
* ```instock```数组中内嵌document的```qty```字段

{% highlight string %}
db.inventory.find( { status: "A" }, { item: 1, status: 1, "instock.qty": 1 } )
{% endhighlight %}

###### Project Specific Array Elements in the Returned Array
对于包含数组的字段，MongoDB提供了如下的projection operator来操作数组：

* [$elemMatch](https://docs.mongodb.com/manual/reference/operator/projection/elemMatch/#mongodb-projection-proj.-elemMatch)

* [$slice](https://docs.mongodb.com/manual/reference/operator/projection/slice/#mongodb-projection-proj.-slice)

* [$](https://docs.mongodb.com/manual/reference/operator/projection/positional/#mongodb-projection-proj.-)

如下的例子使用[$slice](https://docs.mongodb.com/manual/reference/operator/projection/slice/#mongodb-projection-proj.-slice)这一projection operator来返回```instock```数组中最后一个元素。
{% highlight string %}
db.inventory.find( { status: "A" }, { item: 1, status: 1, instock: { $slice: -1 } } )
{% endhighlight %}

[$elemMatch](https://docs.mongodb.com/manual/reference/operator/projection/elemMatch/#mongodb-projection-proj.-elemMatch)、[$slice](https://docs.mongodb.com/manual/reference/operator/projection/slice/#mongodb-projection-proj.-slice)、[$](https://docs.mongodb.com/manual/reference/operator/projection/positional/#mongodb-projection-proj.-)是仅有的三种可以筛选特定数组元素的方法。比如，你不能使用数组索引来筛选特定元素，eg: ```{"instock.0": 1}```这一projection```不能```筛选数组的第一个元素。

###### Additional Considerations
从MongoDB 4.4版本开始，针对projection MongoDB会强制一些额外的限制，请参看[Projection Restrictions](https://docs.mongodb.com/manual/reference/limits/#mongodb-limit-Projection-Restrictions)。

### 3.5 Query for Null or Missing Fields
在MongoDB中不同的query operators对待```null```值的方式也不同。

本文提供一些使用[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法查询```null```值的示例。例子使用```inventory```这个collection，我们向该collection填充一些数据：

{% highlight string %}
db.inventory.insertMany([
   { _id: 1, item: null },
   { _id: 2 }
])
{% endhighlight %}


###### Equality Filter
```{item: null}```查询匹配```item```字段的值为```null```或者不含```item```的document。

{% highlight string %}
db.inventory.find( { item: null } )
{% endhighlight %}

上述查询会返回```inventory```集合中的两个元素。

###### Type Check
```{ item : { $type: 10 } }```查询匹配含有```item```字段且该字段值为```null```的documents。比如，```item```字段的值为[BSON Type](https://docs.mongodb.com/manual/reference/bson-types/) Null(其对应的type number为10）：
{% highlight string %}
db.inventory.find( { item : { $type: 10 } } )
{% endhighlight %}

上述查询只返回item字段值为```null```的document。

###### Existence Check
下面的示例查询不含某一字段的documents。

>注：从MongoDB 4.2版本开始，用户不能使用query filter```$type: 0```作为```$exists: false```的等价。

```{ item : { $exists: false } }```匹配不含```item```字段的documents:
{% highlight string %}
db.inventory.find( { item : { $exists: false } } )
{% endhighlight %}
上述例子只返回不含```item```字段的documents。



### 3.6 Iterate a Cursor in mongosh
[db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)会返回一个```游标```(cursor)。要访问这些documents，你需要遍历该cursor。然而，在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中，假如返回的cursor没有赋值给一个使用```var```关键字定义的变量时，则cursor会自动的遍历20次以打印结果集中最开头的20个documents。

下面的例子描述了一种方法可以手动(manually)的遍历cursor从而访问结果集中的documents，或者使用iterator index来访问结果集中的元素。

###### Manually Iterate the Cursor
在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中，当你将[find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法返回的结果集赋值给一个由```var```关键字定义的变量时，则cursor将不会自动的遍历。

你可以在mongosh中遍历该cursor多达20次，以打印结果集中的元素，请参看如下示例：

>Note: You can set the ```DBQuery.shellBatchSize``` attribute to change the number of documents from the default value of 20.

{% highlight string %}
var myCursor = db.users.find( { type: 2 } );

myCursor
{% endhighlight %}

你也可以使用cursor method[next()](https://docs.mongodb.com/manual/reference/method/cursor.next/#mongodb-method-cursor.next)来访问结果集中的元素，参看如下示例：
{% highlight string %}
var myCursor = db.users.find( { type: 2 } );

while (myCursor.hasNext()) {
   print(tojson(myCursor.next()));
}
{% endhighlight %}

作为另一种打印方式，考虑```printjson()```帮助方法以代替```print(tojson())```:
{% highlight string %}
var myCursor = db.users.find( { type: 2 } );

while (myCursor.hasNext()) {
   printjson(myCursor.next());
}
{% endhighlight %}

另外，你也可以使用[forEach()](https://docs.mongodb.com/manual/reference/method/cursor.forEach/#mongodb-method-cursor.forEach)来遍历cursor，并访问其中的documents，参看如下示例：
{% highlight string %}
var myCursor =  db.users.find( { type: 2 } );

myCursor.forEach(printjson);
{% endhighlight %}

###### Iterator Index
在mongosh中，你可以使用[toArray()](https://docs.mongodb.com/manual/reference/method/cursor.toArray/#mongodb-method-cursor.toArray)方法来遍历cursor，并通过一个数组来返回documents，参看如下：
{% highlight string %}
var myCursor = db.inventory.find( { type: 2 } );
var documentArray = myCursor.toArray();
var myDocument = documentArray[3];
{% endhighlight %}
[toArray](https://docs.mongodb.com/manual/reference/method/cursor.toArray/#mongodb-method-cursor.toArray)方法会将cursor所返回的所有documents加载到RAM。[toArray()](https://docs.mongodb.com/manual/reference/method/cursor.toArray/#mongodb-method-cursor.toArray)会exhausts cursor。

另外有一些[Driver](https://docs.mongodb.com/drivers/)会针对cursor提供索引(index)来访问其中的documents（比如：```cursor[index]```）。这仅仅知识调用[toArray()](https://docs.mongodb.com/manual/reference/method/cursor.toArray/#mongodb-method-cursor.toArray)的一种简写形式，然后在结果集数组上使用索引。

考虑下面的示例：
{% highlight string %}
var myCursor = db.users.find( { type: 2 } );
var myDocument = myCursor[1];
{% endhighlight %}

其中```myCursor[1]```等价于如下：
{% highlight string %}
myCursor.toArray() [1];
{% endhighlight %}

###### Cursor Behaviors
1) **Cursors Opened Within a Session**

从MongoDB 5.0(以及4.4.8)版本开始，在一个[client session](https://docs.mongodb.com/manual/core/read-isolation-consistency-recency/)中所创建的cursors会在对应的[server session](https://docs.mongodb.com/manual/reference/server-sessions/)接收到[killSessions](https://docs.mongodb.com/manual/reference/command/killSessions/#mongodb-dbcommand-dbcmd.killSessions)命令后被关闭，或者遇到session超时，或者client已经exhausted对应的cursor。

默认情况下，server session的超时时间为30分钟。要改变该值，在启动mongod时请设置[localLogicalSessionTimeoutMinutes](https://docs.mongodb.com/manual/reference/parameters/#mongodb-parameter-param.localLogicalSessionTimeoutMinutes)参数。

2) **Cursors Opened Outside of a Session**

3) **Cursor Isolation**

4) **Cursor Batches**

###### Cursor Information






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

