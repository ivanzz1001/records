---
layout: post
title: ceph网络通信
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本章我们介绍一下Ceph的网络通信模块，这是客户端和服务器通信的底层模块，用来在客户端和服务器之间接收和发送请求。其实现功能比较清晰，是一个相对比较独立的模块，理解起来比较容易。

<!-- more -->


## 1. Ceph网络通信框架
一个分布式存储系统需要一个稳定的底层网络通信模块，用于各节点之间互联互通。对于一个网络通信系统，要求如下：

* 高性能。性能评价的两个指标：```带宽```和```延迟```

* 稳定可靠。数据不丢包，在网络中断时，实现重连等异常处理。

网络通信模块实现的源代码在src/msg目录下，其首先定义了一个网络通信的框架，三个子目录里分别对应：Simple、Async、XIO三种不同的实现方式。

Simple是比较简单，目前比较稳定的实现，系统默认的用于生产环境的方式。它最大的特点是：每一个网络连接都会创建两个线程，一个专门用于接收，一个专门用于发送。这种模式实现比较简单，但是对于大规模的集群部署，大量的连接会产生大量的线程，会消耗CPU资源，影响性能。

Async模式使用了基于事件的IO多路复用模式。这是网络通信中广泛采用的方式。但是在Ceph中，官方宣称这种方式还处于试验阶段，不够稳定，还不能用于生产环境。

XIO方式使用了开源的网络通信库```accelio```来实现。这种方式需要依赖第三方的库accelio的稳定性，需要对accelio的使用方式以及代码都比较熟悉。目前也处于试验阶段。特别注意的是，前两种方式只支持TCP/IP协议，而XIO可以支持Infiniband网络。

在msg目录下定义了网络通信的抽象框架，它完成了通信接口和具体实现的分离。在其下分别由msg/simple子目录、msg/async子目录、msg/xio子目录，分别对应三种不同的实现。


### 1.1 Message
类Message是所有消息的基类(位于：src/msg/message.cc)，任何要发送的消息，都要继承该类，格式如下图3-1所示：

![ceph-chapter3-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter3_1.jpg)


我们可以通过Pipe::write_message()看消息发出时候的打包顺序，从而知道上面的消息发送格式。但这里，我们看到message.cc中有一个encode_message()函数，该函数其实只是在对消息进行```转发```或者```路由```时对原来消息的二次打包，其真正发送出去的时候还是满足上面的消息发送格式的，如下所示：

![ceph-chapter3-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter3_2.jpg)


下面我们详细介绍一下消息的结构： header是消息头，类似一个消息的信封(envelope)，user_data是用于要发送的实际数据，footer是一个消息的结束标记，如下所示：
{% highlight string %}
class Message : public RefCountedObject {
protected:
  ceph_msg_header  header;                 //消息头
  ceph_msg_footer  footer;                 //消息尾

  /* user_data*/
  bufferlist       payload;               // "front" unaligned blob
  bufferlist       middle;                // "middle" unaligned blob
  bufferlist       data;                  // data payload (page-alignment will be preserved where possible)

  /* recv_stamp is set when the Messenger starts reading the Message off the wire */
  utime_t recv_stamp;

  /* dispatch_stamp is set when the Messenger starts calling dispatch() on its endpoints */
  utime_t dispatch_stamp;

  /* throttle_stamp is the point at which we got throttle */
  utime_t throttle_stamp;

  /* time at which message was fully read */
  utime_t recv_complete_stamp;

  ConnectionRef connection;               //网络连接类

  uint32_t magic;                         //消息魔术字

  bi::list_member_hook<> dispatch_q;      //boost::intrusive需要的字段，当前只在XIO模式的实现中用到。
};
{% endhighlight %}
下面分别介绍其中的重要参数：

ceph_msg_header为消息头，它定义了消息传输相关的元数据：
{% highlight string %}
struct ceph_msg_header {
	__le64 seq;                    //当前session内消息的唯一序号
	__le64 tid;                    //事务ID
	__le16 type;                   //消息类型
	__le16 priority;               //优先级。值越大，消息的优先级越高
	__le16 version;                //消息编码的版本号

	__le32 front_len;             //bytes in main payload(主payload,也称为front playload, 主净荷数据的长度）
	__le32 middle_len;            //middle的长度
	__le32 data_len;              //data的长度
	__le16 data_off;              //对象数据的偏移量，通常需要CEPH_PAGE_SIZE对齐

	struct ceph_entity_name src;  //消息源，比如'mds0' or 'osd3'.

	/* oldest code we think can decode this.  unknown if zero. */
	__le16 compat_version;
	__le16 reserved;
	__le32 crc;                   //消息头的crc32c校验信息
} __attribute__ ((packed));
{% endhighlight %}

ceph_msg_footer为消息的尾部，附加了一些crc校验数据和消息结束标志：
{% highlight string %}
struct ceph_msg_footer {
	__le32 front_crc, middle_crc, data_crc;

	// sig holds the 64 bits of the digital signature for the message PLR
	__le64  sig;                 //消息的64位signature
	__u8 flags;                  //结束标志
} __attribute__ ((packed));
{% endhighlight %}
消息带的数据分别保存在payload、middle和data这三个bufferlist中。payload一般保存ceph操作相关的元数据，作为主净荷数据存在；middle目前没有使用到；data一般为读写的数据。

> 注： 在源代码src/messages下定义了系统需要的相关消息，其都是Message类的子类。


### 1.2 Connection
类Connection对应端到端的socket链接的封装，用于跟踪链接的状态。其最重要的接口是可以发送消息(位于src/msg/connection.h)：
{% highlight string %}
struct Connection : public RefCountedObject {
  mutable Mutex lock;                              //锁保护Connection的所有字段
  Messenger *msgr;                                 //该Connection对应的消息发送器： Messenger可以管理很多链接，并采用适当的策略来进行消息发送
  RefCountedObject *priv;                          链接的私有数据

  int peer_type;                                  //peer的类型，可以是CEPH_ENTITY_TYPE_MON、CEPH_ENTITY_TYPE_MDS等
  entity_addr_t peer_addr;                        //peer的地址

  //最后一次发送keepalive的时间和最后一次接收keepalive的ACK的时间
  utime_t last_keepalive, last_keepalive_ack;     


private:
  uint64_t features;                              //一些feature的标志位
public:
  bool failed;                                    //当值为true时，该链接为lossy链接已经失效了

  int rx_buffers_version;                         //接收缓冲区的版本

  //接收缓冲区。消息的标识ceph_tid --> (buffer, rx_buffers_version)的映射
  map<ceph_tid_t,pair<bufferlist,int> > rx_buffers;
};
{% endhighlight %}
>注： 对于rx_buffers，我们可以为某一个ceph_tid分配接收缓存，也可以不为其分配。当为其分配接收缓存时，就需要使用rx_buffers_version来标识接收缓存，以标识缓存是否进行过修改。

其最重要的功能就是发送消息的接口：
{% highlight string %}
  virtual int send_message(Message *m) = 0;
{% endhighlight %}

### 1.3 Dispatcher
类Dispatcher是消息分发的接口(src/msg/dispatcher.h)，其分发消息的接口为：
{% highlight string %}
virtual bool ms_dispatch(Message *m) = 0;
virtual void ms_fast_dispatch(Message *m) { assert(0); };
{% endhighlight %}
Server端注册该Dispatcher类用于把接收到的Message请求分发给具体处理的应用层。Client端需要实现一个Dispatcher函数，用于处理收到的ACK应答消息。

我们可以通过测试脚本：
<pre>
src/test/messenger/simple_dispatcher.cc
src/test/messenger/simple_client.cc
src/test/messenger/simple_server.cc
</pre>
来进一步了解Dispatcher接口的使用。

### 1.4 Messenger
Messenger是整个网络抽象模块，定义了网络模块的基本API接口。网络模块对外提供的基本功能，就是能在节点之间发送和接收消息。

先一个节点发送消息的命令如下：
{% highlight string %}
virtual int send_message(Message *m, const entity_inst_t& dest) = 0;
{% endhighlight %}
注： 但是此方法文档标识已经过期(deprecated)，建议使用Connection::send_message()方法

注册一个Dispatcher用来分发消息的接口如下：
{% highlight string %}
void add_dispatcher_head(Dispatcher *d);

void add_dispatcher_tail(Dispatcher *d);
{% endhighlight %}

### 1.5 网络连接策略
Policy定义了Messenger处理Connection的一些策略：
{% highlight string %}
struct Policy {
    bool lossy;                     //如果为true，则当该连接出现错误时就删除
    
    bool server;                    //假如为true，则说明为服务端，则自己不能进行主动的重连操作
    
    bool standby;                   //假如为true，则当连接处于空闲状态时，处于standby状态
    
    bool resetcheck;                //假如为true，则在连接出错的情况下会进行重连
    
    //该connection(s)相应的流控操作
    Throttle *throttler_bytes;
    Throttle *throttler_messages;

    /// Specify features supported locally by the endpoint.
    uint64_t features_supported;

    /// Specify features any remotes must have to talk to this endpoint.
    uint64_t features_required;
};
{% endhighlight %}

### 1.6 网络模块的使用
通过下面最基本的服务器和客户端的示例程序，了解如何调用网络通信模块提供的接口来完成收发请求消息的功能。

###### Server程序分析
Server程序源代码在test/simple_server.cc里，这里只展示有关网络部分的核心流程：

1) 调用Messenger的函数create创建一个Messenger的实例，配置选项g_conf->ms_type为配置的实现类型，目前有三种实现方式： simple、async、xio
{% highlight string %}
messenger = Messenger::create(g_ceph_context, g_conf->ms_type,
				     entity_name_t::MON(-1),
				     "simple_server",
				     0 /* nonce */);
{% endhighlight %}

2) 设置Messenger的属性
{% highlight string %}
messenger->set_magic(MSG_MAGIC_TRACE_CTR);
messenger->set_default_policy(
	Messenger::Policy::stateless_server(CEPH_FEATURES_ALL, 0));
{% endhighlight %}

3) 对于Server，需要bind服务端地址
{% highlight string %}
r = messenger->bind(bind_addr);
if (r < 0)
	goto out;

// Set up crypto, daemonize, etc.
//global_init_daemonize(g_ceph_context, 0);
common_init_finish(g_ceph_context);
{% endhighlight %}

4) 创建一个Dispatcher，并添加到Messenger
{% highlight string %}
dispatcher = new SimpleDispatcher(messenger);

messenger->add_dispatcher_head(dispatcher); // should reach ready()
{% endhighlight %}

5) 启动messenger
{% highlight string %}
messenger->start();
messenger->wait();      // 本函数必须等start完成才能调用
{% endhighlight %}

SimpleDispatcher函数里实现了```ms_dispatch```，用于把接收到的各种请求消息分发给相关的处理函数。

###### 1.7 Client程序分析

源代码在test/simple_client.cc里，这里只展示有关网络部分的核心流程：

1） 调用Messenger的create()函数创建一个Messenger实例：
{% highlight string %}
messenger = Messenger::create(g_ceph_context, g_conf->ms_type,
				      entity_name_t::MON(-1),
				      "client",
				      getpid());
{% endhighlight %}

2) 设置相关的策略
{% highlight string %}
// enable timing prints
messenger->set_magic(MSG_MAGIC_TRACE_CTR);
messenger->set_default_policy(Messenger::Policy::lossy_client(0, 0));
{% endhighlight %}

3) 创建Dispatcher类并添加，用于接收消息
{% highlight string %}
dispatcher = new SimpleDispatcher(messenger);
messenger->add_dispatcher_head(dispatcher);

dispatcher->set_active(); // this side is the pinger
{% endhighlight %}

4） 启动消息
{% highlight string %}
r = messenger->start();
if (r < 0)
	goto out;
{% endhighlight %}

5) 下面开始发送请求，先获取目标Server的链接
{% highlight string %}
conn = messenger->get_connection(dest_server);
{% endhighlight %}

6） 通过Connection来发送请求消息。这里的消息发送方式都是异步发送，接收到请求消息的ACK应答消息后，将在Dispatcher的ms_dispatch或者ms_fast_dispatch处理函数里做相关的处理
{% highlight string %}
int msg_ix;
Message *m;
for (msg_ix = 0; msg_ix < n_msgs; ++msg_ix) {
	/* add a data payload if asked */
	if (! n_dsize) {
		m = new MPing();
	} else {
		m = new_simple_ping_with_data("simple_client", n_dsize);
	}
	conn->send_message(m);
}
{% endhighlight %}
综上所述，通过Ceph的网络框架发送消息比较简单。在Server端，只需要创建一个Messenger实例，设置相应的策略并绑定服务端口，然后设置一个Dispatcher来处理接收到的请求。在Client端，只需要创建一个Messenger实例，设置相关的策略和Dispatcher用于处理返回的应答消息。通过获取对应Server的connection来发送消息即可。




<br />
<br />

**[参看]**

1. [非常详细的 Ceph 介绍、原理、架构](https://blog.csdn.net/mingongge/article/details/100788388)





<br />
<br />
<br />

