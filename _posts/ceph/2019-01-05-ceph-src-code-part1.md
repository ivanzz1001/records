---
layout: post
title: ceph通用模块
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本章介绍Ceph源代码通用库中的一些比较关键而又比较复杂的数据结构。Object和Buffer相关的数据结构是普遍使用的。线程池ThreadPool可以提高消息处理的并发能力。Finisher提供了异步操作时来执行回调函数。Throttle在系统的各个模块各个环节都可以看到，它用来限制系统的请求，避免瞬时大量突发请求对系统的冲击。SafeTimer提供了定时器，为超时和定时任务等提供了相应的机制。理解这些数据结构，能够更好理解后面章节的相关内容。

<!-- more -->


## 2. Buffer
buffer是一个命名空间，在这个命名空间下定义了Buffer相关的数据结构，这些数据结构在ceph的源代码中广泛使用。下面介绍的buffer:raw类是基础类，其子类完成了Buffer数据空间的分配；buffer::ptr类实现了Buffer内部的一段数据；buffer::list封装了多个数据段。

>相关代码位置： src/include/buffer.h src/common/buffer.cc

buffer中定义的相关数据类型在外部引用时，通常都会以如下形式出现：
{% highlight string %}
#ifndef BUFFER_FWD_H
#define BUFFER_FWD_H

namespace ceph {
  namespace buffer {
    class ptr;
    class list;
    class hash;
  }

  using bufferptr = buffer::ptr;
  using bufferlist = buffer::list;
  using bufferhash = buffer::hash;
}

#endif
{% endhighlight %}

### 2.1 buffer::raw
类buffer::raw是一个原始的数据Buffer，在其基础之上添加了长度、引用计数和额外的crc校验信息，结构如下(src/common/buffer.cc)：
{% highlight string %}
  class buffer::raw {
  public:
    char *data;                                                       //数据指针
    unsigned len;                                                     //数据长度
    atomic_t nref;                                                    //引用计数

    mutable simple_spinlock_t crc_spinlock;                           //读写锁，保护crc_map

	/*
	 * crc校验信息，第一个pair未数据段的起始和结束(from, to)，第二个pair是c3c32校验码，pair的第一个字段为base crc32校验码，
	 * 第二个字段为加上数据段后计算出的crc32校验码。
	*/
    map<pair<size_t, size_t>, pair<uint32_t, uint32_t> > crc_map;     
     

	.......
};
{% endhighlight %}
下列类都继承了buffer::raw，实现了data对应内存空间的申请：

* 类raw_malloc实现了用malloc函数分配内存空间的功能

* 类class buffer::raw_mmap_pages实现了通过mmap来把内存匿名映射到进程的地址空间

* 类class buffer::raw_posix_aligned调用了函数posix_memalign来申请内存地址对齐的内存空间

* 类class buffer::raw_hack_aligned是在系统不支持内存对齐申请的情况下自己实现了内存地址的对齐

* 类class buffer::raw_pipe实现了pipe作为Buffer的内存空间

* 类class buffer::raw_char使用了C++的new操作符来申请内存空间

### 2.2 buffer::ptr
类buffer::ptr就是对于buffer::raw的一个部分数据段。结构如下：
{% highlight string %}
/*
* a buffer pointer.  references (a subsequence of) a raw buffer.
*/
class CEPH_BUFFER_API ptr {
	raw *_raw;
	unsigned _off, _len;

	...
}；
{% endhighlight %}
ptr是raw里的一个任意的数据段，```_off```是在```_raw```里的偏移量，```_len```是ptr的长度。raw和ptr的示意图如图2-1所示：
ceph_chapter2_1.jpg
![ceph-chapter2-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter2_1.jpg)

### 2.3 buffer::list
类buffer::list是一个使用广泛的类，它是多个buffer::ptr的列表，也就是多个内存数据段的列表。结构如下：
{% highlight string %}
/*
* list - the useful bit!
*/

class CEPH_BUFFER_API list {
	std::list<ptr> _buffers;            //所有的ptr
	unsigned _len;                      //所有的ptr的数据总长度
	unsigned _memcopy_count;            //当调用函数rebuild用来内存对齐时，需要内存拷贝的数据量
	ptr append_buffer;                  //当有小的数据就添加到这个buffer里

	mutable iterator last_p;            //访问list的迭代器

	....
};
{% endhighlight %}

buffer::list的重要操作如下所示：

1) 添加一个ptr到list的头部
{% highlight string %}
void push_front(ptr& bp) {
	if (bp.length() == 0)
		return;

	_buffers.push_front(bp);
	_len += bp.length();
}
{% endhighlight %}

2) 添加一个raw到list的头部中，先构造一个ptr，后添加到list中
{% highlight string %}
void push_front(raw *r) {
	push_front(ptr(r));
}
{% endhighlight %}

3) 判断内存是否以参数align对齐，每一个ptr都必须以align对齐
{% highlight string %}
bool buffer::list::is_aligned(unsigned align) const
{
	for (std::list<ptr>::const_iterator it = _buffers.begin();it != _buffers.end();++it) 
		if (!it->is_aligned(align))
			return false;

	return true;
}
{% endhighlight %}

4) 添加一个字符到list中，先查看append_buffer是否有足够的空间，如果没有，就新申请一个4KB大小的空间
{% highlight string %}
void buffer::list::append(char c)
{
	// put what we can into the existing append_buffer.
	unsigned gap = append_buffer.unused_tail_length();

	if (!gap) {
		// make a new append_buffer!
		append_buffer = raw_combined::create(CEPH_BUFFER_APPEND_SIZE);
		append_buffer.set_length(0);   // unused, so far.
	}

	append(append_buffer, append_buffer.append(c) - 1, 1);	// add segment to the list
}
{% endhighlight %}

5) 内存对齐

有些情况下，需要内存地址对齐，例如当以directIO方式写入数据至磁盘时，需要内存地址按内存页面大小(page)对齐，也即buffer::list的内存地址都需按page对齐。函数rebuild用来完成对齐的功能。其实现方法也比较简单，检查没有对齐的ptr，申请一块新对齐的内存，把数据拷贝过去，释放内存空间就可以了。

6) buffer::list还集成了其他额外的一些功能

* 把数据写入文件或从文件读取数据的功能

* 计算数据的crc32校验


## 3. encode/decode
在ceph中，很多地方涉及到需要将某一种类型的数据编码到bufferlist中，这里简单的进行一下说明(编码的主要实现位于src/include/encoding.h中）：
{% highlight string %}
template<class T>
inline void encode_raw(const T& t, bufferlist& bl)
{
  bl.append((char*)&t, sizeof(t));
}
template<class T>
inline void decode_raw(T& t, bufferlist::iterator &p)
{
  p.copy(sizeof(t), (char*)&t);
}

#define WRITE_RAW_ENCODER(type)						\
  inline void encode(const type &v, bufferlist& bl, uint64_t features=0) { encode_raw(v, bl); } \
  inline void decode(type &v, bufferlist::iterator& p) { __ASSERT_FUNCTION decode_raw(v, p); }

WRITE_RAW_ENCODER(__u8)
#ifndef _CHAR_IS_SIGNED
WRITE_RAW_ENCODER(__s8)
#endif
WRITE_RAW_ENCODER(char)
WRITE_RAW_ENCODER(ceph_le64)
WRITE_RAW_ENCODER(ceph_le32)
WRITE_RAW_ENCODER(ceph_le16)

// FIXME: we need to choose some portable floating point encoding here
WRITE_RAW_ENCODER(float)
WRITE_RAW_ENCODER(double)

inline void encode(const bool &v, bufferlist& bl) {
  __u8 vv = v;
  encode_raw(vv, bl);
}
inline void decode(bool &v, bufferlist::iterator& p) {
  __u8 vv;
  decode_raw(vv, p);
  v = vv;
}


// -----------------------------------
// int types

#define WRITE_INTTYPE_ENCODER(type, etype)				\
  inline void encode(type v, bufferlist& bl, uint64_t features=0) {	\
    ceph_##etype e;					                \
    e = v;                                                              \
    encode_raw(e, bl);							\
  }									\
  inline void decode(type &v, bufferlist::iterator& p) {		\
    ceph_##etype e;							\
    decode_raw(e, p);							\
    v = e;								\
  }

WRITE_INTTYPE_ENCODER(uint64_t, le64)
WRITE_INTTYPE_ENCODER(int64_t, le64)
WRITE_INTTYPE_ENCODER(uint32_t, le32)
WRITE_INTTYPE_ENCODER(int32_t, le32)
WRITE_INTTYPE_ENCODER(uint16_t, le16)
WRITE_INTTYPE_ENCODER(int16_t, le16)
{% endhighlight %}
从上面可以看出，对基本数据类型的编码还是比较简单，不考虑大小端。


## 5. Finisher
类Finisher用来完成回调函数Context的执行，其内部有一个FinisherThread线程来用于执行Context回调函数:
{% highlight string %}
class Finisher {
  CephContext *cct;
  Mutex        finisher_lock;           //Protects access to queues and finisher_running.
  Cond         finisher_cond;           //Signaled when there is something to process
  Cond         finisher_empty_cond;     //Signaled when the finisher has nothing more to process
  bool         finisher_stop            //Set when the finisher should stop
  bool         finisher_running;        //True when the finisher is currently executing contexts.

  //需要执行的Context列表。若对应的Context不为NULL，则执行complete(0)回调； 否则，表明应该执行finisher_queue_rval
  //队列中相应元素的complete(r)回调
  vector<Context*> finisher_queue;

  //执行complete(r)这样的回调 
  list<pair<Context*,int> > finisher_queue_rval;
};
{% endhighlight %}

## 6. Throttle
类Throttle用来限制消费的资源数量（也常称为槽位"slot")，当请求的slot数量达到max值时，请求就会被阻塞在一个```队列```中，直到有新的slot被释放出来，然后会唤醒阻塞在队列中的第一个请求。代码如下（src/common/throttle.cc）：
{% highlight string %}
class Throttle {
  CephContext *cct;
  const std::string name;
  PerfCounters *logger;
  ceph::atomic_t count, max;    //count: 当前占用的slot数量  max: slot数量的最大值
  Mutex lock;                   //等待的互斥锁
  list<Cond*> cond;             //等待的条件变量。这里采用队列，因为在slot被释放时，排在队列头的请求应该优先被满足
  const bool use_perf;
};
{% endhighlight %}

1） **函数get**

函数原型如下：
{% highlight string %}
bool Throttle::get(int64_t c, int64_t m);
{% endhighlight %}
函数get()用于获取数量为c个slot，参数c默认为1，参数m默认为0。如果m不为默认的值0，就用m值重新设置slot的max值。如果成功获取数量为c个slot，就返回true，否则就阻塞等待。

2) **函数get_or_fail**

原型如下：
{% highlight string %}
bool Throttle::get_or_fail(int64_t c);
{% endhighlight %}
函数get_or_fail()当获取不到数量为c个slot时，就直接返回false，不阻塞等待。

3) **函数put**

原型如下：
{% highlight string %}
int64_t Throttle::put(int64_t c);
{% endhighlight %}

函数put用于释放数量为c个slot资源。

## 7. SafeTimer
类SafeTimer实现了定时器的功能，代码如下(src/common/timer.cc)：
{% highlight string %}
class SafeTimer{
  CephContext *cct;
  Mutex& lock;
  Cond cond;
  bool safe_callbacks;         //是否是safe_callbacks

  SafeTimerThread *thread;     //定时器执行线程

  //目标时间和定时任务执行函数Context
  std::multimap<utime_t, Context*> schedule;

  //定时任务<--->定时任务在schedule中的位置映射
  std::map<Context*, std::multimap<utime_t, Context*>::iterator> events;
  bool stopping;
};
{% endhighlight %}

添加定时任务的命令如下：
{% highlight string %}
void SafeTimer::add_event_at(utime_t when, Context *callback);
{% endhighlight %}

取消定时任务的命令如下：
{% highlight string %}
bool SafeTimer::cancel_event(Context *callback);
{% endhighlight %}

定时任务的执行如下：
{% highlight string %}
void SafeTimer::timer_thread();
{% endhighlight %}



<br />
<br />

**[参看]**

1. [非常详细的 Ceph 介绍、原理、架构](https://blog.csdn.net/mingongge/article/details/100788388)





<br />
<br />
<br />

