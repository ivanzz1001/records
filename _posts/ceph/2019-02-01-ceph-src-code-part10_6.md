---
layout: post
title: ceph peering机制再研究(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本节我们介绍一下PG Recovery过程中的一些重要数据结构。


<!-- more -->

## 1. RecoveryCtx数据结构
{% highlight string %}
class PG : DoutPrefixProvider {
public:    
  struct BufferedRecoveryMessages {
	map<int, map<spg_t, pg_query_t> > query_map;
	map<int, vector<pair<pg_notify_t, pg_interval_map_t> > > info_map;
	map<int, vector<pair<pg_notify_t, pg_interval_map_t> > > notify_list;
  };
	
  struct RecoveryCtx {
	utime_t start_time;
	map<int, map<spg_t, pg_query_t> > *query_map;
	map<int, vector<pair<pg_notify_t, pg_interval_map_t> > > *info_map;
	map<int, vector<pair<pg_notify_t, pg_interval_map_t> > > *notify_list;
	set<PGRef> created_pgs;
	C_Contexts *on_applied;
	C_Contexts *on_safe;
	ObjectStore::Transaction *transaction;
	ThreadPool::TPHandle* handle;
	
	RecoveryCtx(map<int, map<spg_t, pg_query_t> > *query_map,
		map<int,vector<pair<pg_notify_t, pg_interval_map_t> > > *info_map,
		map<int,vector<pair<pg_notify_t, pg_interval_map_t> > > *notify_list,
		C_Contexts *on_applied,
		C_Contexts *on_safe,
		ObjectStore::Transaction *transaction)
	: query_map(query_map), info_map(info_map), 
	notify_list(notify_list),
	on_applied(on_applied),
	on_safe(on_safe),
	transaction(transaction),
	handle(NULL) {}

	RecoveryCtx(BufferedRecoveryMessages &buf, RecoveryCtx &rctx)
	: query_map(&(buf.query_map)),
	info_map(&(buf.info_map)),
	notify_list(&(buf.notify_list)),
	on_applied(rctx.on_applied),
	on_safe(rctx.on_safe),
	transaction(rctx.transaction),
	handle(rctx.handle) {}

	void accept_buffered_messages(BufferedRecoveryMessages &m) {
		assert(query_map);
		assert(info_map);
		assert(notify_list);
		
		for (map<int, map<spg_t, pg_query_t> >::iterator i = m.query_map.begin();i != m.query_map.end();++i) {
			map<spg_t, pg_query_t> &omap = (*query_map)[i->first];
			
			for (map<spg_t, pg_query_t>::iterator j = i->second.begin();j != i->second.end();++j) {
				omap[j->first] = j->second;
			}
		}
		
		for (map<int, vector<pair<pg_notify_t, pg_interval_map_t> > >::iterator i = m.info_map.begin();i != m.info_map.end();++i) {
			vector<pair<pg_notify_t, pg_interval_map_t> > &ovec = (*info_map)[i->first];
			ovec.reserve(ovec.size() + i->second.size());
			ovec.insert(ovec.end(), i->second.begin(), i->second.end());
		}
		
		for (map<int, vector<pair<pg_notify_t, pg_interval_map_t> > >::iterator i = m.notify_list.begin();i != m.notify_list.end();++i) {
			vector<pair<pg_notify_t, pg_interval_map_t> > &ovec = (*notify_list)[i->first];
			ovec.reserve(ovec.size() + i->second.size());
			ovec.insert(ovec.end(), i->second.begin(), i->second.end());
		}
	}
	
  };
  
  
};
{% endhighlight %}
RecoveryCtx作为一次恢复操作的上下文，我们介绍一下其中几个比较重要的字段：

* query_map： 用于缓存PG Query查询信息，后续会将这些缓存信息构造成MOSDPGQuery消息，然后发送到对应的OSD上。query_map的key部分为OSD的序号。

* info_map: 用于缓存pg_notify_t信息，后续会将这些缓存信息构造成MOSDPGInfo查询的消息，然后发送到对应的OSD上。info_map的key部分为OSD的序号。

* notify_list：用于缓存pg_notify_t信息，后续会将这些缓存信息构造成MOSDPGNotify消息，然后发送到对应的OSD上。notify_list的key部分为OSD的序号

* transaction：本RecoveryCtx所关联的事务。在恢复过程中可能涉及到需要将相关信息持久化，就通过此transaction来完成

* handle：ThreadPool::TPHandle的主要作用在于监视线程池中每一个线程的执行时常。每次线程函数执行时，都会设置一个grace超时时间，当线程执行超过该时间，就认为是unhealthy的状态。当执行时间超过suicide_grace时，OSD就会产生断言而导致自杀。这里向transaction对应的ObjectStore传入handle参数，主要是为了处理超时方面的问题
{% highlight string %}
int FileStore::queue_transactions(Sequencer *posr, vector<Transaction>& tls,
				  TrackedOpRef osd_op,
				  ThreadPool::TPHandle *handle)
{
	...

	if (handle)
		handle->suspend_tp_timeout();
	
	op_queue_reserve_throttle(o);
	journal->reserve_throttle_and_backoff(tbl.length());
	
	if (handle)
		handle->reset_tp_timeout();

	...
}
{% endhighlight %}

### 1.1 RecoveryCtx的使用
在OSD中，通常使用如下函数来创建RecoveryCtx对象：
{% highlight string %}
// ----------------------------------------
// peering and recovery

PG::RecoveryCtx OSD::create_context()
{
	ObjectStore::Transaction *t = new ObjectStore::Transaction;
	C_Contexts *on_applied = new C_Contexts(cct);
	C_Contexts *on_safe = new C_Contexts(cct);

	map<int, map<spg_t,pg_query_t> > *query_map = 
		new map<int, map<spg_t, pg_query_t> >;

	map<int,vector<pair<pg_notify_t, pg_interval_map_t> > > *notify_list =
		new map<int, vector<pair<pg_notify_t, pg_interval_map_t> > >;

	map<int,vector<pair<pg_notify_t, pg_interval_map_t> > > *info_map =
		new map<int,vector<pair<pg_notify_t, pg_interval_map_t> > >;

	PG::RecoveryCtx rctx(query_map, info_map, notify_list,on_applied, on_safe, t);
	return rctx;
}
{% endhighlight %}

之后，调用如下函数将对应map里面的数据发送出去：
{% highlight string %}
void OSD::dispatch_context(PG::RecoveryCtx &ctx, PG *pg, OSDMapRef curmap,
                           ThreadPool::TPHandle *handle)
{
	if (service.get_osdmap()->is_up(whoami) &&is_active()) {
		do_notifies(*ctx.notify_list, curmap);
		do_queries(*ctx.query_map, curmap);
		do_infos(*ctx.info_map, curmap);
	}
	delete ctx.notify_list;
	delete ctx.query_map;
	delete ctx.info_map;

	if ((ctx.on_applied->empty() &&ctx.on_safe->empty() &&
	  ctx.transaction->empty() && ctx.created_pgs.empty()) || !pg) {
		delete ctx.transaction;
		delete ctx.on_applied;
		delete ctx.on_safe;
		assert(ctx.created_pgs.empty());
	} else {
		if (!ctx.created_pgs.empty()) {
			ctx.on_applied->add(new C_OpenPGs(ctx.created_pgs, store));
		}

		int tr = store->queue_transaction(
			pg->osr.get(),
			std::move(*ctx.transaction), ctx.on_applied, ctx.on_safe, NULL, TrackedOpRef(),
			handle);

		delete (ctx.transaction);
		assert(tr == 0);
	}
}
{% endhighlight %}

## 2. NamedState数据结构
{% highlight string %}
class PG : DoutPrefixProvider {
public:    
  struct NamedState {
	const char *state_name;
	utime_t enter_time;
	
	const char *get_state_name() { return state_name; }
	
	NamedState(CephContext *cct_, const char *state_name_)
		: state_name(state_name_),
		enter_time(ceph_clock_now(cct_)) {}
		
	virtual ~NamedState() {}
  };
  
};
{% endhighlight %}
NamedState主要是用于对一种状态进行命名。




## 3. statechart状态机

Ceph在处理PG的状态转换时，使用了boost库提供的statechart状态机。因此，这里先简单介绍一下statechart状态机的基本概念和涉及的相关知识，以便更好地理解Peering过程PG的状态转换流程。

### 3.1 状态
在statechart里，状态的定义有两种方式：

* 没有子状态情况下的状态定义
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
定义一个状态需要继承boost::statechart::simple_state或者boost::statechart::state类。上面Reset状态继承了boost::statechart::state类。该类的模板参数中，第一个参数为状态机自己的名字Reset，第二个参数为所属状态机的名字，表明Reset是状态机RecoveryMachine的一个状态。

* 有子状态情况下的状态定义
{%	highlight string %}
struct Start;

struct Started : boost::statechart::state< Started, RecoveryMachine, Start >, NamedState {
};
{% endhighlight %}
状态```Started```也是状态机RecoveryMachine的一个状态，模板参数中多了一个参数```Start```，它是状态```Started```的默认初始子状态，其定义如下：
{% highlight string %}
struct Start : boost::statechart::state< Start, Started >, NamedState {
};
{% endhighlight %}
上面定义的```状态Start```是状态Started的子状态。第一个模板参数是自己的名字，第二个模板参数是该子状态所属父状态的名字。

综上所述，一个状态，要么属于一个状态机，要么属于一个状态，成为该状态的子状态。其定义的模板参数是自己，第二个模板参数是拥有者，第三个模板参数是它的起始子状态。

### 3.2 事件
状态能够接收并处理事件。事件可以改变状态，促使状态发生转移。在boost库的statechart状态机中定义事件的方式如下所示：
{% highlight string %}
struct QueryState : boost::statechart::event< QueryState >{
}; 
{% endhighlight %}
QueryState为一个事件，需要继承boost::statechart::event类，模板参数为自己的名字。

### 3.3 状态响应事件
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
  TrivialEvent(NullEvt)

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

###### 3.3.1 可处理的事件列表及处理对应事件的方法

上述代码列出了状态RecoveryMachine/Initial可以处理的事件列表和处理对应事件的方法：

1） 通过boost::mpl::list定义该状态可以处理多个事件类型。在本例中可以处理```Initialize```、```Load```、```NullEvt```和```event_base```事件。

2）简单事件处理
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

###### 3.3.2 存在的一些疑问

1） **对于NullEvt事件**
{% highlight string %}
boost::statechart::custom_reaction< NullEvt >
{% endhighlight %}
但是我们却并没有定义该事件的reaction函数。因为NullEvt继承自boost::statechart::event_base，因此其会调用如下函数来进行处理：
{% highlight string %}
boost::statechart::result react(const boost::statechart::event_base&) {
	return discard_event();
}
{% endhighlight %}
关于这一点，我们可以通过后面的```statechart示例```来得到验证；

2） **对于```MNotifyRec```、```MInfoRec```以及```MLogRec```事件**

对于上面的3个事件，我们发现并没有将其添加到boost::mpl::list中，对于此类事件，会通过如下语句来进行处理：
{% highlight string %}
boost::statechart::transition< boost::statechart::event_base, Crashed >
{% endhighlight %}
关于这一点，我们也可以通过后面的```statechart示例```来得到验证；

3） **对于boost::statechart::event_base事件，似乎有两种不同的处理方式**
{% highlight string %}
struct Initial : boost::statechart::state< Initial, RecoveryMachine >, NamedState {

  typedef boost::mpl::list <
boost::statechart::transition< boost::statechart::event_base, Crashed >
> reactions;

  boost::statechart::result react(const boost::statechart::event_base&) {
	return discard_event();
  }
};
{% endhighlight %}
这里我们通过试验发现：

* 对于没有添加到boost::mpl::list中的事件，其默认就会调用如下来进行处理
{% highlight string %}
boost::statechart::transition< boost::statechart::event_base, Crashed >
{% endhighlight %}

* 对于添加到了boost::mpl::list中的事件，如果直接找不到对应的react函数，且没有定义其父类型boost::statechart::event_base的react函数，那么直接会编译时报错

* 对于添加到了boost::mpl::list中的事件，如果直接找不到对应的react函数的话，但是定义了其父类型boost::statechart::event_base的react函数，那么会调用其父类型的react函数来进行处理
{% highlight string %}
boost::statechart::result react(const boost::statechart::event_base&) {
	return discard_event();
 }
{% endhighlight %}


### 3.4 状态机的定义
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

### 3.5 context函数
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
同时context()函数在boost::statechart::simple_state中也有实现：
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

例如状态```Started```是RecoveryMachine的一个状态，状态Start是Started状态的一个子状态，那么如果当前状态是```Start```，就可以通过该函数获取它的父状态```Started```的指针：
{% highlight string %}
Started * parent = context< Started >();
{% endhighlight %}
同时也可以获取其祖先状态RecoveryMachine的指针：
{% highlight string %}
RecoveryMachine *machine = context< RecoveryMachine >();
{% endhighlight %}
综上所述，context()函数为获取当前状态的祖先状态上下文提供了一种方法。

### 3.6 事件的特殊处理
事件除了在状态转移列表中触发状态转移，或者进入用户自定义的状态处理函数，还可以有下列特殊的处理方式：

* 在用户自定义的函数里，可以直接调用```transit```来直接跳转到目标状态。例如：
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

* 在用户的自定义函数里，调用函数forward_event()可以把当前事件继续投递给父状态机进行处理
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

### 3.7 statechart示例
{% highlight string %}
// Example program
#include <iostream>
#include <string>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/event_base.hpp>



#define TrivialEvent(T) struct T : boost::statechart::event< T > { \
    T() : boost::statechart::event< T >() {}			   \
    void print(std::ostream *out) const {			   \
      *out << #T;						   \
    }								   \
  };
TrivialEvent(Initialize)
TrivialEvent(Load)
TrivialEvent(NullEvt)
TrivialEvent(GoClean)

struct MInfoRec : boost::statechart::event< MInfoRec > {
    std::string name; 
    MInfoRec(std::string name): name(name){
    }
    
    void print(){
        std::cout<<"MInfoRec: "<<name<<"\n";
    }
};

struct MLogRec : boost::statechart::event< MLogRec > {
    std::string name; 
    MLogRec(std::string name): name(name){
    }
    
    void print(){
        std::cout<<"MLogRec: "<<name<<"\n";
    }
};

struct MNotifyRec : boost::statechart::event< MNotifyRec > {
    std::string name; 
    MNotifyRec(std::string name): name(name){
        
    }
    
    void print(){
        std::cout<<"MNotifyRec: "<<name<<"\n";
    }
};



struct Initial; 

struct RecoveryMachine : boost::statechart::state_machine< RecoveryMachine, Initial > {}; 

struct Reset;

struct Crashed : boost::statechart::state< Crashed, RecoveryMachine > {
    explicit Crashed(my_context ctx) : my_base(ctx)
    {
        std::cout << "Hello, Crashed!\n";
    }
};
    
struct Initial : boost::statechart::state< Initial, RecoveryMachine > {
    
    typedef boost::mpl::list < 
    boost::statechart::transition< Initialize, Reset >,
    boost::statechart::custom_reaction< Load >,
    boost::statechart::custom_reaction< NullEvt >,
    boost::statechart::transition< boost::statechart::event_base, Crashed >
    > reactions;
    
    explicit Initial(my_context ctx) : my_base(ctx)
    {
        std::cout << "Hello, Initial!\n";
    } 
    
    boost::statechart::result react(const Load& l){
        return transit< Reset >();
    }
    
     boost::statechart::result react(const MNotifyRec& notify){
         std::cout<<"Initial::react::MLogRec!\n";
         
         return discard_event();
     }
	boost::statechart::result react(const MInfoRec& i){
		std::cout<<"Initial::react::MNotifiyRec!\n";

		return discard_event();
	}

	boost::statechart::result react(const MLogRec& log){
		std::cout<<"Initial::react::MLogRec!\n";
	
		return discard_event();
	}
    
    boost::statechart::result react(const boost::statechart::event_base&) {
        std::cout << "Initial event_base processed!\n";
	    return discard_event();
    }
    
    void exit() { 
        std::cout << "Bye, Initial!\n";
    } 
};

struct Reset : boost::statechart::state< Reset, RecoveryMachine > {
    explicit Reset(my_context ctx) : my_base(ctx)
    {
        std::cout << "Hello, Reset!\n";
    } 
    
    void exit() { 
        std::cout << "Bye, Reset!\n";
    }
};

int main(int argc, char *argv[])
{
  RecoveryMachine machine;
  
  machine.initiate();
  
  //machine.process_event(NullEvt());                        //语句1
  
  //machine.process_event(GoClean());                        //语句2
  
  //machine.process_event(MNotifyRec("notify record"));      //语句3
  
  return 0x0;
}
{% endhighlight %}

上面的示例与PG中对于Initial状态的处理类似，下面我们来看一下分别执行上述语句时的打印情况：

* 单独执行语句1
<pre>
Hello, Initial!
Initial event_base processed!
</pre>

* 单独执行语句2
<pre>
Hello, Initial!
Bye, Initial!
Hello, Crashed!
</pre>

* 单独执行语句3
<pre>
Hello, Initial!
Bye, Initial!
Hello, Crashed!
</pre>



## 4. PG状态机

### 4.1 PG状态机中的所有事件

下面我们列出Recovery过程中的所有事件：

* QueryState

* MInfoRec

* MLogRec

* MNotifyRec

* MQuery

* AdvMap

* ActMap

* Activate

* RequestBackfillPrio

*  TrivialEvent事件
{% highlight string %}
class PG : DoutPrefixProvider {
public:    
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
  
};
{% endhighlight %}

* MakePrimary

* MakeStray

* NeedActingChange

* IsIncomplete

* GotLog

* SnapTrim

* Reset

* SnapTrimReserved

* SnapTrimTimerReady

### 4.2 PG状态机中的所有状态及其对应的事件

* Crashed状态
{% highlight string %}
struct Crashed : boost::statechart::state< Crashed, RecoveryMachine >, NamedState {
      explicit Crashed(my_context ctx);
};
{% endhighlight %}

* Initial状态
{% highlight string %}
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

* Reset状态
{% highlight string %}
struct Reset : boost::statechart::state< Reset, RecoveryMachine >, NamedState {
  explicit Reset(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< NullEvt >,
boost::statechart::custom_reaction< FlushedEvt >,
boost::statechart::custom_reaction< IntervalFlush >,
boost::statechart::transition< boost::statechart::event_base, Crashed >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const AdvMap&);
  boost::statechart::result react(const ActMap&);
  boost::statechart::result react(const FlushedEvt&);
  boost::statechart::result react(const IntervalFlush&);
  boost::statechart::result react(const boost::statechart::event_base&) {
return discard_event();
  }
};
{% endhighlight %}

* Started状态
{% highlight string %}
struct Started : boost::statechart::state< Started, RecoveryMachine, Start >, NamedState {
  explicit Started(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::custom_reaction< NullEvt >,
boost::statechart::custom_reaction< FlushedEvt >,
boost::statechart::custom_reaction< IntervalFlush >,
boost::statechart::transition< boost::statechart::event_base, Crashed >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const AdvMap&);
  boost::statechart::result react(const FlushedEvt&);
  boost::statechart::result react(const IntervalFlush&);
  boost::statechart::result react(const boost::statechart::event_base&) {
return discard_event();
  }
};
{% endhighlight %}

* Start状态
{% highlight string %}
struct Start : boost::statechart::state< Start, Started >, NamedState {
  explicit Start(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::transition< MakePrimary, Primary >,
boost::statechart::transition< MakeStray, Stray >
> reactions;
};
{% endhighlight %}

* Primary状态
{% highlight string %}
struct Primary : boost::statechart::state< Primary, Started, Peering >, NamedState {
  explicit Primary(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< MNotifyRec >,
boost::statechart::transition< NeedActingChange, WaitActingChange >
> reactions;
  boost::statechart::result react(const ActMap&);
  boost::statechart::result react(const MNotifyRec&);
};
{% endhighlight %}

* WaitActingChange状态
{% highlight string %}
struct WaitActingChange : boost::statechart::state< WaitActingChange, Primary>,
		      NamedState {
  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::custom_reaction< MInfoRec >,
boost::statechart::custom_reaction< MNotifyRec >
> reactions;
  explicit WaitActingChange(my_context ctx);
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const AdvMap&);
  boost::statechart::result react(const MLogRec&);
  boost::statechart::result react(const MInfoRec&);
  boost::statechart::result react(const MNotifyRec&);
  void exit();
};
{% endhighlight %}

* Peering状态
{% highlight string %}
struct Peering : boost::statechart::state< Peering, Primary, GetInfo >, NamedState {
  std::unique_ptr< PriorSet > prior_set;
  bool history_les_bound;  //< need osd_find_best_info_ignore_history_les

  explicit Peering(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::transition< Activate, Active >,
boost::statechart::custom_reaction< AdvMap >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const AdvMap &advmap);
};
{% endhighlight %}

* Active状态
{% highlight string %}
struct Active : boost::statechart::state< Active, Primary, Activating >, NamedState {
  explicit Active(my_context ctx);
  void exit();

  const set<pg_shard_t> remote_shards_to_reserve_recovery;
  const set<pg_shard_t> remote_shards_to_reserve_backfill;
  bool all_replicas_activated;

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::custom_reaction< MInfoRec >,
boost::statechart::custom_reaction< MNotifyRec >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::custom_reaction< Backfilled >,
boost::statechart::custom_reaction< AllReplicasActivated >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const ActMap&);
  boost::statechart::result react(const AdvMap&);
  boost::statechart::result react(const MInfoRec& infoevt);
  boost::statechart::result react(const MNotifyRec& notevt);
  boost::statechart::result react(const MLogRec& logevt);
  boost::statechart::result react(const Backfilled&) {
return discard_event();
  }
  boost::statechart::result react(const AllReplicasActivated&);
};
{% endhighlight %}

* Clean状态
{% highlight string %}
struct Clean : boost::statechart::state< Clean, Active >, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< DoRecovery, WaitLocalRecoveryReserved >
  > reactions;
  explicit Clean(my_context ctx);
  void exit();
};
{% endhighlight %}

* Recovered状态
{% highlight string %}
struct Recovered : boost::statechart::state< Recovered, Active >, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< GoClean, Clean >,
boost::statechart::custom_reaction< AllReplicasActivated >
  > reactions;
  explicit Recovered(my_context ctx);
  void exit();
  boost::statechart::result react(const AllReplicasActivated&) {
post_event(GoClean());
return forward_event();
  }
};
{% endhighlight %}

* Backfilling状态
{% highlight string %}
struct Backfilling : boost::statechart::state< Backfilling, Active >, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< Backfilled, Recovered >,
boost::statechart::custom_reaction< RemoteReservationRejected >
> reactions;
  explicit Backfilling(my_context ctx);
  boost::statechart::result react(const RemoteReservationRejected& evt);
  void exit();
};
{% endhighlight %}

* WaitRemoteBackfillReserved状态
{% highlight string %}
struct WaitRemoteBackfillReserved : boost::statechart::state< WaitRemoteBackfillReserved, Active >, NamedState {
  typedef boost::mpl::list<
boost::statechart::custom_reaction< RemoteBackfillReserved >,
boost::statechart::custom_reaction< RemoteReservationRejected >,
boost::statechart::transition< AllBackfillsReserved, Backfilling >
> reactions;
  set<pg_shard_t>::const_iterator backfill_osd_it;
  explicit WaitRemoteBackfillReserved(my_context ctx);
  void exit();
  boost::statechart::result react(const RemoteBackfillReserved& evt);
  boost::statechart::result react(const RemoteReservationRejected& evt);
};
{% endhighlight %}

* WaitLocalBackfillReserved状态
{% highlight string %}
struct WaitLocalBackfillReserved : boost::statechart::state< WaitLocalBackfillReserved, Active >, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< LocalBackfillReserved, WaitRemoteBackfillReserved >
> reactions;
  explicit WaitLocalBackfillReserved(my_context ctx);
  void exit();
};
{% endhighlight %}

* NotBackfilling状态
{% highlight string %}
struct NotBackfilling : boost::statechart::state< NotBackfilling, Active>, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< RequestBackfill, WaitLocalBackfillReserved>,
boost::statechart::custom_reaction< RemoteBackfillReserved >,
boost::statechart::custom_reaction< RemoteReservationRejected >
> reactions;
  explicit NotBackfilling(my_context ctx);
  void exit();
  boost::statechart::result react(const RemoteBackfillReserved& evt);
  boost::statechart::result react(const RemoteReservationRejected& evt);
};
{% endhighlight %}

* ReplicaActive状态
{% highlight string %}
struct ReplicaActive : boost::statechart::state< ReplicaActive, Started, RepNotRecovering >, NamedState {
  explicit ReplicaActive(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< MQuery >,
boost::statechart::custom_reaction< MInfoRec >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::custom_reaction< Activate >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const MInfoRec& infoevt);
  boost::statechart::result react(const MLogRec& logevt);
  boost::statechart::result react(const ActMap&);
  boost::statechart::result react(const MQuery&);
  boost::statechart::result react(const Activate&);
};
{% endhighlight %}

* RepRecovering状态
{% highlight string %}
struct RepRecovering : boost::statechart::state< RepRecovering, ReplicaActive >, NamedState {
  typedef boost::mpl::list<
boost::statechart::transition< RecoveryDone, RepNotRecovering >,
boost::statechart::transition< RemoteReservationRejected, RepNotRecovering >,
boost::statechart::custom_reaction< BackfillTooFull >
> reactions;
  explicit RepRecovering(my_context ctx);
  boost::statechart::result react(const BackfillTooFull &evt);
  void exit();
};
{% endhighlight %}

* RepWaitBackfillReserved状态
{% highlight string %}
struct RepWaitBackfillReserved : boost::statechart::state< RepWaitBackfillReserved, ReplicaActive >, NamedState {
  typedef boost::mpl::list<
boost::statechart::custom_reaction< RemoteBackfillReserved >,
boost::statechart::custom_reaction< RemoteReservationRejected >
> reactions;
  explicit RepWaitBackfillReserved(my_context ctx);
  void exit();
  boost::statechart::result react(const RemoteBackfillReserved &evt);
  boost::statechart::result react(const RemoteReservationRejected &evt);
};
{% endhighlight %}

* RepWaitRecoveryReserved状态
{% highlight string %}
struct RepWaitRecoveryReserved : boost::statechart::state< RepWaitRecoveryReserved, ReplicaActive >, NamedState {
  typedef boost::mpl::list<
boost::statechart::custom_reaction< RemoteRecoveryReserved >
> reactions;
  explicit RepWaitRecoveryReserved(my_context ctx);
  void exit();
  boost::statechart::result react(const RemoteRecoveryReserved &evt);
};
{% endhighlight %}

* RepNotRecovering状态
{% highlight string %}
struct RepNotRecovering : boost::statechart::state< RepNotRecovering, ReplicaActive>, NamedState {
  typedef boost::mpl::list<
boost::statechart::custom_reaction< RequestBackfillPrio >,
    boost::statechart::transition< RequestRecovery, RepWaitRecoveryReserved >,
boost::statechart::transition< RecoveryDone, RepNotRecovering >  // for compat with pre-reservation peers
> reactions;
  explicit RepNotRecovering(my_context ctx);
  boost::statechart::result react(const RequestBackfillPrio &evt);
  void exit();
};
{% endhighlight %}

* Recovering状态
{% highlight string %}
struct Recovering : boost::statechart::state< Recovering, Active >, NamedState {
  typedef boost::mpl::list <
boost::statechart::custom_reaction< AllReplicasRecovered >,
boost::statechart::custom_reaction< RequestBackfill >
> reactions;
  explicit Recovering(my_context ctx);
  void exit();
  void release_reservations();
  boost::statechart::result react(const AllReplicasRecovered &evt);
  boost::statechart::result react(const RequestBackfill &evt);
};
{% endhighlight %}

* WaitRemoteRecoveryReserved状态
{% highlight string %}
struct WaitRemoteRecoveryReserved : boost::statechart::state< WaitRemoteRecoveryReserved, Active >, NamedState {
  typedef boost::mpl::list <
boost::statechart::custom_reaction< RemoteRecoveryReserved >,
boost::statechart::transition< AllRemotesReserved, Recovering >
> reactions;
  set<pg_shard_t>::const_iterator remote_recovery_reservation_it;
  explicit WaitRemoteRecoveryReserved(my_context ctx);
  boost::statechart::result react(const RemoteRecoveryReserved &evt);
  void exit();
};
{% endhighlight %}

* WaitLocalRecoveryReserved状态
{% highlight string %}
struct WaitLocalRecoveryReserved : boost::statechart::state< WaitLocalRecoveryReserved, Active >, NamedState {
  typedef boost::mpl::list <
boost::statechart::transition< LocalRecoveryReserved, WaitRemoteRecoveryReserved >
> reactions;
  explicit WaitLocalRecoveryReserved(my_context ctx);
  void exit();
};
{% endhighlight %}

* Activating状态
{% highlight string %}
struct Activating : boost::statechart::state< Activating, Active >, NamedState {
  typedef boost::mpl::list <
boost::statechart::transition< AllReplicasRecovered, Recovered >,
boost::statechart::transition< DoRecovery, WaitLocalRecoveryReserved >,
boost::statechart::transition< RequestBackfill, WaitLocalBackfillReserved >
> reactions;
  explicit Activating(my_context ctx);
  void exit();
};
{% endhighlight %}

* Stray状态
{% highlight string %}
struct Stray : boost::statechart::state< Stray, Started >, NamedState {
  map<int, pair<pg_query_t, epoch_t> > pending_queries;

  explicit Stray(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< MQuery >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::custom_reaction< MInfoRec >,
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< RecoveryDone >
> reactions;
  boost::statechart::result react(const MQuery& query);
  boost::statechart::result react(const MLogRec& logevt);
  boost::statechart::result react(const MInfoRec& infoevt);
  boost::statechart::result react(const ActMap&);
  boost::statechart::result react(const RecoveryDone&) {
return discard_event();
  }
};
{% endhighlight %}

* GetInfo状态
{% highlight string %}
struct GetInfo : boost::statechart::state< GetInfo, Peering >, NamedState {
  set<pg_shard_t> peer_info_requested;

  explicit GetInfo(my_context ctx);
  void exit();
  void get_infos();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::transition< GotInfo, GetLog >,
boost::statechart::custom_reaction< MNotifyRec >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const MNotifyRec& infoevt);
};
{% endhighlight %}

* GetLog状态
{% highlight string %}
struct GetLog : boost::statechart::state< GetLog, Peering >, NamedState {
  pg_shard_t auth_log_shard;
  boost::intrusive_ptr<MOSDPGLog> msg;

  explicit GetLog(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::custom_reaction< GotLog >,
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::transition< IsIncomplete, Incomplete >
> reactions;
  boost::statechart::result react(const AdvMap&);
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const MLogRec& logevt);
  boost::statechart::result react(const GotLog&);
};
{% endhighlight %}

* GetMissing状态
{% highlight string %}
struct GetMissing : boost::statechart::state< GetMissing, Peering >, NamedState {
  set<pg_shard_t> peer_missing_requested;

  explicit GetMissing(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< MLogRec >,
boost::statechart::transition< NeedUpThru, WaitUpThru >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const MLogRec& logevt);
};
{% endhighlight %}

* WaitUpThru状态
{% highlight string %}
struct WaitUpThru : boost::statechart::state< WaitUpThru, Peering >, NamedState {
  explicit WaitUpThru(my_context ctx);
  void exit();

  typedef boost::mpl::list <
boost::statechart::custom_reaction< QueryState >,
boost::statechart::custom_reaction< ActMap >,
boost::statechart::custom_reaction< MLogRec >
> reactions;
  boost::statechart::result react(const QueryState& q);
  boost::statechart::result react(const ActMap& am);
  boost::statechart::result react(const MLogRec& logrec);
};
{% endhighlight %}

* Incomplete状态
{% highlight string %}
struct Incomplete : boost::statechart::state< Incomplete, Peering>, NamedState {
  typedef boost::mpl::list <
boost::statechart::custom_reaction< AdvMap >,
boost::statechart::custom_reaction< MNotifyRec >
> reactions;
  explicit Incomplete(my_context ctx);
  boost::statechart::result react(const AdvMap &advmap);
  boost::statechart::result react(const MNotifyRec& infoevt);
  void exit();
};
{% endhighlight %}


### 4.3 PG状态机
在类PG的内部定义了类RecoveryState，该类RecoveryState的内部定义了PG的状态机RecoveryMachine和它的各种状态。
{% highlight string %}
class PG{
	class RecoveryState{
	
		class RecoveryMachine{
			RecoveryState *state;
			
		};
		
		RecoveryMachine machine;
		PG *pg;

		/// context passed in by state machine caller
		RecoveryCtx *orig_ctx;

		/// populated if we are buffering messages pending a flush
		boost::optional<BufferedRecoveryMessages> messages_pending_flush;

		/**
		* populated between start_handle() and end_handle(), points into
		* the message lists for messages_pending_flush while blocking messages
		* or into orig_ctx otherwise
		*/
		boost::optional<RecoveryCtx> rctx;
	
	}recovery_state;
};
{% endhighlight %}
在每个PG对象创建时，在构造函数里创建一个新的RecoveryState类的对象，并创建相应的RecoveryMachine类的对象，也就是创建了一个新的状态机。每个PG类对应一个独立的状态机来控制该PG的状态转换。
{% highlight string %}
class RecoveryState{
public:
	explicit RecoveryState(PG *pg)
	  : machine(this, pg), pg(pg), orig_ctx(0) {
		machine.initiate();
    }
};

PG::PG(OSDService *o, OSDMapRef curmap,
       const PGPool &_pool, spg_t p) :
	recovery_state(this){
}
{% endhighlight %}
上面machine.initiate()调用的是boost::statechart::state_machine中的initiate()方法。



<br />
<br />

**[参看]**

1. [ceph osd heartbeat 分析](https://blog.csdn.net/ygtlovezf/article/details/72330822)

2. [boost官网](https://www.boost.org/doc/)

3. [在线编译器](https://blog.csdn.net/weixin_39846364/article/details/112328477)

4. [boost在线编译器](http://cpp.sh/)
<br />
<br />
<br />

