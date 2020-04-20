---
layout: post
title: ceph本地对象存储(4)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

在前面我们介绍完本地对象存储的概念，对外接口和日志实现后，本章我们介绍一下FileStore的实现。

<!-- more -->

###### FileStore的实现
首先来看一下FileStore的静态类图，如下图所示：

![ceph-chapter7-4](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_4.jpg)

FileStore的静态类图说明如下：

* ObjectStore: 是对象存储的接口。FileStore继承了JournalingObjectStore类

* FileStoreBackend： 定义了本地文件系统支持FileStore的一些接口，不同的文件系统会增加自己支持特性，从而实现了不同的FileStoreBackend。例如,BtrfsFileStoreBackend对应BTRFS的本地文件系统实现； XfsFileStoreBackend对应XFS的本地文件系统实现； ZFSFileStoreBackend对应ZFS的本地文件系统实现。（注： 我们当前生产环境采用XFS文件系统）

* Journal类定义了日志的抽象接口，FileJournal实现了以本地文件或者本地设备为存储的日志系统

## 1. 日志的三种类型

在FileStore里，根据日志提交方式的不同，有三种类型的日志：

* journal writeahead: 日志数据先提交并写入日志磁盘上，然后再完成日志的应用（更新实际对象数据）。这种方式适合XFS、EXT4等不支持快照的本地文件系统使用这种方式。

* journal parallel: 日志提交到日志磁盘上和日志应用到实际对象中并行进行，没有先后顺序。这种方式适用于BTRFS和ZFS等实现了快照操作的文件系统。由于具有文件系统级别快照功能，当日志应用过程出错，导致数据不一致的情况下，文件系统只需要回滚到上一次快照，并replay从上次快照开始的日志就可以了。显然，这种方式比writeahead方式性能更高。但是由于btrfs和zfs目前在Linux都不稳定，这种方式很少用。

* 不使用日志的情况，数据直接写入磁盘后才返回客户端应答。这种方式目前FileStore也实现了，但是性能太差，一般不使用。


后面我们只介绍FileStore默认的方式： journal writeahead方式的日志实现。

## 2. JournalingObjectStore
类JournalingObjectStore实现了ObjectStore和FileJournal的一些交互管理。它通过类SubmitManager和ApplyManagerApplyManager分别实现了日志提交和日志应用的管理。下面分别介绍这两个类。

###### 2.1 SubmitManager

类SubmitManager实现了日志提交的管理，准确地说是日志序号的管理。函数op_submit_start给提交的日志分配一个序号。函数op_submit_finish()在日志提交完成后验证： 当前的op的seq等于上次提交的序号op_submitted加1。最后更新op_submitted的值。
{% highlight string %}
class SubmitManager {
    Mutex lock;
    uint64_t op_seq;                 //日志最新的序号
    uint64_t op_submitted;           //日志上次提交的序号
};
{% endhighlight %}

###### 2.2 ApplyManager
类ApplyManager负责日志应用的相关处理：
{% highlight string %}
class ApplyManager {
    Journal *&journal;
    Finisher &finisher;

    Mutex apply_lock;
    bool blocked;                                       //日志应用是否阻塞。当在进行日志同步(sync数据到磁盘)时，我们一般需要阻塞新的日志apply
    Cond blocked_cond;
    int open_ops;                                       //正在进行日志apply的ops的数量
    uint64_t max_applied_seq;                           //日志应用完成的最大的seq

    Mutex com_lock;

    /*
     * 日志完成后的回调函数，用于没有日志的类型
     *  typedef uint64_t version_t;    (src/include/types.h)
     */
    map<version_t, vector<Context*> > commit_waiters;  

    /*
     * committing_seq: 当前正在进行的同步(sync)日志的最大seq
     * committed_seq: 已经已经完成的同步(sync)日志的最大seq
     */  
    uint64_t committing_seq, committed_seq;
};
{% endhighlight %}

函数ApplyManager::op_apply_start()在日志应用前调用，其实现如下操作：

1） 给apply_lock加锁；

2） 当blocked设置时，就等待，暂停日志应用，这个后面日志同步时会说明

3) 统计值open_ops加1

>注： on_apply_finish只是表明将日志应用到了实际对象中，但是我们知道Linux操作系统层面具有cache，实际的对象数据可能仍在cache中，并没有真正落盘。落了盘的数据才算真正的成功

函数ApplyManager::op_apply_finish()在日志应用完成后调用，其实现如下操作：

1）加apply_lock锁

2) 统计值open_ops自减

3) 如果blocked为true，就调用blocked_cond.Signal()发通知。（说明： 因为我们在进行日志同步前会将blocked设置为true，并且等待所有正在apply的日志完成，因此这里我们可能需要唤醒在commit_start()中的等待，以真正开始进行后续的日志同步）

4) 更新max_applied_seq的值为应用完成的最大日志号

函数ApplyManager::commit_start()用于在日志同步时计算目前完成的最大的日志seq。例如，我们当前apply完成的日志的最大seq为100，而当前我们committed_seq的值为90，那么本次我们需要同步的日志seq范围是[91, 100]，此时真正开始进行日志同步(sync)；而若我们当前的committed_seq值也为100，那么说明日志都已经同步完成，并不需要真正开始同步。

函数ApplyManager::commit_started()表明当前同步已经开始，将blocked置为false，这样可以通知此刻也可以进行日志的apply操作了。

函数ApplyManager::commit_finish()用户在日志同步结束时调用。

下面我们给出一张图来简单说明这个过程：

![ceph-chapter7-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_5.jpg)


## 3. FileStore的更新操作
先介绍数据结构。结构Op代表了一个ObjectStore的操作的上下文信息在FileStore里的封装：
{% highlight string %}
struct Op {
	utime_t start;                              //日志应用的开始时间
	uint64_t op;                                //日志的seq
	vector<Transaction> tls;                    //事务列表
	Context *onreadable, *onreadable_sync;      //事务应用完成之后的异步回调和同步回调函数

	/*
	 * 操作数目和字节数。这里的ops指的是对象的基本操作例如create,write,delete等基本操作，一个
	 * Op带多个Transaction，可能带多个基本操作
	 */
	uint64_t ops, bytes;   

                     
	TrackedOpRef osd_op;
};
{% endhighlight %}
类OpSequencer用于实现请求的顺序执行。在同一个Sequencer类的请求，保证执行的顺序，包括日志commit的顺序和apply的顺序。一般情况下，一个PG对应一个Sequencer类。所以一个PG里的操作都是顺序执行。
{% highlight string %}
class OpSequencer : public Sequencer_impl {
	Mutex qlock;                       // to protect q, for benefit of flush (peek/dequeue also protected by lock)
	list<Op*> q;                       //操作序列
	list<uint64_t> jq;                 //日志序号
	list<pair<uint64_t, Context*> > flush_commit_waiters;
	Cond cond;

public:
	Sequencer *parent;                 //这里通过parent可以构成一个树结构
	Mutex apply_lock;                  //日志应用的互斥锁
	int id;
};
{% endhighlight %}

### 3.1 更新实现
FileStore的更新操作分两步： 首先把操作封装成事务，以事务的形式整体提交日志并持久化到日志磁盘，然后再完成日志的应用，也就是修改实际对象的数据。
{% highlight string %}
int FileStore::queue_transactions(Sequencer *posr, vector<Transaction>& tls,
				  TrackedOpRef osd_op,
				  ThreadPool::TPHandle *handle);
{% endhighlight %}
FileStore更新的入口函数为queue_transactions函数。参数posr用于确保执行的顺序；参数tls是Transaction的列表，一次可以提交多个事务；TrackedOpRef用于跟踪信息。

具体处理流程如下：

1） 首先调用collect_contexts函数把tls中所有事务的on_applied类型的回调函数收集在一个list<Context *>结构里，然后调用list_to_context函数把Context列表变成了一个Context类。这里实际就是包装成一个回调函数。on_commit和on_applied_sync都做了类似的工作。
{% highlight string %}
static void collect_contexts(
	vector<Transaction>& t,
	Context **out_on_applied,
	Context **out_on_commit,
	Context **out_on_applied_sync) {

	assert(out_on_applied);
	assert(out_on_commit);
	assert(out_on_applied_sync);

	list<Context *> on_applied, on_commit, on_applied_sync;

	for (vector<Transaction>::iterator i = t.begin();i != t.end();++i) {

		on_applied.splice(on_applied.end(), (*i).on_applied);
		on_commit.splice(on_commit.end(), (*i).on_commit);
		on_applied_sync.splice(on_applied_sync.end(), (*i).on_applied_sync);

	}

	*out_on_applied = C_Contexts::list_to_context(on_applied);
	*out_on_commit = C_Contexts::list_to_context(on_commit);
	*out_on_applied_sync = C_Contexts::list_to_context(on_applied_sync);

}
{% endhighlight %}

2） 构建op对象。把相关的操作封装到op对象里
{% highlight string %}
Op *o = build_op(tls, onreadable, onreadable_sync, osd_op);
{% endhighlight %}
即把众多事务封装成一个Op对象。

3） 调用函数prepare_entry把所有日志的数据封装到tbl里
{% highlight string %}
int orig_len = journal->prepare_entry(o->tls, &tbl);
{% endhighlight %}
打包后如下图所示：

![ceph-chapter7-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_6.jpg)


4） 分配一个日志的序号，并设置在op结构里
{% highlight string %}
uint64_t op_num = submit_manager.op_submit_start();
o->op = op_num;
{% endhighlight %}

5) 如果日志是m_filestore_journal_parallel模式，就调用函数_op_journal_transactions()开始提交日志。然后调用queue_op，它把请求添加到op_wq里同时开始日志的应用。

6） 如果日志是m_filestore_journal_writeahead模式，就把该op添加到osr中，调用_op_journal_transactions函数提交日志。
{% highlight string %}
osr->queue_journal(o->op);

_op_journal_transactions(tbl, orig_len, o->op,
	       new C_JournaledAhead(this, osr, o, ondisk),
	       osd_op);
{% endhighlight %}

7) 调用函数submit_manager.op_submit_finish(op_num)，通知submit_manager日志提交完成。

### 3.2 writeahead和parallel的处理方式的不同
函数_op_journal_transactions()用于提交日志。当有日志并且日志可写时，就调用journal的submit_entry函数提交日志。当日志提交成功后，就会调用_op_journal_transactions注册的回调函数onjournal。

函数queue_op用于把操作添加到FileStore内部的```线程池```对应的队列OpWq中。线程池中的线程就会调用OpWq::_process()来完成日志的应用，也就是完成实际的操作。

1) 对应日志parallel方式，日志的提交和应用是并发进行的：
{% highlight string %}
 _op_journal_transactions(tbl, orig_len, o->op, ondisk, osd_op);

// queue inside submit_manager op submission lock
queue_op(osr, o);
{% endhighlight %}

2) 对于writeahead方式，先调用函数_op_journal_transactions()提交日志，其注册的回调函数onjournal为C_JournaledAhead:
{% highlight string %}
osr->queue_journal(o->op);

_op_journal_transactions(tbl, orig_len, o->op,
	       new C_JournaledAhead(this, osr, o, ondisk),
	       osd_op);
{% endhighlight %}
在回调函数C_JournaledAhead类里，其finish函数调用_journaled_ahead()，该函数调用queue_op(osr,o)把相应请求添加到op_wq.queue(osr)工作队列中：
{% highlight string %}
void FileStore::_journaled_ahead(OpSequencer *osr, Op *o, Context *ondisk)
{
  dout(5) << "_journaled_ahead " << o << " seq " << o->op << " " << *osr << " " << o->tls << dendl;

  // this should queue in order because the journal does it's completions in order.
  queue_op(osr, o);

  list<Context*> to_queue;
  osr->dequeue_journal(&to_queue);

  // do ondisk completions async, to prevent any onreadable_sync completions
  // getting blocked behind an ondisk completion.
  if (ondisk) {
    dout(10) << " queueing ondisk " << ondisk << dendl;
    ondisk_finishers[osr->id % m_ondisk_finisher_num]->queue(ondisk);
  }
  if (!to_queue.empty()) {
    ondisk_finishers[osr->id % m_ondisk_finisher_num]->queue(to_queue);
  }
}
{% endhighlight %}
工作队列的处理线程实现了日志apply操作，完成实际对象数据的修改。

从上可以看出，writeahead是先完成了日志的提交，然后才开始日志的应用。Parallel方式是同时完成日志的提交和应用。

3) 如果日志是第三种类型，即没有日志的形式，就调用函数apply_manager.add_waiter()把context添加到commit_waiter中，然后在数据应用完成后就会完成on_commit回调。

>注： 事实上，还有第4)种情况，此种方式不采用线程池来完成日志的应用，而是直接调用do_transactions()来向磁盘写对象数据。前面的1)、2）、3）都是会将事务提交到op_wq队列里，
>然后通过队列绑定的线程池来完成日志的应用。

## 4. 日志的应用

操作队列OpWq用于完成日志的应用。处理函数_process调用_do_op函数来完成应用日志，实现真正的修改操作。

函数_do_op()实现操作如下：

1) 首先调用osr->apply_lock.Lock()加锁，通过该锁，实现了同一时刻，osr里只有一个操作在进行；

2) 调用op_apply_start，通知apply_manager类开始日志的应用

3） 调用_do_transactions函数，用于完成日志的应用。它对每一个事务调用_do_transaction函数，解析事务，执行相应的操作。

4) 调用op_apply_finish(o->op)通知apply_manager完成。


## 5. 日志的同步

在FileStore内部会创建一个```sync_thread```，用来定期同步日志，该线程的入口函数为：
{% highlight string %}
void FileStore::sync_entry();
{% endhighlight %}

函数sync_entry定期执行同步操作，处理过程如下：

1） 调用函数op_tp.pause()，来暂停FileStore的op_wq的线程池，等待正在应用的日志完成；

2） 然后调用fsync同步内存中的数据到数据盘，当同步完成后，就可以丢弃相应的日志，释放相应的日志空间
{% highlight string %}
apply_manager.commit_finish();
{% endhighlight %}



<pre>
</pre>



<br />
<br />

**[参看]**




<br />
<br />
<br />

