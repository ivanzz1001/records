---
layout: post
title: MongoDB数据库与集合
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



timeseries.granularity        string              (Optional)

</pre>









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

