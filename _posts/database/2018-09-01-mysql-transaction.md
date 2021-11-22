---
layout: post
title: 数据库事务的隔离级别
tags:
- database
categories: database
description: 数据库事务的隔离级别
---


本文我们主要讲述一下数据库事务的隔离级别。


<!-- more -->


## 1. 数据库事务的隔离级别
数据库事务的隔离级别有4个，由低到高依次为```Read Uncommited```、```Read Committed```、```Repeatable Read```和```Serializable```，这四个级别可以逐个解决脏读、不可重复读、幻读这几类问题。

>注：脏读又称无效数据的读出，是指在数据库访问中，事务T1将某一值修改，然后事务T2读取该值，此后T1因为某种原因撤销对该值的修改，这就导致了T2所读取到的数据是无效的，值得注意的是，脏读一般是针对于update操作的

![db-transaction-isolation](https://ivanzz1001.github.io/records/assets/img/db/db_transaction_isolation.jpg)

注意： 我们讨论隔离级别的场景，主要是在多个事务并发的情况下，因此接下来的讲解都围绕事务并发。

1) **Read uncommitted**

公司发工资了，领导把```5000```元打到singo的账号上，但是该事务并未提交，而singo正好去查看账户，发现工资已经到账，是5000元整，非常高兴。可是不幸的是，领导发现发给singo的工资金额不对，是```2000```元，于是迅速回滚了事务，修改金额后，将事务提交，最后singo实际的工资只有2000元，singo空欢喜一场。

![db-read-uncommitted](https://ivanzz1001.github.io/records/assets/img/db/db_read_uncommitted.jpg)

出现上述情况，即我们所说的```脏读```，两个并发的事务：
<pre>
事务A： 领导给singo发工资

事务B： singo查询工资账户
</pre>
```事务B```读取了```事务A```尚未提交的数据。当隔离级别设置为```Read uncommitted```时，就可能出现脏读，请看下一个隔离级别。


2) **Read committed**

singo拿着工资卡去消费，系统读取到卡里确实有```2000```元，而此时他的老婆也正好在网上转账，把singo的工资卡的```2000```元转到另一账户，并在singo之前提交了事务。当singo扣款时，系统检查到singo的工资卡已经没有钱，扣款失败，singo十分纳闷，明明卡里有钱，为何...

出现上述情况，即我们所说的不可重复读，两个并发事务：
<pre>
事务A： singo消费

事务B： singo老婆网上转账
</pre>
```事务A```先读取了数据，```事务B```紧接着更新了数据，并提交了事务，而```事务A```再次读取该数据时，数据已经发生了改变。

当隔离级别设置为```Read committed```时，避免了脏读，但是可能会造成不可重复读。

大多数数据库的默认级别就是```Read committed```，比如sql server， oracle。如何解决不可重复读这一问题，请看下一个隔离级别。


3) **Repeatable read**

当隔离级别被设置为```Repeatable Read```时，可以避免不可重复读： 当singo拿着工资卡去消费时，一旦系统开始读取工资卡信息（即事务开始），singo的老婆就不可能对该记录进行修改，也就是singo的老婆不能在此时进行转账。

虽然```Repeatable Read```避免了不可重复读，但还有可能出现幻读。

singo的老婆在银行部门工作，她时常通过银行内部系统查看singo的消费记录。有一天，她正在查询到singo当月信用卡的总消费金额
<pre>
(select sum(account) from transaction where month=本月)
</pre>
为80元，而singo此时正好在外边胡吃海喝在收银台买单，消费1000元，即新增一条```1000```元的消费记录（insert transaction...)，并提交了事务。随后singo的老婆将singo当月信用卡消费的明细打印到```A4纸```上，却发现消费金额为```1080```元，singo的老婆发现很诧异，以为出现了幻觉，幻读就这样产生了。

注： MySQL的默认隔离级别就是Repeatable Read。

4) **Serializable**

Serializable是最高的事务隔离级别，同时代价也花费最高，性能很低，一般很少使用。在该级别下，事务顺序执行。不仅避免了脏读、不可重复读、还避免了幻读。









<br />
<br />
**[参看]**:




<br />
<br />
<br />

