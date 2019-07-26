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

1） 明确指定了查询时所要采用的索引

如上我们通过```FORCE INDEX```语句强制指定了所采用的索引，则按该索引顺序返回结果。

2） 带where条件的查询

这种情况下假如在where条件上创建了索引，那么就按该索引顺序返回结果；否则通常会按主键的顺序来返回结果

3） 不带where条件的查询

如果是```select *```这样的查询，通常会按主键顺序返回结果； 否则需要根据查询优化器来分析返回结果的顺序。

4） 带有order by的查询

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



<br />
<br />
**[参看]**:


1. [Mysql 查询主键未指定排序时的默认排序问题](https://blog.csdn.net/weixin_34010949/article/details/91381143)

2. [MySQL的limit用法和分页查询的性能分析及优化](https://segmentfault.com/a/1190000008859706?utm_source=tag-newest)

3. [MySQL order by limit 分页数据重复问题](https://www.jianshu.com/p/544c319fd838)

3. [InnoDB索引](https://www.cnblogs.com/cjsblog/p/8447325.html)

<br />
<br />
<br />

