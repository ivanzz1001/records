---
layout: post
title: kafka进阶
tags:
- mq
categories: mq
description: kafka进阶
---

前面我们对于kafka的介绍主要还是偏向于使用操作层面，这里作为kafka进阶的第一篇，我们讲述一下kafka若干方面的设计原理。



<!-- more -->

## 1. kafka设计动机
我们设计kafka的主要目的是为了能够有一个统一的平台(platform)来处理各种大规模的实时流数据。为了实现这一目标，我们在设计时可能就得考虑各种用例场景:

* kafka必须拥有高吞吐率，以支持大量的实时流数据，例如日志集

* kafka需要能够优雅的处理大量的日志数据，以支持某一时刻数据从离线系统进行加载

* kafka在处理消息时必须具备低延迟

因此，我们希望创建的kafka能够支持分区(partitioned)、分布式(distributed)、实时(real time)的来处理输入的数据，然后产生新的输出。这就是kafka的```分区模型```(partitioning model)以及```消费模型```(consumer model)的设计由来。

最后，由于输入到kafka中的数据流所产生的输出可能还需要提供给其他系统使用，因此系统必须要能够保证高容错性以应对机器可能出现的宕机情况。


为了实现上述这样一些目标，我们就得考虑方方面面，而不仅仅是在传统消息系统层面加上一个database log。

## 2. 持久化

### 2.1 不要畏惧文件系统
kafka严重依赖文件系统来存储和缓存消息，而我们通常有一个观念会认为“磁盘是低速的”，因此就会怀疑使用磁盘来存储数据到底能不能够达到一个有竞争力的性能指标。实际上，在使用磁盘时主要取决于使用者的期望，如果我们期望很高，则磁盘可能确实是一个很低速的设备，反之则其实磁盘性能也未必有那么差。如果我们适当的设计数据在磁盘上的存储结构，则很可能能够获得与网络(network)相同数量级的速度。

在过去十年，有关磁盘性能的关键事实是，硬盘驱动器的吞吐量与磁盘寻道的延迟时间是不相同的。如果我们在一个JBOD配置，转速为7200rpm的SATA RAID-5磁盘矩阵上进行连续写入的话，每秒钟的写入速度可以达到600MB；而如果我们采用随机写的话，则每秒钟的写入速度可能只有100KB，这两者之间相差了6000倍。在所有的使用模式当中，线性的读、写操作均是最优的，并且得到了操作系统的深度优化。现代操作系统通常都提供了预读(pre-read)技术来处理大块数据读取，也提供了延迟写(write-behind)技术来将将多个逻辑写操作合并成一个物理写操作。


为了弥补磁盘性能方面的不足，现代操作系统开始大量使用内存来为磁盘做缓存。现代OS通常都倾向于使用大量空闲内存空间来作为磁盘缓存，而当内存需要收回时，对性能的损耗也是微乎其微的。磁盘所有的读写操作都会通过操作系统这一统一的缓存来进行。该特性（通过操作系统缓冲读写磁盘文件）通常很难被关闭，除非使用direct IO，因此即使一个进程在进程内维持着自己的一份数据缓存，该数据也会在OS pagecache再缓存一次，这样就可能会导致缓存两次。

更重要的是，因为Kafka是构建在JVM之上的，所有了解Java内存使用的人都应该知道：

* 所占用的内存通常会大大超出Java objects的大小，很多时候会超过其2倍，甚至更多

* 随着堆(heap)中对象数据的增加，java垃圾回收会开始变的低速

正是由于上面这些原因，我们更倾向于使用文件系统和操作系统的pagecache，而不是自己在进程中维持一份缓存———这样至少增加了操作系统可用缓存。因此，在一个32GB内存的系统上就可以有多达28~30GB的空间来作为cache。更为重要的是，即使服务(service)重启，这份缓存可能仍然可以使用，而应用程序内的缓存则需要进行重建。同时，采用操作系统缓存也可以大大简化代码的编写，而不必担心缓存与文件系统之间数据的一致性逻辑。假如我们的读操作大部分都是线性读的话，则预读取(read-ahead)通常可以很好的预先把我们要从磁盘读取的数据先加载到cache中。


这就意味着我们的设计可以非常简单：我们并不需要在应用程序内存中缓存数据，然后在应用程序出现panic时flush到文件系统，取而代之的是我们只需要利用操作系统的pagecache就可以了。所有写到文件系统的持久化日志，我们通常都不会强制刷新到硬盘。实际上这部分写入的数据只是被发送到了操作系统内核的pagecache上。

### 2.2 常数时间读

在消息系统中，通常是每个consumer都各自用一个queue来维护消息的元数据，并将其作为持久化数据结构。该队列通常是一个关联BTree或者其他通用的随机访问数据结构。BTrees是一种多功能数据结构，这使得其能够在消息系统中广泛的支持各种事务(transaction)与非事务(non-transaction)功能实现。尽管BTree操作的时间复杂度为O(logN)，但其还是会有一个相对高的性能消耗。通常O(logN)会被认为是常数时间(constant time)，但对于磁盘操作来说这可能是不成立的。磁盘的寻址时间可能通常需要10ms，并且每一块磁盘并不能并发的进行寻址。因此，即使是一次常规的磁盘寻址延迟就会相当高。在一个存储系统中，通常高速的缓存操作与低速的磁盘操作是混杂在一起的，因此我们所观测到的树型结构的性能可能会随着数据量的增加出现线性的下降。

直观上来说，可以像常用的日志解决方案(logging solutions)那样，队列的持久化也可以做成简单的read和append文件操作。这种数据结构的优点在于所有的操作都可以在O(1)时间内完成，并且读写操作并不会相互阻塞。这具有明显的性能优势，因为其完全不会受到所存数据量的影响————单台服务器使用廉价的、低速的1+TB SATA硬盘就可满足要求。尽管这样的配置可能具有低效的寻址能力，然而对于大规模的数据读写却有可接受的性能，并且使用1/3的价格就可以获得3倍的容量大小。

通过访问虚拟磁盘空间，我们可以获得一个较高的性能，这就意味着kafka消息系统可以提供一些传统消息系统所没有的特性。例如，在kafka中，当消息被消费完之后，其并不会马上被删除，我们可能仍然会将数据保存很长一段时间。这就为消费者带来了很大的灵活性。

## 3. kafka效率
在提高kafka系统效率层面，我们做了很大的努力。kafka的主要用例之一就是处理web的active数据，这种数据通常数据量很大：每一次浏览web页面都可能产生大量的kafka写操作。更为重要的是，我们假设每一条发布的消息都至少会被一个consumer所消费，因此我们还力求整个系统尽量做到廉价。

在构建和运营一些相似的系统的过程中，我们还发现在多租户操作中，效率往往是一个关键因素。假设由于应用程序的某个小故障而使得数据的下行服务很容易成为瓶颈时，则这样的系统很可能会出现问题。这时候就要求kafka能够快速的为下游应用程序分担负载压力。这一点对于为数百个其他应用程序提供中心化服务的系统来说尤其重要。

在上一节我们已经讨论了磁盘的性能问题。一旦消除了不良的磁盘访问模式，这种系统的低效问题通常就只剩两种：大量的small IO操作， 以及过量的字节拷贝。

small IO的问题不仅发生在client与server之间，也发生在server本身的持久化操作上。

为了避免small IO，我们的协议构建于*message set*抽象之上，将消息打包组合在一起。这就允许将消息组合在一起，通过一个网络请求来进行处理。而服务器反过来也可以批量的将消息数据写入到日志中， consumer也可以批量的抓取数据。

这一小的优化大大提高了kafka的整体速度。但是```批量```(batching)会产生更大的网络数据包，更大的顺序磁盘操作，以及占用更多的连续内存块，然而却可以使得kafka将突发的随机消息流转换成线性写操作，然后供consumer来进行消费。

另一个低效的地方在于字节拷贝。在低速收发消息时，这可能不会存在问题，但是在高负荷之下则可能会对性能产生很大的影响。为了避免此问题，我们设计了一种标准的二进制协议，使得producer、broker、consumer均可共享使用此协议（这样在数据簇进行传输时，就可以避免相互之间的转换）。

broker所维护的消息日志其本身就仅仅是文件系统中某个目录下的一些文件，每个文件中保存了一系列的消息，保存的格式与producer、consumer中所用的消息格式一致。维持统一的格式，使得可以在一些重要的操作上进行优化：网络可以直接传输持久化的日志数据。现代Unix操作系统提供了一种高度优化的路径来实现pagecache与socket之间的数据传输，在Linux上可以使用sendfile()零拷贝来进行实现.


为了理解sendfile()的零拷贝，我们首先来看一下常规的文件发送流程:

1) 操作系统从硬盘中将数据读入内核空间的pagecache

2） 应用程序从内核空间将数据读到用户空间

3） 应用程序将数据写回到处于内核空间的socket buffer

4） 操作系统将socket buffer中的数据拷贝到NIC的buffer，然后发送到网络上

从上述过程来看，进行了4次数据拷贝和2次系统调用，这显然是十分低效的。使用sendfile()，可以避免数据的多次拷贝，允许将数据直接从pagecache发送到网络。在这一优化的传输路径中，第一次和最后一次数据拷贝是必要的。

一个常见的用例是，多个consumer同时消费一个topic。使用上面介绍的sendfile()零拷贝技术，数据只会拷贝到pagecache中，且只拷贝一次，多个consumer可以共享使用数据，这样就可以使得消息的消费速率能够达到网络带宽的限制。

通过综合使用pagecache与零拷贝技术，我们可能通常会发现差不多所有的consumer都会在一个相同的消费位置，并且几乎看不到磁盘的读操作，因为数据都是来源于pagecache。

### 2.4 端到端的批量压缩
很多时候可能系统的瓶颈确实不是来自于CPU，也不是来自于磁盘，而是来自于网络带宽。这特别容易发生在数据中心向外部网络发送消息的场景。当然，即使在没有kafka支持的情况下，也可以单独对消息进行压缩，然而这可能会有一个比较低的压缩率。高效的压缩通常需要我们将多条数据组合在一起，形成一个批量再进行压缩。

kafka支持高效的批量格式。批量的消息可以打包在一起进行压缩，然后发送到服务器。批量的消息数据也会直接以压缩的格式写入到日志文件，并且只会被consumer所解压。


kafka目前支持Gzip、Snappy、LZ4以及ZStandard压缩协议。

## 3. Producer

### 3.1 负载均衡
producer是直接向partition的Leader所在的broker发送消息数据的，而不会经过任何中间的路由层。为了帮助producer实现此目的，所有kafka节点都必须要能够响应producer获取元数据的请求———当前哪些服务器处于alive状态，某个topic的所有partitions所对应的leader地址是啥。

客户端控制将消息发送到哪个partition。这通常可以随机发往某个partition，也可以实现某种意义上的分区映射。我们暴露了相应的接口来实现分区映射：允许用户传递一个特殊的key，然后使用某种hash映射方法映射到一个具体的partition上（甚至我们可以重写自己的hash映射函数）。例如，假设我们选择的key是user id的话，则给定用户的所有数据都将会发送到同一个partition上。这反过来允许consumer做一些本地化的消费预测。

### 3.2 异步消息发送
kafka通过批量操作实现了巨大的效率提升，为了实现批量kafka producer将会尝试在内存中累积一定量的数据，然后将其组装成一个大的批量以实现在一个请求中发送。我们可以通过配置的方式来指定一个批量最大的字节数(比如64kb)，或者最长等待多长时间来获得一个批量(比如10ms)。这样就能够尽量的累积到足够的数据来发送，以及尽量的避免小的IO操作。这样通过引入一定的延迟，从而实现一个较高的吞吐量。



## 4. Consumer
kafka consumer通过向对应partition的Leader发送fetch请求来消费数据。在每一个请求中，consumer都会制定所要抓取的offset，然后会收到从该偏移位置开始的一簇数据。因此，consumer能够完全的控制所要消费的偏移位置。

### 4.1 Push vs. Pull
我们考虑的一个首要问题是：消费者主动从broker拉取数据，还是broker推送数据到consumer。在这一方面，kafka遵循大多数消息系统的设计方式，producer将数据push到broker，consumer从broker拉取数据。一些以日志为中心(logging-centric)的系统，比如Scribe和Apache Flume，这些系统是采用push的方式往下游推送数据的。其实，push或pull这两种方式各有优缺点。然而，基于push系统很难满足各式各样的用户需求，因为是由broker来控制数据的传输速率的。我们的目标通常是consumer能够能够达到最大的消费速率，然而不幸的是，在一个push系统中当consumer的消费速率较低时，大量的push数据就很可能会压垮整个系统。基于pull的系统通常就没有这方面的问题，consumer可以落后于broker，并在适当的时候追赶上broker。我们可以采用某种类型的补偿协议，使得consumer可以预测其是否会被压垮，然而要准确的获得consumer的最大消费速度其实比想象中的更为困难，因此在构建kafka时我们还是采用传统消息系统常用的pull模式。

基于pull模式的系统的另一个优点在于consumer自身可以控制是否批量的将数据发送给自己。而基于push的系统，由于其并不知道下游consumer的立即处理能力，则其必须选择是单条数据发送，还是积累更多的数据来进行发送。假如需要低延迟的话，则会导致每次只向下游推送一条数据，但这明显比较浪费带宽。而基于pull的系统设计就可以弥补这一缺点，因为consumer总是会拉取当前offset之后的可用消息，因此其可以进行批量优化而不会引入不必要的延迟。

基于pull模式系统的一个缺点在于，假如broker没有数据的话，则consumer可能会陷入一个轮询的死循环中，一直处于处于忙等待状态。为了避免这个问题，在pull请求中我们有相应的参数设置，以允许consumer的请求阻塞在长等待(long-poll waiting)状态，直到有新的数据到达。

你也可以设想一下其他端到端(end-to-end)之间只使用pull模式的设计。例如，producer在本地进行写日志操作，然后broker从producer拉取数据，consumer又从broker拉取数据。一种存储转发(store-and-forward)类型的producer就通常是这样实现的。这种设计看起来也还不错，但仔细考虑其实不是很合理，因为在我们的用例中可能会存在成千上万的producer。根据我们大规模运维持久化数据系统的经验，如果一个系统涉及到成千上万的磁盘，则可能会使得整个系统变得十分的脆弱。在实际使用过程中，我们还发现在大规模SLA（service level agreement)应用中，使用pipeline就可以，而不需要producer具有持久化能力。

### 4.2 consumer消费偏移
跟踪哪些消息已经被消费是消息系统的一个关键性能指标。

大多数消息系统都是在broker上保存哪些消息已经被消费的元数据。即当消息发送给consumer时，broker可能会马上在本地进行记录，或者等待conumser的ack信息。这是一个很直观的选择，并且针对单台服务器来说也确实不太可能将消费偏移保存在其他地方。由于在很多消息系统中，存储所采用的数据结构都没有太好的水平扩展性，因此保存在broker也是一个实用的选择———由于broker知道当前已经消费到什么位置了，因此其就可以删除已消费完的数据，确保不会浪费太多的磁盘空间。

这里有一个隐藏的问题就是，对于消费偏移offset，如何使broker与consumer达成一致呢？假如broker在将消息发送到网络上之后，立马就将该消息标识为```consumed```，则当consumer未能成功处理该消息（例如程序崩溃或超时等）时，则该消息就会被丢失。为了解决这一问题，很多消息系统采用ack机制，这就意味着当broker把消息发送出去时，只是将该消息标记为```sent```，而不是```consumed```状态，然后broker等待consumer对该消息的ack，从而将该消息标志为```consumed```状态。通过这一策略，就避免了消息的丢失，但是却产生了新的问题。首先，假如consumer已经成功的处理了该消息，但是还没来的及ack，consumer程序就崩溃了，那么这条消息就会被消费两次；其次，就是性能方面的问题，现在broker必须要跟踪每一条消息的多个状态（sent状态、consumed状态）。对于一个消息系统，我们必须要处理一些疑难的问题，比如消息已经发送出去，但是没有收到ack。

kafka在处理这一问题时，采用一种不同的方法。在kafka中，topic会被分隔成一个全局有序的partition集合，在任何时间每一个partition都只会被同一个consumer group中的一个consumer所消费。这就意味着在每一个partition中，我们仅仅使用一个整数就可以表示该consumer下一次所消费的位置。这就使得消费状态的表示十分轻巧，每一个partition一个整数即可。我们可以周期性的对该状态进行检查，这就等价于对消息进行ack。

采用此种方法来处理消费偏移还带来了另外一个好处，即consumer可以自由的将offset进行重置，从而重新消费原来的数据。这一点与我们常见的消息队列不一致，但对很多conusmers来说可能确实很有用处。例如，假设consumer的代码存在bug，在消费掉一些消息之后该bug被发现，则在bug修正后可以重新对这些消息进行消费。

### 4.3 离线数据加载
大规模存储要求能够支持consumers间断性的消费消息，例如某个时刻hadoop批量的将数据加载到某个离线系统。

在这种情况下，Hadoop会并发的来进行数据加载，其可以将加载任务分隔成单独的map tasks，之后每一个task对应一个topic/partition，这样就可以实现完全并发。Hadoop提供了task管理功能，当任务失败时，其只需要重新启动任务并从原位置加载数据即可。

### 4.4 静态成员关系
静态成员关系(static membership)的目标是为了提高流应用程序的可用性，消费组(consumer groups)以及其他应用程序构建于```组平衡协议```(group reblance protocol)之上的。重平衡协议依赖于组协调器(group cordinator)来为每一个组成员分配一个ID。组协调器所分配的ID只是暂时性的，当组成员重启(restart)并重新加入组时，其ID会发生改变。对基于consumer的应用程序来说，这种动态的成员关系(dynamic membership)，在进行管理操作时（例如重新部署应用程序、更新配置、或者重启），可能会有大批的任务重新指定到一个不同的实例。对于具有大量状态的应用程序来说，这种任务的变动可能需要花费很长的时间来恢复其本地状态，之后才能够正常工作，则就会导致应用程序可能在一段时间内不能向外提供正常服务。正是注意到了此方面的问题，kafka的组管理协议(group management protocol)允许为组成员提供持久化的实体ID(entity ID)。通过基于这些持久化的ID，组成员关系就能够维持不变，这样组不会触发重平衡。

假如你想要使用静态成员关系(static membership):

* 将kafka broker集群及client的版本更新到```2.3```或之后的版本，并确保更新之后的broker所使用的*inter.broker.protocol.version*大于等于```2.3```。

* 对于一个consumer group，将该组内的每一个consumer设置一个唯一的实例ID（通过*ConsumerConfig#GROUP_INSTANCE_ID_CONFIG*配置项来进行设置）

* 对于kafka流应用，最好是针对每一个KafkaStream实例设置一个唯一的ID（通过*ConsumerConfig#GROUP_INSTANCE_ID_CONFIG*配置项来设置）

假如broker的版本低于```2.3```，但是你在client端通过*ConsumerConfig#GROUP_INSTANCE_ID_CONFIG*设置了唯一的ID，则应用程序会侦测到broker的版本，并抛出UnsupportException。另外，假如碰巧两个不同的实例设置了相同的ID，则在broker端会启用相应的规避机制，并通过触发*org.apache.kafka.common.errors.FencedInstanceIdException*来通知客户端马上关闭程序。

## 5. 消息投递机制
现在我们初步了解了producer与consumer是如何工作的了，现在我们来介绍一下kafka是如何处理producer与consumer之间消息的可靠性？很明显，有多种消息投递策略：

* 至多一次(at most once): 消息可能丢失，绝不重新投递

* 至少一次(at least once): 消息不丢失，但可能会被重新投递

* 投递一次(exactly once): 这是大部分人所期望实现的，每一条消息仅仅只会投递一次

值得注意的是，这其实可以分解为两个问题： 消息发布时的可靠性保证，以及消息消费时的可靠性保证

很多系统的设计目标都是提供```exactly once```这样一种消息投递语义，但请仔细阅读相关说明，其实这些目标大部分其实都是误导性的。例如，这些系统不考虑producer或consumer失效情况，也不考虑有多个消费进程的情况，也不考虑写到硬盘上的数据可能会丢失的情况。

kafka所提供的消息投递语义很直接。当publish一条消息时，我们就会认为该消息会被提交(commit)到日志中。一旦消息被提交，并且至少有一个消息副本存活的情况下，消息就不会丢失。我们将在下一节详细介绍```已提交消息```(committed message)、存活的(alive) partition以及该投递机制下能够应对的```失败类型```(failure type)。现在我们假设一种完美的场景，broker不会丢失数据，然后来理解producer与consumer所提供的消息投递保证： 假如一个producer尝试publish一条消息，此时刚好遇到了网络故障，但是并不能确定该故障是发生在消息被commit之前还是之后。

在kafka 0.11.0.0版本之前，假如producer收到消息被提交的响应，则其只能选择对消息进行重发。这相当于提供了```at least once```这样的投递语义，因为在消息重新发送时，有可能这条消息会被重复写入日志。从```kafka 0.11.0.0```版本开始，kafka producer也支持幂等的投递选项，这保证了重复投递的消息不会被重复写入到log中。为了实现此功能，broker为每一个producer都指定了一个ID，并且为每一条发送的消息都加上了序列号，从而避免消息的重复。也是从kafka 0.11.0.0版本开始，producer也支持以```事务语义```（即所有的消息那么全部写入成功，要么全部写入失败）来向多个topic partitions发送消息。

并不是所有的使用场景都需要这种强保证。对于那些对延迟很敏感的应用来说，我们允许producer指定一个特定的可靠性级别。假如producer要等待消息被提交的响应，则这可能需要花费10ms以上。然而，producer也可以被指定为完全异步消息发送，或者只等待leader数据写入成功。

现在我们从consumer的角度来看消息投递的可靠性保证。所有的replicas在相同的offset处都有相同的log，由consumer自己控制其所消费的日志偏移。假如consumer不会崩溃失效，那么其在内存中保存消费偏移即可；但是假如consumer会崩溃失效，并且当失效后我们期望对应的topic partition能够由另一个新的进程来接管，那么新接管的进程则必须选择一个合适的offset来消费数据。kafka consumer读取消息时，我们也有一些选项来处理消息以及更新offset位置：

* consumer可以先读取消息，然后在log中保存消息的偏移位置，之后再开始处理这条消息。在这种情况下，可能会出现consumer刚保存完偏移位置，但还没来的及处理这条消息，系统就出现了崩溃。之后，如果有另外一个进程接管了原崩溃进程的工作，那么其就会从该位置开始处理。在consumer失败的情况下，这对应于```at-most-once```语义，因为其只会对未处理的消息只处理一次。

* consumer可以先读取消息，然后处理消息，之后再保存保存消息的偏移位置。在这种情况下，可能会出现consumer刚处理完消息，但还没来得及保存偏移位置，系统就出现了崩溃。之后，如果有另外的进程接管了原崩溃进程的工作，那么其可能就会对原来某些已处理的过的消息进行重新处理。在consumer失效的情况下，这对应于```at-least-once```语义。在很多情况下，消息都有一个primary key，因此使得相应的更新操作都是幂等的（收到同一条消息两次，仅仅只是用其本身的备份来覆盖自身而已）

那```exactly once```的语义是怎么样呢？当我们从kafka的一个topic消费数据，同时将产生的输出发送到另一个topic，我们可以借用上面提到的kafka 0.11.0.0所引入的事务型producer。consumer的消费offset会存放在一个topic中，因此我们可以在一个事务中进行offset提交、消息publish到output-topic，这锅这样来保证操作的原子性。假如事务被中断，则consumer的消费偏移(offset)也会被撤销还原到旧的值，根据所采用的隔离级别，consumer也看不到新发布到output-topic的数据。在默认的```读未提交```(read-uncommitted)级别中，即使事务部分被中断，consumers也可以看到所有的消息；而```读提交```(read-committed)级别中，consumer只能读取到已提交的消息。

当需要将消息写入到一个外部系统时，主要的限制在于需要协调consumer的消费偏移以及消息的外部存储。经典的做法是在**存储消费偏移**与**存储消费者输出**时引入二段提交，但更简单与通用的做法是让consumer在同一个地方来存储```消费偏移```以及```消费输出```(consume output)。由于consumer所要写入的大部分输出系统可能都不支持二段提交，因此采用后一种方法可能会更好。


在kafka streams中，kafka有效的支持了```exactly-once```消息投递。此外如果需要在kafka topic之间传递并处理数据时，也可以采用事务型producer/consumer来实现```exactly-once```。对于输出目标地址是其他系统的话，```exactly-once```投递机制通常需要相应系统的协作。否则，kafka默认提供的投递机制是```at-least-once```，用户也可以在producer端通过禁用重试机制，从而实现```at-most-once```投递机制。

## 6. Replication
kafka会对topic每一个分区的log进行复制。这就使得kafka集群在出现故障时，可以自动的进行故障转移，从而使得消息仍然保持可用。

其他消息系统也提供了某些副本(replication-related)特性，但是从我们带有```偏见```的角度来看，这似乎只是一个外加的功能，并不会被经常使用，并且具有很大的副作用： replicas处于inactive状态，严重影响整个系统的吞吐量，并且需要繁杂的人工配置。而kafka默认就是搭配replication一起使用———事实上，将副本因子设置为1的话，也就相当于实现了un-replicated topic。

kafka是以topic partition作为副本复制单元的。在不考虑系统失效的情况下，kafka的每一个分区都有一个leader，以及零个或多个followers。总的副本数（包括leader)就是复制因子(replication factor)。kafka的所有读写操作都是通过leader分区来完成的。通常，分区数会远远超过broker数，因此可以基本保证leader均匀的分布在每一个broker上。处于followers上的log与leader的log保持一致————具有相同的offset以及消息记录（当然，可能会在一个很短的时间内，leader上的数据未复制到副本）。

Followers会像普通的kafka consumer那样从Leaders消费数据，并且消费的数据写入到它们自身的log里。followers从leader拉取数据，然后可以将这些数据打包在一起，批量的写入到日志。

大部分能够自动处理失败故障的分布式系统都要求能够精确定义一个节点```alive```的状态。对于kafka来说，一个节点处于```存活```(liveness)状态必须要满足如下两个条件：

1） 节点必须要能够和zookeeper之间维持会话（通过zookeeper的heartbeat机制）

2） 假如节点是follower的话，则必须要能够复制leader的写操作，并且不会落后太多

我们把满足则两个条件的节点称为```in sync```节点，而不含糊的称为```alive```或者```failed```。Leader会保持对```in sync```节点的跟踪。假如一个follower死亡(dies)、卡住(stuck)、或者落后太多，则leader会将其从ISR列表中移除。判断副本节点有没有被卡住或者滞后，是通过*replica.lag.time.max.ms*配置项来决定的。

在分布式术语中，我们只会尝试处理这样一种```fail/recover```模型：节点突然失效，然后又恢复工作。kafka并不会处理**拜占庭**故障，即节点产生随机恶意的应答信息。


现在我们就可以更精确的定义消息的```committed```状态：消息成功写入到了一个partition中所有ISR节点的日志里。只有被提交的日志才会被consumer所消费，这就意味着consumer不必担心会遇到由于leader失效而导致消息丢失的情况。另一方面，producer可以根据他们的偏好（延迟性/可靠性）来选择是否等待消息被提交的响应。偏好(reference)是由producer的ACK设置来控制的。值的注意的是，但producer获取消息提交的响应时，会根据该topic所设置的最低ISR数来进行检查。假如producer发送的请求并不要求严格ack的话，则即使ISR数低于所设置的最低值时，消息还是可以被提交并且被消费者所消费。


在任何时刻，有至少一个副本处于alive状态，那么kafka就能够保证已经被提交的消息不会丢失。


在短暂的故障转移期过后，如果节点出现故障，kafka仍将保持可用，但在出现网络分区时将变的不可用。


### 6. 副本日志：多数派、ISR和状态机
kafka分区的核心就是replicated log。在分布式数据系统中，副本日志是最基本的原语，并有许多方法来实现一个副本日志系统。replicated log可以作为实现其他分布式系统的基础，将其作为副本日志的一个状态机(state machine)。

副本日志模型主要用于处理*在一个顺序序列上达成共识*（通常会对log entry进行编码0、1、2...)。有很多方法来实现，但最简单、最快速的方式是通过一个Leader来确定好所有值的顺序。只要leader存活，那么所有followers就只需要从leader拷贝相应的值序列。

当然，在理想情况下，假如leader永远不会失效的话，那么当然可以不需要followers。而当leader死亡，我们就需要从followers中选举出一个新的leader。但是followers自身可能也落后于原来的leader，或者可能也出现了崩溃，因此我们必须要确保能够选举出一个最新的follower。一个日志复制算法(log replication algorithm)核心就是需要保证： 假如向client端反馈消息已经committed，那么即使leader崩溃，新选举出来的leader也必须要保证这条消息不会丢失。这就需要作出一个权衡：假如leader需要等待从更多followers那里获得ack来确定一条消息已经committed，那么为了系统具有更高的吞吐率，则我们可能需要选举出更多的leader（对于kafka，就需要设置更多的分区）。


对于副本日志(replicated log)系统，*消息提交所需要获取的ack数量*与*leader选举所需要比较的logs数量*之间需要确保有重叠(注：这就可以保证在每一次leader选举时，都会有一个最权威的broker保存有所有的日记记录，从而确保不会丢失消息记录)，这被称为Quorum。

一种常见的实现方式是使用多数派(majority)投票来决定leader选举，以及消息提交(commit)。尽管目前kafka不是这样做的，但这里我们还是来探讨一下这种实现方式。例如： 当前我们有```2f+1```个replicas。假设leader收到```f+1```个replicas的ack响应才会认为消息提交成功，又假设我们需要从至少```f+1```个replicas里面选举出拥有最新log日志的节点成为新的leader，那么在失效节点数小于等于```f```的情况下，就能够保证leader拥有完整的消息提交日志。因为在任何```f+1```的replicas集中，至少有一个replica拥有完整的消息提交日志，那么这个拥有最完整消息提交日志的replica就会被选举成为新的leader。其实对于每一种日志复制算法(log replication algorithm)，都还有很多需要处理的细节（如怎样更精确保持日志的完备性，在leader失效过程中确保日志数据的一致性，或者更改replica set中的服务器），但这里我们暂时不去探讨。

上面介绍的这种多数派投票实现方式有一个很好的特性： 延迟只取决于最快的服务器。即，假如我们假设副本因子是3，则延迟是取决于较快(faster)的那个follower，而不是较慢的那个。


与此相似类型的算法有很多，包括zookeeper的ZAB算法、Raft算法、Viewstamped Replication算法。而与kafka的实际实现最为接近的来自于微软的[PacificA](http://research.microsoft.com/apps/pubs/default.aspx?id=66814)，我们可以查看相应的教学书籍。


多数派选举（majority vote)不好的一面在于: 如果有多个replicas失败，那么将可能选举不出新的leader。为了能够处理一个replica的失效，我们需要有三份数据拷贝；如果要应对两个replicas的失效，那么就需要5份数据拷贝。在我们的经验中，对于实际的系统如果只用足够的冗余来应对单个故障是不够的，但是却造成每一个写操作都要执行5次，需要5倍的硬盘存储空间，且只有1/5的吞吐量，因此对于大容量数据系统的话这不切实际。这就是为什么quorum算法主要是应用于```集群配置中心```这样的系统，例如zookeeper，而在主数据存储方面使用较少。例如，HDFS的name节点的高可用性是构建于```majority-vote-based journal```，但是但是数据存储本身却没有使用此昂贵的方法。


kafka采用一种略微不同的方式来选择其quorum集。其并没有采用majority vote，而是动态的维持一个ISR集，该集合中的元素保持了与leader的同步。只有此集合中的成员才有资格进行leader选举。往kafka的某个分区写入数据时，需要等到该分区的所有in-sync replicas都写入成功才会被认为消息提交成功。ISR集的每一次更变都会被持久化到zookeeper中。因此，ISR集的任何一个replica都有资格被选为leader。在kafka的使用模型中有很多的partition，这一点对kafka来说是一个很重要的因素，并通过这样来确保leadership的平衡。通过使用ISR模型和```f+1```个replicas，一个kafka topic就可以应对```f```个replicas的失效，并保证提交的数据不会丢失。

对于我们想要处理的大部分使用场景，我们认为此种设计方案是合理的。实际上，为了能够处理```f```个replicas失效，无论是```多数派投票```(majority vote)方法还是ISR方法，在确认一个消息被成功提交前都需要获得相同数量replicas的ack响应（例如， 为了应对一个replica失效，采用多数派投票方法时，至少需要有3个replicas并获得1个ack； 采用ISR方法时，需要2个replicas和1个ack响应）。而```majority vote```方法的优点在于消息的提交不需要依赖于最低速的服务器。然而，我们认为在消息提交时通过client端来选择是否进行阻塞是一种较好的方式，并且在应对由于低速的复制所造成的吞吐量及磁盘使用率低这一方面也是值得的。


另一个不同之处在于，kafka并不需要崩溃的节点恢复所有的数据。对于复制算法(replication algorithm)来说，这是很常见的，因为通常的硬盘存储在数据没有招到破坏的情况下并不会丢失。在这种假设下会有两个主要的问题。首先，在我们实际运维的数据存储系统中，硬盘错误是最常见的问题，并且数据的完整性可能会遭到破坏； 其次，即使我们不考虑此问题，我们也不想在每次执行写操作时都调用fsync来确保数据写入到硬盘，因为这可能会造成系统的速度有2到3倍的下降。kafka所使用的协议允许一个replica重新加入ISR，但是在重新加入之前必须完成与leader的重新同步。

### 6.1 Unclean leader election
值得注意的是，kafka保证数据不会丢失是基于*至少有一个replica处于同步状态*。假如一个分区的所有副本都失效了，则这个保证将不复存在，可能会造成数据的丢失。

然而，对于一个实际的系统，当所有的replicas都失效时，我们还是需要做一些事情以应对这种情况。假如真的不幸出现了所有副本都失效(die)，很重要的一点是我们需要考虑会产生什么后果。有两种不同应对策略：

1） 等待ISR中的一个replica复活，并且将其选为leader(幸运的是，该replica保存有所有的数据）

2） 选择第一个复活的replica(不需要一定在ISR中)作为leader


上面的两种策略，其实就是在可用性(availability)与一致性(consistency)做一个简单的取舍。假如我们需要等待ISR中的replicas，则当ISR中的replicas都失效时，则会导致系统处于不可用状态。如果这些replicas被销毁，或者数据已经丢失，那么整个系统都将再也恢复不过来。另一方面，假如一个处于```非同步状态```(non-in-sync)的replica复活，并且我们允许将其选为Leader，那么会以该replica的日志作为事实标准，尽管这样可能会丢失一些数据。从```0.11.0.0```版本开始，kafka默认选择第一种策略，然后等待一个一致性的副本复活。我们可以使用配置选项*unclean.leader.election.enable*来改变这种策略，用于支持*数据可用性*高于*数据一致性*的场景。


其实并不是kafka会遇到这样的问题，所有```quorum-based```模式的系统都存在。例如在```majority voting```模式的系统中，假如大部分(majority)服务器都永久失效了，则你必须进行选择：100%丢失数据；或者不考虑数据的一致性，而从剩下的服务器中恢复数据。

### 6.2 可用性与可靠性保证
当在写kafka的时候，producer可以选择是否等待该消息的ack：

* 0： 不等待ack

* 1： 等待leader的ack

* -1: 等待该分区所有in-sync replicas的ack

值的注意的是，*acknowledgement by all replicas*并不保证获得所有收到message的replicas的响应。默认情况下，当设置```ack=all```时，收到所有in-sync replicas的ack即可。例如，一个topic被配置为2副本，则当1个失效时(仍剩余一个处于in-sync状态)，则当指定```ack=all```时，仍可以成功的进行写入。然而，假如剩余的一个replica也失效的话，那么所写的消息数据则可能会丢失。尽管这保证了partition的最大可用性，但是对某些要求具有高可靠性的用户来说，可能并不能接受。因此，我们提供了两个topic级别的配置项，用于支持可靠性(durability)高于可用性(availability)的场景：

1） 禁用*unclean leader election*(通过unclean.leader.election.enable配置项)———假如所有的replicas都不可用，则分区会保持不可用状态，直到重新选出新的可用的leader为止。这降低了可用性，但是可以保证数据不会丢失。关于unclean leader选举，请参看上一节的介绍。

2） 指定最小的ISR值———只有当partition的ISR数大于所指定的最小值时，消息才能够写入成功，这样就确保了消息不会只写入到了单个replica，从而避免在单个replica失效时，造成的数据丢失。但请注意，此设置只有在producer使用```ack=all```时，才会保证消息至少要收到指定数量的*in-sync replicas*响应。此设置使得可以在可用性(availability)与可靠性(durability)之间做一个取舍。我们将最小ISR值设置的越大，就越能够更好的保证数据的一致性，因为消息记录会被写入到越多的replicas中，从而降低数据丢失的可能性。然而，这降低了系统的可用性，因为假如in-sync replicas数量低于所设定的阈值时，相应的partition将变得不可用。

### 6.3 replication管理
在上面关于副本日志(replicated logs)的讨论中，都只涉及到单个log，例如，一个topic partition。然而，kafka集群很可能管理着成千上万的partitions。在一个Kafka集群中，我们使用流行的round-robin方式来尝试平衡各个分区(partition)，从而避免一些高容量(high-volumn)topic只集中于少数几个节点上。相似的，我们也会尝试平衡各leader，从而使得每个节点(node)都会是某些分区的leader。

同时，优化leader选举过程也是很重要的，因为这是系统不可用的一个关键窗口(window)。一个简单的leader选举算法，在单个节点失效(node)的情况下，很可能会造成与该节点相关的所有分区(partition)都需要进行重新的leader选举。相反，我们选择其中一个broker作为```controller```。该controller负责侦测broker级别的故障，并且在侦测到broker失效时，负责修改所有受影响的分区的leader。这就使得我们可以批量的来处理leadership修改的通知，从而使得大量的partion选举会变得更简单与高效。假如controller失效的话，则会从剩余的broker当中选举出一个新的controller。

## 7. 日志压缩
关于日志压缩，请参看*kafka日志留存策略*相关文章，这里不做介绍。

## 8. kafka实现细节
本节会讲述kafka的一些实现细节。

### 8.1 网络层

网络层就是一个直接的NIO服务器，在这里并不会做很详细的介绍。sendfile的实现是通过MessageSet接口的writeTo方法。这允许文件存储的消息集可以使用更加高效的transferTo实现，而不是使用进程间的缓冲写。所采用的线程模型是：单个acceptor线程 + N个处理器线程，每个处理器线程处理一定数量的tcp连接。这种设计模型在其他很多地方都被验证过，其足够的简单和高效。协议的设计也足够简单，以便于未来通过其他编程语言来实现客户端(clients).

### 8.2 Messages
kafka消息是由如下3部分组成：

1） 可变长度的header

2) 可变长度的key部分

3） 可变长度的value部分

如下图所示：

![kafka-msg-fmt](https://ivanzz1001.github.io/records/assets/img/mq/kafka-msg-fmt.jpg)

关于header，我们会在下一节进行介绍。对于key和value部分，我们设置为对用户透明： 当前有很大部分的处理都是通过序列化库来完成的，我们很难说针对所有的使用场景都选择某一特定的序列化库。尚且有一些使用kafka的特定应用，用户很可能会想要使用某种特定的序列化方法。RecordBatch接口是一个简单的迭代消息的方法，用于批量的读写NIO Channel。

### 8.3 消息格式
消息记录(messages)总是以批量的方式来进行写操作。这里我们将一批消息称为```record batch```。一个record batch包含一条或多条消息记录。在少数情况下，一个record batch只包含一条消息记录。这里需要指出的是，```record batch```与```record```均各自含有自己的headers。下面我们会进行详细说明。

###### 8.3.1 Record Batch
如下是RecordBatch存储到硬盘上时的格式：
<pre>
baseOffset: int64
batchLength: int32
partitionLeaderEpoch: int32
magic: int8 (current magic value is 2)
crc: int32
attributes: int16
    bit 0~2:
        0: no compression
        1: gzip
        2: snappy
        3: lz4
        4: zstd
    bit 3: timestampType
    bit 4: isTransactional (0 means not transactional)
    bit 5: isControlBatch (0 means not a control batch)
    bit 6~15: unused
lastOffsetDelta: int32
firstTimestamp: int64
maxTimestamp: int64
producerId: int64
producerEpoch: int16
baseSequence: int32
records: [Record]
</pre>
值得指出的是，假如启用了压缩(compression)，则被压缩的消息数据就会直接序列化，放在```消息记录数```(records count)的后面。

crc校验覆盖了从```attributes```到整个record batch结尾字节数据。其是通过```magic```字节来进行隔离的，这就意味着客户端必须要首先解析到magic字段，然后在知道如何解释后面的数据。*partition leader epoch*字段并不会参与到CRC的计算中，这样就可以避免broker每次收到batch数据时都重新计算crc。这里采用```CRC-32C```算法来计算crc。

在进行压缩时，新版本的消息格式与老版本有所不同，在magic v2及以上版本中，当log被清理的时候，baseOffset与baseSequence这两个字段仍然会保留。这在log重新加载以恢复producer的状态时是有必要的。假如我们没有在日志中保存```last sequence number```的话，则会在某些情况下出现故障，例如当某个partition 的leader失效时，producer可能会看到*OutOfSequence*这样的错误。*base sequence number*这一字段必须被保留，以进行重复性检查(broker会检查所进入的producer请求，看是否会有数据重复）。


###### 8.3.2 Record
Record级别的头(header)是从kafka 0.11.0版本引入的，在存入硬盘时其格式如下：
<pre>
length: varint
attributes: int8
    bit 0~7: unused
timestampDelta: varint
offsetDelta: varint
keyLength: varint
key: byte[]
valueLen: varint
value: byte[]
Headers => [Header]
</pre>

1) **Record Header**

record header格式如下：
<pre>
headerKeyLength: varint
headerKey: String
headerValueLength: varint
Value: byte[]
</pre>

### 8.4 Log
一个具有两个partitions的topic(名称为```my_topic```)是由两个目录所组成（名称分别是*my_topic_0*和*my_topic_1*)，在对应的目录中存放着相应的消息记录。日志文件的格式是一系列的```log entries```，每一个log entry是由一个**4字节的整数**和**N字节的消息**所组成，其中4字节的整数N表示消息的长度。每条消息都由一个64bit的整数*offset*来唯一标识，用于指明这条消息的第一个字节在该partition所收到的所有消息中的偏移。每一条消息存放到硬盘上的格式会在下面给出。每一个日志文件的名称都是以其所存放的第一条消息的第一个字节的偏移来命名的，因此，所创建的第一个文件名为**00000000000**(11个0），每个日志段的大小是```S```，是由相应配置指定的。

消息记录精确的二进制格式是按不同版本来进行维护的，其作为一个标准的接口（协议)，使得record batches可以在producer、broker、以及client之间进行无缝传输，而不需要进行任何的重新拷贝与转换。在上一节我们对records的详细格式做了较为详细的介绍。


使用message offset来作为message id的情况并不常见。我们原来的想法是使用producer所产生的GUID，然后在每个broker上维持一个GUID到message offset的映射。然而，由于consumer必须为每个broker服务器都维持一个ID，因此就没有必要全局唯一了（只需要保证单个broker唯一就可以）。更为重要的是，要维持一个一个随机的GUID到message offset的映射增加了系统的复杂度，因为这需要一个重量级的索引结构来实现，且该索引结构需要在硬盘之间来进行同步，而这又涉及到对该索引结构进行持久化。因此，为了简化查询结构，我们使用了一个简单的```分区原子计数器```(per-partition atomic count)，通过与partition id和node id一起，从而唯一的标识一条消息； 这就使得整个查询结构变得十分简单。在我们启用```counter```之后，我们就可以直接使用offset了———因为这两个参数(counter和offset)在一个partition内都是单调递增的。由于offset是对consumer API隐藏的，因此具体的实现细节我们不做深入的介绍。

![kafka-log](https://ivanzz1001.github.io/records/assets/img/mq/kafka_log.jpg)

###### 数据写操作
数据总是会被追加到最后一个日志文件中。当文件达到了一个分片大小时，就会开启一个新的文件来开始写。kafka消息日志的写入主要与两个参数相关： ```M```用于指定每多少条消息强制将数据从操作系统刷入硬盘； ```S```用于指定每多长时间（单位： s)进行一次强制刷新操作。这样就保证了即使在系统崩溃的条件下，最多可能会丢失```M```条消息，或者最多会丢失```S```秒内的消息。

###### 数据读操作

我们可以通过指定一个offset和chunck size来读取kafka消息，其中offset是一个64bit的逻辑偏移，chunk size是每一次读取的最大chunk大小。这会返回一个消息迭代器，存放于```S```字节的buffer空间中。这里```S```至少应该超过单个```message```大小，但是在一个异常大message事件里，我们也可以尝试进行多次读操作，每次将buffer空间扩大一倍，直到成功读取完消息为止。我们通过指定message与buffer size的最大长度，这样我们就可以拒绝处理某些超大的messages，然后给client端一个可读取的消息大小的边界。有时候可能会只读取到消息的一部分，但这可以很容易的通过size来进行侦测。


实际从偏移位置读数据时，首先需要定位到日志段文件(数据是存放在一个一个日志段中的），然后再通过全局偏移(global offset)计算出段(segment)内偏移，然后再从该日志段对应的偏移处来读取数据。对数据的查找类似于在内存中做二分查找。

kafka日志提供了获取最新写入消息的功能，这样就使得客户端可以订阅当前的最新消息。这在指定时间内(retention.ms)consumer消费消息失败的情形下是很有用处的。在这种情况下，当客户端尝试消费一个不存在的offset处的消息时，会返回*OutOfRangeException*异常，这样我们就可以对offset进行复位或者直接报告相应的错误。

如下是consumer读取消息时返回数据时的格式：
<pre>
MessageSetSend (fetch result)
 
total length     : 4 bytes
error code       : 2 bytes
message 1        : x bytes
...
message n        : x bytes



MultiMessageSetSend (multiFetch result)
 
total length       : 4 bytes
error code         : 2 bytes
messageSetSend 1
...
messageSetSend n
</pre>


###### 数据删除
在进行日志数据删除时，是以日志段(log segment)为单位来进行删除的。允许向日志管理器添加相应的删除策略，以决定哪些文件可以被合法的删除。当前的删除策略是： *删除N天之前的日志* 或 *保留最近N GB的日志*。为了避免在锁定读取(locking reads)的同时仍然进行日志段文件的删除，而造成修改日志段列表的情况，我们使用了copy-on-write(写时复制）技术，从而实现了在删除的同时仍然可以通过日志段快照来进行二分查找。

###### Guarantees

kafka日志部分提供了相应的配置参数```M```，用于控制最多M个消息就要强制刷新一下硬盘。在日志恢复进程开始运行的时候，其就会遍历(iterates)当前最新日志段的所有消息，并校验每条消息的entry是否有效。假如消息的大小(size)以及offset小于日志文件的长度，并且该消息所保存的CRC32与重新计算出来的CRC32相匹配的话，则说明该消息的entry有效。假如检测到一个entry被破坏，那么则会从该位置直接将文件截断。


值得注意的是，有两种类型的崩溃(corruption)必须要进行处理： 

* crash发生时，数据块并未写入，但是inode节点已经更新（可能更新了size信息）

* crash发生时，可能先写入了数据，但是inode节点没有更新

之所以会产生上述说的两种情况，是因为通常操作系统并不能保证*inode节点的更新*与*实际的数据写入*之间的先后关系，因此假如先更新了inode节点，但是数据还没来得及写入，系统就崩溃了，这样就会造成相应的数据错误。我们通过CRC可以侦测到这种错误，然后防止这样的错误造成整个kafka日志段的损坏。

### 8.5 Distribution

###### Consumer Offset跟踪
kafka consumer会跟踪其所消费的每一个partition 消息的最大偏移，并且向kafka提交(commit)相关的offset报告，这样就可以使得consumer在重启之后仍能从对应的offset处开始进行消费。kafka提供了相应的功能选项: 将consumer group的所有消费偏移存储在与该group对应的broker上，我们将该broker称为group coordinator。例如，一个consumer group中的任何一个消费者实例(instance)都应该向对应的group coordinator报告其消费偏移，并能够从中获取到相应的消费偏移记录。consumer group是基于组名(group name)来指定coordinator的。consumer可以通过使用*FindCoordinatorRequest*来查找其所对应的coordinator，将该请求发到任何一个broker，然后通过读取*FindCoordinatorResponse*响应，从而获取到coordinator的详细信息。之后consumer就可以向该coordinator报告或者获取消费偏移信息。在coordinator发生变动的情况下，consumer需要重新查找coordinator。consumer实例可以自动或手动的提交offset。

当group coordinator收到*OffsetCommitRequest*之后，其会将offset追加到一个名为```__consumer_offsets```的topic中，等到所有副本都写入成功之后，broker就会向consumer发送响应。offsets在如果在指定的时间内没有复制保存到副本上，那么*OffsetCommitRequest*将会失败，consumer可以通过重试机制来进行处理。broker会周期性的对offsets这个topic进行压缩，因为通常其维持每个partition的最新偏移即可。同时coordinator也会在内存中缓存offsets，这样就可以更快的获取到相应的偏移记录。

当coordinator收到offset抓取请求的时候，其会简单的从offsets cache中返回上一次提交的offset记录。在coordinator刚刚启动，或者刚成为一组consumer groups的cooordinator(刚成为```__consumer_offsets```这个topic某一个partition的leader)时，则其可能需要从topic中加载offsets到内存中。在这种情况下，fetch将会失败，并返回*CoordinatorLoadInProgressException*异常，consumer可以通过重新发送*OffsetFetchRequest*来进行处理。

###### Zookeeper Directories
如下我们会介绍一下zookeeper在consumers与brokers协调方面所采用的结构与算法。

1） **Notation**

当一个路径的某个元素表示为```[xyz]```时，表示该值并未固定，实际上针对每个```xyz```可能都有一个zookeeper znode节点。例如，*/topics/[topic]*表示一个名称为```/topics```的目录下针对每一个topic都有一个子目录。同时也可以表示数值范围，例如```[0...5]```表示子目录0、1、2、3、4。箭头```->```用于表示某一个znode节点的内容，例如，```/hello->world```表示znode节点*/hello*的值为*world*。

2) **Broker节点注册**

{% highlight string %}
/brokers/ids/[0...N] --> {"jmx_port":...,"timestamp":...,"endpoints":[...],"host":...,"version":...,"port":...} (ephemeral node)
{% endhighlight %}
上面是一个broker节点的列表，其中每一个节点都有一个唯一的逻辑ID，用于向consumer来标识自己的身份。在启动的时候，broker就会在zookeeper的*/brokers/ids*目录下以自身的```broker id```来创建znode。逻辑broker id的主要目的是为了允许将broker移动到另一个物理主机，而不会影响到consumer。如果尝试注册一个已经存在的broker id将会导致错误。

由于broker是采用临时节点(ephemeral nodes)来在zookeeper注册自己的，那么broker被关闭或死亡的情况下，相应的znode节点也会消失。

3) **Broker Topic注册**
{% highlight string %}
/brokers/topics/[topic]/partitions/[0...N]/state --> {"controller_epoch":...,"leader":...,"version":...,"leader_epoch":...,"isr":[...]} (ephemeral node)
{% endhighlight %}
每一个broker都会在其所维护的topic下面注册自己，并且存储该topic的partition信息。

4) **Cluster Id**

cluster id是为kafka集群所指定的一个不变的、唯一的标识符。cluster id最长可以有22个字符，所允许的字符由正则表达式```[a-zA-Z0-9_\-]+```指定，与URL安全base64编码所采用的字符类似。通常，在集群第一次启动的时候会自动产生。

在实现方面，从broker 0.10.1版本(含0.10.1)开始，在集群第一次启动的时候就会自动产生cluster id。broker在启动的时候会尝试从```/cluster/id```目录下获取cluster id。假如该节点不存在，则broker会产生一个新的cluster id并同时创建对应的znode节点。

5) **Broker node注册**

broker节点通常都是独立的，因此其只向zookeeper发布其自身所拥有的信息。当一个broker加入集群的时候，其会在zookeeper brokers目录下注册自己，并在该节点下写入hostname以及port信息。同时broker也会注册已存在的topics列表和topic对应的逻辑分区信息。当一个新的topic在broker上创建时也会动态的进行注册。




<br />
<br />

**[参考]**



1. [kafka官网](https://kafka.apache.org/)

2. [历史性难题——如何为Kafka挑选合适的分区数？](https://www.jianshu.com/p/fa7a65febcc0)


<br />
<br />
<br />

