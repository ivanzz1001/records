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


### 3.5 Redis哈希(Hash)操作命令
Redis哈希是一个string类型的field-value映射表，hash特别适合于存储对象。Redis中每个hash可以存储2^32-1个键值对。hash的使用示例如下：
{% highlight string %}
127.0.0.1:6379> HMSET runoobkey name "redis tutorial" description "redis basic commands for caching" likes 20 visitors 23000
OK
127.0.0.1:6379> HGETALL runoobkey
1) "name"
2) "redis tutorial"
3) "description"
4) "redis basic commands for caching"
5) "likes"
6) "20"
7) "visitors"
8) "23000"
{% endhighlight %}
上面的实例中，我们设置了redis的一些描述信息（name、description、likes、visitors）到哈希表```runoobkey```中。

下面我们列出hash操作的一些命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|    1   |HDEL key field1 [field2]: 删除一个或多个哈希表字段                                   |
|    2   |HEXISTS key field: 查看哈希表key中指定的字段是否存在                                 |
|    3   |HGET key field: 获取存储在哈希表key中指定字段的值                                    |
|    4   |HGETALL key: 获取存储在哈希表key中所有的字段和值                                     |
|    5   |HINCBY key field increment: 为哈希表key中指定字段的整数加上增量increment             |
|    6   |HINCBYFLOAT key field increment: 为哈希表key中指定字段的浮点数值加上增量increment    |
|    7   |HKEYS key: 获取所有哈希表中的字段                                                    |
|    8   |HLEN key: 获取哈希表中字段的数量                                                     |
|    9   |HMGET key field1 [field2 ...]: 获取所有给定字段的值                                  |
|   10   |HMSET key field1 value1 [field2 value2 ...]: 同时将多个field-value（键值)对设置到哈希表key中|
|   11   |HSET key field value: 将哈希表key中的字段field的值设置为value                        |
|   12   |HSETNX key field value: 只有在field不存在时，设置哈希表字段的值                      |
|   13   |HVALS key: 获取哈希表key中的所有的值                                                 |
|   14   |HSCAN key cursor [MATCH pattern] [COUNT count]: 迭代哈希表中的键值对                 |

### 3.6 Redis列表(List)操作命令
Redis列表是简单的字符串列表，按照插入顺序排序，你可以添加一个元素到列表的头部（左边）或者尾部（右边）。一个列表最多可以包含2^32-1个元素。List使用示例如下：
{% highlight string %}
127.0.0.1:6379> del runoobkey
(integer) 1
127.0.0.1:6379> LPUSH runoobkey redis
(integer) 1
127.0.0.1:6379> LPUSH runoobkey mysql
(integer) 2
127.0.0.1:6379> LPUSH runoobkey mongodb
(integer) 3
127.0.0.1:6379> LRANGE runoobkey 0 10
1) "mongodb"
2) "mysql"
3) "redis"
{% endhighlight %}

下面我们列出List操作的一些命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |BLPOP key1 [key2] timeout: 移出并获取列表的第一个元素，如果没有元素会阻塞列表直到等待超时或发现可弹出元素为止|
|   2    |BRPOP key2 [key2] timeout: 移出并获取列表的最后一个元素，如果没有元素会阻塞列表直到等待超时或发现可弹出元素为止|
|   3    |BRPOPLPUSH source destination: 从列表中弹出一个值，将弹出的元素插入到另外一个列表中并返回它；如果列表没有元素会阻塞列表直到等待超时或发现可弹出元素为止|
|   4    |LINDEX key index: 通过索引获取列表中元素                                             |
|   5    |LINSERT key BEFORE/AFTER pivot value: 在列表的元素前或后插入元素                     |
|   6    |LLEN key: 获取列表长度                                                               |
|   7    |LPOP key: 移出并获取列表的第一个元素                                                 |
|   8    |LPUSH key value1 [value2 ...]: 将一个或多个值                                        |
|   9    |LPUSHX key value: 将一个值插入到已存在的列表头部                                     |
|   10   |LRANGE key start end: 获取列表指定范围内的元素                                       |
|   11   |LREM key count value: 移除列表元素                                                   |
|   12   |LSET key index value: 通过索引设置元素的值                                           |
|   13   |LTRIM key start end: 对一个列表进行修剪(trim)，就是说让列表只保留指定区间内的元素，不在指定区间之内的元素都将被删除|
|   14   |RPOP key: 移除并获取列表最后一个元素                                                 |
|   15   |RPOPLPUSH source destination: 移除列表的最后一个元素，并将该元素添加到另外一个列表并返回|
|   16   |RPUSH key value1 [value2]: 在列表中添加一个或多个元素                                |
|   17   |RPUSHX key value: 为已存在的列表添加值                                               |


### 3.7 Redis集合(Set)操作命令
Redis的Set时String类型的无序集合。集合成员是唯一的，这就意味着集合中不能出现重复的元素。Redis中集合是通过哈希表实现的，所以添加、删除、查找的时间复杂度是O(1)。一个集合最多可以包含2^32-1个元素。Set使用示例如下：
{% highlight string %}
127.0.0.1:6379> SADD runoobkey redis
(integer) 1
127.0.0.1:6379> SADD runoobkey mongodb
(integer) 1
127.0.0.1:6379> SADD runoobkey mysql
(integer) 1
127.0.0.1:6379> SADD runoobkey mysql
(integer) 0
127.0.0.1:6379> SMEMBERS runoobkey
1) "mysql"
2) "mongodb"
3) "redis"
{% endhighlight %}

下面我们列出Set操作的一些命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |SADD key member1 [member2...]: 向集合添加一个或多个元素                              |
|   2    |SCARD key: 获取集合的成员数                                                          |
|   3    |SDIFF key1 [key2]: 返回给定所有集合的差集                                            |
|   4    |SDIFFSTORE destination key1 [key2]: 返回给定所有集合的差集并存储在destination中      |
|   5    |SINTER key1 [key2]: 返回所有给定集合的交集                                           |
|   6    |SINTERSTORE destination key1 [key2]: 返回所有给定集合的交集并存储在destination中     |
|   7    |SISMEMBER key member: 判断member元素是否是集合key的成员                              |
|   8    |SMEMBERS key: 返回集合中的所有成员                                                   |
|   9    |SMOVE source destination member: 将member元素从source集合移动到destination集合中     |
|   10   |SPOP key: 移除并返回集合中的一个随机元素                                             |
|   11   |SRANDMEMBER key [count]: 返回集合中一个或多个随机数                                  |
|   12   |SREM key member1 [member2]: 移除集合中一个或多个成员                                 |
|   13   |SUNION key1 [key2]: 返回所有给定集合的并集                                           |
|   14   |SUNIONSTORE destination key1 [key2]: 返回所有给定集合的并集存储在destination中       |
|   15   |SSCAN key cursor [MATCH pattern] [COUNT count]: 迭代集合中的元素                     |


### 3.8 Redis有序集合(Sorted Set)操作命令
Redis有序集合和集合一样，也是string类型元素的集合，且不允许重复的成员，不同的是每个元素都会关联一个double类型的分数。Redis正是通过分数来为集合中的成员进行从小到大的排序。
有序集合的成员是唯一的，但分数(score)却可以重复。在redis sorted sets里面当items内容大于64的时候同时使用了hash和skiplist两种设计实现。这也是为了排序和查找性能做的优化。因此： 
<pre>
添加和删除都需要修改skiplist，所以复杂度为O(log(n))。 

但是如果仅仅是查找元素的话可以直接使用hash，其复杂度为O(1) 

其他的range操作复杂度一般为O(log(n))

当然如果是小于64的时候，因为是采用了ziplist的设计，其时间复杂度为O(n)
</pre>


集合是通过哈希表来实现的，所以添加、删除、查找的时间复杂度都是O(1)。一个集合最多可以包含2^32-1个元素。Sorted Set使用示例如下：
{% highlight string %}
127.0.0.1:6379> ZADD runoobkey 1 redis
(integer) 1
127.0.0.1:6379> ZADD runoobkey 2 mongodb
(integer) 1
127.0.0.1:6379> ZADD runoobkey 3 mysql
(integer) 1
127.0.0.1:6379> ZADD runoobkey 3 mysql
(integer) 0
127.0.0.1:6379> ZADD runoobkey 4 mysql
(integer) 0
127.0.0.1:6379> ZRANGE runoobkey 0 10 WITHSCORES
1) "redis"
2) "1"
3) "mongodb"
4) "2"
5) "mysql"
6) "4"
{% endhighlight %}
下面我们列出Set操作的一些命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |ZADD key score1 member1 [score2 member2]: 向有序集合添加一个或多个成员，或者更新已存在的成员的分数|
|   2    |ZCARD key: 获得有序集合的成员数                                                      |
|   3    |ZCOUNT key min max: 计算在有序集合中指定区间分数的成员数                             |
|   4    |ZINCRBY key increment member: 有序集合中指定成员的分数加上增量increment              |
|   5    |ZINTERSTORE destination numkeys key1 [key...]: 计算给定的一个或多个有序集合的交集并存储在新的有序集合中|
|   6    |ZLEXCOUNT key min max: 在有序集合中计算指定字典区间内成员数量                        |
|   7    |ZRANGE key start stop [WITHSCORES]: 通过索引区间返回有序集合指定区间内的成员         |
|   8    |ZRANGEBYLEX key min max [LIMIT offset count]: 通过字典区间返回有序集合的成员         |
|   9    |ZRANGEBYSCORE key min max [WITHSCORES] [LIMIT]: 通过分数返回有序集合指定区间内的成员 |
|   10   |ZRANK key member: 返回有序集合中指定成员的索引                                       |
|   11   |ZREM key member [member ...]: 移除有序集合中一个或多个成员                           |
|   12   |ZREMRANGEBYLEX key min max: 移除有序集合中给定的字典区间的所有成员                   |
|   13   |ZREMRANGEBYRANK key start stop: 移除有序集合中给定排名区间的所有成员                 |
|   14   |ZREMRANGEBYSCORE key min max: 移除有序集合中给定的分数区间的所有成员                 |
|   15   |ZREVRANGE key start stop [WITHSCORES]: 返回有序集中指定区间内的成员，通过索引，分数从高到底|
|   16   |ZREVRANGEBYSCORE key max min [WITHSCORES]: 返回有序集中指定分数区间内的成员，分数从高到低排序|
|   17   |ZREVRANK key member: 返回有序集合中指定成员的排名，有序集成员按分数值递减(从大到小)排序|
|   18   |ZSCORE key member: 返回有序集中，成员的分数值                                        |
|   19   |ZUNIONSTORE destination numkeys key [key ...]: 计算给定的一个或多个有序集的并集，并存储在新的 key中|
|   20   |ZSCAN key cursor [MATCH pattern] [COUNT count]: 迭代有序集合中的元素（包括元素成员和元素分值）|



### 3.9 Redis HyperLogLog操作命令
Redis 2.8.9版本添加了HyperLogLog结构。Redis HyperLogLog是用来做基数统计的算法，HyperLogLog的优点是，在输入元素的数量或体积非常非常大时，计算基数所需的空间总是固定的，并且很小的。在Redis里面，每个HyperLogLog键只需要花费12KB内存，就可以计算接近2^64个不同元素的基数。这和计算基数时，数据元素越多耗费内存就越多的集合形成鲜明的对比。但是，因为HyperLogLog只会根据输入元素来计算基数，所以HyperLogLog不能像集合那样，返回输入的各元素。

什么是**基数**呢? 比如数据{1，3，5，7，5，7，8},那么这个数据集的基数集为{1,3,5,7,8}，基数（不重复元素）为5。基数估计就是在误差可接受的范围内，快速计算基数。

参看如下实例：
{% highlight string %}
127.0.0.1:6379> PFADD runoobkey "redis"
(integer) 1
127.0.0.1:6379> PFADD runoobkey "mongodb"
(integer) 1
127.0.0.1:6379> PFADD runoobkey "mysql"
(integer) 1
127.0.0.1:6379> PFCOUNT runoobkey
(integer) 3
{% endhighlight %}
下面列出Hyperloglog操作的一些命令：

| 序号   |        命令及描述                                                                   | 
|:------:|:------------------------------------------------------------------------------------|
|   1    |PFADD key element [element ...]: 添加指定元素到HyperLogLog中                         |
|   2    |PFCOUNT key [key...]: 返回给定HyperLogLog的基数估算值                                |
|   3    |PFMERGE destkey sourcekey [sourcekey...]:将多个HyperLogLog合并为一个HyperLogLog      | 



<br />
<br />

**[参看]**

1. [Redis 教程](http://www.runoob.com/redis/redis-tutorial.html)

2. [redis官网](https://redis.io/)

3. [redis在线测试](http://try.redis.io/)

<br />
<br />
<br />

