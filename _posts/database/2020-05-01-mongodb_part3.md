---
layout: post
title: MongoDB数据库与集合(2)
tags:
- database
categories: database
description:  MongoDB数据库与集合
---


这里我们接着上文，介绍另外一种集合:Time Series Collections。

>Note: New in version 5.0

<!-- more -->






## 1. Time Series Collections
>New in version 5.0

[Time series collections](https://docs.mongodb.com/manual/reference/glossary/#std-term-time-series-collection)能够高效的存储时序度量数据。时序数据是任何随时间而变化而收集的数据，并唯一的由一个或多个unchanging参数所标识。通常保持不变(unchanging)的那一部分为数据的```元数据```。

<pre>
Example                      Measurement                     Metadata
----------------------------------------------------------------------------------
Weather data                Temperature           Sensor identifier, location

Stock data                  Stock price           Stock ticker, exchange

Website visitors            View count            URL
</pre>

与其他平常的collection相比较，将时序数据存放在Time Series Collections中可以提高查询效率，减少磁盘占用。


###### 1.1 Procedures

1） **Create a Time Series Collection**

>注：只能在[featureCompatibilityVersion](https://docs.mongodb.com/manual/reference/command/setFeatureCompatibilityVersion/#std-label-view-fcv)设置为5.0的系统上创建时序集合

在插入数据到time series collection之前，必须使用[db.createCollection()](https://docs.mongodb.com/manual/reference/method/db.createCollection/#mongodb-method-db.createCollection)方法或[create](https://docs.mongodb.com/manual/reference/command/create/#mongodb-dbcommand-dbcmd.create)命令显式创建collection:
{% highlight string %}
db.createCollection(
    "weather",
    {
       timeseries: {
          timeField: "timestamp",
          metaField: "metadata",
          granularity: "hours"
       }
    }
)
{% endhighlight %}

当创建time series collection时，指定如下选项：
<pre>
   Field                        Type                         Description
-----------------------------------------------------------------------------------------------------------------
timeseries.timeField           string             (Required)指定在每一个时序document中哪一个字段包含date数据。在Time
                                                  Series Collection中的documents必须有一个有效的BSON date字段，并
                                                  将该字段作为timeField。


timeseries.metaField           string             (Optional)用于指定时序document中哪一个字段含有metadata。其中metadata
                                                  需要唯一的标识serial documents。metadata应该要基本保持不变。

                                                  另外，metaField字段的名称不能为_id，也不能与timeField字段同名。该字段的
                                                  可以为任意类型。



timeseries.granularity        string              (Optional)可选值可以为'seconds','minutes',或'hours'。默认情况下，
                                                  MongoDB会将granularity设置为'seconds'以应对高频的数据存储。
 
                                                  通过优化数据在时序集合中的存放方式，从而设置相应的granularity值可以提高
                                                  性能。通常来说，granularity的值应该与连续插入到集合中的数据在时域跨度上
                                                  保持一致。

                                                  假如指定了timeseries.metaField字段，考虑在对应的时间跨度内，连续插入的数据
                                                  通常具有相同的metaField字段。

                                                  假如未指定timeseries.metaField字段，考虑所有插入到集合中的数据的时间跨度。



expireAfterSeconds          string                (Optional)指定时序集合中的documents的过期时间，在过期后，会自动的删除对应
                                                  的documents数据。                                             
</pre>
针对```timeseries```还有如下的一些选项：

* storageEngine
* indexOptionDefaults
* collation
* writeConcern
* comment

2）**Insert Measurements into a Time Series Collection**

插入到时序集合中的每一个document都需要包含一个单独的measurement。如果要插入多个documents的话，执行如下方法：
{% highlight string %}
db.weather.insertMany( [
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T00:00:00.000Z"),
      "temp": 12
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T04:00:00.000Z"),
      "temp": 11
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T08:00:00.000Z"),
      "temp": 11
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T12:00:00.000Z"),
      "temp": 12
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T16:00:00.000Z"),
      "temp": 16
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-18T20:00:00.000Z"),
      "temp": 15
   }, {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T00:00:00.000Z"),
      "temp": 13
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T04:00:00.000Z"),
      "temp": 12
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T08:00:00.000Z"),
      "temp": 11
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T12:00:00.000Z"),
      "temp": 12
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T16:00:00.000Z"),
      "temp": 17
   },
   {
      "metadata": { "sensorId": 5578, "type": "temperature" },
      "timestamp": ISODate("2021-05-19T20:00:00.000Z"),
      "temp": 12
   }
] )
{% endhighlight %}

如果只是插入一个document，那么可以使用[db.collection.insertOne()](https://docs.mongodb.com/manual/reference/method/db.collection.insertOne/#mongodb-method-db.collection.insertOne)方法


3) **Query a Time Series Collectionicons**

要从时序集合中查询数据的话，可以执行如下命令：
{% highlight string %}
db.weather.findOne({
   "timestamp": ISODate("2021-05-18T00:00:00.000Z")
})
{% endhighlight %}

4) **Run Aggregations on a Time Series Collection**

对于一些额外的查询功能，可以使用[聚合框架( aggregation framework)](https://docs.mongodb.com/manual/aggregation/#std-label-aggregation-framework):
{% highlight string %}
db.weather.aggregate( [
   {
      $project: {
         date: {
            $dateToParts: { date: "$timestamp" }
         },
         temp: 1
      }
   },
   {
      $group: {
         _id: {
            date: {
               year: "$date.year",
               month: "$date.month",
               day: "$date.day"
            }
         },
         avgTmp: { $avg: "$temp" }
      }
   }
] )
{% endhighlight %}
上面的例子中，聚合管道(aggregation pipeline)会将所有的documents按日期进行分组，然后返回当天温度这一meansurements的平均值：
{% highlight string %}
 {
  "_id" : {
    "date" : {
      "year" : 2021,
      "month" : 5,
      "day" : 18
    }
  },
  "avgTmp" : 12.714285714285714
}
{
  "_id" : {
    "date" : {
      "year" : 2021,
      "month" : 5,
      "day" : 19
    }
  },
  "avgTmp" : 13
}
{% endhighlight %}

5) **Check if a Collection is of Type Time Series**

执行如下命令检查一个Collection是否为时序集合：
{% highlight string %}
db.runCommand( { listCollections: 1.0 } )
{% endhighlight %}

假如为time series collection的话，其返回如下结果：
{% highlight string %}
{
    cursor: {
       id: <number>,
       ns: 'test.$cmd.listCollections',
       firstBatch: [
         {
            name: <string>,
            type: 'timeseries',
            options: {
               expireAfterSeconds: <number>,
               timeseries: { ... }
            },
            ...
         },
         ...
       ]
    }
 }
{% endhighlight %}


###### 1.2 Behavior

时序集合与普通的集合类似，可以用平常的方法来向时序集合中插入数据，也可以从时序集合中查询数据。MongoDB将时序集合(time series collection)当作内部普通集合的一个视图(writable non-materialized views)，只不过是这个内部普通集合的数据存储方式进行了优化。

当查询时序集合时，每次查询一个document。在时序集合上做查询可以充分的利用内部优化的存储格式，并能更快的返回查询结果。

1） **Index**

时序集合在实现上是采用优化过的collection，可以降低磁盘占用并提高查询速度。Time series collections会自动的对数据进行排序，并按时间建立索引。针对```时序集合```其内部的索引，我们并不能够使用[ listIndexes](https://docs.mongodb.com/manual/reference/command/listIndexes/#mongodb-dbcommand-dbcmd.listIndexes)命令来获取。

>注：要提高查询效率，我们可以手动的在metaField以及timeField上增加二级索引。


2) **Default Compression Algorithm**

在创建collection时，如果没有指定```storageEngine```的话，则默认采用[zstd](https://docs.mongodb.com/manual/reference/glossary/#std-term-zstd)压缩算法。对于Time Series Collection来说，其会忽略该默认的压缩算法，而采用[snappy](https://docs.mongodb.com/manual/reference/glossary/#std-term-snappy)压缩算法。下面的例子将```weather```这个集合的压缩算法更改为```snappy```:
{% highlight string %}
db.createCollection(
  "weather",
  {
     timeseries: {
        timeField: "timestamp"
     },
     storageEngine: {
        wiredTiger: {
           configString: "block_compressor=snappy"
        }
     }
  }
)
{% endhighlight %}


其他有效的block_compressor还有：

* snappy
* zlib
* zstd (default)
* none

## 2. 时序集合的一些限制
1） **Constraints**

所要度量(meansurements)的document最大size为4MB。

2） **Updates and Deletes**

从MongoDB 5.05版本开始，我们可以执行删除(delete)与更新(updates)操作。

删除命令必须满足如下要求：

* 查询条件只能够匹配metaField字段的值

* 删除命令不能限制所删除的documents的数量。在执行删除命令时，必须设置为```justOne: false```参数，或者调用[deleteMany()](https://docs.mongodb.com/manual/reference/method/db.collection.deleteMany/#mongodb-method-db.collection.deleteMany)方法。

更新命令必须满足如下要求：

* 查询只能够匹配metaField字段的值

* 更新命令只能够修改metaField字段的值


## 3. 为时序集合创建自动删除功能(TTL)

## 4. 设置时序集合的粒度(Granularity)

## 5. 在metaField与timeField上创建二级索引

## 6. 将数据迁移到时序集合

## 7. 在时序数据上构建Materialized Views





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

