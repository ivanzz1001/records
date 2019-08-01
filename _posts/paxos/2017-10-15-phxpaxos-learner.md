---
layout: post
title: phxpaxos源码分析： Proposer与Accepter
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

<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之Proposer、Acceptor](https://www.jianshu.com/p/2a78c6215e6d)

<br />
<br />
<br />


