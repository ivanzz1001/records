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

## 3. 零拷贝
这里的拷贝指的是数据在内核缓冲区和应用程序缓冲区直接的传输，并非指进程空间中的内存拷贝（当然这方面也可以实现零拷贝，如传引用和 C++中 move 操作）。现在假设我们有个服务，提供用户下载某个文件，当请求到来时，我们把服务器磁盘上的数据发送到网络中，这个流程伪代码如下：
{% highlight string %}
filefd = open(...);         //打开文件
sockfd = socket(...);       //打开socket
buffer = new buffer(...);   //创建buffer
read(filefd, buffer);       //从文件内容读到buffer中
write(sockfd, buffer);      //将buffer中的内容发送到网络
{% endhighlight %}
数据拷贝流程如下图：
![数据拷贝流程](https://ivanzz1001.github.io/records/assets/img/distribute/distri-arch-copy.webp)

上图中绿色箭头表示 DMA copy，DMA（Direct Memory Access）即直接存储器存取，是一种快速传送数据的机制，指外部设备不通过 CPU 而直接与系统内存交换数据的接口技术。红色箭头表示 CPU copy。即使在有 DMA 技术的情况下还是存在 4 次拷贝，DMA copy 和 CPU copy 各 2 次。

### 3.1 内存映射

内存映射将用户空间的一段内存区域映射到内核空间，用户对这段内存区域的修改可以直接反映到内核空间，同样，内核空间对这段区域的修改也直接反映用户空间，简单来说就是用户空间共享这个内核缓冲区。

使用内存映射来改写后的伪代码如下：
{% highlight string %}
filefd = open(...);         //打开文件
sockfd = socket(...);       //打开socket
buffer = mmap(filefd);      //将文件映射到进程空间
write(sockfd, buffer);      //将buffer中的内容发送到网络
{% endhighlight %}
使用内存映射后数据拷贝流如下图所示:
![内存映射](https://ivanzz1001.github.io/records/assets/img/distri-arch-mmap.webp)
从图中可以看出，采用内存映射后数据拷贝减少为 3 次，不再经过应用程序直接将内核缓冲区中的数据拷贝到 Socket 缓冲区中。RocketMQ 为了消息存储高性能，就使用了内存映射机制，将存储文件分割成多个大小固定的文件，基于内存映射执行顺序写

### 3.2 零拷贝
零拷贝就是一种避免 CPU 将数据从一块存储拷贝到另外一块存储，从而有效地提高数据传输效率的技术。Linux 内核 2.4 以后，支持带有 DMA 收集拷贝功能的传输，将内核页缓存中的数据直接打包发到网络上，伪代码如下：
{% highlight string %}
filefd = open(...);           //打开文件
sockfd = socket(...);        //打开socket
sendfile(sockfd, filefd);    //将文件内容发送到网络
{% endhighlight %}

使用零拷贝后流程如下图：
![零拷贝](https://ivanzz1001.github.io/records/assets/img/distri-arch-zerocopy.webp)

零拷贝的步骤为: 1）DMA 将数据拷贝到 DMA 引擎的内核缓冲区中；2）将数据位置和长度信息的描述符加到套接字缓冲区；3）DMA 引擎直接将数据从内核缓冲区传递到协议引擎；

可以看出，零拷贝并非真正的没有拷贝，还是有 2 次内核缓冲区的 DMA 拷贝，只是消除了内核缓冲区和用户缓冲区之间的 CPU 拷贝。Linux 中主要的零拷贝函数有 sendfile、splice、tee 等。下图是来住 IBM 官网上普通传输和零拷贝传输的性能对比，可以看出零拷贝比普通传输快了 3 倍左右，Kafka 也使用零拷贝技术。

![普通读写和零拷贝性能对比](https://ivanzz1001.github.io/records/assets/img/distri-arch-cmpcopy.webp)

### 4. 序列化
当将数据写入文件、发送到网络、写入到存储时通常需要序列化（serialization）技术，从其读取时需要进行反序列化（deserialization），又称编码（encode）和解码（decode）。序列化作为传输数据的表示形式，与网络框架和通信协议是解耦的。如网络框架 taf 支持 jce、json 和自定义序列化，HTTP 协议支持 XML、JSON 和流媒体传输等。

序列化的方式很多，作为数据传输和存储的基础，如何选择合适的序列化方式尤其重要。

### 4.1 分类
通常而言，序列化技术可以大致分为以下三种类型：

* 内置类型：指编程语言内置支持的类型，如 java 的 java.io.Serializable。这种类型由于与语言绑定，不具有通用性，而且一般性能不佳，一般只在局部范围内使用。

* 文本类型：一般是标准化的文本格式，如 XML、JSON。这种类型可读性较好，且支持跨平台，具有广泛的应用。主要缺点是比较臃肿，网络传输占用带宽大。

* 二进制类型：采用二进制编码，数据组织更加紧凑，支持多语言和多平台。常见的有 Protocol Buffer/Thrift/MessagePack/FlatBuffer 等。

### 4.2 性能指标
衡量序列化/反序列化主要有三个指标：1）序列化之后的字节大小；2）序列化/反序列化的速度；3）CPU 和内存消耗；

下图是一些常见的序列化框架性能对比：

![序列化与反序列化速度对比](https://ivanzz1001.github.io/records/assets/img/distri-arch-serializable1.webp)

![序列化与反序列化字节占用对比](https://ivanzz1001.github.io/records/assets/img/distri-arch-serializable2.webp)

可以看出 Protobuf 无论是在序列化速度上还是字节占比上可以说是完爆同行。不过人外有人，天外有天，听说 FlatBuffer 比 Protobuf 更加无敌，下图是来自 Google 的 FlatBuffer 和其他序列化性能对比，光看图中数据 FB 貌似秒杀 PB 的存在:

![FlatBuffer性能对比](https://ivanzz1001.github.io/records/assets/img/distri-arch-serializable3.webp)

### 4.3 选型考量
在设计和选择序列化技术时，要进行多方面的考量，主要有以下几个方面：
1）性能：CPU 和字节占用大小是序列化的主要开销。在基础的 RPC 通信、存储系统和高并发业务上应该选择高性能高压缩的二进制序列化。一些内部服务、请求较少 Web 的应用可以采用文本的 JSON，浏览器直接内置支持 JSON。

2）易用性：丰富数据结构和辅助工具能提高易用性，减少业务代码的开发量。现在很多序列化框架都支持 List、Map 等多种结构和可读的打印。

3）通用性：现代的服务往往涉及多语言、多平台，能否支持跨平台跨语言的互通是序列化选型的基本条件。

4）兼容性：现代的服务都是快速迭代和升级，一个好的序列化框架应该有良好的向前兼容性，支持字段的增减和修改等。

5）扩展性：序列化框架能否低门槛的支持自定义的格式有时候也是一个比较重要的考虑因素。

## 5. 池子化
池化恐怕是最常用的一种技术了，其本质就是通过创建池子来提高对象复用，减少重复创建、销毁的开销。常用的池化技术有内存池、线程池、连接池、对象池等。

### 5.1 内存池
我们都知道，在 C/C++中分别使用 malloc/free 和 new/delete 进行内存的分配，其底层调用系统调用 sbrk/brk。频繁的调用系统调用分配释放内存不但影响性能还容易造成内存碎片，内存池技术旨在解决这些问题。正是这些原因，C/C++中的内存操作并不是直接调用系统调用，而是已经实现了自己的一套内存管理，malloc 的实现主要有三大实现。

* ptmalloc：glibc 的实现。

* tcmalloc：Google 的实现。

* jemalloc：Facebook 的实现。

下面是来自网上的三种 malloc 的比较图，tcmalloc 和 jemalloc 性能差不多，ptmalloc 的性能不如两者，我们可以根据需要选用更适合的 malloc，如 redis 和 mysql 都可以指定使用哪个 malloc。至于三者的实现和差异，可以网上查阅。

![内存分配性能对比](https://ivanzz1001.github.io/records/assets/img/distri-arch-memalloc.webp)

虽然标准库的实现在操作系统内存管理的基础上再加了一层内存管理，但应用程序通常也会实现自己特定的内存池，如为了引用计数或者专门用于小对象分配。所以看起来内存管理一般分为三个层次。

![内存管理三个层次](https://ivanzz1001.github.io/records/assets/img/distr-arch-memlayer.webp)


### 5.2 线程池
线程创建是需要分配资源的，这存在一定的开销，如果我们一个任务就创建一个线程去处理，这必然会影响系统的性能。线程池的可以限制线程的创建数量并重复使用，从而提高系统的性能。

线程池可以分类或者分组，不同的任务可以使用不同的线程组，可以进行隔离以免互相影响。对于分类，可以分为核心和非核心，核心线程池一直存在不会被回收，非核心可能对空闲一段时间后的线程进行回收，从而节省系统资源，等到需要时在按需创建放入池子中。

### 5.3 连接池
常用的连接池有数据库连接池、redis 连接池、TCP 连接池等等，其主要目的是通过复用来减少创建和释放连接的开销。连接池实现通常需要考虑以下几个问题：

1）初始化：启动即初始化和惰性初始化。启动初始化可以减少一些加锁操作和需要时可直接使用，缺点是可能造成服务启动缓慢或者启动后没有任务处理，造成资源浪费。惰性初始化是真正有需要的时候再去创建，这种方式可能有助于减少资源占用，但是如果面对突发的任务请求，然后瞬间去创建一堆连接，可能会造成系统响应慢或者响应失败，通常我们会采用启动即初始化的方式。

2）连接数目：权衡所需的连接数，连接数太少则可能造成任务处理缓慢，太多不但使任务处理慢还会过度消耗系统资源。

3）连接取出：当连接池已经无可用连接时，是一直等待直到有可用连接还是分配一个新的临时连接。

4）连接放入：当连接使用完毕且连接池未满时，将连接放入连接池（包括 3 中创建的临时连接），否则关闭。

5）连接检测：长时间空闲连接和失效连接需要关闭并从连接池移除。常用的检测方法有：使用时检测和定期检测。

### 5.4 对象池
严格来说，各种池都是对象池模式的应用，包括前面的这三哥们。对象池跟各种池一样，也是缓存一些对象从而避免大量创建同一个类型的对象，同时限制了实例的个数。如 redis 中 0-9999 整数对象就通过采用对象池进行共享。在游戏开发中对象池模式经常使用，如进入地图时怪物和 NPC 的出现并不是每次都是重新创建，而是从对象池中取出。

## 6. 并发化
### 6.1 请求并发
如果一个任务需要处理多个子任务，可以将没有依赖关系的子任务并发化，这种场景在后台开发很常见。如一个请求需要查询 3 个数据，分别耗时 T1、T2、T3，如果串行调用总耗时 T=T1+T2+T3。对三个任务执行并发，总耗时 T=max(T1,T 2,T3)。同理，写操作也如此。对于同种请求，还可以同时进行批量合并，减少 RPC 调用次数。

### 6.2 冗余请求
冗余请求指的是同时向后端服务发送多个同样的请求，谁响应快就使用谁，其他的则丢弃。这种策略缩短了客户端的等待时间，但也使整个系统调用量猛增，一般适用于初始化或者请求少的场景。公司 WNS 的跑马模块其实就是这种机制，跑马模块为了快速建立长连接同时向后台多个 ip/port 发起请求，谁快就用谁，这在弱网的移动设备上特别有用，如果使用等待超时再重试的机制，无疑将大大增加用户的等待时间。

## 7. 异步化
对于处理耗时的任务，如果采用同步等待的方式，会严重降低系统的吞吐量，可以通过异步化进行解决。异步在不同层面概念是有一些差异的，在这里我们不讨论异步 I/O。

### 7.1 调用异步化
在进行一个耗时的 RPC 调用或者任务处理时，常用的异步化方式如下：

* Callback：异步回调通过注册一个回调函数，然后发起异步任务，当任务执行完毕时会回调用户注册的回调函数，从而减少调用端等待时间。这种方式会造成代码分散难以维护，定位问题也相对困难。

* Future：当用户提交一个任务时会立刻先返回一个 Future，然后任务异步执行，后续可以通过 Future 获取执行结果。对 1.4.1 中请求并发，我们可以使用 Future 实现，伪代码如下：
{% highlight string %}
//异步并发任务
Future<Response> f1 = Executor.submit(query1);
Future<Response> f2 = Executor.submit(query2);
Future<Response> f3 = Executor.submit(query3);

//处理其他事情
doSomething();

//获取结果
Response res1 = f1.getResult();
Response res2 = f2.getResult();
Response res3 = f3.getResult();
{% endhighlight %}

* CPS（Continuation-passing style）可以对多个异步过程进行编排，组成更复杂的异步处理，并以同步的代码调用形式实现异步效果。CPS 将后续的处理逻辑当作参数传递给 Then 并可以最终捕获异常，解决了异步回调代码散乱和异常跟踪难的问题。Java 中的 CompletableFuture 和 C++ PPL 基本支持这一特性。典型的调用形式如下：
{% highlight string %}
void handleRequest(const Request &req)
{
    return req.Read().Then([](Buffer &inbuf){
        return handleData(inbuf);
    }).Then([](Buffer &outbuf){
        return handleWrite(outbuf);
    }).Finally(){
        return cleanUp();
    });
}
{% endhighlight %}

### 7.2  流程异步化

一个业务流程往往伴随着调用链路长、后置依赖多等特点，这会同时降低系统的可用性和并发处理能力。可以采用对非关键依赖进行异步化解决。如企鹅电竞开播服务，除了开播写节目存储以外，还需要将节目信息同步到神盾推荐平台、App 首页和二级页等。由于同步到外部都不是开播的关键逻辑且对一致性要求不是很高，可以对这些后置的同步操作进行异步化，写完存储即向 App 返回响应，如下图所示：

![企鹅电竞开播流程异步化](https://ivanzz1001.github.io/records/assets/img/distri-arch-async.webp)

## 8. 缓存
从单核 CPU 到分布式系统，从前端到后台，缓存无处不在。古有朱元璋“缓称王”而终得天下，今有不论是芯片制造商还是互联网公司都同样采取了“缓称王”（缓存称王）的政策才能占据一席之地。缓存是原始数据的一个复制集，其本质就是空间换时间，主要是为了解决高并发读。

### 8.1 缓存的使用场景
缓存是空间换时间的艺术，使用缓存能提高系统的性能。“劲酒虽好，可不要贪杯”，使用缓存的目的是为了提高性价比，而不是一上来就为了所谓的提高性能不计成本的使用缓存，而是要看场景。

适合使用缓存的场景，以之前参与过的项目企鹅电竞为例：

1）一旦生成后基本不会变化的数据：如企鹅电竞的游戏列表，在后台创建一个游戏之后基本很少变化，可直接缓存整个游戏列表；

2）读密集型或存在热点的数据：典型的就是各种 App 的首页，如企鹅电竞首页直播列表；

3）计算代价大的数据：如企鹅电竞的 Top 热榜视频，如 7 天榜在每天凌晨根据各种指标计算好之后缓存排序列表；

4）千人一面的数据：同样是企鹅电竞的 Top 热榜视频，除了缓存的整个排序列表，同时直接在进程内按页缓存了前 N 页数据组装后的最终回包结果；

不适合使用缓存的场景：

1）写多读少，更新频繁；

2）对数据一致性要求严格；

### 8.2 缓存的分类
* 进程级缓存：缓存的数据直接在进程地址空间内，这可能是访问速度最快使用最简单的缓存方式了。主要缺点是受制于进程空间大小，能缓存的数据量有限，进程重启缓存数据会丢失。一般通常用于缓存数据量不大的场景。

* 集中式缓存：缓存的数据集中在一台机器上，如共享内存。这类缓存容量主要受制于机器内存大小，而且进程重启后数据不丢失。常用的集中式缓存中间件有单机版 redis、memcache 等。

* 分布式缓存：缓存的数据分布在多台机器上，通常需要采用特定算法（如 Hash）进行数据分片，将海量的缓存数据均匀的分布在每个机器节点上。常用的组件有：Memcache（客户端分片）、Codis（代理分片）、Redis Cluster（集群分片）。

* 多级缓存：指在系统中的不同层级的进行数据缓存，以提高访问效率和减少对后端存储的冲击。以下图的企鹅电竞的一个多级缓存应用，根据我们的现网统计，在第一级缓存的命中率就已经达 94%，穿透到 grocery 的请求量很小。

![企鹅电竞首页多级缓存](https://ivanzz1001.github.io/records/assets/img/distr-arch-cache.webp)

整体工作流程如下：

1）请求到达首页或者直播间服务后，如果在本地缓存命中则直接返回，否则从下一级缓存核心存储进行查询并更新本地缓存；
2）前端服务缓存没有命中穿透到核心存储服务，如果命中则直接返回给前端服务，没有则请求存储层 grocery 并更新缓存；
3）前两级 Cache 都没有命中回源到存储层 grocery

### 8.3 缓存的模式
关于缓存的使用，已经有人总结出了一些模式，主要分为 Cache-Aside 和 Cache-As-SoR 两类。其中 SoR（system-of-record）：表示记录系统，即数据源，而 Cache 正是 SoR 的复制集。

* Cache-Aside：旁路缓存，这应该是最常见的缓存模式了。对于读，首先从缓存读取数据，如果没有命中则回源 SoR 读取并更新缓存。对于写操作，先写 SoR，再写缓存。这种模式架构图如下：

![Cache-Aside结构图](https://ivanzz1001.github.io/records/assets/img/distr-arch-cache-aside.webp)

逻辑代码：
{% highlight string %}
//读操作
data = Cache.get(key);
if(data == NULL)
{
    data = SoR.load(key);
    Cache.set(key, data);
}

//写操作
if(SoR.save(key, data))
{
    Cache.set(key, data);
}
{% endhighlight %}
这种模式用起来简单，但对应用层不透明，需要业务代码完成读写逻辑。同时对于写来说，写数据源和写缓存不是一个原子操作，可能出现以下情况导致两者数据不一致：

1）在并发写时，可能出现数据不一致。如下图所示，user1 和 user2 几乎同时进行读写。在 t1 时刻 user1 写 db，t2 时刻 user2 写 db，紧接着在 t3 时刻 user2 写缓存，t4 时刻 user1 写缓存。这种情况导致 db 是 user2 的数据，缓存是 user1 的数据，两者不一致。

![Cache-Aside并发读写](https://ivanzz1001.github.io/records/assets/img/distr-arch-cache-asidep1.webapp)

2）先写数据源成功，但是接着写缓存失败，两者数据不一致。对于这两种情况如果业务不能忍受，可简单的通过先 delete 缓存然后再写 db 解决，其代价就是下一次读请求的 cache miss。

* Cache-As-SoR：缓存即数据源，该模式把 Cache 当作 SoR，所以读写操作都是针对 Cache，然后 Cache 再将读写操作委托给 SoR，即 Cache 是一个代理。如下图所示：

![Cache-As-SoR结构图](https://ivanzz1001.github.io/records/assets/img/distr-arch-cache-sor.webapp)

Cache-As-SoR 有三种实现：

1）Read-Through：发生读操作时，首先查询 Cache，如果不命中则再由 Cache 回源到 SoR 即存储端实现 Cache-Aside 而不是业务）。

2）Write-Through：称为穿透写模式，由业务先调用写操作，然后由 Cache 负责写缓存和 SoR。

3）Write-Behind：称为回写模式，发生写操作时业务只更新缓存并立即返回，然后异步写 SoR，这样可以利用合并写/批量写提高性能。


### 8.4 缓存的回收策略
在空间有限、低频热点访问或者无主动更新通知的情况下，需要对缓存数据进行回收，常用的回收策略有以下几种：

1）基于时间：基于时间的策略主要可以分两种：

* 基于 TTL（Time To Live）：即存活期，从缓存数据创建开始到指定的过期时间段，不管有没有访问缓存都会过期。如 redis 的 EXPIRE。

* 基于 TTI（Time To Idle）：即空闲期，缓存在指定的时间没有被访问将会被回收。

2）基于空间：缓存设置了存储空间上限，当达到上限时按照一定的策略移除数据。

3）基于容量：缓存设置了存储条目上限，当达到上限时按照一定的策略移除数据。

4）基于引用：基于引用计数或者强弱引用的一些策略进行回收。

缓存的常见回收算法如下：

* FIFO（First In First Out）：先进选出原则，先进入缓存的数据先被移除。

* LRU（Least Recently Used）：最基于局部性原理，即如果数据最近被使用，那么它在未来也极有可能被使用，反之，如果数据很久未使用，那么未来被使用的概率也较。

* LFU：（Least Frequently Used）：最近最少被使用的数据最先被淘汰，即统计每个对象的使用次数，当需要淘汰时，选择被使用次数最少的淘汰。

### 8.5 缓存的崩溃与修复
由于在设计不足、请求攻击（并不一定是恶意攻击）等会造成一些缓存问题，下面列出了常见的缓存问题和解决方案。

1) 缓存穿透：大量使用不存在的 key 进行查询时，缓存没有命中，这些请求都穿透到后端的存储，最终导致后端存储压力过大甚至被压垮。这种情况原因一般是存储中数据不存在，主要有两个解决办法。

* 设置空置或默认值：如果存储中没有数据，则设置一个空置或者默认值缓存起来，这样下次请求时就不会穿透到后端存储。但这种情况如果遇到恶意攻击，不断的伪造不同的 key 来查询时并不能很好的应对，这时候需要引入一些安全策略对请求进行过滤。
* 布隆过滤器：采用布隆过滤器将，将所有可能存在的数据哈希到一个足够大的 bitmap 中，一个一定不存在的数据会被这个 bitmap 拦截掉，从而避免了对底层数据库的查询压力。

2) 缓存雪崩：指大量的缓存在某一段时间内集体失效，导致后端存储负载瞬间升高甚至被压垮。通常是以下原因造成：

* 缓存失效时间集中在某段时间，对于这种情况可以采取对不同的 key 使用不同的过期时间，在原来基础失效时间的基础上再加上不同的随机时间；
* 采用取模机制的某缓存实例宕机，这种情况移除故障实例后会导致大量的缓存不命中。有两种解决方案：① 采取主从备份，主节点故障时直接将从实例替换主；② 使用一致性哈希替代取模，这样即使有实例崩溃也只是少部分缓存不命中。

3) 缓存热点：虽然缓存系统本身性能很高，但也架不住某些热点数据的高并发访问从而造成缓存服务本身过载。假设一下微博以用户 id 作为哈希 key，突然有一天志玲姐姐宣布结婚了，如果她的微博内容按照用户 id 缓存在某个节点上，当她的万千粉丝查看她的微博时必然会压垮这个缓存节点，因为这个 key 太热了。这种情况可以通过生成多份缓存到不同节点上，每份缓存的内容一样，减轻单个节点访问的压力。

### 8.6 缓存的一些好实践



<br />
<br />

**[参看]:**

1. [后台服务架构高性能设计之道](https://zhuanlan.zhihu.com/p/513288895?utm_id=0)


<br />
<br />
<br />