---
layout: post
title: 微信PaxosStore内存云揭秘
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


本文转自微信后台团队的《微信PaxosStore内存云揭秘：十亿Paxos/分钟的挑战》。


PaxosStore是微信设计的一套分布式存储系统，并已对核心业务存储做了架构改造。内存云是微信PaxosStore存储体系的组成部分，本文将分享内存云的改造过程。

微信存储QuorumKV是一个分布式的存储系统，覆盖但不限于微信后台核心业务： 账号/用户信息/关系链/朋友圈，等等。

过去的一年，我们受Google MegaStore启发，重新设计了一套全新的分布式存储系统，即PaxosStore分布式存储，并于最近半年对核心业务存储做了架构改造，目前已成功上线并完成了数千台机器的平滑切换，系统首次访问成功率提升一个数量级，并获得更好的容灾能力，可用性显著增强。

内存云作为微信PaxosStore存储体系的组成部分，目前存储着微信基础账号、消息计数等核心用户数据，每天峰值请求高达数十亿/分钟，本文将向大家分享内存云的Paxos改造过程


<!-- more -->

## 1. 背景
微信内存云，目前有2千多台机器： 单机内存64GB，存储盘为机械盘。作为核心存储之一，内存云承载了基础账号、消息计数等核心数据的存储，保障微信登陆、消息收发等微信基础功能。内存云每天峰值请求十多亿/分钟，读写比约为3.3:1。基于QuorumKV的内存云具体架构如下图1所示：

```图1``` QuorumKV架构

![paxos-store](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_arch.png)

微信QuorumKV陪伴内存云走过了数次扩容，经历元旦、春晚等例行节日请求爆发增长的洗礼，也沉淀着不少故障处理经验（不限内存云）。本文主要描述新架构如何根本性的改善QuorumKV的容灾能力（即CAP，保证C的前提下，增强A），在不牺牲性能的前提下，消灭部分故障场景下的最终失败和人为介入。

QuorumKV本质上是一个NWR协议（N为3，W/R为2）的分布式存储，和其他NWR协议不同的地方在于：QuorumKV将NWR应用在版本上，当且仅当版本达成一致的情况下读写单机数据，从而保证强一致性。

>注： NWR是一种在分布式存储系统中用于控制一致性级别的一种策略。在Amazon的Dynamo云存储系统中，就应用NWR来控制一致性
>
>下面我们来看看这3个字母各代表的含义：
>N: 在分布式存储系统中，有多少份备份数据
>W: 代表一次成功的更新操作要求至少有W份数据写入成功
>R: 代表一次成功的读数据操作要求至少有R份数据成功读取

<br />

但是这里引入了两个问题：

* 数据写一份，依靠异步同步至对机；

* 当WR无法形成多数时，单key不可用，需进行修复

不要小看这两个问题，这是目前QuorumKV的绝大部分日常故障导致失败的根源，特别是对写量大的模块而言：

* 数据机故障离线，部分key最新数据在故障机上，不可用；

* 版本机故障离线，部分key（版本不平）仲裁失败，不可用；

```表2```： 分布式协议对比

![paxos-store-cmp](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_cmp.jpeg)

明确问题的根源是QuorumKV采用的NWR分布式协议，那么在保证强一致性的前提下可行的解决方案为Paxos/Raft分布式协议。上面的```表2```列出了我们在协议选择上的一些考量： 采用无租约的方式使得系统在保证强一致性的前提下达到最大可用性；相对而言，租约的方式必然引入主备切换导致的不可用。

在对比调研过一些业界方案（etcd/megastore等），我们最终确定新架构协议方案是： 无租约版Paxos分布式协议（如图3所示）

```图3```： 新架构（无租约版Paxos分布式协议）

![paxos-store-new](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_new.png)


## 2. 面向挑战
接下来要做的事情是，基于paxos分布式协议在机械盘上搭建一套稳定高性能的分布式存储。

### 2.1 挑战1： Paxos分布式协议
我们在谈及Paxos算法时，通常会提及Leslie Lamport大神的Paxos Made simple；但基于paxos算法的分布式协议不止于此，```图4```列出一个完整协议涉及的3个层次：

```图4```： 

![paxos-store-3](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_3.jpeg)

###### 2.1.1 Paxos算法
PaxosStore孕育了2套Paxos算法组件： Paxos Certain组件和Paxos KV组件，内存云使用的正是其中的PaxosKV组件。Paxos KV组件核心代码1912行，经过严格测试，目前用于线上多个Key-Value存储模块，包括但不限于： 用户账号信息存储、朋友圈存储等。

###### 2.1.2 PaxosLog
在构建PaxosLog(简称为PLog)时，我们针对Key-Value存储做了2项优化。

1） **PLog As DB**

普通青年的通常做法是PLog和DB分离: 增量更新记录在PLog中，并在Chosen之后顺序应用到DB中。这里的问题在于：

* 至少2次写操作： 1次PLog写，1次DB写

* Key和PLog的对应关系
<pre>
a) N:1读写性能受限于单个PLog
b) 1:1相同的数据重复写2次
</pre>

```图5 精简1```：plog as db
 
![paxos-plog-db](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_plog_db.jpeg)

进步青年思考了下得出PLog As DB方案： 我们希望Key和PLog保持1:1的对应关系，从而达到最大的并发性能，同时又不引入重复写。

2) **精简的PLog： 只保留最新的LogEntry**

```图6 精简2```： 最新的LogEntry 

![paxos-plog-entry](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_log_entry.jpeg)

作为PLog As DB的延伸，既然每个最新LogEntry包含全量的数据，那么物理上保留连续的PLog就没有必要了。

简单轻便的PLog结构，带来的优势还有：

* LogCompact伴随每次写进行，不占用额外的存储和计算开销

* Log Catch-Up通过分发最新的LogEntry即可完成，无需顺序追流水或者Snapshot，应用DB

###### 2.1.3 基于PaxosLog的强一致读写协议

1) **强一致性读协议**

```图7``` 强一致性读协议

![paxos-read](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_read_protocol.jpeg)

强一致性读协议本身和Paxos算法没有太大关系，要点是多数派： 广播的方式获取集群中多数机器（包含自身）的PLog状态，即最新的LogEntry位置和对应LogEntry处于Pending/Chosen状态。

* 当集群中多数在Log Entry i上处于Chosen状态时，可以确定Log Entry i是最新的。对于读多写少的业务，主要面对这种情况，整体读就可以非常轻量且失败非常低。

* 当集群中多数在Log Entry i上处于Pending状态时，无法确定Log Entry i-1是否最新，因为可能存在某台机器Log Entry i处于Chosen 状态。对于读多写多的业务，读的失败就会相对高很多。

可行的优化有： a) 尽量收集多机的状态信息，如果所有机器的Log Entry i都处于Pending状态，就可以确定Log Entry i-1的数据是最新的； b) 使用隐含条件：只有A/B机器可读写。


2) **强一致写协议**

强一致写协议的大多数问题来自Paxos算法本身，这里我们主要做了3项优化（或者说解决3个问题）。

第一： 写优化

>The leader for each log position is a distinguished replica chosen alongside the preceding log position’s consensus value. The leader arbitrates which value may use proposal number zero.
>
>The first writer to submit a value to the leader wins the right to ask all replicas to accept that value as proposal number zero. All other writers must fall back on two-phase Paxos.


上述文字摘抄自MegaStore: FastWrites部分，描述： LogEntry i-1值的归属者可以在写LogEntry i时跳过Paxos算法的Prepare阶段直接进行Accept阶段。基于这段迷一样的文字，我们实现Paxos 优化写算法： 减少1次写盘、2次协议消息发送、2次协议消息接收；最终实现了写耗时和失败的降低，如下图8所示.

```图8``` 优化写和普通写的性能对比

![paxos-store-time](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_time.jpeg)

第二： 如何确定谁的提议写成功了？

Paxos算法只保证LogEntry i确定唯一值，但在多个Proposer的条件下（即A/B机均可发起强一致写），只有确定值的归属者可以返回成功。这里我们复用了etcd中requestid的方案。

```图9``` requestid方案

![paxos-request-id](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_request_id.jpeg)

其中member_id用于区分不同的Proposer，timestamp为毫秒级别的时间戳，req_cnt随写请求单调递增。基于requestid方案可以满足单机25w/s的写请求，足够了。

>备注： requestid只需要在单个plog维度上保证唯一即可。

第三： Paxos活锁问题

Paxos算法本身不保证终止性，当出现写冲突时算法可能永远终结不了，即存在活锁问题；因此，在实际工程中我们需要进行一些权衡：

* 限制单次Paxos写触发Prepare次数

* 随机避让

我们目前使用了Prepare次数限制策略，从现网监控来看由写冲突导致的失败比例极小：

![paxos-live-lock](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_live_lock.jpeg)


### 2.2 挑战2： 基于机械盘的DirectIO存储
我们在新架构上采用DirectIO的方案实现了一套保证数据安全落盘的存储组件。DirectIO方案与其他两种写盘方案的对比如下表11所示。

```表11``` 写方式比较

![paxos-store-write](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_write.jpeg)

然而，仅仅保证数据安全落盘还不够，我们还要做到稳定：基于Bitcask写模型搭建的存储，需要定期的整理磁盘文件（我们称之为Merge)，以重复利用磁盘空间。其中涉及的操作有： Merge新文件写盘、旧文件删除，对外表现为：Merge写和正常写竞争、文件删除引起系统卡顿。为了克服这2个问题，我们做了以下优化：

* 控制Merge写盘速度和大小

* 在DirectIO的4K块中引入BlockID，以支持文件的循环利用（不再删文件）

```图12``` DirectIO BlockID

![paxos-write-fix](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_write_fix.jpeg)

### 2.3 挑战3： 复杂的现网场景
###### 2.3.1 机械盘RaidCache

RaidCache以电池作为后盾，可以在WriteBack模式下持有待写盘的数据批量写盘，以极大提升机械盘的写盘性能；缺点在于当电池掉电时，为了数据安全必须从WriteBack模式切换到WriteThrough模式，写盘性能急剧下降（真实的机械盘）。磁盘从WB降级为WT是现网运营中常见的问题，通常1~2小时后电池充电完毕即可重回WB模式，我们来看看这1~2小时磁盘退化的影响。

单次磁盘退化导致磁盘写操作耗时和失败率升高，通常情况下被Paxos协议本身的多数派所容忍；但是本机作为Proposer主动发起写时，写盘失败带来的就是单次写请求失败，前端会自动跳转对机重试，此时问题来了： 磁盘退化者写失败之前将LogEntry置于Pending，对机重试的最终结果将Pending推成Chosen，但此时requestid表明Chosen值源于磁盘退化者，对机写被抢占，返回最终失败。

简单的说，磁盘退化期间写最终失败较高：通过将requestid前传到对机，让对机用已有的requestid重试可以将写最终失败降低1个数量级。

###### 2.3.2 PLog对齐

当单机包含KW级别的PLog时，保持系统中所有PLog均处于对齐状态就变得很困难；但只有在所有PLog均处于对齐状态时，系统才能保持最大化的可用性。经历一番权衡后，我们的系统中挂载了以下逻辑（按时效性排序）来保证PLog对齐：

* 失败（本地落后）触发异步Catch-up

* 三级超时队列： 如果LogEntry超时后依旧处于Pending状态，就触发协议写重试；

* 全量数据校验： 校验数据一致性的同时也触发PLog对齐

###### 2.3.3 LearnerOnly模式
机器重启后发现文件丢失，数据被回退了怎么办？Paxos协议保证了觉大部分情况下强一致性和可用性，但不是全部。

某LogEntry承诺机器A的Paxos请求后，因为数据回退状态清空，重新上线后承诺机器B的Paxos请求，但是前后两次承诺相互矛盾，从而导致数据不一致。

上述描述的是拜占庭失败导致的数据不一致，这种失败违反了Paxos协议的假设前提；但现网运营中确实又需要处理这种情况，从而有了LearnerOnly模式的引入。

* LearnerOnly模式下，本机只接收Chosen后的LogEntry，不参与Paxos协议写，也就不会违背任何承诺；

* 三级超时队列，使得LogEntry尽快走向Chosen

* 异步Catch-Up和全量数据校验使得系统PLog较快对齐

假设进入LearnerOnly模式2小时后，系统中旧LogEntry处于Pending状态的可能性可以忽略不计，那么该机可以解除LearnerOnly模式。

## 3. 成果
除了上文描述的优化外，我们还定制了一套本地迁移的方案用于新旧架构的平滑切换，由于篇幅限制，在此就不一一展开了。最终我们实现了千台机器安全无故障的从QuorumKV架构切换到新架构，下面同步下新架构的性能数据和容灾能力。

### 3.1 性能数据
压力测试条件： Value约120B，读写3.3:1

压力测试机型： 64GB内存，机械盘

```表13``` 新旧架构性能对比 

![paxos-store-cmp](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_cmp.jpg)

```图14``` 新旧架构平均耗时和最终失败对比

![paxos-time-cmp](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_time_cmp.jpg)

具体数据如```表13```所示，可以看出在请求量相当的条件下，新架构在平均耗时和最终失败上都优于旧架构；并且新架构只需要6台机器。

>备注： 新架构部署上少了3台机器，带来单机约50%的内存和磁盘增长（容纳3份数据）

### 3.2 系统可用性
1) **实例1： 网络故障期间可用性对比**

```图15``` 网络故障期间 PaxosStore内存云首次访问失败率监控曲线

![paxos-store-bad](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_bad.jpg)

```图16``` 网络故障期间QuorumKV内存云首次访问失败率监控曲线

![paxos-store-fail](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_store_fail.jpg)

某次内网故障期间（持续约半小时），PaxosStore内存云展现了卓越的容灾能力： 图15为故障期间PaxosStore内存云的首次访问失败率监控，可以看到失败率是十分平稳的（因为网络故障期间前端请求有所降低，失败率反而小了些）；与之对应QuorumKV内存云则表现不理想。

>备注： 需要强调的是得益于微信后台整体优秀的容灾设计，用户对本次网络故障感知度很低


2) **某城市切换架构前后失败率对比**

```图17``` PaxosStore内存云首次访问失败率监控曲线

![paxos-memkv-fail1](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_memkv_fail1.jpg)

```图18``` QuorumKV内存云首次访问失败率监控曲线

![paxos-memkv-fail2](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_memkv_fail2.jpg)

图17和图18分别给出了新旧架构KV首次访问失败率监控曲线（百万分之一），可以看到切换后系统的首次访问成功率从5个9提升至6个9，系统可用性得以增强。

>备注： 此处为极为微观的数据，前端会有简单的重试达到100%成功

## 4. 小结
全新的内存云存储，通过精简的PLog As DB、分布式强一致性读写协议等一连串优化，在性能上得到显著提升；结合DirectIO存储系统，PLog全量对齐等，系统首次访问失败率指标下降一个量级。

PaxosStore是微信内部一次大规模paxos工程改造实践，创新地实现了非租约Paxos架构。


<br />
<br />
**参看：**

1. [Paxos从理论到实践 ](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw?)

2. [NWR协议](https://www.cnblogs.com/liyulong1982/p/5999169.html)

3. [PaxosStore解读](https://blog.csdn.net/chdhust/article/details/77750327)

4. [paxos 视频](https://video.tudou.com/v/XMTcwNTQyNDU4MA==.html?spm=a2h0k.8191414.0.0&from=s1.8-1-1.2)

5. [用paxos实现多副本日志系统--basic paxos部分](https://blog.csdn.net/u011750989/article/details/81814700)

6. [用paxos实现多副本日志系统--multi paxos部分](https://blog.csdn.net/u011750989/article/details/81814704)

<br />
<br />
<br />


