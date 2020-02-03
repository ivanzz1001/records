---
layout: post
title: libpaxos库的安装
tags:
- paxos
categories: paxos
description: libpaxos库的安装
---

libpaxos库实现了paxos协议。在了解具体的paxos协议之前，我们先来编译安装libpaxos库。

<!-- more -->

参看文章：

* [搭建paxos测试过程](http://blog.csdn.net/zhaoforyou/article/details/53573407)
* [libpaxos官网](https://bitbucket.org/sciascid/libpaxos)


## 1. LibPaxos简要介绍
在这里我们要介绍的是LibPaxos3,它是对LibPaxos2的完全重写。LibPaxos3在如下方面得到了提高：

* 不适用组播
* 更清晰的设计
* 基于CMake的更好的构建系统
* 拥有单元测试用例

LibPaxos3被分割成两个库： libpaxos和libevpaxos。

LibPaxos(见```libpaxos/paxos```)实现了Paxos公开协议的内核，并没有夹杂网络部分的代码。libpaxos并不依赖与任何特定的网络库。

Libevpaxos(见```libpaxos/evpaxos```)是实际的网络Paxos的实现。该库构建在libpaxos及libevent上。

LibPaxos3需要需要依赖于libevent,msgpack。可选支持LMDB。

## 2. LibPaxos源代码编译


### 2.1 编译安装libevent
libevent是一个异步事件回调封装。支持epoll，select，poll等模式；支持定时器、IO、信号等事件。官网地址为：http://libevent.org/。

这里我们下载```libevent-2.0.22-stable.tar.gz ```版本，执行如下命令：
<pre>
# wget https://github.com/libevent/libevent/releases/download/release-2.0.22-stable/libevent-2.0.22-stable.tar.gz
# tar -zxvf libevent-2.0.22-stable.tar.gz
# cd libevent-2.0.22-stable/

# make
# make install 
</pre>

默认情况下，libevent头文件会被安装到/usr/local/include目录下，库文件会被安装到/usr/local/lib目录下。


### 2.2 编译安装MessagePack
messagepack是一个跨平台的序列化库，号称比json生成的数据小，速度非常块快。我们可以在这里下载：https://github.com/msgpack/msgpack-c。
<pre>
# git clone https://github.com/msgpack/msgpack-c.git
# cd msgpack-c
# cat README.md

# cmake .
# make
# sudo make install
</pre>
默认情况下，msgpack头文件会被安装到/usr/local/include目录下，库文件会被安装到/usr/local/lib目录下。


### 2.3 LMDB库
LMDB库是一个快速KV库。比leveldb快15%，但内存占用比leveldb大。比leveldb的优点是支持多进程同时读取。我们这里暂时不用。


### 2.4 编译安装LibPaxos

执行如下命令：
<pre>
# git clone https://bitbucket.org/sciascid/libpaxos.git
# cd libpaxos/
# cat README.md 

# mkdir build
# cd build
# cmake ..
# make
</pre>
 

## 3. 测试运行
在官方网站中给出有以下运行示例：
<pre>
# cd libpaxos/build
# ./sample/acceptor 0 ../paxos.conf > /dev/null &
# ./sample/acceptor 1 ../paxos.conf > /dev/null &
# ./sample/proposer 0 ../paxos.conf > /dev/null &
# ./sample/learner ../paxos.conf > learner.txt &
# ./sample/client 127.0.0.1:5550 1
</pre>
但在实际运行过程中，发现存在问题。此处，我们暂时按如下方式运行（这里不在后台运行，因此需要开5个控制终端）：
<pre>
# ./sample/acceptor 0 ../paxos.conf          //控制终端1运行

# ./sample/acceptor 1 ../paxos.conf         //控制终端2运行

# ./sample/proposer 2 ../paxos.conf         //控制终端3运行

# ./sample/learner ../paxos.conf            //控制终端4运行

# ./sample/client ../paxos.conf             //控制终端5运行
</pre>
在终端5上运行结果如下：
{% highlight string %}
[root@localhost build]# ./sample/client ../paxos.conf 
17 Aug 02:08:06. Connect to 127.0.0.1:8800
17 Aug 02:08:06. Connect to 127.0.0.1:8801
17 Aug 02:08:06. Connect to 127.0.0.1:8802
Connected to proposer
17 Aug 02:08:06. Connected to 127.0.0.1:8800
17 Aug 02:08:06. Connected to 127.0.0.1:8801
17 Aug 02:08:06. Connected to 127.0.0.1:8802
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
0 value/sec, 0.00 Mbps, latency min 0 us max 0 us avg 0 us
....
{% endhighlight %}


paxos.conf配置文件如下：
<pre>
[root@localhost libpaxos]# cat paxos.conf 
## LibPaxos configuration file

# Specify an id, ip address and port for each replica.
# Ids must start from 0 and must be unique.
replica 0 127.0.0.1 8800
replica 1 127.0.0.1 8801
replica 2 127.0.0.1 8802

# Alternatively it is possible to specify acceptors and proposers separately.
# acceptor 0 127.0.0.1 8800
# acceptor 1 127.0.0.1 8801
# acceptor 2 127.0.0.1 8802

#proposer 0 127.0.0.1 5550
# proposer 1 127.0.0.1 5551
# proposer 2 127.0.0.1 5552


# Verbosity level: must be one of quiet, error, info, or debug.
# Default is info.
# verbosity debug

# Enable TCP_NODELAY?
# Default is 'yes'.
# tcp-nodelay no

################################### Learners ##################################

# Should learners start from instance 0 when starting up?
# Default is 'yes'.
# learner-catch-up no

################################## Proposers ##################################

# How many seconds should pass before a proposer times out an instance?
# Default is 1.
# proposer-timeout 10

# How many phase 1 instances should proposers preexecute?
# Default is 128.
# proposer-preexec-window 1024

################################## Acceptors ##################################

# Acceptor storage backend: must be one of memory or lmdb.
# Default is memory.
# storage-backend lmdb

# Should the acceptor trash previous storage files and start from scratch?
# This is here only for testing purposes.
# Default is 'no'.
# acceptor-trash-files yes

############################ LMDB acceptor storage ############################

# Should lmdb write to disk synchronously?
# Default is 'no'.
# lmdb-sync yes

# Path for lmdb database environment.
lmdb-env-path /tmp/acceptor

# lmdb's map size in bytes (maximum size of the database).
# Accepted units are mb, kb and gb.
# Default is 10mb.
# lmdb-mapsize 1gb
</pre>


<br />
<br />

**参看：**

1. [优酷教育(paxos)](https://edu.youku.com/)



