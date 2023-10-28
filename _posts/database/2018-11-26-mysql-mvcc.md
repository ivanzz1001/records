---
layout: post
title: MySQL MVCC实现原理(转)
tags:
- database
categories: database
description:  MySQL MVCC实现原理
---


从网上看了很多关于MVCC的相关文章，发现基本都有某种错误之处。本文转自[聊聊 MySQL的 MVCC](https://zhuanlan.zhihu.com/p/475415473), 个人觉得也含有一些错误，因此最好再对照如下两篇文章来理解：

* [浅谈MySQL中的MVCC](https://zhuanlan.zhihu.com/p/482398377)

* [MySQL MVCC底层原理详解](https://zhuanlan.zhihu.com/p/442643107)

<!-- more -->


## 1. MVCC是什么
MVCC，全称Multi-Version Concurrency Control，即多版本并发控制。MVCC是一种并发控制的方法，一般在数据库管理系统中，实现对数据库的并发访问，在编程语言中实现事务内存。mvcc - @百度百科

多版本控制: 指的是一种提高并发的技术。最早的数据库系统，只有读读之间可以并发，读写，写读，写写都要阻塞。引入多版本之后，只有写写之间相互阻塞，其他三种操作都可以并行，这样大幅度提高了InnoDB的并发度。

## 2. MVCC能解决什么问题

### 2.1 并发场景梳理
数据库并发场景有三种，分别为：

* 读-读：不存在任何问题，也不需要并发控制

* 读-写：有线程安全问题，可能会造成事务隔离性问题，可能遇到脏读，幻读，不可重复读

* 写-写：有线程安全问题，可能会存在更新丢失问题，比如第一类更新丢失，第二类更新丢失


### 2.2 MVCC的好处
多版本并发控制（MVCC）是一种用来解决```读-写```冲突的无锁并发控制，也就是为事务分配单向增长的时间戳，为每个修改保存一个版本，版本与事务时间戳关联，读操作只读该事务开始前的数据库的快照。 所以MVCC可以为数据库解决以下问题: 

* 在并发读写数据库时，可以做到在读操作时不用阻塞写操作，写操作也不用阻塞读操作，提高了数据库并发读写的性能

* 同时还可以解决脏读，幻读（部分解决），不可重复读等事务隔离问题，但不能解决更新丢失问题

>ps: 写-写冲突还是需要通过加锁的方式来实现

## 3. MVCC实现原理
MVCC的目的就是多版本并发控制，在数据库中的实现，就是为了解决读写冲突，它的实现原理主要是依赖记录中的 3个隐式字段，undo日志, Read View 来实现的。所以我们先来看看这个三个point的概念.


### 3.1 三个隐式字段
每行记录除了我们自定义的字段外，还有数据库隐式定义的```DB_TRX_ID```,```DB_ROLL_PTR```,```DB_ROW_ID```等字段:

* DB_TRX_ID: 6byte，最近修改(修改/插入)事务ID, 记录创建这条记录/最后一次修改该记录的事务ID

* DB_ROLL_PTR: 7byte，回滚指针，指向这条记录的上一个版本（存储于rollback segment里）

* DB_ROW_ID: 6byte，隐含的自增ID（隐藏主键），如果数据表没有主键，InnoDB会自动以DB_ROW_ID产生一个聚簇索引

实际还有一个删除flag隐藏字段, 既记录被更新或删除并不代表真的删除，而是删除flag变了:

![mvcc-1](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc1.webp)

如上图，```DB_ROW_ID```是数据库默认为该行记录生成的唯一隐式主键; ```DB_TRX_ID```是当前操作该记录的事务ID; 而```DB_ROLL_PTR```是一个回滚指针，用于配合undo日志，指向上一个旧版本

### 3.2 undo日志
undo log主要分为两种：

* insert undo log

代表事务在insert新记录时产生的undo log, 只在事务回滚时需要，并且在事务提交后可以被立即丢弃

* update undo log

事务在进行update或delete时产生的undo log; 不仅在事务回滚时需要，在快照读时也需要；所以不能随便删除，只有在快速读或事务回滚不涉及该日志时，对应的日志才会被统一清除

对MVCC有帮助的实质是update undo log ，undo log实际上就是存在rollback segment中旧记录链，它的执行流程如下:

1) 比如一个有个事务插入person表插入了一条新记录，记录如下，name为Jerry, age为24岁，隐式主键是1，事务ID和回滚指针，我们假设为NULL

![mvcc-2](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc2.webp)


2) 现在来了一个事务1对该记录的name做出了修改，改为Tom

在事务1修改该行(记录)数据时，数据库会先对该行加排他锁

然后把该行数据拷贝到undo log中，作为旧记录，既在undo log中有当前行的拷贝副本。拷贝完毕后，修改该行name为Tom，并且修改隐藏字段的事务ID为当前事务1的ID, 我们默认从1开始，之后递增，回滚指针指向拷贝到undo log的副本记录，既表示我的上一个版本就是它。

事务提交后，释放锁

![mvcc-3](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc3.webp)

3） 又来了个事务2修改person表的同一个记录，将age修改为30岁

在事务2修改该行数据时，数据库也先为该行加锁

然后把该行数据拷贝到undo log中，作为旧记录，发现该行记录已经有undo log了，那么最新的旧数据作为链表的表头，插在该行记录的undo log最前面

修改该行age为30岁，并且修改隐藏字段的事务ID为当前事务2的ID, 那就是2，回滚指针指向刚刚拷贝到undo log的副本记录

事务提交，释放锁

![mvcc-4](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc4.webp)

从上面，我们就可以看出，不同事务或者相同事务的对同一记录的修改，会导致该记录的undo log成为一条记录版本线性表，既链表，undo log的链首就是最新的旧记录，链尾就是最早的旧记录（当然就像之前说的该undo log的节点可能是会被清除掉，像图中的第一条insert undo log，其实在事务提交之后可能就被删除丢失了，不过这里为了演示，所以还放在这里）


### 3.3 Read View(读视图)
#### 3.3.1 什么是Read View?
什么是Read View，说白了Read View就是事务进行快照读操作的时候生产的读视图(Read View)，在该事务执行的快照读的那一刻，会生成数据库系统当前的一个快照，记录并维护系统当前活跃事务的ID(当每个事务开启时，都会被分配一个ID, 这个ID是递增的，所以最新的事务，ID值越大)

>注意： ReadView是与SQL绑定的，而并不是事务，所以即使在同一个事务中，每次SQL启动时构造的ReadView的up_trx_id和low_trx_id也都是不一样的，至于DATA_TRX_ID大于low_trx_id本身出现也只有当多个SQL并发的时候，在一个SQL构造完ReadView之后，另外一个SQL修改了数据后又进行了提交，对于这种情况，数据其实是不可见的。

所以我们知道 Read View主要是用来做可见性判断的, 即当我们某个事务执行快照读的时候，对该记录创建一个Read View读视图，把它比作条件用来判断当前事务能够看到哪个版本的数据，既可能是当前最新的数据，也有可能是该行记录的undo log里面的某个版本的数据。

Read View遵循一个可见性算法，主要是将要被修改的数据的最新记录中的```DB_TRX_ID```(即当前事务ID）取出来，与系统当前其他活跃事务的ID去对比（由Read View维护），如果```DB_TRX_ID```跟Read View的属性做了某些比较，不符合可见性，那就通过DB_ROLL_PTR回滚指针去取出Undo Log中的```DB_TRX_ID```再比较，即遍历链表的```DB_TRX_ID```(从链首到链尾，即从最近的一次修改查起），直到找到满足特定条件的```DB_TRX_ID```, 那么这个```DB_TRX_ID```所在的旧记录就是当前事务能看见的最新老版本

那么这个判断条件是什么呢？

![mvcc-5](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc5.webp)

如上，它是一段MySQL判断可见性的一段源码，即changes_visible方法（不完全哈，但能看出大致逻辑），该方法展示了我们拿DB_TRX_ID去跟Read View某些属性进行怎么样的比较。

在展示之前，我先简化一下Read View，我们可以把Read View简单的理解成有三个全局属性：

* trx_list(名字随便起的): 一个数值列表，用来维护Read View生成时刻系统正活跃的事务ID

* up_limit_id: ReadView生成时刻系统尚未分配的下一个事务ID，也就是目前已出现过的事务ID的最大值+1

* low_limit_id: 记录trx_list列表中事务ID最小的ID

![mvcc-6](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc6.webp)


这样，对于当前事务的启动瞬间来说，一个数据版本的row trx_id，有以下几种可能：
>ps: 实际read view并不是在事务启动瞬间创建的

* 如果落在绿色部分，表示这个版本是已提交的事务或者是当前事务自己生成的，这个数据是可见的；

* 如果落在红色部分，表示这个版本是由将来启动的事务生成的，是肯定不可见的；

* 如果落在黄色部分，那就包括两种情况

  - 若 row trx_id在数组中，表示这个版本是由还没提交的事务生成的，不可见；

  - 若 row trx_id不在数组中，表示这个版本是已经提交了的事务生成的，可见。

所以你现在知道了，InnoDB利用了“所有数据都有多个版本”的这个特性，实现了“秒级创建快照”的能力。

#### 3.3.2 举例分析

创建如下表并插入2条数据: 
{% highlight string %}
CREATE TABLE `t` (
`id` int(11) NOT NULL,

`k` int(11) DEFAULT NULL,

PRIMARY KEY (`id`)
) ENGINE=InnoDB;

insert into t(id, k) values(1,1),(2,2);
{% endhighlight %}

接下来，我们继续看一下图1中的三个事务，分析下事务A的语句返回的结果，为什么是k=1。

>ps: 下面的例子可能存在错误，ReadView是与SQL绑定，而并非与事务绑定。这里我们主要看一下其分析过程

![mvcc-7](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc7.webp)

这里，我们不妨做如下假设:
<pre>
事务A开始前，系统里面只有一个活跃事务ID是99；

事务A、B、C的版本号分别是100、101、102，且当前系统里只有这四个事务；

三个事务开始前，(1,1）这一行数据的row trx_id是90。
</pre>

这样，事务A的视图数组就是[99,100], 事务B的视图数组是[99,100,101], 事务C的视图数组是[99,100,101,102]。

为了简化分析，我先把其他干扰语句去掉，只画出跟事务A查询逻辑有关的操作:


![图4 事务A查询数据逻辑图](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc8.webp)

1） 从图中可以看到，第一个有效更新是事务C，把数据从(1,1)改成了(1,2)。这时候，这个数据的最新版本的row trx_id是102，而90这个版本已经成为了历史版本。


2） 第二个有效更新是事务B，把数据从(1,2)改成了(1,3)。这时候，这个数据的最新版本（即row trx_id）是101，而102又成为了历史版本。

你可能注意到了，在事务A查询的时候，其实事务B还没有提交，但是它生成的(1,3)这个版本已经变成当前版本了。但这个版本对事务A必须是不可见的，否则就变成脏读了。

好，现在事务A要来读数据了，它的视图数组是[99,100]。当然了，读数据都是从当前版本读起的。所以，事务A查询语句的读数据流程是这样的:
>ps: 事实上，在事务A中是在真正开始读取k的值时才创建ReadView的，此时活跃的事务数组应该为[99, 100, 101], 此时low_limit_id为99, up_limit_id为103，在事务A中是可以读取到事务C所提交的数据

1) 找到(1,3)的时候，判断出row trx_id=101，比高水位大，处于红色区域，不可见；

2） 接着，找到上一个历史版本，一看row trx_id=102，比高水位大，处于红色区域，不可见；

3） 再往前找，终于找到了（1,1)，它的row trx_id=90，比低水位小，处于绿色区域，可见。

这样执行下来，虽然期间这一行数据被修改过，但是事务A不论在什么时候查询，看到这行数据的结果都是一致的，所以我们称之为一致性读。

这个判断规则是从代码逻辑直接转译过来的，但是正如你所见，用于人肉分析可见性很麻烦。

所以，我来给你翻译一下。一个数据版本，对于一个事务视图来说，除了自己的更新总是可见以外，有三种情况:

* 版本未提交，不可见；

* 版本已提交，但是是在视图创建后提交的，不可见；

* 版本已提交，而且是在视图创建前提交的，可见。

现在，我们用这个规则来判断图4中的查询结果，事务A的查询语句的视图数组是在事务A启动的时候生成的，这时候：

1) (1,3)还没提交，属于情况1，不可见；

2) (1,2)虽然提交了，但是是在视图数组创建之后提交的，属于情况2，不可见；

3) (1,1)是在视图数组创建之前提交的，可见。

**当前读**

细心的同学可能有疑问了：事务B的update语句，如果按照一致性读，好像结果不对哦？

你看图5中，事务B的视图数组是先生成的，之后事务C才提交，不是应该看不见(1,2)吗，怎么能算出(1,3)来？

![图5 事务B更新逻辑图](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_mvcc9.webp)

是的，如果事务B在更新之前查询一次数据，这个查询返回的k的值确实是1。

但是，当它要去更新数据的时候，就不能再在历史版本上更新了，否则事务C的更新就丢失了。因此，事务B此时的set k=k+1是在（1,2）的基础上进行的操作。

所以，这里就用到了这样一条规则: 更新数据都是先读后写的，而这个读，只能读当前的值，称为“当前读”（current read）。

因此，在更新的时候，当前读拿到的数据是(1,2)，更新后生成了新版本的数据(1,3)，这个新版本的row trx_id是101。

所以，在执行事务B查询语句的时候，一看自己的版本号是101，最新数据的版本号也是101，是自己的更新，可以直接使用，所以查询得到的k的值是3。

这里我们提到了一个概念，叫作当前读。其实，除了update语句外，select语句如果加锁，也是当前读。

>ps: 实际上，是在事务B执行修改操作的时候，发现事务C并不在活跃的事务列表中了，因此可以判断事务C已经提交


## 4. RC、RR级别下的InnoDB快照读有什么不同？
正是Read View生成时机的不同，从而造成RC,RR级别下快照读的结果的不同：


1） 在RR级别下的某个事务的对某条记录的第一次快照读会创建一个快照及Read View, 将当前系统活跃的其他事务记录起来，此后在调用快照读的时候，还是使用的是同一个Read View，所以只要当前事务在其他事务提交更新之前使用过快照读，那么之后的快照读使用的都是同一个Read View，所以对之后的修改不可见；

也即RR级别下，快照读生成Read View时，Read View会记录此时所有其他活动事务的快照，这些事务的修改对于当前事务都是不可见的。而早于Read View创建的事务所做的修改均是可见

2） 而在RC级别下的，事务中，每次快照读都会新生成一个快照和Read View, 这就是我们在RC级别下的事务中可以看到别的事务提交的更新的原因


总之在RC隔离级别下，是每个快照读都会生成并获取最新的Read View；而在RR隔离级别下，则是同一个事务中的第一个快照读才会创建Read View, 之后的快照读获取的都是同一个Read View。



<br />
<br />
**[参看]**:

1. [聊聊 MySQL的 MVCC](https://zhuanlan.zhihu.com/p/475415473)

2. [浅谈MySQL中的MVCC](https://zhuanlan.zhihu.com/p/482398377)

3. [MySQL MVCC底层原理详解](https://zhuanlan.zhihu.com/p/442643107)

<br />
<br />
<br />
