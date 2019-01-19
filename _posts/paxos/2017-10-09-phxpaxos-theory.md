---
layout: post
title: phxpaxos理论介绍(1)： 朴素Paxos算法理论推导与证明
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->


## 1. 朴素Paxos

这篇文章摘取部分我在微信内部关于Paxos的分享PPT，通过注解的方式尝试与大家说明白朴素Paxos的理论证明。

为何要重点说朴素的Paxos? 个人认为这个才是paxos的精髓所在，也是所有Paxos相关算法的基石所在。另外，本文将着重讲解Paxos的算法推导过程，而不是运行过程。因为以在我学习算法的经验来看，推导过程对于掌握一门算法至关重要，只有掌握了理论推导过程，才能明白这个算法每一个步骤的含义。

这些PPT内容大部分都引自Lamport的论文 “The Part-Time Parliament”:

![part-time-parliament](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_part_1.jpg)

上面这是PPT的题图，摆在中间的正是Paxos最为重要的三条约束，掌握这三条约束，即可掌握朴素Paxos。

![paxos-verify-value](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_verify_value.jpg)

在正式开始讲解之前，希望抛开所有对Paxos的展开，而回到最朴素的Paxos。最朴素的Paxos解决什么问题？ 这里举个例子：三个人分别只允许呆在不同的三个城市，他们手上有一张纸和一支笔，他们可以在纸上写下任何内容，但是，当他们停下他们的笔之后，我们希望三个人最后写下的内容都是一样的。

这个就是最朴素的Paxos尝试解决的问题，确定一个值。暂时千万别去想更多的东西，聚焦在确定一个值这么一个看似非常简单的事情身上。

![paxos-define](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_define.jpg)

直入主题，提出一轮投票的定义。通过投票来决定一个提议，是一个非常原始的方法，也是非常显然的公理，这里不展开说。这里提议对应刚刚说到的这个值（即上面提到的```确定一个值```)。这一页每个定义都要弄明白，因为下面会常常用到这些定义。比如你要记住，一轮投票会有一个编号标识他们，称之为```Bbal```。你还要理解集合的意思，一轮投票集合B概括了这一轮投票的**所有参与人**、**投票编号**、**提议**、以及**投票情况**等。

比较难理解的```Bqrm```这里展开解释一下： 一轮投票获得通过，必须有```Bqrm```的人进行了投票，这个```Bqrm```每次可能都是不同的集合，但是它的特征是肯定超过总体投票成员的半数，也就是我们常说的多数派。

![paxos-multi-bal](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_multi_bal.jpg)

很显然，一轮投票是解决不了一致性问题的，因为任意一个人都有可能去发起投票，而不能靠上帝去指定某个人去发起，所以必然会面临多轮投票带来的问题。这里提出多轮投票的定义。注意这个多轮投票集合的定义是希腊字母```Beta```，一轮投票集合是大写的字母```B```，是不一样的。我们希望寻求方法解决多轮投票带来的冲突，从而去达到确定一个值的目标。

![paxos-max-vote](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_max_vote.jpg)

最重要的定义```MaxVote```的提出。要尝试解决多轮投票带来的冲突问题，必然要去建立多轮投票之间的联系，```MaxVote```是一个联系。

```MaxVote```通过给出一个编号，以及成员，可以在多轮投票里面找到这些成员小于这个**编号**的所有投票当中，最大编号的那个投票。然后我们希望用到这次投票对应的提议。仔细阅读样例表格里面的每个```MaxVote```，从而去理解这个定义。

![paxos-bal-limit](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_bal_limit.jpg)

在提出了所有数学定义后，就可以去理解这最优美的三个约束条件了。正是通过这三个约束，使得多轮投票的冲突问题得到解决。



第一点很好理解，要求每轮投票的编号唯一。第二点要求任意两轮投票的```Bqrm```交集不为空，其实意思很明确，就是要求```Bqrm```超过半数的意思。第三点是解决冲突的关键所在，它强行约束了每轮投票的提议，使得这轮投票的提议不与之前的产生冲突。通俗一点讲就是，一旦我发现在我之前已经有人投过某个提议的票，那我就要用这个提议，并且是我之前最大编号的投票对应的提议，作为我这次的提议。

![paxos-multi-vote-limit](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_multi_vote_limit.jpg)

这是在三个约束条件之下的多轮投票过程。反复阅读这两页，从而理解```约束条件3```。注意在约束条件下，提议内容的变化。


![paxos-vote-consistency](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_vote_consistency.jpg)

看似这个投票过程可以引出最终一致性的提议内容，但严格的算法推导必然需要严格的证明。这里提出反证法。

![paxos-prove-process](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_prove_process.jpg)


上图我们已将证明过程简化，相信经过你们认真的推敲，可定是可以搞明白的。注意表格里面的样例，当编号为```2```的这轮投票通过后，又出现了一轮编号为```3```的投票（2和3之间不可能存在一轮投票），提议跟之前的冲突。我们通过推导得出这个情况是不存在的。

![paxos-get-maxvote](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_get_maxvote.jpg)


前面只是提出```MaxVote```的定义，这里解释计算这个```MaxVote```的实际操作过程。其实就是我们惯用的轮询法，逐个问呗。只要每次发起投票前，都向多数派的成员逐一询问它们比我当前这轮投票编号小的最大编号投票，即可获得整个集合的```MaxVote```，从而确定当前这轮投票的提议。

![paxos-wrong-maxvote](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_wrong_maxvote.jpg)

如上图所示，聪明的读者可能早已经发现了问题，我们上文所说的多轮投票，似乎编号都是严格递增的，但是现实情况完全不是这样，现实的多轮投票往往都是乱序的，这个大家应该毫无疑问。那么在这种情况下，```MaxVote```的值可能会是错的。想象一下，在算出一个```MaxVote(5,...)```之后，才出现一个编号比5小的投票，那么这个投票很可能会影响到这个```MaxVote```的值。也就是一个先来后到的乱序问题。而如果```MaxVote```是错的，我们的证明就失效了。

![paxos-maxvote-revolution](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_maxvote_revolution.jpg)

如上图所示，为了满足这个约束，我们需要对```MaxVote```的计算过程进行约束。看过Paxos算法过程的，且是聪明的读者，看到这可能会想起，哦原来Prepare的Promise要求是这么来的。

![paxos-alg-standard](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_alg_standard.jpg)

到这里算法已经是非常完善了，剩下就是怎么将这个算法引申到计算机上，在计算机层面上提出算法的过程。大家可以看到实际的算法过程，很多角色都是与我们刚刚描述的东西相对应的。


![paxos-alg-note](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_alg_note.jpg)

正式算法过程，也就是**Lamport**论文```Paxos Made Simple```提出的。我希望大家再回头来看这个算法过程的时候，知道每一步的含义，以及本后的本质。

![paxos-alg-display](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_alg_display.jpg)

上图为一个过程的演示，这里就不多解释了。（上图中```before_res```是**before response**的缩写）


## 2. 总结
朴素Paxos算法不容易，需要反复推敲，建议有耐心的读者多看几遍，只要有耐心，肯定是可以弄懂的。关于这个算法有什么用？ 如何去实现？ 我们后面会再进行讲解。




<br />
<br />
**参看：**

1. [Paxos从理论到实践](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)

2. [phxpaxos](https://github.com/Tencent/phxpaxos/blob/master/README.zh_CN.md)


3. [Paxos算法](https://zh.wikipedia.org/zh-cn/Paxos%E7%AE%97%E6%B3%95)

4. [腾讯开源的 Paxos库 PhxPaxos 代码解读-](https://www.cnblogs.com/lijingshanxi/p/10250878.html)

<br />
<br />
<br />


