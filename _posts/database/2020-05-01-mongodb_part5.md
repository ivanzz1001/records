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

