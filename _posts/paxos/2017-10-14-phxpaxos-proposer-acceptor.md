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

## 1. proposal的发起与接受

本文我们会通过分析phxpaxos中自带的phxecho示例程序，以进一步理解```Paxos确定一个值```的过程。在分析具体的程序之前，我们再简要重申一下Paxos算法所包含3个角色：

* **Proposer**: 提案发起者。负责发起提案，提案本身由编号(number)、值(value)两部分组成

* **Acceptor**: 提案接收者。负责接收由Proposer发起的提案，通过一定的规则首先确定提案编号，最终确定提案值。

* **Learner**: 被选中提案的学习者。Learner不参与提案的确定过程，只负责学习已确定好的提案值。

简单来说，提案由每个节点的Proposer、Acceptor参与决策，如果某个节点由于网络故障等原因未参与决策，由Learner负责学习已经选中的提案值。

Paxos算法分为提案选定(Prepare)、确定提案值(accept)两个阶段。每个阶段都有可能需要执行多次，每次都花费一个RTT网络耗时。为了达到产品级应用的目的，PhxPaxos采用选主并在主上执行Paxos算法的方式来避免提案冲突，随后为所有的提案执行noop操作，有条件跳过Prepare阶段将网络花费降至理论最小值（一个RTT)。

>注： 示例程序phxecho的运行并未执行选主，但仍是以Multi-Paxos方式来进行工作的。Multi-Paxos的工作并不依赖于Master，只是在有Master的情况下可以避免提案冲突，达到最高的工作效率。

本章我们先来看Proposer、Acceptor两个角色，Learner我们会在后续的章节进行讲解。

### 2. Proposer
我们先来看```PhxEchoServer::Echo()```函数，其通过调用如下函数来发起一个提案：
{% highlight string %}
int PNode :: Propose(const int iGroupIdx, const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx)
{
    if (!CheckGroupID(iGroupIdx))
    {
        return Paxos_GroupIdxWrong;
    }

    return m_vecGroupList[iGroupIdx]->GetCommitter()->NewValueGetID(sValue, llInstanceID, poSMCtx);
}
{% endhighlight %}
如上我们看到，是通过进一步调用对应Group的Committer来发起提案的。Committer::NewValueGetID()函数本身较为简单，我们不进行说明，我们来看其所调用的下一级函数：
{% highlight string %}
int Committer :: NewValueGetIDNoRetry(const std::string & sValue, uint64_t & llInstanceID, SMCtx * poSMCtx)
{
    LogStatus();

    int iLockUseTimeMs = 0;
    bool bHasLock = m_oWaitLock.Lock(m_iTimeoutMs, iLockUseTimeMs);
    if (!bHasLock)
    {
        if (iLockUseTimeMs > 0)
        {
            BP->GetCommiterBP()->NewValueGetLockTimeout();
            PLGErr("Try get lock, but timeout, lockusetime %dms", iLockUseTimeMs);
            return PaxosTryCommitRet_Timeout; 
        }
        else
        {
            BP->GetCommiterBP()->NewValueGetLockReject();
            PLGErr("Try get lock, but too many thread waiting, reject");
            return PaxosTryCommitRet_TooManyThreadWaiting_Reject;
        }
    }

    int iLeftTimeoutMs = -1;
    if (m_iTimeoutMs > 0)
    {
        iLeftTimeoutMs = m_iTimeoutMs > iLockUseTimeMs ? m_iTimeoutMs - iLockUseTimeMs : 0;
        if (iLeftTimeoutMs < 200)
        {
            PLGErr("Get lock ok, but lockusetime %dms too long, lefttimeout %dms", iLockUseTimeMs, iLeftTimeoutMs);

            BP->GetCommiterBP()->NewValueGetLockTimeout();

            m_oWaitLock.UnLock();
            return PaxosTryCommitRet_Timeout;
        }
    }

    PLGImp("GetLock ok, use time %dms", iLockUseTimeMs);
    
    BP->GetCommiterBP()->NewValueGetLockOK(iLockUseTimeMs);

    //pack smid to value
    int iSMID = poSMCtx != nullptr ? poSMCtx->m_iSMID : 0;
    
    string sPackSMIDValue = sValue;
    m_poSMFac->PackPaxosValue(sPackSMIDValue, iSMID);

    m_poCommitCtx->NewCommit(&sPackSMIDValue, poSMCtx, iLeftTimeoutMs);
    m_poIOLoop->AddNotify();

    int ret = m_poCommitCtx->GetResult(llInstanceID);

    m_oWaitLock.UnLock();
    return ret;
}
{% endhighlight %}
对于同一个Committer，可能会有多个线程同时调用其来发起```Proposal```，因此首先需要通过获取```m_oWaitLock```来使其串行化（这里假设RTT最少为200ms，因此如果剩余时间iLeftTimeoutMs低于该值，不再进行后续的提交操作）。

接着执行如下步骤：

1） **将StateMachine ID打包进MSG**

通过调用PackPaxosValue()函数来将StateMachine ID打包进所要提交的消息中(此处我们传递的状态机ID为```1```)：
{% highlight string %}
void SMFac :: PackPaxosValue(std::string & sPaxosValue, const int iSMID)
{
    char sSMID[sizeof(int)] = {0};
    if (iSMID != 0)
    {
        memcpy(sSMID, &iSMID, sizeof(sSMID));
    }

    sPaxosValue = string(sSMID, sizeof(sSMID)) + sPaxosValue;
}
{% endhighlight %}

2） **初始化该Committer对应的上下文**

调用CommitCtx::NewCommit()重新对该Committer所关联的CommitCtx进行初始化，准备进行提交：
{% highlight string %}
void CommitCtx :: NewCommit(std::string * psValue, SMCtx * poSMCtx, const int iTimeoutMs)
{
    m_oSerialLock.Lock();

    m_llInstanceID = (uint64_t)-1;
    m_iCommitRet = -1;
    m_bIsCommitEnd = false;
    m_iTimeoutMs = iTimeoutMs;

    m_psValue = psValue;
    m_poSMCtx = poSMCtx;

    if (psValue != nullptr)
    {
        PLGHead("OK, valuesize %zu", psValue->size());
    }

    m_oSerialLock.UnLock();
}
{% endhighlight %}

3) **通知IOLoop有新的Commit**

调用IOLoop::AddNotify()唤醒可能处于挂起状态的IOLoop，让其执行我们的提交操作：
{% highlight string %}
void IOLoop :: AddNotify()
{
    m_oMessageQueue.lock();
    m_oMessageQueue.add(nullptr);
    m_oMessageQueue.unlock();
}
{% endhighlight %}
注意，这里我们并没有将真正需要发送的消息投递到队列，这仅仅只是起一个通知作用。事实上，```m_oMessageQueue```是作为**接收消息队列**使用，而不是**发送消息队列**使用。这里发送一个```nullptr```仅仅起唤醒作用。

> 题外话，IOLoop唤醒之后，其实是IOLoop::OneLoop()循环中，通过调用m_poInstance()->CheckNewValue()来处理该新的提议的。


4） **等待提交结果**

调用CommitCtx::GetResult()不断等待提交结果：
{% highlight string %}
int CommitCtx :: GetResult(uint64_t & llSuccInstanceID)
{
    m_oSerialLock.Lock();

    while (!m_bIsCommitEnd)
    {
        m_oSerialLock.WaitTime(1000);
    }

    if (m_iCommitRet == 0)
    {
        llSuccInstanceID = m_llInstanceID;
        PLGImp("commit success, instanceid %lu", llSuccInstanceID);
    }
    else
    {
        PLGErr("commit fail, ret %d", m_iCommitRet);
    }
    
    m_oSerialLock.UnLock();

    return m_iCommitRet;
}
{% endhighlight %}
这里会不断的进行等待，直到提交超时或者受到其他Acceptor返回过来的结果。

### 2.1 对新的Commit进行检查

在IOLoop::OneLoop()中会调用本函数检查是否有新的提交：
{% highlight string %}
void Instance :: CheckNewValue()
{
    if (!m_oCommitCtx.IsNewCommit())
    {
        return;
    }

    if (!m_oLearner.IsIMLatest())
    {
        return;
    }

    if (m_poConfig->IsIMFollower())
    {
        PLGErr("I'm follower, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Follower_Cannot_Commit);
        return;
    }

    if (!m_poConfig->CheckConfig())
    {
        PLGErr("I'm not in membership, skip this new value");
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Im_Not_In_Membership);
        return;
    }

    if ((int)m_oCommitCtx.GetCommitValue().size() > MAX_VALUE_SIZE)
    {
        PLGErr("value size %zu to large, skip this new value",
            m_oCommitCtx.GetCommitValue().size());
        m_oCommitCtx.SetResultOnlyRet(PaxosTryCommitRet_Value_Size_TooLarge);
        return;
    }

    m_oCommitCtx.StartCommit(m_oProposer.GetInstanceID());

    if (m_oCommitCtx.GetTimeoutMs() != -1)
    {
        m_oIOLoop.AddTimer(m_oCommitCtx.GetTimeoutMs(), Timer_Instance_Commit_Timeout, m_iCommitTimerID);
    }
    
    m_oTimeStat.Point();

    if (m_poConfig->GetIsUseMembership()
            && (m_oProposer.GetInstanceID() == 0 || m_poConfig->GetGid() == 0))
    {
        //Init system variables.
        PLGHead("Need to init system variables, Now.InstanceID %lu Now.Gid %lu", 
                m_oProposer.GetInstanceID(), m_poConfig->GetGid());

        uint64_t llGid = OtherUtils::GenGid(m_poConfig->GetMyNodeID());
        string sInitSVOpValue;
        int ret = m_poConfig->GetSystemVSM()->CreateGid_OPValue(llGid, sInitSVOpValue);
        assert(ret == 0);

        m_oSMFac.PackPaxosValue(sInitSVOpValue, m_poConfig->GetSystemVSM()->SMID());
        m_oProposer.NewValue(sInitSVOpValue);
    }
    else
    {
        if (m_oOptions.bOpenChangeValueBeforePropose) {
            m_oSMFac.BeforePropose(m_poConfig->GetMyGroupIdx(), m_oCommitCtx.GetCommitValue());
        }
        m_oProposer.NewValue(m_oCommitCtx.GetCommitValue());
    }
}
{% endhighlight %}
下面我们简要分析一下本函数的执行：

1） 通过CommitCtx检查是否是一个新的提交；

2） 如果当前Instance所关联的Learner检测到还有一些老的提议没有学习到，则这里会禁止提交。假如不禁止进行强行提交的话，也会因为Proposer ID过低而被其他Acceptor所拒绝，因此后续我们得不断的提高Proposer ID，这样不但浪费时间而且占用带宽。因此在我们明确知道Learner还有一些旧的提议没有学习完之前，我们拒绝提交。

3） 若本Instance只是作为一个Follower的话，不参与提交

4） 若当前节点并不是集群中的一员的话,禁止提交
>注： 在我们的示例程序中，我们通过如下方式运行
>./phxecho 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 
>这样当前节点就已经是集群中一员了。在Config::Init()函数中会将后面3个节点信息添加到SystemVSM中


5) 启动提交
{% highlight string %}
m_oCommitCtx.StartCommit(m_oProposer.GetInstanceID());
{% endhighlight %}
关于```m_oProposer```是如何初始化并产生新的proposerID的，我们会在后面通过相关章节进行讲解。

6) 根据提交时设置的超时时间，来判断我们是否要启动定时器（目前我们将超时时间设置为-1，因此并不需要启动超时定时器）
{% highlight string %}
if (m_oCommitCtx.GetTimeoutMs() != -1)
{
    m_oIOLoop.AddTimer(m_oCommitCtx.GetTimeoutMs(), Timer_Instance_Commit_Timeout, m_iCommitTimerID);
}
    
{% endhighlight %}

7) proposer发起新的提交

默认情况下，我们并不使用*membership*，*m_oOptions.bOpenChangeValueBeforePropose*的值也为false，因此我们会直接进入proposer的提交。
{% highlight string %}
m_oProposer.NewValue(m_oCommitCtx.GetCommitValue());
{% endhighlight %}

### 2.2 Proposer::NewValue()
接着上面，我们先来看Proposer::NewValue()函数：
{% highlight string %}
int Proposer :: NewValue(const std::string & sValue)
{
    BP->GetProposerBP()->NewProposal(sValue);

    if (m_oProposerState.GetValue().size() == 0)
    {
        m_oProposerState.SetValue(sValue);
    }

    m_iLastPrepareTimeoutMs = START_PREPARE_TIMEOUTMS;
    m_iLastAcceptTimeoutMs = START_ACCEPT_TIMEOUTMS;

    if (m_bCanSkipPrepare && !m_bWasRejectBySomeone)
    {
        BP->GetProposerBP()->NewProposalSkipPrepare();

        PLGHead("skip prepare, directly start accept");
        Accept();
    }
    else
    {
        //if not reject by someone, no need to increase ballot
        Prepare(m_bWasRejectBySomeone);
    }

    return 0;
}
{% endhighlight %}
1) 设置我们要提交的值设置到```m_oProposerState```中，通常情况下一轮新的Paxos提交，初始*m_oProposerState.GetValue()*的值为空。这里我们设置的值为：
<pre>
|---------------------------------
| StateMachineID |   message     |
|   (4Bytes)     |               |
----------------------------------
</pre>

2) 设置prepare的超时时间与accept的超时时间
<pre>
m_iLastPrepareTimeoutMs = START_PREPARE_TIMEOUTMS;   //2s
m_iLastAcceptTimeoutMs = START_ACCEPT_TIMEOUTMS;     //1s
</pre>

3) 执行Prepare操作。

满足如下场景，允许跳过prepare阶段：
>本节点之前执行国Prepare阶段，并且Prepare阶段的直接结果为Accept

当不满足上述场景时，需要执行完整的Paxos两阶段流程。这里我们虽然使用的是Multi-Paxos，但通常第一次提交我们还是要执行Prepare，因此下面我们来详细的分析一下Prepare()函数。

### 2.3 Prepare阶段

本函数正式进入我们在```Paxos理论```中介绍的Prepare阶段，我们来看：
{% highlight string %}
void Proposer :: Prepare(const bool bNeedNewBallot)
{
    PLGHead("START Now.InstanceID %lu MyNodeID %lu State.ProposalID %lu State.ValueLen %zu",
            GetInstanceID(), m_poConfig->GetMyNodeID(), m_oProposerState.GetProposalID(),
            m_oProposerState.GetValue().size());

    BP->GetProposerBP()->Prepare();
    m_oTimeStat.Point();
    
    ExitAccept();
    m_bIsPreparing = true;
    m_bCanSkipPrepare = false;
    m_bWasRejectBySomeone = false;

    m_oProposerState.ResetHighestOtherPreAcceptBallot();
    if (bNeedNewBallot)
    {
        m_oProposerState.NewPrepare();
    }

    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_msgtype(MsgType_PaxosPrepare);
    oPaxosMsg.set_instanceid(GetInstanceID());
    oPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oPaxosMsg.set_proposalid(m_oProposerState.GetProposalID());

    m_oMsgCounter.StartNewRound();

    AddPrepareTimer();

    PLGHead("END OK");

    BroadcastMessage(oPaxosMsg);
}
{% endhighlight %}

发起Prepare主要做4件事情：

1） Proposer状态重置，表明当前开始进入Prepare阶段
<pre>
ExitAccept();
m_bIsPreparing = true;
m_bCanSkipPrepare = false;
m_bWasRejectBySomeone = false;
</pre>

2) 按需使用编号
{% highlight string %}
m_oProposerState.ResetHighestOtherPreAcceptBallot();
if (bNeedNewBallot)
{
    m_oProposerState.NewPrepare();
}
{% endhighlight %}
这里```按需```的意思是指，当有其他节点明确拒绝了该提案，按Paxos协议必须使用新的提案编号重写发起提案；而如果并无其他节点拒绝，即由于超时等原因导致的重新发起提案，可沿用原来的编号。

3) 构造PaxosMsg

通过如下方式构造PaxosMsg:
{% highlight string %}
PaxosMsg oPaxosMsg;
oPaxosMsg.set_msgtype(MsgType_PaxosPrepare);
oPaxosMsg.set_instanceid(GetInstanceID());
oPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
oPaxosMsg.set_proposalid(m_oProposerState.GetProposalID());
{% endhighlight %}
这里我们查看```paxos_msg.proto```文件，其消息格式如下：
{% highlight string %}
message PaxosMsg
{
	required int32 MsgType = 1;
	optional uint64 InstanceID = 2;
	optional uint64 NodeID = 3;
	optional uint64 ProposalID = 4;
	optional uint64 ProposalNodeID = 5;
	optional bytes Value = 6;
	optional uint64 PreAcceptID = 7;
	optional uint64 PreAcceptNodeID = 8;
	optional uint64 RejectByPromiseID = 9;
	optional uint64 NowInstanceID = 10;
	optional uint64 MinChosenInstanceID = 11;
	optional uint32 LastChecksum = 12;
	optional uint32 Flag = 13;
	optional bytes SystemVariables = 14;
	optional bytes MasterVariables = 15;
};
{% endhighlight %}

4） 清除消息计数器，以开启新的一轮消息记录
<pre>
m_oMsgCounter.StartNewRound();
</pre>

5) 设置Prepare超时定时器。
{% highlight string %}
void Proposer :: OnPrepareTimeout()
{
    PLGHead("OK");

    if (GetInstanceID() != m_llTimeoutInstanceID)
    {
        PLGErr("TimeoutInstanceID %lu not same to NowInstanceID %lu, skip",
                m_llTimeoutInstanceID, GetInstanceID());
        return;
    }

    BP->GetProposerBP()->PrepareTimeout();
    
    Prepare(m_bWasRejectBySomeone);
}
{% endhighlight %}
Prepare超时原因有很多，比如网络丢包。当Prepare超时时，处理方式也很简单，重新执行Prepare()。注： 这里用```m_llTimeoutInstanceID```来标识一个Prepare超时定时器，因为可能有很多定时器同时在运行。

6) 发送Prepare消息
{% highlight string %}
int Base :: BroadcastMessage(const PaxosMsg & oPaxosMsg, const int iRunType, const int iSendType)
{
    if (m_bIsTestMode)
    {
        return 0;
    }

    BP->GetInstanceBP()->BroadcastMessage();

    if (iRunType == BroadcastMessage_Type_RunSelf_First)
    {
        if (m_poInstance->OnReceivePaxosMsg(oPaxosMsg) != 0)
        {
            return -1;
        }
    }
    
    string sBuffer;
    int ret = PackMsg(oPaxosMsg, sBuffer);
    if (ret != 0)
    {
        return ret;
    }

    ret = m_poMsgTransport->BroadcastMessage(m_poConfig->GetMyGroupIdx(), sBuffer, iSendType);

    if (iRunType == BroadcastMessage_Type_RunSelf_Final)
    {
        m_poInstance->OnReceivePaxosMsg(oPaxosMsg);
    }

    return ret;
}
{% endhighlight %}

这里在消息发送之前依然会再进行一次检查，看是否有必要进行发送。这里可能主要涉及到两个时间段，这两个时间段内整个集群的状态都可能会发生改变：

* Prepare准备过程所耗费的时间段

* Prepare定时器超时时间段（因为第二次Prepare()也同样会调用到BroadcastMessage()函数）

接着我们再来看对Paxos消息的打包：
{% highlight string %}
int Base :: PackMsg(const PaxosMsg & oPaxosMsg, std::string & sBuffer)
{
    std::string sBodyBuffer;
    bool bSucc = oPaxosMsg.SerializeToString(&sBodyBuffer);
    if (!bSucc)
    {
        PLGErr("PaxosMsg.SerializeToString fail, skip this msg");
        return -1;
    }

    int iCmd = MsgCmd_PaxosMsg;
    PackBaseMsg(sBodyBuffer, iCmd, sBuffer);

    return 0;
}
{% endhighlight %}
上面先调用protobuf相关序列化函数来进行序列化，之后再调用PackBaseMsg进行进一步的打包。打包后的消息类似如下：
<pre>
|------------------------------------------------------------------------------------------------------------------------
|              |     Header                              |                                               |              |
|    GroupID   |-----------------------------------------|                  PaxosMsg                     |    checksum  |
|              | header_len |   Header_content           |                                               |              |
|------------------------------------------------------------------------------------------------------------------------
|   group_id   |   len      | gid   | rid | cmd | version| MsgType | InstanceID  | NodeID   | ProposalID |  crc32_sum   |
|    4byte     |  (2Byte)   |8byte  |8byte|4byte| 4byte  |  4byte  |   8byte     |  8byte   |  8byte     |    4byte     |
-------------------------------------------------------------------------------------------------------------------------
</pre>
消息打完包之后，接着调用如下函数发往相应的节点：
{% highlight string %}
int Communicate :: BroadcastMessage(const int iGroupIdx, const std::string & sMessage, const int iSendType)
{
    const std::set<nodeid_t> & setNodeInfo = m_poConfig->GetSystemVSM()->GetMembershipMap();
    
    for (auto & it : setNodeInfo)
    {
        if (it != m_iMyNodeID)
        {
            Send(iGroupIdx, it, NodeInfo(it), sMessage, iSendType);
        }
    }

    return 0;
}
{% endhighlight %}
注意上面并不会发送给节点自身。后续收到其他节点的返回消息时，我们默认节点自身已经promise了。

### 2.4 MsgType_PaxosPrepareReply阶段
当收到发来的网络消息时，首先会回调Instance::OnReceive()函数，假如我们收到的是Paxos消息，则会回调到Instance::OnReceivePaxosMsg()函数，现在我们来看一下当收到*MsgType_PaxosPrepareReply*消息时的回调函数*Instance::ReceiveMsgForProposer()*:
{% highlight string %}
int Instance :: ReceiveMsgForProposer(const PaxosMsg & oPaxosMsg)
{
    if (m_poConfig->IsIMFollower())
    {
        PLGErr("I'm follower, skip this message");
        return 0;
    }

    ///////////////////////////////////////////////////////////////
    
    if (oPaxosMsg.instanceid() != m_oProposer.GetInstanceID())
    {
        if (oPaxosMsg.instanceid() + 1 == m_oProposer.GetInstanceID())
        {
            //Exipred reply msg on last instance.
            //If the response of a node is always slower than the majority node, 
            //then the message of the node is always ignored even if it is a reject reply.
            //In this case, if we do not deal with these reject reply, the node that 
            //gave reject reply will always give reject reply. 
            //This causes the node to remain in catch-up state.
            //
            //To avoid this problem, we need to deal with the expired reply.
            if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply)
            {
                m_oProposer.OnExpiredPrepareReply(oPaxosMsg);
            }
            else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply)
            {
                m_oProposer.OnExpiredAcceptReply(oPaxosMsg);
            }
        }

        BP->GetInstanceBP()->OnReceivePaxosProposerMsgInotsame();
        //PLGErr("InstanceID not same, skip msg");
        return 0;
    }

    if (oPaxosMsg.msgtype() == MsgType_PaxosPrepareReply)
    {
        m_oProposer.OnPrepareReply(oPaxosMsg);
    }
    else if (oPaxosMsg.msgtype() == MsgType_PaxosAcceptReply)
    {
        m_oProposer.OnAcceptReply(oPaxosMsg);
    }

    return 0;
}
{% endhighlight %}
在这里我们首先会处理前一个paxos instance所返回来的过期(Expired)消息，因为假如集群中某个节点一直落后于整个集群的话，那么当该节点返回reject响应时我们可能仍需要忽略。若我们不处理这些reject响应，则使得该节点后续可能都会一直返回reject响应。

接着调用Proposer::OnPrepareReply()来处理prepare响应：
{% highlight string %}
void Proposer :: OnPrepareReply(const PaxosMsg & oPaxosMsg)
{
    PLGHead("START Msg.ProposalID %lu State.ProposalID %lu Msg.from_nodeid %lu RejectByPromiseID %lu",
            oPaxosMsg.proposalid(), m_oProposerState.GetProposalID(), 
            oPaxosMsg.nodeid(), oPaxosMsg.rejectbypromiseid());

    BP->GetProposerBP()->OnPrepareReply();
    
    if (!m_bIsPreparing)
    {
        BP->GetProposerBP()->OnPrepareReplyButNotPreparing();
        //PLGErr("Not preparing, skip this msg");
        return;
    }

    if (oPaxosMsg.proposalid() != m_oProposerState.GetProposalID())
    {
        BP->GetProposerBP()->OnPrepareReplyNotSameProposalIDMsg();
        //PLGErr("ProposalID not same, skip this msg");
        return;
    }

    m_oMsgCounter.AddReceive(oPaxosMsg.nodeid());

    if (oPaxosMsg.rejectbypromiseid() == 0)
    {
        BallotNumber oBallot(oPaxosMsg.preacceptid(), oPaxosMsg.preacceptnodeid());
        PLGDebug("[Promise] PreAcceptedID %lu PreAcceptedNodeID %lu ValueSize %zu", 
                oPaxosMsg.preacceptid(), oPaxosMsg.preacceptnodeid(), oPaxosMsg.value().size());
        m_oMsgCounter.AddPromiseOrAccept(oPaxosMsg.nodeid());
        m_oProposerState.AddPreAcceptValue(oBallot, oPaxosMsg.value());
    }
    else
    {
        PLGDebug("[Reject] RejectByPromiseID %lu", oPaxosMsg.rejectbypromiseid());
        m_oMsgCounter.AddReject(oPaxosMsg.nodeid());
        m_bWasRejectBySomeone = true;
        m_oProposerState.SetOtherProposalID(oPaxosMsg.rejectbypromiseid());
    }

    if (m_oMsgCounter.IsPassedOnThisRound())
    {
        int iUseTimeMs = m_oTimeStat.Point();
        BP->GetProposerBP()->PreparePass(iUseTimeMs);
        PLGImp("[Pass] start accept, usetime %dms", iUseTimeMs);
        m_bCanSkipPrepare = true;
        Accept();
    }
    else if (m_oMsgCounter.IsRejectedOnThisRound()
            || m_oMsgCounter.IsAllReceiveOnThisRound())
    {
        BP->GetProposerBP()->PrepareNotPass();
        PLGImp("[Not Pass] wait 30ms and restart prepare");
        AddPrepareTimer(OtherUtils::FastRand() % 30 + 10);
    }

    PLGHead("END");
}
{% endhighlight %}
此函数按如下步骤对响应消息进行处理：

1） 首先当前Proposer检查自身是否处于preparing阶段

2） 检查proposalID是否相同

3） MsgCounter用于记录当前收到来自哪些节点的响应，以及收到的响应结果（promised/reject)

4) 如果本轮prepare通过，那么进入Accept()；否则开启定时器以进行下一轮的投票。

### 2.5 发起Accept请求
当Prepare阶段成功之后，proposer就会发起Accept请求：
{% highlight string %}
void Proposer :: Accept()
{
    PLGHead("START ProposalID %lu ValueSize %zu ValueLen %zu", 
            m_oProposerState.GetProposalID(), m_oProposerState.GetValue().size(), m_oProposerState.GetValue().size());

    BP->GetProposerBP()->Accept();
    m_oTimeStat.Point();
    
    ExitPrepare();
    m_bIsAccepting = true;
    
    PaxosMsg oPaxosMsg;
    oPaxosMsg.set_msgtype(MsgType_PaxosAccept);
    oPaxosMsg.set_instanceid(GetInstanceID());
    oPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oPaxosMsg.set_proposalid(m_oProposerState.GetProposalID());
    oPaxosMsg.set_value(m_oProposerState.GetValue());
    oPaxosMsg.set_lastchecksum(GetLastChecksum());

    m_oMsgCounter.StartNewRound();

    AddAcceptTimer();

    PLGHead("END");

    BroadcastMessage(oPaxosMsg, BroadcastMessage_Type_RunSelf_Final);
}
{% endhighlight %}
1) 生成PaxosMsg
<pre>
|-------------------------------------------------------------------------------------------
|  MsgType  |  InstanceID   |   NodeID    |  ProposalID   |    Value     |    LastChecksum  |
|   4Byte   |    8Byte      |   8Byte     |    8Byte      |     变长      |      4Byte      |
----------------------------------------------------------------------------------------------
</pre>

2) 添加Accept超时定时器，比如网络丢包。当Accept超时时，处理方式也很简单，重新从Prepare开始

3） 向集群各个节点发起Accept请求

### 2.6 对Accept响应的处理
当收到对应消息时，首先会回调*Instance::OnReceive()*函数，对于Paxos消息会回调到*Instance::OnReceivePaxosMsg()*，对于MsgType_PaxosAcceptReply消息，会回调*Instance::ReceiveMsgForProposer()*，接着再调用到*Proposer::OnAcceptReply()*，现在我们来看一看该函数：
{% highlight string %}
void Proposer :: OnAcceptReply(const PaxosMsg & oPaxosMsg)
{
    PLGHead("START Msg.ProposalID %lu State.ProposalID %lu Msg.from_nodeid %lu RejectByPromiseID %lu",
            oPaxosMsg.proposalid(), m_oProposerState.GetProposalID(), 
            oPaxosMsg.nodeid(), oPaxosMsg.rejectbypromiseid());

    BP->GetProposerBP()->OnAcceptReply();

    if (!m_bIsAccepting)
    {
        //PLGErr("Not proposing, skip this msg");
        BP->GetProposerBP()->OnAcceptReplyButNotAccepting();
        return;
    }

    if (oPaxosMsg.proposalid() != m_oProposerState.GetProposalID())
    {
        //PLGErr("ProposalID not same, skip this msg");
        BP->GetProposerBP()->OnAcceptReplyNotSameProposalIDMsg();
        return;
    }

    m_oMsgCounter.AddReceive(oPaxosMsg.nodeid());

    if (oPaxosMsg.rejectbypromiseid() == 0)
    {
        PLGDebug("[Accept]");
        m_oMsgCounter.AddPromiseOrAccept(oPaxosMsg.nodeid());
    }
    else
    {
        PLGDebug("[Reject]");
        m_oMsgCounter.AddReject(oPaxosMsg.nodeid());

        m_bWasRejectBySomeone = true;

        m_oProposerState.SetOtherProposalID(oPaxosMsg.rejectbypromiseid());
    }

    if (m_oMsgCounter.IsPassedOnThisRound())
    {
        int iUseTimeMs = m_oTimeStat.Point();
        BP->GetProposerBP()->AcceptPass(iUseTimeMs);
        PLGImp("[Pass] Start send learn, usetime %dms", iUseTimeMs);
        ExitAccept();
        m_poLearner->ProposerSendSuccess(GetInstanceID(), m_oProposerState.GetProposalID());
    }
    else if (m_oMsgCounter.IsRejectedOnThisRound()
            || m_oMsgCounter.IsAllReceiveOnThisRound())
    {
        BP->GetProposerBP()->AcceptNotPass();
        PLGImp("[Not pass] wait 30ms and Restart prepare");
        AddAcceptTimer(OtherUtils::FastRand() % 30 + 10);
    }

    PLGHead("END");
}
{% endhighlight %}

1) 若当前并不accepting阶段，直接返回；

2） 若proposalID不匹配，直接返回

3） MsgCounter记录相关的返回消息

4） 如果本轮Accept被采纳，则向Learnner发出通知消息；否则重新发起Prepare请求

## 3. Acceptor
Acceptor作为提案的被动参与者，也分为OnPrepare和OnAccept阶段。

### 3.1 OnPrepare阶段
当收到其他节点发送过来的Prepare请求时，首先会调用到*Instance::OnReceive()*，之后再回调到*Instance::OnReceivePaxosMsg()*，之后经过相关调用回调到*Acceptor :: OnPrepare()*:
{% highlight string %}
int Acceptor :: OnPrepare(const PaxosMsg & oPaxosMsg)
{
    PLGHead("START Msg.InstanceID %lu Msg.from_nodeid %lu Msg.ProposalID %lu",
            oPaxosMsg.instanceid(), oPaxosMsg.nodeid(), oPaxosMsg.proposalid());

    BP->GetAcceptorBP()->OnPrepare();
    
    PaxosMsg oReplyPaxosMsg;
    oReplyPaxosMsg.set_instanceid(GetInstanceID());
    oReplyPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oReplyPaxosMsg.set_proposalid(oPaxosMsg.proposalid());
    oReplyPaxosMsg.set_msgtype(MsgType_PaxosPrepareReply);

    BallotNumber oBallot(oPaxosMsg.proposalid(), oPaxosMsg.nodeid());
    
    if (oBallot >= m_oAcceptorState.GetPromiseBallot())
    {
        PLGDebug("[Promise] State.PromiseID %lu State.PromiseNodeID %lu "
                "State.PreAcceptedID %lu State.PreAcceptedNodeID %lu",
                m_oAcceptorState.GetPromiseBallot().m_llProposalID, 
                m_oAcceptorState.GetPromiseBallot().m_llNodeID,
                m_oAcceptorState.GetAcceptedBallot().m_llProposalID,
                m_oAcceptorState.GetAcceptedBallot().m_llNodeID);
        
        oReplyPaxosMsg.set_preacceptid(m_oAcceptorState.GetAcceptedBallot().m_llProposalID);
        oReplyPaxosMsg.set_preacceptnodeid(m_oAcceptorState.GetAcceptedBallot().m_llNodeID);

        if (m_oAcceptorState.GetAcceptedBallot().m_llProposalID > 0)
        {
            oReplyPaxosMsg.set_value(m_oAcceptorState.GetAcceptedValue());
        }

        m_oAcceptorState.SetPromiseBallot(oBallot);

        int ret = m_oAcceptorState.Persist(GetInstanceID(), GetLastChecksum());
        if (ret != 0)
        {
            BP->GetAcceptorBP()->OnPreparePersistFail();
            PLGErr("Persist fail, Now.InstanceID %lu ret %d",
                    GetInstanceID(), ret);
            
            return -1;
        }

        BP->GetAcceptorBP()->OnPreparePass();
    }
    else
    {
        BP->GetAcceptorBP()->OnPrepareReject();

        PLGDebug("[Reject] State.PromiseID %lu State.PromiseNodeID %lu", 
                m_oAcceptorState.GetPromiseBallot().m_llProposalID, 
                m_oAcceptorState.GetPromiseBallot().m_llNodeID);
        
        oReplyPaxosMsg.set_rejectbypromiseid(m_oAcceptorState.GetPromiseBallot().m_llProposalID);
    }

    nodeid_t iReplyNodeID = oPaxosMsg.nodeid();

    PLGHead("END Now.InstanceID %lu ReplyNodeID %lu",
            GetInstanceID(), oPaxosMsg.nodeid());;

    SendMessage(iReplyNodeID, oReplyPaxosMsg);

    return 0;
}
{% endhighlight %}
OnPrepare函数看似并未做任何有效性校验，但这部分校验是必不可少的，并未省去，而是出现在了调用OnPrepare的Instance类的上层函数中。这里的校验主要是保证参数中的instance ID与acceptor一致。

若发过来的提案编号（BallotNumber)```大于等于```当前Acceptor所承诺(promised)的提案编号，则对对新发送过来的提案编号进行promise，并返回其上一次所承诺的提案信息给proposer； 否则，则返回拒绝消息给proposer。

我们在进行promise时，会把当前所promise的信息写入到LevelDB与LogStore当中。写入到LogStore中的消息格式如下：
<pre>
|------------------------------------------------------------------
|  Length   |   InstanceID      |    AcceptorStateData            |
|   4Byte   |     8Byte         |                                 |
-------------------------------------------------------------------

1) Length字段用于保存InstanceID与AcceptorStateData所占用的空间总和
2) AcceptorStateData消息格式如下
message AcceptorStateData
{
	required uint64 InstanceID = 1;
	required uint64 PromiseID = 2;
	required uint64 PromiseNodeID = 3;
	required uint64 AcceptedID = 4;
	required uint64 AcceptedNodeID = 5;
	required bytes AcceptedValue = 6;
	required uint32 Checksum = 7;
}
</pre>

写入到LevelDB的消息格式如下：
<pre>
-------------------------------------------------------------
|        Key         |         Value                        |
-------------------------------------------------------------

1) key为instanceID
2) Value为本记录在LogStore中的位置，包括logstore文件名、偏移、校验值。logstore文件名还记得以前我们讲述过的vfile目录下的0.f、1.f这样的文件吗？
</pre>

### 3.2 OnAccept阶段
当收到其他节点发送过来的Accept请求时，会回调执行到Acceptor::OnAccess()函数：
{% highlight string %}
void Acceptor :: OnAccept(const PaxosMsg & oPaxosMsg)
{
    PLGHead("START Msg.InstanceID %lu Msg.from_nodeid %lu Msg.ProposalID %lu Msg.ValueLen %zu",
            oPaxosMsg.instanceid(), oPaxosMsg.nodeid(), oPaxosMsg.proposalid(), oPaxosMsg.value().size());

    BP->GetAcceptorBP()->OnAccept();

    PaxosMsg oReplyPaxosMsg;
    oReplyPaxosMsg.set_instanceid(GetInstanceID());
    oReplyPaxosMsg.set_nodeid(m_poConfig->GetMyNodeID());
    oReplyPaxosMsg.set_proposalid(oPaxosMsg.proposalid());
    oReplyPaxosMsg.set_msgtype(MsgType_PaxosAcceptReply);

    BallotNumber oBallot(oPaxosMsg.proposalid(), oPaxosMsg.nodeid());

    if (oBallot >= m_oAcceptorState.GetPromiseBallot())
    {
        PLGDebug("[Promise] State.PromiseID %lu State.PromiseNodeID %lu "
                "State.PreAcceptedID %lu State.PreAcceptedNodeID %lu",
                m_oAcceptorState.GetPromiseBallot().m_llProposalID, 
                m_oAcceptorState.GetPromiseBallot().m_llNodeID,
                m_oAcceptorState.GetAcceptedBallot().m_llProposalID,
                m_oAcceptorState.GetAcceptedBallot().m_llNodeID);

        m_oAcceptorState.SetPromiseBallot(oBallot);
        m_oAcceptorState.SetAcceptedBallot(oBallot);
        m_oAcceptorState.SetAcceptedValue(oPaxosMsg.value());
        
        int ret = m_oAcceptorState.Persist(GetInstanceID(), GetLastChecksum());
        if (ret != 0)
        {
            BP->GetAcceptorBP()->OnAcceptPersistFail();

            PLGErr("Persist fail, Now.InstanceID %lu ret %d",
                    GetInstanceID(), ret);
            
            return;
        }

        BP->GetAcceptorBP()->OnAcceptPass();
    }
    else
    {
        BP->GetAcceptorBP()->OnAcceptReject();

        PLGDebug("[Reject] State.PromiseID %lu State.PromiseNodeID %lu", 
                m_oAcceptorState.GetPromiseBallot().m_llProposalID, 
                m_oAcceptorState.GetPromiseBallot().m_llNodeID);
        
        oReplyPaxosMsg.set_rejectbypromiseid(m_oAcceptorState.GetPromiseBallot().m_llProposalID);
    }

    nodeid_t iReplyNodeID = oPaxosMsg.nodeid();

    PLGHead("END Now.InstanceID %lu ReplyNodeID %lu",
            GetInstanceID(), oPaxosMsg.nodeid());

    SendMessage(iReplyNodeID, oReplyPaxosMsg);
}
{% endhighlight %}
如果收到的Accept请求的提案编号（BallotNumber)```大于等于```当前Acceptor所承诺(promised)的，那么会对发送过来的提案进行promise，并持久化到LevelDB与PaxosLog中，持久化格式与上面OnPrepare中的一模一样； 否则返回拒绝消息给proposer。


## 4. 总结
本章简要介绍了Paxos算法的原理，了解到Paxos算法的三大角色：Proposer、Acceptor、Learner。讲解了Proposer、Acceptor两个角色的主要代码实现，以及二者如何参与到Prepare、Accept两个阶段中。

至于最后一个角色Learner，原本的理解认为应该是参与度最低的，逻辑最少的角色。但PhxPaxos中，Learner是三者中实现最复杂的，这部分内容将在下一章单独讲解。



<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之Proposer、Acceptor](https://www.jianshu.com/p/2a78c6215e6d)

<br />
<br />
<br />


