---
layout: post
title: phxpaxos源码分析： 归档机制
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. 基本概念
上一章我们讲解了PhxPaxos的状态机，各种场景下可以定制不同的状态机，每个状态机独立消费一个paxos log以驱动业务状态变更。

状态机消费完paxos log之后，paxos log是否可以删除呢？答案是不行。有如下几个原因：

* 某些paxos节点可能由于网络等原因落后于其他节点，需要学习现有的paxos log；

* 业务消费完paxos log之后，可能由于重启等原因出现数据丢失，需要通过paxos log做重放(replay)

那什么时候可以删除paxos log呢？答案是不知道。因为某个节点可能永远处于离线状态，这时候必须保留从最初到现在所有的paxos log。但另一方面，如果数据不删除将无限增长，这是无法忍受的。

PhxPaxos因此引入了Checkpoint机制，关于该机制的详细描述请参见[《状态机Checkpoint详解》](https://github.com/Tencent/phxpaxos/wiki/%E7%8A%B6%E6%80%81%E6%9C%BACheckpoint%E8%AF%A6%E8%A7%A3)，这里简要说明如下：

1） 一个Checkpoint代表着一份某一时刻被固化下来的状态机数据，它通过```sm.h```下的*StateMachine::GetCheckpointInstanceID()*函数反馈它的精确时刻；

2） 每次启动replay时，只需要从*GetCheckpointInstanceID()*所指向的paxos log位置开始，而不是从0开始；

3） Node::SetHoldPaxosLogCount()控制需要保留多少在StateMachine::GetCheckpointInstanceID()之前的paxos log

4) 保留一定数量的paxos log的目的在于，如果其他节点数据不对齐，可以通过保留的这部分paxos log完成对齐，而不需要checkpoint数据介入；

5) 如果对齐数据已被删除，这时需要Checkpoint数据传输；

## 2. 代码设计
Checkpoint机制相关类图如下：

![paxos-checkpoint](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_checkpoint.jpg)

参照上一节讲的功能及类图补充说明如下：

* **Replayer**： replay线程。当业务状态机已经消费指定paxos log后，交由Checkpoint重新执行。由Checkpoint relay的数据允许被删除

* **Cleaner**: paxos log清理线程，根据配置的保留条数等清理paxos log

* **CheckpointSender**: 归档数据发送线程。将归档数据发往其他节点，用于归档数据同步。

* **CheckpointReceiver**： 归档数据接收器。接收其他节点发送过来的归档数据。通常归档数据接收完成之后，就会调用*Learner::OnSendCheckpoint_End()*，之后要求我们LoadCheckpointState()

* **CheckpointMgr**： Checkpoint机制管理器，统一管理整个归档机制。

## 3. 状态机(StateMachine)

再来看StateMachine接口，这次我们重点关注Checkpoint相关接口：
{% highlight string %}
class StateMachine
{
public:

    virtual const uint64_t GetCheckpointInstanceID(const int iGroupIdx) const;

    virtual int LockCheckpointState();
    virtual int GetCheckpointState(const int iGroupIdx, std::string& sDirPath,
                                   std::vector<std::string>& vecFileList);
    virtual void UnLockCheckpointState();
    virtual int LoadCheckpointState(const int iGroupIdx, const std::string& sCheckpointTmpFileDirPath,
                                    const std::vector<std::string>& vecFileList, const uint64_t llCheckpointInstanceID);

    virtual bool ExecuteForCheckpoint(const int iGroupIdx, const uint64_t llInstanceID,
                                      const std::string& sPaxosValue);
};
{% endhighlight %}
接口说明如下：

* GetCheckpointInstanceID(): Checkpoint所指向的最大InstanceID，在此之前的paxos log数据状态机已经不需要了。但通常为了其他节点的数据对齐需要，我们仍然会保留其之前的SetHoldPaxosLogCount()个paxos log

* LockCheckpointState(): 用于开发者锁定状态机Checkpoint数据，这个锁定的意思是指这份数据文件不能被修改、移动和删除。因为接下来PhxPaxos就要将这些文件发送给其他节点，而如果这个过程中出现了修改，则发送的数据可能乱掉。

* GetCheckpointState(): PhxPaxos获取Checkpoint文件列表，从而将文件发送给其他节点

* UnLockCheckpointState(): 当PhxPaxos发送Checkpoint数据到其他节点后，会调用解锁函数，解除对开发者的状态机Checkpoint数据的锁定

* LoadCheckpointState(): 当一个节点获得来自其他节点的Checkpoint数据时，会调用这个函数，将这份数据交由开发者进行处理（开发者往往要做的事情就是将这份Checkpoint数据覆盖当前节点的数据），当调用此函数完成后，PhxPaxos将会进行进程自杀操作，通过重启来完成一个新Checkpoint数据的启动。

* ExecuteForCheckpoint(): paxos log归档的replay接口

## 4. Replayer
Replayer是一个独立的线程，负责将选中的提案值Checkpoint操作。实现逻辑非常简单，读取本机的Checkpoint InstanceID，定时和*Max Chosen InstanceID*比较，如果Checkpoint落后于*Max Chosen InstanceID*，则通过调用状态机的*ExecuteForCheckpoint()*进行重演:
{% highlight string %}
void Replayer :: run()
{
    PLGHead("Checkpoint.Replayer [START]");
    uint64_t llInstanceID = m_poSMFac->GetCheckpointInstanceID(m_poConfig->GetMyGroupIdx()) + 1;

    while (true)
    {
        if (m_bIsEnd)
        {
            PLGHead("Checkpoint.Replayer [END]");
            return;
        }

        if (!m_bCanrun)
        {
            //PLGImp("Pausing, sleep");
            m_bIsPaused = true;
            Time::MsSleep(1000);
            continue;
        }
        //本节点的所有提案值已全部重演
        if (llInstanceID >= m_poCheckpointMgr->GetMaxChosenInstanceID())
        {
            //PLGImp("now maxchosen instanceid %lu small than excute instanceid %lu, wait",
            //m_poCheckpointMgr->GetMaxChosenInstanceID(), llInstanceID);
            Time::MsSleep(1000);
            continue;
        }
        //重演
        bool bPlayRet = PlayOne(llInstanceID);

        if (bPlayRet)
        {
            PLGImp("Play one done, instanceid %lu", llInstanceID);
            llInstanceID++;
        }
        else
        {
            PLGErr("Play one fail, instanceid %lu", llInstanceID);
            Time::MsSleep(500);
        }
    }
}
{% endhighlight %}
PlayOne()的实现逻辑如下：
{% highlight string %}
bool Replayer :: PlayOne(const uint64_t llInstanceID)
{
    //读取数据库中的paxos log
    AcceptorStateData oState;
    int ret = m_oPaxosLog.ReadState(m_poConfig->GetMyGroupIdx(), llInstanceID, oState);

    if (ret != 0)
    {
        return false;
    }
    //调用状态机的ExecuteForCheckpoint
    bool bExecuteRet = m_poSMFac->ExecuteForCheckpoint(
                           m_poConfig->GetMyGroupIdx(), llInstanceID, oState.acceptedvalue());

    if (!bExecuteRet)
    {
        PLGErr("Checkpoint sm excute fail, instanceid %lu", llInstanceID);
    }

    return bExecuteRet;
}
{% endhighlight %}

## 5. Paxos Log清理(Cleaner)
在PhxPaxos中，每个Group启动一个Cleaner线程清理本Group的paxos log。在Group中，instanceID是不断递增的，每个instanceID对应一个paxos log，当加入Checkpoint之后，我们有三个关键的instanceID:

* **min Chosen InstanceID**： 本节点，选定提案的最小instanceID，即最老的paxos log所对应的instanceID

* **Checkpoint instanceID**: 本节点，Checkpoint确定的最大instanceID，即在这之前的paxos log数据Checkpoint都已经不需要了。

* **max Chosen InstanceID**: 本节点，选定提案的最大instanceID，即最新的paxos log所对应的instanceID

>注意: 这里强调的是*本节点*， 不同节点数据可能不同

除此之外，还有一个配置： Node::SetHoldPaxosLogCount()控制需要保留多少在StateMachine::GetCheckpointInstanceID()之前的PaxosLog。保留一定数量的PaxosLog的目的在于，如果其他节点数据不对齐，可以通过保留的这部分paxos log完成对齐，而不需要checkpoint数据介入。

正常情况下，三者关系如下：

![paxos-instance-id](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_instance_id.jpg)



<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之状态机](https://www.jianshu.com/p/89377cc9b405)

4. [如何进行成员变更](https://github.com/Tencent/phxpaxos/wiki/%E5%A6%82%E4%BD%95%E8%BF%9B%E8%A1%8C%E6%88%90%E5%91%98%E5%8F%98%E6%9B%B4)

5. [一致性协议](https://www.jianshu.com/p/0b475b430abe?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)

<br />
<br />
<br />


