---
layout: post
title: Centos7环境下Kafka的安装(单机版)
tags:
- mq
categories: mq
description: Centos7环境下Kafka的安装(单机版)
---

Kafka是一种高吞吐的分布式发布订阅消息系统，能够替代传统的消息队列用于解耦数据处理，缓存未处理消息等，同时具有更高的吞吐率，支持分区、多副本、冗余，因此被广泛用于大规模消息数据处理应用。Kafka支持Java及多种其他语言客户端，可与Hadoop、Storm、Spark等其他大数据工具结合使用。


本教程主要介绍Kafka 在Centos7上的安装和使用，下面会分别介绍Kafka的单点部署。我们当前操作系统环境为：



<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
# uname -a
Linux oss-uat-06 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

当前我们安装的kafka版本为```kafka_2.12```。


## 1. 安装JDK



关于JDK的安装，请参看其他文档。

## 2. 安装zookeeper

这里安装的zookeeper采用单机模式工作，因此如下的操作配置都是采用```单机模式配置```:


1) 下载zookeeper

到Apache zookeeper官网```http://zookeeper.apache.org/```下载```3.4.9```版本的zookeeper:
<pre>
# wget https://archive.apache.org/dist/zookeeper/zookeeper-3.4.9/zookeeper-3.4.9.tar.gz
</pre>

2) 解压并安装

直接解压即可，不需要进行额外的安装操作：
<pre>
# tar -zxvf zookeeper-3.4.9.tar.gz
# cd zookeeper-3.4.9
# ls
bin        CHANGES.txt  contrib     docs             ivy.xml  LICENSE.txt  README_packaging.txt  recipes  zookeeper-3.4.9.jar      zookeeper-3.4.9.jar.md5
build.xml  conf         dist-maven  ivysettings.xml  lib      NOTICE.txt   README.txt            src      zookeeper-3.4.9.jar.asc  zookeeper-3.4.9.jar.sha1
</pre>

3） 配置zookeeper环境变量

这里我们为了后面的操作方便，为zookeeper配置环境变量（此步骤可选），向/etc/profile中添加如下：
<pre>
export ZOOKEEPER_HOME=/root/zookeeper-3.4.9

export PATH=$PATH:$ZOOKEEPER_HOME/bin;$ZOOKEEPER_HOME/conf
</pre>
再执行```source /etc/profile```是环境变量生效。

4) 配置zookeeper

zookeeper服务器包含在单个jar文件中（本环境下为zookeeper-3.4.9.jar)，安装此服务需要用户自己创建一个配置文件。默认配置文件路径为zookeeper-3.4.9/conf/目录下，文件名为```zoo.cfg```。我们进入conf目录可以看到一个```zoo_sample.cfg```文件，可供参考。这里我们以zoo_sample.cfg作为模板，配置我们自己的zoo.cfg:
<pre>
# cp conf/zoo_sample.cfg conf/zoo.cfg
</pre>
下面是zoo.cfg内容：
{% highlight string %}
# The number of milliseconds of each tick
tickTime=2000
# The number of ticks that the initial 
# synchronization phase can take
initLimit=10
# The number of ticks that can pass between 
# sending a request and getting an acknowledgement
syncLimit=5
# the directory where the snapshot is stored.
# do not use /tmp for storage, /tmp here is just 
# example sakes.
dataDir=/opt/oss_kafka/dataDir/zookeeper


# the port at which the clients will connect
clientPort=2181
# the maximum number of client connections.
# increase this if you need to handle more clients
#maxClientCnxns=60
#
# Be sure to read the maintenance section of the 
# administrator guide before turning on autopurge.
#
# http://zookeeper.apache.org/doc/current/zookeeperAdmin.html#sc_maintenance
#
# The number of snapshots to retain in dataDir
#autopurge.snapRetainCount=3
# Purge task interval in hours
# Set to "0" to disable auto purge feature
#autopurge.purgeInterval=1
{% endhighlight %}
上面主要是配置了```dataDir```，下面简要介绍一下里面的几个字段：

* ```tickTime```: 服务器与客户端之间交互的基本时间单元（ms)

* ```dataDir```: 保存zookeeper数据的路径

* ```dataLogDir```: 保存zookeeper日志的路径，如果此配置项不存在时默认路径与dataDir一致

* ```clientPort```: 客户端访问zookeeper时所连接的端口

使用单机模式时需要注意，在这种配置方式下，如果zookeeper服务器出现故障，zookeeper服务将会停止。


5) 启动zookeeper

<pre>
# zkServer.sh start
ZooKeeper JMX enabled by default
Using config: /opt/oss_kafka/pkgDir/zookeeper-3.4.9/bin/../conf/zoo.cfg
Starting zookeeper ... STARTED

# netstat -nlp | grep 2181
tcp6       0      0 :::2181                 :::*                    LISTEN      2906/java

# zkServer.sh status
ZooKeeper JMX enabled by default
Using config: /opt/oss_kafka/pkgDir/zookeeper-3.4.9/bin/../conf/zoo.cfg
Mode: standalone
</pre>


6) 简单测试
<pre>
# zkCli.sh -server localhost:2181
Connecting to localhost
2018-05-16 19:43:10,846 [myid:] - INFO  [main:Environment@100] - Client environment:zookeeper.version=3.4.9-1757313, built on 08/23/2016 06:50 GMT
2018-05-16 19:43:10,850 [myid:] - INFO  [main:Environment@100] - Client environment:host.name=oss-uat-06
2018-05-16 19:43:10,850 [myid:] - INFO  [main:Environment@100] - Client environment:java.version=1.8.0_161
2018-05-16 19:43:10,852 [myid:] - INFO  [main:Environment@100] - Client environment:java.vendor=Oracle Corporation
</pre>
上面登录了zookeeper，然后简单执行如下测试：
{% highlight string %}
[zk: 127.0.0.1:2181(CONNECTED) 1] ls /
[zookeeper]

[zk: 127.0.0.1:2181(CONNECTED) 3] create /zk_test my_data
[zk: 127.0.0.1:2181(CONNECTED) 4] ls /
[zookeeper, zk_test]
[zk: 127.0.0.1:2181(CONNECTED) 6] get /zk_test
my_data
cZxid = 0xa
ctime = Wed May 16 20:04:25 CST 2018
mZxid = 0xa
mtime = Wed May 16 20:04:25 CST 2018
pZxid = 0xa
cversion = 0
dataVersion = 0
aclVersion = 0
ephemeralOwner = 0x0
dataLength = 7
numChildren = 0

[zk: 127.0.0.1:2181(CONNECTED) 6] delete /zk_test
{% endhighlight %}


## 3. 安装kafka

这里我们安装```kafka_2.12-0.10.2.1```版本。采用单机工作模式

### 3.1 下载并安装kafka

1) 下载kafka 

到kafka官方网站```http://kafka.apache.org/```下载相应版本kafka:
<pre>
# wget http://mirrors.hust.edu.cn/apache/kafka/0.10.2.1/kafka_2.12-0.10.2.1.tgz
</pre>

2) 解压并安装

直接解压即可，不需要进行额外的安装操作：
<pre>
# tar -zxvf kafka_2.12-0.10.2.1.tgz
# cd  kafka_2.12-0.10.2.1
# ls
bin  config  libs  LICENSE  NOTICE  site-docs
</pre>


### 3.2 启动kafka

1) 启动kafka服务器

修改kafka配置```config/server.properties```，主要修改如下字段：
{% highlight string %}
# The id of the broker. This must be set to a unique integer for each broker.
broker.id=1

# Switch to enable topic deletion or not, default value is false
delete.topic.enable=true

listeners=PLAINTEXT://10.17.156.73:9092

log.dirs=/opt/dataDir/kafka/logs

zookeeper.connect=localhost:2181
{% endhighlight %}

4) 启动kafka

* 以前台方式启动
<pre>
# bin/kafka-server-start.sh config/server.properties
[2013-04-22 15:01:47,028] INFO Verifying properties (kafka.utils.VerifiableProperties)
[2013-04-22 15:01:47,051] INFO Property socket.send.buffer.bytes is overridden to 1048576 (kafka.utils.VerifiableProperties)

</pre>

* 以后台方式启动
<pre>
# nohup bin/kafka-server-start.sh config/server.properties >/dev/null 2>&1 &
</pre>


### 3.3 kafka简单测试


**1） 创建topic**
<pre>
#  bin/kafka-topics.sh --create --zookeeper localhost:2181 --replication-factor 1 --partitions 1 --topic test
Created topic "test".

//查看所有topic
# bin/kafka-topics.sh --list --zookeeper localhost:2181
test

//查看指定topic
# bin/kafka-topics.sh --zookeeper localhost:2181 --describe --topic test
</pre>


**2） 生产者发送消息**

kafka可以通过使用一个客户端将来自于```标准输入```或者```文件```的数据发送到kafka集群中。默认情况下，每一行作为一条单独的消息发送：
<pre>
# bin/kafka-console-producer.sh --broker-list 10.17.156.73:9092 --topic test
This is a message
This is another message
</pre>
这里注意设定的broker-list的IP地址必须为上面kafka配置的IP地址。


**3） 消费者消费消息**

我们从另外一个窗口开启一个客户端来消费消息：
<pre>
# bin/kafka-console-consumer.sh --bootstrap-server 10.17.156.73:9092 --topic test --from-beginning
This is a message
This is another message
</pre>
这里注意设定的broker-list的IP地址必须为上面kafka配置的IP地址。


**4) 查看topic某分区偏移量最大（小）值**
<pre>
# bin/kafka-run-class.sh kafka.tools.GetOffsetShell --topic test  --time -1 --broker-list 10.17.156.73:9092 --partitions 0
test:0:31
</pre>


## 4. kafka如何彻底删除topic及数据

删除kafka topic及其数据，严格来说并不是很难的操作。但是，往往给kafka使用者带来诸多问题。项目组之前接触过多个开发者，发现都会偶然出现无法彻底删除kafka的情况。本文总结多个删除kafka topic的应用场景，总结一套删除kafka topic的标准操作方法。

1） **停止对应topic的produce和consume操作**

如果需要被删除的topic此时正在被程序produce和consume，则这些生产和消费程序需要停止。因为如果有程序正在生产或者消费该topic，则该topic的offset信息一直会在broker更新。调用kafka delete命令则无法删除该topic。同时，需要设置*auto.create.topics.enable = false*(注：默认设置为true)。如果设置为true，则produce或者fetch不存在的topic也会自动创建这个topic，这样会给删除topic带来很多意想不到的问题。

所以，这一步很重要，必须设置*auto.create.topics.enable = false*，并认真把生产和消费程序彻底全部停止。


2） **更改相关配置**

如果kafka配置文件server.properties中没有配置*delete.topic.enable=true*，则调用kafka的delete命令无法真正将topic删除，而是把topic标记为```marked for deletion```。

因此我们需要首先修改配置文件，添加*delete.topic.enable=true*，然后执行如下命令进行重启：
{% highlight string %}
# bin/kafka-server-stop.sh
# ps -ef 
# nohup bin/kafka-server-start.sh config/server.properties >/dev/null 2>&1 &
{% endhighlight %}

3) **调用命令删除topic**

执行命令删除指定的topic，命令格式如下：
{% highlight string %}
# ./bin/kafka-topics --delete --zookeeper <zookeeper server:port> --topic <topic name>
{% endhighlight %}
例如我们这里要删除test这个topic，则：
<pre>
# ./bin/kafka-topics.sh --delete --zookeeper localhost:2181 --topic test
# ./bin/kafka-topics.sh --zookeeper localhost:2181 --list 
</pre>


4) **删除kafka存储目录**

删除kafka存储目录（server.properties文件log.dirs配置，默认为"/data/kafka-logs"）相关topic的数据目录。

注意：如果kafka有多个broker，且每个broker配置了多个数据盘（比如*/data/kafka-logs,/data1/kafka-logs ...*），且topic也有多个分区和replica，则需要对所有broker的所有数据盘进行扫描，删除该topic的所有分区数据。


一般而言，经过上面4步就可以正常删除掉topic和topic的数据。但是，如果经过上面四步，还是无法正常删除topic，则需要对kafka在zookeeer的存储信息进行删除。具体操作如下：

（注意：以下步骤里面，kafka在zk里面的节点信息是采用默认值，如果你的系统修改过kafka在zk里面的节点信息，则需要根据系统的实际情况找到准确位置进行操作）

5) **删除zookeeper中的topic相关信息**

首先找一台部署了zookeeper的服务器，使用如下命令登录zookeeper server:
<pre>
# bin/zkCli.sh -server localhost:2181
#  
</pre>

登录到zk shell之后，找到topic所在目录，执行*ls /brokers/topics*，找到要删除的topic后，然后执行删除命令
<pre>
[zk: localhost:2181(CONNECTED) 0] ls /brokers/topics
[test, __consumer_offsets]
[zk: localhost:2181(CONNECTED) 1] rmr /brokers/topics/test 
</pre>
此时，topic彻底被删除了。

如果某个topic是被标记为*marked for deletion*，可以通过如下命令查看：
<pre>
[zk: localhost:2181(CONNECTED) 2] ls /admin/delete_topics 
[test]
</pre>
此种情况下，我们需要执行如下命令来进行删除：
<pre>
[zk: localhost:2181(CONNECTED) 3] rmr /admin/delete_topics/test
</pre>

```备注:``` 网络上很多其它文章还说明，需要删除topic在zk上面的消费节点记录、配置节点记录，比如
{% highlight string %}
# rmr /consumers/<consumer-group>

# rmr /config/topics/<topic name>
{% endhighlight %}
其实正常情况是不需要进行这两个操作的，如果需要，那都是由于操作不当导致的。比如```步骤1```停止生产和消费程序没有做，```步骤2```没有正确配置。也就是说，正常情况下严格按照```步骤1~5```的步骤，是一定能够正常删除topic的。

6) **重新查看topic相关信息进行验证**

执行完上述步骤后，我们可以执行如下的命令查看topic相关信息：
<pre>
# bin/kafka-topics.sh --zookeeper localhost:2181 --list
</pre>
查看现在kafka的topic信息。正常情况下删除的topic就不会再显示。但是，如果还能够查询到删除的topic，则重启zk和kafka即可。




<br />
<br />

**[参考]**

1. [在CentOS 7上安装Kafka](https://www.cnblogs.com/weifeng1463/p/8873161.html)

2. [Kafka安装之二 在CentOS 7上安装Kafka](https://www.cnblogs.com/wuwei928/p/9025685.html)

3. [CentOS7环境下搭建Kafka](https://blog.csdn.net/houjixin/article/details/70230658)

4. [CentOS 7环境下Kafka的安装和基本使用](https://www.cnblogs.com/Yang2012/p/8078586.html)

5. [zookeeper 安装的三种模式](https://www.cnblogs.com/jxwch/p/6433310.html)

6. [ZooKeeper Getting Started Guide](http://zookeeper.apache.org/doc/current/zookeeperStarted.html)

7. [kafka如何彻底删除topic及数据](https://blog.csdn.net/belalds/article/details/80575751)

8. [kafka系列之指定了一个offset,怎么查找到对应的消息？](https://www.jianshu.com/p/b32ac197aacb)
<br />
<br />
<br />

