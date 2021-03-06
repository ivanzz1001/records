---
layout: post
title: 面试必备之乐观锁与悲观锁(转）
tags:
- 分布式系统
categories: distribute-systems
description: 面试必备之乐观锁与悲观锁
---

本章我们介绍一下系统设计当中经常遇到的乐观锁(Optimistic Lock)与悲观锁(Pessimistic Lock)。

<!-- more -->


## 1. 乐观锁与悲观锁

### 1.1 何谓乐观锁与悲观锁
乐观锁对应于生活中乐观的人总是想着事情往好的方向发展，悲观锁对应于生活中悲观的人总是想着事情往坏的方向发展。这两种人各有优缺点，不能不以场景而定说一种人好于另外一种人。

1) **悲观锁**

总是假设最坏的情况，每次去拿数据的时候都认为别人会修改，所以每次在拿数据的时候都会上锁，这样别人想拿这个数据就会阻塞直到它拿到锁（**共享资源每次只给一个线程使用，其他线程阻塞，用完后再把资源转让给其他线程**）。传统的关系型数据库里边就用到了很多这种锁机制，比如行锁、表锁等，读锁、写锁等，都是在做操作之前先上锁。Java中```synchronized```和```ReentrantLock```等独占锁就是悲观锁思想的实现。

2) **乐观锁**

总是假设最好的情况，每次去拿数据的时候都认为别人不会修改，所以不会上锁，但是在更新的时候会判断一下在此期间别人有没有去更新这个数据，可以使用版本号机制和CAS算法实现。**乐观锁适用于多读的应用类型，这样可以提高吞吐量**，像数据库提供的类似于write_condition机制，其实都是提供的乐观锁。在Java中```java.util.concurrent.atomic```包下面的原子变量类就是使用了乐观锁的一种实现方式CAS实现的。

### 1.2 两种锁的使用场景
从上面两种锁的介绍，我们知道两种锁各有优缺点，不可认为一种好于另一种。像**乐观锁适应于写比较少的情况下（多读场景）**，即冲突真的很少发生的时候，这样可以省去了锁的开销，加大了系统的整个吞吐量。但如果是多写的情况，一般会经常产生冲突，这就会导致上层应用会不断的进行retry，这样反倒是降低了性能，所以**一般多写的场景下用悲观锁就比较合适**。

### 1.3 乐观锁常见的两种实现方式
乐观锁一般会使用版本号机制或CAS算法实现。

1) **版本号机制**

一般是在数据表中加上一个数据版本号version字段，表示数据被修改的次数。当数据被修改时，version值会加一。当线程A要更新数据值时，在读取数据的同时也会读取version值，在提交更新时，若刚才读取到的version值与当前数据库中的version值相等时才更新，否则重试更新操作，直到更新成功。

举一个简单的例子：
<pre>
假设数据库中账户信息表中有一个version字段，当前值为1；而当前账户余额字段(balance)为$100。

1. 操作员A此时将其读出(version=1)，并从其账户余额中扣除$50 ($100-$50)

2. 在操作员A操作的过程中，操作员B也读入此用户信息(version=1)，并从其账户余额中扣除$20 ($100 - $20)

3. 操作员A完成了修改工作，将数据版本号加一（version=2)，连同账户扣除后余额(balance=$50)，提交至数据库更新，此时由于提交数据版本
   大于数据库记录当前版本，数据被更新，数据库记录version更新为2.

4. 操作员B完成了操作，也将版本号加一（version=2)，然后试图向数据库提交数据(balance=$80)，但此时比对数据库记录版本时发现，操作员
   B提交的数据版本号为2，数据库记录当前版本也为2， 不满足“当前最后更新的version与操作员第一次的版本号相等”的乐观锁策略，因此，操
   作员B的提交被驳回。
</pre>
这样，就避免了操作员B用基于version=1的旧数据修改的结果覆盖操作员A的操作结果的可能。

2) **CAS算法**

即**compare and swap(比较与交换）**，是一种有名的无锁算法。无锁编程，即不使用锁的情况下实现多线程之间的变量同步，也就是在没有线程被阻塞的情况下实现变量的同步，所以也叫非阻塞同步(Non-blocking Synchronization)。```CAS算法```涉及到三个操作数：

* 需要读写的内存值 V

* 进行比较的值 A

* 拟写入的新值 B

当且仅当V的值等于A时，CAS通过原子方式用新值B来更新V的值，否则不会执行任何操作（比较和替换是一个原子操作）。一般情况下是一个**自旋操作**，即**不断的重试**。

### 1.4 乐观锁的缺点
```ABA问题```是乐观锁的一个常见的问题。

1) **ABA问题**

如果一个变量V初次读取的时候是A值，并且在准备赋值的时候检查到它仍然是A值，那我们就能说明它的值没有被其他线程修改过吗？ 很明显不能的，因为在这段时间它的值可能被改为其他值，然后又改回A，那CAS操作就会误认为它从来没有被修改过。这个问题被称为CAS操作的```ABA问题```。

JDK1.5以后的```AtomicStampedReference类```就提供了此种能力，其中的```compareAndSet方法```就是首先检查当前引用是否等于预期引用，并且当前标志是否等于预期标志，如果全部相等，则以原子方式将该引用和该标志的值设置为给定的更新值。

2) **循环时间长开销大**

自旋CAS（也就是不成功就一直循环执行直到成功）如果长时间不成功，会给CPU带来非常大的执行开销。如果JVM能支持处理器提供的pause指令，那么效率会有一定的提升。pause指令有两个作用：第一它可以延迟流水线执行指令(de-pipeline)，使CPU不会消耗过多的执行资源，延迟的时间取决于具体实现的版本，在一些处理器上延迟的时间是零； 第二它可以避免在退出循环的时候因内存顺序冲突(memory order violation)而引起CPU流水线被清空(CPU pipeline flush)，从而提高CPU的执行效率。

3) **只能保证一个共享变量的原子操作**

CAS只对单个共享变量有效，当操作涉及跨多个共享变量时CAS无效。但是从JDK1.5开始，提供了```AtomicReference类```来保证引用对象之间的原子性，你可以把多个变量放在一个对象里来进行CAS操作。所以，我们可以使用锁或者利用AtomicReference类把多个共享变量合并成一个共享变量来操作。

### 1.5 CAS与synchronized的使用场景
简单来说，CAS适用于写比较少的情况（多读场景，冲突一般比较少），synchronized适用于写比较多的情况下（多写场景，冲突一般比较多）。

1. 对于资源竞争比较少（线程冲突叫轻）的情况，使用synchronized同步锁进行线程阻塞和唤醒切换以及用户态内核态之间的切换操作额外浪费消耗CPU资源； 而CAS基于硬件实现，不需要进入内核，不需要切换线程，操作自旋几率较小，因此可以获得更高的性能。

2. 对于资源竞争严重（线程冲突严重）的情况，CAS自旋的概率比较大，从而浪费更多的CPU资源，效率低于synchronized

补充： Java并发编程这个领域中synchronized这个关键字一直都是元老级的角色，很久之前很多人都会称它为“**重量级锁**”。但是，在JavaSE 1.6之后进行了主要包括为了减少获得锁和释放锁带来的性能损耗而引入的**偏向锁**和**轻量级锁**以及其他**各种优化**之后变得在某些情况下并不那么重了。synchronized的底层实现主要依靠```Lock Free```的队列，基本思路是**自旋后阻塞，竞争切换后继续竞争锁，稍微牺牲了公平性，但获得了高吞吐量**。在线程冲突较少的情况下，可以获得和CAS类似的性能；而线程冲突严重的情况下，性能远高于CAS。



<br />
<br />

**[参看]:**

1. [面试必备之乐观锁与悲观锁](https://blog.csdn.net/qq_34337272/article/details/81072874)

2. [mysql事务，select for update，及数据的一致性处理](https://www.cnblogs.com/houweijian/p/5869243.html)

3. [MySQL系列-事务及乐观锁悲观锁](https://blog.csdn.net/ufo___/article/details/80868317)

4. [PAUSE指令在Skylake上引起的性能问题](https://cloud.tencent.com/developer/article/1373203)

<br />
<br />
<br />


