---
layout: post
title: ceph本地对象存储(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本地对象存储模块完成了数据如何原子地写入磁盘，这就涉及事务和日志的概念。对象如何在本地文件系统中组织的代码实现在src/os中。本章将介绍在单个OSD上数据如何写入磁盘中。

目前有4种本地对象存储实现：

* FileStore: 这是目前比较稳定，生产环境上使用的主流对象存储引擎，也是本章重点介绍的对象存储引擎

* BlueStore: 这是目前社区在实现的一个新版本，社区丢弃了本地文件系统，自己写了一个简单的，专门支持RADOS用户态的文件系统

* KStore： 这是以本地KV存储系统实现的对象存储，它基于RADOS的框架用来实现一个分布式的KV存储系统

* Memstore: 它把数据和元数据都保存在内存中，用来测试和验证使用

KStore和Memstore两种存储引擎比较简单，这里就不介绍了（会在后面适当的章节进行详细分析）。BlueStore社区还正在开发之中，这里也暂时不介绍。本章将详细介绍目前在生产环境中使用的FileStore存储的实现。

在了解ceph filestore本地对象对象存储的具体实现之前，建议先参看本章```附录```相关内容，以对object的attr及omap操作有一个直观上的认识。

我们可以通过ceph_test_objectstore来对本地对象存储有一个更深的理解，执行如下命令进行编译：
<pre>
# cd src
# make ceph_test_objectstore
</pre>

<!-- more -->

## 1. 基本概念介绍
RADOS本地对象存储系统(也称为对象存储引擎）基于本地文件系统实现，目前默认的文件系统为XFS。一个对象包含```数据```和```元数据```两种数据。对应本地文件系统里，一个对象就是一个固定大小（默认4MB)的文件，其元数据保存在文件的扩展属性或者本地独立的KV存储系统中。

### 1.1 对象的元数据
对象的元数据就是用于对象描述信息的数据，它以简单的key-value(键值对）形式存在，在RADOS存储系统中有两种实现：xattrs和omap:

* xattrs保存在对象对应文件的扩展属性中，这要求支持对象存储的本地文件系统支持扩展属性。

* omap就是object map的简称，是一些键值对，保存在本地文件系统之外的独立的key-value存储系统中，例如leveldb、rockdb等。

有些文件系统可能不支持扩展属性，有些虽然也支持扩展属性但对key或者value占用空间的大小有限制，或者扩展属性占的总的空间大小有限制。对于leveldb等本地键值存储系统基本没有这样的限制，但是它的写性能优于读性能。所以一般情况下，xattrs保存一些比较小而经常访问的信息。omap保存一些大而不是经常访问的数据。


### 1.2 事务和日志的基本概念
假设磁盘正在执行一个操作，此时由于发生磁盘错误，或者系统宕机，或者断电等其他原因，导致只有部分数据写入成功。这种情况就会出现磁盘上的数据有一部分是旧数据，部分是新写入的数据，使得磁盘数据不一致。

当一个操作要么全部成功，要么全部失败，不允许只有部分操作成功，就称这个操作具有原子性。引入事务和日志，就是为了实现操作的原子性，解决数据的不一致性问题。

引入日志后，数据写入变为两步： 1） 先把要写入的数据全部封装成一个事务，其整体作为一条日志，先写入日志文件并持久化到磁盘，这个过程称为日志提交(journal submit)。2）然后再把数据写入对象文件中，这称为日志的应用（journal apply)。

当系统在日志提交的过程中出错，系统重启后，直接丢弃不完整的日志条目即可，该条日志对应的实际对象数据并没有修改，数据可以保持一致。当在日志应用的过程中出错，由于数据已经写入并回刷到日志盘中，系统重启后，重放(replay)日志，就可以保证新数据重新完整写入，保证了数据的一致性。

这个机制需要确保所有的更新操作都是幂等操作。所谓幂等操作，就是数据的更新可以多次写入，不会产生任何副作用。对象存储的操作一般都具有幂等性。

在事务的提交过程中，一条日志记录可以对应一个事务。为了提高日志提交的性能，一般都允许多条事务并发提交，一个事务可以对应多条日志，批量提交。所以事务的提交过程，一般和日志的提交过程是一个概念。

日志有三个处理阶段，对应过程分别为：

* 日志提交(journal submit): 日志写入日志磁盘

* 日志的应用(journal apply)： 日志对应的修改更新到对象的磁盘文件中。这个修改不一定写入磁盘，可能缓存在本地文件系统的页缓存(page cache)中

* 日志的同步(Journal sync或者journal commit): 当确定日志对应的修改操作已经刷回到磁盘中，就可以把相应的日志记录删除，释放所占用的日志空间。


### 1.3 事务的封装
ObjectStore的内部类Transaction用来实现相关的事务。它有两种封装形式，一种是```use_tbl```(transaction bufferlist)，事务把操作的元数据和数据都封装在bufferlist类型的tbl中：
{% highlight string %}
bool use_tbl {false};   //use_tbl for encode/decode
bufferlist tbl;
{% endhighlight %}

另一种是不使用tbl，把元数据操作以struct Op的结构体，封装在op_bl中，把操作相关的数据封装在data_bl中：
{% highlight string %}
struct Op {
      __le32 op;
      __le32 cid;
      __le32 oid;
      __le64 off;
      __le64 len;
      __le32 dest_cid;
      __le32 dest_oid;                  //OP_CLONE, OP_CLONERANGE
      __le64 dest_off;                  //OP_CLONERANGE
      __le32 hint_type;                 //OP_COLL_HINT
      __le64 expected_object_size;      //OP_SETALLOCHINT
      __le64 expected_write_size;       //OP_SETALLOCHINT
      __le32 split_bits;                //OP_SPLIT_COLLECTION2
      __le32 split_rem;                 //OP_SPLIT_COLLECTION2
} __attribute__ ((packed)) ;


bufferlist data_bl;
bufferlist op_bl;
bufferptr op_ptr;     //临时操作指针
{% endhighlight %}
由于struct Op里保存的是coll_id和object_id，所以需要保存coll_t到coll_id的映射关系，以及ghobject_t与object_id的映射关系。从这种存储方式就可以看出，这种方式和前一种的区别，是在数据封装上实现了一种压缩。当事务中多个操作有相同的对象时，只保存一次ghobject_t结构体，其他情况只保存index来索引。
{% highlight string %}
map<coll_t, __le32> coll_index;                                        //coll_t -> coll_id的映射关系
map<ghobject_t, __le32, ghobject_t::BitwiseComparator> object_index;   //ghobject_t -> object_id的映射关系

__le32 coll_id {0};                                                   //当前分配的coll_id的最大值
__le32 object_id {0};                                                 //当前分配的object_id的最大值
{% endhighlight %}

数据结构TransactionData记录了一个事务中有关操作的统计信息：
{% highlight string %}
struct TransactionData {
      __le64 ops;                          //本事务中操作数目
      __le32 largest_data_len;             //最大的数据长度
      __le32 largest_data_off;             //在对象中的偏移
      __le32 largest_data_off_in_tbl;      //在tbl中的偏移
      __le32 fadvise_flags;                //一些标志
};
{% endhighlight %}
比如我们对一个Transaction t进行了两次write操作，那么t.data.ops的值就是2，后面我们在构造FileStore::Op时可能就会需要用到该值。

一个事务中，对应如下三类回调函数，分别在事务不同的处理阶段调用。当事务完成相应阶段工作后，就调用相应的回调函数来通知事件完成。注意每一类都可以注册多个回调函数：
{% highlight string %}
list<Context *> on_applied;
list<Context *> on_commit;
list<Context *> on_applied_sync;
{% endhighlight %}
on_commit是事务提交完成之后调用的回调函数；on_applied_sync和on_applied都是事务应用完成之后的回调函数。前者是被同步调用执行，后者是在Finisher线程里异步调用执行。

注： on_commit是在事务提交完成之后会通过Finisher线程进行异步回调。这里对```事务提交完成```进行一下说明，对于journal writeahead模式，由于首先是进行写日志操作，然后才是应用日志操作，那么此种情况下，写日志完成即认为```事务提交完成```；而对于journal parallel模式，由于写日志与应用日志是并行进行的，因此这两个操作任意一个完成即认为```事务提交完成```。




## 2. ObjectStore对象存储接口

下面我们先对ObjectStore中的一些概念及术语做一个简单的介绍：

**1) ObjectStore中Object的内容及语义**

ObjectStore所存储的每一个object都是由处于collection(coll_t)中ghobject_t或hobject_t所标识（注： 这里我们可以将collection理解为一个PG，一个object对应一个PG，但是一个PG可以对应多个object，一个coll_t中的objects可以认为都属于同一个PG）。ObjectStore支持在一个collection内创建(creation)、修改(mutation)、删除(deletion)、枚举(enumeration)对象的操作。这里枚举对象时，是按hash后的对象名的字典序进行输出的。object名称在全局是唯一的。

每一个object都有4个不同的部分： byte data、xattrs、omap_header以及omap entries。

* data： 从概念上来讲，一个object的data部分就相当于文件系统上的一个文件。能够支持对data数据的随机读写操作。对一个object的data部分的轻量级的实现有助于降低系统的工作负载。在系统层面上，我们通常会限制每个对象的最大大小为100MB。

* xattrs: 等价于文件系统的扩展属性。Xattrs是一系列的键值对，支持对xattrs进行枚举操作。在实现层面，xattr并不属于ceph，而是依赖于所采用的文件系统，因此所支持的xattrs通常小于64KB。通常来说，ceph假设在访问对象时一并访问该对象相应的xattr是很方便的，并不会付出过高的代价。

* omap_header: 是一个单独的数据块，可以作为一个整体来进行读写操作

* omap entries: 从概念上来讲，omap entries类似于xattrs，但保存于不同的位置。换句话说，在xattrs中所保存的key/value，在omap entries中可以有相同的key/value。在枚举xattrs时，并不会包括omap entries，反之亦然。通常xattrs最多只能有64KB，而omap entries大小可以达到MB级别，另外两者之间的访问方式也有很大的不同。ceph必须要提供高效

2） **Collections**

一个collection就是一个简单的对象分组。collections具有名称(coll_t)，并且可以被枚举。与一个单独的object类似，一个collection也有一系列的xattrs。

3） **transaction**

一个Transaction代表一系列的原子修改操作。在transaction的生命周期中会产生3个事件回调，每个回调可以包含任意数量的callback上下文(Context)。3个事件回调分别为：

* on_applied_sync

* on_applied

* on_commit

当Transaction所对应的修改请求对ObjectStore的其他子操作已经可见(比如数据已经可读)，就会回调on_applied_sync或on_applied。on_applied_sync与on_applied之间的唯一区别就是回调线程、调用环境的不同。对于on_applied_sync是直接由ObjectStore执行线程所回调，通常回调可以快速完成，且在回调时并不需要执行获取environment锁的操作；相反，对于on_applied则会通过一个单独的Finisher线程来完成，这就意味着其可能需要竞争environment锁。

>注： on_applied有时候也会被称为on_readable，on_applied_sync有时候会被称为on_readable_sync

on_commit回调也是通过Finisher线程来进行，表明所有的修改操作都已经被提交，并完成了持久化操作。

在实现层面，每一个修改原语(也包括其所关联的数据）都可以被序列化到一个单独的buffer中。在序列化时，并不会拷贝任何的数据，而是通过bufferlist直接引用原始的缓冲块。这就意味着在提交过程中，必须保持整个buffer数据的稳定，直到on_commit回调完成。实际上，bufferlist会帮你完成这个过程，只有在你通过buffer::raw_static来引用一块已存在的内存时才需要自己注意在提交期间保证buffer的数据不会被修改。

有一些ObjectStore的实现会选择根据Transaction的序列化格式来实现自己的日志(journaling)形式。这就要求相应的encode/decode逻辑能够正确的识别version信息，并且在Transaction的编码格式改变时也能够正确的进行升级。这种情况其实已经发生了，Transaction object也包含一些辅助的变量来协助进行解码：
<pre>
对于bobtail版本之前的ceph， 可通过sobject_encoding来识别older/simpler类型的oid； 可以通过use_pool_override来侦测pool oid被覆盖的场景。
</pre>

4) **事务的隔离**

除非另行说明，否则事务的隔离都是由调用者来负责的。换句话说，假如任何存储元素(即上文说的object的4个部分)在事务中被修改，那么在事务处于pending期间(从一个事务产生，到收到on_applied_sync回调这一时间段都处于pending状态)，调用者都不应该尝试去读取该元素。ObjectStore并不会侦测隔离性是否被破坏，也没有相应的机制来报告隔离性遭到了破坏。

如果一个transaction本身包含创建(create)或删除(delete)存储元素的操作，那么执行enumeration操作也有可能会破坏事务的隔离性。在这种情况下如果执行enumeration操作，那么允许ObjectStore自行决定是返回事务元素被破坏前还是破坏后的结果。换句话说，ObjectStore返回的枚举结果集对于是否包含该事务所修改的元素是不确定的。例如，假设一个事务(transaction)含有"Create A"与"delete B"两个修改元素的操作，那么在transaction处于pending期间，如果执行enumeration操作，则ObjectStore可能会返回A/B存在性的4种组合.

下面通过对象存储的接口说明和代码示例，可以了解对象存储的基本功能及如何使用这些功能，从而对对象存储有一个概要了解：
{% highlight string %}
class ObjectStore{

/*
 * 在同一个Sequencer下面的所有transactions都会被顺序的提交执行，而不同Sequencer下面的事务则可能会被并发的提交执行。
 *
 * 每个ObjectStore客户端创建并维护他们自己的Sequencer对象。当有一系列的事务需要提交时，由调用者来选择一个特定的
 * Sequencer。
 */
struct Sequencer_impl : public RefCountedObject {};

/*
 * 获取对象的一个分组信息
 */
struct CollectionImpl : public RefCountedObject{};


/*
 * 事务
 */
class Transaction{

	/*
	 * 记录了一个事务中有关操作的统计信息
	 */
	struct TransactionData {};

	/*
	 * 用于解析Transaction的辅助类
	 */
	class iterator{
		//遍历Transaction中的每一个元素，并进行解码
	};


	//1) 对transaction注册on_applied/on_applied_sync/on_commit等相关操作
	//1) 对transaction中的每一个ghobject_t的相关操作： object attr/object omap/object omap_header/collection attr
   

};


// synchronous wrappers
unsigned apply_transactions(Sequencer *osr, vector<Transaction>& tls, Context *ondisk=0);

int queue_transactions(....);

//1)  mount/umount操作

//2) mkfs/statfs操作

//3) journal操作

//4) metadata操作

//5) 文件的read/exists/stat等操作

//6) attr相关操作

//7) collections相关操作： 本objectstore所管理的所有分组相关信息

//8) omap/omap_header相关操作

//9) get_fsid/set_fsid相关操作： 获得本objectstore的唯一标识相关的操作

};
{% endhighlight %}
由上面可看到，基本上ObjectStore就是对一个文件系统相关功能的一个抽象。ObjectStore可以将Transaction打包进一个队列，然后由相关线程负责相应事务的提交。

下面我们通过对象存储的接口说明和代码示例，可以了解对象存储的基本功能及如何使用这些功能，从而对对象存储有一个概要了解。

### 2.1 对外接口说明
类ObjectStore是对象存储系统抽象操作接口。所有的对象存储引擎都有继承并实现它定义的接口。下面列出一些代表性的函数接口：

* objectstore初始化相关的函数
<pre>
mkfs   创建objectstore相关的系统信息
mount  加载objectstore相关的系统信息
statfs 获取objectstore系统信息
</pre> 

* 获取属性相关信息
<pre>
getattr   获取对象的扩展属性xattr
omap_get  获取对象的omap信息 
</pre>

* queue_transactions是所有ObjectStore更新操作的接口。更新相关的操作(例如创建一个对象，修改属性，写数据等）都是以事务的方式提交给ObjectStore，该函数被重载成各种不同的接口。其参数为：
{% highlight string %}
Sequencer *osr   用于保证在同一个Sequencer的操作是顺序的

vector<Transaction *> &tls   要提交的事务或事务的列表

Context *onreadable 和 Context *onreadable_sync 这两个函数分别对应于事务的on_apply和on_apply_sync。当事务apply完成后，
修改后的数据就可以被读取了。

Context *ondisk   这个回调函数就是事务进行on_commit后的回调函数
{% endhighlight %}

### 2.2 ObjectStore代码示例
下面通过一段ObjectStore的测试代码，来展示ObjectStore的基本功能和接口使用：

1) 首先调用create方法创建一个ObjectStore实例。参数分别为配置选项CephContext，对象存储的类型： filestore，对象存储的目录名，日志文件名。如下
{% highlight string %}
ObjectStore *store_ = ObjectStore::create(g_ceph_context,
                                              string(GetParam()),
                                              string("store_test_temp_dir"),
                                              string("store_test_temp_journal"));
{% endhighlight %}

2) 创建并加载ObjectStore本地对象存储
{% highlight string %}
store_->mkfs();             //此处会创建数据存储目录store_test_temp_dir下面的相关文件，以及journal日志文件store_test_temp_journal
store_->mount()
{% endhighlight %}

3) 创建了一个collection，并写数据到一个对象中。对象的任何写操作，都先调用事务的相关写操作。事务把相关操作的元数据和数据封装，然后调用store的apply_transaction来真正实现数据的更改。
{% highlight string %}
ObjectStore::Sequencer osr("test");
int r;
coll_t cid;
ghobject_t hoid(hobject_t(sobject_t("Object 1", CEPH_NOSNAP)));

ObjectStore::Transaction t;
bufferlist bl;

bl.append("123456");
t.create_collection(cid, 0);             //创建一个collection(pg)，直观上我们看到的就是一个文件夹。并不需要每次都创建，
t.write(cid, hoid, 0, bl.length(), bl);  //写对象数据

r = store->apply_transaction(&osr, std::move(t));   //事务的应用，真正实现数据的写入操作

store->umount();               
r = store->mount();
{% endhighlight %}


## 8. 附录--ceph存储object的attr和omap操作

在ceph中，所有的存储不管是块存储、对象存储、还是文件存储最后都转化成了底层的对象object，这个object包含3个元素data、xattr、omap。data是保存对象的数据。xattr是保存对象的扩展属性，每个对象文件都可以设置文件的属性，这个属性是一个key/value对，但是受到文件系统的限制，key/value对的个数和每个value的大小都进行了限制。如果要设置的对象的key/value不能存储在文件的扩展属性中，还存在另外一种方式保存:omap，omap实际上是保存到了key/value对的数据库levelDB中，在这里value的值限制要比xattr中好的多。

一开始不太明白key/value是做什么的？在ceph中起到什么作用？这里要说明的是这些key/value是保存对象的元数据相关信息，这些元数据相关信息是可以单独创建和设置的，等于对象存储的扩展，支持属性的键值对存储。这个作用就是提供给ceph内部使用，暂时用处不大。

###### 8.1 块存储设备元数据管理
对于块存储，其元数据主要是块的相关信息。我们在创建一个块设备后，会创建一个默认的rbd(rados block device)元数据文件，用其来存放相应的元数据。

1) 创建块设备

首先执行如下命令查看当前已有的存储池：
<pre>
# rados lspools
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email
</pre>
创建pool的语法为：
{% highlight string %}
# ceph osd pool create {pool-name} {pg-num} [{pgp-num}] [replicated] \
     [crush-ruleset-name] [expected-num-objects]
# ceph osd pool create {pool-name} {pg-num}  {pgp-num}   erasure \
     [erasure-code-profile] [crush-ruleset-name] [expected_num_objects]
{% endhighlight %}
如下我们创建名为```rbd-01```的pool（其pg-num为128):
<pre>
# ceph osd pool create rbd-01 128 128
pool 'rbd-01' created
# ceph osd pool set rbd-01 size 3
set pool 42 size to 3
# ceph osd pool set rbd-01 crush_ruleset 5
set pool 42 crush_ruleset to 5

# ceph osd pool ls
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email
rbd-01
</pre>

之后，通过如下命令在rbd-01上面创建一个大小为100GB的块设备：
<pre>
# rbd create rbd-01/rbd-image01 --size=102400
# rbd info rbd-01/rbd-image01
rbd image 'rbd-image01':
        size 102400 MB in 25600 objects
        order 22 (4096 kB objects)
        block_name_prefix: rbd_data.2f7502ae8944a
        format: 2
        features: layering, exclusive-lock, object-map, fast-diff, deep-flatten
        flags: 
# rados -p rbd-01 ls
rbd_id.rbd-image01
rbd_object_map.2f7502ae8944a
rbd_directory
rbd_header.2f7502ae8944a
</pre>

2) 查看块设备存被映射到了哪些OSD上

采用如下的命令查看```rbd-image01```这个块设备映射到了哪些OSD上：
{% highlight string %}
# ceph osd map rbd-01 rbd-image01
osdmap e1262 pool 'rbd-01' (42) object 'rbd-image01' -> pg 42.f11b8ea3 (42.23) -> up ([5,15,23], p5) acting ([5,15,23], p5)
{% endhighlight %}

3) 设置xattr元数据信息
<pre>
# rados -p rbd-01 setxattr rbd-image01 creator ivanzz1001
# rados -p rbd-01 setxattr rbd-image01 description 'just for test'
# rados -p rbd-01 listxattr rbd-image01
creator
description
# rados -p rbd-01 getxattr rbd-image01 creator
ivanzz1001
</pre>

4) 查看rbd-image01对象

通过上面我们发现rbd-image01被存储在pg 42.23内，因此我们可以进入osd.5相应的目录下查看```rbd-image01```的相关信息：
<pre>
# pwd
/var/lib/ceph/osd/ceph-5/current
# cd 42.23_head
# ls
__head_00000023__2a  rbd-image01__head_F11B8EA3__2a
</pre>

获取对象的扩展属性信息：
<pre>
# getfattr ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._
user.ceph._@1
user.ceph._creator
user.ceph._description
user.ceph.snapset
user.cephos.spill_out

# getfattr -n user.ceph._creator ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._creator="ivanzz1001"

# getfattr -n user.ceph._description ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._description="just for test"
</pre>
通过上面我们可以直观的看到刚才我们所设置的扩展属性信息。


###### 8.2 对象存储元数据管理（xattr)

1) 上传文件到benchmark存储池
<pre>
# rados lspools
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email

# pwd
/root/ceph_cluster/test
# cat  hello.txt 
hello,world

# rados -p benchmark put hello.txt /root/ceph_cluster/test/hello.txt
# rados -p benchmark ls | grep hello
hello.txt
</pre>

2) 查看上传的hello.txt对象
{% highlight string %}
# ceph osd map benchmark hello.txt
osdmap e1262 pool 'benchmark' (39) object 'hello.txt' -> pg 39.d92fd82b (39.2b) -> up ([18,12,7], p18) acting ([18,12,7], p18)

# ls
hello.txt__head_D92FD82B__27

# cat hello.txt__head_D92FD82B__27 
hello,world
{% endhighlight %}

3) 设置对象的扩展属性
<pre>
# rados -p benchmark setxattr hello.txt creator ivanzz1001
# rados -p benchmark setxattr hello.txt description 'just for test'
# rados -p benchmark listxattr hello.txt
creator
description
# rados -p benchmark getxattr hello.txt creator
ivanzz1001
</pre>

4) 通过linux相关命令查看设置的扩展属性
<pre>
# getfattr ./hello.txt__head_D92FD82B__27 
# file: hello.txt__head_D92FD82B__27
user.ceph._
user.ceph._@1
user.ceph._creator
user.ceph._description
user.ceph.snapset
user.cephos.spill_out

# getfattr -n user.ceph._creator ./hello.txt__head_D92FD82B__27 
# file: hello.txt__head_D92FD82B__27
user.ceph._creator="ivanzz1001"
</pre>


###### 8.3 对象存储元数据管理(omap)
1） 设置omap

对上面所创建的benchmark这个pool中的```hello.txt```这个对象设置omap:
<pre>
# rados -p benchmark setomapval hello.txt hcreator 'ivanzz1001'
# rados -p benchmark setomapval hello.txt hdescription 'just for test'
# rados -p benchmark listomapkeys hello.txt
hcreator
hdescription
</pre>

2) 查看omap

使用如下命令查看omap值：
<pre>
# rados -p benchmark getomapval hello.txt hcreator
value (10 bytes) :
00000000  69 76 61 6e 7a 7a 31 30  30 31                    |ivanzz1001|
0000000a

# rados -p benchmark getomapval hello.txt hdescription
value (13 bytes) :
00000000  6a 75 73 74 20 66 6f 72  20 74 65 73 74           |just for test|
0000000d
</pre>
目前对于omap具体存放在哪个文件还未知晓，猜测是在对应OSD的```omap```文件夹下。例如：/var/lib/ceph/osd/ceph-7/current/omap




<br />
<br />

**[参看]**


1. [ceph存储 object的attr和omap操作](https://blog.csdn.net/skdkjzz/article/details/51579520)

2. [Ceph Pool操作总结](https://blog.csdn.net/hxpjava1/article/details/80167792)

3. [linux下文件文件夹扩展属性操作](https://blog.csdn.net/liuhong1123/article/details/7247744)

4. [Ceph leveldb及KV存储](/var/lib/ceph/osd/ceph-7/current/omap)

<br />
<br />
<br />

