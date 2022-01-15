---
layout: post
title: MongoDB介绍(Documents)
tags:
- database
categories: database
description:  MongoDB介绍
---


本章介绍以下MongoDB中的document相关概念。

<!-- more -->

MongoDB将数据记录作为BSON documents来进行存储。BSON是[JSON](https://docs.mongodb.com/manual/reference/glossary/#std-term-JSON) documents的一种二进制表示方式，但是其具有比JSON更多的数据类型(data types)。参看[bsonspec.org](http://bsonspec.org/)与[BSON Types](https://docs.mongodb.com/manual/reference/bson-types/).

![mongodb-bson](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-document.bakedsvg.svg)

## 1. Document Structure
MongoDB文档是由field-value对所组成，并且有如下结构：
{% highlight string %}
{
   field1: value1,
   field2: value2,
   field3: value3,
   ...
   fieldN: valueN
}
{% endhighlight %}

其中value字段可以是任何的BSON [data types](https://docs.mongodb.com/manual/reference/bson-types/)，包括其他的documents、arrays以及arrays of documents。例如，下面的document包含多种类型的values:
{% highlight string %}
var mydoc = {
               _id: ObjectId("5099803df3f4948bd2f98391"),
               name: { first: "Alan", last: "Turing" },
               birth: new Date('Jun 23, 1912'),
               death: new Date('Jun 07, 1954'),
               contribs: [ "Turing machine", "Turing test", "Turingery" ],
               views : NumberLong(1250000)
            }
{% endhighlight %}

上面的字段含有如下数据类型：

* ```_id```维持着一个[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#std-label-objectid)

* ```name```维持着一个内嵌的document，该document含有first及last两个字段；

* ```birth```及```death```维持着日期类型的值

* ```contribs```维持着字符串数组

* ```views```维持着一个NumberLong类型的值

1） **FieldNames**

字段的名称是字符串类型。

documents的字段名称具有如下限制：

* 字段名称```_id```保留，用作主键(primary key)。其值在整个集合中唯一，并且是不可修改的，可以是除数组之外的任何类型。假如```_id```包含子字段(subfields)的话，则子字段的名称不能以```$```符号开头

* 字段的名称不能包含```null```字符

* 服务器允许存储的字段名称包含```.```(dot)和美元符号($)

* MongoDB 5.0对字段名称中的```$```及```.```符号提供了更高级的支持。同时也有一些限制，请参看[Field Name Considerations](https://docs.mongodb.com/manual/core/dot-dollar-considerations/#std-label-crud-concepts-dot-dollar-considerations)以了解更多细节。

BSON文档可以有多个字段含有相同的名称。然而大部分[MongoDB接口](https://docs.mongodb.com/drivers/)的内部数据结构实现都不支持有相同的字段名称。假如你需要操作含有相同字段名称的documents，请参看[driver documentation](https://docs.mongodb.com/drivers/)。

有一些由MongoDB内部进程所创建的documents可能含有相同名称的字段，但是并没有相关的方法可以向用户documents中添加重复的字段。

2) **Field Value Limit**

从MongoDB 2.6到[featureCompatibilityVersion (fCV)](https://docs.mongodb.com/manual/reference/command/setFeatureCompatibilityVersion/#std-label-view-fcv)被设置为```4.0```的版本：针对[indexed collections](https://docs.mongodb.com/manual/indexes/)，索引字段的值的长度有[Maximum Index Key Length](https://docs.mongodb.com/manual/reference/limits/#mongodb-limit-Index-Key-Limit)限制。请参看[Maximum Index Key Length](https://docs.mongodb.com/manual/reference/limits/#mongodb-limit-Index-Key-Limit)以了解更多细节。

### 1.1 Dot Notation 
MongoDB使用```.```符号来访问数组元素和内嵌的document字段。

1） **Arrays**

可以通过```.```符号来设置或访问下标从0开始的数组元素。具体方式为：
{% highlight string %}
"<array>.<index>"
{% endhighlight %}
其中```<array>```为数组名，```<index>```为从0开始的下标。另外注意外层的```引号```。

下面的例子中，含有一个字段为contribs的document:
{% highlight string %}
{
   ...
   contribs: [ "Turing machine", "Turing test", "Turingery" ],
   ...
}
{% endhighlight %}

如果要指定```contribs```数组的第三个元素，请使用```"contribs.2"```。

对于查询数组，请参看:

* [Query An Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)

* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)

>See Also:
>
> [$[]](https://docs.mongodb.com/manual/reference/operator/update/positional-all/#mongodb-update-up.---)针对更新操作的所有位置操作符
> 
> [$[<identifier>]](https://docs.mongodb.com/manual/reference/operator/update/positional-filtered/#mongodb-update-up.---identifier--)针对更新操作的过滤的位置操作符
>
> [$](https://docs.mongodb.com/manual/reference/operator/update/positional/#mongodb-update-up.-)针对更新操作的位置操作符
> 
> [$](https://docs.mongodb.com/manual/reference/operator/projection/positional/#mongodb-projection-proj.-)当数组索引位置不确定时的projection操作符
> 
> [Query An Array](https://docs.mongodb.com/manual/tutorial/query-arrays/#std-label-read-operations-arrays)点符号访问数组示例


2) **Embedded Documents**

可以使用```.```符号来设置或访问内嵌document的字段。具体方式如下：
{% highlight string %}
"<embedded document>.<field>"
{% endhighlight %}
>注: 外层含有引号


例如，给定含有下面字段的document:
{% highlight string %}
{
   ...
   name: { first: "Alan", last: "Turing" },
   contact: { phone: { type: "cell", number: "111-222-3333" } },
   ...
}
{% endhighlight %}

* 要设置```name```字段的```last```分量的值时，可以使用```"name.last"```

* 要设置```contact```字段的电话号码时，可以使用```"contact.phone.number"```

更多查询内嵌document的示例，请参看:

* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)

* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)


### 1.2 Document Limitations
Documents有如下的一些属性：

1） **Document Size Limit**

BSON document的最大大小为16M字节。最大大小的限制确保了单个document不会占用太多的内存，在进行传输时也不会消耗太多的带宽。要存储超过最大大小的documents，MongoDB提供了GridFs API。参看[mongofiles](https://docs.mongodb.com/database-tools/mongofiles/#mongodb-binary-bin.mongofiles)，以及关于GridFs驱动的相关文档

2） **Document Field Order**

与JavaScript对象不同，BSON中的字段是有顺序的。

2.1） Field Order in Queries

对于查询，字段的顺序行为如下：

* 当进行documents比较时，field的顺序也是有意义的。例如，当比较含有```a```、```b```两个字段的document时：

  * {a: 1, b: 1}等于{a: 1, b: 1}
  * {a: 1, b: 1}不等于{b: 1, a: 1}

* 在执行高效的查询时，查询引擎可能会在进行查询处理时对字段进行排序。其他场景下，在执行```$project```、```$addFields```、```$set```、```$unset```这些projection操作符时也可能会对字段进行排序

  * 查询返回的中间结果或最后结果都有可能对字段进行排序
  * 由于有些操作可能对字段进行排序，因此在你使用上面列出的projection操作符时，不应该依赖于查询结果中字段的顺序

2.2） Field Order in Write Operations

对于写操作，MongoDB会保留document字段的顺序，除非遇到如下场景：

* ```_id```字段总是作为document的第一个字段

* 包含对字段重命名([rename](https://docs.mongodb.com/manual/reference/operator/update/rename/#mongodb-update-up.-rename))的更新操作可能会更改document字段的顺序

2.3) The _id Field

在MongoDB中，存放在collection中的每一个document都有一个唯一的```_id```字段作为该document的主键(primary key)。假如一个插入的document省略了```_id```字段，则MongoDB会自动的为```_id```字段产生一个[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#std-label-objectid)

这同样适用于```insert:true```的update操作。


```_id```字段具有如下行为和限制：

* 默认情况下，MongoDB会在创建collection时为```_id```字段创建一个唯一索引

* ```_id```字段总是作为documents的一个字段。假如服务器收到了一个document，其```_id```字段不是第一个字段，那么服务器会自动的将该字段移动为第一个字段

* 假如```_id```字段的值包含子字段，则字段的fieldName不能以```$```符号开头。

* ```_id```字段的值可以是任何[BSON data type](https://docs.mongodb.com/manual/reference/bson-types/)，但是不能为array、regex、或undefined

>警告：为了确保功复制功能运行正常，不要在_id字段存放BSON正则表达式

如下是存储```_id```字段的值时的一些常用选项：

* 使用一个[ObjectId](https://docs.mongodb.com/manual/reference/bson-types/#std-label-objectid)

* 假如可行的话，使用一个自然的唯一标识符。这可以节省存储空间，并且可以避免额外的索引。

* 产生一个自增(auto-incrementing)数字

* 通过应用程序代码产生一个UUID。为了更高效的在collection以及```_id```索引中存储UUID的值，将UUID作为一个BSON ```BinData```类型。

采用```BinData```类型作为索引的key在如下的条件下可以获得更高的效率：

  * 二进制子类型的值在0~7或128~135范围内
  * 字节数组的长度为：0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 或32.

* 使用驱动(driver)所提供的UUID设备来产生UUIDs。需要知道的是，driver所提供的UUID序列化与反序列化可能有不同的实现逻辑，因此可能与其他的driver不完全兼容。参看[driver documentation](https://api.mongodb.com/)以了解更多UUID互操作性信息。

>Note:
>
> 大多数MongoDB驱动客户端都会包含_id字段，并在执行插入操作之前产生一个ObjectId。然而，假如客户端发送一个不带_id字段的document，那么MongoDB将会增加一个_id字段，并为其生成一个ObjectId

### 1.3 Other Uses of the Document Structure
除了用于定义数据记录(data record)，MongoDB还全面使用document structure，包括且不限于：[query filters](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter), [update specifications documents](https://docs.mongodb.com/manual/core/document/#std-label-document-update-specification), [index specification documents](https://docs.mongodb.com/manual/core/document/#std-label-document-index-specification)


1) **Query Filter Documents**

通过指定过滤条件来查询特定的documents以进行读取(read)、update、delete操作。

我们可以使用```<field>:<value>```表达式来指定相等条件(equality condition)以及[query operator](https://docs.mongodb.com/manual/reference/operator/query/)表达式。
{% highlight string %}
{
  <field1>: <value1>,
  <field2>: { <operator>: <value> },
  ...
}
{% endhighlight %}

其他示例，请参看:

* [Query Documents](https://docs.mongodb.com/manual/tutorial/query-documents/)
* [Query on Embedded/Nested Documents](https://docs.mongodb.com/manual/tutorial/query-embedded-documents/)
* [Query an Array](https://docs.mongodb.com/manual/tutorial/query-arrays/)
* [Query an Array of Embedded Documents](https://docs.mongodb.com/manual/tutorial/query-array-of-documents/)

2) **Update Specification Documents**

使用[update operators](https://docs.mongodb.com/manual/reference/operator/update/#std-label-update-operators)来更新特定的documents的指定字段。
{% highlight string %}
{
  <operator1>: { <field1>: <value1>, ... },
  <operator2>: { <field2>: <value2>, ... },
  ...
}
{% endhighlight %}

更多示例，请参看[Update specifications](https://docs.mongodb.com/manual/tutorial/update-documents/#std-label-update-documents-modifiers)

3) **Index Specification Documents**

Index specification documents用于定义索引的字段(field)以及索引类型：
{% highlight string %}
{ <field1>: <type1>, <field2>: <type2>, ...  }
{% endhighlight %}


### 1.4 Further Reading 
关于MongoDB文档模型(document model)的更多信息，请下载[ MongoDB Application Modernization Guide](https://www.mongodb.com/modernize?tck=docs_server)

可以从中下载到如下资源：

* MongoDB数据模型的方法论介绍

* 从RDBMS数据模型迁移到MongoD的最佳实践及考量白皮书(white paper)

* 与RDBMS所对应的MongoDB schema参考

* Application Modernization scorecard





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

