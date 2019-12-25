---
layout: post
title: Zookeeper的安装及使用
tags:
- paxos
categories: paxos
description: Zookeeper的安装及使用
---

Apache ZooKeeper是由Apache Hadoop的子项目发展而来，于2010年11月正式成为了Apache的顶级项目。Zookeeper为分布式应用提供了高效且可靠的分布式协调服务、提供了诸如统一命名服务、配置管理和分布式锁等分布式的基础服务。在解决分布式数据一致性方面，Zookeeper并没有直接采用paxos算法，而是采用了一种被称为```ZAB```(Zookeeper Atomic Broadcast)的一致性协议。

本文我们将首先对Zookeeper进行一个整体上的介绍，包括Zookeeper的设计目标、由来以及它的基本概念，然后将会重点介绍ZAB这一Zookeeper中非常重要的一致性协议。

<!-- more -->


## 1. zookeeper简介

Zookeeper是一个开放源代码的分布式协调服务，由知名互联网公司雅虎创建，是Google Chubby的开源实现。Zookeeper的设计目标是将那些复杂且容易出错的分布式一致性服务封装起来，构成一个高效可靠的原语集，并以一系列简单易用的接口提供给用户使用。

### 1.1 Zookeeper是什么
Zookeeper是一个典型的分布式数据一致性的解决方案，分布式应用程序可以基于它实现诸如数据发布/订阅、负载均衡、命名服务、分布式协调/通知、集群管理、Master选举、分布式锁和分布式队列等功能。Zookeeper可以保证如下分布式一致性特性。

* 顺序一致性： 从同一个客户端发起的事务请求，最终将会严格的按照其发起顺序被应用到Zookeeper中去

* 原子性： 所有事务请求的处理结果在整个集群中所有机器上的应用情况是一致的，也就是说，要么整个集群所有机器都成功应用了某一个事务，要么都没用应用，一定不会出现集群中部分机器应用了该事务，而另外一部分没有应用的情况。

* 单一视图(single system image): 无论客户端连接的是哪个Zookeeper服务器，其看到的服务端数据模型都是一致的

* 可靠性： 一旦服务端成功地应用了一个事务，并完成对客户端的响应，那么该事务所引起的服务端状态变更将会一直保存下来，除非有另一个事务又对其进行了变更。

* 实时性： 通常人们看到实时性的第一反应是，一旦一个事务被成功应用，那么客户端能够立即从服务端上读取到这个事务变更后的最新数据状态。这里需要注意的是，Zookeeper仅仅保证在一定的时间段内，客户端最终一定能够从服务端上读取到最新的数据状态。

### 1.2 Zookeeper的设计目标
Zookeeper致力于提供一个高性能、高可用，且具有严格的顺序访问控制能力（主要是写操作的严格顺序性）的分布式协调服务。高性能使得Zookeeper能够应用于那些对系统吞吐有明确要求的大型分布式系统中，高可用使得分布式的单点问题得到了很好的解决，而严格的顺序访问控制使得客户端能够基于Zookeeper实现一些复杂的同步原语。下面我们来具体看一下Zookeeper的四个设计目标：

###### 目标1： 简单的数据模型
Zookeeper使得分布式程序能够通过一个共享的、树型结构的名字空间来进行相互协调。这里所说的树型结构的名字空间，是指Zookeeper服务器内存中的一个数目模型，其由一系列被称为```znode```的数据节点组成。总的来说，其数据模型类似于一个文件系统，而ZNode之间的层次关系，就像文件系统的目录结构一样。不过和传统的磁盘文件系统不同的是，Zookeeper将全量数据存储在内存中，以此来实现提高服务器吞吐、减少延迟的目的。

###### 目标2： 可以构建集群
一个Zookeeper集群通常由一组机器组成，一般3~5台机器就可以组成一个可用的Zookeeper集群了。如下图所示：

![zookeeper-service](https://ivanzz1001.github.io/records/assets/img/paxos/zookeeper-service.png)

组成Zookeeper集群的每台机器都会在内存中维护当前的服务器状态，并且每台机器之间都互相保持着通信。值得一提的是，只要集群中存在超过一半的机器能够正常工作，那么整个集群就能够正常对外服务。

Zookeeper的客户端程序会选择和集群中任意一台机器共同来创建一个TCP连接，而一旦客户端和某台Zookeeper服务器之间的连接断开后，客户端会自动连接到集群中的其他机器。

###### 目标3： 顺序访问
对于来自客户端的每个更新请求，Zookeeper都会分配一个全局唯一的递增编号，这个编号反映了所有事务操作的先后顺序（我们称这个编号为zxid，即Zookeeper Transaction ID)，应用程序可以使用Zookeeper的这个特性来实现更高层次的同步原语。

###### 目标4： 高性能
由于Zookeeper将全量数据存储在内存中，并直接服务于客户端的所有非事务请求，因此它尤其适用于以读操作为主的应用场景。作者曾经以3台3.4.3版本的Zookeeper服务器组成集群进行性能压测，100%读请求的场景下压测结果是12~13W的QPS.

### 1.3 Zookeeper基本概念
本节将介绍Zookeeper的几个核心概念。后面很多地方都会涉及到。

1） **集群角色**

通常在分布式系统中，构成一个集群的每一台机器都有自己的角色，最典型的集群模式就是master/slave模式（即主备模式）。在这种模式下，我们把能够处理所有写操作的机器称为master机器，把所有通过异步复制获取最新数据，并提供读服务的机器称为Slave机器。

而在Zookeeper中，这些概念被颠覆了。它没有沿用传统的Master/Slave概念，而是引入了Leader、Follower和Observer三种角色。Zookeeper集群中的所有机器通过一个Leader选举过程来选定一台被称为```Leader```的机器，Leader服务器为客户端提供读和写服务。除Leader外，其他机器包括Follower和Observer。Follower和Observer都能够提供读服务，唯一的区别在于，Observer机器不参与Leader选举过程，也不参与写操作的“过半写成功”策略，因此Observer可以在不影响写性能的情况下提升集群的读性能。

2） **会话(Session)**

Session是指客户端会话，在讲解会话之前，我们首先来了解一下客户端连接。在Zookeeper中，一个客户端连接是指客户端与服务器之间的一个TCP长连接。Zookeeper对外的服务端口默认是```2181```，客户端启动的时候，首先会与服务器建立一个TCP连接，从第一次连接建立开始，客户端会话的生命周期也开始了，通过这个连接，客户端能够通过心跳检测与服务器保持有效的会话，也能够向Zookeeper服务器发送请求并接受响应，同时还能够通过该连接接收来自服务器的Watch事件通知。

Session的sessionTimeout值用来设置一个客户端会话的超时时间。当由于服务器压力太大、网络故障或是客户端主动断开连接等各种原因导致客户端连接断开时，只要在sessionTimeout规定的时间内能够重新连接上集群中任意一台服务器，那么之前创建的会话仍然有效。


3） **数据节点(Znode)**

在谈到分布式的时候，我们通常说的“节点”是指组成集群的每一台机器。然而，在Zookeeper中，“节点”分为两类，第一类同样是指构成集群的机器，我们称之为机器节点； 第二类则是指数据模型中的数据单元，我们称之为数据节点———ZNode。Zookeeper将所有数据存储在内存中，数据模型是一棵树(ZNode Tree)，由斜杠(/)进行分割的路径，就是一个Znode，例如/foo/path1。每个ZNode上都会保存自己的数据内容，同时还会保存一系列属性信息。

在Zookeeper中，ZNode可以分为持久节点和临时节点两类。所谓持久节点是指一旦这个ZNode被创建了，除非主动进行ZNode移除操作，否者这个ZNode将一直保存在Zookeeper上。而临时节点就不一样了，它的生命周期和客户端会话绑定，一旦客户端会话失效，那么这个客户端所创建的所有临时节点都会被移除。另外，Zookeeper还允许用户为每个节点添加一个特殊的属性： SEQUENTIAL。一旦节点被标记上这个属性，那么这个节点被创建的时候，Zookeeper会自动在其节点后面追加一个整形数字，这个整形数字是一个由父节点维护的自增数字。

4） **版本**

在上面我们提到，Zookeeper的每个ZNode上都会存储数据，对应于每个ZNode，Zookeeper都会为其维护一个叫做```Stat```的数据结构，Stat中记录了这个ZNode的三个数据版本，分别是version(当前ZNode的版本）、cversion(当前ZNode子节点的版本）和aversion(当前ZNode的ACL版本）。

5） **Watcher**

Watcher(事件监听器），是Zookeeper中的一个很重要的特性。Zookeeper允许用户在指定节点上注册一些Watcher，并且在一些特定事件触发的时候，Zookeeper服务端会将事件通知到感兴趣的客户端上去，该机制是Zookeeper实现分布式协调服务的重要特性。

6） **ACL**

Zookeeper采用ACL(Access Control Lists)策略来进行权限控制，类似于Unix文件系统的权限控制。Zookeeper定义了如下5种权限：

* CREATE: 创建子节点的权限

* READ： 获取节点数据和子节点列表的权限

* WRITE: 更新节点数据的权限

* DELETE: 删除子节点的权限

* ADMIN: 设置节点ACL的权限

其中尤其需要注意的是，CREATE和DELETE这两种权限都是针对子节点的权限控制。


## 2. ZooKeeper环境的搭建
Zookeeper有两种运行模式： 单机模式和集群模式。这里我们分别简要的讲解一下。由于Zookeeper的运行需要依赖java环境，因此首先需要搭建Java运行环境，请参看相关文档:
<pre>
# javac -version
javac 1.8.0_131
</pre>

当前zookeeper最新的稳定版本是```v3.5.6```，当前的操作系统环境为：
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
# uname -a
Linux oss-uat-06 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

### 2.1 单机模式

当前最新版本(v3.5.6)支持源代码安装、bin解压安装两种方式：
<pre>
[   ] apache-zookeeper-3.5.6-bin.tar.gz 2019-10-16 08:35  8.8M  
[   ] apache-zookeeper-3.5.6.tar.gz     2019-10-16 08:35  3.0M  
</pre>
如果我们要采用源代码安装，请下载*apache-zookeeper-[version].tar.gz*文件（需要Java 1.8 u211版本以上）。如果不想自己编译就直接下载*apache-zookeeper-[version]-bin.tar.gz*即可。 

1） 下载Zookeeper

到Apache zookeeper官网http://zookeeper.apache.org/下载3.5.6版本的zookeeper（这里我们采用bin安装):
<pre>
# wget https://mirrors.tuna.tsinghua.edu.cn/apache/zookeeper/stable/apache-zookeeper-3.5.6-bin.tar.gz
</pre>

2) 解压并安装

直接解压即可，不需要进行额外的安装操作:
<pre>
# tar -zxvf apache-zookeeper-3.5.6-bin.tar.gz
# cd apache-zookeeper-3.5.6-bin
# ls
bin  conf  docs  lib  LICENSE.txt  NOTICE.txt  README.md  README_packaging.txt
</pre>
在conf目录下有一个示例配置文件zoo_sample.cfg，我们来看一下：
<pre>
# cat conf/zoo_sample.cfg | grep -v ^#
tickTime=2000
initLimit=10
syncLimit=5
dataDir=/tmp/zookeeper
clientPort=2181
</pre>
这里我们简要介绍一下里面相关字段的含义：

* tickTime: zookeeper所使用的基本时间单元（单位： ms)。其会被用于作为heartbeat的时间间隔，最低的session过期时间为tickTime的两倍。

* dataDir: 用于指定存放内存数据库快照的位置。对数据库进行更新的事务日志也会存放在该目录（除非另行指定）

* clientPort: 用于监听客户端连接的端口

* initLimit: 在```初始```同步阶段，follower连接到leader并进行同步的时间限制(单位：tickTime)。如果Zookeeper所管理的数据较大时，可以适当调大本值

* syncLimit: 当followers与leader进行同步时的时间限制(单位: tickTime)

这里我们将```zoo_sample.cfg```重命名为```zoo.cfg```:
<pre>
# cp conf/zoo_sample.cfg conf/zoo.cfg
</pre>

3) 启动zookeeper

执行如下命令启动zookeeper server:
<pre>
# bin/zkServer.sh start
ZooKeeper JMX enabled by default
Using config: /root/zookeeper-inst/apache-zookeeper-3.5.6-bin/bin/../conf/zoo.cfg
Starting zookeeper ... STARTED

# netstat -nlp | grep 2181
tcp6       0      0 :::2181                 :::*                    LISTEN      88510/java 
</pre>
可以查看zookeeper server的相关日志确认已经启动成功：
<pre>
# cat logs/zookeeper-root-server-localhost.localdomain.out 
2019-11-08 18:52:39,581 [myid:] - INFO  [main:QuorumPeerConfig@133] - Reading configuration from: /root/zookeeper-inst/apache-zookeeper-3.5.6-bin/bin/../conf/zoo.cfg
2019-11-08 18:52:39,609 [myid:] - INFO  [main:QuorumPeerConfig@385] - clientPortAddress is 0.0.0.0/0.0.0.0:2181
2019-11-08 18:52:39,609 [myid:] - INFO  [main:QuorumPeerConfig@389] - secureClientPort is not set
2019-11-08 18:52:39,611 [myid:] - INFO  [main:DatadirCleanupManager@78] - autopurge.snapRetainCount set to 3
2019-11-08 18:52:39,611 [myid:] - INFO  [main:DatadirCleanupManager@79] - autopurge.purgeInterval set to 0
2019-11-08 18:52:39,612 [myid:] - INFO  [main:DatadirCleanupManager@101] - Purge task is not scheduled.
2019-11-08 18:52:39,612 [myid:] - WARN  [main:QuorumPeerMain@125] - Either no config or no quorum defined in config, running  in standalone mode
2019-11-08 18:52:39,620 [myid:] - INFO  [main:ManagedUtil@46] - Log4j found with jmx enabled.
2019-11-08 18:52:39,653 [myid:] - INFO  [main:QuorumPeerConfig@133] - Reading configuration from: /root/zookeeper-inst/apache-zookeeper-3.5.6-bin/bin/../conf/zoo.cfg
2019-11-08 18:52:39,653 [myid:] - INFO  [main:QuorumPeerConfig@385] - clientPortAddress is 0.0.0.0/0.0.0.0:2181
2019-11-08 18:52:39,653 [myid:] - INFO  [main:QuorumPeerConfig@389] - secureClientPort is not set
2019-11-08 18:52:39,654 [myid:] - INFO  [main:ZooKeeperServerMain@117] - Starting server
</pre>

###### 2.1.1 zookeeper的测试
首先我们执行如下命令连接到zookeeper:
<pre>
#  bin/zkCli.sh -server 127.0.0.1:2181
Connecting to 127.0.0.1:2181
2019-11-08 18:57:31,115 [myid:] - INFO  [main:Environment@109] - Client environment:zookeeper.version=3.5.6-c11b7e26bc554b8523dc929761dd28808913f091, built on 10/08/2019 20:18 GMT
...
...
WATCHER::

WatchedEvent state:SyncConnected type:None path:null
[zk: 127.0.0.1:2181(CONNECTED) 0] 
</pre>
之后我们执行```help```命令，查看相关帮助信息：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 0] help
ZooKeeper -server host:port cmd args
        addauth scheme auth
        close 
        config [-c] [-w] [-s]
        connect host:port
        create [-s] [-e] [-c] [-t ttl] path [data] [acl]
        delete [-v version] path
        deleteall path
        delquota [-n|-b] path
        get [-s] [-w] path
        getAcl [-s] path
        history 
        listquota path
        ls [-s] [-w] [-R] path
        ls2 path [watch]
        printwatches on|off
        quit 
        reconfig [-s] [-v version] [[-file path] | [-members serverID=host:port1:port2;port3[,...]*]] | [-add serverId=host:port1:port2;port3[,...]]* [-remove serverId[,...]*]
        redo cmdno
        removewatches path [-c|-d|-a] [-l]
        rmr path
        set [-s] [-v version] path data
        setAcl [-s] [-v version] [-R] path acl
        setquota -n|-b val path
        stat [-w] path
        sync path
Command not found: Command not found help
[zk: 127.0.0.1:2181(CONNECTED) 1] 
</pre>

这里可以先尝试执行一些简单的命令。首先执行```ls```命令：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 2] ls /
[zookeeper]
</pre>

接下来我们通过运行*create /zk_test my_data*命令来新创建一个znode。该命令会创建一个新的znode，并将该znode与字符串```my_data```相关联：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 3] create /zk_test my_data
Created /zk_test
</pre>
>注： znode节点必须一级一级创建

然后我们再执行```ls```命令，可以看到：
<pre>
[zkshell: 11] ls /
[zookeeper, zk_test]
</pre>
上面我们注意到已经创建了```zk_test```目录。

接下来，我们通过执行```get```命令来检查```zk_test```这个znode是否和我们的数据进行了关联：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 8] get -s -w /zk_test
my_data
cZxid = 0x2
ctime = Fri Nov 08 19:04:58 CST 2019
mZxid = 0x2
mtime = Fri Nov 08 19:04:58 CST 2019
pZxid = 0x2
cversion = 0
dataVersion = 0
aclVersion = 0
ephemeralOwner = 0x0
dataLength = 7
numChildren = 0
</pre>
我们也可以更改```zk_test```这个znode所关联的数据：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 9] set /zk_test junk

WATCHER::

WatchedEvent state:SyncConnected type:NodeDataChanged path:/zk_test
[zk: 127.0.0.1:2181(CONNECTED) 10] get -s -w /zk_test
junk
cZxid = 0x2
ctime = Fri Nov 08 19:04:58 CST 2019
mZxid = 0x7
mtime = Fri Nov 08 19:12:36 CST 2019
pZxid = 0x2
cversion = 0
dataVersion = 1
aclVersion = 0
ephemeralOwner = 0x0
dataLength = 4
numChildren = 0
</pre>

我们看到确实已经完成了修改。最后我们可以删除该节点：
<pre>
[zk: 127.0.0.1:2181(CONNECTED) 12] delete /zk_test

WATCHER::

WatchedEvent state:SyncConnected type:NodeDeleted path:/zk_test
</pre>

<br />

### 2.2 集群模式的搭建
如果要提供一个可靠的Zookeeper服务，我们应该部署一个zookeeper集群。只要集群的大部分节点处于```up```状态，zookeeper服务就是可用的。因为Zookeeper需要一个```多数派```(majority)，因此整个集群的节点个数最好是奇数个。


>说明： 正如上面所提到的，在建立zookeeper集群时至少需要3个节点，以达到最基本的容错性。
>
>而在实际生产环境中，三个节点往往是不够的。我们为了维持zookeeper在维护期间的最大的可用性，我们可能需要5个zookeeper节点。假如在一个节点失效，并且我们在对另一个节点进行维护的过程中，zookeeper仍能正常的提供服务。
>
>我们在对集群进行冗余考虑时，应该包含各个方面。假如我们的Zookeeper集群有3个节点，但是它们都连到同一台交换机上，如果这台交换机出现了故障，则会导致整个集群不可用。


这里我们假设需要部署的3台zookeeper服务器分别为： zoo1、zoo2、zoo3
<pre>
zookeeper服务器名               IP地址                        域名
-----------------------------------------------------------------------------------------
   zoo1                      192.168.79.128                (未设置)
   zoo2                      192.168.79.129                (未设置)
   zoo3                      192.168.79.131                (未设置)
</pre>



如下我们列出如何搭建zookeeper集群的相关步骤（这些步骤在每一台机器上都应该执行一遍）：

* 安装[JDK](https://www.oracle.com/technetwork/java/javase/downloads/index.html)

* 设置Java虚拟机的heap大小。这一点是相当重要的，可以避免不必要的内存交换，否则可能会严重影响Zookeeper的性能。为了确定一个合适的值，请使用负载测试方法。如果不想测试的话，对于4G内存的主机，建议将Java虚拟机heap大小设置为3GB。

* 安装Zookeeper server

* 创建配置文件(文件名称可以随意)
<pre>
tickTime=2000
dataDir=/opt/zookeeper/
clientPort=2181
initLimit=5
syncLimit=2
server.1=192.168.79.128:2888:3888
server.2=192.168.79.129:2888:3888
server.3=192.168.79.131:2888:3888
</pre>
你可以在[Configuration Parameter](https://zookeeper.apache.org/doc/r3.5.6/zookeeperAdmin.html#sc_configuration)相关章节找到这些配置参数的含义。在这里我们只需要知道： Zookeeper集群中的每一个节点之间都需要互相能够连通。你可以通过在配置文件中使用*server.id=host:port:port*这样的形式来实现。其中```host```是该机器的主机名；第一个Port用于在Leader选举成功后，Follower用该端口来连接Leader； 第二个Port用于Leader选举。集群中的每一个节点会在对应的数据目录下创建一个myid文件，用于唯一的标识自己。

* myid文件只有一行，就是存放我们为该机器指定的id。因此对于上面```server.1```主机上的myid文件，其内容为```1```。id在整个zookeeper集群中必须唯一，且范围是[1,255]。（注： 如果你启用了zookeeper的一些扩展特性，如TTL节点话，则对应的id范围则必须为[1,254]）。本步骤我们需要通过手动方式来创建myid文件，例如在```zoo1```这台主机上，我们执行命令：
<pre>
# mkdir -p /opt/zookeeper/
# echo "1" > /opt/zookeeper/myid
# cat /opt/zookeeper/myid
</pre>

* 假如配置文件建立好之后，可以通过如下方式启动Zookeeper Server
<pre>
# ls
bin  conf  docs  lib  LICENSE.txt  NOTICE.txt  README.md  README_packaging.txt
# bin/zkServer.sh start 
</pre>

* 查看集群的运行情况

我们可以查看对应目录下的日志：
<pre>
# tail -f logs/zookeeper-root-server-localhost.localdomain.out 
        at org.apache.zookeeper.server.quorum.Learner.sockConnect(Learner.java:233)
        at org.apache.zookeeper.server.quorum.Learner.connectToLeader(Learner.java:262)
        at org.apache.zookeeper.server.quorum.Follower.followLeader(Follower.java:77)
        at org.apache.zookeeper.server.quorum.QuorumPeer.run(QuorumPeer.java:1271)
2019-11-19 23:59:12,552 [myid:1] - INFO  [QuorumPeer[myid=1](plain=/0:0:0:0:0:0:0:0:2181)(secure=disabled):Learner@391] - Getting a diff from the leader 0x0
2019-11-19 23:59:12,557 [myid:1] - INFO  [QuorumPeer[myid=1](plain=/0:0:0:0:0:0:0:0:2181)(secure=disabled):Learner@546] - Learner received NEWLEADER message
2019-11-19 23:59:12,626 [myid:1] - INFO  [QuorumPeer[myid=1](plain=/0:0:0:0:0:0:0:0:2181)(secure=disabled):Learner@529] - Learner received UPTODATE message
2019-11-19 23:59:12,631 [myid:1] - INFO  [QuorumPeer[myid=1](plain=/0:0:0:0:0:0:0:0:2181)(secure=disabled):CommitProcessor@256] - Configuring CommitProcessor with 4 worker threads.
2019-11-19 23:59:17,007 [myid:1] - INFO  [/192.168.79.128:3888:QuorumCnxManager$Listener@918] - Received connection request 192.168.79.131:36560
2019-11-19 23:59:17,013 [myid:1] - INFO  [WorkerReceiver[myid=1]:FastLeaderElection@679] - Notification: 2 (message format version), 3 (n.leader), 0x0 (n.zxid), 0x1 (n.round), LOOKING (n.state), 3 (n.sid), 0x0 (n.peerEPoch), FOLLOWING (my state)0 (n.config version)
</pre>


我们也可以通过如下方式查看集群：
<pre>
# bin/zkServer.sh status
/usr/bin/java
ZooKeeper JMX enabled by default
Using config: /app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../conf/zoo.cfg
Client port found: 2181. Client address: localhost.
Mode: follower
</pre>

执行```ps -ef```命令可以查看zookeeper的实际启动参数(输出格式已做整理)：
{% highlight string %}
# ps -ef | grep zookeeper
root      14176      1  2 22:32 pts/0    00:00:03 java -Dzookeeper.log.dir=/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../logs 
-Dzookeeper.log.file=zookeeper-root-server-localhost.localdomain.log 
-Dzookeeper.root.logger=INFO,CONSOLE 
-XX:+HeapDumpOnOutOfMemoryError 
-XX:OnOutOfMemoryError=kill -9 %p 
-cp /app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../zookeeper-server/target/classes:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../build/classes:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../zookeeper-server/target/lib/*.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../build/lib/*.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/zookeeper-jute-3.5.6.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/zookeeper-3.5.6.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/slf4j-log4j12-1.7.25.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/slf4j-api-1.7.25.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-transport-native-unix-common-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-transport-native-epoll-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-transport-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-resolver-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-handler-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-common-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-codec-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/netty-buffer-4.1.42.Final.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/log4j-1.2.17.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/json-simple-1.1.1.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jline-2.11.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-util-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-servlet-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-server-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-security-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-io-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jetty-http-9.4.17.v20190418.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/javax.servlet-api-3.1.0.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jackson-databind-2.9.10.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jackson-core-2.9.10.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/jackson-annotations-2.9.10.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/commons-cli-1.2.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../lib/audience-annotations-0.5.0.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../zookeeper-*.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../zookeeper-server/src/main/resources/lib/*.jar:\
/app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../conf:
-Xmx1000m 
-Dcom.sun.management.jmxremote
-Dcom.sun.management.jmxremote.local.only=false 
org.apache.zookeeper.server.quorum.QuorumPeerMain /app/zookeeper/apache-zookeeper-3.5.6-bin/bin/../conf/zoo.cfg
{% endhighlight %}

#### 2.2.1 集群模式的测试
通过查看日志或者执行```bin/zkServer.sh status```命令，我们发现目前zookeeper集群的主从情况如下：

* 192.168.79.128(Follower)

* 192.168.79.129(Follower)

* 192.168.79.131(Leader)

1) Follower上操作

接着我们使用zookeeper客户端登录*192.168.79.128*这个zookeeper服务器（在192.168.79.128主机上操作)：
{% highlight string %}
# bin/zkCli.sh -server 127.0.0.1:2181
[zk: 127.0.0.1:2181(CONNECTED) 2] ls /
[zookeeper]
{% endhighlight %}
执行命令*create /zk_test my_data*尝试创建一个节点:
{% highlight string %}
[zk: 127.0.0.1:2181(CONNECTED) 3] create /zk_kafka_topic topics
Created /zk_kafka_topic
[zk: 127.0.0.1:2181(CONNECTED) 4] ls /
[zk_kafka_topic, zookeeper]
{% endhighlight %}
可以看到节点创建成功了。同时在执行上面这条命令时，我们在192.168.79.128主机上使用tcpdump进行抓包：
<pre>
# tcpdump -i ens33 tcp and port 2888 -w follower.pcap
</pre>
之后使用wireshark进行分析，我们看到有如下流程：
{% highlight string %}
No     Time       Source         Destination     Protocol    Length      Info
----------------------------------------------------------------------------------------------------------
193  31.941347  192.168.79.128  192.168.79.131     tcp        181        52762->2888 [PSH,ACK] Seq=2051940045 Ack=293419740 Win=246 Len=115 TSval=2088146587 TSecr=2088127400
194  31.942296  192.168.79.131  192.168.79.128     tcp        66         2888->52762 [ACK] Seq=293419740 Ack=2051940160 Win=227 Len=0 TSval=2088128309 TSecr=2088146587
195  31.943332  192.168.79.131  192.168.79.128     tcp        179        2888->52762 [PSH,ACK] Seq=293419740 Ack=2051940160 Win=227 Len=113 TSval=2088128310 TSecr=2088146587
196  31.943931  192.168.79.131  192.168.79.129     tcp        179        2888->43924 [PSH,ACK] Seq=2728426865 Ack=2309074595 Win=227 Len=113 TSval=2088128310 TSecr=2088115286
{% endhighlight %}
从这里我们看到，follower首先是将写请求发送到Leader，之后再由Leader向各个Follower执行写操作。


<br />
<br />
**参看：**

1. [Zookeeper和 Google Chubby对比分析](https://www.cnblogs.com/grefr/p/6088115.html)

2. [The Chubby lock service for loosely coupled distributed systems](https://github.com/lwhile/The-Chubby-lock-service-for-loosely-coupled-distributed-systems-zh_cn)

3. [zookeeper官网](https://zookeeper.apache.org/)

4. [ZooKeeper Getting Started Guide](https://zookeeper.apache.org/doc/r3.5.6/zookeeperStarted.html)

5. [ZooKeeper Administrator's Guide](https://zookeeper.apache.org/doc/r3.5.6/zookeeperAdmin.html)

6. [zookeeper3.5.6安装](https://blog.csdn.net/qinqinde123/article/details/102854529)

<br />
<br />
<br />


