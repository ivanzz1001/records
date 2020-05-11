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
  uint64_t    objectno;                                  //分片序号
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




<br />
<br />

**[参看]**

1. [Ceph 的物理和逻辑结构](https://www.cnblogs.com/sammyliu/p/4836014.html)

2. [小甲陪你一起看Ceph](https://cloud.tencent.com/developer/article/1428004)



<br />
<br />
<br />

