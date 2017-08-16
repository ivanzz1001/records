---
layout: post
title: crushmap算法详解
tags:
- ceph
- crushmap
categories: ceph
description: crushmap算法详解
---

本文接上篇《crushmap详解-2》，结合ceph源代码及其他参考资料来详尽的探讨具体的crush算法。为了参看的方便，下面我们继续列出当前的crushmap:

<!-- more -->
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

## 1. CRUSH算法
![crushmap3-alg](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap3_alg.png)

上面是CRUSH算法实现的一个伪代码。通常情况下CRUSH函数的输入参数是一个对象名或其他标识符。

1） TAKE(a)操作会在整个存储拓扑结构中选择一个item(通常是一个bucket)，然后会将其指定给一个vector。该vector作为后续操作的一个输入。

2) SELECT(n,t)操作会遍历上述vector中的每一个元素，并且会在以该元素作为根的子树中选择类型为t的n个不同的item。存储设备均有一个已知的、固定的类型，并且系统中的每一个bucket均有一个类型字段以反映bucket的不同层级结构。


crush算法的基本思想就是：从```step take```根开始逐级遍历bucket层级结构，直到找到指定数量的副本节点或者失败退出。 
<br />
<br />

**Collisions, Failure, and Overload**

在select(n,t)操作当中，为了选出n个t类型的不同的item，其会不断的进行递归。 在该递归操作当中，CRUSH通过一个被修正过的r'来拒绝或选择item。之所有采用修正的r'(而不是当前的简单的副本编号）主要有如下3个原因：
* 该item已经处于当前被选中的集合中（发生了碰撞---select(n,t)算出来的结果必须必须不能冲突）
* 一个设备(device)处于failed状态
* 一个设备处于overloaded状态（负载过重）

Failed及Overloaded状态的设备均会在cluster map中进行标记，但会被保留在设备层级结构中以避免不必要的数据迁移。CRUSH会选择性的转移一部分负载过高的设备上的数据，这可以通过伪随机的拒绝一些的PG映射。针对Failed及Overloaded状态的设备，CRUSH算法会统一的在storage cluster的其他分支（跨storage cluster)来选择最后的映射（参看算法1第11行）；而对于collisions，一个新的r'会被用于下一次的递归搜索（参看算法1第14行），这样可以避免总体数据分步偏离更大可能发生碰撞的子树。



## 2. CRUSH算法源代码解析

在源代码分析过程中，我们可以通过执行如下命令来具体了解程序的执行过程：
{% highlight string %}
/root/ceph-src/ceph/src/crushtool --test -i test_crushmap.bin --show-mappings --ruleset 5 --num-rep=3 --min_x=0 --max_x=10
{% endhighlight %}

接着上一篇《crushmap详解-2》，函数调用到do_rule:
![crushmap3-do-rule](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap3_do_rule.png)

上面do_rule()函数使用指定的crush及rule规则将输入x映射到OSD设备上（这里rule为1，maxout为3，weight值均为65536）。

下面我们来分析crush_do_rule:
{% highlight string %}
/**
 * crush_do_rule - calculate a mapping with the given input and rule
 * @map: the crush_map
 * @ruleno: the rule id
 * @x: hash input
 * @result: pointer to result vector
 * @result_max: maximum result size
 * @weight: weight vector (for map leaves)
 * @weight_max: size of weight vector
 * @scratch: scratch vector for private use; must be >= 3 * result_max
 */
int crush_do_rule(const struct crush_map *map,
		  int ruleno, int x, int *result, int result_max,
		  const __u32 *weight, int weight_max,
		  int *scratch)
{
	int result_len;
	int *a = scratch;
	int *b = scratch + result_max;
	int *c = scratch + result_max*2;
	int recurse_to_leaf;
	int *w;
	int wsize = 0;
	int *o;
	int osize;
	int *tmp;
	struct crush_rule *rule;
	__u32 step;
	int i, j;
	int numrep;
	int out_size;
	/*
	 * the original choose_total_tries value was off by one (it
	 * counted "retries" and not "tries").  add one.
	 */
	int choose_tries = map->choose_total_tries + 1;
	int choose_leaf_tries = 0;
	/*
	 * the local tries values were counted as "retries", though,
	 * and need no adjustment
	 */
	int choose_local_retries = map->choose_local_tries;
	int choose_local_fallback_retries = map->choose_local_fallback_tries;

	int vary_r = map->chooseleaf_vary_r;

	if ((__u32)ruleno >= map->max_rules) {
		dprintk(" bad ruleno %d\n", ruleno);
		return 0;
	}

	rule = map->rules[ruleno];
	result_len = 0;
	w = a;
	o = b;

	for (step = 0; step < rule->len; step++) {
		int firstn = 0;
		struct crush_rule_step *curstep = &rule->steps[step];

		switch (curstep->op) {
		case CRUSH_RULE_TAKE:
			if ((curstep->arg1 >= 0 &&
			     curstep->arg1 < map->max_devices) ||
			    (-1-curstep->arg1 >= 0 &&
			     -1-curstep->arg1 < map->max_buckets &&
			     map->buckets[-1-curstep->arg1])) {
				w[0] = curstep->arg1;
				wsize = 1;
			} else {
				dprintk(" bad take value %d\n", curstep->arg1);
			}
			break;

		case CRUSH_RULE_SET_CHOOSE_TRIES:
			if (curstep->arg1 > 0)
				choose_tries = curstep->arg1;
			break;

		case CRUSH_RULE_SET_CHOOSELEAF_TRIES:
			if (curstep->arg1 > 0)
				choose_leaf_tries = curstep->arg1;
			break;

		case CRUSH_RULE_SET_CHOOSE_LOCAL_TRIES:
			if (curstep->arg1 >= 0)
				choose_local_retries = curstep->arg1;
			break;

		case CRUSH_RULE_SET_CHOOSE_LOCAL_FALLBACK_TRIES:
			if (curstep->arg1 >= 0)
				choose_local_fallback_retries = curstep->arg1;
			break;

		case CRUSH_RULE_SET_CHOOSELEAF_VARY_R:
			if (curstep->arg1 >= 0)
				vary_r = curstep->arg1;
			break;

		case CRUSH_RULE_CHOOSELEAF_FIRSTN:
		case CRUSH_RULE_CHOOSE_FIRSTN:
			firstn = 1;
			/* fall through */
		case CRUSH_RULE_CHOOSELEAF_INDEP:
		case CRUSH_RULE_CHOOSE_INDEP:
			if (wsize == 0)
				break;

			recurse_to_leaf =
				curstep->op ==
				 CRUSH_RULE_CHOOSELEAF_FIRSTN ||
				curstep->op ==
				CRUSH_RULE_CHOOSELEAF_INDEP;

			/* reset output */
			osize = 0;

			for (i = 0; i < wsize; i++) {
				int bno;
				/*
				 * see CRUSH_N, CRUSH_N_MINUS macros.
				 * basically, numrep <= 0 means relative to
				 * the provided result_max
				 */
				numrep = curstep->arg1;
				if (numrep <= 0) {
					numrep += result_max;
					if (numrep <= 0)
						continue;
				}
				j = 0;
				/* make sure bucket id is valid */
				bno = -1 - w[i];
				if (bno < 0 || bno >= map->max_buckets) {
					// w[i] is probably CRUSH_ITEM_NONE
					dprintk("  bad w[i] %d\n", w[i]);
					continue;
				}
				if (firstn) {
					int recurse_tries;
					if (choose_leaf_tries)
						recurse_tries =
							choose_leaf_tries;
					else if (map->chooseleaf_descend_once)
						recurse_tries = 1;
					else
						recurse_tries = choose_tries;
					osize += crush_choose_firstn(
						map,
						map->buckets[bno],
						weight, weight_max,
						x, numrep,
						curstep->arg2,
						o+osize, j,
						result_max-osize,
						choose_tries,
						recurse_tries,
						choose_local_retries,
						choose_local_fallback_retries,
						recurse_to_leaf,
						vary_r,
						c+osize,
						0);
				} else {
					out_size = ((numrep < (result_max-osize)) ?
                                                    numrep : (result_max-osize));
					crush_choose_indep(
						map,
						map->buckets[bno],
						weight, weight_max,
						x, out_size, numrep,
						curstep->arg2,
						o+osize, j,
						choose_tries,
						choose_leaf_tries ?
						   choose_leaf_tries : 1,
						recurse_to_leaf,
						c+osize,
						0);
					osize += out_size;
				}
			}

			if (recurse_to_leaf)
				/* copy final _leaf_ values to output set */
				memcpy(o, c, osize*sizeof(*o));

			/* swap o and w arrays */
			tmp = o;
			o = w;
			w = tmp;
			wsize = osize;
			break;


		case CRUSH_RULE_EMIT:
			for (i = 0; i < wsize && result_len < result_max; i++) {
				result[result_len] = w[i];
				result_len++;
			}
			wsize = 0;
			break;

		default:
			dprintk(" unknown op %d at step %d\n",
				curstep->op, step);
			break;
		}
	}
	return result_len;
}
{% endhighlight %}

该函数接受8个参数：
* map: 当前所采用的crushmap
* ruleno: 当前所使用的规则编号
* x：hash input，即当前所要映射的对象的hash值
* result: 保存输出结果的的数组起始位置
* result_max: 保存输出结果的数组的大小
* weight: 权重向量（用于映射叶子）
* weight_max: 权重向量中元素的个数
* scratch: 辅助空间（内部使用，其大小应确保为>=3*result_max)


### 2.1 相关变量初始化

根据我们前面的配置（如下tries均为`retries`含义，在使用时应注意）：
<pre>
# begin crush map
tunable choose_local_tries 0
tunable choose_local_fallback_tries 0
tunable choose_total_tries 50
tunable chooseleaf_descend_once 1
tunable straw_calc_version 1
</pre>
因此，这里choose_tries为51，choose_leaf_tries为0，choose_local_retries为0，choose_local_fallback_retries为0，vary_r为0。 我们这里使用的ruleset 5，因此rule->len为4。

### 2.2 crush映射步骤

参看下面的映射规则：
{% highlight string %}
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
{% endhighlight %}
根据前面《crushmap详解-1》我们得到如下：
<pre>
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

上面每一个step的输出都作为下一个step的输入（参看上面伪代码37行）。

1） CRUSH_RULE_TAKE

首先执行的是```CRUSH_RULE_TAKE```步骤，因为sata-00是一个bucket，而不是单个的device，因此这里curstep->arg1应小于0。此时有：
<pre>
w[0] = -10;
wsize = 1;
</pre>

2） CRUSH_RULE_CHOOSE_FIRSTN
{% highlight string %}
printf("\n(Before)wsize:%d bno:%d x:%d numrep:%d curstep->arg1:%d curstep->arg2:%d osize:%d recurse_to_leaf:%d\n",
		wsize,					// 1 
		bno,                    // 9 
		x,                      // hash input
		numrep,                 // 1
		curstep->arg1,          // 1
		curstep->arg2,          // 13
		osize,                  // 0
		recurse_to_leaf);       // 0


		osize += crush_choose_firstn(
			map,
			map->buckets[bno],
			weight, weight_max,
			x, numrep,
			curstep->arg2,
			o+osize, j,
			result_max-osize,
			choose_tries,
			recurse_tries,
			choose_local_retries,
			choose_local_fallback_retries,
			recurse_to_leaf,
			vary_r,
			c+osize,
			0);

		printf("(After)osize:%d\n",osize);      // 1
{% endhighlight %}

如上所示，经过上一步```step take sata-00```之后，wsize为1；bno为上一步```step take sata-00```所选中的bucket编号9(-1+10 = 9):
<pre>
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
</pre>
numrep为当前step所指定的副本数1； curstep->arg1同numrep为副本数1；curstep->arg2为replica-domain，因此值为13; osize 为0；recurse_to_leaf为0，表示不递归叶子节点。 (After)osize为1.
<br />


*针对上面的numrep,有如下解释：*
{% highlight string %}
/*
 * see CRUSH_N, CRUSH_N_MINUS macros.
 * basically, numrep <= 0 means relative to
 * the provided result_max
 */
{% endhighlight %}


3） CRUSH_RULE_CHOOSELEAF_FIRSTN
<pre>
printf("\n(Before)wsize:%d bno:%d x:%d numrep:%d curstep->arg1:%d curstep->arg2:%d osize:%d recurse_to_leaf:%d\n",
	wsize,             // 1
	bno,               // 8
	x,                 // hash input
	numrep,            // 3 
	curstep->arg1,     // 0
	curstep->arg2,     // 12 
	osize,             // 0
	recurse_to_leaf);  // 1


	osize += crush_choose_firstn(
		map,
		map->buckets[bno],
		weight, weight_max,
		x, numrep,
		curstep->arg2,
		o+osize, j,
		result_max-osize,
		choose_tries,
		recurse_tries,
		choose_local_retries,
		choose_local_fallback_retries,
		recurse_to_leaf,
		vary_r,
		c+osize,
		0);

	printf("(After)osize:%d\n",osize);   //3
</pre>

如上所示，经过上一步```step choose firstn 1 type replica-domain```之后，wsize为1；bno为上一步```step choose firstn 1 type replica-domain```之后所选中的replica-0; numrep值为3，表示副本数； curstep->arg1为0；curstep->arg2为host-domain，因此值为12；osize值为0；recurse_to_leaf值为1,表示需要递归叶子节点。（After）osize为3.

<br />

**小结**

从上面我们可以看到，是通过：
<pre>
 step choose firstn 1 type replica-domain
 step chooseleaf firstn 0 type host-domain
</pre>
使我们最后达到的副本数为3。也即从```sata-00```下选择到1个replica-domain，然后再从这1个replica-domain下选择3个host-domain,则最后刚好达到3个副本。我们再来看另外一个规则：
<pre>
rule replicated_ruleset {
        ruleset 0
        type replicated
        min_size 1
        max_size 10
        step take default
        step choose firstn 0 type osd
        step emit
}
</pre>
此规则直接在default下选择osd。因为可以直接选择到osd，因此可以不用递归，否则最后一步一般都要进行递归操作。此处0表示副本数为result_max，即为3。



## 3. 分析crush_choose_firstn()

上面我们在进行CRUSHMAP映射时，调用到了crush_choose_firstn()函数，该函数较为复杂，我们下边来分析该函数：
{% highlight string %}
/**
 * crush_choose_firstn - choose numrep distinct items of given type
 * @map: the crush_map
 * @bucket: the bucket we are choose an item from
 * @x: crush input value
 * @numrep: the number of items to choose
 * @type: the type of item to choose
 * @out: pointer to output vector
 * @outpos: our position in that vector
 * @out_size: size of the out vector
 * @tries: number of attempts to make
 * @recurse_tries: number of attempts to have recursive chooseleaf make
 * @local_retries: localized retries
 * @local_fallback_retries: localized fallback retries
 * @recurse_to_leaf: true if we want one device under each item of given type (chooseleaf instead of choose)
 * @vary_r: pass r to recursive calls
 * @out2: second output vector for leaf items (if @recurse_to_leaf)
 * @parent_r: r value passed from the parent
 */
static int crush_choose_firstn(const struct crush_map *map,
			       struct crush_bucket *bucket,
			       const __u32 *weight, int weight_max,
			       int x, int numrep, int type,
			       int *out, int outpos,
			       int out_size,
			       unsigned int tries,
			       unsigned int recurse_tries,
			       unsigned int local_retries,
			       unsigned int local_fallback_retries,
			       int recurse_to_leaf,
			       unsigned int vary_r,
			       int *out2,
			       int parent_r)
{
	int rep;
	unsigned int ftotal, flocal;
	int retry_descent, retry_bucket, skip_rep;
	struct crush_bucket *in = bucket;
	int r;
	int i;
	int item = 0;
	int itemtype;
	int collide, reject;
	int count = out_size;

	dprintk("CHOOSE%s bucket %d x %d outpos %d numrep %d tries %d recurse_tries %d local_retries %d local_fallback_retries %d parent_r %d\n",
		recurse_to_leaf ? "_LEAF" : "",
		bucket->id, x, outpos, numrep,
		tries, recurse_tries, local_retries, local_fallback_retries,
		parent_r);

	for (rep = outpos; rep < numrep && count > 0 ; rep++) {
		/* keep trying until we get a non-out, non-colliding item */
		ftotal = 0;
		skip_rep = 0;
		do {
			retry_descent = 0;
			in = bucket;               /* initial bucket */

			/* choose through intervening buckets */
			flocal = 0;
			do {
				collide = 0;
				retry_bucket = 0;
				r = rep + parent_r;
				/* r' = r + f_total */
				r += ftotal;

				/* bucket choose */
				if (in->size == 0) {
					reject = 1;
					goto reject;
				}
				if (local_fallback_retries > 0 &&
				    flocal >= (in->size>>1) &&
				    flocal > local_fallback_retries)
					item = bucket_perm_choose(in, x, r);
				else
					item = crush_bucket_choose(in, x, r);
				if (item >= map->max_devices) {
					dprintk("   bad item %d\n", item);
					skip_rep = 1;
					break;
				}

				/* desired type? */
				if (item < 0)
					itemtype = map->buckets[-1-item]->type;
				else
					itemtype = 0;
				dprintk("  item %d type %d\n", item, itemtype);

				/* keep going? */
				if (itemtype != type) {
					if (item >= 0 ||
					    (-1-item) >= map->max_buckets) {
						dprintk("   bad item type %d\n", type);
						skip_rep = 1;
						break;
					}
					in = map->buckets[-1-item];
					retry_bucket = 1;
					continue;
				}

				/* collision? */
				for (i = 0; i < outpos; i++) {
					if (out[i] == item) {
						collide = 1;
						break;
					}
				}

				reject = 0;
				if (!collide && recurse_to_leaf) {
					if (item < 0) {
						int sub_r;
						if (vary_r)
							sub_r = r >> (vary_r-1);
						else
							sub_r = 0;
						if (crush_choose_firstn(map,
							 map->buckets[-1-item],
							 weight, weight_max,
							 x, outpos+1, 0,
							 out2, outpos, count,
							 recurse_tries, 0,
							 local_retries,
							 local_fallback_retries,
							 0,
							 vary_r,
							 NULL,
							 sub_r) <= outpos)
							/* didn't get leaf */
							reject = 1;
					} else {
						/* we already have a leaf! */
						out2[outpos] = item;
					}
				}

				if (!reject) {
					/* out? */
					if (itemtype == 0)
						reject = is_out(map, weight,
								weight_max,
								item, x);
					else
						reject = 0;
				}

reject:
				if (reject || collide) {
					ftotal++;
					flocal++;

					if (collide && flocal <= local_retries)
						/* retry locally a few times */
						retry_bucket = 1;
					else if (local_fallback_retries > 0 &&
						 flocal <= in->size + local_fallback_retries)
						/* exhaustive bucket search */
						retry_bucket = 1;
					else if (ftotal < tries)
						/* then retry descent */
						retry_descent = 1;
					else
						/* else give up */
						skip_rep = 1;
					dprintk("  reject %d  collide %d  "
						"ftotal %u  flocal %u\n",
						reject, collide, ftotal,
						flocal);
				}
			} while (retry_bucket);
		} while (retry_descent);

		if (skip_rep) {
			dprintk("skip rep\n");
			continue;
		}

		dprintk("CHOOSE got %d\n", item);
		out[outpos] = item;
		outpos++;
		count--;

		if (map->choose_tries && ftotal <= map->choose_total_tries)
			map->choose_tries[ftotal]++;
	}

	dprintk("CHOOSE returns %d\n", outpos);
	return outpos;
}
{% endhighlight %}



<br />
<br />
<br />


