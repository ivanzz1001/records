---
layout: post
title: ceph peering机制
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph中比较复杂的模块： Peering机制。该过程保障PG内各个副本之间数据的一致性，并实现PG的各种状态的维护和转换。本章首先介绍boost库的statechart状态机基本知识，Ceph使用它来管理PG的状态转换。其次介绍PG的创建过程以及相应的状态机创建和初始化。然后详细介绍peering机制三个具体的实现阶段：GetInfo、GetLog、GetMissing。


<!-- more -->


## 1. statechart状态机
Ceph在处理PG的状态转换时，使用了boost库提供的statechart状态机。因此先简单介绍一下statechart状态机的基本概念和涉及的相关知识，以便更好地理解Peering过程PG的状态机转换流程。下面例举时截取了PG状态机的部分代码。


### 1.1 状态
在statechart里，一个状态的定义方式有两种：

* 没有子状态情况下的状态定义；
{% highlight string %}
//boost
template< class MostDerived,
          class Context,
          class InnerInitial = mpl::list<>,
          history_mode historyMode = has_no_history >
class state : public simple_state<
  MostDerived, Context, InnerInitial, historyMode >
{
};

//ceph
struct Reset : boost::statechart::state< Reset, RecoveryMachine >, NamedState {
};
{% endhighlight %}

这里定义了状态Reset，它需要继承boost::statechart::state类。该类的模板参数中，第一个参数为状态自己的名字Reset，第二个参数为该状态所属状态机的名字，表明Reset是状态机RecoveryMachine的一个状态。

* 有子状态情况下的状态定义
{% highlight string %}
struct Start;

struct Started : boost::statechart::state< Started, RecoveryMachine, Start >, NamedState {
};
{% endhighlight %}

状态```Started```也是状态机RecoveryMachine的一个状态，模板参数中多了一个参数```Start```，它是状态```Started```的默认初始子状态，其定义如下：
{% highlight string %}
struct Start : boost::statechart::state< Start, Started >, NamedState {
};
{% endhighlight %}

这里定义的Start是状态Started的子状态。第一个模板参数是自己的名字，第二个模板参数是该子状态所属父状态的名字。

综上所述，一个状态，要么属于一个状态机，要么属于一个状态，成为该状态的子状态。其定义的模板参数是自己，第二个参数是拥有者，第三个参数是它的起始子状态。

### 1.2 事件
状态能够接收并处理事件。事件可以改变状态，促使状态发生转移。在boost库的statechart状态机中定义事件的方式如下所示：
{% highlight string %}
struct QueryState : boost::statechart::event< QueryState >{
}; 
{% endhighlight %}

QueryState为一个事件，需要继承boost::statechart::event类，模板参数为自己的名字。

### 1.3 状态响应事件
在一个状态内部，需要定义状态机处于当前状态时，可以接受的事件以及如何处理这些事件的方法：
{% highlight string %}
#define TrivialEvent(T) struct T : boost::statechart::event< T > { \
    T() : boost::statechart::event< T >() {}			   \
    void print(std::ostream *out) const {			   \
      *out << #T;						   \
    }								   \
  };
  TrivialEvent(Initialize)
  TrivialEvent(Load)
  TrivialEvent(GotInfo)
  TrivialEvent(NeedUpThru)
  TrivialEvent(CheckRepops)
  TrivialEvent(NullEvt)
  TrivialEvent(FlushedEvt)
  TrivialEvent(Backfilled)
  TrivialEvent(LocalBackfillReserved)
  TrivialEvent(RemoteBackfillReserved)
  TrivialEvent(RemoteReservationRejected)
  TrivialEvent(RequestBackfill)
  TrivialEvent(RequestRecovery)
  TrivialEvent(RecoveryDone)
  TrivialEvent(BackfillTooFull)

  TrivialEvent(AllReplicasRecovered)
  TrivialEvent(DoRecovery)
  TrivialEvent(LocalRecoveryReserved)
  TrivialEvent(RemoteRecoveryReserved)
  TrivialEvent(AllRemotesReserved)
  TrivialEvent(AllBackfillsReserved)
  TrivialEvent(Recovering)
  TrivialEvent(GoClean)

  TrivialEvent(AllReplicasActivated)

  TrivialEvent(IntervalFlush)

struct Initial : boost::statechart::state< Initial, RecoveryMachine >, NamedState {
  explicit Initial(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::transition< Initialize, Reset >,
boost::statechart::custom_reaction< Load >,
boost::statechart::custom_reaction< NullEvt >,
boost::statechart::transition< boost::statechart::event_base, Crashed >
> reactions;

  boost::statechart::result react(const Load&);
  boost::statechart::result react(const MNotifyRec&);
  boost::statechart::result react(const MInfoRec&);
  boost::statechart::result react(const MLogRec&);
  boost::statechart::result react(const boost::statechart::event_base&) {
return discard_event();
  }
};
{% endhighlight %}
上述代码列出了状态RecoveryMachine/Initial可以处理的事件列表和处理对应事件的方法：

1） 通过boost::mpl::list定义该状态可以处理多个事件类型。本例中可以处理Initialize、Load、NullEvt和event_base事件。

2） 简单事件处理
{% highlight string %}
boost::statechart::transition< Initialize, Reset >
{% endhighlight %}
定义了状态Initial接收到事件Initialize后，无条件直接跳转到Reset状态；

3） 用户自定义事件处理： 当接收到事件后，需要根据一些条件来决定状态如何转移，这个逻辑需要用户自己定义实现
{% highlight string %}
boost::statechart::custom_reaction< Load >
{% endhighlight %}

custom_reaction定义了一个用户自定义的事件处理方法，必须有一个react()的处理函数处理对应该事件。状态转移的逻辑需要用户自己在react函数里实现：
{% highlight string %}
boost::statechart::result react(const Load&);
{% endhighlight %}

4) NullEvt事件用户自定义处理，但是没有实现react()函数来处理，最终事件匹配了boost::statechart::event_base事件，直接调用函数discard_event把事件丢弃掉。

### 1.4 状态机的定义
RecoveryMachine为定义的状态机，需要继承boost::statechart::state_machine类：
{% highlight string %}
struct Initial;
class RecoveryMachine : public boost::statechart::state_machine< RecoveryMachine, Initial > {
};
{% endhighlight %}

模板参数第一个参数为自己的名字，第二个参数为状态机默认的初始状态Initial。

状态机的基本操作有两个：

* 状态机的初始化
{% highlight string %}
machine.initiate();
{% endhighlight %}
```initiate()```是继承自boost::statechart::state_machine的成员函数。

* 函数process_event()用来向状态机投递事件，从而触发状态机接收并处理该事件
{% highlight string %}
machine.process_event(evt);
{% endhighlight %}

```process_event()```也是继承自boost::statechart::state_machine的成员函数。

### 1.5 context函数
context是状态机的一个比较有用的函数，它可以获取当前状态的所有祖先状态的指针。通过它可以获取父状态以及祖先状态的一些内部参数和状态值。context()函数是实现在boost::statechart::state_machine中的：
{% highlight string %}
// Returns a reference to the context identified by the template
// parameter. This can either be _this_ object or one of its direct or
// indirect contexts.
template< class Context >
Context & context()
{
  // As we are in the outermost context here, only this object can be
  // returned.
  return *polymorphic_downcast< MostDerived * >( this );
}

template< class Context >
const Context & context() const
{
  // As we are in the outermost context here, only this object can be
  // returned.
  return *polymorphic_downcast< const MostDerived * >( this );
}
{% endhighlight %}

同事context()函数在boost::statechart::simple_state中也有实现：
{% highlight string %}
template< class OtherContext >
    OtherContext & context()
    {
      typedef typename mpl::if_<
        is_base_of< OtherContext, MostDerived >,
        context_impl_this_context,
        context_impl_other_context
      >::type impl;
      return impl::template context_impl< OtherContext >( *this );
    }

    template< class OtherContext >
    const OtherContext & context() const
    {
      typedef typename mpl::if_<
        is_base_of< OtherContext, MostDerived >,
        context_impl_this_context,
        context_impl_other_context
      >::type impl;
      return impl::template context_impl< OtherContext >( *this );
    }
{% endhighlight %}
从simple_state的实现来看，可以获取当前状态的祖先状态指针，也可以获取当前状态所属状态机的指针。

例如状态```Started```是RecoveryMachine的一个状态，状态Start是Started状态的一个子状态，那么如果当前状态是```Start```，就可以通过该函数获取它的父状态Started的指针：
{% highlight string %}
Started * parent = context< Started >();
{% endhighlight %}
同时也可以获取其祖先状态RecoveryMachine的指针：
{% highlight string %}
RecoveryMachine *machine = context< RecoveryMachine >();
{% endhighlight %}

综上所述，context()函数为获取当前状态的祖先状态上下文提供了一种方法。

### 1.6 事件的特殊处理
事件除了在状态转移列表中触发状态转移，或者进入用户自定义的状态处理函数，还可以有下列特殊的处理方式：

* 在用户自定义的函数里，可以直接调用函数```transit```来直接跳转到目标状态。例如：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Initial::react(const MLogRec& i)
{
  PG *pg = context< RecoveryMachine >().pg;
  assert(!pg->is_primary());
  post_event(i);
  return transit< Stray >();
}
{% endhighlight %}
可以直接跳转到状态```Stray```。

* 在用户自定义的函数里，可以调用函数post_event()直接产生相应的事件，并投递给状态机
{% highlight string %}
PG::RecoveryState::Start::Start(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Start")
{
  context< RecoveryMachine >().log_enter(state_name);

  PG *pg = context< RecoveryMachine >().pg;
  if (pg->is_primary()) {
    dout(1) << "transitioning to Primary" << dendl;
    post_event(MakePrimary());
  } else { //is_stray
    dout(1) << "transitioning to Stray" << dendl; 
    post_event(MakeStray());
  }
}
{% endhighlight %}

* 在用户的自定义函数里，调用函数discard_event()可以直接丢弃事件，不做任何处理
{% highlight string %}
boost::statechart::result PG::RecoveryState::Primary::react(const ActMap&)
{
  dout(7) << "handle ActMap primary" << dendl;
  PG *pg = context< RecoveryMachine >().pg;
  pg->publish_stats_to_osd();
  pg->take_waiters();
  return discard_event();
}
{% endhighlight %}

* 在用户的自定义函数里，调用函数forward_event()可以把当前事件继续投递给```父状态机```进行处理
{% highlight string %}
boost::statechart::result PG::RecoveryState::WaitUpThru::react(const ActMap& am)
{
  PG *pg = context< RecoveryMachine >().pg;
  if (!pg->need_up_thru) {
    post_event(Activate(pg->get_osdmap()->get_epoch()));
  }
  return forward_event();
}
{% endhighlight %}


## 2. PG状态机
在类PG的内部定义了类RecoveryState，该类RecoveryState的内部定义了PG的状态机RecoveryMachine和它的各种状态。
{% highlight string %}
class PG{
	class RecoveryState{
		class RecoveryMachine{
		};
	};
};
{% endhighlight %}
在每个PG对象创建时，在构造函数里创建一个新的RecoveryState类的对象，并创建相应的RecoveryMachine类的对象，也就是创建了一个新的状态机。每个PG类对应一个独立的状态机来控制该PG的状态转换。
{% highlight string %}
PG::PG(OSDService *o, OSDMapRef curmap,
       const PGPool &_pool, spg_t p) :
	recovery_state(this){
}

class RecoveryState{
public:
	explicit RecoveryState(PG *pg)
      : machine(this, pg), pg(pg), orig_ctx(0) {
      machine.initiate();
    }
};
{% endhighlight %}

上面machine.initiate()调用的是boost::statechart::state_machine中的initiate()方法。

下图10-1为PG状态机的总体状态转换图，相对比较复杂，在介绍相关的内容模块时再逐一详细介绍。


![ceph-chapter10-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_1.jpg)


## 3. PG的创建过程
在PG的创建过程中完成了PG对应状态机的创建和状态机的初始化操作。一个PG的创建过程会由其所在OSD在PG中担任的角色不同，创建的机制也不相同。

### 3.1 PG在主OSD上的创建
当创建一个Pool时，通过客户端命令行给Monitor发送创建Pool的命令，Monitor对该Pool的每一个PG对应的主OSD发送创建PG的请求。

函数OSD::handle_pg_create()用于处理Monitor发送的创建PG的请求，其消息类型为MOSDPGCreate数据结构：
{% highlight string %}
/*
 * PGCreate - instruct an OSD to create a pg, if it doesn't already exist
 */

struct MOSDPGCreate : public Message {

  const static int HEAD_VERSION = 3;
  // At head_version 2 the unspecified compat_version was set to 2
  const static int COMPAT_VERSION = 2;

  version_t          epoch;
  map<pg_t,pg_create_t> mkpg;             //要创建的PG列表，一次可以创建多个PG
  map<pg_t,utime_t> ctimes;               //对应PG的创建时间
};
{% endhighlight %}

数据结构pg_create_t包含了一个PG创建相关的参数：
{% highlight string %}
// placement group id
struct pg_t {
	uint64_t m_pool;
	uint32_t m_seed;
	int32_t m_preferred;
...
};


struct pg_create_t {
	epoch_t created;   // epoch pg created
	pg_t parent;       // split from parent (if != pg_t())
	__s32 split_bits;
};
{% endhighlight %}


函数handle_pg_create()的处理过程如下：
{% highlight string %}
/*
 * holding osd_lock
 */
void OSD::handle_pg_create(OpRequestRef op)
{
	...
}
{% endhighlight %}

1) 首先调用函数require_mon_peer()确保是由Monitor发送的创建消息；

2） 调用函数require_same_or_newer_map()检查epoch是否一致。如果对方的epoch比自己拥有的新，就调用wait_for_new_map()等待更新自己的epoch，返回false，拒绝该pg create请求

3） 对消息中mkpg列表里的每一个PG，开始执行如下创建操作

&emsp; a) 检查该PG的参数split_bits，如果不为0，那么就是PG的分裂请求，这里不做处理；检查PG的preferred，如果设置了，就跳过，目前不支持；检查确认该Pool存在；检查本OSD是该PG的主OSD；如果参数up不等于acting，说明该PG有temp_pg，至少确定该PG存在，直接跳过；

&emsp; b) 调用函数handle_pg_peering_evt()处理PG peering创建事件
{% highlight string %}
b1) 调用_have_pg()判断PG是否存在。如果该PG已经存在，跳过；

b2) 调用函数PG::_create()在本地对象存储中创建相应的collection

b3) 调用函数_create_lock_pg()创建PG对象并初始化

b4) 调用函数pg->handle_create(&rctx)给新创建PG状态机投递事件，PG的状态发生相应的改变，后面会介绍；

b5) 所有修改操作都打包在事务rctx.transaction中，调用函数dispatch_context()将事件提交到本地对象存储中
{% endhighlight %}

4) 调用函数maybe_update_heartbeat_peers()来更新OSD的心跳列表；


### 3.2 PG在从OSD上的创建
Monitor并不会给PG的从OSD发送消息来创建该PG，而是由该主OSD上的PG在Peering过程中创建。主OSD给从OSD的PG状态机投递事件时，在函数OSD::handle_pg_peering_evt()中，如果发现该PG不存在，才完成创建该PG。

函数handle_pg_peering_evt()是处理Peering状态机事件的入口，函数会查找相应的PG，如果该PG不存在，就创建该PG。该PG的状态机进入RecoveryMachine/Stray状态。

### 3.3 PG的加载
当OSD重启时，调用函数OSD::init()，该函数调用load_pgs()加载已经存在的PG，其处理过程和创建PG的过程相似。
{% highlight string %}
void OSD::load_pgs(){
	...
	pg->handle_loaded(&rctx);
	...
}

void PG::handle_loaded(RecoveryCtx *rctx)
{
  dout(10) << "handle_loaded" << dendl;
  Load evt;
  recovery_state.handle_event(evt, rctx);
}

boost::statechart::result PG::RecoveryState::Initial::react(const Load& l)
{
  PG *pg = context< RecoveryMachine >().pg;

  // do we tell someone we're here?
  pg->send_notify = (!pg->is_primary());

  return transit< Reset >();
}
{% endhighlight %}
然后出发PG的Peering过程。


## 4. PG创建后状态机的状态转换
下图10-2为PG总体状态转换图的简化版： 状态Peering、Active、ReplicaActive的内部状态没有添加进去：

![ceph-chapter10-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_2.jpg)

再结合前面PG状态转换图的详细版本，我们可以大体画出PG状态机的一个层次结构：

![ceph-chapter10-3](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_3.jpg)

通过上图可以了解PG的高层状态转换过程，如下所示：

1） 当PG创建后，同时在该类内部创建了一个属于该PG的RecoveryMachine类型的状态机，该状态机的初始化状态为默认初始化状态Initial。

2) 在PG创建后，调用函数pg->handle_create(&rctx)来给状态机投递事件
{% highlight string %}
void PG::handle_create(RecoveryCtx *rctx)
{
  dout(10) << "handle_create" << dendl;
  rctx->created_pgs.insert(this);
  Initialize evt;
  recovery_state.handle_event(evt, rctx);
  ActMap evt2;
  recovery_state.handle_event(evt2, rctx);
}
{% endhighlight %}
由以上代码可知：该函数首先向RecoveryMachine投递了Initialize类型的事件。由上图```10-2```可知，状态机在RecoveryMachine/Initial状态接收到Initialize类型的事件后直接转移到Reset状态。其次，向RecoveryMachine投递了ActMap事件。

3） 状态Reset接收到ActMap事件，跳转到Started状态
{% highlight string %}
boost::statechart::result PG::RecoveryState::Reset::react(const ActMap&)
{
  PG *pg = context< RecoveryMachine >().pg;
  if (pg->should_send_notify() && pg->get_primary().osd >= 0) {
    context< RecoveryMachine >().send_notify(
      pg->get_primary(),
      pg_notify_t(
	pg->get_primary().shard, pg->pg_whoami.shard,
	pg->get_osdmap()->get_epoch(),
	pg->get_osdmap()->get_epoch(),
	pg->info),
      pg->past_intervals);
  }

  pg->update_heartbeat_peers();
  pg->take_waiters();

  return transit< Started >();
}
{% endhighlight %}
在自定义的react函数里直接调用了transit函数跳转到Started状态。

4) 进入状态RecoveryMachine/Started后，就进入RecoveryMachine/Started的默认的子状态RecoveryMachine/Started/Start中
{% highlight string %}
PG::RecoveryState::Start::Start(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Start")
{
  context< RecoveryMachine >().log_enter(state_name);

  PG *pg = context< RecoveryMachine >().pg;
  if (pg->is_primary()) {
    dout(1) << "transitioning to Primary" << dendl;
    post_event(MakePrimary());
  } else { //is_stray
    dout(1) << "transitioning to Stray" << dendl; 
    post_event(MakeStray());
  }
}
{% endhighlight %}
由以上代码可知，在Start状态的构造函数中，根据本OSD在该PG中担任的角色不同分别进行如下处理：

* 如果是主OSD，就调用函数post_event()，抛出事件MakePrimary，进入主OSD的默认子状态Primary/Peering中；

* 如果是从OSD，就调用函数post_event()，抛出事件MakeStray，进入Started/Stray状态；

对于一个OSD的PG处于Stray状态，是指该OSD上的PG副本目前状态不确定，但是可以响应主OSD的各种查询操作。它有两种可能：一种是最终转移到状态ReplicaActive，处于活跃状态，成为PG的一个副本；另一种可能的情况是：如果是数据迁移的源端，可能一直保持Stray状态，该OSD上的副本可能在数据迁移完成后，PG以及数据就都被删除了。


<br />
<br />

**[参看]**

1. [Ceph源码解析：PG peering](https://www.cnblogs.com/chenxianpao/p/5565286.html)

2. [boost官网文档](https://www.boost.org/doc/libs/)

<br />
<br />
<br />

