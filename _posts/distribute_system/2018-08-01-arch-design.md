---
layout: post
title: 后台服务架构高性能设计之道(转)
tags:
- 分布式系统
categories: distribute-systems
description: 后台服务架构高性能设计之道
---

```N高N可```, 高性能、高并发、高可用、高可靠、可扩展、可维护、可用性等是后台开发耳熟能详的词了，它们中有些词在大部分情况下表达相近意思。本序列文章旨在探讨和总结后台架构设计中常用的技术和方法，并归纳成一套方法论。



<!-- more -->

## 1. 前言

本文主要探讨和总结服务架构设计中高性能的技术和方法，如下图的思维导图所示，左边部分主要偏向于编程应用，右边部分偏向于组件应用，文章将按图中的内容展开。

![高性能思维导图](https://ivanzz1001.github.io/records/assets/img/distribute/distr-arch-mind.jpg)

## 2. 无锁化
大多数情况下，多线程处理可以提高并发性能，但如果对共享资源的处理不当，严重的锁竞争也会导致性能的下降。面对这种情况，有些场景采用了无锁化设计，特别是在底层框架上。无锁化主要有两种实现，串行无锁和数据结构无锁。

### 2.1 串行无锁
无锁串行最简单的实现方式可能就是单线程模型了，如 redis/Nginx 都采用了这种方式。在网络编程模型中，常规的方式是主线程负责处理 I/O 事件，并将读到的数据压入队列，工作线程则从队列中取出数据进行处理，这种半同步/半异步模型需要对队列进行加锁，如下图所示：

![单reactor多线程模型](https://ivanzz1001.github.io/records/assets/img/distribute/distri-arch-nolock1.webp)

上图的模式可以改成无锁串行的形式，当 MainReactor accept 一个新连接之后从众多的 SubReactor 选取一个进行注册，通过创建一个 Channel 与 I/O 线程进行绑定，此后该连接的读写都在同一个线程执行，无需进行同步。

![主从reactor责任链模型](https://ivanzz1001.github.io/records/assets/img/distribute/distri-arch-nolock2.webp)

### 2.2 结构无锁
利用硬件支持的原子操作可以实现无锁的数据结构，很多语言都提供 CAS 原子操作（如 go 中的 atomic 包和 C++11 中的 atomic 库），可以用于实现无锁队列。我们以一个简单的线程安全单链表的插入操作来看下无锁编程和普通加锁的区别。

{% highlight string %}
struct Node
{
    Node(const T &value) : data(value) { }
    T data;
    Node *next = nullptr;
};
{% endhighlight %}

* 有锁链表```WithLockList```:
{% highlight string %}
template<typename T>
class WithLockList
{
    mutex mtx;
    Node<T> *head;
public:
    void pushFront(const T &value)
    {
        auto *node = new Node<T>(value);
        lock_guard<mutex> lock(mtx); //①
        node->next = head;
        head = node;
    }
};
{% endhighlight %}

* 无锁链表
{% highlight string %}
template<typename T>
class LockFreeList
{
    atomic<Node<T> *> head;
public:
    void pushFront(const T &value)
    {
        auto *node = new Node<T>(value);
        node->next = head.load();
        while(!head.compare_exchange_weak(node->next, node)); //②
    }
};
{% endhighlight %}

从代码可以看出，在有锁版本中 ① 进行了加锁。在无锁版本中，② 使用了原子 CAS 操作 compare_exchange_weak，该函数如果存储成功则返回 true，同时为了防止伪失败（即原始值等于期望值时也不一定存储成功，主要发生在缺少单条比较交换指令的硬件机器上），通常将 CAS 放在循环中。

下面对有锁和无锁版本进行简单的性能比较，分别执行 1000,000 次 push 操作。测试代码如下：
{% highlight string %}
int main()
{
    const int SIZE = 1000000;
    //有锁测试
    auto start = chrono::steady_clock::now();
    WithLockList<int> wlList;
    for(int i = 0; i < SIZE; ++i)
    {
        wlList.pushFront(i);
    }
    auto end = chrono::steady_clock::now();
    chrono::duration<double, std::micro> micro = end - start;
    cout << "with lock list costs micro:" << micro.count() << endl;

    //无锁测试
    start = chrono::steady_clock::now();
    LockFreeList<int> lfList;
    for(int i = 0; i < SIZE; ++i)
    {
        lfList.pushFront(i);
    }
    end = chrono::steady_clock::now();
    micro = end - start;
    cout << "free lock list costs micro:" << micro.count() << endl;

    return 0;
}
{% endhighlight %}
三次输出如下，可以看出无锁版本有锁版本性能高一些:
<pre>
with lock list costs micro:548118 
free lock list costs micro:491570
with lock list costs micro:556037 
free lock list costs micro:476045 
with lock list costs micro:557451 
free lock list costs micro:481470
</pre>








<br />
<br />

**[参看]:**

1. [后台服务架构高性能设计之道](https://zhuanlan.zhihu.com/p/513288895?utm_id=0)


<br />
<br />
<br />