---
layout: post
title: crushmap详解(1)
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


## 生成crushmap.bin
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


## crushmap的内部数据结构
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

如上，对于device id用正数表示，对于buckets id用负数表示。




