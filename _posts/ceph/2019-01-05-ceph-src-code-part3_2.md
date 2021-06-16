---
layout: post
title: ceph网络通信(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本章我们介绍一下Ceph的网络通信模块，这是客户端和服务器通信的底层模块，用来在客户端和服务器之间接收和发送请求。其实现功能比较清晰，是一个相对比较独立的模块，理解起来比较容易。

<!-- more -->

## 1. Messenger
{% highlight string %}
class Messenger{
private:
	list<Dispatcher *> dispatchers;
	list<Dispatcher *> fast_dispatchers;

public:
	
	//获取dispatcher列表的当前长度
	virtual int get_dispatch_queue_len() = 0;

	/**
	* Get age of oldest undelivered message
	* (0 if the queue is empty)
	*/
	virtual double get_dispatch_queue_max_age(utime_t now) = 0;
	
	
	//添加新的dispatcher到列表头
	void add_dispatcher_head(Dispatcher *d) { 
		bool first = dispatchers.empty();
		dispatchers.push_front(d);
		
		if (d->ms_can_fast_dispatch_any())
			fast_dispatchers.push_front(d);
			
		if (first)
			ready();
	}
	
	//添加新的dispatcher到列表尾
	void add_dispatcher_tail(Dispatcher *d) { 
		bool first = dispatchers.empty();
		dispatchers.push_back(d);
		
		if (d->ms_can_fast_dispatch_any())
			fast_dispatchers.push_back(d);
		if (first)
			ready();
	}
	
	/*
	 * 将message放入队列。
	 * (注：此方法已经过时，在新的代码中不要使用，请用Connection::send_message()来替代）
	 */
	virtual int send_message(Message *m, const entity_inst_t& dest) = 0;
	

public:

	//遍历fast_dispatchers列表，查看其是否可以通过fast dispatch来分发消息
	bool ms_can_fast_dispatch(Message *m) {
		for (list<Dispatcher*>::iterator p = fast_dispatchers.begin();p != fast_dispatchers.end();++p) {
			if ((*p)->ms_can_fast_dispatch(m))
				return true;
		}
		
		return false;
	}
	
	//通过fast dispatch来分发消息
	void ms_fast_dispatch(Message *m) {
		m->set_dispatch_stamp(ceph_clock_now(cct));
		
		for (list<Dispatcher*>::iterator p = fast_dispatchers.begin();p != fast_dispatchers.end();++p) {
			if ((*p)->ms_can_fast_dispatch(m)) {
				(*p)->ms_fast_dispatch(m);
				return;
			}
		}
		
		assert(0);
	}
	
	//在fast dispatch之前，进行消息的预处理
	void ms_fast_preprocess(Message *m) {
		for (list<Dispatcher*>::iterator p = fast_dispatchers.begin();p != fast_dispatchers.end();++p) {
			(*p)->ms_fast_preprocess(m);
		}
	}
	
	//遍历dispatchers列表，直到其中一个dispatcher可以处理，就return退出。如果没有一个
	//dispatcher可以处理，则assert()报错
	void ms_deliver_dispatch(Message *m) {
		m->set_dispatch_stamp(ceph_clock_now(cct));
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p) {
			if ((*p)->ms_dispatch(m))
				return;
		}
		
		lsubdout(cct, ms, 0) << "ms_deliver_dispatch: unhandled message " << m << " " << *m << " from "
			<< m->get_source_inst() << dendl;
		assert(!cct->_conf->ms_die_on_unhandled_msg);
		m->put();
	}
	
	//当有新的连接产生时，调用此函数通知dispatcher
	void ms_deliver_handle_connect(Connection *con) {
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p)
			(*p)->ms_handle_connect(con);
	}
	
	//通知每一个fast dispatcher有新的连接产生
	void ms_deliver_handle_fast_connect(Connection *con) {
		for (list<Dispatcher*>::iterator p = fast_dispatchers.begin();p != fast_dispatchers.end();++p)
			(*p)->ms_handle_fast_connect(con);
	}
	
	/**
	 * Notify each Dispatcher of a new incomming Connection. Call
	 * this function whenever a new Connection is accepted.
	*/
	void ms_deliver_handle_accept(Connection *con) {
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p)
			(*p)->ms_handle_accept(con);
	}
	

	/**
	 * Notify each fast Dispatcher of a new incoming Connection. Call
	 * this function whenever a new Connection is accepted.
	*/
	void ms_deliver_handle_fast_accept(Connection *con) {
		for (list<Dispatcher*>::iterator p = fast_dispatchers.begin();p != fast_dispatchers.end();++p)
			(*p)->ms_handle_fast_accept(con);
	}
	
	//当侦测到一个lossy connection断连之后，回调此函数报告dispatcher有可能丢失消息
	void ms_deliver_handle_reset(Connection *con) {
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p) {
			if ((*p)->ms_handle_reset(con))
				return;
		}
	}
	
	//报告每一个dispatcher，connection可能被远程端forgotten，这暗含着消息可能丢失
	void ms_deliver_handle_remote_reset(Connection *con) {
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p)
			(*p)->ms_handle_remote_reset(con);
	}
	
	//为一个新的outgoing连接查找到AuthAuthorizer
	AuthAuthorizer *ms_deliver_get_authorizer(int peer_type, bool force_new) {
		AuthAuthorizer *a = 0;
		
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p) {
			if ((*p)->ms_get_authorizer(peer_type, &a, force_new))
				return a;
		}
		return NULL;
	}
	
	
	//校验一个新的incomming连接是否正确
	bool ms_deliver_verify_authorizer(Connection *con, int peer_type,
	  int protocol, bufferlist& authorizer, bufferlist& authorizer_reply,
	  bool& isvalid, CryptoKey& session_key) {
	  
		for (list<Dispatcher*>::iterator p = dispatchers.begin();p != dispatchers.end();++p) {
			if ((*p)->ms_verify_authorizer(con, peer_type, protocol, authorizer, authorizer_reply, isvalid, session_key))
				return true;
		}
		return false;
	}
};
{% endhighlight %}
Messenger管理着两类dispatcher: 

* 普通dispatcher

* fast dispater

代码实现较为简单，主要功能就是当有新的message到来时，分发消息；当有新的ingoing连接进来时，回调accept以及verify authorizer等；当有新的outgoing连接建立时，回调connect以及get authorizer等。

这里存在的一个疑问是，当调用add_dispatcher_head()或add_dispatcher_tail()加入一个dispatcher时，有可能同时加入到普通dispatcher列表，也有可能加入到fast dispatcher列表。那么当一个消息发送到dispatcher queue时，是否会回调两次呢？

答案是不会的。因为在消息进行投递时，其首先会调用ms_can_fast_dispatch()来判断是否可以进行快速投递，如果可以则不进行普通投递了。

## 2. SimpleMessenger
{% highlight string %}
class SimplePolicyMessenger : public Messenger{
};


/**
 * SimpleMessenger负责消息的发送与接收。一般来说，其主要包含如下几个components:
 *
 * 1) Connection
 *    每一个逻辑会话(session)都关联着一个Connection
 *
 * 2) Pipe
 *    每一个网络连接都是通过pipe来进行处理的，负责消息的收发。通常情况下Pipe与Connection
 *    之间的比例关系是1:1，但是在socket重连或者connection竞争时逻辑sessions可能会在Pipes
 *    之间分离。
 *
 * 3）IncommingQueue
 *    Incomming消息与IncommingQueue相关联，并且每一个Pipe都关联着这样一个队列
 *
 * 4）DispatchQueue
 *   IncommingQueues本身也存放于DispatchQueue中，并由DispatchQueue的工作线程来负责消息的清理
 *   以及处理。
 *
 * 5）SimpleMessenger
 *   作为对外暴露的类传递给其他的消息处理器，并提供大部分API的细节
 */
class SimpleMessenger : public SimplePolicyMessenger {
public:
	/*
	 * 初始化SimpleMessenger
	 *
	 * @param cct: 所对应的CephContext
     * @param name: 用户所指定的name值
     * @param _nonce: 本SimpleMessenger所使用的唯一ID。在守护进程重启时，该值不能重复
     * @param features: local_connection的features	 
	 */
	SimpleMessenger(CephContext *cct, entity_name_t name,
		  string mname, uint64_t _nonce, uint64_t features);

	int get_dispatch_queue_len() {
		return dispatch_queue.get_queue_len();
	}

	double get_dispatch_queue_max_age(utime_t now) {
		return dispatch_queue.get_max_age(now);
	}
	
	/*
	 * SimpleMessage拥有一个accepter，让accepter绑定对应的网络地址
	 */
	int bind(const entity_addr_t& bind_addr);
	int rebind(const set<int>& avoid_ports);
	
	/*
	 * 启动回收线程(reaper_thread)，做pipe回收等相关工作
	 */
	virtual int start();
	virtual void wait();
	virtual int shutdown();
	
	
	/*
	 * 发送消息到指定的目标地址
	 */
	virtual int send_message(Message *m, const entity_inst_t& dest) {
		return _send_message(m, dest);
	}

	int send_message(Message *m, Connection *con) {
		return _send_message(m, con);
	}
	
	/*
	 * 根据目标地址获取一个Connection
	 */
	virtual ConnectionRef get_connection(const entity_inst_t& dest);
	
	/*
	 * 获取local_connection
	 */
	virtual ConnectionRef get_loopback_connection();

protected:
	/*
	 * 启动DispatchQueue来分发消息，并且如果accepter已经绑定，也启动accepter线程监听socket连接
	 */
	 virtual void ready();
	 
public:
	/*
	 * 一个SimpleMessenger可以拥有一个accepter，用于接受远程连接
	 */
	Accepter accepter;
	
	/*
	 * 所拥有的消息分发队列（该分发队列是一个带优先级的队列)
	 */
	DispatchQueue dispatch_queue;
	
	/*
	 * 当accepter接受了一个新的连接，调用此函数将对应的pipe加入到pipes和accept_pipes列表，
	 * 并且启动读线程
	 */
	Pipe *add_accept_pipe(int sd);
	
	
private:

	/**
	* A thread used to tear down Pipes when they're complete.
	*/
	class ReaperThread : public Thread {
		SimpleMessenger *msgr;
		
	public:
		explicit ReaperThread(SimpleMessenger *m) : msgr(m) {}
		
		void *entry() {
			msgr->reaper_entry();
			return 0;
		}
	} reaper_thread;
	
	
	/*
	 * 采用addr创建一个新的Pipe，并将con与该pipe关联。
	 * （注：本函数返回成功，并不保证底层的socket实际建立成功)
	 *
	 * @param addr: 所要建立的连接的目标地址
	 * @param type: 对端的类型(OSD/MDS/MON等)
	 * @param con: 所创建的pipe与con相关连，用于存储该pipe的一些状态信息
	 * @param first: 向该pipe发送的第一条消息
	 */
	Pipe *connect_rank(const entity_addr_t& addr, int type, PipeConnection *con,
		     Message *first);
			 
	//最终通过pipe来发送消息
	int _send_message(Message *m, const entity_inst_t& dest);
	
	//最终通过pipe来发送消息
	int _send_message(Message *m, Connection *con);
	
	/*
	 * 将message放入队列以发送到指定的目标地址。
	 * （如有必要，submit_message()也负责创建新的Pipes,以及关闭老的pipes)
	 *
	 * @param m: 所要发送的消息
	 * @param con: 已存在的目标连接，如果不清楚，请设置为NULL，这样就会根据目标地址创建一个新的pip来发送
	 * @param addr: 所要发送到的目标地址
	 * @param dest_type: 目标类型(osd/mds/mon等)
	 * @param already_locked: 假如为false的话，那么SimpleMessenger在访问共享数据结构之前会先获取锁；否则其
	 *        会假设当前已经获取了锁。值得注意的是，假如通过already_locked为false来调用此函数，那么con不能为NULL
	 *
	 */
	void submit_message(Message *m, PipeConnection *con,
		      const entity_addr_t& addr, int dest_type,
		      bool already_locked);
			  
	/*
	 * 回收不必要的pipe
	 */
	void reaper();
	
private:
	//accepter是否绑定地址
	bool did_bind;
	
	//存放pipe的列表(包括自己本身发出去的连接，以及accept的连接)
	ceph::unordered_map<entity_addr_t, Pipe*> rank_pipe;
	
	//当前正在accepting的pipes
	set<Pipe*> accepting_pipes;
	
	//所有的pipe列表
	set<Pipe*>      pipes;
	
	//等待回收的pipe列表
	list<Pipe*>     pipe_reap_queue;
	
	
	//根据目标地址查找当前已存在的pipe
	Pipe *_lookup_pipe(const entity_addr_t& k) {
		ceph::unordered_map<entity_addr_t, Pipe*>::iterator p = rank_pipe.find(k);
		if (p == rank_pipe.end())
			return NULL;
			
		// see lock cribbing in Pipe::fault()
		if (p->second->state_closed.read())
			return NULL;
		return p->second;
	}
	
public:

  //local_connection，可用于简化消息的投递（不需要通过socket来发送，可以直接投递到队列)
  ConnectionRef local_connection;
  uint64_t local_features;
};
{% endhighlight %}

## 3. Connection
{% highlight string %}
/*
 * Connection代表的是一个抽象的连接，用于保存连接的状态信息。
 * （注：与我们通常意义上的连接含义有些不同，主要用于保存连接状态信息，并不直接与socket fd相关联)
 */
struct Connection : public RefCountedObject {
public:
	//所对应的Messenger
	Messenger *msgr;
	
	//所对应的连接的类型(osd/mon/mds等)
	int peer_type
	
	//对端地址
	entity_addr_t peer_addr;
	
private:
	//用于保存连接的一些feature
	uint64_t features;
	
public:
	//假如当前连接是lossy connection，那么当其状态为failed时，本字段值为true
	bool failed; // true if we are a lossy connection that has failed.

	int rx_buffers_version;
	
	//接收缓存（根据事务ID来索引)
	map<ceph_tid_t,pair<bufferlist,int> > rx_buffers;
	
public:
	/*
	 * 发送消息到connection
	 * （注：send_message()函数比较奇怪，因为connection的含义为表示实际连接的状态，因此
	 * 本不应该在这里设置消息发送函数)
	 */
	virtual int send_message(Message *m) = 0;
};
{% endhighlight %}

## 4. PipeConnection
{% highlight string %}
class PipeConnection : public Connection {
public:
	Pipe* pipe;

public:
	int send_message(Message *m) override;
};
{% endhighlight %}
PipeConnection继承自Connection，最主要是其可以与pipe相关连，从而可与实际的socket建立联系。

上面讲到Connection::send_message()函数有些奇怪，这里我们来看一下其实现：
{% highlight string %}
int PipeConnection::send_message(Message *m)
{
	assert(msgr);
	return static_cast<SimpleMessenger*>(msgr)->send_message(m, this);
}
{% endhighlight %}
可以看到其并不能直接发送消息，而是调用SimpleMessenger来发送，最终是调用所关联的pipe来发送。

## 5. Accepter
{% highlight string %}
class Accepter : public Thread {
	
	//所对应的simple messenger
	SimpleMessenger *msgr;

	//是否退出标志
	bool done;
	
	//所监听的socket句柄
	int listen_sd;
	
public:
	/*
	 * 监听socket句柄，并接受新来的连接
	 */
	void *entry();
	
	//绑定指定的地址
	int bind(const entity_addr_t &bind_addr, const set<int>& avoid_ports);
	int rebind(const set<int>& avoid_port);
};
{% endhighlight %}

## 6. Pipe
{% highlight string %}
/*
 * Pipe是SimpleMessenger中最复杂的组件。Pipe拥有读写两个线程，并且负责对应socket
 * 上的一切。除了负责消息的投递之外，Pipe还负责将socket上的错误信息propagate到
 * SimpleMessenger，并且为SimpleMessenger提供可靠的消息传递机制
 * 
 */
class Pipe : public RefCountedObject {

	/*
	 * 读线程负责socket上的所有读操作---不仅仅是消息，也包括acks以及其他一些protocol bits
	 * (注：在启动阶段，write线程也会进行一些读操作，主要是为了完成相应连接的建立)
	 */
	class Reader : public Thread {
	Pipe *pipe;
	public:
		explicit Reader(Pipe *p) : pipe(p) {}
		
		void *entry() { pipe->reader(); return 0; }
	} reader_thread;
	
	
	//负责socket建立完成之后的写操作
	class Writer : public Thread {
	Pipe *pipe;
	public:
		explicit Writer(Pipe *p) : pipe(p) {}
		void *entry() { pipe->writer(); return 0; }
	} writer_thread;
	
	/*
	 * 延迟消息队列，主要是为了在消息投递时注入延迟。
	 * 只有在请求中指定了延迟，才会将消息投入进延迟队列。在延迟时间到了之后，才会将消息
	 * 从延迟队列出去，放入in_q(即SimpleMessage::dispatch_queue)中。
	 */
	class DelayedDelivery: public Thread {
	}*delay_thread;
	
	
public: 
	//所对应的simple messenger
	SimpleMessenger *msgr;
	
 private:
 
	//对应的socket句柄
	int sd;
	
	struct iovec msgvec[SM_IOV_MAX];	
	
public:
	/*
	 * 连接对应的端口、peer type信息
	 */
	int port;
	int peer_type;
	entity_addr_t peer_addr;
	
	/*
	 * 当前连接的状态
	 *  
	 * enum {
	 *	STATE_ACCEPTING,
	 *	STATE_CONNECTING,
	 *	STATE_OPEN,
	 *	STATE_STANDBY,
	 *	STATE_CLOSED,
	 *	STATE_CLOSING,
	 *	STATE_WAIT       // just wait for racing connection
	 * };
	 */ 
	int state;
	
protected:
	/*
	 * 连接的Connection state
	 */
	PipeConnectionRef connection_state;
	
	/*
	 * 消息发送的优先级队列
	 */
	map<int, list<Message*> > out_q;  // priority queue for outbound msgs
	
	/*
	 * 接受到的消息的投递队列
	 */
	DispatchQueue *in_q;
	
	//已经发送的消息
    list<Message*> sent;
	
	/*
	 * 当accepter接受到新连接进来，就会调用reader()来进行处理，然后在reader中调用本
	 * 函数完成服务端的握手
	 */
	int accept();   // server handshake
	
	/*
	 * 当客户端连接后，在writer()函数中调用本函数完成客户端握手
	 */
    int connect();  // client handshake
	
	//具体的读函数
    void reader();
	
	//具体的写函数
    void writer();
};
{% endhighlight %}

## 7. Dispatcher
{% highlight string %}
/*
 * 消息发送的目标接收者需要实现此接口
 */
class Dispatcher {
public:

	/*
	 * Messenger会调用此函数来查询是否有能力fast dispatch消息。如果想要一条
	 * 消息能够fast dispatch，你需要满足：
	 *
	 * 1）能够快速的处理消息，并且不需要花长时间来竞争lock
	 *
	 * 2) 即使在对应Connection没有获得ms_handle_accept()通知的情况下（比如已经执行了
	 * mark_down()，或者在连接上执行了ms_handle_reset()等),仍能够接受该消息并进行处理。
	 *
	 * 3）在不依赖特定系统状态的情况下，要能够决定消息是否能够快速投递。
	 * （注：一条消息可能会多次调用ms_can_fast_dispatch,而状态可能可能会在多次调用过程
	 *  中发生改变)
	 */
	virtual bool ms_can_fast_dispatch(Message *m) const { return false;}
	
	/*
	 * 用于决定一个dispatcher是否包含在fast-dispatcher列表中。
	 * 假如可以通过fast_dispatch()处理任何消息的话，那么本函数返回true，否则返回false;
	 */
	virtual bool ms_can_fast_dispatch_any() const { return false; }
	
	/*
	 * 对消息进行fast_dispatch
	 */
	virtual void ms_fast_dispatch(Message *m) { assert(0); }
	
	/*
	 * 在消息被dispatch之前，让dispatcher可以先对消息进行预处理。本函数针对每一个message都会
	 * 被调用，且是在决定进行fast/regular dispatch之前被调用，但值得注意的是其只用于拥有fast-dispatch
	 * 能力的系统。
	 */
	virtual void ms_fast_preprocess(Message *m) {}
	
	//普通的消息分发
	virtual bool ms_dispatch(Message *m) = 0;
	
	/*
	 * 当一个连接新创建，或者连接重连时，本函数会被同步的调用。
	 */
	virtual void ms_handle_connect(Connection *con) {}
	
	/*
	 * 假如Connection支持fast dispatch的话，那么当连接新创建或者连接重连时，本函数就会被同步地回调
	 */
	virtual void ms_handle_fast_connect(Connection *con) {}
	
	/*
	 * 当接受到一个incomming连接时，就会回调此函数
	 */
	virtual void ms_handle_accept(Connection *con) {}
	
	/*
	 * 当接受到一个incomming连接，且该连接支持fast dispatch时本函数就会被回调
	 *
	 * 系统会保证在该incomming connection的任何消息被投递之前，本函数会被回调
	 */
	virtual void ms_handle_fast_accept(Connection *con) {}
	
	/*
	 * 如果本函数被回调，则说明ordered+reliable的消息投递语义被破坏。可能由于网络连接
	 * 故障导致消息被丢失
	 *
	 * （注：只针对lossy connection，本函数才可能被回调）
	 */
	virtual bool ms_handle_reset(Connection *con) = 0;
	
	/*
	 * 如果本函数被回调，则说明ordered+reliable的消息投递语义被破坏，原因是连接被remote端reset。
	 * 
	 * 这通常隐含着incomming消息丢失，并且前面发出去的一些outgoing消息也可能丢失
	 */
	virtual void ms_handle_remote_reset(Connection *con) = 0;
	
	/*
	 * 获取给定peer type的AuthAuthorizer。假如对应的peer type不需要认证信息的话，则返回false
	 */
	virtual bool ms_get_authorizer(int dest_type, AuthAuthorizer **a, bool force_new) { return false; }
	
	/*
	 * 对新的incomming连接校验其authorizer信息
	 */
	virtual bool ms_verify_authorizer(Connection *con,
					int peer_type,
					int protocol,
					ceph::bufferlist& authorizer,
					ceph::bufferlist& authorizer_reply,
					bool& isvalid,
					CryptoKey& session_key) { return false; }
	
};
{% endhighlight %}

## 8. DispatchQueue
{% highlight string %}
/*
 * DispatchQueue含有所有Pipe中需要分发的消息，其是一个带有优先级的队列，
 * 因此在使用时请仔细的组织各类消息的优先级，
 */
class DispatchQueue {

	/*
	 * 消息队列中的每一个Item类型
	 */
	class QueueItem {
		int type;
		ConnectionRef con;
		MessageRef m;
	};
	
	
	//对应的simple messenger
	SimpleMessenger *msgr;
	
	//对应的优先级队列
	PrioritizedQueue<QueueItem, uint64_t> mqueue;
	
	//用于存放收到的incomming消息
	set<pair<double, Message*> > marrival;
	map<Message *, set<pair<double, Message*> >::iterator> marrival_map;
	
	
	/*
	 * 负责分发消息的线程
	 */
	class DispatchThread : public Thread {
		DispatchQueue *dq;
	public:
		explicit DispatchThread(DispatchQueue *dq) : dq(dq) {}
		void *entry() {
			dq->entry();
			return 0;
		}
	} dispatch_thread;
	
	//存放发往本地的消息
	list<pair<Message *, int> > local_messages;
	
	/*
	 * 负责投递到本地消息的线程
	 */
	class LocalDeliveryThread : public Thread {
	DispatchQueue *dq;
	public:
		explicit LocalDeliveryThread(DispatchQueue *dq) : dq(dq) {}
		void *entry() {
		dq->run_local_delivery();
		return 0;
		}
	} local_delivery_thread;
	
	//消息投递之前，打印消息，申请相应的资源等
	uint64_t pre_dispatch(Message *m);
	
	//消息投递完成之后，释放相应的资源
	void post_dispatch(Message *m, uint64_t msize);
	
public:
	/*
	 * 进行本地消息投递，即将消息插入到last_messages中，然后由local_delivery_thread真正负责投递
	 */
	void local_delivery(Message *m, int priority);
	
	//本地消息投递的真正实现
	void run_local_delivery();
	
	//调用对应的SimpleMessenger判断消息是否可以进行fast dispatch
	bool can_fast_dispatch(Message *m) const;
	
	//调用SimpleMessenger对消息进行快速投递
	void fast_dispatch(Message *m);
	
	//调用SimpleMessenger对消息进行预处理
	void fast_preprocess(Message *m);
	
	//将消息插入到优先级队列
	void enqueue(Message *m, int priority, uint64_t id);
	
	//丢弃某一种类型的消息
	void discard_queue(uint64_t id);
	
	//丢弃本地消息
	void discard_local();
	
	//创建dispatch_thread以及local_delivery_thread
	void start();
	
	//dispatch_thread的回调函数
	void entry();
};
{% endhighlight %}

现在我们来看一下消息投递的具体流程：

1）**本地消息**
{% highlight string %}
void SimpleMessenger::submit_message(Message *m, PipeConnection *con,
				     const entity_addr_t& dest_addr, int dest_type,
				     bool already_locked)
{
	...
	// local?
	if (my_inst.addr == dest_addr) {
		// local
		ldout(cct,20) << "submit_message " << *m << " local" << dendl;
		dispatch_queue.local_delivery(m, m->get_priority());
		return;
	}

	...
}

void DispatchQueue::local_delivery(Message *m, int priority)
{
	m->set_connection(msgr->local_connection.get());
	m->set_recv_stamp(ceph_clock_now(msgr->cct));
	Mutex::Locker l(local_delivery_lock);

	if (local_messages.empty())
		local_delivery_cond.Signal();
	local_messages.push_back(make_pair(m, priority));
	return;
}
{% endhighlight %}
从上面我们可以看到，对于发送的目标地址是本地的消息，这直接将消息插入到local_messages队列。

2) **收到的incomming消息的投递**

主要有如下两个地方：

* 对投入到DelayQueue中的消息进行再投递
{% highlight string %}
void *Pipe::DelayedDelivery::entry(){
	...
	if (pipe->in_q->can_fast_dispatch(m)) {
		if (!stop_fast_dispatching_flag) {
			delay_dispatching = true;
			delay_lock.Unlock();
			pipe->in_q->fast_dispatch(m);
			delay_lock.Lock();
			delay_dispatching = false;

			if (stop_fast_dispatching_flag) {
				// we need to let the stopping thread proceed
				delay_cond.Signal();
				delay_lock.Unlock();
				delay_lock.Lock();
			}
		}
	} else {
		pipe->in_q->enqueue(m, m->get_priority(), pipe->conn_id);
	}

	...
}
{% endhighlight %}

* reader()读取到的消息投递进mqueue
{% highlight string %}
void Pipe::reader()
{
	...
	in_q->fast_preprocess(m);
	
	if (delay_thread) {
		utime_t release;
		if (rand() % 10000 < msgr->cct->_conf->ms_inject_delay_probability * 10000.0) {
			release = m->get_recv_stamp();
			release += msgr->cct->_conf->ms_inject_delay_max * (double)(rand() % 10000) / 10000.0;
			lsubdout(msgr->cct, ms, 1) << "queue_received will delay until " << release << " on " << m << " " << *m << dendl;
		}
		delay_thread->queue(release, m);
	} else {
		if (in_q->can_fast_dispatch(m)) {
			reader_dispatching = true;
			pipe_lock.Unlock();
			in_q->fast_dispatch(m);
			pipe_lock.Lock();
			reader_dispatching = false;

			if (state == STATE_CLOSED ||
			  notify_on_dispatch_done) { // there might be somebody waiting
				notify_on_dispatch_done = false;
				cond.Signal();
			}
		} else {
			in_q->enqueue(m, m->get_priority(), conn_id);
		}
	}

	...
}
{% endhighlight %}


<br />
<br />

**[参看]**







<br />
<br />
<br />

