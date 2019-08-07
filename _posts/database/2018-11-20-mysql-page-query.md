---
layout: post
title: MySQL分页查询若干问题
tags:
- database
categories: database
description:  MySQL分页查询若干问题
---


本章我们介绍一下MySQL分页查询的一些问题，其中主要包括两方面：

* MySQL返回查询结果的排序问题

* MySQL分页查询的性能问题


<!-- more -->

## 1. MySQL对查询结果的排序

在很多实际应用中，我们经常需要跑批量任务分批按顺序把主键取出来。我们先执行如下语句：
{% highlight string %}
SELECT id FROM foo.bar LIMIT 10 OFFSET 0
+-----+
| id  |
+-----+
| 109 |
| 13  |
| 14  |
| 15  |
| 128 |
| 129 |
| 130 |
| 190 |
| 226 |
| 227 |
+-----+
{% endhighlight %}
我们发现虽然用```主键```去查，但结果没有按照主键排序。

现在我们用```SELECT *```来试试：
{% highlight string %}
SELECT * FROM foo.bar LIMIT 10 OFFSET 0
+----+-------+---+
| id | a     | b |
+----+-------+---+
| 1  | 24274 | 0 |
| 2  | 24274 | 0 |
| 3  | 24274 | 0 |
| 4  | 24274 | 0 |
| 5  | 24274 | 0 |
| 6  | 24274 | 0 |
| 7  | 24274 | 0 |
| 8  | 24274 | 0 |
| 9  | 24274 | 0 |
| 10 | 24274 | 0 |
+----+-------+---+
{% endhighlight %}
现在的结果是按主键进行排序。

###### 原因分析
为什么会出现这种情况呢？ 我们在未明确使用```ORDER BY```语句对数据进行排序的情况下，到底是按什么顺序返回结果的呢？下面我们尝试分析。

首先执行如下语句查看```SELECT *```的执行计划：
{% highlight string %}
EXPLAIN SELECT * FROM foo.bar LIMIT 10 OFFSET 0 \G
***************************[ 1. row ]***************************
id            | 1
select_type   | SIMPLE
table         | bar
partitions    | <null>
type          | ALL
possible_keys | <null>
key           | <null>
key_len       | <null>
ref           | <null>
rows          | 211
filtered      | 100.0
Extra         | <null>
{% endhighlight %}

通过上面我们看到```SELECT *```并没有走索引，使用了全表扫描，因此顺序为主键顺序。

接下来我们再来看```SELECT id```的执行计划：
{% highlight string %}
EXPLAIN SELECT id FROM foo.bar LIMIT 10 OFFSET 0 \G
***************************[ 1. row ]***************************
id            | 1
select_type   | SIMPLE
table         | bar
partitions    | <null>
type          | index
possible_keys | <null>
key           | idx_a
key_len       | 8
ref           | <null>
rows          | 211
filtered      | 100.0
Extra         | Using index
{% endhighlight %}
通过上面我们发现```select id```并没有用到聚簇索引。InnoDB二级索引会自动添加主键作为索引列最后一项，使用该索引也能做到覆盖查询。查询优化器使用该索引，导致返回的顺序不符合预期：
{% highlight string %}
SELECT a,id FROM foo.bar LIMIT 10 OFFSET 0
+------+-----+
| a    | id  |
+------+-----+
| 1004 | 109 |
| 1823 | 13  |
| 1823 | 14  |
| 1823 | 15  |
| 1823 | 128 |
| 1823 | 129 |
| 1823 | 130 |
| 1823 | 190 |
| 1823 | 226 |
| 1823 | 227 |
+------+-----+
{% endhighlight %}
通过上面我们发现，果然之前```select id```用的是```a```索引(idx_a)，并且是按照a,id的顺序排序。

现在我们执行如下语句：
{% highlight string %}
SELECT id FROM foo.bar FORCE INDEX(PRI) LIMIT 10 OFFSET 0
+----+
| id |
+----+
| 1  |
| 2  |
| 3  |
| 4  |
| 5  |
| 6  |
| 7  |
| 8  |
| 9  |
| 10 |
+----+
{% endhighlight %}
上面我们看到强制使用主键索引果然没问题了。

或者是我们使用```order by```来引导查询优化器使用主键索引也可以。

###### 总结

由此可见，对于查询结果的返回顺序，主要是受```索引的影响```。通常情况由如下：

1） **明确指定了查询时所要采用的索引**

如上我们通过```FORCE INDEX```语句强制指定了所采用的索引，则按该索引顺序返回结果。

2） **带where条件的查询**

这种情况下假如在where条件上创建了索引，那么就按该索引顺序返回结果；否则通常会按主键的顺序来返回结果

3） **不带where条件的查询**

如果是```select *```这样的查询，通常会按主键顺序返回结果； 否则需要根据查询优化器来分析返回结果的顺序。

4） **带有order by的查询**

本身这种查询只是引导数据库再对查询后的结果做进一步的排序。（注： 数据库所采用的排序算法并不一定是稳定的排序算法，因此多次执行相同的语句查询返回的结果可能会并不一样）

*题外话：* 我们在执行MySQL limit分页时，通常需要有一个明确的结果返回顺序，否则就可能会出现在不同的页面出现相同的查询记录。因此通常我们会使用order by语句对查询的结果按主键进行排序；或者通过上面讲的以主键作为where查询条件，这样就能保证按主键的顺序正常返回结果。因此我们经常看到如下写法：
{% highlight string %}
SELECT * FROM `cdb_posts` ORDER BY pid LIMIT 1000000,30


//在第2个LIMIT前面我们可以不用再加ORDER BY了，因此此时where条件的查询默认就会使用主键索引来进行，因此默认就是主键排序了
SELECT * FROM `cdb_posts` WHERE pid >= (SELECT pid FROM  `cdb_posts` ORDER BY pid LIMIT 1000000,1) LIMIT 30
{% endhighlight %}

## 2. MySQL Limit分页查询

### 2.1 limit用法
在我们使用查询语句的时候，经常要返回前几条或者中间某几行数据，这个时候怎么办呢？不用担心，mysql已经为我们提供了这样一个功能：
{% highlight string %}
SELECT * FROM table LIMIT [offset,] rows | `rows OFFSET offset`
{% endhighlight %}
*LIMIT*子句可以被用于强制*SELECT*语句返回指定的记录数。*LIMIT*接受一个或两个数字参数，且参数必须是整数常量。如果给定两个参数，第一个参数指定第一个返回记录行的```偏移量```，第二个参数指定返回记录行的最大数目。初始记录行的偏移量是0（而不是1）。

为了与PostgreSQL兼容，MySQL也支持: LIMIT rows OFFSET offset
{% highlight string %}
mysql> SELECT * FROM table LIMIT 5,10; // 检索记录行 6-15 
{% endhighlight %}

为了检索从某一个偏移量到记录集的结束所有的记录行，可以指定第二个参数为-1：
{% highlight string %}
mysql> SELECT * FROM table LIMIT 95,-1; // 检索记录行 96-last.
{% endhighlight %}

如果只给定一个参数，它表示返回最大的记录行数目：
{% highlight string %}
mysql> SELECT * FROM table LIMIT 5; //检索前 5 个记录行 
{% endhighlight %}
换句话说，```LIMIT n```等价于```LIMIT 0,n```。

### 2.2 MySQL分页查询语句的性能分析
MySQL分页sql语句，如果和```MSSQL``` 的TOP语法相比，那么MySQL的LIMIT语法要显得优雅了许多。使用它来分页是再自然不过的事情了。

1） **最基本的分页方式**
<pre>
SELECT ... FROM ... WHERE ... ORDER BY ... LIMIT ...
</pre>
在中小数据量的情况下，这样的SQL足够用了，唯一需要注意的问题就是确保使用了索引。举例来说，如果实际SQL类似下面语句，那么在category_id与id这两列建立复合索引比较好：
<pre>
SELECT * FROM articles WHERE category_id = 123 ORDER BY id LIMIT 50, 10
</pre>

2) **子查询的分页方式**

随着数据量的增加，页数会越来越多，查看后几页的SQL就可能类似：
<pre>
SELECT * FROM articles WHERE category_id = 123 ORDER BY id LIMIT 10000, 10
</pre>
一言以蔽之，就是越往后分页，LIMIT语句的偏移量就会越大，速度也会明显变慢。此时，我们可以通过```子查询```的方式来提高分页效率，大致如下：
{% highlight string %}
SELECT * FROM articles WHERE  id >=  
(SELECT id FROM articles  WHERE category_id = 123 ORDER BY id LIMIT 10000, 1) LIMIT 10
{% endhighlight %}

3) **JOIN分页方式**

此种方式大致如下：
{% highlight string %}
SELECT * FROM `content` AS t1   
JOIN (SELECT id FROM `content` ORDER BY id desc LIMIT ".($page-1)*$pagesize.", 1) AS t2   
WHERE t1.id <= t2.id ORDER BY t1.id desc LIMIT $pagesize; 
{% endhighlight %}
经过测试，join分页和子查询分页的效率基本在一个等级上，消耗的时间也基本一致。

为什么会这样呢？因为子查询是在索引上完成的，而普通的查询是在数据文件上完成的。通常来说，索引文件要比数据文件小得多，所以操作起来也会更有效率。

实际可以利用类似*策略模式*的方式去处理分页，比如判断如果是100页以内，就使用最基本的分页方式，大于100页，则使用子查询的分页方式。

### 2.3 MySQL LIMIT分页性能问题
对于有大数据量的mysql表来说，使用LIMIT分页存在很严重的性能问题。假如我们需要查询从1000000之后的30条记录：
{% highlight string %}
SQL代码1：平均用时6.6秒 SELECT * FROM `cdb_posts` ORDER BY pid LIMIT 1000000 , 30

SQL代码2：平均用时0.6秒 SELECT * FROM `cdb_posts` WHERE pid >= (SELECT pid FROM  
`cdb_posts` ORDER BY pid LIMIT 1000000 , 1) LIMIT 30
{% endhighlight %}
因为要取出所有字段内容，第一种需要跨越大量数据块并取出，而第二种基本上是根据索引字段定位后，才取出相应的内容，效率自然大大提升。对limit的优化，不是直接用limit，而是首先获取到offset的id，然后直接用limit size来获取数据。

可以看出，越往后分页，LIMIT语句的偏移量就会越大，两者速度差距也会越来越明显。

实际可以利用类似*策略模式*的方式去处理分页，比如判断如果是100页以内，就使用最基本的分页方式，大于100页，则使用子查询的分页方式。

###### 优化思想： 避免数据量大时扫描过多的记录

![mysql-limit-query](https://ivanzz1001.github.io/records/assets/img/db/mysql-limit-query.png)

为了保证index索引列连续，可以为每个表加一个自增字段，并且加上索引。

## 3. mysql orderby limit翻页数据重复的问题



<br />
<br />
**[参看]**:


1. [Mysql 查询主键未指定排序时的默认排序问题](https://blog.csdn.net/weixin_34010949/article/details/91381143)

2. [MySQL的limit用法和分页查询的性能分析及优化](https://segmentfault.com/a/1190000008859706?utm_source=tag-newest)

3. [MySQL order by limit 分页数据重复问题](https://www.jianshu.com/p/544c319fd838)

3. [InnoDB索引](https://www.cnblogs.com/cjsblog/p/8447325.html)

4. [mysql orderby limit 翻页数据重复的问题](https://www.cnblogs.com/wuwenshuai/p/7158389.html)

<br />
<br />
<br />

