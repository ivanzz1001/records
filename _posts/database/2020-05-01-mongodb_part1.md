---
layout: post
title: MongoDB的介绍及简单使用
tags:
- database
categories: database
description:  MongoDB的安装及使用
---

如下是MongoDB官网对自身的介绍：
>MongoDB was founded in 2007 by Dwight Merriman, Eliot Horowitz and Kevin Ryan – the team behind DoubleClick.
>
>At the Internet advertising company DoubleClick (now owned by Google), the team developed and used many custom data stores to work around the shortcomings of existing databases. The business served 400,000 ads per second, but often struggled with both scalability and agility. Frustrated, the team was inspired to create a database that tackled the challenges it faced at DoubleClick.
>
>This was when MongoDB was born.


<!-- more -->

## 1. MongoDB产品矩阵
MongoDB公司目前主要有如下产品矩阵：

* Atlas
* Enterprise Advanced
* Community Edition
* Realm

下面我们分别对其进行简单介绍。

1） **Atlas**

MongoDB Atlas是由公司创始团队所开发的一款多云数据库服务(multi-cloud database service)。当我们需要在（不同供应商的）云服务器上构建弹性、高效的全球应用时，Atlas可以帮助我们简化数据库的部署和管理。如下图所示：

![mongodb-atlas](https://ivanzz1001.github.io/records/assets/img/db/mongodb/atlas-plp-hero.svg)

>More: https://docs.atlas.mongodb.com/

2) **Enterprise Advanced**

企业版提供了一系列的产品(products)和服务(services)来提高MongoDB数据的安全性、性能，以及帮助用户更好的管理MongoDB数据库。

* Database Management

在数据库管理方面，提供了相应的管理工具来简化MongoDB数据库的运维。主要包括： Automation、Monitoring、Optimization、Backups等

* Data and Business Protection 

提供先进的访问控制(access control)和数据安全特性来保护数据库。可以很容易的将已存在的安全设施、工具等与MongoDB继承。主要包括： Auditing、Encryption、Authentication、Commercial License

* Support and Services 

购买了MongoDB企业版的客户，可以获得MongoDB公司的技术支持与服务。

3) **Community Edition**

MongoDB社区版分布式数据库实现了灵活的文档数据模型(document date model)，并支持ad-hoc查询、二级索引、实时聚合等功能，从而为数据的访问与分析提供有力的支撑。

4） **Realm**

MongoDB Realm是一个开发平台，主要用于支持现代数据驱动的应用程序。可以使用Realm来构建mobile、web、desktop、以及IOT应用。

## 2. MongoDB介绍
欢迎使用MongoDB 5.0手册！MongoDB是一个文档数据库，旨在简化开发和扩展(scaling)。手册(manual)会介绍MongoDB关键概念(key concepts)、查询语言，并提供MongoDB操作及管理上的一些思考与流程。

MongoDB提供了本地部署与云(cloud-hosted)部署两种选项：

* 对于本地部署(locally hosted deployments), MongoDB又分为```社区版```与```企业版```
    * MongoDB社区版是[开源并免费使用](https://github.com/mongodb/mongo)的版本
	* MongoDB企业版作为MongoDB Enterprise Advanced订阅的一部分提供，并对MongoDB的部署提供全面的支持。此外，MongoDB企业版也增加了一些特性的支持，比如LDAP以及Kerberos支持、磁盘加密和审计。
	
* MongoDB Atlas是云中托管的 MongoDB Enterprise 服务选项，无需安装开销，并提供免费层级来开始使用。

### 2.1 文档数据库(Document Database)
在MongoDB中一条记录(record)就是一个文档(document)，其是由field/value对所组成的一个数据结构。MongoDB documents与JSON Objects类似。values字段可以为其他documents、arrays或arrays of documents。如下所示：

![mongodb-documents](https://ivanzz1001.github.io/records/assets/img/db/mongodb/crud-annotated-document.bakedsvg.svg)

使用documents具有如下优点：

* 在许多编程语言中，documents对应于原生数据类型(native data types)

* 内嵌documents和数组可以降低昂贵连接(join)的需求

* 动态schema可以更好的支持多种形态

1） **集合/视图/按需实例化视图**

MongoDB将文档存储在集合中。集合类似于关系数据库中的表。另外，对于集合(collections)，MongoDB还支持：

* Read-only Views(Staring in MongoDB 3.4)

* On-Demand Materialized Views (Starting in MongoDB 4.2)

### 2.2 Key Features 
1) **High Performance¶**

MongoDB提供了高性能的数据持久化能力。特别是：

* 支持内嵌数据模型以降低数据库系统IO活动

* 通过索引实现高效的数据查询，并且索引的keys可以来自于内嵌documents与arrays

2) **Rich Query Language**

MongoDB提供了丰富的查询语言(query language)以支持读写操作(CRUD)，另外还支持如下：

* [Data Aggregation](https://docs.mongodb.com/manual/core/aggregation-pipeline/)

* [Text Search](https://docs.mongodb.com/manual/text-search/)和[Geospatial Queries](https://docs.mongodb.com/manual/tutorial/geospatial-tutorial/)

>注：请参看如下
> [SQL to MongoDB Mapping Chart](https://docs.mongodb.com/manual/reference/sql-comparison/)
> [SQL to Aggregation Mapping Chart](https://docs.mongodb.com/manual/reference/sql-aggregation-comparison/)

3) **High Availability**

MongoDB中实现的复制设施，称为[replica set](https://docs.mongodb.com/manual/replication/)，实现了：

* automatic failover

* data redundancy

一个```replica set```是一组维护相同数据集的MongoDB服务器，提供数据冗余，并增强数据的可用性。

4） **Horizontal Scalability**

MongoDB在核心功能层面提供水平扩展能力：

* 通过Sharding方式将数据存放到集群中

* 从3.4版本开始，MongoDB支持基于shard key的方式来创建zone。在一个平衡的集群中，MongoDB将zone所覆盖的读取和写入操作仅定向到zone中对应的分片。

5) **Support for Multiple Storage Engines**

MongoDB支持多存储引擎:

* [WiredTiger Storage Engine](https://docs.mongodb.com/manual/core/wiredtiger/) (including support for [Encryption at Rest](https://docs.mongodb.com/manual/core/security-encryption-at-rest/))

* [In-Memory Storage Engine](https://docs.mongodb.com/manual/core/inmemory/)

另外，MongoDB提供可插拔的存储引擎API以支持第三方开发新的存储引擎。


## 3. Getting Started 

在本节我们会介绍向MongoDB中插入数据以及查询数据。可以使用[MongoDB Web Shell](https://mws.mongodb.com/?version=latest)控制台来进行实验，暂时无需安装MongoDB。

1) **Switch Database**

当我们连接上```MongoDB Web Shell```控制台后，默认会有一个当前数据库。输入```db```命令就可以显示当前数据库：
{% highlight string %}
>>> db 
test 
test>
>>>
{% endhighlight %}
从上面我们看到，返回结果为```test```，这是当前所连MongoDB的默认数据库。

如果想要切换到其他数据库，可以使用```use <db>```命令来进行切换。例如，切换到```examples```数据库：
{% highlight string %}
>>> use examples 
switched to db examples 
examples>
>>>
{% endhighlight %}
值得指出的是，在你进行切换(switch)之前并不需要创建数据库。MongoDB会在你第一次存入数据时自动的创建相应的库。

为了检验我们确实切换到了```examples```数据库，我们可以在```MongoDB Web Shell```控制台继续输入```db```命令：
{% highlight string %}
>>> db 
examples 
examples>
>>>
{% endhighlight %}

2) **Populate a Collection (Insert)**

MongoDB将documents存放到[collections](https://docs.mongodb.com/manual/core/databases-and-collections/)中。collections与传统关系型数据库中的表(tables)类似。假如一个collection不存在，MongoDB会在你第一次将数据存放到该collection时进行创建。

下面的例子使用[db.collection.insertMany()](https://docs.mongodb.com/manual/reference/method/db.collection.insertMany/#mongodb-method-db.collection.insertMany)方法插入新的documents到```movies```集合中。你可以拷贝如下示例到```MongoDB Web Shell```:
{% highlight string %}
>>> use examples 
switched to db examples 
examples>
>>> db.movies.insertMany([
   {
      title: 'Titanic',
      year: 1997,
      genres: [ 'Drama', 'Romance' ],
      rated: 'PG-13',
      languages: [ 'English', 'French', 'German', 'Swedish', 'Italian', 'Russian' ],
      released: ISODate("1997-12-19T00:00:00.000Z"),
      awards: {
         wins: 127,
         nominations: 63,
         text: 'Won 11 Oscars. Another 116 wins & 63 nominations.'
      },
      cast: [ 'Leonardo DiCaprio', 'Kate Winslet', 'Billy Zane', 'Kathy Bates' ],
      directors: [ 'James Cameron' ]
   },
   {
      title: 'The Dark Knight',
      year: 2008,
      genres: [ 'Action', 'Crime', 'Drama' ],
      rated: 'PG-13',
      languages: [ 'English', 'Mandarin' ],
      released: ISODate("2008-07-18T00:00:00.000Z"),
      awards: {
         wins: 144,
         nominations: 106,
         text: 'Won 2 Oscars. Another 142 wins & 106 nominations.'
      },
      cast: [ 'Christian Bale', 'Heath Ledger', 'Aaron Eckhart', 'Michael Caine' ],
      directors: [ 'Christopher Nolan' ]
   },
   {
      title: 'Spirited Away',
      year: 2001,
      genres: [ 'Animation', 'Adventure', 'Family' ],
      rated: 'PG',
      languages: [ 'Japanese' ],
      released: ISODate("2003-03-28T00:00:00.000Z"),
      awards: {
         wins: 52,
         nominations: 22,
         text: 'Won 1 Oscar. Another 51 wins & 22 nominations.'
      },
      cast: [ 'Rumi Hiiragi', 'Miyu Irino', 'Mari Natsuki', 'Takashi Naitè' ],
      directors: [ 'Hayao Miyazaki' ]
   },
   {
      title: 'Casablanca',
      genres: [ 'Drama', 'Romance', 'War' ],
      rated: 'PG',
      cast: [ 'Humphrey Bogart', 'Ingrid Bergman', 'Paul Henreid', 'Claude Rains' ],
      languages: [ 'English', 'French', 'German', 'Italian' ],
      released: ISODate("1943-01-23T00:00:00.000Z"),
      directors: [ 'Michael Curtiz' ],
      awards: {
         wins: 9,
         nominations: 6,
         text: 'Won 3 Oscars. Another 6 wins & 6 nominations.'
      },
      lastupdated: '2015-09-04 00:22:54.600000000',
      year: 1942
   }
])
{
  acknowledged: true,
  insertedIds:{
    '0': ObjectId("61d11d5c168ab70a91d75c6b"),
	'1': ObjectId("61d11d5c168ab70a91d75c6c"),
	'2': ObjectId("61d11d5c168ab70a91d75c6d"),
	'3': ObjectId("61d11d5c168ab70a91d75c6e")
  }
}
examples>
>>>
{% endhighlight %}
上面的插入操作返回结果为一个document，其包含应答标识```acknowledged```及数组，数组的每一个元素为成功插入的document所对应的```_id```。


3） **Select All Documents**

要从一个collection中查询documents，我们可以使用[ db.collection.find()](https://docs.mongodb.com/manual/reference/method/db.collection.find/#mongodb-method-db.collection.find)方法。要查询collection中的所有documents，可以传递一个empty document作为查询过滤条件.

>查询过滤条件，请参看[query filter document](https://docs.mongodb.com/manual/core/document/#std-label-document-query-filter)

在```MongoDB Web Shell```中，输入如下命令查询上面```步骤2）```所插入到**movies**这个collection中的数据：
{% highlight string %}
>>> db.movies.find( { } )
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6b"),
      title: 'Titanic',
      year: 1997,
      genres: [ 'Drama', 'Romance' ],
      rated: 'PG-13',
      languages: [ 'English', 'French', 'German', 'Swedish', 'Italian', 'Russian' ],
      released: ISODate("1997-12-19T00:00:00.000Z"),
      awards: {
         wins: 127,
         nominations: 63,
         text: 'Won 11 Oscars. Another 116 wins & 63 nominations.'
      },
      cast: [ 
        'Leonardo DiCaprio',
        'Kate Winslet',
        'Billy Zane', 
        'Kathy Bates' 
	  ],
      directors: [ 'James Cameron' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6c"),
      title: 'The Dark Knight',
      year: 2008,
      genres: [ 'Action', 'Crime', 'Drama' ],
      rated: 'PG-13',
      languages: [ 'English', 'Mandarin' ],
      released: ISODate("2008-07-18T00:00:00.000Z"),
      awards: {
         wins: 144,
         nominations: 106,
         text: 'Won 2 Oscars. Another 142 wins & 106 nominations.'
      },
      cast: [ 
        'Christian Bale', 
        'Heath Ledger',
        'Aaron Eckhart', 
        'Michael Caine' 
      ],
      directors: [ 'Christopher Nolan' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6d"),
      title: 'Spirited Away',
      year: 2001,
      genres: [ 'Animation', 'Adventure', 'Family' ],
      rated: 'PG',
      languages: [ 'Japanese' ],
      released: ISODate("2003-03-28T00:00:00.000Z"),
      awards: {
         wins: 52,
         nominations: 22,
         text: 'Won 1 Oscar. Another 51 wins & 22 nominations.'
      },
      cast: [ 
        'Rumi Hiiragi',
        'Miyu Irino', 
        'Mari Natsuki',
        'Takashi Naitè' 
      ],
      directors: [ 'Hayao Miyazaki' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6e"),
      title: 'Casablanca',
      genres: [ 'Drama', 'Romance', 'War' ],
      rated: 'PG',
      cast: [ 
        'Humphrey Bogart', 
        'Ingrid Bergman', 
        'Paul Henreid',
        'Claude Rains' 
      ],
      languages: [ 'English', 'French', 'German', 'Italian' ],
      released: ISODate("1943-01-23T00:00:00.000Z"),
      directors: [ 'Michael Curtiz' ],
      awards: {
         wins: 9,
         nominations: 6,
         text: 'Won 3 Oscars. Another 6 wins & 6 nominations.'
      },
      lastupdated: '2015-09-04 00:22:54.600000000',
      year: 1942
   }
]
{% endhighlight %}


4) **Filter Data with Comparison Operators**

对于相等匹配(<field> equals <value>)，可以向db.collection.find()方法中传入```<field>: <value>```查询条件：

* 在MongoDB Web Shell中，执行如下命令查询由```Christopher Nolan```所导演的movies
{% highlight string %}
>>> db.movies.find( { "directors": "Christopher Nolan" } );
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6c"),
      title: 'The Dark Knight',
      year: 2008,
      genres: [ 'Action', 'Crime', 'Drama' ],
      rated: 'PG-13',
      languages: [ 'English', 'Mandarin' ],
      released: ISODate("2008-07-18T00:00:00.000Z"),
      awards: {
         wins: 144,
         nominations: 106,
         text: 'Won 2 Oscars. Another 142 wins & 106 nominations.'
      },
      cast: [ 
        'Christian Bale', 
        'Heath Ledger',
        'Aaron Eckhart', 
        'Michael Caine' 
      ],
      directors: [ 'Christopher Nolan' ]
   }
]
{% endhighlight %}

此外，我们也可以使用比较操作符([comparison operators](https://docs.mongodb.com/manual/reference/operator/query-comparison/#std-label-query-selectors-comparison))来执行更高级的查询。

* 执行如下命令查询在2000年之前所上映的movies
{% highlight string %}
>>> db.movies.find( { "released": { $lt: ISODate("2000-01-01") } } );
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6b"),
      title: 'Titanic',
      year: 1997,
      genres: [ 'Drama', 'Romance' ],
      rated: 'PG-13',
      languages: [ 'English', 'French', 'German', 'Swedish', 'Italian', 'Russian' ],
      released: ISODate("1997-12-19T00:00:00.000Z"),
      awards: {
         wins: 127,
         nominations: 63,
         text: 'Won 11 Oscars. Another 116 wins & 63 nominations.'
      },
      cast: [ 
        'Leonardo DiCaprio',
        'Kate Winslet',
        'Billy Zane', 
        'Kathy Bates' 
	  ],
      directors: [ 'James Cameron' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6e"),
      title: 'Casablanca',
      genres: [ 'Drama', 'Romance', 'War' ],
      rated: 'PG',
      cast: [ 
        'Humphrey Bogart', 
        'Ingrid Bergman', 
        'Paul Henreid',
        'Claude Rains' 
      ],
      languages: [ 'English', 'French', 'German', 'Italian' ],
      released: ISODate("1943-01-23T00:00:00.000Z"),
      directors: [ 'Michael Curtiz' ],
      awards: {
         wins: 9,
         nominations: 6,
         text: 'Won 3 Oscars. Another 6 wins & 6 nominations.'
      },
      lastupdated: '2015-09-04 00:22:54.600000000',
      year: 1942
   }
]
{% endhighlight %}

* 执行如下命令查询获得超过100个奖项的movies
{% highlight string %}
>>> db.movies.find( { "awards.wins": { $gt: 100 } } );
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6b"),
      title: 'Titanic',
      year: 1997,
      genres: [ 'Drama', 'Romance' ],
      rated: 'PG-13',
      languages: [ 'English', 'French', 'German', 'Swedish', 'Italian', 'Russian' ],
      released: ISODate("1997-12-19T00:00:00.000Z"),
      awards: {
         wins: 127,
         nominations: 63,
         text: 'Won 11 Oscars. Another 116 wins & 63 nominations.'
      },
      cast: [ 
        'Leonardo DiCaprio',
        'Kate Winslet',
        'Billy Zane', 
        'Kathy Bates' 
	  ],
      directors: [ 'James Cameron' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6c"),
      title: 'The Dark Knight',
      year: 2008,
      genres: [ 'Action', 'Crime', 'Drama' ],
      rated: 'PG-13',
      languages: [ 'English', 'Mandarin' ],
      released: ISODate("2008-07-18T00:00:00.000Z"),
      awards: {
         wins: 144,
         nominations: 106,
         text: 'Won 2 Oscars. Another 142 wins & 106 nominations.'
      },
      cast: [ 
        'Christian Bale', 
        'Heath Ledger',
        'Aaron Eckhart', 
        'Michael Caine' 
      ],
      directors: [ 'Christopher Nolan' ]
   }
]
{% endhighlight %}

* 执行如下命令，查询语言为```Japanese```或```Mandarin```的movies
{% highlight string %}
>>> db.movies.find( { "languages": { $in: [ "Japanese", "Mandarin" ] } } )
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6c"),
      title: 'The Dark Knight',
      year: 2008,
      genres: [ 'Action', 'Crime', 'Drama' ],
      rated: 'PG-13',
      languages: [ 'English', 'Mandarin' ],
      released: ISODate("2008-07-18T00:00:00.000Z"),
      awards: {
         wins: 144,
         nominations: 106,
         text: 'Won 2 Oscars. Another 142 wins & 106 nominations.'
      },
      cast: [ 
        'Christian Bale', 
        'Heath Ledger',
        'Aaron Eckhart', 
        'Michael Caine' 
      ],
      directors: [ 'Christopher Nolan' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6d"),
      title: 'Spirited Away',
      year: 2001,
      genres: [ 'Animation', 'Adventure', 'Family' ],
      rated: 'PG',
      languages: [ 'Japanese' ],
      released: ISODate("2003-03-28T00:00:00.000Z"),
      awards: {
         wins: 52,
         nominations: 22,
         text: 'Won 1 Oscar. Another 51 wins & 22 nominations.'
      },
      cast: [ 
        'Rumi Hiiragi',
        'Miyu Irino', 
        'Mari Natsuki',
        'Takashi Naitè' 
      ],
      directors: [ 'Hayao Miyazaki' ]
   }
]
{% endhighlight %}

>Tips: [Query and Projection Operators](https://docs.mongodb.com/manual/reference/operator/query/#std-label-query-projection-operators-top)

5) **Specify Fields to Return (Projection)**

我们可以通过向db.collection.find(<query document>, <projection document>)方法传递```projection document```的方式来指定查询结果所要返回的字段。在```projection document```中指定：

* ```<field>: 1```表示需要在结果中返回对应的字段

* ```<field>: 0```表示在结果中不返回对应的字段

在MongoDB Web Shell中执行如下查询，返回movies集合中的```id```、```title```、```directors```以及```year```字段：
{% highlight string %}
>>> db.movies.find( { }, { "title": 1, "directors": 1, "year": 1 } );
[
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6b"),
      title: 'Titanic',
      year: 1997,
      directors: [ 'James Cameron' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6c"),
      title: 'The Dark Knight',
      year: 2008,
      directors: [ 'Christopher Nolan' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6d"),
      title: 'Spirited Away',
      year: 2001,
      directors: [ 'Hayao Miyazaki' ]
   },
   {
      _id: ObjectId("61d11d5c168ab70a91d75c6e"),
      title: 'Casablanca',
      directors: [ 'Michael Curtiz' ],
      year: 1942
   }
]
{% endhighlight %}
默认情况下会自动返回```_id```，因此我们并不需要在查询中特别指定```_id```。如果我们不想返回该字段的话，那么将该字段设置为0即可。例如，执行如下查询命令返回所匹配的```title```和```genres```字段：
{% highlight string %}
>>> db.movies.find( { }, { "_id": 0, "title": 1, "genres": 1 } );
[
   { title: 'Titanic', genres: [ 'Drama', 'Romance' ] },
   { title: 'The Dark Knight', genres: [ 'Action', 'Crime', 'Drama' ] },
   {
      title: 'Spirited Away',
      genres: [ 'Animation', 'Adventure', 'Family' ]
   },
   { title: 'Casablanca', genres: [ 'Drama', 'Romance', 'War' ] }
]
{% endhighlight %}

6) **Aggregate Data ($group)**

可以使用聚集(aggregation)来对documents中的值进行分组，然后返回结果。在MongoDB中Aggregation是由[aggregation pipeline](https://docs.mongodb.com/manual/aggregation/#std-label-aggregation-framework)。

我们可以使用find()操作来获取数据，然而如果想做一些比[CRUD operations](https://docs.mongodb.com/manual/crud/#std-label-crud)更复杂的操作的话，比如manipulate data、perform calculations、write more expressive queries，我们可以使用aggregation pipeline。

在Shell中，执行如下的aggregation pipeline可以统计每一个```genres```值的出现次数。
{% highlight string %}
>>> db.movies.aggregate( [
   { $unwind: "$genres" },
   {
     $group: {
       _id: "$genres",
       genreCount: { $count: { } }
     }
   },
   { $sort: { "genreCount": -1 } }
] )
[
   { _id: 'Drama', genreCount: 3 },
   { _id: 'Romance', genreCount: 2 },
   { _id: 'Action', genreCount: 1 },
   { _id: 'Adventure', genreCount: 1 },
   { _id: 'Animation', genreCount: 1 },
   { _id: 'Crime', genreCount: 1 },
   { _id: 'Family', genreCount: 1 },
   { _id: 'War', genreCount: 1 }
   
]
{% endhighlight %}

上面命令在pipeline中使用了：

* ```$unwind```为genres数组中的每一个元素输出一个document

* ```$group```与```$count```累加器计算genres中元素的出现次数，次数会被存放在```genreCount```字段中；

* ```$sort```用于对输出结果根据genreCount字段按降序返回




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

