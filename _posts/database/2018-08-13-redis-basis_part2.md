---
layout: post
title: redis基础教程(2)
tags:
- database
categories: database
description: redis基础教程(2)
---

本章主要讲述以下Redis发布订阅、Redis事务、以及Redis数据备份与恢复。


<!-- more -->

## 1. Redis发布订阅

Redis发布订阅(pub/sub)是一种消息通信模式： 发布者(pub)发送消息，订阅者(sub)接收消息。Redis客户端可以订阅任意数量的频道。下图展示了频道channel1，以及订阅这个频道的3个客户端client2、client5、client1之间的关系:

![db-redis-pubsub](https://ivanzz1001.github.io/records/assets/img/db/db_redis_pubsub1.png)

当有新消息通过```PUBLISH```命令发送给channel1时，这个消息就会被发送给订阅它的三个客户端：

![db-redis-pubsub](https://ivanzz1001.github.io/records/assets/img/db/db_redis_pubsub2.png)

下面我们演示如何使用Redis发布订阅。首先通过如下的方式创建了订阅频道名为```redisChat```:
{% highlight string %}
localhost:6379> subscribe redisChat
Reading messages... (press Ctrl-C to quit)
1) "subscribe"
2) "redisChat"
3) (integer) 1
{% endhighlight %}
现在，我们先重新开启一个redis客户端，然后在同一个频道```redisChat```发布两次消息，订阅者就能接收到消息：
{% highlight string %}
redis 127.0.0.1:6379> PUBLISH redisChat "Redis is a great caching technique"

(integer) 1

redis 127.0.0.1:6379> PUBLISH redisChat "Learn redis by runoob.com"

(integer) 1

# 订阅者的客户端会显示如下消息
1) "message"
2) "redisChat"
3) "Redis is a great caching technique"
1) "message"
2) "redisChat"
3) "Learn redis by runoob.com"
{% endhighlight %}


**1) Redis发布订阅命令**

下面我们列出Redis发布订阅涉及到的相关命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |PSUBSCRIBE pattern [pattern ...]: 订阅一个或多个符合给定模式的频道                   |
|   2    |PUBSUB subcommand [argument [argument ...]]: 查看订阅与发布系统状态                  |
|   3    |PUBLISH channel message: 将信息发布到指定频道                                        |
|   4    |PUNSUBSCRIBE [pattern [pattern ...]]: 退订所有给定模式的频道                         |
|   5    |SUBSCRIBE channel [channel ...]: 订阅给定的一个或多个频道信息                        |
|   6    |UNSUBSCRIBE [channel [channel ...]]: 退订指定的频道                                  |


## 2. Redis事务

```MULTI```、```EXEC```、```DISCARD```和```WATCH```是Redis事务的基础。Redis事务可以一次执行多个命令，并且带有以下两个重要的保证：


* 事务是一个单独的隔离操作：一个事务中的所有命令都会被序列化然后顺序的执行。在事务的执行过程中，不会被其他客户端发送过来的命令所中断。

* 事务中的所有命令要么全部执行，要么都不执行，因此Redis事务是原子性的。如果客户端在使用```MULTI```开启了一个事务之后，却因为断线而没有成功执行```EXEC```，那么事务中的所有命令都不会被执行；如果成功开启事务之后执行```EXEC```，那么事务中的所有命令都会被执行。当使用AOF做持久化的时候，Redis确保使用一个write系统调用来将事务写到硬盘。然而，在极端情况下假如Redis服务器崩溃或者由管理员将redis server杀死，则有可能造成只有部分命令被写入到磁盘中。在Redis启动的时候如果侦测到这种情况，则会报告相应的错误并直接退出。使用```redis-check-aof```工具可以对aof文件进行修复并将该不完整的事务移除，然后Redis Server就可以正常的启动了。

### 2.1 Redis事务示例
一个Redis事务由```MULTI```命令所开启。该命令总是会返回```OK```响应，此时用户可以输入多个命令，Redis服务器并不会马上执行这些命令，而是将这些命令按顺序缓存起来，而一旦```EXEC```命令被调用，则所有缓存起来的命令都会被执行。如果想取消事务，可以调用```DISCARD```命令，此时则会刷新缓存起来的命令，并退出事务。

如下示例会在一个事务中自动的递增```foo```和```bar```:
{% highlight string %}
127.0.0.1:6379> MULTI
OK
127.0.0.1:6379> INCR foo
QUEUED
127.0.0.1:6379> INCR bar
QUEUED
127.0.0.1:6379> EXEC
1) (integer) 1
2) (integer) 1
127.0.0.1:6379>
{% endhighlight %}
从上面的会话过程我们看到，```EXEC```命令执行完后会返回一组响应，其中的每一个响应信息都对应事务中的每一条指令。

### 2.2 事务中的错误
在一个事务执行过程中可能会遇到两种类型的命令错误：

* 命令本身有错误，导致在送入缓存队列时出错，因此我们可以在执行EXEC命令之前就获得错误。

* 命令在执行```EXEC```之后产生错误。例如对一个string类型key做list相关操作

通常客户端可以很容易的感知第一种类型的错误，一般假如命令没有错误会直接返回```QUEUED```响应信息，否则会返回对应的错误。在大多数情况下，若出现第一种错误，客户端一般会调用```DISCARD```以放弃本次事务。
<pre>
注： 在Redis2.6.5之后，如果在将命令缓存进队列的时候出现错误，则Redis会记住该错误，对后序客户端发送的命令都会返回错误；
对于Redis2.6.5之前的版本，Redis服务器则会执行所缓存的正确的命令
</pre>

而对于第二种执行```EXEC```时产生的错误，Redis则会继续往下执行。例如：
{% highlight string %}
127.0.0.1:6379> MULTI
OK
127.0.0.1:6379> set a 3
QUEUED
127.0.0.1:6379> lpop a
QUEUED
127.0.0.1:6379> exec
1) OK
2) (error) WRONGTYPE Operation against a key holding the wrong kind of value
127.0.0.1:6379> get a
"3"
{% endhighlight %}

### 2.3 Redis事务不支持回滚
假如你有关系型数据库使用的背景，你会注意到Redis在```EXEC```执行过程中假如遇到错误，仍然会继续执行，而不是进行回滚。这看起来可能有些奇怪。然而，Redis的这种做法有如下优点：

* Redis只可能在命令出现语法错误，或者对错误数据类型的key进行操作： 这种只可能会发生在程序代码有错的情况下，并且通常发生在开发环境中，很容易被发现，而在实际生产环境中一般不会发生这样的错误。

* Redis内部实现可以更简洁与高效，因为其并不需要有roll back相应的功能。

### 2.4 丢弃缓存的事务命令
可以通过```DISCARD```命令丢弃已经缓存的事务命令，并退出整个事务。例如：
{% highlight string %}
127.0.0.1:6379> set foo 1
OK
127.0.0.1:6379> MULTI
OK
127.0.0.1:6379> INCR foo
QUEUED
127.0.0.1:6379> DISCARD
OK
127.0.0.1:6379> get foo
"1"
{% endhighlight %}

### 2.5 使用check-and-set操作实现乐观锁
```WATCH```命令可以为Redis事务提供check-and-set(CAS)行为。被 WATCH 的键会被监视，并会发觉这些键是否被改动过了。如果有至少一个被监视的键在EXEC执行之前被修改了，那么整个事务都会被取消，EXEC返回nil-reply来表示事务已经失败。举个例子，假设我们需要原子性的为某个值进行增1（假设Redis不提供INCR功能），则我们可能会如下操作：
<pre>
val = GET mykey
val = val + 1
SET mykey $val
</pre>
在一段时间内，假如我们只有一个客户端执行上面的操作，则也许并不会出现什么问题。假如有多个客户端在相同的时间内执行上面的操作的话，则会产生竞争。举个例子， 如果客户端 A 和 B 都读取了键原来的值， 比如 10 ， 那么两个客户端都会将键的值设为 11 ， 但正确的结果应该是 12 才对。在有了```WATCH```机制之后，我们就可以轻松的解决此类问题。
<pre>
WATCH mykey
val = GET mykey
val = val + 1
MULTI
SET mykey $val
EXEC
</pre>
使用上面的代码，假如有竞争条件发生： 在我们执行WATCH到EXEC这段时间假如有另外一个客户端对val值进行了修改，则整个事务将会执行失败。这种情况下，我们一般可以对上面的代码进行重试即可。上面这种形式的锁称作```乐观锁(optimistic locking)```，是一种功能强大的锁机制。并且因为在大多数情况下，不同的客户端会访问不同的键，碰撞的情况一般都很少，所以通常并不需要进行重试。


### 2.6 了解WATCH
```WATCH```的真正含义是什么呢？ 实际上是让```EXEC```可以有条件的执行：让Redis在被WATCH的key没有发生改变的情况下执行事务（注意：在同一事务中进行修改是被允许的)。否则，事务将不会进入执行（注意：假如你WATCH一个voltile key，该key在你WATCH期间过期，则EXEC仍会执行)。

WATCH命令可以调用多次。对键的监视从 WATCH 执行之后开始生效， 直到调用 EXEC 为止。用户还可以在单个 WATCH 命令中监视任意多个键， 就像这样：
{% highlight string %}
redis> WATCH key1 key2 key3
OK
{% endhighlight %}
当```EXEC```执行后，这些键会被取消监视，不管事务是否执行成功。在一个客户端连接断开之后，也会取消所有的监视。

也可以通过使用无参数的```UNWATCH```命令来取消所有的监视。对于一些需要改动多个键的事务， 有时候程序需要同时对多个键进行加锁， 然后检查这些键的当前值是否符合程序的要求。 当值达不到要求时， 就可以使用 UNWATCH 命令来取消目前对键的监视， 中途放弃这个事务， 并等待事务的下次尝试。

我们可以通过```WATCH```来实现```ZPOP```。举个例子， 以下代码实现了原创的 ZPOP 命令， 它可以原子地弹出有序集合中分值（score）最小的元素：
<pre>
WATCH zset
element = ZRANGE zset 0 0
MULTI
ZREM zset element
EXEC
</pre> 
程序只要重复执行这段代码， 直到 EXEC 的返回值不是nil-reply回复即可。

### 2.7 Redis脚本和事务
从定义上来说，Redis脚本本身就是一种事务。所以，任何在Redis事务中可以完成的事，都可以通过脚本来完成。并且在通常情况下脚本会更简洁与高效。

这看起来Redis事务与脚本有些重复，那是因为Redis脚本是在Redis2.6版本之后才引入的，而事务则从一开始就存在。不过我们并不打算在短时间内就移除事务功能， 因为事务提供了一种即使不使用脚本， 也可以避免竞争条件的方法， 而且事务本身的实现并不复杂。

不过也许在不远的将来，可能所有用户都会只使用脚本来实现事务也说不定。 如果真的发生这种情况的话， 那么我们将废弃并最终移除事务功能。



## 3. Redis脚本
Redis脚本使用lua解释器来执行脚本。Redis2.6版本通过内嵌支持lua环境。执行脚本的常用命令为```EVAL```。EVAL命令的基本语法如下：
{% highlight string %}
redis 127.0.0.1:6379> EVAL script numkeys key [key ...] arg [arg ...]
{% endhighlight %} 

例如，下面的示例演示了redis脚本的工作过程：
{% highlight string %}
127.0.0.1:6379> EVAL "return {KEYS[1],KEYS[2],ARGV[1],ARGV[2]}" 2 key1 key2 first second
1) "key1"
2) "key2"
3) "first"
4) "second"
{% endhighlight %}

下面我们列出Redis脚本操作的一些命令：


| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |EVAL script numkeys key [key ...] arg [arg ...]: 执行lua脚本                         |
|   2    |EVALSHA sha1 numkeys key [key ...] arg [arg ...]: 执行lua脚本                        |
|   3    |SCRIPT EXISTS script [script...]: 查看指定的脚本是否已经被保存在缓存当中             |
|   4    |SCRIPT FLUSH: 从脚本缓存中移除所有脚本                                               |
|   5    |SCRIPT KILL: 杀死当前正在运行的lua脚本                                               |
|   6    |SCRIPT LOAD script: 将脚本script添加到脚本缓存中，但并不立即执行这个脚本             |


## 4.  Redis数据备份与恢复

**1） Redis数据备份**

**SAVE**命令用于创建当前数据库的备份。其基本使用方法如下：
{% highlight string %}
redis 127.0.0.1:6379> SAVE 
OK
{% endhighlight string %}
该命令将在redis安装目录中创建dump.rdb文件。
<pre>
默认在配置文件中，通常有如下两个配置：
dbfilename dump.rdb
dir ./
</pre>


**2） 数据恢复**

如果需要恢复数据，只需要将相应文件名的文件放到正确的目录即可。获取redis目录可以使用```CONFIG```命令，如下所示：
{% highlight string %}
127.0.0.1:6379> CONFIG get dir
1) "dir"
2) "/usr/local/redis-3.2.11/bin"
127.0.0.1:6379> CONFIG GET dbfilename
1) "dbfilename"
2) "dump.rdb"
{% endhighlight %}

**3) 后台保存数据**

创建Redis备份文件也可以使用命令```BGSAVE```，该命令在后台执行。例如：
{% highlight string %}
127.0.0.1:6379> BGSAVE

Background saving started
{% endhighlight %}

## 5. Redis安全
我们可以通过redis配置文件设置密码参数，这样客户端连接到redis服务就需要密码验证，这样可以让你的redis服务更安全。我们可以通过如下命令查看是否设置了密码验证：
{% highlight string %}
127.0.0.1:6379> CONFIG get requirepass
1) "requirepass"
2) ""
{% endhighlight %}
默认情况下，```requirepass```参数为空，这就意味着你无需通过密码验证就可以连接到redis服务。你可以通过如下的命令来修改该参数：
{% highlight string %}
127.0.0.1:6379> CONFIG SET requirepass "runoob"
OK
127.0.0.1:6379> CONFIG get requirepass
(error) NOAUTH Authentication required.
127.0.0.1:6379> auth runoob
OK
127.0.0.1:6379> config get requirepass
1) "requirepass"
2) "runoob"
{% endhighlight %}

设置了密码之后，我们重新连接到redis服务器，就需要密码验证，否则无法执行命令。例如：
{% highlight string %}
[root@bogon ~]# /usr/local/redis-3.2.11/bin/redis-cli 
127.0.0.1:6379> keys *
(error) NOAUTH Authentication required.
127.0.0.1:6379> auth runoob
OK
127.0.0.1:6379> keys *
1) "foo"
127.0.0.1:6379> get foo
"1"
{% endhighlight %}

## 6. Redis性能测试

**1） 基本语法**
redis的性能测试是通过同时执行多个命令实现的。redis性能测试的基本命令如下：
<pre>
# redis-benchmark [option] [option value]
</pre>

例如，以下示例同时执行10000个请求来检测性能：
<pre>
#redis-benchmark -n 10000  -q

PING_INLINE: 141043.72 requests per second
PING_BULK: 142857.14 requests per second
SET: 141442.72 requests per second
GET: 145348.83 requests per second
INCR: 137362.64 requests per second
LPUSH: 145348.83 requests per second
LPOP: 146198.83 requests per second
SADD: 146198.83 requests per second
SPOP: 149253.73 requests per second
LPUSH (needed to benchmark LRANGE): 148588.42 requests per second
LRANGE_100 (first 100 elements): 58411.21 requests per second
LRANGE_300 (first 300 elements): 21195.42 requests per second
LRANGE_500 (first 450 elements): 14539.11 requests per second
LRANGE_600 (first 600 elements): 10504.20 requests per second
MSET (10 keys): 93283.58 requests per second
</pre>

**2) 测试工具参数**

Redis性能测试工具可选参数如下所示：

|  序号 |  选项  |            描述                          |     默认值   | 
|:-----:|:------:|:-----------------------------------------|:-------------|
|   1   |   -h   |指定服务器主机名                          |127.0.0.1     |
|   2   |   -p   |指定服务器端口                            |6379          |
|   3   |   -s   |指定服务器socket                          |              |
|   4   |   -c   |指定并发连接数                            |50            |
|   5   |   -n   |指定请求数                                |10000         |
|   6   |   -d   |以字节的形式指定SET/GET值的数据大小       |2             |
|   7   |   -k   |1=keep alive  0=reconnect                 |1             |
|   8   |   -r   |SET/GET/INCR使用随机key，SADD使用随机值   |              |
|   9   |   -P   |通过管道传输<numreq>请求                  |1             |
|   10  |   -q   |强制退出redis。仅显示query/sec值          |              |
|   11  |  --csv |以CSV格式输出                             |              |
|   12  |   -l   |生成循环，永久执行测试                    |              |
|   13  |   -t   |仅运行以逗号分割的测试命令列表            |              |
|   14  |   -I   |Idle模式。仅打开N个idle连接并等待         |              |

注： 上面12选项是```l```(小写L)

## 7. Redis客户端连接

Redis通过监听一个TCP端口或者Unix socket的方式来接收来自客户端的连接，当一个连接建立后，Redis内部会进行以下操作：

* 首先，客户端socket会被设置为非阻塞模式，因为Redis在网络事件处理上采用的是非阻塞多路复用模型

* 然后，为这个socket设置```TCP_NODELAY```属性，禁用Nagle算法。

* 然后创建一个可读的文件事件用于监听这个客户端socket的数据发送

**1） 最大连接数**

在 Redis2.4 中，最大连接数是被直接硬编码在代码里面的，而在2.6版本中这个值变成可配置的。```maxclients```的默认值是 10000，你也可以在 redis.conf 中对这个值进行修改。
<pre>
config get maxclients

1) "maxclients"
2) "10000"
</pre>

我们也可以在服务启动时设置最大连接数，例如我们设置最大连接数为为100000:
<pre>
# redis-server --maxclients 100000
</pre>

**2) 客户端命令**

|  序号 |      命令      |               描述                            |
|:-----:|:--------------:|:----------------------------------------------|
|   1   |CLIENT LIST     |返回连接到 redis 服务的客户端列表              |
|   2   |CLIENT SETNAME  |设置当前连接名称                               |
|   3   |CLIENT GETNAME  |获取通过 CLIENT SETNAME 命令设置的服务名称     |
|   4   |CLIENT PAUSE    |挂起客户端连接，指定挂起的时间以毫秒计         |
|   5   |CLIENT KILL     |关闭客户端连接                                 |





<br />
<br />

**[参看]**

1. [Redis 教程](http://www.runoob.com/redis/redis-tutorial.html)

2. [redis官网](https://redis.io/)

3. [redis在线测试](http://try.redis.io/)

4. [redis的持久化方式RDB和AOF的区别](https://blog.csdn.net/m0_38110132/article/details/76906422)

<br />
<br />
<br />

