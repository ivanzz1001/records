---
layout: post
title: PG info介绍
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


PGInfo存在于PG的整个生命周期中，其在对象数据的写入、数据恢复、PG Peering过程中均发挥重要的作用。本章试图研究pg info在整个PG生命周期中的变化过程，从而对PG及PGInfo有一个更深入的理解。
{% highlight string %}
class PG : DoutPrefixProvider {
public:
	// pg state
	pg_info_t        info;
};

class ReplicatedPG : public PG, public PGBackend::Listener {
public:
	const pg_info_t &get_info() const {
		return info;
	}
};

class PGBackend {
public:
	Listener *parent;
	Listener *get_parent() const { return parent; }

	PGBackend(Listener *l, ObjectStore *store, coll_t coll, ObjectStore::CollectionHandle &ch) :
		store(store),
		coll(coll),
		ch(ch),
		parent(l) {}

	const pg_info_t &get_info() { return get_parent()->get_info(); }
};
{% endhighlight %}

从上面的代码可知，ReplicatedPG以及PGBackend中使用到的PGInfo均为PG::info。
<!-- more -->






<br />
<br />

**[参看]**



<br />
<br />
<br />

