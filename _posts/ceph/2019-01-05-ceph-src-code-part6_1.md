---
layout: post
title: ceph的数据读写
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph的服务端OSD（书中简称OSD模块或者OSD）的实现。其对应的源代码在src/osd目录下。OSD模块是Ceph服务进程的核心实现，它实现了服务端的核心功能。本章先介绍OSD模块静态类图相关数据结构，再着重介绍服务端数据的写入和读取流程。

<!-- more -->


## 1. OSD模块静态类图
OSD模块的静态类图如下图6-1所示：

![ceph-chapter5-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter6_1.jpg)

OSD模块的核心类及其之间的静态类图说明如下：

* 类OSD和OSDService是核心类，处理一个osd节点层面的工作。在早期的版本中，OSD和OSDService是一个类。由于OSD的类承载了太多的功能，后面的版本中引入OSDService类，分担一部分原OSD类的功能；

* 类PG处理PG相关的状态维护以及实现PG层面的基本功能。其核心功能是用boost库的statechart状态机来实现的PG状态转换；

* 类ReplicatedPG继承了类PG，在其基础上实现了PG内的数据读写以及数据恢复相关的操作；

* 类PGBackend的主要功能是把数据以事务的形式同步到一个PG其他从OSD节点上

&emsp; - PGBackend的内部类PGTransaction就是同步的事务接口，其两个类型的实现分别对应RPGTransaction和ECTransaction两个子类；

&emsp; - PGBackend两个子类ReplicatedBackend和ECBackend分别对应PG的两种类型的实现


* 类SnapMapper额外保存对象和对象的快照信息，在对象的属性里保存了相关的快照信息。这里保存的快照信息为冗余信息，用于数据校验。

## 2. 相关数据结构
下面将介绍OSD模块相关的一些核心的数据结构。从最高的逻辑层次为pool的概念，然后是PG的概念。其次是OSDMap记录了集群的所有的配置信息。数据结构OSDOp是一个操作上下文的封装。结构object_info_t保存了一个对象的元数据信息和访问信息。对象ObjectState是在object_info_t的基础上添加了一些内存的状态信息。SnapSetContext和ObjectContext分别保存了快照和对象的上下文相关的信息。Session保存了一个端到端的链接相关的上下文信息。

### 2.1 Pool
Pool是整个集群层面定义的一个逻辑的存储池。对一个Pool可以设置相应的数据冗余类型，目前有副本和纠删码两种实现。数据结构pg_pool_t用于保存Pool的相关信息。Pool的数据结构如下(src/osd/osd_types.h)：
{% highlight string %}
struct pg_pool_t {
	enum {
		TYPE_REPLICATED = 1,     //副本
		//TYPE_RAID4 = 2,        //从来没有实现的raid4
		TYPE_ERASURE = 3,        //纠删码
	};
	
	uint64_t flags;              //pool的相关的标志，见FLAG_*
	__u8 type;                   //类型
	__u8 size, min_size;         //pool的size和min_size，也就是副本数和至少保证的副本数（一个PG中的OSD个数)。
	__u8 crush_ruleset;          //rule set的编号
	__u8 object_hash;            //将对象名映射为ps的hash函数
	
private:
  __u32 pg_num, pgp_num;         //PG、PGP的数量
  
public:
  map<string,string> properties;  //过时，当前已经不再使用
  string erasure_code_profile;    //EC的配置信息
  
  ...
  uint64_t quota_max_bytes;      //pool最大的存储字节数
  uint64_t quota_max_objects;    //pool最大的object个数
  ....
};
{% endhighlight %}

* type: 定义了Pool的类型，目前有replication和ErasureCode两种类型；

* size和min_size定义了Pool的冗余模式

&emsp; - 如果是replication模式，size定义了副本数目，min_size为副本的最小数目。例如：如果size设置为3，副本数为3，min_size设置为1就只允许两副本损坏。

&emsp; - 如果是Erasure Code(M+N)，size是总的分片数M+N； min_size是实际数据的分片数M。

* crush_ruleset: Pool对应的crush规则号

* erasure_code_profile: EC的配置方式

* object_hash: 通过对象名映射到PG的hash函数

* pg_num: Pool里的PG的数量

通过上面的介绍可以了解到，Pool根据类型不同，定义了两种模式，分别保存了两种模式相关的参数。此外，在结构pg_pool_t里还定义了Pool级别的快照相关的数据结构、Cache Tier相关的数据结构，以及其他一些统计信息。在介绍快照(参见第9章）和Cache Tier（参见第13章）时再详细介绍相关的字段。

### 2.2 PG
PG可以认为是一组对象的集合，该集合里的对象有共同的特征： 副本都分布在相同的OSD列表中。PG的数据结构如下（src/osd/osd_types.h)：
{% highlight string %}
struct pg_t {
  uint64_t m_pool;              //pg所在的pool
  uint32_t m_seed;              //pg的序号
  int32_t m_preferred;          //pg优先选择的主OSD
};
{% endhighlight %}

结构体pg_t只是一个PG的静态描述信息。类PG及其子类ReplicatedPG都是和PG相关的处理。
{% highlight string %}
struct spg_t {
  pg_t pgid;
  shard_id_t shard;
};
{% endhighlight %}
数据结构spt_t在pg_t的基础上，增加了一个shard_id字段，代表了该PG所在的OSD在对应的OSD列表中的序号。例如，现在有一个PG 2.f，其映射到的OSD为(2,5,8)，那么在OSD5上该PG对应的shard值就为1。

在Erasure Code模式下，该字段保存了每个分片的序号。该序号在EC的数据encode和decode过程中很关键；对于副本模式，该字段没有意义，都设置为shard_id_t::NO_SHARD值。

###### PG的分裂

当一个pool里的PG数量不够时，系统允许通过增加PG的数量，就会产生PG的分裂，使得一个PG分裂为2的幂次方个PG。PG分裂后，新的PG和其父PG的OSD列表是一致的，其数据的移动也是本地数据的移动，开销比较小。


### 2.3 OSDMap
类OSDMap定义了Ceph整个集群的全局信息。它由Monitor实现管理，并以全量或者增量的方式向整个集群扩散。每一个epoch对应的OSDMap都需要持久化保存在meta下对应对象的omap属性中。

下面介绍OSDMap核心成员，内部类Incremental以增量的形式保存了OSDMap新增的信息，其内部成员和OSDMap类似，这里就不介绍了。代码位于src/osd/OSDMap.h:
{% highlight string %}
class OSDMap{
public:
	class Incremental{
	};

private:
  //系统相关信息
  uuid_d fsid;                   //当前集群的fsid
  epoch_t epoch;                 //当前集群的epoch值
  utime_t created, modified;     //创建、修改的时间戳
  int32_t pool_max;              //最大的pool数量

  uint32_t flags;                //一些标志信息


  //OSD相关的信息
  int num_osd;                  //OSD总数量（并不会持久化保存，请参看calc_num_osds)
  int num_up_osd;               //处于up状态的OSD数量（并不会持久化保存，请参看calc_num_osds)
  int num_in_osd;               //处于in状态的OSD数量（并不会持久化保存，请参看calc_num_osds)

  int32_t max_osd;              //OSD的最大数目
  vector<uint8_t> osd_state;    //OSD的状态
  struct addrs_s {
    vector<ceph::shared_ptr<entity_addr_t> > client_addr;
    vector<ceph::shared_ptr<entity_addr_t> > cluster_addr;
    vector<ceph::shared_ptr<entity_addr_t> > hb_back_addr;
    vector<ceph::shared_ptr<entity_addr_t> > hb_front_addr;
    entity_addr_t blank;
  };
  ceph::shared_ptr<addrs_s> osd_addrs;   //OSD的地址（从上面我们看到，一个OSD可以有多个不同类型的地址
  vector<__u32>   osd_weight;           //OSD权重（16.16固定浮点数，即16bit整数部分，16bit小数部分）
  vector<osd_info_t> osd_info;          //OSD的基本信息
  ceph::shared_ptr< vector<uuid_d> > osd_uuid;   //OSD对应的UUID
  vector<osd_xinfo_t> osd_xinfo;                 //OSD的一些扩展信息


  //PG相关的信息
  ceph::shared_ptr< map<pg_t,vector<int32_t> > > pg_temp  // temp pg mapping (e.g. while we rebuild)
  ceph::shared_ptr< map<pg_t,int32_t > > primary_temp;    // temp primary mapping (e.g. while we rebuild)
  ceph::shared_ptr< vector<__u32> > osd_primary_affinity; // < 16.16 fixed point, 0x10000 = baseline


 //pool相关的信息map<int64_t,pg_pool_t> pools;             //pool的id到类pg_pool_t的映射
  map<int64_t,string> pool_name;                          //pool的id到pool的名字的映射
  map<string,map<string,string> > erasure_code_profiles;  //pool的EC相关的信息
  map<string,int64_t> name_pool;                          //pool的名字到pool的id的映射


  //Crush相关的信息
  ceph::shared_ptr<CrushWrapper> crush;                   //CRUSH算法
 
  ....
};
{% endhighlight %}
通过OSDMap数据成员的了解，可以看到，OSDMap包含了四类信息：首先是集群的信息；其次是pool相关的信息；然后是临时PG相关的信息；最后就是所有OSD的状态信息。


### 2.4 OSDOp
类MOSDOp封装了一些基本操作先关的数据(src/messages/MOSDOp.h)：
{% highlight string %}
class MOSDOp : public Message {

  static const int HEAD_VERSION = 7;
  static const int COMPAT_VERSION = 3;

private:
  uint32_t client_inc;
  __u32 osdmap_epoch;             //OSDMap的epoch值
  __u32 flags;
  utime_t mtime;
  eversion_t reassert_version;
  int32_t retry_attempt;          // 0 is first attempt.  -1 if we don't know.

  object_t oid;                   //操作的对象
  object_locator_t oloc;          //对象的位置信息
  pg_t pgid;                      //对象所在的PG的id
  bufferlist::iterator p;
 

  atomic<bool> partial_decode_needed;
  atomic<bool> final_decode_needed;
  //
public:
  vector<OSDOp> ops;              //针对oid的多个操作集合


private:
  //快照相关
  snapid_t snapid;                //snapid，如果是CEPH_NOSNAP，就是head对象；否则就是等于snap_seq
  snapid_t snap_seq;              //如果是head对象，就是最新的快照序号；如果是snap对象，就是snap对应的seq
  vector<snapid_t> snaps;         //所有的snap列表

  uint64_t features;              //一些feature的标志

  osd_reqid_t reqid;             // reqid explicitly set by sender

public:
  friend class MOSDOpReply;
};
{% endhighlight %}
MOSDOp在其成员ops向量里封装了多个类型为OSDOp操作数据。MOSDOp封装的操作都是关于对象oid相关的操作，一个MOSDOp只封装针对同一个对象oid的操作。但是对于rados_clone_range这样的操作，需要有一个目标对象oid，还有一个源对象oid，那么源对象的oid就保存在结构OSDOp里。


数据结构OSDOp封装了一个OSD操作需要的数据和元数据(src/osd/osd_types.h)：
{% highlight string %}
struct OSDOp {
  ceph_osd_op op;                 //具体操作数据的封装
  sobject_t soid;                 //src oid，并不是op操作的对象，而是源操作对象。例如rados_clone_range需要目标obj和源obj

  bufferlist indata, outdata;     //操作的输入输出的data
  int32_t rval;                   //操作返回值

  OSDOp() : rval(0) {
    memset(&op, 0, sizeof(ceph_osd_op));
  }
};
{% endhighlight %}

### 2.5 object_info_t
结构object_info_t保存了一个对象的元数据信息和访问信息(src/osd/osd_types.h)。其作为对象的一个属性，持久化保存在对象xattr中，对应的key为OI_ATTR(" _")，value就是object_info_t的encode后的数据。
{% highlight string %}
struct object_info_t {
  hobject_t soid;                      //对应的对象
  eversion_t version, prior_version;   //对象的当前版本，前一个版本
  version_t user_version;              //用户操作的版本
  osd_reqid_t last_reqid;              //最后请求的reqid

  uint64_t size;                       //对象的大小
  utime_t mtime;                       //修改时间
  utime_t local_mtime;                 //修改的本地时间

  // note: these are currently encoded into a total 16 bits; see
  // encode()/decode() for the weirdness.
  typedef enum {
    FLAG_LOST     = 1<<0,
    FLAG_WHITEOUT = 1<<1,       // object logically does not exist
    FLAG_DIRTY    = 1<<2,       // object has been modified since last flushed or undirtied
    FLAG_OMAP     = 1 << 3,     // has (or may have) some/any omap data
    FLAG_DATA_DIGEST = 1 << 4,  // has data crc
    FLAG_OMAP_DIGEST = 1 << 5,  // has omap crc
    FLAG_CACHE_PIN = 1 << 6,    // pin the object in cache tier
    // ...
    FLAG_USES_TMAP = 1<<8,       // deprecated; no longer used.
  } flag_t;

  flag_t flags;                  //对象的一些标记

  .... 
  
  vector<snapid_t> snaps;                //clone对象的快照信息

  uint64_t truncate_seq, truncate_size;  //truncate操作的序号和size

  //watchers记录了客户端监控信息，一旦对象的状态发生变化，需要通知客户端
  map<pair<uint64_t, entity_name_t>, watch_info_t> watchers;   

  // opportunistic checksums; may or may not be present
  __u32 data_digest;                    //data crc32c
  __u32 omap_digest;                    //omap crc32c
};
{% endhighlight %}


### 2.6 ObjectState
对象ObjectState是在object_info_t的基础上添加了一个字段exists，用来标记对象是否存在(src/osd/osd_types.h)：
{% highlight string %}
struct ObjectState {
  object_info_t oi;      //对象元数据信息
  bool exists;           // the stored object exists (i.e., we will remember the object_info_t)

  ObjectState() : exists(false) {}

  ObjectState(const object_info_t &oi_, bool exists_)
    : oi(oi_), exists(exists_) {}
};
{% endhighlight %}
为什么要加一个额外的bool变量来标记呢？因为object_info_t可能是从缓存的attrs[OI_ATTR]中获取的，并不能确定对象是否存在。

### 2.7 SnapSetContext
SnapSetContext保存了快照的相关信息，即SnapSet的上下文信息。关于SnapSet的内容，可以参考快照相关的介绍()：
{% highlight string %}
struct SnapSetContext {
  hobject_t oid;               //对象
  SnapSet snapset;             //SnapSet, 对象快照相关的记录
  int ref;                     //本结构的引用计数
  bool registered : 1;         //是否在SnapSet Cache中记录
  bool exists : 1;             //snapset是否存在

  explicit SnapSetContext(const hobject_t& o) :
    oid(o), ref(0), registered(false), exists(true) { }
};
{% endhighlight %}

### 2.8 ObjectContext
ObjectContext可以说是对象在内存中的一个管理类，保存了一个对象的上下文信息：
{% highlight string %}
struct ObjectContext {
  ObjectState obs;           //主要是object_info_t，描述了对象的状态信息

  SnapSetContext *ssc;       //快照上下文信息，如果没有快照就为NULL

  Context *destructor_callback;   //析构时的回调

private:
  Mutex lock;
public:
  Cond cond;

  //正在写操作的数目，正在读操作的数目
  //等待写操作的数目，等待读操作的数目
  int unstable_writes, readers, writers_waiting, readers_waiting;


  //如果该对象的写操作被阻塞去恢复另一个对象，设置这个属性
  ObjectContextRef blocked_by;      //本对象被某一个对象阻塞
  set<ObjectContextRef> blocking;   //本对象阻塞的对象集合


  //任何在obs.io.watchers的watchers，其要么在watchers队列中，要么在unconnected_watchers队列中
  map<pair<uint64_t, entity_name_t>, WatchRef> watchers;

  // attr cache
  map<string, bufferlist> attr_cache;      //属性的缓存


  struct RWState {
    enum State {
      RWNONE,
      RWREAD,
      RWWRITE,
      RWEXCL,
    };

    list<OpRequestRef> waiters;           //等待状态变化的waiters
    int count;                            //读或写的数目

    State state:4;                        //读写的状态
    
    bool recovery_read_marker:1;          //如果设置，获得锁后，重新执行backfill操作

    bool snaptrimmer_write_marker:1;      //如果设置，获得锁后重新加入snaptrim队列中
  }

  .....
};
{% endhighlight %}
下面两个字段比较难理解，进行一些补充说明：

* blocked_by记录了当前对象被其他对象阻塞，blocking记录了本对象阻塞其他对象的情况。当一个对象的写操作依赖其他对象时，就会出现这些情况。这一般对应一个操作涉及多个对象，比如copy操作。吧对象obj1上的部分数据拷贝到对象obj2，如果源对象obj1处于missing状态，需要恢复，那么obj2对象就block了obj1对象。

* 内部类RWState通过定义了4种状态，实现了对对象的读写加锁。

### 2.9 Session
类Session是和Connection相关的一个类，用于保存Connection上下文相关的信息(src/osd/osd.h)：
{% highlight string %}
struct Session : public RefCountedObject {
	EntityName entity_name;              //peer实例的名称
	OSDCap caps;                         //
	int64_t auid;
	ConnectionRef con;                   //相关的Connection
	WatchConState wstate;

	Mutex session_dispatch_lock;
	list<OpRequestRef> waiting_on_map;   //所有的OpRequest请求都先添加到这个队列里

	OSDMapRef osdmap;                    //waiting_for_pg当前所对应的OSDMap
	map<spg_t, list<OpRequestRef> > waiting_for_pg;   //当前需要更新OSDMap的pg和对应的请求

	Spinlock sent_epoch_lock;
	epoch_t last_sent_epoch;
	Spinlock received_map_lock;
	epoch_t received_map_epoch;          //largest epoch seen in MOSDMap from here

explicit Session(CephContext *cct) :
  RefCountedObject(cct),
  auid(-1), con(0),
  session_dispatch_lock("Session::session_dispatch_lock"), 
  last_sent_epoch(0), received_map_epoch(0)
{}
}；
{% endhighlight %}
函数update_waiting_for_pg()用于检查是否有最新的osdmap:

1) 如果该PG有分裂的PG，就把分裂出的新的PG以及对应的OpRequest加入到Session的waiting_for_pg队列里；

2） 如果该PG不分裂，就不把PG和OpRequest加入到waiting_for_pg队列里。






<br />
<br />

**[参看]**





<br />
<br />
<br />

