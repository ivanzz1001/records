---
layout: post
title: ceph客户端
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章我们介绍Ceph的客户端实现。客户端是系统对外提供的功能接口，上层应用通过它来访问ceph存储系统。本章首先介绍librados和Osdc两个模块，通过它们可以直接访问RADOS对象存储系统。其次介绍Cls扩展模块，使用它们可方便地扩展现有的接口。最后介绍librbd模块。由于librados和librbd的多数实现流程都比较类似，本章在介绍相关数据结构后，只选取一些典型的操作流程介绍。

<!-- more -->

## 1. librados
librados是RADOS对象存储系统访问的接口库，它提供了pool的创建、删除，对象的创建、删除、读写等基本操作接口。架构如下图5-5所示：

![ceph-chapter5-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter5_5.jpg)


最上层是类RadosClient，它是Librados的核心管理类，处理整个RADOS系统层面以及pool层面的管理。类IoctxImpl实现单个pool层对象的读写等操作。OSDC模块实现了请求的封装和通过网络模块发送请求的逻辑，其核心类Objecter完成对象的地址计算、消息的发送等工作。



### 1.1 RadosClient
代码如下(src/librados/radosclient.h)：
{% highlight string %}
class librados::RadosClient : public Dispatcher
{
public:
  using Dispatcher::cct;
  md_config_t *conf;                                     //配置文件
private:
  enum {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
  } state;                                              //和monitor的网络连接状态

  MonClient monclient;                                  //Monitor客户端
  Messenger *messenger;                                 //网络消息接口

  uint64_t instance_id;                                 //rados客户端实例的ID
  
  Objecter *objecter;                                   //objecter对象指针

  Mutex lock;
  Cond cond;
  SafeTimer timer;                                     //定时器
  int refcnt;                                          //引用计数

  version_t log_last_version;
  rados_log_callback_t log_cb;
  void *log_cb_arg;
  string log_watch;
  
public:
  Finisher finisher;                                  //用于执行回调函数的Finisher类
  
  ...
};
{% endhighlight %}

通过RadosClient的成员函数，可以了解RadosClient的实现功能如下：

1） 网络连接

connect()函数是RadosClient的初始化函数，完成了许多的初始化工作：

&emsp;a) 调用函数monclient.build_initial_monmap()，从配置文件里检查是否有初始的Monitor地址信息；

&emsp;b) 创建网络通信模块messenger，并设置相关的Policy信息；

&emsp;c) 创建Objecter对象并初始化；
{% highlight string %}
objecter = new (std::nothrow) Objecter(cct, messenger, &monclient,
		  &finisher,
		  cct->_conf->rados_mon_op_timeout,
		  cct->_conf->rados_osd_op_timeout);
if (!objecter)
	goto out;
objecter->set_balanced_budget();

objecter->init();
{% endhighlight %}

&emsp;d) 调用monclient.init()函数初始化monclient
{% highlight string %}
err = monclient.init();
{% endhighlight %}

&emsp;e) Timer定时器初始化，Finisher对象初始化


2) pool的同步和异步创建
{% highlight string %}
int librados::RadosClient::pool_create(string& name, unsigned long long auid,
				       int16_t crush_rule);

int librados::RadosClient::pool_create_async(string& name, PoolAsyncCompletionImpl *c,
					     unsigned long long auid,
					     int16_t crush_rule);
{% endhighlight %}

&emsp;a) 函数pool_create同步创建pool。其实现过程为调用Objecter::create_pool()函数，构造PoolOp操作，通过monitor的客户端monc发送请求给Monitor创建一个pool，并同步等待请求的返回；

&emsp;b) 函数pool_create_async异步创建。与同步方式的区别在于注册了回调函数，当创建成功后，执行回调函数通知完成。

3） pool的同步和异步删除

函数delete_pool完成同步删除，函数delete_pool_async异步删除。其过程和pool的创建过程相同，向Monitor发送删除请求。

4） 查找pool和列举pool

函数lookup_pool()用于查找pool，函数pool_list用于列出所有的pool。pool相关的信息都保存在OsdMap中。

5) 获取pool和系统的信息

函数get_pool_stats()用于获取pool的统计信息，函数get_fs_stats()用于获取系统的统计信息。
{% highlight string %}
int librados::RadosClient::get_pool_stats(std::list<string>& pools,
					  map<string,::pool_stat_t>& result);

int librados::RadosClient::get_fs_stats(ceph_statfs& stats);
{% endhighlight %}



6) 命令处理

函数mon_command()处理Monitor相关的命令，它调用函数monclient.start_mon_command()把命令发送给monitor处理；函数osd_command处理OSD相关的命令，它调用函数objecter->osd_command()把命令发送给对应OSD处理。函数pg_command()处理PG相关命令，它调用函数objecter->pg_command()把命令发送给该PG的主OSD来处理。

7） 创建IoCtxImpl对象

函数create_ioctx()创建一个pool相关的山下文信息IoCtxImpl对象。


### 1.2 IoCtxImpl
类IoCtxImpl(src/librados/IoCtxImpl.h)是pool操作相关的上下文信息，一个IoCtxImpl对象对应着一个pool（注： 对于一个pool，我们可以创建多个IoCtxImpl对象)，可以在该pool里创建、删除对象，完成对象的数据读写等各种操作，包括同步和异步的实现。其处理过程都比较简单，而且过程类似：

1） 把请求封装成ObjectOperation类（该类定义在src/osdc/Objecter.h中）

2) 然后再添加pool的地址信息，封装成Objecter::Op对象

3） 调用函数objecter->op_submit发送给相应的OSD。如果是同步操作，就等待操作完成；如果是异步操作，就不用等待，直接返回。当操作完成后，调用相应的回调函数通知。


## 2. OSDC
OSDC是客户端比较底层的模块，其核心在于封装操作数据，计算对象的地址，发送请求和处理超时。代码位于src/osdc目录下：
<pre>
# ls
Filer.cc  Filer.h  Journaler.cc  Journaler.h  Makefile.am  ObjectCacher.cc  ObjectCacher.h  Objecter.cc  Objecter.h  Striper.cc  Striper.h  WritebackHandler.h
</pre>

### 2.1 ObjectOperation
类ObjectOperation用于将操作相关的参数统一封装在该类里，该类可以一次封装多个对象的操作(src/osdc/objecter.h)：
{% highlight string %}
struct ObjectOperation {
  vector<OSDOp> ops;                 //多个操作
  int flags;                         //操作的标志
  int priority;                      //优先级

  vector<bufferlist*> out_bl;        //每个操作对应的输出缓冲区队列
  vector<Context*> out_handler;      //每个操作对应的回调函数队列
  vector<int*> out_rval;             //每个操作对应的操作结果队列
};
{% endhighlight %}
封装的对象操作主要包括如下几类：

* object的创建、读写、遍历

* pg的遍历

* object的xattr的创建、遍历

* object的omap的创建、遍历

类OSDOp封装对象的一个操作。结构体ceph_osd_op封装一个操作的操作码和相关的输入和输出参数(src/osd/osd_types.h)：
{% highlight string %}
struct OSDOp {
  ceph_osd_op op;                        //各种操作码和操作参数
  sobject_t soid;                        //操作对象

  bufferlist indata, outdata;            //输入和输出bufferlist
  int32_t rval;                          //操作结果
};
{% endhighlight %}
>注： 这里OSDOp通常只是操作的封装，具体操作的对象名可能不会在这里设置，一般需要搭配Objecter一起使用


### 2.2 op_target

结构op_target_t封装了对象所在的PG，以及PG对应的OSD列表等地址信息(src/osdc/objecter.h)：
{% highlight string %}
struct op_target_t {
	int flags;                               //标志
	object_t base_oid;                       //所操作的对象
	object_locator_t base_oloc;              //对象的pool信息
	object_t target_oid;                     //最终操作的目标对象
	object_locator_t target_oloc;            //最终目标对象的pool信息。这里由于Cache tier的存在，导致产生最终操作的目标和pool的不同
	
	bool precalc_pgid;                       //是否使用预先计算好的PG id
	pg_t base_pgid;                          //
	
	pg_t pgid;                              //上一次所映射的PG
	unsigned pg_num;                        //last pg_num we mapped to
	unsigned pg_num_mask;                   //last pg_num_mask we mapped to
	vector<int> up;                         //set of up osds for last pg we mapped to
	vector<int> acting;                     //set of acting osds for last pg we mapped to
	int up_primary;                         //primary for last pg we mapped to based on the up set
	int acting_primary;                     //primary for last pg we mapped to based on the acting set
	int size;                               //the size of the pool when were were last mapped
	int min_size;                           //the min size of the pool when were were last mapped
	bool sort_bitwise;                      //whether the hobject_t sort order is bitwise
	
	bool used_replica;
	bool paused;
	
	int osd;                                ///< the final target osd, or -1
};
{% endhighlight %}

### 2.3 Op
结构Op封装了完成一个操作的相关上下文信息，包括target地址信息、链接信息等(src/osdc/objecter.h)：
{% highlight string %}
struct Op : public RefCountedObject {
    OSDSession *session;                  //osd相关的session信息
    int incarnation;                      //引用次数

    op_target_t target;                   //操作的目标地址信息

    ConnectionRef con;                   // for rx buffer only
    uint64_t features;                   // explicitly specified op features

    vector<OSDOp> ops;                   //对应多个操作的封装

    snapid_t snapid;                     //快照的id
    SnapContext snapc;                   //pool层级的快照信息
    ceph::real_time mtime;

    bufferlist *outbl;                   //输出的bufferlist
    vector<bufferlist*> out_bl;          //每个操作对应的bufferlist
    vector<Context*> out_handler;        //每个操作对应的回调函数
    vector<int*> out_rval;               //每个操作对应的输出结果

    int priority;                        
    Context *onack, *oncommit;
    uint64_t ontimeout;
    Context *oncommit_sync; // used internally by watch/notify

    ceph_tid_t tid;
    eversion_t replay_version; // for op replay
    int attempts;

    version_t *objver;
    epoch_t *reply_epoch;

    ceph::mono_time stamp;

    epoch_t map_dne_bound;

    bool budgeted;

    /// true if we should resend this message on failure
    bool should_resend;

    /// true if the throttle budget is get/put on a series of OPs,
    /// instead of per OP basis, when this flag is set, the budget is
    /// acquired before sending the very first OP of the series and
    /// released upon receiving the last OP reply.
    bool ctx_budgeted;

    int *data_offset;

    epoch_t last_force_resend;

    osd_reqid_t reqid; // explicitly setting reqid
};
{% endhighlight %}

### 2.4 Objecter

类Objecter主要完成对象的地址计算、消息的发送等工作(src/osdc/objecter.h)：
{% highlight string %}
class Objecter : public md_config_obs_t, public Dispatcher {
public:
  Messenger *messenger;                          //网络消息发送接收器
  MonClient *monc;                               //monitor客户端
  Finisher *finisher;                            //消息异步处理的Finisher线程
private:
  OSDMap    *osdmap;                             //osd map
public:
  using Dispatcher::cct;                         //ceph上下文
  std::multimap<string,string> crush_location;   //指定的crush_location来读取数据

  atomic_t initialized;

private:
  atomic64_t last_tid;                           //事务id
  atomic_t inflight_ops;                         //当前处于flight状态的op数量
  atomic_t client_inc;                           //用于唯一标识一个client
  uint64_t max_linger_id;
  atomic_t num_unacked;
  atomic_t num_uncommitted;
  atomic_t global_op_flags;                      // flags which are applied to each IO op
  bool keep_balanced_budget;
  bool honor_osdmap_full;

};
{% endhighlight %}

>注：关于crush_location的使用，可以参看https://ceph.com/planet/ceph%E6%A0%B9%E6%8D%AEcrush%E4%BD%8D%E7%BD%AE%E8%AF%BB%E5%8F%96%E6%95%B0%E6%8D%AE/

### 2.4 Striper
对象有分片(stripe)时，类Stripe用于完成对象分片数据的计算。数据结构ceph_file_layout用来保存文件或者image的分片信息(src/include/fs_types.h)：
{% highlight string %}
struct ceph_file_layout {               //文件 -> 对象的映射
	__le32 fl_stripe_unit;              //条带单元块大小（单位：字节），必须是page_size的倍数
	__le32 fl_stripe_count;             //stripe跨越的对象数
	__le32 fl_object_size;              //object的大小
				  
	__le32 fl_cas_hash;                 /* UNUSED.  0 = none; 1 = sha256 */

	/* pg -> disk layout */
	__le32 fl_object_stripe_unit;       /* UNUSED.  for per-object parity, if any */

	/* object -> pg layout */
	__le32 fl_unused;                  /* unused; used to be preferred primary for pg (-1 for none) */
	__le32 fl_pg_pool;                 /* namespace, crush ruleset, rep level */
} __attribute__ ((packed));
{% endhighlight %}

对象ObjectExtent用来记录对象内的分片信息(src/osd/osd_types.h)：
{% highlight string %}
class ObjectExtent {

public:
  object_t    oid;                                       //对象的id
  uint64_t    objectno;                                  //属于object set中的哪一个Object
  uint64_t    offset;                                    //对象内的偏移
  uint64_t    length;                                    //长度
  uint64_t    truncate_size;	                         //对象truncate的操作的size

  object_locator_t oloc;                                 //对象的位置信息，例如在哪个pool中等等

  vector<pair<uint64_t,uint64_t> >  buffer_extents;     //Extents在buffer中的偏移和长度(off -> len)，可能有多个extents
};

void Striper::file_to_extents(
  CephContext *cct, const char *object_format,
  const file_layout_t *layout,                         //分片信息
  uint64_t offset, uint64_t len,                       //文件的偏移、长度
  uint64_t trunc_size,
  map<object_t,vector<ObjectExtent> >& object_extents, //分布到每个对象的数据段
  uint64_t buffer_offset)                              //在buffer中的偏移量
{% endhighlight %}
函数file_to_extents()完成了file到对象stripe后的映射。只有了解清楚了每个概念，计算方法都比较简单。下面举例说明。

```例5-1``` file_to_extents示例

![ceph-chapter5-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter5_6.jpg)

如上图5-5所示，要计算的文件的offset为2KB，length为16KB。文件的分片信息： stripe unit为4KB，stripe count为3，object size为8KB。则对象Object 0对应的ObjectExtent为：
{% highlight string %}
object_extents["Object 0"] = {
	oid = "Object 0",
	objectno = 0,
	offset = 2KB,
	length = 6KB,
	buffer_extents = { [0, 2KB], [10KB, 4KB]}
}
{% endhighlight %}

其中，oid就是映射对象的id，objectno为stripe对象的序号， offset为映射的数据段在对象内的起始偏移，length为对象内的长度。buffer_extents为映射的数据在buffer内的偏移和长度。

### 2.5 ObjectCacher
类ObjectCacher提供了客户端的基于LRU算法的对象数据缓存功能，其实比较简单，这里就不深入分析了(src/osdc/ObjectCacher.h)。

### 2.6 小结

librados层的整体架构如下：

![ceph-chapter5-7](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter5_7.jpg)

对于librados我们可以分如下两个层面来看：

1） 从大的接口封装层面来说，提供了C语言的封装rados_t，以及C++语言的抽象librados::Rados(注意： Objecter处于全局名称空间，而其他处于librados名称空间)

2) 从RadosClient、IoCtxImpl、Objecter层面来说，是一个抽象到具体的一个实现的过程。RadosClient是粗粒度的Rados操作接口；IoCtxImpl是针对某一个pool的较为细粒度的操作接口；而Objecter则是object层面的更为细粒度的操作实现。


## 3. 客户端写操作分析
以下代码是通过librados库的接口写入数据到对象中的典型例程，对象的其他操作过程都类似：
{% highlight string %}
rados_t cluster;
rados_ioctx_t ioctx;

rados_create(&cluster, NULL);
rados_conf_read_file(cluster, NULL);
rados_connect(cluster);
rados_ioctx_create(cluster, pool_name.c_str(), &ioctx);
rados_write(ioctx, "foo", buf, sizeof(buf), 0);
{% endhighlight %}

上述代码是C语言接口完成的，其流程如下：

1） 首先调用rados_create()函数创建一个RadosClient对象，输出类型为rados_t，它是一个void类型的指针，通过librados::RadosClient对象的强制转换产生。第二个参数id为一个标识符，一般传入为NULL。

2） 调用函数rados_conf_read来读取配置文件。第二个参数为配置文件的路径，如果是NULL，就搜索默认的配置文件。

3） 调用rados_connect()函数，它调用了RadosClient的connect()函数，做相关的初始化工作。

4） 调用函数rados_ioctx_create()，它调用RadosClient的create_ioctx()函数，创建pool相关的IoCtxImpl类，其输出类型为rados_ioctx_t，它也是void类型的指针，有IoCtxImpl对象转换而来；

5） 调用函数rados_write()函数，向该pool的名为```foo```的对象写入数据。此调用IoCtxImpl的write()操作

### 3.1 写操作消息封装
本函数完成具体的写操作，代码如下(src/librados/IoCtxImpl.cc)：
{% highlight string %}
int librados::IoCtxImpl::write(const object_t& oid, bufferlist& bl,
			       size_t len, uint64_t off)
{
  if (len > UINT_MAX/2)
    return -E2BIG;
  ::ObjectOperation op;
  prepare_assert_ops(&op);
  bufferlist mybl;
  mybl.substr_of(bl, 0, len);
  op.write(off, mybl);
  return operate(oid, &op, NULL);
}
{% endhighlight %}
其实现过程如下：

1） 创建ObjectOperation对象，封装写操作的相关参数；

2） 调用函数operate()完成处理:
{% highlight string %}
int librados::IoCtxImpl::operate(const object_t& oid, ::ObjectOperation *o,
				 ceph::real_time *pmtime, int flags);
{% endhighlight %}
&emsp; a) 调用函数objecter->prepare_mutate_op()把ObjectOperation类型封装成Op类型，添加了object_locator_t相关的pool信息；
{% highlight string %}
// a locator constrains the placement of an object.  mainly, which pool
// does it go in.
struct object_locator_t {
  // You specify either the hash or the key -- not both
  int64_t pool;                     ///< pool id
  string key;                       ///< key string (if non-empty)
  string nspace;                    ///< namespace
  int64_t hash;                     ///< hash position (if >= 0)
};
{% endhighlight %}

&emsp; b) 调用objecter->op_submit()把消息发送出去；

&emsp; c) 等待操作完成；

### 3.2 发送数据op_submit
函数op_submit()用来把封装好的操作Op通过网络发送出去：
{% highlight string %}
void Objecter::op_submit(Op *op, ceph_tid_t *ptid, int *ctx_budget)
{
  shunique_lock rl(rwlock, ceph::acquire_shared);
  ceph_tid_t tid = 0;
  if (!ptid)
    ptid = &tid;
  _op_submit_with_budget(op, rl, ptid, ctx_budget);
}
{% endhighlight %}
函数_op_submit_with_budget()用来处理Throttle相关的流量限制。如果osd_timeout大于0，就设置定时器，当操作超时，就调用定时器回调函数op_cancel取消操作：
{% highlight string %}
void Objecter::_op_submit_with_budget(Op *op, shunique_lock& sul,
				      ceph_tid_t *ptid,
				      int *ctx_budget)
{
  assert(initialized.read());

  assert(op->ops.size() == op->out_bl.size());
  assert(op->ops.size() == op->out_rval.size());
  assert(op->ops.size() == op->out_handler.size());

  // throttle.  before we look at any state, because
  // _take_op_budget() may drop our lock while it blocks.
  if (!op->ctx_budgeted || (ctx_budget && (*ctx_budget == -1))) {
    int op_budget = _take_op_budget(op, sul);
    // take and pass out the budget for the first OP
    // in the context session
    if (ctx_budget && (*ctx_budget == -1)) {
      *ctx_budget = op_budget;
    }
  }

  if (osd_timeout > timespan(0)) {
    if (op->tid == 0)
      op->tid = last_tid.inc();
    auto tid = op->tid;
    op->ontimeout = timer.add_event(osd_timeout,
				    [this, tid]() {
				      op_cancel(tid, -ETIMEDOUT); });
  }

  _op_submit(op, sul, ptid);
}
{% endhighlight %}

函数_op_submit完成了关键的地址寻址和发送工作，其处理过程如下：

1) 调用函数_calc_target来计算对象的目标OSD

2） 调用函数_get_session获取目标OSD的链接，如果返回值为-EAGAIN，就升级为写锁，重新获取；

3） 检查当前的状态标志，如果当前是```CEPH_OSDMAP_PAUSEWR```或者```OSD空间满```(注： 因为这里我们讲述的是对象的写操作，因此flag中CEPH_OSD_FLAG_WRITE肯定被设置），就暂时不发送请求；否则调用函数_prepare_osd_op()准备请求的消息，然后调用函数_send_op()发送出去。

### 3.3 对象寻址_calc_target
函数_calc_target用于完成对象到OSD的寻址过程：
{% highlight string %}
int Objecter::_calc_target(op_target_t *t, epoch_t *last_force_resend,
			   bool any_change);
{% endhighlight %}
其处理过程如下：

1） 首先根据```t->base_oloc.pool```的pool信息，获取pg_pool_t对象；
{% highlight string %}
const pg_pool_t *pi = osdmap->get_pg_pool(t->base_oloc.pool);
if (!pi) {
	t->osd = -1;
	return RECALC_OP_TARGET_POOL_DNE;
}
{% endhighlight %}

2） 检查如果强制重发，force_resend设置为true;

3) 检查cache tier，如果是读操作，并且有读缓存，就设置target_oloc.pool为该pool的read_tier值；如果是写操作，并且有写缓存，就设置target_oloc.pool为该pool的write_tier值
{% highlight string %}
if (need_check_tiering &&
	(t->flags & CEPH_OSD_FLAG_IGNORE_OVERLAY) == 0) {

	if (is_read && pi->has_read_tier())
		t->target_oloc.pool = pi->read_tier;
	if (is_write && pi->has_write_tier())
		t->target_oloc.pool = pi->write_tier;
}
{% endhighlight %}


4) 调用函数osdmap->object_locator_to_pg()获取目标对象所在的PG
{% highlight string %}
int ret = osdmap->object_locator_to_pg(t->target_oid, t->target_oloc,
					   pgid);
{% endhighlight %}

5) 调用函数osdmap->pg_to_up_acting_osds()，通过CRUSH算法，获取该PG对应的OSD列表；
{% highlight string %}
osdmap->pg_to_up_acting_osds(pgid, &up, &up_primary,
			       &acting, &acting_primary);
{% endhighlight %}

6） 如果是写操作，target的OSD就设置为主OSD；如果是读操作，如果设置了CEPH_OSD_FLAG_BALANCE_READS标志，就随机选择一个副本读取；如果设置了CEPH_OSD_FLAG_LOCALIZE_READS标志，就尽可能选择本地副本来读取；否则选择主OSD来进行读取
{% highlight string %}
int osd;
bool read = is_read && !is_write;

if (read && (t->flags & CEPH_OSD_FLAG_BALANCE_READS)) {
	int p = rand() % acting.size();
	if (p)
		t->used_replica = true;

	osd = acting[p];
	ldout(cct, 10) << " chose random osd." << osd << " of " << acting << dendl;

} 
else if (read && (t->flags & CEPH_OSD_FLAG_LOCALIZE_READS) && acting.size() > 1) {

	// look for a local replica.  prefer the primary if the
	// distance is the same.
	int best = -1;
	int best_locality = 0;

	for (unsigned i = 0; i < acting.size(); ++i) {
		int locality = osdmap->crush->get_common_ancestor_distance(
			cct, acting[i], crush_location);

		ldout(cct, 20) << __func__ << " localize: rank " << i << " osd." << acting[i]
			<< " locality " << locality << dendl;

		if (i == 0 || (locality >= 0 && best_locality >= 0 && locality < best_locality) ||
			(best_locality < 0 && locality >= 0)) {

				best = i;
				best_locality = locality;

				if (i)
					t->used_replica = true;
		}
	}

	assert(best >= 0);
	osd = acting[best];
} 
else {
	osd = acting_primary;
}
t->osd = osd;
{% endhighlight %}


### 3.4 查询object对应的PG

object到PG的映射是通过object_locator_to_pg()函数来实现的(src/osd/osdmap.cc):
{% highlight string %}
// mapping
int OSDMap::object_locator_to_pg(
	const object_t& oid,
	const object_locator_t& loc,
	pg_t &pg) const
{
  // calculate ps (placement seed)
  const pg_pool_t *pool = get_pg_pool(loc.get_pool());
  if (!pool)
    return -ENOENT;
  ps_t ps;
  if (loc.hash >= 0) {
    ps = loc.hash;
  } else {
    if (!loc.key.empty())
      ps = pool->hash_key(loc.key, loc.nspace);
    else
      ps = pool->hash_key(oid.name, loc.nspace);
  }
  pg = pg_t(ps, loc.get_pool(), -1);
  return 0;
}
{% endhighlight %}
从上面来看，过程比较简单，首先计算ps(placement seed)，然后根据pool id就可以获得PG。

### 3.5 通过pg求OSD列表
pg到OSD列表的映射是通过pg_to_up_acting_osds()来实现的（src/osd/osdmap.cc)：
{% highlight string %}
/**
* map a pg to its acting set as well as its up set. You must use
* the acting set for data mapping purposes, but some users will
* also find the up set useful for things like deciding what to
* set as pg_temp.
* Each of these pointers must be non-NULL.
*/
void pg_to_up_acting_osds(pg_t pg, vector<int> *up, int *up_primary,
	vector<int> *acting, int *acting_primary) const {

	_pg_to_up_acting_osds(pg, up, up_primary, acting, acting_primary);
}
void pg_to_up_acting_osds(pg_t pg, vector<int>& up, vector<int>& acting) const {
	int up_primary, acting_primary;
	pg_to_up_acting_osds(pg, &up, &up_primary, &acting, &acting_primary);
}

void OSDMap::_pg_to_up_acting_osds(const pg_t& pg, vector<int> *up, int *up_primary,
                                   vector<int> *acting, int *acting_primary) const
{
	const pg_pool_t *pool = get_pg_pool(pg.pool());
	if (!pool) {
		if (up)
			up->clear();

		if (up_primary)
			*up_primary = -1;

		if (acting)
			acting->clear();

		if (acting_primary)
			*acting_primary = -1;

		return;
	}

	vector<int> raw;
	vector<int> _up;
	vector<int> _acting;
	int _up_primary;
	int _acting_primary;
	ps_t pps;

	_pg_to_osds(*pool, pg, &raw, &_up_primary, &pps);
	_raw_to_up_osds(*pool, raw, &_up, &_up_primary);
	_apply_primary_affinity(pps, *pool, &_up, &_up_primary);
	_get_temp_osds(*pool, pg, &_acting, &_acting_primary);

	if (_acting.empty()) {
		_acting = _up;

		if (_acting_primary == -1) {
			_acting_primary = _up_primary;
		}
	}

	if (up)
		up->swap(_up);

	if (up_primary)
		*up_primary = _up_primary;

	if (acting)
		acting->swap(_acting);

	if (acting_primary)
		*acting_primary = _acting_primary;
}
{% endhighlight %}
pg_to_up_acting_osds()函数实现将pg映射为acting set。我们在进行对象读写时，都需要通过acting set来进行（对于pg_temp等少数情况，我们可能只需要知道PG对应的up set即可，也可以通过本函数来返回）。其实现步骤如下：

1） _pg_to_osds()会根据pool所对应的crush rule来计算出PG所对应的OSD
{% highlight string %}
int ruleno = crush->find_rule(pool.get_crush_ruleset(), pool.get_type(), size);
if (ruleno >= 0)
	crush->do_rule(ruleno, pps, *osds, size, osd_weight);
{% endhighlight %}
此时计算出来的OSD其实只是原始的仍在osd map列表中存在的OSD(这里在调用该函数时我们传递的是raw)，之间是没有primary、secondary、tertiary这样区分的。

2） _raw_to_up_osds()函数从raw中返回当前仍处于UP状态的OSD，把处于down状态的进行```标记```或```移除```。同时这里简单的将返回的OSD列表中的第一个作为up_primary

3) _apply_primary_affinity()用于计算up_primary的亲和性，就是根据一定的规则从up osds里面选出其中一个作为primay。

4） _get_temp_osds()用于求出PG对应的acting set
{% highlight string %}
void OSDMap::_get_temp_osds(const pg_pool_t& pool, pg_t pg,
                            vector<int> *temp_pg, int *temp_primary) const
{
	pg = pool.raw_pg_to_pg(pg);

	map<pg_t,vector<int32_t> >::const_iterator p = pg_temp->find(pg);
	temp_pg->clear();
	if (p != pg_temp->end()) {
		for (unsigned i=0; i<p->second.size(); i++)
		{
			if (!exists(p->second[i]) || is_down(p->second[i])) 
			{
				if (pool.can_shift_osds()) {
					continue;
				} else {
					temp_pg->push_back(CRUSH_ITEM_NONE);
				}
			} 
			else {
				temp_pg->push_back(p->second[i]);
			}
		}
	}

	map<pg_t,int32_t>::const_iterator pp = primary_temp->find(pg);
	*temp_primary = -1;

	if (pp != primary_temp->end()) {
		*temp_primary = pp->second;
	} 
	else if (!temp_pg->empty()) { // apply pg_temp's primary
		for (unsigned i = 0; i < temp_pg->size(); ++i) 
		{
			if ((*temp_pg)[i] != CRUSH_ITEM_NONE) {
				*temp_primary = (*temp_pg)[i];
				break;
			}
		}
	}
}
{% endhighlight %}

上面我们看到首先调用pool.raw_pg_to_pg(pg)将raw pg转换，然后再在pg_temp中查找转换后的PG所对应的OSD列表即为acting set。

### 3.5 写操作的应答
上面讲写请求发送出去之后，就会等待相应的应答。当从网络收到应答之后，首先会回调：
{% highlight string %}
bool Objecter::ms_dispatch(Message *m)
{
  ldout(cct, 10) << __func__ << " " << cct << " " << *m << dendl;
  if (!initialized.read())
    return false;

  switch (m->get_type()) {
    // these we exlusively handle
  case CEPH_MSG_OSD_OPREPLY:
    handle_osd_op_reply(static_cast<MOSDOpReply*>(m));
  }

  ....
}
{% endhighlight %}
对于对象的读写操作，应答都是```CEPH_MSG_OSD_OPREPLY```消息，其会调用handle_osd_op_reply()来进行处理。

## 4. Cls
Cls是Ceph的一个扩展模块，它允许用户自定义对象的操作接口和实现方法，为用户提供了一种比较方便的接口扩展方式。目前rbd和lock等模块都采用了这种机制（即通过加载额外的动态链接库的方式来进行处理，类似于Nginx中的dynamic module）。

### 4.1 模块以及方法的注册
类ClassHandler用来管理所有的扩展模块。函数register_class用来注册模块(src/osd/ClassHandler.h)：
{% highlight string %}
class ClassHandler
{
public:
  CephContext *cct;
  Mutex mutex;

  map<string, ClassData> classes;    //所有注册的模块： 模块名->模块元数据信息。
};
{% endhighlight %}
一个ClassData代表一个模块，一个模块中可以有很多方法。可以在一个ClassHandler中注册多个模块。


类ClassData描述了一个模块相关的元数据信息。它描述了一个扩展模块的相关信息，包括模块名、模块相关的操作方法以及依赖的模块：
{% highlight string %}
struct ClassData {
    enum Status { 
      CLASS_UNKNOWN,                           //初始未知状态
      CLASS_MISSING,                           //缺失状态(动态链接库找不到)
      CLASS_MISSING_DEPS,                      //依赖的模块缺失
      CLASS_INITIALIZING,                      //正在初始化
      CLASS_OPEN,                             //已经初始化(动态链接库以及加载成功)
    } status;                                 //当前模块的加载状态

    string name;                              //模块的名字
    ClassHandler *handler;                    //管理模块的指针
    void *handle;                             //指向加载动动态链接库(dlopen())

    map<string, ClassMethod> methods_map;     //模块下所有注册的ClassMethod
    map<string, ClassFilter> filters_map;     //模块下所有的过滤器ClassFilter

    set<ClassData *> dependencies;            //本模块所依赖的模块
    set<ClassData *> missing_dependencies;    //缺失的依赖模块
};
{% endhighlight %}

ClassMethod定义一个模块具体的方法名，以及函数类型：
{% highlight string %}
struct ClassMethod {
	struct ClassHandler::ClassData *cls;      //所属模块的ClassData的指针
	string name;                              //方法的名字
	int flags;                                //方法相关的标志
	cls_method_call_t func;                   //C类型函数指针
	cls_method_cxx_call_t cxx_func;           //C++类型函数指针
};
{% endhighlight %}

在src/objclass/Objectclass.h， src/objclass/class_api.c里定义了一些辅助函数用来注册模块以及方法：

* 注册一个模块如下
{% highlight string %}
int cls_register(const char *name, cls_handle_t *handle);
{% endhighlight %}

* 注册一个模块的方法如下
{% highlight string %}
int cls_register_method(cls_handle_t hclass, const char *method,
                        int flags,
                        cls_method_call_t class_call, cls_method_handle_t *handle);
{% endhighlight %}

### 4.2 cls模块的工作原理
1） ClassHandler的初始化
在src/osd/OSD.cc的init()函数中有如下：
{% highlight string %}
int OSD::init()
{
	....

	class_handler = new ClassHandler(cct);
	cls_initialize(class_handler);

	...
}
{% endhighlight %}
通过上面我们看到是调用cls_initialize()来完成初始化，现在我们来看看该函数(src/objclass/Class_api.cc):
{% highlight string %}
static ClassHandler *ch;

#define dout_subsys ceph_subsys_objclass

void cls_initialize(ClassHandler *h)
{
  ch = h;
}
{% endhighlight %}
这里可以看到，是将OSD::init()中创建的ClassHandler对象赋给了一个静态全局变量。

2） 加载动态链接库

通过上面创建出的ClassHandler对象调用open_all_classes()来加载ceph配置文件中以```libcls_```开头的动态链接库，加载步骤如下：
{% highlight string %}
int ClassHandler::_load_class(ClassData *cls)
{
  // already open
  if (cls->status == ClassData::CLASS_OPEN)
    return 0;

  if (cls->status == ClassData::CLASS_UNKNOWN ||
      cls->status == ClassData::CLASS_MISSING) {
    char fname[PATH_MAX];
    snprintf(fname, sizeof(fname), "%s/" CLS_PREFIX "%s" CLS_SUFFIX,
	     cct->_conf->osd_class_dir.c_str(),
	     cls->name.c_str());
    dout(10) << "_load_class " << cls->name << " from " << fname << dendl;

    cls->handle = dlopen(fname, RTLD_NOW);
    if (!cls->handle) {
      struct stat st;
      int r = ::stat(fname, &st);
      if (r < 0) {
        r = -errno;
        dout(0) << __func__ << " could not stat class " << fname
                << ": " << cpp_strerror(r) << dendl;
      } else {
	dout(0) << "_load_class could not open class " << fname
      	        << " (dlopen failed): " << dlerror() << dendl;
      	r = -EIO;
      }
      cls->status = ClassData::CLASS_MISSING;
      return r;
    }

    cls_deps_t *(*cls_deps)();
    cls_deps = (cls_deps_t *(*)())dlsym(cls->handle, "class_deps");
    if (cls_deps) {
      cls_deps_t *deps = cls_deps();
      while (deps) {
	if (!deps->name)
	  break;
	ClassData *cls_dep = _get_class(deps->name);
	cls->dependencies.insert(cls_dep);
	if (cls_dep->status != ClassData::CLASS_OPEN)
	  cls->missing_dependencies.insert(cls_dep);
	deps++;
      }
    }
  }

  // resolve dependencies
  set<ClassData*>::iterator p = cls->missing_dependencies.begin();
  while (p != cls->missing_dependencies.end()) {
    ClassData *dc = *p;
    int r = _load_class(dc);
    if (r < 0) {
      cls->status = ClassData::CLASS_MISSING_DEPS;
      return r;
    }
    
    dout(10) << "_load_class " << cls->name << " satisfied dependency " << dc->name << dendl;
    cls->missing_dependencies.erase(p++);
  }
  
  // initialize
  void (*cls_init)() = (void (*)())dlsym(cls->handle, "__cls_init");
  if (cls_init) {
    cls->status = ClassData::CLASS_INITIALIZING;
    cls_init();
  }
  
  dout(10) << "_load_class " << cls->name << " success" << dendl;
  cls->status = ClassData::CLASS_OPEN;
  return 0;
}
{% endhighlight %}
从上面我们可以看到步骤较为简单：

* 调用dlopen()打开动态链接库

* 调用dlsym()读取动态链接库中的class_deps()函数，从而获得该动态链接库所需要的一些依赖；

* 调用dlsym()读取动态链接库中的```__cls_init()```函数，从而完成对应模块的初始化。下面我们以src/cls/rbd/cls_rbd.cc为例，大体看一下__cls_init()的实现：
{% highlight string %}
void __cls_init()
{
  CLS_LOG(20, "Loaded rbd class!");

  cls_register("rbd", &h_class);
  cls_register_cxx_method(h_class, "create",
			  CLS_METHOD_RD | CLS_METHOD_WR,
			  create, &h_create);

  ...
}
{% endhighlight %}
从上面我们可以看到，其首先调用cls_register()向ClassHandler注册该模块，其实就是在ClassHandler中创建了一个ClassData。然后，调用cls_register_cxx_method()来向该模块注册C++方法，这样该ClassData中就有该模块所有相关的方法了。

### 4.3 模块的方法执行
模块方法的执行在类ReplicatedPG的函数do_osd_ops()里实现（src/osd/ReplicatedPG.cc）。执行方法对应的操作码为CEPH_OSD_OP_CALL值：
{% highlight string %}
int ReplicatedPG::do_osd_ops(OpContext *ctx, vector<OSDOp>& ops)
{
	....
	case CEPH_OSD_OP_CALL:
		ClassHandler::ClassData *cls;
		result = osd->class_handler->open_class(cname, &cls);
		assert(result == 0);      //函数init_op_flags()已经对结果做了验证

		//根据方法名获取方法
		ClassHandler::ClassMethod *method = cls->get_method(mname.c_str());

		//执行方法
		result = method->exec((cls_method_context_t)&ctx, indata, outdata);
	....
}
{% endhighlight %}



<br />
<br />

**[参看]**

1. [Ceph 的物理和逻辑结构](https://www.cnblogs.com/sammyliu/p/4836014.html)

2. [小甲陪你一起看Ceph](https://cloud.tencent.com/developer/article/1428004)



<br />
<br />
<br />

