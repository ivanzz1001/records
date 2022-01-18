---
layout: post
title: MongoDB介绍(BSON Types)
tags:
- database
categories: database
description:  MongoDB介绍
---


本文介绍BSON类型(types)相关内容。

<!-- more -->


## 1. BSON Types

[BSON](https://docs.mongodb.com/manual/reference/glossary/#std-term-BSON)是一种二进制序列化格式，MongoDB使用该格式来存储documents和远程过程调用(RPC).

每一种BSON类型都有整数(integer)和字符串(string)两种标识，如下表所示：
<pre>
Type                         Number              Alias                         Notes
--------------------------------------------------------------------------------------------
Double                        1                "double" 
String                        2                "string"
Object                        3                "object"
Array                         4                "array"
Binary data                   5                "binData"
Undefined                     6                "undefined"                   Deprecated
ObjectId                      7                "objectId"
Boolean  	                  8                "bool"
Date                          9                "date"
Null                          10               "null"
Regular Expression            11               "regex"
DBPointer                     12               "dbPointer"                   Deprecated
JavaScript                    13               "javascript"
Symbol                        14               "symbol"                      Deprecated
JavaScript code with scope    15               "javascriptWithScope"         Deprecated in MongoDB 4.4
32-bit integer                16               "int"
Timestamp                     17               "timestamp"
64-bit integer                18               "long"
Decimal128                    19               "decimal"                     New in version 3.4
Min key                       -1               "minKey"
Max key                       127              "maxKey" 
</pre>

* [$type](https://docs.mongodb.com/manual/reference/operator/query/type/#mongodb-query-op.-type)操作符支持使用这些值来根据BSON Type查询字段。同时```$type```也支持```number```别名，用于匹配integer、decimal、double、以及long类型

* [$type](https://docs.mongodb.com/manual/reference/operator/aggregation/type/#mongodb-expression-exp.-type)聚合操作符返回其参数的BSON类型

* [$isNumber](https://docs.mongodb.com/manual/reference/operator/aggregation/isNumber/#mongodb-expression-exp.-isNumber)聚合操作符在其参数为BSON integer、decimal、double、long类型时返回```true```(New in version 4.4).

为了确定一个字段的类型，请参看[ Check Types in the mongo Shell](https://docs.mongodb.com/manual/core/shell-types/#std-label-check-types-in-shell)

假如你将BSON转换为JSON，请参看[Extended JSON](https://docs.mongodb.com/manual/reference/mongodb-extended-json/).

下面我们会描述特定的BSON类型的一些特殊考量。

### 1.1 ObjectId

ObjectIds are small, likely unique, fast to generate, and ordered。ObjectId的值占用12字节长度，组成如下：

* 4字节时间戳，代表了ObjectId的创建时间，单位为Unix epoch到当前的秒数。

* 由每个进程所产生的5字节随机值。该随机值对于该机器(machine)和进程(process)来说唯一

* 3字节增量计数，初始化为一个随机值。

BSON格式本身是小端格式(little-endian)，timestamp以及counter value是大端格式(big-endian)。

在MongoDB中，存储在collection中的每一个document需要一个唯一的[_id](https://docs.mongodb.com/manual/reference/glossary/#std-term-_id)字段来作为primary key。假如在执行document插入时省略了```_id```字段，那么MongoDB driver会自动的为```_id```字段产生一个ObjectId。

当通过update来进行document插入时，如果设置[upsert:true](https://docs.mongodb.com/manual/reference/method/db.collection.update/#std-label-upsert-parameter)，则同样会自动产生ObjectId。

MongoDB客户端应该为```_id```字段添加一个唯一的ObjectId。此外，使用ObjectId来为```_id```赋值还具有如下好处：

* 在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中，你可以使用[ObjectId.getTimestamp()](https://docs.mongodb.com/manual/reference/method/ObjectId.getTimestamp/#mongodb-method-ObjectId.getTimestamp)方法来获取ObjectId的创建时间

* 当```_id```字段存放的值为ObjectId时，如果对```_id```字段排序时，等价于按创建时间进行排序。

>注：由于ObjectId的值应该随时间而增长，但它们却并没有必要保持是单调的。原因如下
>
> 1) 由于创建ObjectId时时间的分辨率是秒，因此在同一秒内的ObjectId并不能保证顺序
> 
> 2) ObjectId可能是由客户端产生的，不同的客户端也可能有不同的系统时钟


### 1.2 String 
BSON字符串是采用UTF-8编码。通常来说，每一种编程语言的驱动都会在序列化与反序列化BSON时，将对应语言的字符串编码转换成UTF-8编码格式。这使得采用BSON字符串可以很容易的存储大部分的国际化字符。另外，MongoDB的正则表达式查询([$regex queries](https://docs.mongodb.com/manual/reference/operator/query/regex/#mongodb-query-op.-regex))也支持UTF-8格式的正则字串。

给定UTF-8字符集的字串，使用[sort()](https://docs.mongodb.com/manual/reference/method/cursor.sort/#mongodb-method-cursor.sort)来进行排序，通常是合理的、正确的。然而，由于内部sort()使用C++的strcmp api，因此在排序时对于某一些字符可能有误。

### 1.3 Timestamps
BSON有一种特别的timestamp类型供MongoDB内部使用，并且与通常的[Date类型](https://docs.mongodb.com/manual/reference/bson-types/#std-label-document-bson-type-date)没有关系。该内部的时间戳是一个64位的值：

* 数字的高32位是一个time_t类型的值(seconds since the Unix epoch)

* 数字的低32位是在一秒内操作的增量值(incrementing ordinal)

由于BSON采用小端格式(little-endian)，因此首先存放的是低有效位。[mongod实例](https://docs.mongodb.com/manual/reference/program/mongod/#mongodb-binary-bin.mongod)在所有平台上，不管是大端还是小端，在比较两个内部timestamps时，总是先比较time_t部分，然后再是ordinal部分。

在单个[mongod实例](https://docs.mongodb.com/manual/reference/program/mongod/#mongodb-binary-bin.mongod)内，timestamp值总是唯一的。

在执行复制(replication)操作时，[oplog](https://docs.mongodb.com/manual/reference/glossary/#std-term-oplog)有一个```ts```字段。该字段的值反映了操作时间(operation time)，其也是一个BSON timestamp类型的值。

>Note:
>
> BSON timestamp是MongoDB内部使用的。在大多数情况下，进行应用程序开发时，我们都是使用BSON date类型。参看[Date](https://docs.mongodb.com/manual/reference/bson-types/#std-label-document-bson-type-date)以了解更多信息。

当插入一个document到collection时，如果top-level fields中有空时间戳值(empty timestamp values)的话，MongoDB会将对应的空时间戳值替换为当前时间，除非遇到如下场景。假如```_id```字段的值也是一个empty timestamp的话，其总是会被原模原样的被插入进去。

示例：插入一个empty timestamp值的document到collection中
{% highlight string %}
db.test.insertOne( { ts: new Timestamp() } );
{% endhighlight %}
然后执行[db.test.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法，返回的查询结果 类似如下：
{% highlight string %}
{ "_id" : ObjectId("542c2b97bac0595474108b48"), "ts" : Timestamp(1412180887, 1) }
{% endhighlight %}
从上面的结果我们看到，服务器已经将```ts```替换成了该document插入时的时间戳值。

### 1.4 Date
BSON Date是一个64位的整数，用于表示从Unix epoch(1970年1月1日）到当前的毫秒(milliseconds)数。这样一个64bit的整数可以表示的时间范围有290000000年。

BSON Date是signed类型，负值表示1970年以前的日期。

1) 示例1

在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中使用new Date()构造函数来创建一个Date对象
{% highlight string %}
var mydate1 = new Date()
{% endhighlight %}

2) 示例2 

在[mongosh](https://docs.mongodb.com/mongodb-shell/#mongodb-binary-bin.mongosh)中使用ISODate()构造函数创建Date对象：
{% highlight string %}
var mydate2 = ISODate()
{% endhighlight %}

3) 示例3 

以字符串形式返回Date对象：
{% highlight string %}
mydate1.toString()
{% endhighlight %}

4) 示例4 

返回Date对象的month部分。month是从0开始的，因此January的month值为0：
{% highlight string %}
mydate1.getMonth()
{% endhighlight %}

## 2. Comparison/Sort Order 

## 3. MongoDB Extended JSON(v2)







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

