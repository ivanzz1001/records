---
layout: post
title: phxpaxos源码分析： Learner
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->


## 1. 基本概念
在Paxos中，Learner角色负责向其他节点学习选中的提案值。具体包括如下两种场景：

* Learner所在节点参与了提案选举，Learner需要指定其接受(accept)的提案值是否被选中(chosen)

* Learner所在的节点已落后于其他节点，Learner需要选择合适的策略快速完成追赶，并重新参与到提案选举当中。

## 2. 选中(chosen)通知
我们在Proposer::OnAcceptReply()函数中，当对应的提案通过时，就会调用Learner::ProposerSendSuccess()函数来发送对应的消息(MsgType_PaxosLearner_ProposerSendSuccess)来通知到各个节点：
{% highlight string %}
void Learner :: ProposerSendSuccess(
        const uint64_t llLearnInstanceID,
        const uint64_t llProposalID)
{
    BP->GetLearnerBP()->ProposerSendSuccess();

    PaxosMsg oPaxosMsg;
    
    oPaxosMsg.set_msgtype(MsgType_PaxosLearner_ProposerSendSuccess);
    oPaxosMsg.set_instanceid(llLearnInstanceID);
    oPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oPaxosMsg.set_proposalid(llProposalID);
    oPaxosMsg.set_lastchecksum(GetLastChecksum());

    //run self first
    BroadcastMessage(oPaxosMsg, BroadcastMessage_Type_RunSelf_First);
}
{% endhighlight %}
函数首先构造一个PaxosMsg，然后通过UDP发送到各个节点。通知的消息格式如下：
<pre>
|-----------------------------------------------------------------------------
|   MsgType   |  InstanceID   |   NodeID    |  ProposalID   |  LastChecksum  |
------------------------------------------------------------------------------
</pre>

当节点收到对应消息时，就会回调到*Learner :: OnProposerSendSuccess()*:
{% highlight string %}
void Learner :: OnProposerSendSuccess(const PaxosMsg & oPaxosMsg)
{
    BP->GetLearnerBP()->OnProposerSendSuccess();

    PLGHead("START Msg.InstanceID %lu Now.InstanceID %lu Msg.ProposalID %lu State.AcceptedID %lu "
            "State.AcceptedNodeID %lu, Msg.from_nodeid %lu",
            oPaxosMsg.instanceid(), GetInstanceID(), oPaxosMsg.proposalid(), 
            m_poAcceptor->GetAcceptorState()->GetAcceptedBallot().m_llProposalID,
            m_poAcceptor->GetAcceptorState()->GetAcceptedBallot().m_llNodeID, 
            oPaxosMsg.nodeid());

    if (oPaxosMsg.instanceid() != GetInstanceID())
    {
        //Instance id not same, that means not in the same instance, ignord.
        PLGDebug("InstanceID not same, skip msg");
        return;
    }

    if (m_poAcceptor->GetAcceptorState()->GetAcceptedBallot().isnull())
    {
        //Not accept any yet.
        BP->GetLearnerBP()->OnProposerSendSuccessNotAcceptYet();
        PLGDebug("I haven't accpeted any proposal");
        return;
    }

    BallotNumber oBallot(oPaxosMsg.proposalid(), oPaxosMsg.nodeid());

    if (m_poAcceptor->GetAcceptorState()->GetAcceptedBallot()
            != oBallot)
    {
        //Proposalid not same, this accept value maybe not chosen value.
        PLGDebug("ProposalBallot not same to AcceptedBallot");
        BP->GetLearnerBP()->OnProposerSendSuccessBallotNotSame();
        return;
    }

    //learn value.
    m_oLearnerState.LearnValueWithoutWrite(
            oPaxosMsg.instanceid(),
            m_poAcceptor->GetAcceptorState()->GetAcceptedValue(),
            m_poAcceptor->GetAcceptorState()->GetChecksum());
    
    BP->GetLearnerBP()->OnProposerSendSuccessSuccessLearn();

    PLGHead("END Learn value OK, value %zu", m_poAcceptor->GetAcceptorState()->GetAcceptedValue().size());

    TransmitToFollower();
}
{% endhighlight %}

1) 若当前Learner收到的MsgType_PaxosLearner_ProposerSendSuccess消息多对应的instanceID并不是当前Learner所期望学习的instanceID，则直接跳过；

2） 若当前Learner所对应的Acceptor并没有accept任何提案，则直接跳过；

3） 若当前Learner对应的Acceptor所accept的提案(BallotNumber)等于当前收到的PaxosMsg提案，则成功学习到所对应的提案值。

4） 将学习到的提案值发送给follower
{% highlight string %}
void Learner :: TransmitToFollower()
{
    if (m_poConfig->GetMyFollowerCount() == 0)
    {
        return;
    }
    
    PaxosMsg oPaxosMsg;
    
    oPaxosMsg.set_msgtype(MsgType_PaxosLearner_SendLearnValue);
    oPaxosMsg.set_instanceid(GetInstanceID());
    oPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oPaxosMsg.set_proposalnodeid(m_poAcceptor->GetAcceptorState()->GetAcceptedBallot().m_llNodeID);
    oPaxosMsg.set_proposalid(m_poAcceptor->GetAcceptorState()->GetAcceptedBallot().m_llProposalID);
    oPaxosMsg.set_value(m_poAcceptor->GetAcceptorState()->GetAcceptedValue());
    oPaxosMsg.set_lastchecksum(GetLastChecksum());

    BroadcastMessageToFollower(oPaxosMsg, Message_SendType_TCP);

    PLGHead("ok");
}
{% endhighlight %}


正常情况下，所有节点处于online状态，共同参与paxos选举。因此，各个节点的instanceID一致。为了避免冲突，paxos建议只由主节点的proposer发起提案，这样保证接受提案与习得提案编号一致。

此时，Learn习得的提案值实际上就是本节点Accept的数据，因此，learn只更新内存状态即可，无需再次落盘（acceptor已经落盘）。最后，如果存在follower节点，数据同步到follower（follower节点不参与paxos算法，相当于某个paxos节点的同步备）。

## 3. 提案值追赶
一点节点处于落后状态，它就无法再参与到paxos提案选举中来。这时需要由learner发起主动学习完成追赶。

PhxPaxos启动时(Instance :: Init()中），会开启learner定时器。learner定时器发送learn请求到各个节点，发送请求携带本节点的instanceID，NodeID信息：
{% highlight string %}
void Learner :: AskforLearn_Noop(const bool bIsStart)
{
    Reset_AskforLearn_Noop();

    m_bIsIMLearning = false;

    m_poCheckpointMgr->ExitCheckpointMode();

    AskforLearn();
    
    if (bIsStart)
    {
        AskforLearn();
    }
}
{% endhighlight %}
上面AskforLearn()就会发送MsgType_PaxosLearner_AskforLearn到各个节点。各个节点收到请求后，Instance::OnReceive()会回调Instance::OnReceivePaxosMsg()，之后再回调到Instance::ReceiveMsgForLearner()，最后回调到如下函数：
{% highlight string %}
void Learner :: OnAskforLearn(const PaxosMsg & oPaxosMsg)
{
    BP->GetLearnerBP()->OnAskforLearn();
    
    PLGHead("START Msg.InstanceID %lu Now.InstanceID %lu Msg.from_nodeid %lu MinChosenInstanceID %lu", 
            oPaxosMsg.instanceid(), GetInstanceID(), oPaxosMsg.nodeid(),
            m_poCheckpointMgr->GetMinChosenInstanceID());
    
    SetSeenInstanceID(oPaxosMsg.instanceid(), oPaxosMsg.nodeid());

    if (oPaxosMsg.proposalnodeid() == m_poConfig->GetMyNodeID())
    {
        //Found a node follow me.
        PLImp("Found a node %lu follow me.", oPaxosMsg.nodeid());
        m_poConfig->AddFollowerNode(oPaxosMsg.nodeid());
    }
    
    if (oPaxosMsg.instanceid() >= GetInstanceID())
    {
        return;
    }

    if (oPaxosMsg.instanceid() >= m_poCheckpointMgr->GetMinChosenInstanceID())
    {
        if (!m_oLearnerSender.Prepare(oPaxosMsg.instanceid(), oPaxosMsg.nodeid()))
        {
            BP->GetLearnerBP()->OnAskforLearnGetLockFail();

            PLGErr("LearnerSender working for others.");

            if (oPaxosMsg.instanceid() == (GetInstanceID() - 1))
            {
                PLGImp("InstanceID only difference one, just send this value to other.");
                //send one value
                AcceptorStateData oState;
                int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), oPaxosMsg.instanceid(), oState);
                if (ret == 0)
                {
                    BallotNumber oBallot(oState.acceptedid(), oState.acceptednodeid());
                    SendLearnValue(oPaxosMsg.nodeid(), oPaxosMsg.instanceid(), oBallot, oState.acceptedvalue(), 0, false);
                }
            }
            
            return;
        }
    }
    
    SendNowInstanceID(oPaxosMsg.instanceid(), oPaxosMsg.nodeid());
}
{% endhighlight %}

## 4. 更快的对齐数据
下述文字截取自[《微信自研生产级paxos类库PhxPaxos实现原理介绍》](https://mp.weixin.qq.com/s?__biz=MzI4NDMyNTU2Mw==&mid=2247483695&idx=1&sn=91ea422913fc62579e020e941d1d059e&scene=21#wechat_redirect)：
>上文说到当各台机器的当前运行实例编号不一致的时候，就需要Learner介入工作来对齐数据了。Learner通过其他机器拉取到当前实例的chosen value，从而跳转到下一个编号的实例，如此反复最终将自己的实例编号更新到与其他机器一致。那么这里学习一个实例的网络延时代价是一个RTT。可能这个延迟看起来还不错，但是当新的数据仍然通过一个RTT的代价不断写入的时候，而落后的机器仍然以一个RTT来进行学习，这样会出现很难追上的情况。
>
>这里需要改进，我们可以提前获取差距，批量打包进行学习。比如A机器Learner记录当前实例编号是 x， B机器是y， 而 x<y， 那么B机器通过通信获取这个差距，将(x,y]的choosen value一起打包发送给A机器， A机器进行批量的学习。这是一个很不错的方法。
>
>但仍然不够快，当落后的数据极大，B机器发送数据需要的网络耗时也将变大，那么发送数据的过程中，A机器处于一种空闲状态，由于paxos另外一个瓶颈在于写盘，如果不能利用这段时间来进行写盘，那性能仍然堪忧。我们参考流式传输，采用类似的方法实现Learner的边发边学，B机器源源不断的往A机器输送数据，而A机器只需要收到一个实例最小单元的包体，即可立即解开进行学习并完成写盘。
>
>具体的实现大概是先进行一对一的协商，建立一个Session通道，在Session通道里直接采用直塞的方式无脑发送数据。当然也不是完全无脑，Session通过心跳机制进行维护，一旦Session断开即停止发送。



<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之Proposer、Acceptor](https://www.jianshu.com/p/2a78c6215e6d)

<br />
<br />
<br />


