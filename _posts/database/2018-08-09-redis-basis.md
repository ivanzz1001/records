---
layout: post
title: redis基础教程
tags:
- database
categories: database
description: redis基础教程
---


Redis是一个开源的内存数据库，可作为数据库、缓存、消息代理来使用。它支持如下数据类型：

* string

* hash

* list

* set

* sorted set

并提供如基于区间的查询、bitmap、基数统计(hyperloglog)、基于半径的地理位置查询等功能。Redis内置支持数据复制、lua脚本、LRU（Least recently used）淘汰算法、事务(transaction)以及不同级别的数据持久化功能。

<!-- more -->


## 1. Redis简介
REmote DIctionary Server(Redis) 是一个由Salvatore Sanfilippo写的key-value存储系统。Redis是一个开源的使用ANSI C语言编写、遵守BSD协议、支持网络、可基于内存亦可持久化的日志型、Key-Value数据库，并提供多种语言的API。它通常被称为数据结构服务器，因为其支持的值类型可以是string、hash、list、set、sorted set等。

**Redis优势**

* 性能极高： redis读的速度是110000次/s，写的速度是81000次/s

* 丰富的数据类型

* 原子操作： Redis的所有操作都是原子性的，意思就是要么成功执行要么失败完全不执行。单个操作是原子性的，多个操作也支持事务。

* 丰富的特性： Redis还支持publish/subscribe、notification、key过期等特性

## 2. Redis配置
Redis的配置文件位于Redis安装目录下，文件名为redis.conf。你可以通过```CONFIG```命令查看或设置配置项。例如：
{% highlight string %}
127.0.0.1:6379> CONFIG GET loglevel
1) "loglevel"
2) "notice"

127.0.0.1:6379> CONFIG GET *
  1) "dbfilename"
  2) "dump.rdb"
  3) "requirepass"
  4) ""
  5) "masterauth"
  6) ""
  7) "unixsocket"
  ...
{% endhighlight %}

我们可以通过修改redis.conf配置文件，然后重启redis-server来使配置生效。也可以通过如下方式直接修改：
{% highlight string %}
127.0.0.1:6379> CONFIG SET loglevel "notice"
OK
127.0.0.1:6379> CONFIG GET loglevel
1) "loglevel"
2) "notice"
{% endhighlight %}


## 3. Redis命令
这里我们将Redis命令归结为如下几类： 1) Redis数据库管理命令； 2） Redis键(key)操作命令； 3) Redis各数据类型操作命令； 4) Redis HyperLogLog操作命令； 5) Redis发布订阅； 6） Redis事务。 下面我们分这些类型分别进行讲解。

### 3.1 Redis客户端连接相关命令

**1) Redis客户端连接命令**

通过redis-cli可以远程连接redis服务器。例如：
{% highlight string %}
# ./redis-cli -h 127.0.0.1 -p 6379 -a "mypass"
127.0.0.1:6379> ping
PONG
{% endhighlight %}

**2) 验证密码是否正确**
{% highlight string %}
127.0.0.1:6379> auth "password"
OK
{% endhighlight %}

**3) 打印字符串**
{% highlight string %}
127.0.0.1:6379> echo "hello,world"
"hello,world"
{% endhighlight %}

**4)  切换数据库**
{% highlight string %}
127.0.0.1:6379> select 2
OK
{% endhighlight %}

**5) 当前当前关闭连接**
{% highlight string %}
127.0.0.1:6379[2]> quit
{% endhighlight %}

### 3.2 Redis服务器命令
Redis服务器命令主要用于管理Redis服务。例如，我们可以通过如下方式获取Redis服务器的统计信息：
{% highlight string %}
127.0.0.1:6379> info
# Server
redis_version:3.2.11
redis_git_sha1:00000000
redis_git_dirty:0
redis_build_id:97eb487d6f26f02a
redis_mode:standalone
os:Linux 3.10.0-514.el7.x86_64 x86_64
arch_bits:64
multiplexing_api:epoll
gcc_version:4.8.5
process_id:16518
run_id:3025e67d25d24431296d22e460a6f17802381621
....
{% endhighlight %}
关于Redis服务器相关命令还有很多，请参看如下列表：

![db-redis-command](https://ivanzz1001.github.io/records/assets/img/db/db_redis_server_cmd.jpg)


### 3.3 Redis键(key)操作命令
Redis键命令用于管理redis键。主要有如下：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|  1     |DEL key: 该命令用于在key存在时删除key                                                |
|  2     |DUMP key: 序列化给定key，并返回被序列化的值                                          |
|  3     |EXISTS key: 检查给定key是否存在                                                      |
|  4     |EXPIRE key seconds: 为给定key设置过期时间                                            |
|  5     |EXPIREAT key timestamp: EXPIREAT的作用域EXPIRE类似，都用于为key设置过期时间。不同在于EXPIREAT命令接受的时间参数是unix时间错(unix timestamp)|
|  6     |PEXPIRE key milliseconds: 设置key的过期时间，以毫秒计                                |
|  7     |PEXPIREAT key milliseconds-timestamp: 设置key的过期时间戳(unix timestamp)以毫秒计算  |
|  8     |KEYS pattern: 查找所有符合给定模式(pattern)的key                                     |
|  9     |MOVE key db: 将当前数据库的key移动到给定的数据库db当中                               |
|  10    |PERSIST key: 移除key的过期时间，key将保持永久                                        |
|  11    |PTTL key: 以毫秒为单位返回key的剩余过期时间                                          |
|  12    |TTL key: 以秒为单位，返回给定key的剩余生存时间(TTL: time to live)                    |
|  13    |RANDOMKEY: 从当前数据库中随机返回一个key                                             |
|  14    |RENAME key newkey: 修改key的名称                                                     |
|  15    |RENAMENX key newkey: 仅当newkey不存在时，将key改为newkey                             |
|  16    |TYPE key: 返回key所存储的值的类型                                                    |     

### 3.4 Redis字符串(string)操作命令
Redis字符串(string)数据类型的相关命令用于管理redis字符串值。主要有如下：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|  1     |SET key value: 设置指定key的值                                                       |
|  2     |GET key: 获取指定key的值                                                             |
|  3     |GETRANGE key start end: 返回key中字符串值的子字符                                    |
|  4     |GETSET key value: 将给定key的值设为value，并返回key的旧值(old value)                 |
|  5     |GETBIT key offset: 对key所存储的字符串值，获取指定偏移量上的位(bit)                  |
|  6     |MGET key1[key2...]: 获取一个或多个给定key的值                                        |
|  7     |SETBIT key offset value: 对key所存储的字符串值，设置或清除指定偏移量上的位(bit)      |
|  8     |SETEX key seconds value: 将值value关联到key，并将key的过期时间设为seconds(以秒为单位)|
|  9     |SETNX key value: 只有在key不存在时设置key的值                                        |
|  10    |SETRANGE key offset value: 用value参数覆写给定key所存储的字符串值，从偏移量offset开始|
|  11    |STRLEN key: 返回key所存储的字符串值的长度                                            |
|  12    |MSET key1 value1 [key2 value2 ...]: 同时设置一个或多个key-value对                    |
|  13    |MSETNX key1 value1 [key2 value2 ...]: 同时设置一个或多个key-value对，当且仅当所有给定key不存在|
|  14    |PSETEX key milliseconds value: 这个命令和SETEX命令相似，但它以毫秒为单位设置key的生存时间，而不是像SETEX那样以秒为单位|
|  15    |INCR key: 将key中存储的数字增1                                                       |
|  16    |INCRBY key increment: 将key所存储的值加上给定的增量值(increment)                     |
|  17    |INCRBYFLOAT key increment: 将key所存储的值加上给定的浮点增量值(increment)            |
|  18    |DECR key: 将key中存储的数字值减1                                                     |
|  19    |DECRBY key decrement: 将key中存储的数字值减去给定的值(decrement)                     |
|  20    |APPEND key value: 如果key已经存在是一个字符串，APPEND命令将指定的value追加到该key原来值(value)的末尾|


<br />
<br />

**[参看]**

1. [Redis 教程](http://www.runoob.com/redis/redis-tutorial.html)

2. [redis官网](https://redis.io/)

3. [redis在线测试](http://try.redis.io/)

<br />
<br />
<br />

