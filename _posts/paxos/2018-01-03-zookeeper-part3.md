---
layout: post
title: Zookeeper架构、ZAB协议、选举
tags:
- paxos
categories: paxos
description: Zookeeper之ZAB协议
---


本节我们我们首先会介绍Zookeeper的整体架构，之后再重点介绍一下相关的选举算法。


<!-- more -->

## 1. Zookeeper是什么
Zookeeper是一个分布式协调服务，可用于服务发现、分布式锁、分布式领导选举、配置管理等。

这一切的基础，都是Zookeeper提供了一个类似于Linux文件系统的树形结构（可认为是轻量级的内存文件系统，但只适合存少量信息，完全不适合存储大量文件或者大文件），同时提供了对每个节点的监控与通知机制。

既然是一个文件系统，就不得不提Zookeeper是如何保证数据一致性的。本文将介绍Zookeeper如何保证数据一致性，如何进行Leader选举，以及数据监控、通知机制的语义保证。

## 2. Zookeeper架构
### 2.1 角色
Zookeeper集群是一个基于主从复制的高可用集群，每个服务器承担如下三种角色中的一种：

* Leader： 一个Zookeeper集群同一时间只会有一个实际工作的Leader，它会发起并维护与各Follower及Observer间的心跳。所有的写操作必须要通过Leader完成，再由Leader将写操作广播给其他服务器。

* Follower：一个Zookeeper集群可以同时存在多个Follower，它会响应Leader的心跳。Follower可直接处理并返回客户端的读请求，同时会将写请求转发给Leader处理，并负责在Leader处理写请求时对请求进行投票。

* Observer： 角色与Follower类似，但无投票权

![zoo-arch](https://ivanzz1001.github.io/records/assets/img/paxos/zookeeper_arch.png)

### 2.2 原子广播
为了保证写操作的一致性与可用性，Zookeeper专门设计了一种名为原子广播(ZAB)的支持崩溃恢复的一致性协议。基于该协议，Zookeeper实现了一种主从模式的系统架构来保持集群中各个副本之间的数据一致性。

根据ZAB协议，所有的写操作都必须通过Leader完成，Leader写入本地日志后再复制到所有的Follower节点。

一旦Leader节点无法工作，ZAB协议能够自动从Follower节点重新选出一个合适的替代者，即新的Leader，该过程即为```领导者```选举。在Leader选举过程中，是ZAB协议中最为重要和复杂的过程。

### 2.3 写操作
###### 2.3.1 写Leader
通过Leader进行写操作流程如下图所示

![zoo-write](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_writeleader.png)

由上图可见，通过Leader进行写操作，主要分为5步：

1） 客户端向Leader发起写请求；

2） Leader将写请求以Proposal的形式发给所有Follower并等待ACK；

3） Follower收到Leader的Proposal后返回ACK；

4） Leader得到过半数的ACK(Leader对自己默认有一个ACK）后向所有的Follower和Observer发送Commit

5） Leader将处理结果返回给客户端

这里要注意：

* Leader不需要得到Observer的ACK，即Observer无权投票；

* Leader不需要得到所有Follower的ACK，只要收到过半的ACK即可，同时Leader本身对自己有一个ACK。上图中有4个Follower，只需要其中两个返回ACK即可，因为(2+1)/(4+1) > 1/2;

* Observer虽然无权投票，但仍须同步Leader的数据，从而在处理读请求时可以返回尽可能新的数据

###### 2.3.2 写Follower/Observer
通过Follower/Observer进行写操作流程如下图所示：

![zoo-write](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_writefollower.png)

从上图可见：

* Follower/Observer均可以接受写请求，但不能直接处理，而需要将写请求转发给Leader处理；

* 除了多了一步请求转发，其他流程与直接写Leader无任何区别；

### 2.4 读操作
Leader/Follower/Observer都可直接处理读请求，从本地内存中读取数据并返回给客户端即可。

![zoo-read](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_read.png)

由于处理读请求不需要服务器之间的交互，Follower/Observer越多，整体可处理的读请求量越大，也即读性能越好。



## 3. Zookeeper Leader选举
为了避免理解上的歧义，将投票动作和投票信息区分开，在如下章节中，我将服务器的投票信息称之为```选票```。

### 3.1 基本概念
###### 3.1.1 Notification
Notification其实是选举过程中的通信信息，选举过程主要围绕Notification进行：
{% highlight string %}
static public class Notification {
    /*
     * Format version, introduced in 3.4.6
     */

    public final static int CURRENTVERSION = 0x2;
    int version;

    /*
     * Proposed leader
     */
    long leader;

    /*
     * zxid of the proposed leader
     */
    long zxid;

    /*
     * Epoch
     */
    long electionEpoch;

    /*
     * current state of sender
     */
    QuorumPeer.ServerState state;

    /*
     * Address of sender
     */
    long sid;
    
    QuorumVerifier qv;
    /*
     * epoch of the proposed leader
     */
    long peerEpoch;
}
{% endhighlight %}
选Leader过程中，Zookeeper Server(QuorumPeer)都会根据Notification信息生成Vote(选票信息）。为了方便以下理解，我们不妨将Notification看成每个Zookeeper server的选票信息。下面我们简要介绍一下各字段的含义：

* zxid: 事务ID，事务请求的唯一标记，由Leader服务器负责进行分配。高32位是peerEpoch，低32位是请求的计数，从0开始。

* peerEpoch: 每次Leader选举完成之后，都会选出一个新的peerEpoch，用来标记事务请求所属轮次。

* electionEpoch： 每次Leader选举，electionEpoch就会自增1，统计选票信息时，首先保证electionEpoch相同

* sid: 服务器ID。每个zookeeper服务器，都需要在数据文件夹下创建一个名为```myid```的文件，该文件包含整个Zookeeper集群唯一的ID（整数）。例如，某Zookeeper集群包含三台服务器，hostname分别为zoo1、zoo2和zoo3，其中myid分别为1、2和3，则在配置文件中其ID与hostname必须一一对应，如下所示。在该配置文件中，server.后面的数据即为myid:
<pre>
server.1=zoo1:2888:3888
server.2=zoo2:2888:3888
server.3=zoo3:2888:3888
</pre>

* leader： 所提议的Leader

###### 3.1.2 其他概念
* lastProcessedZxid: 最后一次commit的事务请求的zxid

### 3.2 Zookeeper Server
###### 3.2.1 QuorumPeer
Zookeeper Server启动始于QuorumPeerMain::main()。Zookeeper Server的主要逻辑都在QuorumServer中，此类具体负责的逻辑如下流程图：

![zoo-quorum](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_quorum.png)


本文仅介绍Leader选举流程内容，其他流程（follower流程、leader流程、observer流程）见《ZAB协议恢复模式-数据同步》。

从上面流程图可以看到，QuorumPeer将一直进行，直到running=false。while无限循环中，根据当前zookeeper服务器的投票状态进入不同的业务逻辑。

服务器启动时处于```LOOKING```状态；退出任何子流程以后状态立即被改成```LOOKING```状态。LOOKING状态，表示Zookeeper服务端在进行选举流程。

在集群环境下，任何一台服务器都可能被选中成为Leader，但每台服务器成为Leader的可能性会有所不同，具体为：zxid、peerEpoch、electionEpoch、sid大者更容易被选举为Leader，选举流程部分会详细讲述此中缘由。

###### 3.2.2 服务器状态
* LOOKING: 不确定Leader状态。该状态下的服务器认为当前集群中没有Leader，会发起Leader选举；

* FOLLOWING: 跟随者状态。表明当前服务器角色是Follower，并且它知道Leader是谁

* LEADING： 领导者状态。表明当前服务器角色是Leader，它会维护与Follower间的心跳

* OBSERVING: 观察者状态。表明当前服务器角色是Observer，与Follower唯一的不同在于不参与选举，也不参与集群写操作时的投票；

###### 3.2.3 支持的Leader选举算法
可通过electionAlg配置项设置Zookeeper用于Leader选举的算法。到3.4.10版本为止，可选项有：

* 0---基于UDP的LeaderElection

* 1---基于UDP的FastLeaderElection

* 2---基于UDP和认证的FastLeaderElection

* 3---基于TCP的FastLeaderElection

在3.4.10版本中，默认值是3，也即基于TCP的FastLeaderElection。另外三种算法已经被弃用，并且有计划在之后的版本中将它们彻底删除而不再支持。




### 3.3 Leader选举
###### 3.3.1 选举须知
Leader选举流程比较复杂，在正式进入选举流程之前，需要先弄清楚以下内容：

>每个Zookeeper服务端进入LOOKING状态以后，都会发起选举流程，默认情况下是*快速选举*，所以由FastLeaderElection::lookForLeader()方法承担此责任。

每个Zookeeper服务器接收到选票提议以后，只有两个选择：

* 接受选票提议，认可提议中推荐的服务器作为Leader候选人；

* 不接受选票提议，推荐自己上一次推荐的服务器作为Leader候选人（选举开始总是推荐自己作为候选人，选举中会根据收到的选票信息决定是否更换推荐候选人）

默认情况下，至少超过半数（即n/2+1)服务器投票给同一个Leader候选人时，Leader候选人才有可能被选中为Leader。（这里说的是有可能，还需要进行一些其他逻辑验证）

###### 3.3.2 流程

![zoo-flow](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_flow.png)
以上为代码的完整流程，看起来比较复杂，我们可以按照以下内容简单理解以下。

需要说明的是，此流程结束仅仅是确认那个服务器成为Leader，具体Leader是否能够最终成为Leader，还有另外的流程决定，这部分内容请参看《ZAB协议恢复模式-数据同步》


###### 3.3.3 流程详述
流程比较复杂(QuorumPeer::run())，接下来对流程图中标有数字的地方详细介绍。

1） 自增logicalclock

logicalclock就是Notification中的electionEpoch。选举的第一个操作是logicalclock自增，接着更新提议，其实第一次总是提议自己作为Leader。

如果和现实中总统选举做一个类比的话，每次总统选举时都要明确这是第几届选举，logicalclock对应的就是```第几届```(即表示这是该服务器发起的第多少轮投票）。整个选举必须保证处于同一届选举中方有效。

2） 发送选票信息 

这是一个异步操作（由sendNotifications封装），将提议信息放到FastLeaderElection#sendqueue队列中，然后异步的发送给所有其他的zookeeper server(这里值得是所有参与投票的服务器，不会发送给Observer类型的服务器）。

3） 从选票队列中取选票信息

当前server收到其他服务器的选举回复信息以后，将选票信息放在FastLeaderElection#recvqueue。当服务器循环从此队列中取选票信息时，如果队列中有选票信息就立即返回，如果没有则等待。这里有一个超时时间，如果超过此时间依然没有选票信息，则返回null，这么做可以防止死等。

4） 判断消息是否发送出去

当从recvqueue没有取得选票信息时，会检查是否已经将提议的leader发送给其他server了，如果queueSendMap(待发送队列）为空，说明已经全部发送出去了； 否则认为没有发送出去，此时会重连其他zookeeper server，以保证链路畅通。

5）重连其他zookeeper server

如果链路出现异常，可能会导致提议信息无法发送成功，所以如果queueSendMap中的信息没有全部发送出去，此时会重连其他zookeeper server，以保证zookeeper集群的链路畅通。

6） LOOKING状态时，electionEpoch比较

如果收到的选票信息状态为LOOKING，说明对方也在选举中。









<br />
<br />
**参看：**

1. [Zookeeper架构、ZAB协议、选举](https://www.cnblogs.com/fanguangdexiaoyuer/p/10311228.html)

2. [ZAB协议](https://www.cnblogs.com/liuyi6/p/10726338.html)

3. [Zookeeper 原理 之ZAB，选举](https://blog.csdn.net/weixin_40792878/article/details/87475881)

4. [zookeeper官网](https://zookeeper.apache.org/)

5. [](https://yq.aliyun.com/articles/298075)

<br />
<br />
<br />


