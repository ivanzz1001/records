---
layout: post
title: OSD启动流程
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本章我们简单介绍一下OSD启动的几个主要的流程。

<!-- more -->


## 1. 集群信息相关初始化
在启动OSD之前，首先需要创建相应的ObjectStore对象(src/ceph_osd.cc):
{% highlight string %}
ObjectStore *store = ObjectStore::create(g_ceph_context,
	store_type,
	g_conf->osd_data,
	g_conf->osd_journal,
	g_conf->osd_os_flags);
{% endhighlight %}
对于Jewel版本，默认使用的是filestore。

之后调用OSD::peek_meta()来获取superblock相关的信息：
{% highlight string %}
int OSD::peek_meta(ObjectStore *store, std::string& magic,
		   uuid_d& cluster_fsid, uuid_d& osd_fsid, int& whoami)
{
  string val;

  int r = store->read_meta("magic", &val);
  if (r < 0)
    return r;
  magic = val;

  r = store->read_meta("whoami", &val);
  if (r < 0)
    return r;
  whoami = atoi(val.c_str());

  r = store->read_meta("ceph_fsid", &val);
  if (r < 0)
    return r;
  r = cluster_fsid.parse(val.c_str());
  if (!r)
    return -EINVAL;

  r = store->read_meta("fsid", &val);
  if (r < 0) {
    osd_fsid = uuid_d();
  } else {
    r = osd_fsid.parse(val.c_str());
    if (!r)
      return -EINVAL;
  }

  return 0;
}
{% endhighlight %}
我们来看一下OSD根目录下的meta文件如下：
<pre>
# ls
ceph_fsid  current  fsid  keyring  magic  ready  store_version  superblock  type  whoami
</pre>

之后会创建相应的Messenger进行通信：
{% highlight string %}
Messenger *ms_public = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "client",
			getpid());
Messenger *ms_cluster = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "cluster",
			getpid(), CEPH_FEATURES_ALL);
Messenger *ms_hbclient = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "hbclient",
			getpid());
Messenger *ms_hb_back_server = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "hb_back_server",
			getpid());
Messenger *ms_hb_front_server = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "hb_front_server",
			getpid());
Messenger *ms_objecter = Messenger::create(g_ceph_context, g_conf->ms_type,
			entity_name_t::OSD(whoami), "ms_objecter",
			getpid());
{% endhighlight %}

之后会创建MonClient来与Monitor进行通信。Monitor是基于Paxos协议实现的一个典型的分布式系统，在其上会保存cluster_map、OSD_map、PG_map、Monitor_map以及MDS_map等相关信息。
{% highlight string %}
MonClient mc(g_ceph_context);
if (mc.build_initial_monmap() < 0)
	return -1;
{% endhighlight %}
在build_initial_monmap()函数中会读取所有的monitor的地址信息。

## 2. OSD的启动
在src/ceph_osd.cc文件中，通过如下代码段完成OSD的启动：
{% highlight string %}
 osd = new OSD(g_ceph_context,
                store,
                whoami,
                ms_cluster,
                ms_public,
                ms_hbclient,
                ms_hb_front_server,
                ms_hb_back_server,
                ms_objecter,
                &mc,
                g_conf->osd_data,
                g_conf->osd_journal);

  int err = osd->pre_init();
  if (err < 0) {
    derr << TEXT_RED << " ** ERROR: osd pre_init failed: " << cpp_strerror(-err)
	 << TEXT_NORMAL << dendl;
    return 1;
  }

  ms_public->start();
  ms_hbclient->start();
  ms_hb_front_server->start();
  ms_hb_back_server->start();
  ms_cluster->start();
  ms_objecter->start();

  // start osd
  err = osd->init();
{% endhighlight %}
上面首先创建一个OSD对象，然后调用pre_init()以及init()完成OSD的初始化。

### 2.1 OSD::pre_init()
OSD::pre_init()函数实现如下：
{% highlight string %}
int OSD::pre_init()
{
  Mutex::Locker lock(osd_lock);
  if (is_stopping())
    return 0;

  if (store->test_mount_in_use()) {
    derr << "OSD::pre_init: object store '" << dev_path << "' is "
         << "currently in use. (Is ceph-osd already running?)" << dendl;
    return -EBUSY;
  }

  cct->_conf->add_observer(this);
  return 0;
}
{% endhighlight %}
pre_init()函数实现较为简单，主要检测当前ObjectStore是否被占用，以及对conf添加observer。

### 2.2 OSD::init()
OSD::init()是完成OSD初始化的主函数：
{% highlight string %}
int OSD::init(){
	...

	store->mount();

	...

	read_superblock();

	...

	// make sure snap mapper object exists
	if (!store->exists(coll_t::meta(), OSD::make_snapmapper_oid())) {
		dout(10) << "init creating/touching snapmapper object" << dendl;
		ObjectStore::Transaction t;
		t.touch(coll_t::meta(), OSD::make_snapmapper_oid());
		r = store->apply_transaction(service.meta_osr.get(), std::move(t));
		if (r < 0)
		goto out;
	}

	...

	osdmap = get_map(superblock.current_epoch);

	...

	clear_temp_objects();

	...

	load_pgs();

	...

	monc->set_want_keys(CEPH_ENTITY_TYPE_MON | CEPH_ENTITY_TYPE_OSD);
	r = monc->init();

	...
	service.init();
	service.publish_map(osdmap);
	service.publish_superblock(superblock);

	...

	consume_map();
	peering_wq.drain();

	...

	// subscribe to any pg creations
	monc->sub_want("osd_pg_creates", last_pg_create_epoch, 0);

	...
	
	monc->renew_subs();

	start_boot();

	...
	
}
{% endhighlight %}

如下我们会对其中一些较为关键的步骤进行详细说明。


###### 2.2.1 store->mount()函数
store->mount()实际调用的是FileStore::mount()函数：
{% highlight string %}
int FileStore::mount(){
}
{% endhighlight %}
mount()函数主要完成相关数据的检查，读取/var/lib/ceph/osd/ceph-0/current/commit_op_seq中相关的提交信息，以及完成相关backend线程的启动。

###### 2.2.2 read_superblock()
本函数用于读取superblock相关信息：
{% highlight string %}
#define OSD_SUPERBLOCK_GOBJECT ghobject_t(hobject_t(sobject_t(object_t("osd_superblock"), 0)))

int OSD::read_superblock()
{
  bufferlist bl;
  int r = store->read(coll_t::meta(), OSD_SUPERBLOCK_GOBJECT, 0, 0, bl);
  if (r < 0)
    return r;

  bufferlist::iterator p = bl.begin();
  ::decode(superblock, p);

  dout(10) << "read_superblock " << superblock << dendl;

  return 0;
}
{% endhighlight %}
经分析其最终会读取到meta目录下的superblock文件：：
<pre>
# pwd
/var/lib/ceph/osd/ceph-0/current/meta
# ls 
DIR_2  DIR_5  DIR_7  DIR_8  osd\usuperblock__0_23C2FCDE__none  snapmapper__0_A468EC03__none
</pre>

读取出来的OSDSuperblock结构如下：
{% highlight string %}
class OSDSuperblock {
public:
  uuid_d cluster_fsid, osd_fsid;
  int32_t whoami;                    // my role in this fs.
  epoch_t current_epoch;             // most recent epoch
  epoch_t oldest_map, newest_map;    // oldest/newest maps we have.
  double weight;

  CompatSet compat_features;

  // last interval over which i mounted and was then active
  epoch_t mounted;                  // last epoch i mounted
  epoch_t clean_thru;               // epoch i was active and clean thru

  OSDSuperblock() : 
    whoami(-1), 
    current_epoch(0), oldest_map(0), newest_map(0), weight(0),
    mounted(0), clean_thru(0) {
  }
};
{% endhighlight %}

注： 在每一次OSD map发生变动时都会对superblock进行修改，请参看OSD::handle_osd_map()，后面在适当的章节我们会仔细分析一下此函数的实现。

###### 2.2.3 检查snap mapper对象是否存在
{% highlight string %}
// make sure snap mapper object exists
if (!store->exists(coll_t::meta(), OSD::make_snapmapper_oid())) {
	dout(10) << "init creating/touching snapmapper object" << dendl;
	ObjectStore::Transaction t;
	t.touch(coll_t::meta(), OSD::make_snapmapper_oid());
	r = store->apply_transaction(service.meta_osr.get(), std::move(t));
	if (r < 0)
 		 goto out;
}
{% endhighlight %}
这里实际是检查meta目录下的snap mapper文件是否存在：
<pre>
# pwd
/var/lib/ceph/osd/ceph-0/current/meta
# ls 
DIR_2  DIR_5  DIR_7  DIR_8  osd\usuperblock__0_23C2FCDE__none  snapmapper__0_A468EC03__none
</pre>

###### 2.2.4 get_map()
通过如下代码片段来获取当前的osd map信息：
{% highlight string %}
// load up "current" osdmap
assert_warn(!osdmap);
if (osdmap) {
	derr << "OSD::init: unable to read current osdmap" << dendl;
	r = -EINVAL;
	goto out;
}
osdmap = get_map(superblock.current_epoch);
check_osdmap_features(store);
{% endhighlight %}
因为OSD的恢复严重依赖于osd map信息，因此这里要加载```superblock.current_epoch```时的OSD Map信息。下面简单来看一下相关实现：
{% highlight string %}
// osd map cache (past osd maps)
OSDMapRef get_map(epoch_t e) {
	return service.get_map(e);
}

OSDMapRef get_map(epoch_t e) {
	OSDMapRef ret(try_get_map(e));
	assert(ret);
	return ret;
}

OSDMapRef OSDService::try_get_map(epoch_t epoch)
{
	Mutex::Locker l(map_cache_lock);
	OSDMapRef retval = map_cache.lookup(epoch);
	if (retval) {
		dout(30) << "get_map " << epoch << " -cached" << dendl;
		return retval;
	}
	
	OSDMap *map = new OSDMap;
	if (epoch > 0) {
		dout(20) << "get_map " << epoch << " - loading and decoding " << map << dendl;
		bufferlist bl;
		if (!_get_map_bl(epoch, bl)) {
			delete map;
			return OSDMapRef();
		}
		map->decode(bl);
	} else {
		dout(20) << "get_map " << epoch << " - return initial " << map << dendl;
	}
	return _add_map(map);
}

bool OSDService::_get_map_bl(epoch_t e, bufferlist& bl)
{
	bool found = map_bl_cache.lookup(e, &bl);
	if (found)
		return true;
	found = store->read(coll_t::meta(),
		OSD::get_osdmap_pobject_name(e), 0, 0, bl) >= 0;

	if (found)
		_add_map_bl(e, bl);
	return found;
}

  static ghobject_t get_osdmap_pobject_name(epoch_t epoch) {
    char foo[20];
    snprintf(foo, sizeof(foo), "osdmap.%d", epoch);
    return ghobject_t(hobject_t(sobject_t(object_t(foo), 0)));
  }
{% endhighlight %}
从这里我们可以看到其实际是从map_cache中读取，如果读取不到就调用_get_map_bl()从OSD的meta目录下来获取。但是我们查看meta目录：
<pre>
# pwd
/var/lib/ceph/osd/ceph-0/current/meta
# ls
DIR_2  DIR_5  DIR_7  DIR_8  osd\usuperblock__0_23C2FCDE__none  snapmapper__0_A468EC03__none

# cd DIR_8
# ls
DIR_0  DIR_1  DIR_2  DIR_3  DIR_4  DIR_5  DIR_6  DIR_7  DIR_8  DIR_9  DIR_A  DIR_B  DIR_C  DIR_D  DIR_E  DIR_F

# cd DIR_0
# ls
osdmap.8595__0_0A3F2D08__none  osdmap.8658__0_0A3ED408__none  osdmap.8720__0_0A3E9F08__none  osdmap.8793__0_0A3E4608__none  osdmap.8856__0_0A3E0908__none  osdmap.8919__0_0A3E3008__none  osdmap.8977__0_0A3DEC08__none  osdmap.9031__0_0A3D6008__none
osdmap.8614__0_0A3F3708__none  osdmap.8672__0_0A3EE308__none  osdmap.8735__0_0A3EAA08__none  osdmap.8812__0_0A3E6808__none  osdmap.8870__0_0A3E0408__none  osdmap.8933__0_0A3DCF08__none  osdmap.8991__0_0A3DFB08__none  osdmap.9046__0_0A3D7F08__none
osdmap.8629__0_0A3EC208__none  osdmap.8687__0_0A3EFE08__none  osdmap.8764__0_0A3EBC08__none  osdmap.8827__0_0A3E6708__none  osdmap.8885__0_0A3E1308__none  osdmap.8948__0_0A3DDA08__none  osdmap.9002__0_0A3D5E08__none  osdmap.9060__0_0A3D0A08__none
osdmap.8643__0_0A3ED908__none  osdmap.8706__0_0A3E8008__none  osdmap.8779__0_0A3E4B08__none  osdmap.8841__0_0A3E7208__none  osdmap.8904__0_0A3E2508__none  osdmap.8962__0_0A3DD108__none  osdmap.9017__0_0A3D5508__none  osdmap.9075__0_0A3D0108__none

# cd ../DIR_2
# ls
osdmap.8586__0_0A3F2A28__none  osdmap.8678__0_0A3EE728__none  osdmap.8755__0_0A3EA528__none  osdmap.8832__0_0A3E7B28__none  osdmap.8924__0_0A3E3428__none  osdmap.8997__0_0A3DFF28__none  osdmap.9066__0_0A3D0E28__none
osdmap.8605__0_0A3F3C28__none  osdmap.8692__0_0A3EF228__none  osdmap.8784__0_0A3E4F28__none  osdmap.8847__0_0A3E7628__none  osdmap.8939__0_0A3DC328__none  osdmap.9008__0_0A3D5228__none  osdmap.9080__0_0A3D0528__none
osdmap.8634__0_0A3EC628__none  osdmap.8711__0_0A3E8428__none  osdmap.8799__0_0A3E5A28__none  osdmap.8861__0_0A3E0D28__none  osdmap.8953__0_0A3DDE28__none  osdmap.9022__0_0A3D6928__none  osdmap.9095__0_0A3D1028__none
osdmap.8649__0_0A3EDD28__none  osdmap.8726__0_0A3E9328__none  osdmap.8803__0_0A3E5128__none  osdmap.8876__0_0A3E1828__none  osdmap.8968__0_0A3DD528__none  osdmap.9037__0_0A3D6428__none
osdmap.8663__0_0A3EE828__none  osdmap.8740__0_0A3EAE28__none  osdmap.8818__0_0A3E6C28__none  osdmap.8890__0_0A3E1728__none  osdmap.8982__0_0A3DE028__none  osdmap.9051__0_0A3D7328__none
</pre>
从上面我们可以看到，在OSD上也保留了相应的OSD map信息。

>注：对于ghobject_t对象，通过LFNIndex可以映射为多级目录。这也就是我们上面看到的meta目录下的多个DIR目录

###### 2.2.5 clear_temp_objects()


<br />
<br />

**[参看]**

1. [Jewel OSD进程启动处理流程](http://m.blog.chinaunix.net/uid-22954220-id-5758878.html)

<br />
<br />
<br />

