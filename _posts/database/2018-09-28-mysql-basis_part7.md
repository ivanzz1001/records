---
layout: post
title: SQL联接
tags:
- database
categories: database
description: mysql数据库基础
---


本章主要介绍一些数据库连接相关方面的内容:

* 内联接(INNER JOIN)

* 左联接(LEFT JOIN)

* 右联接(RIGHT JOIN)

* 全外联接(FULL OUTER JOIN)


<!-- more -->

## 1. SQL联接
SQL包含各种连接，从大的种类上来说可以分为：

* 内联接

* 外联接

* 交叉联接

下面给出各种连接的```韦恩图```:

![db-sql-join](https://ivanzz1001.github.io/records/assets/img/db/sql_joins.png)

为了讲解下面的各种联接，我们先定义如下```student```表与```course```表：
{% highlight string %}
mysql> CREATE TABLE student (
    -> stuid int PRIMARY KEY,
    -> stuname varchar(128) NOT NULL
    -> )ENGINE InnoDB DEFAULT CHARACTER SET utf8;
Query OK, 0 rows affected (0.06 sec)

mysql> CREATE TABLE course (
    -> id int PRIMARY KEY AUTO_INCREMENT,
    -> coursename varchar(128) NOT NULL,
    -> stuid int,
    -> CONSTRAINT foreign_stuid FOREIGN KEY (stuid) REFERENCES student(stuid)
    -> )ENGINE InnoDB DEFAULT CHARACTER SET utf8;
Query OK, 0 rows affected (0.09 sec)

mysql> INSERT INTO student (stuid, stuname) VALUES (1001, 'ivan1001');
mysql> INSERT INTO student (stuid, stuname) VALUES (1002, '郭晋安');
mysql> INSERT INTO student (stuid, stuname) VALUES (1003, '杨怡');
mysql> INSERT INTO student (stuid, stuname) VALUES (1004, '林文龙');
mysql> INSERT INTO student (stuid, stuname) VALUES (1005, '周丽淇');
mysql> INSERT INTO student (stuid, stuname) VALUES (1006, '天堂哥');
mysql> INSERT INTO student (stuid, stuname) VALUES (1007, '欢喜哥');


mysql> INSERT INTO course (id, coursename, stuid) VALUES (1, 'MySQL从入门到精通', 1001);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (2, '爱情与婚姻', 1002);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (3, 'Java从入门到放弃', 1003);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (4, '商务礼仪', 1004);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (5, '表演的艺术', 1005);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (6, '民法', 1006);
mysql> INSERT INTO course (id, coursename, stuid) VALUES (7, '民法', 1001);

mysql> select * from student;
+-------+-----------+
| stuid | stuname   |
+-------+-----------+
|  1001 | ivan1001  |
|  1002 | 郭晋安    |
|  1003 | 杨怡      |
|  1004 | 林文龙    |
|  1005 | 周丽淇    |
|  1006 | 天堂哥    |
|  1007 | 欢喜哥    |
+-------+-----------+
7 rows in set (0.00 sec)

mysql> select * from course;
+----+-------------------------+-------+
| id | coursename              | stuid |
+----+-------------------------+-------+
|  1 | MySQL从入门到精通       |  1001 |
|  2 | 爱情与婚姻              |  1002 |
|  3 | Java从入门到放弃        |  1003 |
|  4 | 商务礼仪                |  1004 |
|  5 | 表演的艺术              |  1005 |
|  6 | 民法                    |  1006 |
|  7 | 民法                    |  1001 |
+----+-------------------------+-------+
7 rows in set (0.00 sec)
{% endhighlight %}


## 2. 内联接

内联接是一种```一一映射```关系，就是两张表都有的才显示出来。用韦恩图表示是两个集合的交集：

![inner-join](https://ivanzz1001.github.io/records/assets/img/db/inner_join.png)

其中内联接又可以细分成如下三种：

* 等值联接： 在连接条件中使用等于运算符(=)比较被联接列的列值，其查询结果中列出被联接表中的所有列，包括其中的重复列

* 不等值联接： 在连接条件中使用除等号运算符以外的其他比较运算符比较被连接列的列值，这些运算符包括```>,>=,<=,<,!>,!<```,和```<>```

* 自然联接： 在连接条件中使用等于运算符(=)比较被连接的列值，但它使用选择列表指出查询结果集合中所包括的列，并删除联接表中的重复列。注意，自然联接要求外键名称(foreign key)与所指向的表的主键名称(primary key)必须相同，且nature join会合并相同的列

参看如下示例：
{% highlight string %}

mysql> select * from course as cs, student as st where cs.stuid=st.stuid;
+----+-------------------------+-------+-------+-----------+
| id | coursename              | stuid | stuid | stuname   |
+----+-------------------------+-------+-------+-----------+
|  1 | MySQL从入门到精通       |  1001 |  1001 | ivan1001  |
|  2 | 爱情与婚姻              |  1002 |  1002 | 郭晋安    |
|  3 | Java从入门到放弃        |  1003 |  1003 | 杨怡      |
|  4 | 商务礼仪                |  1004 |  1004 | 林文龙    |
|  5 | 表演的艺术              |  1005 |  1005 | 周丽淇    |
|  6 | 民法                    |  1006 |  1006 | 天堂哥    |
|  7 | 民法                    |  1001 |  1001 | ivan1001  |
+----+-------------------------+-------+-------+-----------+
7 rows in set (0.00 sec)

mysql> select * from course as cs INNER JOIN student as st ON cs.stuid=st.stuid;
+----+-------------------------+-------+-------+-----------+
| id | coursename              | stuid | stuid | stuname   |
+----+-------------------------+-------+-------+-----------+
|  1 | MySQL从入门到精通       |  1001 |  1001 | ivan1001  |
|  2 | 爱情与婚姻              |  1002 |  1002 | 郭晋安    |
|  3 | Java从入门到放弃        |  1003 |  1003 | 杨怡      |
|  4 | 商务礼仪                |  1004 |  1004 | 林文龙    |
|  5 | 表演的艺术              |  1005 |  1005 | 周丽淇    |
|  6 | 民法                    |  1006 |  1006 | 天堂哥    |
|  7 | 民法                    |  1001 |  1001 | ivan1001  |
+----+-------------------------+-------+-------+-----------+
7 rows in set (0.00 sec)

mysql> select * from course as cs NATURAL JOIN student as st;
+-------+----+-------------------------+-----------+
| stuid | id | coursename              | stuname   |
+-------+----+-------------------------+-----------+
|  1001 |  1 | MySQL从入门到精通       | ivan1001  |
|  1002 |  2 | 爱情与婚姻              | 郭晋安    |
|  1003 |  3 | Java从入门到放弃        | 杨怡      |
|  1004 |  4 | 商务礼仪                | 林文龙    |
|  1005 |  5 | 表演的艺术              | 周丽淇    |
|  1006 |  6 | 民法                    | 天堂哥    |
|  1001 |  7 | 民法                    | ivan1001  |
+-------+----+-------------------------+-----------+
7 rows in set (0.00 sec)
{% endhighlight %}

## 2. 外联接
外联接主要包括：

* 左外联接(left join 或 left outer join)

* 右外联接(right join 或 right outer join)

* 全联接(full join)

### 2.1 左外联接

左外联接的结果包括```LEFT OUTER```子句中指定的左表的所有行，而不仅仅是联接列所匹配的行。如果左表的某行在右表中没有匹配，则在相关联的结果集中右表的所有选择列均为NULL。下面给出左外连接```韦恩图```:

![left-join](https://ivanzz1001.github.io/records/assets/img/db/left_join.png)

参看如下示例：
{% highlight string %}
mysql> SELECT cs.*, st.stuname FROM course AS cs  LEFT JOIN student AS st on st.stuid=cs.stuid;
+----+-------------------------+-------+-----------+
| id | coursename              | stuid | stuname   |
+----+-------------------------+-------+-----------+
|  1 | MySQL从入门到精通       |  1001 | ivan1001  |
|  2 | 爱情与婚姻              |  1002 | 郭晋安    |
|  3 | Java从入门到放弃        |  1003 | 杨怡      |
|  4 | 商务礼仪                |  1004 | 林文龙    |
|  5 | 表演的艺术              |  1005 | 周丽淇    |
|  6 | 民法                    |  1006 | 天堂哥    |
|  7 | 民法                    |  1001 | ivan1001  |
+----+-------------------------+-------+-----------+
7 rows in set (0.00 sec)

mysql> SELECT cs.*, st.stuname FROM student AS st  LEFT JOIN course cs ON st.stuid=cs.stuid;
+------+-------------------------+-------+-----------+
| id   | coursename              | stuid | stuname   |
+------+-------------------------+-------+-----------+
|    1 | MySQL从入门到精通       |  1001 | ivan1001  |
|    7 | 民法                    |  1001 | ivan1001  |
|    2 | 爱情与婚姻              |  1002 | 郭晋安    |
|    3 | Java从入门到放弃        |  1003 | 杨怡      |
|    4 | 商务礼仪                |  1004 | 林文龙    |
|    5 | 表演的艺术              |  1005 | 周丽淇    |
|    6 | 民法                    |  1006 | 天堂哥    |
| NULL | NULL                    |  NULL | 欢喜哥    |
+------+-------------------------+-------+-----------+
8 rows in set (0.00 sec)
{% endhighlight %}

### 2.2 右外联接
右外联接是左外联接的反向联接，将返回右表的所有行。如果右表的某行在左表中没有匹配的行，则将左表返回空值。下面给出右外联接```韦恩图```:

![right-join](https://ivanzz1001.github.io/records/assets/img/db/right_join.png)

{% highlight string %}
mysql> SELECT cs.*, st.stuname FROM course AS cs  RIGHT JOIN student AS st on st.stuid=cs.stuid;
+------+-------------------------+-------+-----------+
| id   | coursename              | stuid | stuname   |
+------+-------------------------+-------+-----------+
|    1 | MySQL从入门到精通       |  1001 | ivan1001  |
|    7 | 民法                    |  1001 | ivan1001  |
|    2 | 爱情与婚姻              |  1002 | 郭晋安    |
|    3 | Java从入门到放弃        |  1003 | 杨怡      |
|    4 | 商务礼仪                |  1004 | 林文龙    |
|    5 | 表演的艺术              |  1005 | 周丽淇    |
|    6 | 民法                    |  1006 | 天堂哥    |
| NULL | NULL                    |  NULL | 欢喜哥    |
+------+-------------------------+-------+-----------+
8 rows in set (0.00 sec)

mysql> SELECT cs.*, st.stuname FROM student AS st  RIGHT JOIN course AS cs on st.stuid=cs.stuid;
+----+-------------------------+-------+-----------+
| id | coursename              | stuid | stuname   |
+----+-------------------------+-------+-----------+
|  1 | MySQL从入门到精通       |  1001 | ivan1001  |
|  2 | 爱情与婚姻              |  1002 | 郭晋安    |
|  3 | Java从入门到放弃        |  1003 | 杨怡      |
|  4 | 商务礼仪                |  1004 | 林文龙    |
|  5 | 表演的艺术              |  1005 | 周丽淇    |
|  6 | 民法                    |  1006 | 天堂哥    |
|  7 | 民法                    |  1001 | ivan1001  |
+----+-------------------------+-------+-----------+
7 rows in set (0.00 sec)
{% endhighlight %}

### 2.3 全联接
完整外部联接返回左表和右表中的所有行。当某行在另一个表中没有匹配行时，则另一个表的选择列包含空值。如果表之间有匹配行，则整个结果集包含基表的数据值。下面给出全联接的```韦恩图```:

![full-join](https://ivanzz1001.github.io/records/assets/img/db/full_join.png)

参看如下示例：
{% highlight string %}
mysql> SELECT * FROM course AS cs FULL OUTER JOIN student as st ON cs.stuid=st.stuid;
ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server 
version for the right syntax to use near 'FULL OUTER JOIN student as st ON cs.stuid=st.stuid' at line 1
{% endhighlight %}

这里注意到MySQL并不支持全联接，这里我们只能用如下的代码进行模拟：
{% highlight string %}
mysql> SELECT * from course as cs LEFT JOIN student as st on cs.stuid=st.stuid
    -> UNION
    -> SELECT * from course AS cs RIGHT JOIN student as st on cs.stuid=st.stuid;
+------+-------------------------+-------+-------+-----------+
| id   | coursename              | stuid | stuid | stuname   |
+------+-------------------------+-------+-------+-----------+
|    1 | MySQL从入门到精通       |  1001 |  1001 | ivan1001  |
|    2 | 爱情与婚姻              |  1002 |  1002 | 郭晋安    |
|    3 | Java从入门到放弃        |  1003 |  1003 | 杨怡      |
|    4 | 商务礼仪                |  1004 |  1004 | 林文龙    |
|    5 | 表演的艺术              |  1005 |  1005 | 周丽淇    |
|    6 | 民法                    |  1006 |  1006 | 天堂哥    |
|    7 | 民法                    |  1001 |  1001 | ivan1001  |
| NULL | NULL                    |  NULL |  1007 | 欢喜哥    |
+------+-------------------------+-------+-------+-----------+
8 rows in set (0.03 sec)
{% endhighlight %}


### 2.4 左外联接不包含内联接
这是```LEFT JOIN EXCLUDING INNER JOIN```：这个查询是只查询左边表有的数据，共同有的也不查出来。用```韦恩图```表示如下：

![left-join-excluding](https://ivanzz1001.github.io/records/assets/img/db/left_join_excluding.png)

参看如下示例：
{% highlight string %}
mysql> SELECT * FROM course as cs LEFT JOIN student AS st on cs.stuid=st.stuid WHERE st.stuid IS NULL;
Empty set (0.03 sec)
{% endhighlight %}

### 2.5 右外联接不包含内联接
这是```RIGHT JOIN EXCLUDING INNER JOIN```: 这个查询是只查询右边表有的数据，共同有的也不查出来。用```韦恩图```表示如下：

![right-join-excluding](https://ivanzz1001.github.io/records/assets/img/db/right_join_excluding.png)

参看如下示例：
{% highlight string %}
mysql> SELECT * FROM course as cs RIGHT JOIN student AS st on cs.stuid=st.stuid WHERE cs.stuid IS NULL;
+------+------------+-------+-------+-----------+
| id   | coursename | stuid | stuid | stuname   |
+------+------------+-------+-------+-----------+
| NULL | NULL       |  NULL |  1007 | 欢喜哥    |
+------+------------+-------+-------+-----------+
1 row in set (0.00 sec)
{% endhighlight %}



### 2.6 外联接不包含内联接
这是```OUTER JOIN EXCLUDING INNER JOIN```: 意思就是查询左右表各自拥有的那部分数据。用```韦恩图```表示如下：

![join-excluding](https://ivanzz1001.github.io/records/assets/img/db/join_excluding.png)

参看如下示例：
{% highlight string %}
mysql> SELECT * FROM course AS cs FULL OUTER JOIN student as st ON cs.stuid=st.stuid WHERE cs.stuid is NULL AND st.stuid IS NULL;
ERROR 1064 (42000): You have an error in your SQL syntax; check the manual that corresponds to your MySQL server 
version for the right syntax to use near 'FULL OUTER JOIN student as st ON cs.stuid=st.stuid' at line 1
{% endhighlight %}

这里由于MySQL不支持FULL JOIN，因此我们通过如下代码进行模拟：
{% highlight string %}
mysql> SELECT * from course as cs LEFT JOIN student as st on cs.stuid=st.stuid WHERE st.stuid IS NULL
    -> UNION
    -> SELECT * from course AS cs RIGHT JOIN student as st on cs.stuid=st.stuid WHERE cs.stuid IS NULL;
+------+------------+-------+-------+-----------+
| id   | coursename | stuid | stuid | stuname   |
+------+------------+-------+-------+-----------+
| NULL | NULL       |  NULL |  1007 | 欢喜哥    |
+------+------------+-------+-------+-----------+
1 row in set (0.00 sec)
{% endhighlight %}

## 3. 交叉联接

交叉联接返回左表中的所有行，左表中的每一行与右表中的所有行组合。交叉联接又被称为笛卡尔积。参看如下示例：
{% highlight string %}
mysql> SELECT * from course AS cs CROSS JOIN student as st;
+----+-------------------------+-------+-------+-----------+
| id | coursename              | stuid | stuid | stuname   |
+----+-------------------------+-------+-------+-----------+
|  1 | MySQL从入门到精通       |  1001 |  1001 | ivan1001  |
|  2 | 爱情与婚姻              |  1002 |  1001 | ivan1001  |
|  3 | Java从入门到放弃        |  1003 |  1001 | ivan1001  |
|  4 | 商务礼仪                |  1004 |  1001 | ivan1001  |
|  5 | 表演的艺术              |  1005 |  1001 | ivan1001  |
|  6 | 民法                    |  1006 |  1001 | ivan1001  |
|  7 | 民法                    |  1001 |  1001 | ivan1001  |
|  1 | MySQL从入门到精通       |  1001 |  1002 | 郭晋安    |
|  2 | 爱情与婚姻              |  1002 |  1002 | 郭晋安    |
|  3 | Java从入门到放弃        |  1003 |  1002 | 郭晋安    |
|  4 | 商务礼仪                |  1004 |  1002 | 郭晋安    |
|  5 | 表演的艺术              |  1005 |  1002 | 郭晋安    |
|  6 | 民法                    |  1006 |  1002 | 郭晋安    |
|  7 | 民法                    |  1001 |  1002 | 郭晋安    |
|  1 | MySQL从入门到精通       |  1001 |  1003 | 杨怡      |
|  2 | 爱情与婚姻              |  1002 |  1003 | 杨怡      |
|  3 | Java从入门到放弃        |  1003 |  1003 | 杨怡      |
|  4 | 商务礼仪                |  1004 |  1003 | 杨怡      |
|  5 | 表演的艺术              |  1005 |  1003 | 杨怡      |
|  6 | 民法                    |  1006 |  1003 | 杨怡      |
|  7 | 民法                    |  1001 |  1003 | 杨怡      |
|  1 | MySQL从入门到精通       |  1001 |  1004 | 林文龙    |
|  2 | 爱情与婚姻              |  1002 |  1004 | 林文龙    |
|  3 | Java从入门到放弃        |  1003 |  1004 | 林文龙    |
|  4 | 商务礼仪                |  1004 |  1004 | 林文龙    |
|  5 | 表演的艺术              |  1005 |  1004 | 林文龙    |
|  6 | 民法                    |  1006 |  1004 | 林文龙    |
|  7 | 民法                    |  1001 |  1004 | 林文龙    |
|  1 | MySQL从入门到精通       |  1001 |  1005 | 周丽淇    |
|  2 | 爱情与婚姻              |  1002 |  1005 | 周丽淇    |
|  3 | Java从入门到放弃        |  1003 |  1005 | 周丽淇    |
|  4 | 商务礼仪                |  1004 |  1005 | 周丽淇    |
|  5 | 表演的艺术              |  1005 |  1005 | 周丽淇    |
|  6 | 民法                    |  1006 |  1005 | 周丽淇    |
|  7 | 民法                    |  1001 |  1005 | 周丽淇    |
|  1 | MySQL从入门到精通       |  1001 |  1006 | 天堂哥    |
|  2 | 爱情与婚姻              |  1002 |  1006 | 天堂哥    |
|  3 | Java从入门到放弃        |  1003 |  1006 | 天堂哥    |
|  4 | 商务礼仪                |  1004 |  1006 | 天堂哥    |
|  5 | 表演的艺术              |  1005 |  1006 | 天堂哥    |
|  6 | 民法                    |  1006 |  1006 | 天堂哥    |
|  7 | 民法                    |  1001 |  1006 | 天堂哥    |
|  1 | MySQL从入门到精通       |  1001 |  1007 | 欢喜哥    |
|  2 | 爱情与婚姻              |  1002 |  1007 | 欢喜哥    |
|  3 | Java从入门到放弃        |  1003 |  1007 | 欢喜哥    |
|  4 | 商务礼仪                |  1004 |  1007 | 欢喜哥    |
|  5 | 表演的艺术              |  1005 |  1007 | 欢喜哥    |
|  6 | 民法                    |  1006 |  1007 | 欢喜哥    |
|  7 | 民法                    |  1001 |  1007 | 欢喜哥    |
+----+-------------------------+-------+-------+-----------+
49 rows in set (0.00 sec)
{% endhighlight %}



<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [数据库左连接、右连接、内连接、全连接笔记](https://blog.csdn.net/u014204541/article/details/79739980)

3. [深入理解SQL的四种连接-左外连接、右外连接、内连接、全连接](https://www.cnblogs.com/yyjie/p/7788413.html)

4. [Visual Representation of SQL Joins](https://www.codeproject.com/Articles/33052/Visual-Representation-of-SQL-Joins)

5. [SQL的几种连接：内连接、左联接、右连接、全连接、交叉连接](https://www.cnblogs.com/zxlovenet/p/4005256.html)

<br />
<br />
<br />

