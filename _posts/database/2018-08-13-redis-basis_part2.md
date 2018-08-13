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

Redis事务可以一次执行多个命令，并且带有以下两个重要的保证：

* 一个事务中的所有命令都会被序列表，然后顺序的执行。不会发生在执行过程中被其他client发送的命令所中断的情况，这就保证了所有的命令都被隔离在一个操作中执行。

* Redis事务是原子性的，即要么所有的命令都被执行，要么都不被执行。```EXEC```命令触发事务中的所有命令被执行，因此假如连接到Redis服务器的客户端在调用```MULTI```命令之前断开，则所有的操作均不会被执行；假如```EXEC```命令被调用的话，则所有命令都被执行。






<br />
<br />

**[参看]**

1. [Redis 教程](http://www.runoob.com/redis/redis-tutorial.html)

2. [redis官网](https://redis.io/)

3. [redis在线测试](http://try.redis.io/)

<br />
<br />
<br />

