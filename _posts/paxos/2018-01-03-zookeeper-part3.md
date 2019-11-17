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

* peerEpoch: 每次Leader选举完成之后，都会选出一个新的peerEpoch，用来标记事务请求所属轮次

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

1） **自增logicalclock**

logicalclock就是Notification中的electionEpoch。选举的第一个操作是logicalclock自增，接着更新提议，其实第一次总是提议自己作为Leader。

如果和现实中总统选举做一个类比的话，每次总统选举时都要明确这是第几届选举，logicalclock对应的就是```第几届```(即表示这是该服务器发起的第多少轮投票）。整个选举必须保证处于同一届选举中方有效。

2） **发送选票信息**

这是一个异步操作（由sendNotifications封装），将提议信息放到FastLeaderElection#sendqueue队列中，然后异步的发送给所有其他的zookeeper server(这里值得是所有参与投票的服务器，不会发送给Observer类型的服务器）。

3） **从选票队列中取选票信息**

当前server收到其他服务器的选举回复信息以后，将选票信息放在FastLeaderElection#recvqueue。当服务器循环从此队列中取选票信息时，如果队列中有选票信息就立即返回，如果没有则等待。这里有一个超时时间，如果超过此时间依然没有选票信息，则返回null，这么做可以防止死等。

4） **判断消息是否发送出去**

当从recvqueue没有取得选票信息时，会检查是否已经将提议的leader发送给其他server了，如果queueSendMap(待发送队列）为空，说明已经全部发送出去了； 否则认为没有发送出去，此时会重连其他zookeeper server，以保证链路畅通。

5）**重连其他zookeeper server**

如果链路出现异常，可能会导致提议信息无法发送成功，所以如果queueSendMap中的信息没有全部发送出去，此时会重连其他zookeeper server，以保证zookeeper集群的链路畅通。

6） **LOOKING状态时，electionEpoch比较**

如果收到的选票信息状态为LOOKING，说明对方也在选举中。然后按如下步骤进行处理：
{% highlight string %}
a) electionEpoch比较
进行electionEpoch比较的目的是统一当前是第几届选举。

如果收到选票的electionEpoch更大，那么使用收到的选票的electionEpoch作为“届”，然后清空收到的选票信息，更新提议信息（这里有一个判定过程），重新发送更新后的提议；

如果收到选票的electionEpoch更小，直接忽略此选票；

如果收到选票的electionEpoch和当前相同，那么认为是合法的选票，接着判断是否更新选票。若要更新，则进行更新并重新发送出去，否则则不需要重发（因为没有更新)

b) 接受选票的提议
当且仅当以下三个条件满足其一时，将接受选票的提议，并重新发送选票信息:
n.peerEpoch > self.proposedEpoch
n.peerEpoch == self.proposedEpoch&& n.zxid > self.proposedZxid
n.peerEpoch == self.proposedEpoch&& n.zxid = self.proposedZxid && n.leader > self.proposedLeader

上面n指的是收到的选票，self指的是当前服务器自身的提议。由此可知： peerEpoch、zxid、leader越大。

注： proposedLeader开始的时候一定是当前server的id，但随着选举的进行，会变成上一次提议的leader。
{% endhighlight %}


7） **Leader是否有效**

(接着步骤6，即LOOKING状态下）如果某一个server已经得到半数以上的选票，那么进入Leader是否有效的验证逻辑，具体如下：

无限循环的从recvqueue中取选票，满足一下条件之一时退出循环：

* recvqueue没有选举票（超时时间内一直没有获取到选票）     ----情形1

* 取到一个更新的选票信息（满足“接受选票提议”的条件，则说明提议更新）；   ----情形2

这里其实是一个Leader有效性的校验。依次从recvqueue中取出所有的选票，校验发现所有的选票均满足“接受选票提议”时，说明没有服务器的选票能够推翻之前的结论，所以此时可以认为Leader是有效的。

针对上面```情形1```，即等了一段时间一直没有获取到新的有效选票，那么我们认为当前集群节点已经达成共识了，此时可以直接将本主机设置为LEADING状态或者是FOLLOWING/OBSERVING状态（根据收到的选票来判断，如果选的是自己则设置为LEADING）；针对```情形2```，说明还有一些人在进行投票，此时重复回到步骤6)，继续尽可能多的获得更多选票信息，以期获得更大的共识。

8) **FOLLOWING、LEADING状态时，electionEpoch比较**

a) 选票集合

为了将此部分解释清楚，需要先能清楚选举过程中用到的两个集合：

* recvset: 用来记录选票信息，以方便后续统计
{% highlight string %}
HashMap<Long, Vote> recvset = new HashMap<Long, Vote>();
{% endhighlight %}

* outofelection: 用来记录选举逻辑之外的选票，例如当一个服务器加入zookeeper集群时，因为集群已经存在，不用重新选举，只需要在满足一定条件下加入集群即可。
{% highlight string %}
HashMap<Long, Vote> outofelection = new HashMap<Long, Vote>();
{% endhighlight %}

b) electionEpoch比较

如果收到的选票显示处于FOLLOWING、LEADING状态，说明集群目前有Leader，只需要确保当前服务器和Leader能够正常通信，并收到集群半数以上服务器推荐此Leader时，就直接加入到集群中去。

因为Leader已经存在，所有所有的选票都会加入到outofelection中。如果outofelection有一条选票是来自Leader的，那么就可以认为自己和Leader正常通信；如果outofelection中统计出有超过半数的服务器都推荐了这个Leader，那么毫无疑问，此选票推荐的就是我们的Leader。

c) 源码逻辑

如果logicalclock与n.electionEpoch相同，那么将此选票加入到选票列表中，如果此张选票通过“选票有效性验证”，那么将此选票推举的候选人作为Leader；

因为Leader已经存在，将所有选票放在outofelection中，进行一次“选票有效性验证”，如果通过就可以将此选票推举的候选人作为Leader。

>以上两步的差别是在进行有效性校验时，一个用的是recvset，一个用的是outofelection。从代码上看，zookeeper认为只要electionEpoch相同就认为这是在选举，所以判断选票数目的时候使用的是recvset。
>
>以上两步逻辑比较绕，如果理解起来比较困难，可以参考一下源码。

d) 源码
{% highlight string %}
case FOLLOWING:
case LEADING:
if (n.electionEpoch == logicalclock) {
   //如果Notification的electionEpoch和当前的electionEpoch相同，那么说明在同一轮的选举中，
   recvset.put(n.sid, new Vote(n.leader, n.zxid, n.electionEpoch, n.peerEpoch));
 
   //判定选举是否结束
    if (ooePredicate(recvset,outofelection, n)){
       // 选举结束，设置状态
       self.setPeerState((n.leader == self.getId()) ? ServerState.LEADING : learningState());
 
       Vote endVote = new Vote(n.leader, n.zxid, n.electionEpoch, n.peerEpoch);
       leaveInstance(endVote);
       return endVote;
    }
}
 
outofelection.put(n.sid,
       new Vote(n.version, n.leader, n.zxid, n.electionEpoch, n.peerEpoch, n.state));
// 判定选举是否结束
if(ooePredicate(outofelection, outofelection, n)) {
   synchronized (this) {
       logicalclock = n.electionEpoch;
       self.setPeerState((n.leader == self.getId()) ? ServerState.LEADING : learningState());
    }
   Vote endVote = new Vote(n.leader, n.zxid, n.electionEpoch, n.peerEpoch);
   leaveInstance(endVote);
   return endVote;
}
break;
{% endhighlight %}

e) 选票有效性验证

leader候选人获得超过半数的选票，通过Leader有效性校验。
{% highlight string %}
protected boolean termPredicate(Map<Long, Vote> votes, Vote vote) {
    SyncedLearnerTracker voteSet = new SyncedLearnerTracker();
    voteSet.addQuorumVerifier(self.getQuorumVerifier());
    if (self.getLastSeenQuorumVerifier() != null
            && self.getLastSeenQuorumVerifier().getVersion() > self
                    .getQuorumVerifier().getVersion()) {
        voteSet.addQuorumVerifier(self.getLastSeenQuorumVerifier());
    }

    /*
     * First make the views consistent. Sometimes peers will have different
     * zxids for a server depending on timing.
     */
    for (Map.Entry<Long, Vote> entry : votes.entrySet()) {
        if (vote.equals(entry.getValue())) {
            voteSet.addAck(entry.getKey());
        }
    }

    return voteSet.hasAllQuorums();
}
{% endhighlight %}

f) Leader有效性验证

如果自己不是Leader，那么一定收要到过Leader的信息，即收到Leader信息，并且Leader的回复信息中宣称自己的状态是ServerState.LEADING; 如果自己是Leader，那么当前logicalclock一定要等于选票信息中的electionEpoch。


###### 3.3.4 核心类
以下为选举过程中使用到的核心类:

1) QuorumPeer: 控制整个Leader选举过程


2) FastLeaderElection: 默认的选举算法。此类中还有几个重要的内部类，如下
<pre>
a) FastLeaderElection::Messenger::WorkerReceiver
从QuorumCnxManager::recvQueue中获取网络包，并将其发到FastLeaderElection::recvqueue中

b)  FastLeaderElection::Messenger::WorkerSender
从FastLeaderElection::sendqueue中获取网络包，并将其放到QuorumCnxManager::queueSendMap中，并发送到网络上
</pre>

3) QuorumCnxManager: 实际发生网络交互的地方。QuorumCnxManager保证与每一个zookeeper服务器之间只有一个链接。主要数据结构如下
{% highlight string %}
a) queueSendMap
sid（key） -> buffer queue（value），为每个参与投票的server都保留一个队列。

b) recvQueue
message queue，所有收到的消息都放到recvQueue。

c) listener
server主线程，收发消息时和上面两个队列交互。
{% endhighlight %}


###### 3.3.5 投票流程总结

1） **自增选举轮次**

Zookeeper规定所有有效的投票都必须在同一轮次中。每个服务器在开始新一轮投票时，会先对自己维护的logicalclock进行自增操作。

2) **初始化选票**

每个服务器在广播自己的选票前，会将自己的投票箱清空。该投票箱记录了所收到的选票。例：服务器2投票给服务器3，服务器3投票给服务器1，则服务器1的投票箱为(2, 3), (3, 1), (1, 1)。票箱中只会记录每一投票者的最后一票，如投票者更新自己的选票，则其它服务器收到该新选票后会在自己票箱中更新该服务器的选票。

3) **发送初始化选票**

每个服务器最开始都是通过广播把票投给自己。
{% highlight string %}
synchronized(this){
    logicalclock.incrementAndGet();
    updateProposal(getInitId(), getInitLastLoggedZxid(), getPeerEpoch());
}

LOG.info("New election. My id =  " + self.getId() +
        ", proposed zxid=0x" + Long.toHexString(proposedZxid));
sendNotifications();
{% endhighlight %}

4) **接收外部投票**

服务器会尝试从其它服务器获取投票，并记入自己的投票箱内。如果无法获取任何外部投票，则会确认自己是否与集群中其它服务器保持着有效连接。如果是，则再次发送自己的投票；如果否，则马上与之建立连接。

5) **判断选举轮次**

收到外部投票后，首先会根据投票信息中所包含的logicalclock来进行不同处理:

* 外部投票的logicalclock大于自己的logicalclock，说明该服务器的选举轮次落后于其他服务器的选举轮次，立即清空自己的投票箱并将自己的logicalclock更新为收到的logicalclock，然后再对比自己之前的投票与收到的投票以确定是否需要变更自己的投票，最后再次将自己的投票广播出去。

* 外部投票的logicalclock小于自己的logicalclock，则当前服务器直接忽略该投票，继续处理下一个投票

* 外部投票的logicalclock与自己的logical相等，则进行选票PK（参见如下）

6) **选票PK**

选票PK是基于(self_id, self_zxid)与(vote_id, vote_zxid)的对比:

* 外部投票的logicalclock大于自己的logicalclock，则将自己的logicClock及自己的选票的logicClock变更为收到的logicalclock

* 若logicalclock一致，则对比二者的vote_zxid，若外部投票的vote_zxid比较大（假设收到的选票为n)，则将自己的票中的(vote_myid, vote_zxid)更新为(n.vote_myid, n.vote_zxid)并广播出去。此外还将本次收到的选票```n```及自己更新后的票放入自己的票箱。

* 若二者vote_zxid一致，则比较二者的vote_myid，若外部投票的vote_myid比较大，则将自己的票中的vote_myid更新为收到的票中的vote_myid并广播出去，另外将收到的票及自己更新后的票放入自己的票箱

7) **统计选票**

如果已经确定有过半服务器认可了自己的投票（可能是更新后的投票）则终止投票。否则继续接收其它服务器的投票。

8） **更新服务器状态**

投票终止后，服务器开始更新自身状态。若过半的票投给了自己，则将自己的服务器状态更新为LEADING，否则将自己的状态更新为FOLLOWING

## 4. 几种Leader选举场景

### 4.1 集群启动Leader选举
>注： 图中三元组(logicalclock,proposedLeader, zxid)

1) **初始投票给自己**

集群刚启动时，所有服务器的logicalclock都为1，zxid都为0。各服务器初始化之后，都投票给自己，并将自己的一票存入自己的票箱。如下图所示：

![election](https://ivanzz1001.github.io/records/assets/img/paxos/start_election_1.png)

在上图中，(1, 1, 0)第一位数代表投出该选票的服务器的logicalclock，第二位数代表被推荐的服务器的proposedLeader，第三位代表被推荐的服务器的最大的zxid。由于该步骤中所有选票都投给自己，所以第二位的proposedLeader即是自己的服务器ID(s_id)，第三位的zxid即是自己的zxid。

此时各自的票箱中只有自己投给自己的一票。

2) **更新选票**

服务器收到外部投票后，进行选票PK，相应更新自己的选票并广播出去，并将合适的选票存入自己的票箱，如下图所示。

![election](https://ivanzz1001.github.io/records/assets/img/paxos/start_election_2.png)
```服务器1```收到服务器2的选票（1, 2, 0）和服务器3的选票（1, 3, 0）后，由于所有的logicalclock都相等，所有的zxid都相等，因此根据myid判断应该将自己的选票按照服务器3的选票更新为（1, 3, 0），并将自己的票箱全部清空，再将服务器3的选票与自己的选票存入自己的票箱，接着将自己更新后的选票广播出去。此时服务器1票箱内的选票为(1, 3, 0)，(3, 3, 0)。

同理，服务器2收到服务器3的选票后也将自己的选票更新为（1, 3, 0）并存入票箱然后广播。此时服务器2票箱内的选票为(2, 3, 0)，(3, 3,0)。

服务器3根据上述规则，无须更新选票，自身的票箱内选票仍为（3, 3, 0）。

服务器1与服务器2更新后的选票广播出去后，由于三个服务器最新选票都相同，最后三者的票箱内都包含三张投给服务器3的选票。

3) **根据选票确定角色**

![election](https://ivanzz1001.github.io/records/assets/img/paxos/start_election_3.png)

根据上述选票，三个服务器一致认为此时服务器3应该是Leader。因此服务器1和2都进入FOLLOWING状态，而服务器3进入LEADING状态。之后Leader发起并维护与Follower间的心跳。

### 4.2 Follower重启
1) **Follower重启投票给自己**

Follower重启，或者发生网络分区后找不到Leader，会进入LOOKING状态并发起新的一轮投票。

![follower-restart](https://ivanzz1001.github.io/records/assets/img/paxos/follower_restart_election_1.png)

2) **发现已有Leader后成为Follower**

服务器3收到服务器1的投票后，将自己的状态LEADING以及选票返回给服务器1。服务器2收到服务器1的投票后，将自己的状态FOLLOWING及选票返回给服务器1。此时服务器1知道服务器3是Leader，并且通过服务器2与服务器3的选票可以确定服务器3确实得到了超过半数的选票。因此服务器1进入FOLLOWING状态。

![follower-restart](https://ivanzz1001.github.io/records/assets/img/paxos/follower_restart_election_2.png)
### 4.3 Leader重启

1） **FOLLOWER发起新投票**

Leader(服务器3）宕机后，Follower(服务器1和2）发现Leader不工作了，因此进入LOOKING状态并发起新的一轮投票，并且都将票投给自己。

![leader-restart](https://ivanzz1001.github.io/records/assets/img/paxos/leader_restart_election_1.png)


2) **广播更新选票**

```服务器1```和```服务器2```根据外部投票确定是否需要更新自身的选票。这里有两种情况：

* 服务器1和服务器2的```zxid```相同（例如，在服务器3宕机前，服务器1和服务器2完全与之同步），此时选票的更新主要取决于myid的大小

* 服务器1和服务器2的```zxid```不同。在旧Leader宕机前，其所主导的写操作只需过半服务器确认即可，而不需要所有服务器确认。换句话说，服务器1和服务器2可能一个与旧的Leader同步(即zxid与之相同），另一个不同步（即zxid比之小）。此时选票的更新主要取决于谁的zxid较大

在上图中，服务器1的zxid为11，而服务器2的zxid为10，因此服务器2将自身选票更新为(3,1,11)，如下图所示：

![leader-restart](https://ivanzz1001.github.io/records/assets/img/paxos/leader_restart_election_2.png)


3） **选出新的Leader**

经过上一步选票更新后，服务器1和服务器2均将选票投给服务器1，因此服务器2成为Follower，而服务器1成为新的Leader并维护与服务器2的心跳。

![leader-restart](https://ivanzz1001.github.io/records/assets/img/paxos/leader_restart_election_3.png)

4） **旧Leader恢复后发起选举**

旧的Leader恢复后，进入LOOKING状态并发起新一轮领导选举，并将选票投给自己。此时服务器1会将自己的LEADING状态及选票(3,1,11)返回给服务器3，而服务器2将自己的FOLLOWING状态及选票(3,1,11)返回给服务器3。如下图所示：

![leader-restart](https://ivanzz1001.github.io/records/assets/img/paxos/leader_restart_election_4.png)

5） **旧Leader成为Follower**

服务器3了解到Leader为服务器1，且根据选票了解到服务器1确实得到过半服务器的选票，因此自己进入FOLLOWING状态：

![leader-restart](https://ivanzz1001.github.io/records/assets/img/paxos/leader_restart_election_5.png)

## 5. 一致性保证
ZAB协议保证了在Leader选举的过程中，已经被commit的数据不会丢失，未被commit的数据对客户端不可见。

### 5.1 Commit过的数据不丢失

1) **FailOver前状态**

为了更好地演示Leader Failover过程，本列中共使用了5个Zookeeper服务器。A作为Leader，共收到P1、P2、P3三条消息，并且commit了1和2，且总体顺序为P1、P2、C1、P3、C2。根据顺序性原则，其他Follower收到的消息的顺序肯定与之相同。其中B与A完全同步，C收到P1、P2、C1，D收到P1、P2，E收到P1，如下图所示：

![recovery](https://ivanzz1001.github.io/records/assets/img/paxos/recovery_1.png)

这里要注意：

* 由于A没有C3，意味着收到P3的服务器的总个数不会超过一半，也即包含A在内最多只有两台服务器收到P3。在这里A和B收到P3，其它服务器均未收到P3

* 由于A已写入C1、C2，说明它已经Commit了P1、P2，因此整个集群有超过一半的服务器，即最少三个服务器收到P1、P2。在这里所有服务器都收到了P1，除E外其它服务器也都收到了P2

2） **选出新的Leader**

旧Leader也即A宕机后，其它服务器根据上述FastLeaderElection算法选出B作为新的Leader。C、D和E成为Follower且以B为Leader后，会主动将自己最大的zxid发送给B，B会将Follower的zxid与自身zxid间的所有被Commit过的消息同步给Follower，如下图所示。

![recovery](https://ivanzz1001.github.io/records/assets/img/paxos/recovery_2.png)

在上图中:

* P1和P2都被A Commit，因此B会通过同步保证P1、P2、C1与C2都存在于C、D和E中

* P3由于未被A Commit，同时幸存的所有服务器中P3未存在于大多数据服务器中，因此它不会被同步到其它Follower

3) **通知Follower可对外服务**

同步完数据后，B会向D、C和E发送NEWLEADER命令并等待大多数服务器的ACK（下图中D和E已返回ACK，加上B自身，已经占集群的大多数），然后向所有服务器广播UPTODATE命令。收到该命令后的服务器即可对外提供服务。

![recovery](https://ivanzz1001.github.io/records/assets/img/paxos/recovery_3.png)

### 5.2 未Commit过的消息对客户端不可见
在上例中，P3未被A Commit过，同时因为没有过半的服务器收到P3，因此B也未Commit P3（如果有过半服务器收到P3，即使A未Commit P3，B会主动Commit P3，即C3），所以它不会将P3广播出去。

具体做法是，B在成为Leader后，先判断自身未Commit的消息（本例中即P3）是否存在于大多数服务器中从而决定是否要将其Commit。然后B可得出自身所包含的被Commit过的消息中的最小zxid（记为min_zxid）与最大zxid（记为max_zxid）。C、D和E向B发送自身Commit过的最大消息zxid（记为max_zxid）以及未被Commit过的所有消息（记为zxid_set）。B根据这些信息作出如下操作:

* 如果Follower的max_zxid与Leader的max_zxid相等，说明该Follower与Leader完全同步，无须同步任何数据

* 如果Follower的max_zxid在Leader的(min_zxid，max_zxid)范围内，Leader会通过TRUNC命令通知Follower将其zxid_set中大于Follower的max_zxid（如果有）的所有消息全部删除



上述操作保证了未被Commit过的消息不会被Commit从而对外不可见。

上述例子中Follower上并不存在未被Commit的消息。但可考虑这种情况，如果将上述例子中的服务器数量从五增加到七，服务器F包含P1、P2、C1、P3，服务器G包含P1、P2。此时服务器F、A和B都包含P3，但是因为票数未过半，因此B作为Leader不会Commit P3，而会通过TRUNC命令通知F删除P3。如下图所示。


![recovery](https://ivanzz1001.github.io/records/assets/img/paxos/recovery_4.png)

## 6. 总结

* 由于使用主从复制模式，所有的写操作都要由Leader主导完成，而读操作可通过任意节点完成，因此Zookeeper读性能远好于写性能，更适合读多写少的场景

* 虽然使用主从复制模式，同一时间只有一个Leader，但是Failover机制保证了集群不存在单点失败（SPOF）的问题

* ZAB协议保证了Failover过程中的数据一致性

*服务器收到数据后先写本地文件再进行处理，保证了数据的持久性

<br />
<br />
**参看：**

1. [Zookeeper架构、ZAB协议、选举](https://www.cnblogs.com/fanguangdexiaoyuer/p/10311228.html)

2. [ZAB协议](https://www.cnblogs.com/liuyi6/p/10726338.html)

3. [Zookeeper 原理 之ZAB，选举](https://blog.csdn.net/weixin_40792878/article/details/87475881)

4. [zookeeper官网](https://zookeeper.apache.org/)

5. [ZAB协议恢复模式-leader选举](https://yq.aliyun.com/articles/298075)

<br />
<br />
<br />


