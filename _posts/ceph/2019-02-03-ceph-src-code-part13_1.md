---
layout: post
title: ceph中对象读写的顺序性及并发性保证(转)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

分布式系统中经常需要考虑对象(或者记录、文件、数据块等）的读写顺序以及并发访问问题。通常来说，如果两个对象没有共享的资源，就可以进行并发的访问；如果有共享的部分，就需要对这部分资源进行加锁。而对于同一个对象的并发读写（尤其是并发写更新时），就需要注意顺序性以及并发访问的控制，以免数据错乱。本文主要针对ceph中对象读写的顺序及并发性保证机制进行介绍。



<!-- more -->

## 1. PG的概念
ceph中引入PG(Placement Group)的概念，这是一个逻辑上的组，通过crush算法映射到一组OSD上，每个PG里包含完整的副本关系，比如3个数据副本分布到一个PG里的3个不同的OSD上。下面引用ceph论文中的一张图就可以比较直观的理解了。将文件按照指定的对象大小分割成多个对象，每个对象根据hash映射到某个PG，然后再根据crush算法映射到后端的某几个OSD上：


![ceph-chapter13-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter13_1.png)

## 2. 不同对象的并发控制

不同的对象有可能落到同一个PG里，ceph实现里，再OSD的处理线程中就会给PG加锁，一直到queue_transactions里把事务放到journal的队列里（以filestore为例）才释放PG的锁。从这里可以看出，对于同一个PG里的不同对象，是通过PG锁来进行并发的控制，好在这个过程中没有涉及到对象的IO，不会太影响效率；对于不同PG的对象，就可以直接进行并发访问。
{% highlight string %}
void OSD::ShardedOpWQ::_process(uint32_t thread_index, heartbeat_handle_d *hb ) {
	...

	(item.first)->lock_suspend_timeout(tp_handle);

	...

	(item.first)->unlock();
}
{% endhighlight %}




## 3. 同一个对象的并发顺序控制

从上面的介绍可以得知，同一个对象的访问也会受到PG锁的限制，但这是在OSD的处理逻辑里。对于同一个对象的访问，要考虑的情况比较复杂。从用户使用场景上来说，有两种使用方式。比如：

1） 一个client情况，客户端对同一个对象的更新处理逻辑是串行的，要等前一次写请求完成，再进行后一次的读取或者写更新；

2） 多个client对同一个对象的并发访问，这样的应用场景类似于NFS，目前的分布式系统里很少能做到，涉及到多个client同时更新带来的数据一致性问题，一般需要集群文件系统的支持；

对于多client的场景，ceph的rbd也是不能保证的，cephfs或许可以（没深入研究过，待取证）。因此，这里主要以单client访问ceph rbd块设备的场景进行阐述，看一个极端的例子：同一个client先后发送了2次对同一个对象的异步写请求。以这个例子展开进行说明。


### 3.1 tcp消息的顺序性保证
了解tcp的都知道，tcp使用序号来保证消息的顺序。发送方每次发送数据时，tcp就给每个数据包分配一个序列号，并且在一个特定的时间内等待接收主机对分配的这个序列号进行确认，如果发送方在一个特定时间内没有收到接收方的确认，则发送放会重传此数据包。接收方利用序号对接收的数据进行确认，以便检测对方发送的数据是否有丢失或者乱序等，接收方一旦收到已经顺序化的数据，它就将这些数据按正确的顺序重组成数据流并传递到应用层进行处理。注意： tcp的序号时用来保证同一个tcp连接上消息的顺序性。

### 3.2 ceph消息层的顺序性保证
ceph的消息有个seq序号，以simple消息模型为例，Pipe里有3个序号：in_seq、in_seq_acked、out_seq。

* in_seq是对于Reader来说，已经收到的消息的序号

* in_seq_acked表示成功处理并应答的消息序号，接收端收到发送端的消息并处理完成后将ack成功发到发送端后设置的；

* out_seq是发送端生成的，一般新建一个连接时随机生成一个序号，然后后续的消息发送时out_seq递增赋值给消息的序号m->seq。

>注： 参看Pipe::reader()与Pipe::writer()函数
 
![ceph-chapter13-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter13_2.png)










<br />
<br />

**[参看]**

1. [ceph中对象读写的顺序性及并发性保证](http://sysnote.github.io/2016/08/29/ceph-io-sequence/)




<br />
<br />
<br />

