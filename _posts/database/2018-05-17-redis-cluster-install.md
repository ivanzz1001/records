---
layout: post
title: redis-cluster安装
tags:
- database
categories: database
description: redis-cluster安装
---



本文简要记录一下redis-cluster的安装及简单使用。具体的安装环境如下：


<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 


# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
搭建redis-cluster，我们当前使用的安装包为```redis-4.0.2```。

## 1. redis-cluster安装

1） 下载redis-4.0.2

到redis官方网站下载对应版本的redis安装包，这里下载```redis 3.2.11```版本：
<pre>
# wget http://download.redis.io/releases/redis-4.0.2.tar.gz
</pre>

2) 解压并安装
<pre>
# tar -jxvf redis-4.0.2.tar.gz
# cd redis-4.0.2

# mkdir -p /apps/redis-4.0.2
# make 
# make PREFIX=/apps/redis-4.0.2 install
cd src && make install
make[1]: Entering directory `/root/redis-inst/pkgs/redis-3.2.11/src'

Hint: It's a good idea to run 'make test' ;)

    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
make[1]: Leaving directory `/root/redis-inst/pkgs/redis-3.2.11/src'

# ls /apps/redis-4.0.2/bin/
redis-benchmark  redis-check-aof  redis-check-rdb  redis-cli  redis-sentinel  redis-server
</pre>
可以看到上面将redis安装到了/apps/redis-4.0.2/目录。同时我们在redis-4.02源代码的src目录下将```redis-trib.rb```也拷贝到*/apps/redis-4.0.2/bin/*目录下（在后面创建集群时需要用到redis-trib.rb）:
<pre>
# cp src/redis-trib.rb /apps/redis-4.0.2/bin
</pre>

3）创建相关目录

我们在*/apps/redis-4.0.2*目录下创建如下子目录：
<pre>
# mkdir /apps/redis-4.0.2/conf
# mkdir /apps/redis-4.0.2/log
# mkdir /apps/redis-4.0.2/run
# mkdir /apps/redis-4.0.2/workdir
# ls /apps/redis-4.0.2/
bin  conf  log  run  workdir
</pre>
其中：

* conf： 用于存放相关配置文件

* log: 用于存放redis-server的运行日志

* run: 用于存放redis-server的pid文件

* workdir: 用于存放redis-server运行时的aof文件、rdb文件等

4） 修改配置文件

首先我们将redis-4.0.2源代码目录下的```redis.conf```以及```sentinel.conf```拷贝到*/apps/redis-4.0.2/conf*目录下，后面我们将以此配置文件作为参考来创建自己的配置：
<pre>
# cp redis.conf /apps/redis-4.0.2/conf/redis-sample.conf
# cp sentinel.conf /apps/redis-4.0.2/conf/sentinel-sample.conf
# ls /apps/redis-4.0.2/conf
redis-sample.conf  sentinel-sample.conf
</pre>

复制redis-sample.conf文件为redis_6379.conf，并进行修改，修改后的配置文件如下：
<pre>
# cat /apps/app/redis-4.0.2/conf/redis_6379.conf | grep -v ^# | grep -v ^$
bind 10.18.20.183
protected-mode yes
port 6379
tcp-backlog 511
timeout 0
tcp-keepalive 300
daemonize yes
supervised no
pidfile /apps/redis-4.0.2/run/redis_6379.pid
loglevel debug
logfile "/apps/redis-4.0.2/log/redis.log"
databases 16
always-show-logo yes
save 900 1
save 300 10
save 60 10000
stop-writes-on-bgsave-error yes
rdbcompression yes
rdbchecksum yes
dbfilename dump.rdb
dir ./
slave-serve-stale-data yes
slave-read-only yes
repl-diskless-sync no
repl-diskless-sync-delay 5
repl-disable-tcp-nodelay no
slave-priority 100
lazyfree-lazy-eviction no
lazyfree-lazy-expire no
lazyfree-lazy-server-del no
slave-lazy-flush no
appendonly yes
appendfilename "appendonly.aof"
appendfsync everysec
no-appendfsync-on-rewrite no
auto-aof-rewrite-percentage 100
auto-aof-rewrite-min-size 64mb
aof-load-truncated yes
aof-use-rdb-preamble no
lua-time-limit 5000
cluster-enabled yes
cluster-config-file nodes-6379.conf
cluster-node-timeout 15000
cluster-slave-validity-factor 10
slowlog-log-slower-than 10000
slowlog-max-len 128
latency-monitor-threshold 0
notify-keyspace-events ""
hash-max-ziplist-entries 512
hash-max-ziplist-value 64
list-max-ziplist-size -2
list-compress-depth 0
set-max-intset-entries 512
zset-max-ziplist-entries 128
zset-max-ziplist-value 64
hll-sparse-max-bytes 3000
activerehashing yes
client-output-buffer-limit normal 0 0 0
client-output-buffer-limit slave 256mb 64mb 60
client-output-buffer-limit pubsub 32mb 8mb 60
hz 10
aof-rewrite-incremental-fsync yes
</pre>

## 2. 创建集群
redis官方提供了```redis-trib.rb```这个工具用来协助我们创建redis-cluster，其是用ruby语言编写的，运行该程序需要依赖于ruby环境(redis-4.0.2源代码中自带的redis-trib.rb需要ruby-2.2.0以上）。这里我们首先需要安装ruby：
<pre>
# yum install ruby rubygems ruby-devel
</pre>
但通常我们这样安装的话，可能安装到的ruby版本较低，不能满足我们的要求。因此我们需要自己来手动安装。

### 2.1 ruby的安装
这里我们所安装的ruby版本为```2.2.4```。

1） 下载ruby-2.2.4

可以到[ruby官网](http://www.ruby-lang.org/en/news/2017/09/14/ruby-2-4-2-released/)去下载：
<pre>
# wget https://cache.ruby-lang.org/pub/ruby/2.4/ruby-2.4.2.tar.bz2
</pre>

2) 解压并安装
<pre>
# tar -zxvf ruby-2.4.2.tar.bz2
# ls
redis-4.0.2  redis-4.0.2.tar.gz  ruby-2.4.2  ruby-2.4.2.tar.bz2
# cd ruby-2.4.2
</pre>
我们将ruby安装到*/usr/local/ruby-2.4.2*目录下，这里我们需要先手动创建，否则后面可能出现相关错误：
<pre>
# mkdir -p /usr/local/ruby-2.4.2
</pre>
接着执行如下命令进行编译安装：
<pre>
# ./configure --prefix=/usr/local/ruby-2.4.2
# make 
# make install
# ls /usr/local/ruby-2.4.2
bin  include  lib  share
</pre>

之后我们将*/usr/local/ruby-2.4.2/bin*目录添加到环境变量PATH中：
<pre>
# export PATH=$PATH:/usr/local/ruby-2.4.2/bin/
</pre>

现在查看一下ruby是否安装成功：
<pre>
# ruby --version
ruby 2.4.2p198 (2017-09-14 revision 59899) [x86_64-linux]
</pre>

### 2.2 创建redis-cluster集群

我们需要使用```redis-trib.rb```来协助我们创建集群，该文件的运行除了需要ruby环境(上面我们已经安装）外，还需要对应的redis库，这里我们可以执行如下命令进行安装：
<pre>
//检查当前是否安装了redis库
# gem list --local

*** LOCAL GEMS ***

bigdecimal (default: 1.3.0)
did_you_mean (1.1.0)
io-console (default: 0.4.6)
json (default: 2.0.4)
minitest (5.10.1)
net-telnet (0.1.1)
power_assert (0.4.1)
psych (default: 2.2.2)
rake (12.0.0)
rdoc (default: 5.0.0)
test-unit (3.2.3)
xmlrpc (0.2.1)
# gem install redis
</pre>
但如果直接这样安装，很可能安装的版本不对。此时我们可以到[gem redis](https://rubygems.org/gems/redis)去下载。当前我们需要下载的版本是redis-4.0.2.gem:
<pre>
# wget https://rubygems.org/downloads/redis-4.0.2.gem
# ls
redis-4.0.2  redis-4.0.2.gem  redis-4.0.2.tar.gz  ruby-2.4.2  ruby-2.4.2.tar.bz2
# gem install --local  ./redis-4.0.2.gem 
</pre>


###### 不带副本的redis-cluster
假设我们在如下三个节点上搭建redis-cluster:

* 192.168.1.10

* 192.168.1.11

* 192.168.1.12

1) **启动redis**

我们在*/apps/redis-4.0.2/workdir*目录下创建启动脚本（redis.sh)：
<pre>
# cat /apps/redis-4.0.2/workdir/redis.sh
cd /apps/redis-4.0.2/workdir
../bin/redis-server /apps/app/redis-4.0.2/conf/redis_6379.conf
</pre>

接着执行如下命令启动redis-server:
<pre>
# chmod 777 ./redis.sh
# wordir/redis.sh
# ps -ef | grep redis
apps      7781     1  0 19:07 ?        00:00:09 ../bin/redis-server 10.18.20.183:6379 [cluster]
</pre>

2） **创建集群分配哈希槽**

由于这里我们创建的redis-cluster是不带副本的，因此可以直接执行如下命令：
<pre>
# redis-trib.rb create 192.168.1.10:6379 192.168.1.11:6379 192.168.1.12:6379
</pre>
创建完成后我们分别登陆192.168.1.10、192.168.1.11、192.168.1.12机器上执行查看相应状态：

* 192.168.1.10机器
{% highlight string %}
$ ls /apps/redis-4.0.2/workdir/
appendonly.aof  nodes-6379.conf  redis.sh

$ bin/redis-cli -h 192.168.1.10 -p 6379
192.168.1.10:6379> info
...
# Replication
role:master
connected_slaves:0
master_replid:d9ae1fb51d202a79e291a95c60f1344255ee6ab4
master_replid2:0000000000000000000000000000000000000000
master_repl_offset:0
second_repl_offset:-1
repl_backlog_active:0
repl_backlog_size:1048576
repl_backlog_first_byte_offset:0
repl_backlog_histlen:0
{% endhighlight %}
上面看到role为master，没有slave。

* 192.168.1.11机器
{% highlight string %}
$ ls /apps/redis-4.0.2/workdir/
appendonly.aof  nodes-6379.conf  redis.sh

$ bin/redis-cli -h 192.168.1.11 -p 6379
192.168.1.11:6379> info
...
# Replication
role:master
connected_slaves:0
master_replid:fdc190a37dc4bf8e1e0411b3d26088c937445e6b
master_replid2:0000000000000000000000000000000000000000
master_repl_offset:0
second_repl_offset:-1
repl_backlog_active:0
repl_backlog_size:1048576
repl_backlog_first_byte_offset:0
repl_backlog_histlen:0
{% endhighlight %}
上面看到role为master，没有slave。

* 192.168.1.12机器

略。

3） **验证**

我们可以在各节点上执行如下命令(例如: 192.168.1.10节点）：
{% highlight string %}
# bin/redis-trib.rb check 192.168.1.10:6379
>>> Performing Cluster Check (using node 192.168.1.10:6379)
M: 35a48a13d7afce6d245df2fe5309fe23c667efb8 10.18.20.183:6379
   slots:0-5460 (5461 slots) master
   0 additional replica(s)
M: 33dcae5df6353b68aecdd4775a4a25e8461a1e14 192.168.1.11:6379
   slots:10923-16383 (5461 slots) master
   0 additional replica(s)
M: 7f457130d65d40b0142c8ba0bbabbfa582afee21 192.168.1.12:6379
   slots:5461-10922 (5462 slots) master
   0 additional replica(s)
[OK] All nodes agree about slots configuration.
>>> Check for open slots...
>>> Check slots coverage...
[OK] All 16384 slots covered.
{% endhighlight %}
之后我们可以登录进去执行相关的写操作：
{% highlight string %}
# bin/redis-cli -h 192.168.1.10 -p 6379
192.168.1.10> set name joke
(error) MOVED 5798 192.168.1.11:6379
{% endhighlight %}
我们看到上面提示写入失败，需要我们将数据写入到```192.168.1.11```节点上（这是因为进行了hash分片)，之后我们登录192.168.1.11节点进行写入：
{% highlight string %}
# bin/redis-cli -h 192.168.1.11 -p 6379
192.168.1.11:6379> set name joke
OK
192.168.1.11:6379> get name
"joke"
{% endhighlight %}

>注： 从这样来看，就要求我们可能需要实现相关代理来访问redis集群了。

###### 创建具有slave的redis-cluster

假设我们在如下六个节点上搭建redis-cluster:

* 192.168.1.10

* 192.168.1.11

* 192.168.1.12

* 192.168.1.13(初始作为10的slave)

* 192.168.1.14(初始作为11的slave)

* 192.168.1.15(初始作为12的slave)

1) **启动redis**

同上，略。

2） **创建集群分配哈希槽**

在此一步骤中，我们执行如下命令：
<pre>
# redis-trib.rb create --replicas 1 192.168.1.10:6379 192.168.1.11:6379 192.168.1.12:6379   192.168.1.13:6379 192.168.1.14:6379 192.168.1.15:6379
</pre>
创建完成后我们登录各机器进行查看(以192.168.1.10为例)：
{% highlight string %}
$ ls /apps/redis-4.0.2/workdir/
appendonly.aof  nodes-6379.conf  redis.sh

$ bin/redis-cli -h 192.168.1.10 -p 6379
192.168.1.10:6379> info
...
# Replication
role:master
connected_slaves:1
slave0:ip=192.168.1.13,port=6379,state=online,offset=42,lag=0
master_replid:d9ae1fb51d202a79e291a95c60f1344255ee6ab4
master_replid2:0000000000000000000000000000000000000000
master_repl_offset:0
second_repl_offset:-1
repl_backlog_active:0
repl_backlog_size:1048576
repl_backlog_first_byte_offset:0
repl_backlog_histlen:0
{% endhighlight %}


<br />
<br />

**[参看]**

1. [Redis cluster的部署](https://blog.csdn.net/weixin_41476978/article/details/81606909)

2. [Redis Cluster集群部署搭建详解](https://www.linuxidc.com/Linux/2018-03/151185.htm)

3. [Linux安装Redis4.02安装和redis的cluster集群配置](https://blog.csdn.net/aaaa5460/article/details/82388387)

4. [Linux 安装Ruby详解(在线和离线安装)](https://www.cnblogs.com/xuliangxing/p/7132656.html?utm_source=itdadao&utm_medium=referral)

<br />
<br />
<br />

