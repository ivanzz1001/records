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

3） 





<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之Proposer、Acceptor](https://www.jianshu.com/p/2a78c6215e6d)

<br />
<br />
<br />


