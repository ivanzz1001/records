---
layout: post
title: ceph数据一致性浅析
tags:
- ceph
categories: ceph
description: ceph数据一致性浅析
---



ceph作为一个分布式存储系统，保证数据的一致性是很重要的一个方面。ceph的数据存储是以PG为单位来进行的，而数据的一致性也是通过PG的相关操作来实现的。本章涉及到的内容包括：

* PG的创建过程

* Ceph Peering机制

下面我们先给出一幅PG状态机的总体状态转换图：



<!-- more -->

## 1. PG的创建过程

PG的创建是由monitor节点发起的，形成请求message发送给osd，在OSD上创建PG。

### 1.1 monitor节点处理

1） **register_new_pgs()**

在monitor中由PGMonitor发现是否创建了pool， pool中是否存在PG需要进行创建。首先来看函数PGMonitor::register_new_pgs()
{% highlight string %}
bool PGMonitor::register_new_pgs()
{
    ....
    // first pgs in this pool
    bool new_pool = pg_map.pg_pool_sum.count(poolid) == 0;

    for (ps_t ps = 0; ps < pool.get_pg_num(); ps++)                      //标记位置1
    {
        pg_t pgid(ps, poolid, -1);                                       //标记位置2
        if (pg_map.pg_stat.count(pgid))                                  //标记位置3
        {
	        dout(20) << "register_new_pgs  have " << pgid << dendl;
	        continue;
        }
        created++;
        register_pg(osdmap, pgid, pool.get_last_change(), new_pool);     //标记位置4
    }   
}
{% endhighlight %}
标记位置1：循环遍历当前这个pool中的所有PG

标记位置2: 根据当前这个pool中PG的序号和pool的id，形成pgid。(pgid就是用来统计哪个pool中的第几个pg而已，使用m_seed作为pg序号，m_pool作为pool序号，如下所示：）
<pre>
struct pg_t {
  uint64_t m_pool;
  uint32_t m_seed;
  int32_t m_preferred;

  ....
};
</pre>

标记位置3： pg_map中统计了所有的PG，如果发现当前的PG不在pg_map中，说明这个pg是需要被创建的

标记位置4： 使用register_pg()函数开始处理申请这个PG


2） **申请PG**

register_pg()函数开始对PG的申请进行处理，这时已经有了pgid和pool的信息：
{% highlight string %}
void PGMonitor::register_pg(OSDMap *osdmap,
                            pg_t pgid, epoch_t epoch,
                            bool new_pool)
{
     ....

     pg_stat_t &stats = pending_inc.pg_stat_updates[pgid];       //标记位置1
     stats.state = PG_STATE_CREATING;                            //标记位置2
     stats.created = epoch;
     stats.parent = parent;
     stats.parent_split_bits = split_bits;
     stats.mapping_epoch = epoch;


     //标记位置3
     osdmap->pg_to_up_acting_osds(
       pgid,
       &stats.up,
       &stats.up_primary,
       &stats.acting,
       &stats.acting_primary);
}


void OSDMap::_pg_to_up_acting_osds(const pg_t& pg, vector<int> *up, int *up_primary,
                                   vector<int> *acting, int *acting_primary) const
{
    
}
{% endhighlight %}
标记位置1： 将刚创建的pgid统计到pending_inc.pg_stat_updates结构中

标记位置2： 设置这个PG的状态为PG_STATE_CREATING

标记位置3： 将PG映射到OSD上，这时候需要osdmap，最终通过_pg_to_up_acting_osds()函数完成映射

接下来会将这个```pending_inc```进行打包，然后推行propose_pending()，开始提议，等待完成最后推行（这其实是paxos协议的一部分）。

3） **propose_pending()**

在check_osd_map()函数中首先执行了register_new_pgs()，之后将映射好的PG通过propose_pending()完成推行，形成一致的PGMap。(说明： 类PGMonitor继承了PaxosService）:
{% highlight string %}
void PGMonitor::check_osd_map(epoch_t epoch)
{
    if (map_pg_creates())
        propose = true;
    if (register_new_pgs())
        propose = true;

    if ((need_check_down_pgs || !need_check_down_pg_osds.empty()) && check_down_pgs())
        propose = true;

    if (propose)
        propose_pending();
}

void PaxosService::propose_pending()
{
    paxos->queue_pending_finisher(new C_Committed(this));
    paxos->trigger_propose();               //标记位置1
}
{% endhighlight %}
标记位置1： 触发提议的表决

4） **触发提议表决**
{% highlight string %}
bool Paxos::trigger_propose()
{
    if (is_active()) {
       dout(10) << __func__ << " active, proposing now" << dendl;
       propose_pending();
       return true;
    } else {
       dout(10) << __func__ << " not active, will propose later" << dendl;
       return false;
    }
}

void Paxos::propose_pending()
{
    ...
    state = STATE_UPDATING;
    begin(bl);
}
{% endhighlight %}
上面完成投票表决之后，就会调用refresh_from_paxos()完成提议的推行。

5) **进行提议的推行**
{% highlight string %}
void Monitor::refresh_from_paxos(bool *need_bootstrap)
{
    for (int i = 0; i < PAXOS_NUM; ++i) {
        paxos_service[i]->refresh(need_bootstrap);      //标记位置1
    }
    for (int i = 0; i < PAXOS_NUM; ++i) {
       paxos_service[i]->post_refresh();
    }
}

void PaxosService::refresh(bool *need_bootstrap)
{
     update_from_paxos(need_bootstrap);
}

void PGMonitor::update_from_paxos(bool *need_bootstrap)
{
    PGMap::Incremental inc;
    try {
        bufferlist::iterator p = bl.begin();
        inc.decode(p);                             //标记位置1
    } catch (const std::exception &e)   {
        dout(0) << "update_from_paxos: error parsing "
         << "incremental update: " << e.what() << dendl;
        assert(0 == "update_from_paxos: error parsing incremental update");
        return;
    }

    pg_map.apply_incremental(g_ceph_context, inc);          //标记位置2
}
{% endhighlight %}
决策推行函数update_from_paxos()，在这里会根据仲裁决定，然后处理结果，上面说道已经推行了创建的PG。

标记位置1： 重新解析pgid;

标记位置2： 将这个决议交给pg_map进行处理，调用PGMap::apply_incremental()

6) **发送请求到OSD以创建PG**

我们在上面Monitor::refresh_from_paxos()后面会调用post_refresh()，此函数会最后向对应的OSD发出创建PG的命令：
{% highlight string %}
void PaxosService::post_refresh()
{
  post_paxos_update();
}

void PGMonitor::post_paxos_update()
{
  if (mon->osdmon()->osdmap.get_epoch()) {
    send_pg_creates();
  }
}

void PGMonitor::send_pg_creates()
{
    if (osdmap.is_up(osd))
       send_pg_creates(osd, NULL, 0);
}

epoch_t PGMonitor::send_pg_creates(int osd, Connection *con, epoch_t next)
{
    m = new MOSDPGCreate(pg_map.last_osdmap_epoch);    


    if (con) {
        con->send_message(m);                   
    } else {
       assert(mon->osdmon()->osdmap.is_up(osd));
       mon->messenger->send_message(m, mon->osdmon()->osdmap.get_inst(osd));
    }
}
{% endhighlight %}
上面创建了一个```MOSDPGCreate```对象，然后向其发送消息进行PG的创建。```MOSDPGCreate```实现了Message类：
{% highlight string %}
struct MOSDPGCreate : public Message {
  MOSDPGCreate()
    : Message(MSG_OSD_PG_CREATE, HEAD_VERSION, COMPAT_VERSION) {}
  MOSDPGCreate(epoch_t e)
    : Message(MSG_OSD_PG_CREATE, HEAD_VERSION, COMPAT_VERSION),
      epoch(e) { }
};
{% endhighlight %}

### 1.2 osd节点处理
osd这时收到一个消息，根据消息命令字```MSG_OSD_PG_CREATE```，发现这是一个创建PG的消息，然后交给handle_pg_create()进行处理：
{% highlight string %}
void OSD::dispatch_op(OpRequestRef op)
{
  switch (op->get_req()->get_type()) {

  case MSG_OSD_PG_CREATE:
    handle_pg_create(op);
}

void OSD::handle_pg_create(OpRequestRef op)
{
    MOSDPGCreate *m = (MOSDPGCreate*)op->get_req();

    for (map<pg_t,pg_create_t>::iterator p = m->mkpg.begin();p != m->mkpg.end(); ++p, ++ci) 
    {
         pg_t on = p->first;             //标记位置1

         // is it still ours?
        vector<int> up, acting;
        int up_primary = -1;
        int acting_primary = -1;
        osdmap->pg_to_up_acting_osds(on, &up, &up_primary, &acting, &acting_primary);   // 标记位置2
        int role = osdmap->calc_pg_role(whoami, acting, acting.size());

        ...

        spg_t pgid;
        bool mapped = osdmap->get_primary_shard(on, &pgid);            


        //标记位置3
        handle_pg_peering_evt(pgid,history,pi,osdmap->get_epoch(),
              PG::CephPeeringEvtRef(new PG::CephPeeringEvt(osdmap->get_epoch(),osdmap->get_epoch(),PG::NullEvt()))
        );                 

    }

}
{% endhighlight %}

标记位置1： 在消息中恢复出所要创建的PG到on上

标记位置2： 根据PG编号，重新计算该PG所映射到的OSD

标记位置3： 实际创建PG。因为Monitor并不会给PG的从OSD发送消息来创建该PG，而是由该主OSD上的PG在Peering过程中创建，所以最先创建的肯定是primary PG，从PG的创建也是在peering过程中通过handler_pg_peering_evt来完成的。

说明：
<pre>
PG的加载： 当OSD重启时，调用函数OSD::init()，该函数调用load_pgs函数加载已经存在的PG，其处理过程和创建PG的过程相似
</pre>

## 2. PG创建后状态机的状态转换
如下图10-2为PG总体状态转换图的简化版： 状态Peering、Active、ReplicaActive4的内部状态没有添加进去。

![ceph-pg-peering](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_pg_peering1.jpg)

通过该图可以了解PG的高层状态转换过程，如下所示：

1) 当PG创建后，同时在该类内部创建了一个属于该PG的RecoveryMachine类型的状态机，该状态机的初始化状态为默认初始化状态Initial

2) 在PG创建后，调用函数pg->handle_create(&rctx)来给状态机投递事件：
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
由以上代码可知： 该函数首先向RecoveryMachine投递了Initialize类型的事件。由上图10-2可知，状态机在RecoveryMachine/Initial状态接收到Initialize类型的事件后直接转移到Reset状态。其次，向RecoveryMachine投递了ActMap事件。

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

4） 进入状态RecoveryMachine/Started后，就进入RecoveryMachine





<br />
<br />

**[参看]**

1. [PG 的创建过程](http://www.it610.com/article/2077868.htm)

2. [ceph——创建pg](https://blog.csdn.net/w007d/article/details/80906250)

3. [解析Ceph: 恢复与数据一致性](https://blog.csdn.net/xingkong_678/article/details/51485077)

4. [错误的状况下，ceph IO 的一致性如何保证](https://blog.csdn.net/guzyguzyguzy/article/details/53608829)

5. [ceph数据一致性浅析](https://max.book118.com/html/2017/0618/116348354.shtm)

6. [Ceph源码解析：PG peering](https://www.cnblogs.com/chenxianpao/p/5565286.html)

7. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)

8. [Ceph中一些PG相关的状态说明和基本概念说明、故障模拟](https://blog.csdn.net/pansaky/article/details/86700301)

9. [ceph OSD读写流程](http://www.sysnote.org/2015/11/25/ceph-osd-rw2/)

9. [mongoose库](https://github.com/cesanta/mongoose)

<br />
<br />
<br />

