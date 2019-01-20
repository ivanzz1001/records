---
layout: post
title: crushmap算法详解-1
tags:
- ceph
- crushmap
categories: ceph
description: crushmap详解
---

本文从一个具体的crushmap.txt文件入手，讲述其通过crushtool生成crushmap.bin文件的过程。
<!-- more -->


## 1. 生成crushmap.txt
我们从一个现有的ceph集群中导出crushmap.txt:
{% highlight string %}
ceph osd getcrushmap -o ./crushmap.bin

crushtool -d ./crushmap.bin -o ./crushmap.txt
{% endhighlight %}
生成的crushmap.txt如下：
<pre>
[root@localhost ceph-test]# cat crushmap.txt 
# begin crush map
tunable choose_local_tries 0
tunable choose_local_fallback_tries 0
tunable choose_total_tries 50
tunable chooseleaf_descend_once 1
tunable straw_calc_version 1

# devices
device 0 osd.0
device 1 osd.1
device 2 osd.2
device 3 osd.3
device 4 osd.4
device 5 osd.5
device 6 osd.6
device 7 osd.7
device 8 osd.8

# types
type 0 osd
type 1 host
type 2 chassis
type 3 rack
type 4 row
type 5 pdu
type 6 pod
type 7 room
type 8 datacenter
type 9 region
type 10 root
type 11 osd-domain
type 12 host-domain
type 13 replica-domain
type 14 failure-domain

# buckets
host node7-1 {
        id -2           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.0 weight 0.150
        item osd.1 weight 0.150
        item osd.2 weight 0.150
}
rack rack-01 {
        id -3           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-1 weight 0.450
}
host node7-2 {
        id -4           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.3 weight 0.150
        item osd.4 weight 0.150
        item osd.5 weight 0.150
}
rack rack-02 {
        id -5           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-2 weight 0.450
}
host node7-3 {
        id -6           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.6 weight 0.150
        item osd.7 weight 0.150
        item osd.8 weight 0.150
}
rack rack-03 {
        id -7           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-3 weight 0.450
}
root default {
        id -1           # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item rack-01 weight 0.450
        item rack-02 weight 0.450
        item rack-03 weight 0.450
}
host-domain host-group-0-rack-01 {
        id -8           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-1 weight 0.450
}
host-domain host-group-0-rack-02 {
        id -11          # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-2 weight 0.450
}
host-domain host-group-0-rack-03 {
        id -12          # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-3 weight 0.450
}
replica-domain replica-0 {
        id -9           # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item host-group-0-rack-01 weight 0.450
        item host-group-0-rack-02 weight 0.450
        item host-group-0-rack-03 weight 0.450
}
failure-domain sata-00 {
        id -10          # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item replica-0 weight 1.350
}

# rules
rule replicated_ruleset {
        ruleset 0
        type replicated
        min_size 1
        max_size 10
        step take default
        step choose firstn 0 type osd
        step emit
}
rule replicated_rule-5 {
        ruleset 5
        type replicated
        min_size 1
        max_size 10
        step take sata-00
        step choose firstn 1 type replica-domain
        step chooseleaf firstn 0 type host-domain
        step emit
}

# end crush map
</pre>

如上所示，一个crushmap包括有5个部分：tunable、devices、types、buckets、rules。 其中tunable是一些可调参数的设置，在后续rules生成相关PG映射时会用到；devices为当前ceph存储集群中的osd节点； types为当前crushmap中所定义的bucket类型；buckets为节点的一种组织方式，其类型可以为types中的任何一种；rules指定相应的规则。


## 2. 生成crushmap.bin
我们将上面的crushmap.txt重新生成crushmap.bin，然后直观的看一下该crushmap的层级结构：
{% highlight string %}
crushtool -c crushmap.txt -o crushmap-new.bin

crushtool --test -i crushmap-new.bin --tree
{% endhighlight %}

显示结果如下：
<pre>
[root@localhost ceph-test]# crushtool --test -i crushmap-new.bin --tree
WARNING: no output selected; use --output-csv or --show-X
ID      WEIGHT  TYPE NAME
-10     1.34999 failure-domain sata-00
-9      1.34999         replica-domain replica-0
-8      0.45000                 host-domain host-group-0-rack-01
-2      0.45000                         host node7-1
0       0.14999                                 osd.0
1       0.14999                                 osd.1
2       0.14999                                 osd.2
-11     0.45000                 host-domain host-group-0-rack-02
-4      0.45000                         host node7-2
3       0.14999                                 osd.3
4       0.14999                                 osd.4
5       0.14999                                 osd.5
-12     0.45000                 host-domain host-group-0-rack-03
-6      0.45000                         host node7-3
6       0.14999                                 osd.6
7       0.14999                                 osd.7
8       0.14999                                 osd.8
-1      1.34999 root default
-3      0.45000         rack rack-01
-2      0.45000                 host node7-1
0       0.14999                         osd.0
1       0.14999                         osd.1
2       0.14999                         osd.2
-5      0.45000         rack rack-02
-4      0.45000                 host node7-2
3       0.14999                         osd.3
4       0.14999                         osd.4
5       0.14999                         osd.5
-7      0.45000         rack rack-03
-6      0.45000                 host node7-3
6       0.14999                         osd.6
7       0.14999                         osd.7
8       0.14999                         osd.8
</pre>


## 3. crushmap的内部数据结构
crushmap内部是由一个```struct crush_map```的数据结构来表示的：
<pre>
struct crush_bucket {
	__s32 id;        /* this'll be negative */
	__u16 type;      /* non-zero; type=0 is reserved for devices */
	__u8 alg;        /* one of CRUSH_BUCKET_* */
	__u8 hash;       /* which hash function to use, CRUSH_HASH_* */
	__u32 weight;    /* 16-bit fixed point */
	__u32 size;      /* num items */
	__s32 *items;

	/*
	 * cached random permutation: used for uniform bucket and for
	 * the linear search fallback for the other bucket types.
	 */
	__u32 perm_x;  /* @x for which *perm is defined */
	__u32 perm_n;  /* num elements of *perm that are permuted/defined */
	__u32 *perm;
};

struct crush_rule {
	__u32 len;
	struct crush_rule_mask mask;
	struct crush_rule_step steps[0];
};

/*
 * CRUSH map includes all buckets, rules, etc.
 */
struct crush_map {
	struct crush_bucket **buckets;
	struct crush_rule **rules;

	__s32 max_buckets;
	__u32 max_rules;
	__s32 max_devices;

	/* choose local retries before re-descent */
	__u32 choose_local_tries;
	/* choose local attempts using a fallback permutation before
	 * re-descent */
	__u32 choose_local_fallback_tries;
	/* choose attempts before giving up */ 
	__u32 choose_total_tries;
	/* attempt chooseleaf inner descent once for firstn mode; on
	 * reject retry outer descent.  Note that this does *not*
	 * apply to a collision: in that case we will retry as we used
	 * to. */
	__u32 chooseleaf_descend_once;

	/* if non-zero, feed r into chooseleaf, bit-shifted right by (r-1)
	 * bits.  a value of 1 is best for new clusters.  for legacy clusters
	 * that want to limit reshuffling, a value of 3 or 4 will make the
	 * mappings line up a bit better with previous mappings. */
	__u8 chooseleaf_vary_r;

	/*
	 * version 0 (original) of straw_calc has various flaws.  version 1
	 * fixes a few of them.
	 */
	__u8 straw_calc_version;

	/*
	 * allowed bucket algs is a bitmask, here the bit positions
	 * are CRUSH_BUCKET_*.  note that these are *bits* and
	 * CRUSH_BUCKET_* values are not, so we need to or together (1
	 * << CRUSH_BUCKET_WHATEVER).  The 0th bit is not used to
	 * minimize confusion (bucket type values start at 1).
	 */
	__u32 allowed_bucket_algs;

	__u32 *choose_tries;
};
</pre>

我们将上述crushmap.txt生成crushmap-new.bin过程中在内存中crushmap dump出来（这里我们通过更改源代码调用dump函数）：
<pre>
key::tunables:choose_local_tries[0]="0"
key::tunables:choose_local_fallback_tries[0]="0" 
key::tunables:choose_total_tries[0]="50" 
key::tunables:chooseleaf_descend_once[0]="1" 
key::tunables:chooseleaf_vary_r[0]="0" 
key::tunables:straw_calc_version[0]="1" 
key::tunables:allowed_bucket_algs[0]="22"
key::tunables:profile[0]="unknown"
key::tunables:optimal_tunables[0]="0" 
key::tunables:legacy_tunables[0]="0" 
key::tunables:require_feature_tunables[0]="1" 
key::tunables:require_feature_tunables2[0]="1" 
key::tunables:require_feature_tunables3[0]="0" 
key::tunables:has_v2_rules[0]="0" 
key::tunables:has_v3_rules[0]="0"
key::tunables:has_v4_buckets[0]="0"


# devices
key::device:devices:id[0]="0"
key::device:devices:name[0]="osd.0"
key::device:devices:id[1]="1" 
key::device:devices:name[1]="osd.1" 
key::device:devices:id[2]="2" 
key::device:devices:name[2]="osd.2" 
key::device:devices:id[3]="3" 
key::device:devices:name[3]="osd.3" 
key::device:devices:id[4]="4" 
key::device:devices:name[4]="osd.4" 
key::device:devices:id[5]="5"
key::device:devices:name[5]="osd.5"
key::device:devices:id[6]="6" 
key::device:devices:name[6]="osd.6"
key::device:devices:id[7]="7"
key::device:devices:name[7]="osd.7"
key::device:devices:id[8]="8" 
key::device:devices:name[8]="osd.8"


# types
key::type:types:type_id[0]="0" 
key::type:types:name[0]="osd" 
key::type:types:type_id[1]="1" 
key::type:types:name[1]="host"
key::type:types:type_id[2]="2" 
key::type:types:name[2]="chassis" 
key::type:types:type_id[3]="3" 
key::type:types:name[3]="rack" 
key::type:types:type_id[4]="4"
key::type:types:name[4]="row"
key::type:types:type_id[5]="5" 
key::type:types:name[5]="pdu"
key::type:types:type_id[6]="6" 
key::type:types:name[6]="pod" 
key::type:types:type_id[7]="7" 
key::type:types:name[7]="room" 
key::type:types:type_id[8]="8" 
key::type:types:name[8]="datacenter" 
key::type:types:type_id[9]="9" 
key::type:types:name[9]="region" 
key::type:types:type_id[10]="10" 
key::type:types:name[10]="root" 
key::type:types:type_id[11]="11"
key::type:types:name[11]="osd-domain"
key::type:types:type_id[12]="12" 
key::type:types:name[12]="host-domain" 
key::type:types:type_id[13]="13" 
key::type:types:name[13]="replica-domain" 
key::type:types:type_id[14]="14" 
key::type:types:name[14]="failure-domain" 


# buckets
key::bucket:buckets:id[0]="-1" 
key::bucket:buckets:name[0]="default" 
key::bucket:buckets:type_id[0]="10"
key::bucket:buckets:type_name[0]="root" 
key::bucket:buckets:weight[0]="88473"     //这里乘上了0x10000
key::bucket:buckets:alg[0]="straw" 
key::bucket:buckets:hash[0]="rjenkins1"
key::item:items:bucket:buckets:id[0]="-3"
key::item:items:bucket:buckets:weight[0]="29491" 
key::item:items:bucket:buckets:pos[0]="0" 
key::item:items:bucket:buckets:id[1]="-5" 
key::item:items:bucket:buckets:weight[1]="29491" 
key::item:items:bucket:buckets:pos[1]="1" 
key::item:items:bucket:buckets:id[2]="-7" 
key::item:items:bucket:buckets:weight[2]="29491" 
key::item:items:bucket:buckets:pos[2]="2"

key::bucket:buckets:id[1]="-2"
key::bucket:buckets:name[1]="node7-1" 
key::bucket:buckets:type_id[1]="1" 
key::bucket:buckets:type_name[1]="host"
key::bucket:buckets:weight[1]="29490"
key::bucket:buckets:alg[1]="straw"
key::bucket:buckets:hash[1]="rjenkins1" 
key::item:items:bucket:buckets:id[3]="0" 
key::item:items:bucket:buckets:weight[3]="9830"
key::item:items:bucket:buckets:pos[3]="0" 
key::item:items:bucket:buckets:id[4]="1" 
key::item:items:bucket:buckets:weight[4]="9830" 
key::item:items:bucket:buckets:pos[4]="1" 
key::item:items:bucket:buckets:id[5]="2" 
key::item:items:bucket:buckets:weight[5]="9830" 
key::item:items:bucket:buckets:pos[5]="2"

key::bucket:buckets:id[2]="-3" 
key::bucket:buckets:name[2]="rack-01" 
key::bucket:buckets:type_id[2]="3" 
key::bucket:buckets:type_name[2]="rack" 
key::bucket:buckets:weight[2]="29491" 
key::bucket:buckets:alg[2]="straw" 
key::bucket:buckets:hash[2]="rjenkins1"
key::item:items:bucket:buckets:id[6]="-2" 
key::item:items:bucket:buckets:weight[6]="29491"
key::item:items:bucket:buckets:pos[6]="0" 

key::bucket:buckets:id[3]="-4" 
key::bucket:buckets:name[3]="node7-2" 
key::bucket:buckets:type_id[3]="1" 
key::bucket:buckets:type_name[3]="host" 
key::bucket:buckets:weight[3]="29490" 
key::bucket:buckets:alg[3]="straw"
key::bucket:buckets:hash[3]="rjenkins1" 
key::item:items:bucket:buckets:id[7]="3" 
key::item:items:bucket:buckets:weight[7]="9830" 
key::item:items:bucket:buckets:pos[7]="0"
key::item:items:bucket:buckets:id[8]="4" 
key::item:items:bucket:buckets:weight[8]="9830"
key::item:items:bucket:buckets:pos[8]="1"
key::item:items:bucket:buckets:id[9]="5"
key::item:items:bucket:buckets:weight[9]="9830"
key::item:items:bucket:buckets:pos[9]="2"

key::bucket:buckets:id[4]="-5" 
key::bucket:buckets:name[4]="rack-02"
key::bucket:buckets:type_id[4]="3"
key::bucket:buckets:type_name[4]="rack" 
key::bucket:buckets:weight[4]="29491" 
key::bucket:buckets:alg[4]="straw" 
key::bucket:buckets:hash[4]="rjenkins1" 
key::item:items:bucket:buckets:id[10]="-4" 
key::item:items:bucket:buckets:weight[10]="29491" 
key::item:items:bucket:buckets:pos[10]="0"

key::bucket:buckets:id[5]="-6"
key::bucket:buckets:name[5]="node7-3" 
key::bucket:buckets:type_id[5]="1" 
key::bucket:buckets:type_name[5]="host"
key::bucket:buckets:weight[5]="29490" 
key::bucket:buckets:alg[5]="straw" 
key::bucket:buckets:hash[5]="rjenkins1"
key::item:items:bucket:buckets:id[11]="6" 
key::item:items:bucket:buckets:weight[11]="9830"
key::item:items:bucket:buckets:pos[11]="0" 
key::item:items:bucket:buckets:id[12]="7"
key::item:items:bucket:buckets:weight[12]="9830" 
key::item:items:bucket:buckets:pos[12]="1" 
key::item:items:bucket:buckets:id[13]="8" 
key::item:items:bucket:buckets:weight[13]="9830" 
key::item:items:bucket:buckets:pos[13]="2"

key::bucket:buckets:id[6]="-7" 
key::bucket:buckets:name[6]="rack-03" 
key::bucket:buckets:type_id[6]="3" 
key::bucket:buckets:type_name[6]="rack"
key::bucket:buckets:weight[6]="29491"
key::bucket:buckets:alg[6]="straw"
key::bucket:buckets:hash[6]="rjenkins1" 
key::item:items:bucket:buckets:id[14]="-6" 
key::item:items:bucket:buckets:weight[14]="29491"
key::item:items:bucket:buckets:pos[14]="0" 

key::bucket:buckets:id[7]="-8" 
key::bucket:buckets:name[7]="host-group-0-rack-01"
key::bucket:buckets:type_id[7]="12"
key::bucket:buckets:type_name[7]="host-domain" 
key::bucket:buckets:weight[7]="29491"
key::bucket:buckets:alg[7]="straw"
key::bucket:buckets:hash[7]="rjenkins1" 
key::item:items:bucket:buckets:id[15]="-2" 
key::item:items:bucket:buckets:weight[15]="29491"
key::item:items:bucket:buckets:pos[15]="0" 

key::bucket:buckets:id[8]="-9"
key::bucket:buckets:name[8]="replica-0"
key::bucket:buckets:type_id[8]="13" 
key::bucket:buckets:type_name[8]="replica-domain"
key::bucket:buckets:weight[8]="88473" 
key::bucket:buckets:alg[8]="straw"
key::bucket:buckets:hash[8]="rjenkins1"
key::item:items:bucket:buckets:id[16]="-8"
key::item:items:bucket:buckets:weight[16]="29491" 
key::item:items:bucket:buckets:pos[16]="0" 
key::item:items:bucket:buckets:id[17]="-11"
key::item:items:bucket:buckets:weight[17]="29491" 
key::item:items:bucket:buckets:pos[17]="1"
key::item:items:bucket:buckets:id[18]="-12" 
key::item:items:bucket:buckets:weight[18]="29491" 
key::item:items:bucket:buckets:pos[18]="2"

key::bucket:buckets:id[9]="-10" 
key::bucket:buckets:name[9]="sata-00" 
key::bucket:buckets:type_id[9]="14" 
key::bucket:buckets:type_name[9]="failure-domain"
key::bucket:buckets:weight[9]="88473"
key::bucket:buckets:alg[9]="straw" 
key::bucket:buckets:hash[9]="rjenkins1"
key::item:items:bucket:buckets:id[19]="-9" 
key::item:items:bucket:buckets:weight[19]="88473"
key::item:items:bucket:buckets:pos[19]="0" 

key::bucket:buckets:id[10]="-11"
key::bucket:buckets:name[10]="host-group-0-rack-02" 
key::bucket:buckets:type_id[10]="12" 
key::bucket:buckets:type_name[10]="host-domain"
key::bucket:buckets:weight[10]="29491"
key::bucket:buckets:alg[10]="straw"
key::bucket:buckets:hash[10]="rjenkins1" 
key::item:items:bucket:buckets:id[20]="-4"
key::item:items:bucket:buckets:weight[20]="29491" 
key::item:items:bucket:buckets:pos[20]="0"

key::bucket:buckets:id[11]="-12"
key::bucket:buckets:name[11]="host-group-0-rack-03"
key::bucket:buckets:type_id[11]="12" 
key::bucket:buckets:type_name[11]="host-domain" 
key::bucket:buckets:weight[11]="29491" 
key::bucket:buckets:alg[11]="straw" 
key::bucket:buckets:hash[11]="rjenkins1"
key::item:items:bucket:buckets:id[21]="-6"
key::item:items:bucket:buckets:weight[21]="29491" 
key::item:items:bucket:buckets:pos[21]="0"


# rules
key::rule:rules:rule_id[0]="0" 
key::rule:rules:rule_name[0]="replicated_ruleset"
key::rule:rules:ruleset[0]="0"
key::rule:rules:type[0]="1" 
key::rule:rules:min_size[0]="1"
key::rule:rules:max_size[0]="10" 
key::step:steps:rule:rules:op[0]="take" 
key::step:steps:rule:rules:item[0]="-1" 
key::step:steps:rule:rules:item_name[0]="default" 
key::step:steps:rule:rules:op[1]="choose_firstn" 
key::step:steps:rule:rules:num[0]="0" 
key::step:steps:rule:rules:type[0]="osd" 
key::step:steps:rule:rules:op[2]="emit"

key::rule:rules:rule_id[1]="1" 
key::rule:rules:rule_name[1]="replicated_rule-5" 
key::rule:rules:ruleset[1]="5"
key::rule:rules:type[1]="1" 
key::rule:rules:min_size[1]="1" 
key::rule:rules:max_size[1]="10" 
key::step:steps:rule:rules:op[3]="take"
key::step:steps:rule:rules:item[1]="-10" 
key::step:steps:rule:rules:item_name[1]="sata-00"
key::step:steps:rule:rules:op[4]="choose_firstn" 
key::step:steps:rule:rules:num[1]="1" 
key::step:steps:rule:rules:type[1]="replica-domain" 
key::step:steps:rule:rules:op[5]="chooseleaf_firstn"
key::step:steps:rule:rules:num[2]="0"
key::step:steps:rule:rules:type[2]="host-domain" 
key::step:steps:rule:rules:op[6]="emit" 
</pre>

如上，对于device id用正数表示，对于buckets id用负数表示。从上面我们看出，一个```crushmap```主要包含如下几个方面：
{% highlight string %}
crushmap{
	tunables;    //用于控制crushmap映射的一些变量
	devices;	 //用于存储数据的osd，如osd.0、osd.1、osd.2
	types;		 //用于将devices进行逻辑分布划分的概念，如host-domain、replica-domain
	buckets;	 //逻辑上的一个buckets概念，其实就是types的一个具体实例，用于从更高一层次管理devices。
				 //buckets具有层级结构，其下层可以是另一个buckets(id<0)，也可以是devices(id>0)
	rules;		 //用于控制如何将PG映射到各个devices上

};
{% endhighlight %}


## 4. crushmap.txt生成crushmap过程分析

查看crushtool源代码，看到主要的生成代码如下：

![crushmap-compile-entry](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-compile-entry.png)

其中，```crush```为CrushWrapper数据类型，其主要结构如下：
{% highlight string %}
class CrushWrapper {
public:
  std::map<int32_t, string> type_map; /* bucket/device type names */
  std::map<int32_t, string> name_map; /* bucket/device names */
  std::map<int32_t, string> rule_name_map;

private:
  struct crush_map *crush;

......
};
{% endhighlight %}

在cc.compile()中首先读取crushmap.txt的每一行，将注释等去除，去除多余的空格，然后调用```boost/spirit```来解析：
{% highlight string %}
int CrushCompiler::compile(istream& in, const char *infn)
{
  if (!infn)
    infn = "<input>";

  // always start with legacy tunables, so that the compiled result of
  // a given crush file is fixed for all time.
  crush.set_tunables_legacy();

  string big;
  string str;
  int line = 1;
  map<int,int> line_pos;  // pos -> line
  map<int,string> line_val;
  while (getline(in, str)) {
    // remove newline
    int l = str.length();
    if (l && str[l] == '\n')
      str.erase(l-1, 1);

    line_val[line] = str;

    // strip comment
    int n = str.find("#");
    if (n >= 0)
      str.erase(n, str.length()-n);
    
    if (verbose>1) err << line << ": " << str << std::endl;

    // work around spirit crankiness by removing extraneous
    // whitespace.  there is probably a more elegant solution, but
    // this only broke with the latest spirit (with the switchover to
    // "classic"), i don't want to spend too much time figuring it
    // out.
    string stripped = consolidate_whitespace(str);
    if (stripped.length() && big.length() && big[big.length()-1] != ' ') big += " ";

    line_pos[big.length()] = line;
    line++;
    big += stripped;
  }
  
  if (verbose > 2) err << "whole file is: \"" << big << "\"" << std::endl;
  
  crush_grammar crushg;
  const char *start = big.c_str();
  //tree_parse_info<const char *> info = ast_parse(start, crushg, space_p);
  tree_parse_info<> info = ast_parse(start, crushg, space_p);
  
  // parse error?
  if (!info.full) {
    int cpos = info.stop - start;
    //out << "cpos " << cpos << std::endl;
    //out << " linemap " << line_pos << std::endl;
    assert(!line_pos.empty());
    map<int,int>::iterator p = line_pos.upper_bound(cpos);
    if (p != line_pos.begin())
      --p;
    int line = p->second;
    int pos = cpos - p->first;
    err << infn << ":" << line //<< ":" << (pos+1)
	<< " error: parse error at '" << line_val[line].substr(pos) << "'" << std::endl;
    return -1;
  }

  //out << "parsing succeeded\n";
  //dump(info.trees.begin());
  return parse_crush(info.trees.begin());
}
{% endhighlight %}

上面```ast_parse```是boost/spirit的相关函数，其解析时需要相应的模板（参看：http://www.cnblogs.com/catch/p/3921751.html）：
{% highlight string %}
struct crush_grammar : public grammar<crush_grammar>
{
  enum {
    _int = 1,
    _posint,
    _negint,
    _name,
    _device,
    _bucket_type,
    _bucket_id,
    _bucket_alg,
    _bucket_hash,
    _bucket_item,
    _bucket,
    _step_take,
    _step_set_chooseleaf_tries,
    _step_set_chooseleaf_vary_r,
    _step_set_choose_tries,
    _step_set_choose_local_tries,
    _step_set_choose_local_fallback_tries,
    _step_choose,
    _step_chooseleaf,
    _step_emit,
    _step,
    _crushrule,
    _crushmap,
    _tunable,
  };

  template <typename ScannerT>
  struct definition
  {
    rule<ScannerT, parser_context<>, parser_tag<_int> >      integer;
    rule<ScannerT, parser_context<>, parser_tag<_posint> >      posint;
    rule<ScannerT, parser_context<>, parser_tag<_negint> >      negint;
    rule<ScannerT, parser_context<>, parser_tag<_name> >      name;

    rule<ScannerT, parser_context<>, parser_tag<_tunable> >      tunable;

    rule<ScannerT, parser_context<>, parser_tag<_device> >      device;

    rule<ScannerT, parser_context<>, parser_tag<_bucket_type> >    bucket_type;

    rule<ScannerT, parser_context<>, parser_tag<_bucket_id> >      bucket_id;
    rule<ScannerT, parser_context<>, parser_tag<_bucket_alg> >     bucket_alg;
    rule<ScannerT, parser_context<>, parser_tag<_bucket_hash> >    bucket_hash;
    rule<ScannerT, parser_context<>, parser_tag<_bucket_item> >    bucket_item;
    rule<ScannerT, parser_context<>, parser_tag<_bucket> >      bucket;

    rule<ScannerT, parser_context<>, parser_tag<_step_take> >      step_take;
    rule<ScannerT, parser_context<>, parser_tag<_step_set_choose_tries> >    step_set_choose_tries;
    rule<ScannerT, parser_context<>, parser_tag<_step_set_choose_local_tries> >    step_set_choose_local_tries;
    rule<ScannerT, parser_context<>, parser_tag<_step_set_choose_local_fallback_tries> >    step_set_choose_local_fallback_tries;
    rule<ScannerT, parser_context<>, parser_tag<_step_set_chooseleaf_tries> >    step_set_chooseleaf_tries;
    rule<ScannerT, parser_context<>, parser_tag<_step_set_chooseleaf_vary_r> >    step_set_chooseleaf_vary_r;
    rule<ScannerT, parser_context<>, parser_tag<_step_choose> >    step_choose;
    rule<ScannerT, parser_context<>, parser_tag<_step_chooseleaf> >      step_chooseleaf;
    rule<ScannerT, parser_context<>, parser_tag<_step_emit> >      step_emit;
    rule<ScannerT, parser_context<>, parser_tag<_step> >      step;
    rule<ScannerT, parser_context<>, parser_tag<_crushrule> >      crushrule;

    rule<ScannerT, parser_context<>, parser_tag<_crushmap> >      crushmap;

    definition(crush_grammar const& /*self*/)
    {
      // base types
      integer     =   leaf_node_d[ lexeme_d[
					    (!ch_p('-') >> +digit_p)
					    ] ];
      posint     =   leaf_node_d[ lexeme_d[ +digit_p ] ];
      negint     =   leaf_node_d[ lexeme_d[ ch_p('-') >> +digit_p ] ];
      name = leaf_node_d[ lexeme_d[ +( alnum_p || ch_p('-') || ch_p('_') || ch_p('.')) ] ];

      // tunables
      tunable = str_p("tunable") >> name >> posint;

      // devices
      device = str_p("device") >> posint >> name;

      // bucket types
      bucket_type = str_p("type") >> posint >> name;

      // buckets
      bucket_id = str_p("id") >> negint;
      bucket_alg = str_p("alg") >> name;
      bucket_hash = str_p("hash") >> ( integer |
				       str_p("rjenkins1") );
      bucket_item = str_p("item") >> name
				  >> !( str_p("weight") >> real_p )
				  >> !( str_p("pos") >> posint );
      bucket = name >> name >> '{' >> !bucket_id >> bucket_alg >> *bucket_hash >> *bucket_item >> '}';

      // rules
      step_take = str_p("take") >> name;
      step_set_choose_tries = str_p("set_choose_tries") >> posint;
      step_set_choose_local_tries = str_p("set_choose_local_tries") >> posint;
      step_set_choose_local_fallback_tries = str_p("set_choose_local_fallback_tries") >> posint;
      step_set_chooseleaf_tries = str_p("set_chooseleaf_tries") >> posint;
      step_set_chooseleaf_vary_r = str_p("set_chooseleaf_vary_r") >> posint;
      step_choose = str_p("choose")
	>> ( str_p("indep") | str_p("firstn") )
	>> integer
	>> str_p("type") >> name;
      step_chooseleaf = str_p("chooseleaf")
	>> ( str_p("indep") | str_p("firstn") )
	>> integer
	>> str_p("type") >> name;
      step_emit = str_p("emit");
      step = str_p("step") >> ( step_take |
				step_set_choose_tries |
				step_set_choose_local_tries |
				step_set_choose_local_fallback_tries |
				step_set_chooseleaf_tries |
				step_set_chooseleaf_vary_r |
				step_choose |
				step_chooseleaf |
				step_emit );
      crushrule = str_p("rule") >> !name >> '{'
			   >> str_p("ruleset") >> posint
			   >> str_p("type") >> ( str_p("replicated") | str_p("erasure") )
			   >> str_p("min_size") >> posint
			   >> str_p("max_size") >> posint
			   >> +step
			   >> '}';

      // the whole crush map
      crushmap = *(tunable | device | bucket_type) >> *(bucket | crushrule);
    }

    rule<ScannerT, parser_context<>, parser_tag<_crushmap> > const&
    start() const { return crushmap; }
  };
};

{% endhighlight %}

通过上述模板解析成```tree_parse_info```结构，然后再进行解析：
![crushmap-compile-parse](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-compile-parse.png)

然后再调用```crush_finalize```修正max_devices：
![crushmap-compile-finalize](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-compile-finalize.png)


## 5. 对crushmap进行编码
上面对crushmap.txt解析之后在内存中生成crush_map进行编码：
![crushmap-encode-pic1](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-encode-pic1.png)

![crushmap-encode-pic2](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-encode-pic2.png)

下面结合程序代码来看编码过程：
{% highlight string %}
void CrushWrapper::encode(bufferlist& bl, bool lean) const
{
  assert(crush);

  __u32 magic = CRUSH_MAGIC;
  ::encode(magic, bl);

  ::encode(crush->max_buckets, bl);
  ::encode(crush->max_rules, bl);
  ::encode(crush->max_devices, bl);

  // buckets
  for (int i=0; i<crush->max_buckets; i++) {
    __u32 alg = 0;
    if (crush->buckets[i]) alg = crush->buckets[i]->alg;
    ::encode(alg, bl);
    if (!alg)
      continue;

    ::encode(crush->buckets[i]->id, bl);
    ::encode(crush->buckets[i]->type, bl);
    ::encode(crush->buckets[i]->alg, bl);
    ::encode(crush->buckets[i]->hash, bl);
    ::encode(crush->buckets[i]->weight, bl);
    ::encode(crush->buckets[i]->size, bl);
    for (unsigned j=0; j<crush->buckets[i]->size; j++)
      ::encode(crush->buckets[i]->items[j], bl);

    switch (crush->buckets[i]->alg) {
    case CRUSH_BUCKET_UNIFORM:
      ::encode((reinterpret_cast<crush_bucket_uniform*>(crush->buckets[i]))->item_weight, bl);
      break;

    case CRUSH_BUCKET_LIST:
      for (unsigned j=0; j<crush->buckets[i]->size; j++) {
	::encode((reinterpret_cast<crush_bucket_list*>(crush->buckets[i]))->item_weights[j], bl);
	::encode((reinterpret_cast<crush_bucket_list*>(crush->buckets[i]))->sum_weights[j], bl);
      }
      break;

    case CRUSH_BUCKET_TREE:
      ::encode((reinterpret_cast<crush_bucket_tree*>(crush->buckets[i]))->num_nodes, bl);
      for (unsigned j=0; j<(reinterpret_cast<crush_bucket_tree*>(crush->buckets[i]))->num_nodes; j++)
	::encode((reinterpret_cast<crush_bucket_tree*>(crush->buckets[i]))->node_weights[j], bl);
      break;

    case CRUSH_BUCKET_STRAW:
      for (unsigned j=0; j<crush->buckets[i]->size; j++) {
	::encode((reinterpret_cast<crush_bucket_straw*>(crush->buckets[i]))->item_weights[j], bl);
	::encode((reinterpret_cast<crush_bucket_straw*>(crush->buckets[i]))->straws[j], bl);
      }
      break;

    case CRUSH_BUCKET_STRAW2:
      for (unsigned j=0; j<crush->buckets[i]->size; j++) {
	::encode((reinterpret_cast<crush_bucket_straw2*>(crush->buckets[i]))->item_weights[j], bl);
      }
      break;

    default:
      assert(0);
      break;
    }
  }

  // rules
  for (unsigned i=0; i<crush->max_rules; i++) {
    __u32 yes = crush->rules[i] ? 1:0;
    ::encode(yes, bl);
    if (!yes)
      continue;

    ::encode(crush->rules[i]->len, bl);
    ::encode(crush->rules[i]->mask, bl);
    for (unsigned j=0; j<crush->rules[i]->len; j++)
      ::encode(crush->rules[i]->steps[j], bl);
  }

  // name info
  ::encode(type_map, bl);
  ::encode(name_map, bl);
  ::encode(rule_name_map, bl);

  // tunables
  ::encode(crush->choose_local_tries, bl);
  ::encode(crush->choose_local_fallback_tries, bl);
  ::encode(crush->choose_total_tries, bl);
  ::encode(crush->chooseleaf_descend_once, bl);
  ::encode(crush->chooseleaf_vary_r, bl);
  ::encode(crush->straw_calc_version, bl);
  ::encode(crush->allowed_bucket_algs, bl);
}
{% endhighlight %}

*注意：上面的编码都是采用机器的默认大小端（测试机器默认为小端）*

(1) 编码CRUSH_MAGIC
<pre>
#define CRUSH_MAGIC 0x00010000ul   /* for detecting algorithm revisions */
</pre>

(2) 编码max_buckets,max_rules，max_devices

这里我们max_buckets为0x00000010,而我们实际的buckets数目为12，这是允许的。max_rules为0x00000002，max_devices为0x00000009。

(3) 编码buckets
* bucket->alg
* bucket->id
* bucket->type
* bucket->alg   (repeat)
* bucket->hash
* bucket->weight
* bucket->size
* bucket->items[i]
* bucket->alg_alternate_info

我们来分析第一个bucket：
<pre>
root default {
        id -1           # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item rack-01 weight 0.450
        item rack-02 weight 0.450
        item rack-03 weight 0.450
}
</pre>

其alg为straw(CRUSH_BUCKET_STRAW = 4)；id为-1（负数以补码表示为0xFFFFFFFF)；type为root（结合我们上面定义bucket types时其值为10,即0x000A)；alg为0x04; hash值为0x00（即rjenkins1)；weight为0x00015999(即十进制的88473，这里需要再除以0x10000,得到的值刚好为1.350）；size为0x00000003,也恰好为我们的item数；3个items,分别是rack-01,rack-02,rack-03(值分别是0xFFFFFFFD,0xFFFFFFFB,0xFFFFFFF9）；3个alg_alternate_info(值分别为：0x00007333,0x00010000; 0x00007333,0x00010000;0x00007333,0x00010000)

（4） 编码rules


（5） 编码name info

（6） 编码tunables

<br />
<br />
<br />