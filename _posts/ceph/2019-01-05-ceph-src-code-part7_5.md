---
layout: post
title: ceph本地对象存储omap的实现
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章我们讲述一下ObjectStore中omap的实现。

<!-- more -->

## 1. omap的实现
omap的静态类图如下所示。类ObjectMap定义了omap的抽象接口，类DBObjectMap以KeyValueDB本地存储的方式实现了ObjectMap接口。

![ceph-chapter7-7](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_7.jpg)

目前实现KeyValueDB的本地存储分别为： Facebook开源的levelDB存储，对应类LevelDBStore；Google开源的RocksDB存储，对应类RocksDBStore实现；希捷kinetic客户端存储，对应的类KineticStore。默认采用的是LevelDB实现。

如下表7-1所示，LevelDB是一个key-value存储系统，它是一维的flat模式的KV存储。表7-2是对象存储，多个不同对象的多个不同属性的二维存储模式。

![ceph-chapter7-8](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_8.jpg)

二者如何映射呢？由于对象的名字是全局唯一的，属性在对象内也是唯一的，所以在LevelDB层面就可以用Object名字和属性的名字联合作为在LevelDB的key，这是能想到的比较直观的解决方式。

例如，object1的属性(key1,value1)保存在LevelDB如下：
<pre>
(object1_name + key1, value1)
</pre>
这个方法的一个缺点： 当一个对象有多个kv值时，Object1的name多次作为key存储，由于Object的name一般较长，这种存储方式浪费空间比较大。于是就提出了一种压缩的存储方法，也就是目前omap的存储方式。


### 1.1 omap存储
目前omap在LevelDB中存储分两步：

1） 在LevelDB中，保存键值对
<pre>
key: HOBJECT_TO_SEQ + ghobject_key(oid)
value: header
</pre>
其中HOBJECT_TO_SEQ是固定的前缀标识字符串，函数ghobject_key获取对应的对象唯一的key字符串。关于omap中用到的一些常量字符串，如下所示：
{% highlight string %}
const string DBObjectMap::USER_PREFIX = "_USER_";
const string DBObjectMap::XATTR_PREFIX = "_AXATTR_";
const string DBObjectMap::SYS_PREFIX = "_SYS_";
const string DBObjectMap::COMPLETE_PREFIX = "_COMPLETE_";
const string DBObjectMap::HEADER_KEY = "HEADER";
const string DBObjectMap::USER_HEADER_KEY = "USER_HEADER";
const string DBObjectMap::GLOBAL_STATE_KEY = "HEADER";
const string DBObjectMap::HOBJECT_TO_SEQ = "_HOBJTOSEQ_";

// Legacy
const string DBObjectMap::LEAF_PREFIX = "_LEAF_";
const string DBObjectMap::REVERSE_LEAF_PREFIX = "_REVLEAF_";
{% endhighlight %}

header保存对象在LevelDB中的唯一标识seq，以及支持快照的父对象的信息，同时保存了对象的collection和oid(这里冗余保存，因为其实我们通过key信息也可以获得）。
{% highlight string %}
struct _Header {
    uint64_t seq;                  //自己在leveldb中的序号
    uint64_t parent;               //父对象的序号
    uint64_t num_children;         //子对象的数量

    ghobject_t oid;                //对象标识      

    SequencerPosition spos;        //保存了当前的日志序号
};

struct SequencerPosition {
  uint64_t seq;  ///< seq
  uint32_t trans; ///< transaction in that seq (0-based)
  uint32_t op;    ///< op in that transaction (0-based)
}；
{% endhighlight %}

2） 对象的属性保存以下格式的键值对
{% highlight string %}
Key: USER_PREFIX + header_key(header->seq) + XATTR_PREFIX +key
Value: value(omap的值）
{% endhighlight %}
综上所述，设置和获取对象的属性，需要两步：先根据对象的oid，构造键(HOBJECT_TO_SEQ + ghobject_key(oid))，获取header；根据对象的header中的seq，拼接在LevelDB中的key值(USER_PREFIX + header_key(header->seq) + XATTR_PREFIX +key)，获取value值。

变量state用于保存KeyValueDB的全局状态，目前只有seq信息：
{% highlight string %}
struct State {
	static const __u8 CUR_VERSION = 3;
	__u8 v;                   //版本
	uint64_t seq;             //全局分配的seq

	// legacy is false when complete regions never used
	bool legacy;
};
{% endhighlight %}
函数write_state()用于每次分配seq后，把state信息写入LevelDB中，保存：
{% highlight string %}
SYS_PREFIX + GLOBAL_STATE_KEY -> state
{% endhighlight %}

### 1.2 omap的克隆
omap的克隆在10.2.10版本中有两种实现方法：
<pre>
int DBObjectMap::clone(const ghobject_t &oid,
		       const ghobject_t &target,
		       const SequencerPosition *spos);

int DBObjectMap::legacy_clone(const ghobject_t &oid,
		       const ghobject_t &target,
		       const SequencerPosition *spos);
</pre>
对于clone()方式，则是真正的进行数据的复制（会对header、以及对象的属性key/value都进行复制），这也是v10.2.10版本的默认方式。但这里我们介绍一下legacy_clone，其实现了no-copy方式的复制：

![ceph-chapter7-9](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter7_9.png)

* 当克隆一个新的对象时，会产生两个新的Header，原来的Header作为这两个新的Header的parent，这时候无论是原来的Object还是cloned Object在查询或者写操作时都会查询parent的情况，并且实现copy-on-write。

* 当读取一个子对象的属性时，如果子对象不存在该属性，需要去父对象获取

可以看出，omap的clone机制也实现了copy-on-write机制。

### 1.3 部分代码实现分析
###### 1.3.1 SequencerPosition
类SequencerPosition用来验证操作的顺序性，当操作执行时，后面的操作序号必须大于之前的操作序号。

{% highlight string %}
struct SequencerPosition {
  uint64_t seq;      //日志的序号
  uint32_t trans;    //一个日志内多个事务的序号
  uint32_t op;       //一个事务内多个op的序号
{% endhighlight %}

###### 1.3.2 lookup_create_map_header
本函数用于获取对象的header:

1) 首先调用函数_lookup_map_header查找对象的header
{% highlight string %}
a) 首先在caches里查找是否缓存

b) 调用底层KeyValueDB查找header
   int r = db->get(HOBJECT_TO_SEQ, map_header_key(oid), &out);

c) 如果查找成功，就返回header对象；如果不成功，返回一个新创建的header对象
{% endhighlight %}

2) 调用函数_generate_new_header来设置Header的字段，并调用函数write_state写入变量state
{% highlight string %}
int DBObjectMap::write_state(KeyValueDB::Transaction _t) {
  assert(header_lock.is_locked_by_me());
  dout(20) << "dbobjectmap: seq is " << state.seq << dendl;
  KeyValueDB::Transaction t = _t ? _t : db->get_transaction();
  bufferlist bl;
  state.encode(bl);
  map<string, bufferlist> to_write;
  to_write[GLOBAL_STATE_KEY] = bl;
  t->set(SYS_PREFIX, to_write);
  return _t ? 0 : db->submit_transaction(t);
}
{% endhighlight %}

3) 调用函数set_map_header，把新的header设置到LevelDB中

###### 1.3.3 get_xattrs
本函数用于获取对象的属性，实现如下：

1） 首先获取对应的Header头部；

2) 调用db设置具体的数据
{% highlight string %}
int DBObjectMap::get_xattrs(const ghobject_t &oid,
			    const set<string> &to_get,
			    map<string, bufferlist> *out)
{
  MapHeaderLock hl(this, oid);
  Header header = lookup_map_header(hl, oid);
  if (!header)
    return -ENOENT;
  return db->get(xattr_prefix(header), to_get, out);
}
{% endhighlight %}

###### 1.3.4 set_keys
本函数用于设置对象的属性，实现如下：

1） 获取KeyValueDB::Transaction的一个事务
{% highlight string %}
KeyValueDB::Transaction t = db->get_transaction();
{% endhighlight %}

2) 先获取对象的Header
{% highlight string %}
MapHeaderLock hl(this, oid);
Header header = lookup_create_map_header(hl, oid, t);
if (!header)
	return -EINVAL;
if (check_spos(oid, header, spos))
	return 0;
{% endhighlight %}

3) 先调用事务set函数设置属性
{% highlight string %}
t->set(user_prefix(header), set);
{% endhighlight %}

4) 调用db提交事务
{% highlight string %}
return db->submit_transaction(t);
{% endhighlight %}

<br />
<br />

**[参看]**

1. [解析Ceph: 存储引擎实现之一–FileStore](https://www.talkwithtrend.com/Article/176745)

2. [CEPH OBJECTSTORE API介绍](http://ceph.org.cn/2016/05/02/ceph-objectstore-api%E4%BB%8B%E7%BB%8D/)


<br />
<br />
<br />

