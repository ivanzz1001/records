---
layout: post
title: kafka集群参数配置
tags:
- mq
categories: mq
description: kafka集群参数配置
---

kafka集群可配置参数有很多，大部分都可以采用默认，很多时候我们可能并不需要关心。在这里我们主要讲述一下那些常用的参数配置。


<!-- more -->


## 1. kafka参数配置
kafka采用key-value（键值）对的方式来进行参数配置。主要分成如下几大类：

* Broker Configs

* Topic Configs

* Producer Configs

* Consumer Configs

* Kafka Connect Configs

* Kafka Streams Configs

* AdminClient Configs

下面我们对每类中常用的一些参数配置做一个简单介绍。（注： 下文讲述的配置是以kafka 2.4.0版本来进行说明的）

### 1.1 Broker Configs

对于一个broker，如下3个参数是必须要进行配置的：

* broker.id

* log.dirs

* zookeeper.connect

对于topic级别的配置，我们会在下一节进行讲述。

1） **zookeeper.connect**

zookeeper在kafka中充当分布式协调框架的作用，负责协调管理和保存kafka集群的所有Meta数据，比如集群有哪些broker在运行，有哪些topic，以及每个topic有哪些partition，partition中leader replica在哪些机器上。

本配置项用于指定broker所连接的kafka集群，配置格式为*hostname:port*。为实现容错性，通常zookeeper是以集群的方式运行的，因此我们也可以配置多个zookeeper地址，地址与地址之间以```逗号```分割，例如：
<pre>
zookeeper.connect=hostname1:port1,hostname2:port2,hostname3:port3
</pre>

通过上面的方式，默认kafka会将集群的相关信息存放到```/```目录下，而如果我们想更改到一个我们自己设定的目录的话，我们可以采用如下方式：
<pre>
zookeeper.connect=hostname1:port1,hostname2:port2,hostname3:port3/chroot/path
</pre>
这样，我们可以实现的一个功能是： 假如我们有两套不同的kafka集群共用同一个zookeeper集群，这样我们就可以通过不同的根目录来存放相应的信息，而不会产生冲突。

2) **advertised.listeners**

kafka会将本参数所配置的地址发布到zookeeper，以使其他客户端可以使用该发布的地址。假如不配置本参数的话，则默认与*listeners*配置参数相同。（通常我们不需要配置本参数）

3） **listeners**

监听器，即外部连接者通过*protocol://hostname:port*访问开放的kafka服务，这里hostname最好是主机名，而不是IP地址。本参数用于指明kafka的listener列表，列表元素之间以```逗号```分割。如果我们所指定的listener name并不是一个安全协议的话，我们还需要通过*listener.security.protocol.map*来配置所采用的协议。如果我们将hostname配置为```0.0.0.0```，表示绑定所有网卡；如果我们将hostname配置为空，则表示绑定到默认的网卡。例如：
<pre>
listeners=PLAINTEXT://myhost:9092,SSL://:9091 CLIENT://0.0.0.0:9092,REPLICATION://localhost:9093
</pre>


4） **auto.create.topics.enable**

本参数用于控制是否允许在kafka上自动创建topic。通常在我们consume(消费)或者produce(生产）消息时，如果对应的topic不存在，则会自动创建该topic。但这样可能造成topic名混乱，因此我们可以将此配置参数设置为false，以禁止自动创建。

5) **background.threads**

用于指明有多少个线程来处理```后台任务```

6） **broker.id**

用于指明本broker的ID。假如未设置的话，则会自动产生一个唯一的broker id。为了避免zookeeper自动产生的broker id与我们设置的broker id相冲突，zookeeper会从*reserved.broker.max.id+1*开始产生。

7） **delete.topic.enable**

用于配置是否允许删除topic。如果是通过admin tool来删除topic的话，通常不受本配置项的影响。

8） **log.dirs**

用于指明kafka的message信息存储在什么位置，可以设置多个路径，比如：/home/kafka1,/home/kafka2,/home/kafka3。最好这些dirs是mount到不同物理磁盘上。好处是：提升读写性能、故障转移，防止磁盘坏了导致broker进程关闭，相当于简约RAID方案

9) **log.retention.bytes**

Kafka会定期为那些超过磁盘空间阈值的topic进行日志段的删除。这个阈值由broker端参数*log.retention.bytes*和topic级别参数*retention.bytes*控制，默认是-1，表示Kafka当前未开启这个留存机制，即不管topic日志量涨到多少，Kafka都不视其为“超过阈值”。如果用户要开启这种留存机制，必须显式设置log.retention.bytes（或retention.bytes）。

一旦用户设置了阈值，那么Kafka就会在定时任务中尝试比较当前日志量```总大小```是否超过阈值至少一个日志段的大小(参看*log.segment.bytes*)。这里所说的总大小是指所有日志段文件的大小，不包括索引文件的大小。如果是则会尝试从最老的日志段文件开始删起。注意这里的“超过阈值至少一个日志段的大小”，这就是说超过阈值的部分必须要大于一个日志段的大小，否则不会进行删除的，原因就是因为删除的标的是日志段文件——即文件只能被当做一个整体进行删除，无法删除部分内容。

举个例子来说明，假设日志段大小是700MB，当前分区共有4个日志段文件，大小分别是700MB，700MB，700MB和1234B——显然1234B那个文件就是active日志段。此时该分区总的日志大小是3*700MB+1234B=2100MB+1234B，如果阈值设置为2000MB，那么超出阈值的部分就是100MB+1234B，小于日志段大小700MB，故Kafka不会执行任何删除操作，即使总大小已经超过了阈值；反之如果阈值设置为1400MB，那么超过阈值的部分就是700MB+1234B > 700MB，此时Kafka会删除最老的那个日志段文件。

注：“上面我们说超过阈值至少一个日志段大小，会对日志进行删除”，其实准确来说是会对日志按“日志留存策略”进行处理。kafka目前支持两种日志留存策略：delete与compact(可以通过log.cleanup.policy来指定)

如果将此配置项设置为-1，表示不对日志进行控制。这是从```空间维度```来对日志进行管理的。

10) **log.retention.hours**

kafka日志段的最长保留时间。通常结合log.retention.minutes、log.retention.ms一起使用。这是从```时间维度```来对日志进行管理的。


11） **log.roll.hours**

基于时间的日志分割。即使文件没有到达log.segment.bytes，只要文件创建时间到达此属性，就会创建新文件。这个设置也可以有topic层面的设置进行覆盖；

12) **log.segment.bytes**

用于指定每个日志段的大小。

13） **message.max.bytes**

kafka批量处理消息时，每一个批量所允许的最大字节数。

14） **unclean.leader.election.enable**

此配置项用于指定是否允许落后太多的replica竞选Leader，如果允许的话可能会发生信息丢失。默认值为```false```。

15） **log.cleanup.policy**

用于指定在达到一个留存窗口(retention window)后，对日志段的留存策略。当前所支持的留存策略有```delete```、```compact```。

16） **num.partitions**

用于指定每一个topic的默认分区数，默认值为1.

17） **replica.lag.time.max.ms**

在一个指定的时间之内，如果follower没有发送任何fetch请求，或者没有同步上Leader，那么leader就会将该follower从ISR中移除。默认值是10秒


###### 实践经验

生产上如果有消息丢失的情况，可以先检查一下如下的一些参数是否设置合理：

1） unclean.leader.election.enable = false

2） replication.factor >=3：将消息多冗余几份

3） min.insync.replicas >= 2，消息至少要被写入2个副本才算是```已提交```,1个leader副本，至少1个follower副本

4) retries > 0，producer端参数，防止网络抖动，自动重试消息发送，在网络不稳定的时候可以考虑。acks = all，表示所有副本broker全部接收到消息才算```已提交```。请参看[Producer Configs](https://kafka.apache.org/documentation/#brokerconfigs)相关配置段。

另外，producer.send(msg,callback), callback可以知道消息到底发送成功了没有。

5） enable.auto.commit = false，这是consumer端参数，采用手动提交offset。特别是当consumer端是多线程处理时，offset提交了，但是线程处理出错了。


###### 更新Broker Configs
 从Kafka 1.1开始，Broker的```有一些```配置我们可以动态的更新，而并不需要重启Broker。我们可以通过```Broker Configs```的*Dynamic Update Mode*这一列来看是否可以动态更新：

* read-only: 说明如果对应的配置进行了更新，需要通过重启broker才能生效

* per-broker: 只在对应的broker上生效，可以动态的更新

* cluster-wide: 对整个集群有效，可以动态的更新


要更改当前broker(broker id为0）的配置（比如，修改日志清理线程的数量）：
<pre>
# bin/kafka-configs.sh --bootstrap-server localhost:9092 --entity-type brokers --entity-name 0 --alter --add-config log.cleaner.threads=2
</pre>

要查看broker 0的当前broker configs，可以通过如下命令：
<pre>
# bin/kafka-configs.sh --bootstrap-server localhost:9092 --entity-type brokers --entity-name 0 --describe
</pre>

如果要删除broker 0的动态配置，让其恢复默认值或者静态配置文件的配置的值（例如，恢复日志清理线程的数量)，可以执行如下操作：
<pre>
# bin/kafka-configs.sh --bootstrap-server localhost:9092 --entity-type brokers --entity-name 0 --alter --delete-config log.cleaner.threads
</pre>

有一些configs可以配置为整个集群(cluster-wide)生效，这样就可以使得集群中的所有broker都有一样的值。通过如下的命令可以修改整个集群的配置值（例如，修改所有broker的日志清理线程数量）：
<pre>
# bin/kafka-configs.sh --bootstrap-server localhost:9092 --entity-type brokers --entity-default --alter --add-config log.cleaner.threads=2
</pre>

要查看当前整个集群的默认配置值的话，可以执行如下命令：
<pre>
# bin/kafka-configs.sh --bootstrap-server localhost:9092 --entity-type brokers --entity-default --describe
</pre>

所有可以在cluster级别进行的配置，也可以在per-broker级别进行配置。如果在不同的级别(cluster level/broker level)都进行了配置，则按照如下顺序决定所使用的值：

* 存储在zookeeper中的per-broker配置值

* 存储在zookeeper中的cluster-wide配置值

* server.properties静态配置的broker config

* kafka的默认值

### 1.2 topic级别的配置

与topic相关的配置我们可以在broker server层面进行默认设置，也可以在per-topic层面进行设置，per-topic层面的设置会覆盖broker server层面的设置。我们可以在创建topic时，通过使用```--config```选项来进行相应的参数配置。例如，我们要创建一个名为```my-topic```的topic，并设置我们自己想要的*max message size*以及*flush rate*，那么我们可以通过如下命令
<pre>
# bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic my-topic --partitions 1 \
    --replication-factor 1 --config max.message.bytes=64000 --config flush.messages=1
</pre>

我们也可以在后续对相应的配置参数进行修改，例如：
<pre>
# bin/kafka-configs.sh --zookeeper localhost:2181 --entity-type topics --entity-name my-topic
    --alter --add-config max.message.bytes=128000
</pre>

如果要查看当前topic(my-topic)的配置信息，可以执行如下命令：
<pre>
# bin/kafka-configs.sh --zookeeper localhost:2181 --entity-type topics --entity-name my-topic --describe
</pre>

如果要删除对应的参数配置，让其恢复到默认值，我们可以执行如下命令：
<pre>
# bin/kafka-configs.sh --zookeeper localhost:2181  --entity-type topics --entity-name my-topic
    --alter --delete-config max.message.bytes
</pre>

topic参数和broker参数，就好像是局部和全局变量的关系。如果两者都进行了设置，则以topic参数为准。通常情况下，我们可能会对*message.max.bytes*配置参数，针对具体的topic进行单独设置，而其他配置参数进行单独设置的场景并不多。

设置topic级别的参数有2种方式：创建topic 时设置，另一个是修改参数。kafka有kafka-topics命令来创建topic，结尾处使用--config设置topic参数；另一个用kafka-configs来修改参数。


### 1.3 producer configs
如下是producer的一些配置：

1） **acks**

producer写入数据时，要求leader至少需获得多少ack才认为数据写入成功。这控制着数据的持久化，acks所允许设置的值有：

* acks=0: 假如设置为0，则producer不会等待以获得任何ack。消息记录会直接添加到socket缓冲，并且认为发送成功。在此种情况下，我们并不能确保broker已经成功接受到消息记录，并且我们所配置的```retries```参数也不会起任何作用（因为客户端无从感知是否操作失败，因此也就没有必要进行任何retry操作）。消息发送后反馈回来的offset值也一直是```-1```。

* acks=1: 这意味着leader会将相应的消息记录写到本地日志，之后就马上将响应返回给producer，并不会等待follower的写入响应信息。在这种情况下，假如leader写入成功并向producer返回ack信息之后，leader突然崩溃，导致写入的record信息没来得及复制到其他followers上，这样就有可能会造成消息数据的丢失。

* acks=all: 这意味着在producer写入数据时，Leader会等待所有ISR的ack响应，只有所有ISR成员都写入成功，才会向producer反馈写入成功。这样，就可以保证消息记录不会丢失，因为至少有一个follower保持着与Leader的同步。这是最强的```可用性```保证。*acks=all*等价于*acks=-1*。

2） **bootstrap.servers**

producer会使用此配置项所设置的*host/port*列表来连接kafka集群。值得指出的是，producer只是会用此列表所配置的参数来建立与kafka cluster的初始连接，从而获得完整的集群信息，因此并不会真正影响到后续的消息发送等操作。bootstrap.servers的配置格式为：
<pre>
bootstrap.servers=host1:port1,host2:port2,host3:port3...
</pre>

3) **compression.type**

用于指定producer所生产的消息采用哪种压缩方式。默认情况下，不会对消息进行压缩。本配置选项可选值有： none、gzip、snappy、lz4、zstd。

4） **retries**

如果我们设置一个```大于```0的值，则当遇到一个潜在的瞬时错误而导致消息发送失败时，客户端会对消息记录进行重新发送。值得指出的是，这里并不会区分到底是哪种错误（本地网络错误、服务端反馈回来的错误等）导致的消息重发。假如我们没有将*max.in.flight.requests.per.connection*设置为1的情况下允许```retries```的话，则可能会导致消息记录的乱序。比如，我们现在有两条消息都发送到同一个partition，第一条消息发送失败，然后进行重试，但是此时第二条消息发送成功了，那么此时消息就出现了乱序。

假如在重试次数被耗尽之前，我们所设置的*delivery.timeout.ms*就已经超时了，那么此时也会将该produce请求标记为失败。我们通常并不会更改本参数的值，而是使用*delivery.timeout.ms*来控制重试的行为。

### 1.4 Consumer Configs

1) **group.id**

一个唯一的字符串，用于标识本consumer属于哪一个consumer group。假如consumer使用组管理功能（通过subscribe(topic)方式）或者基于kafka的offset管理策略的话，则必须要提供此参数。本参数的默认值为```null```。

2） **allow.auto.create.topics**

当订阅（subscribe)或指定(assign)一个topic时，如果该topic不存在，用于指定是否允许自动创建该topic。当我们订阅(subscribe)一个topic时，假如broker的*auto.create.topics.enable*配置为true时，且所订阅的topic不存在，则会自动创建该topic。

注： 当我们使用的broker版本低于```0.11.0```时，必须要将此配置项设置为false。

3） **auto.offset.reset** 

当在kafka中没有找到initial offset，或者当前的offset所对应的数据在服务器上已经找不到（例如：数据已经删除）时，我们应该采用何种策略，有如下4中选择：

* earliest: 自动的将offset重置到earliest位置

* latest: 自动将offset重置到latest位置

* none: 假如在consumer group中找不到对应的offset的话，则抛出异常

* anything else: 向consumer抛出异常

本参数的默认值为```latest```。

4) **enable.auto.commit**

假如本参数设置为```true```的话，则consumer所消费的offset会周期性的在后台被提交。默认值为true。

5) **partition.assignment.strategy**

当使用consumer group时，通过本参数所指定的策略来决定partition属于哪一个consumer实例所拥有。这里我们可以按照优先级指定一组class names或者class types，只要求所指定的这些class实现*org.apache.kafka.clients.consumer.ConsumerPartitionAssignor*接口即可。


### 1.5 Kafka Connect Configs

暂不介绍

### 1.6 Kafka Streams Configs

暂不介绍

### 1.7 Admin Configs

暂不介绍

### 1.8 补充说明
我们在使用kafka的过程中，有时候可能需要调整JVM参数，这里简单介绍一下。

* kafka_heap_opts： 指定堆大小，默认是1GB，这可能太小了，最好设置大一点。社区推荐是6GB

* kafka_jvm_performance_opts: 指定GC参数，java8默认是新生代垃圾回收器UseParallelGC，可以调整为G1收集器。

具体做法是先设置这两个环境变量，然后再启动kafka broker:
<pre>
# export kafka_heap_opts=--Xms6g  --Xmx6g

# export kafka_jvm_performance_opts= -server -XX:UseG1GC

# kafka-server-start.sh config/server.properties
</pre>


## 2. kafka日志留存机制

关于kafka日志留存（log retention)策略的介绍，网上已有很多文章。不过目前其策略已然发生了一些变化，故本文针对较新版本的kafka做一次同一的讨论。如果没有显示说明，本文一律以```kafka 1.0.0```作为分析对象。

所谓日志留存策略，就是kafka保存topic数据的规则，我将按照以下几个方面分别介绍留存策略：

* 留存策略类型

* 留存机制及其工作原理

### 2.1 留存策略类型

目前，与日志留存方式相关的策略类型主要有两种： delete和compact。这两种留存方式的机制完全不同。本文主要讨论针对delete类型的留存策略。用户可以通过设置broker端参数*log.cleanup.policy*来指定集群上所有topic默认的策略类型。也可以通过topic级别参数*cleanup.policy*来为某些topic设置不同于默认值的策略类型。当前*log.cleanup.policy*的默认值是[delete,compact]，这是一个list类型的参数，表示集群上所有topic会同时开启delete和compact两种留存策略———这是```0.10.1.0```新引入的功能，在```0.10.1.0```之前，该参数只能两选一，不能同时兼顾。但在实际使用中很多用户都抱怨compact类型的topic存在过期key消息未删除的情况，故社区修改了该参数配置，允许一个topic同时开启两种留存策略。

再次强调下，本文只讨论delete类型的留存策略。

### 2.2 留存机制及其工作原理
在开始详细介绍各种留存机制之前，先简要说下kafka是如何处理日志留存的。每个kafka broker启动时，都会在后台开启一个定时任务，定期地去检查并执行所有topic日志留存，这个定时任务触发的时间周期由broker端参数*log.retention.check.interval.ms*控制，默认是5分钟，即每台broker每5分钟都会尝试去检查一下是否有可以删除的日志。因此如果你要缩短这个间隔，只需要调小*log.retention.check.interval.ms*即可。

鉴于日志留存和日志删除实际上是一个问题的两个方面，因而我们下面讨论的是关于kafka根据什么规则来删除日志。但有一点要强调一下，待删除的```标的```是日志段，即LogSegment，也就是以```.log```结尾的一个个文件，而非整个文件夹。另外还有一点也很重要，当前日志段(active logsegment)是永远不会被删除的，不管用户配置了哪种留存机制。

当前留存机制共有3种：

1） 基于空间维度

2） 基于时间维度

3） 基于```起始位移```维度

前两种策略相信大家已经耳熟能详，而第三种策略由于新加入的时间不长，目前网上对其的介绍并不多。下面我们一个一个来看。

###### 2.2.1 基于空间维度

此种留存机制也被称为*size-based retention*，指的是kafka定期为那些超过磁盘空间阈值的topic进行日志段的删除。这个阈值由broker端参数*log.retention.bytes*和topic级别参数*retention.bytes*控制，默认值为```-1```，表示kafka当前未开启这个留存机制，即不管topic日志量涨到多少，kafka都不视其为“超过阈值”。如果用户要开启这种留存机制，必须显式设置*log.retention.bytes*(或*retention.bytes*)。


一旦用户设置了阈值，那么kafka就会在定时任务中尝试比较当前日志量总大小是否超过阈值至少一个日志段的大小。这里所说的总大小是指所有日志段文件的大小，并不包括索引文件的大小。如果是则会尝试从最老的日志段文件开始删起。注意这里的“超过阈值至少一个日志段的大小”，这就是说超过阈值的部分必须要大于一个日志段的大小，否则是不会进行删除的，原因就是因为删除的```标的```是日志段文件———即文件只能被当做一个整体进行删除，无法删除部分内容。

举个例子来说明，假设日志段大小是700MB，当前分区共有4个日志段文件，大小分别是700MB、700MB、700MB和1234B，显然1234B那个文件就是active日志段。此时该分区总的日志大小是:
<pre>
3 * 700MB + 1234B = 2100MB + 1234B
</pre>
如果阈值设置为2000MB，那么超出阈值的部分就是```100MB+1234B```，小于日志段大小700MB，故kafka不会执行任何删除操作，即使总大小已经超过了阈值；反之如果阈值设置为1400MB，那么超过阈值的部分就是```700MB+1234B>700MB```，此时kafka会删除最老的那个日志段文件。 

###### 2.2.2 基于时间维度
也称*time-based retention*，指的是kafka定期为那些超过时间阈值的topic进行日志段删除操作。这个阈值由broker端参数*log.retention.ms*、*log.retention.minutes*、*log.retention.hours*以及topic级别参数*retention.ms*控制。

如果同时设置了*log.retention.ms*、*log.retention.minutes*、*log.retention.hours*，那么以*log.retention.ms*优先级为最高。*log.retention.minutes*次之，*log.retention.hours*最次。当前这三个参数的默认值依次是null、null和168，故kafka为每个topic默认保存7天的日志。

这里需要讨论下这```7天```是如何界定的。在```0.10.0.0```之前，kafka每次检查时都会将当前时间与每个日志段文件的最新修改时间做比较，如果两者的差值超过了上面设定的阈值（比如上面说的7天），那么kafka就会尝试删除该文件。不过这种界定方法是有问题的，因为文件的最新修改时间是可变动的———比如用户在终端通过touch命令查看该日志段文件，或kafka对该文件切分时都可能导致最新修改时间的变化，从而扰乱了该规则的判断。因此自```0.10.0.0```版本起，kafka在消息体中引入了时间戳字段（当然不是单纯为了修复这个问题），并且为每个日志段文件都维护一个最大时间戳字段。通过将当前时间与该最大时间戳字段进行比较来判定是否过期。使用当前最大时间戳字段的好处在于它对用户是透明的，用户在外部无法直接修改它，故不会造成判定上的混乱。

最大时间戳字段的更新机制也很简单，每次日志段写入新消息时，都会尝试更新该字段，因为消息时间戳通常是递增的，故每次写入操作时都会保证最大时间戳字段是会被更新的，而一旦一个日志段写满了被切分之后它就不再接收任何新的消息，其最大时间戳字段的值也将保持不变。倘若该值距离当前时间超过了设定的阈值，那么日志段文件就会被删除。

###### 2.2.3 基于起始位移维度
用户对前两种留存机制实际上是相当熟悉的，下面我们讨论下第三种留存机制：基于日志起始位移(log start offset)。这实际上是```0.11.0.0```版本新增加的功能。起始增加这个功能的初衷主要是为了kafka流处理应用———在流处理应用中存在着大量的中间消息，这些消息可能已经被处理过了，但依然保存在topic日志中，占用了大量的磁盘空间。如果通过设置基于时间维度的机制来删除这些消息就需要用户设置很小的时间阈值，这可能导致这些消息尚未被下游操作算子(operator)处理就被删除；如果设置的过大，则极大的增加了空间占用。故社区在```0.11.0.0```引入了第三种留存机制：基于起始位移。

所谓起始位移，就是指分区日志的当前起始位移———注意它是分区级别的值，而非```日志段```级别。故每个分区都只维护一个起始位移值。该值在初始化时被设置为最老日志段文件的基础位移(base offfset)，随着日志段的不断删除，该值会被更新为当前最老日志段的基础位移。另外kafka还提供了一个脚本命令帮助用户设置指定分区的起始位移：kafka_delete_records.sh。

该留存机制时默认开启的，不需要用户任何配置。kafka会为每个日志段做这样的检查：

1) 获取日志段A的下一个日志段B的基础位移；

2） 如果该值小于分区当前起始位移则删除日志段A。

下面依然拿例子来说明。假设我有一个topic，名称为test，该topic只有一个分区，该分区下有5个日志段文件，分别是*A1.log、A2.log、A3.log、A4.log和A5.log*，其中```A5.log```是active日志段。这5个日志段文件中消息范围分别是0~9999，10000~19999，20000~29999，30000~39999和40000~43210(A5未写满）。如果此时我确信前3个日志段中的消息已经被处理过了，于是想删除这3个日志段，此时我该怎么做呢？由于我无法预知这些日志段文件产生的速度以及被消费的速度，因此不管是基于时间的删除机制还是基于空间的删除机制都是不适用的。此时我们便可以使用*kafka_delete_records.sh*脚本将该分区的起始位移设置为```A4.log```的起始位移，即40000。为了做到这一点，我们需要首先创建一个json文件```a.json```，内容如下：
{% highlight string %}
{"partitions":[{"topic": "test", "partition": 0,"offset": 40000}],"version":1}
{% endhighlight %}

然后执行下列命令：
<pre>
# bin/kafka-delete-records.sh --bootstrap-server localhost:9092 --offset-json-file a.json 
Executing records delete operation

Records delete operation completed:

partition: test-0 low_watermark: 40000
</pre>

如果一切正常，应该可以看到类似于上面这样的输出。此时```test```这个topic的partition 0分区的起始位移被手动调整为40000，那么理论上所有最大消息位移```小于40000```的日志段都可以被删除了。有了这个机制，用户可以实现更为灵活的留存策略。


以上就是当前kafka针对于delete留存类型的topic的3种留存机制。


<br />
<br />

**[参考]**


1. [kafka官网](https://kafka.apache.org/)

2. [kafka configuration](https://kafka.apache.org/documentation/#configuration)

3. [kafka集群参数配置](https://blog.csdn.net/yujianping_123/article/details/96874189)

4. [kafka log.retention.bytes](https://blog.csdn.net/qq_35440040/article/details/93163775)

<br />
<br />
<br />

