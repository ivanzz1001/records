---
layout: post
title: 分布式一致性哈希算法原理
tags:
- 分布式系统理论
categories: distribute-systems
description: 分布式系统一致性哈希算法原理
---

本文我们介绍一下分布式一致性哈希算法的基本原理。

<!-- more -->


## 1. 分布式算法

在做服务器负载均衡的时候，可供选择的负载均衡的算法有很多，包括： 轮询算法（Round Robin)、哈希算法（Hash）、最少连接算法（Least Connection）、响应速度算法(Response Time)、加权法(Weighted)等。其中哈希算法是最为常用的算法。

典型的应用场景是： 有N台服务器提供缓存服务，需要对服务器进行负载均衡，将请求平均分发到每台服务器上，每台机器负责1/N的服务。

常用的算法是对hash结果取余数(hash() mod N): 对机器编号从0到N-1，按照自定义的hash()算法，对每个请求的hash()值按N取模，得到```余数i```，然后将请求分发到编号为```i```的机器。但这样的算法存在致命问题，如果某一台机器宕机，那么应该落在该机器上的请求就无法得到正确的处理，这时需要将宕掉的服务器从算法中移除，此时会有```(N-1)/N```的服务器的缓存数据需要重新计算；如果新增一台机器，会有```N/(N+1)```的服务器的缓存数据需要进行重新计算。对于系统而言，这通常是不可接受的颠簸（因为这意味着大量缓存的失效或者数据需要转移）。那么，如何设计一个负载均衡策略，使得受到影响的请求尽可能的少呢？

在Memcached、Key-Value Store、Bittorrent DHT、LVS中都采用了Consistent Hashing算法，可以说Consistent Hashing是分布式系统负载均衡的首选算法。


## 2. 分布式缓存问题
在大型web应用中，缓存可以算是当今的一个标准开发配置了。在大规模的缓存应用中，应运而生了分布式缓存系统。分布式缓存系统的基本原理，大家也有所耳闻。key-value如何均匀的分散到集群中？ 说到此，最常规的方式莫过于hash取模的方式。比如集群中可用机器数量为```N```，那么key值为```K```的数据请求很简单的应该路由到```hash(K) mod N```对应的机器。的确，这种结构是简单的，也是实用的。但是在一些高速发展的Web系统中，这样的解决方案仍有些缺陷。随着系统访问压力的增长，缓存系统不得不通过增加机器节点的方式提高集群的响应速度和数据承载量。增加机器意味着按照hash取模的方式，在增加机器节点的这一时刻，大量的缓存命不中，缓存数据需要重新建立，甚至是进行整体的缓存数据迁移，瞬间会给DB带来极高的系统负载，甚至导致DB服务器宕机。那么就没有办法解决hash取模的方式带来的诟病吗？


假设我们有一个网站，最近发现随着流量的增加，服务器压力越来越大，之前直接读写数据库的方式不太给力了，于是我们想引入```Memcached```作为缓存机制。现在我们一共有三台机器可以作为Memcached服务器，如下图所示：


![hc-memcached](https://ivanzz1001.github.io/records/assets/img/distribute/hc_memcached_system.png)

很显然，最简单的策略是将每一次Memcached请求随机发送到一台Memcached服务器，但是这种策略可能会带来两个问题： 一是同一份数据可能被存在不同的机器上而造成数据冗余； 二是有可能某数据已经被缓存但是访问却没有命中，因为无法保证对相同key的所有访问都被发送到相同的服务器。

要解决上述问题只需做到如下一点： 保证对相同key的访问会被发送到相同的服务器。很多方法可以实现这一点，最常用的方法是计算哈希。例如对于每次访问，可以按如下算法计算其哈希值：
{% highlight string %}
h = Hash(key) % 3
{% endhighlight %}

其中Hash是从一个字符串到正整数的哈希映射函数。这样，如果我们将Memcached Server分别编号为0、1、2，那么就可以根据上式和key计算出服务器编号```h```，然后去访问。

这个方法虽然解决了上面提到的两个问题，但是存在一些其它的问题。如果将上述方法抽象，可以认为通过：
{% highlight string %}
h = Hash(key) % N
{% endhighlight %}
这个算式计算每个key的请求应该被发送到哪台服务器，其中```N```为服务器的台数，并且服务器按照```0 ~ (N-1)```编号。
 

这个算法的问题在于容错性和扩展性不好。所谓容错性是指当系统中某一个或几个服务器变得不可用时，整个系统是否可以正确高效运行； 而扩展性是指当加入新的服务器后，整个系统是否可以正确高效运行。


现假设有一台服务器宕机了，那么为了填补空缺，要将宕机的服务器从编号列表中移除，后面的服务器按顺序前移一位并将其编号值```减1```，此时每个key就要按 **h = Hash(key) % (N-1)** 重新计算； 同样，如果新增了一台服务器，虽然原有服务器编号不用改变，但是要按 **h = Hash(key) % (N+1)** 重新计算哈希值。因此系统中一旦有服务器变更，大量的key会被重定位到不同的服务器，从而造成大量的缓存不能命中。这这种情况在分布式系统中是非常糟糕的。

一个设计良好的分布式哈希方案应该具有良好的单调性，即服务节点的增减不会造成大量哈希重定位。一致性哈希算法就是这样一种哈希方案。

Hash算法的一个衡量指标是```单调性```(Monotonicity)，定义如下：单调性是指如果已经有一些内容通过哈希分配到了相应的缓冲中，又有新的缓冲加入到系统中，哈希的结果应该能够保证原有已分配的内容可以被映射到新的缓冲中去，而不会被映射到旧的缓冲集合中的其他缓冲区。


容易看到，上面的简单hash算法**hash(object) % N**难以满足单调性要求。

## 3. 一致性Hash算法背景

一致性哈希算法在1997年由麻省理工学院的Karger等人在解决分布式Cache中提出的，设计目标是为了解决因特网中的热点(Hot spot)问题，初衷和CARP十分类似。一致性哈希修正了CARP使用的简单哈希算法带来的问题，使得DHT可以在P2P环境中真正得到应用。

但现在一致性hash算法在分布式系统中也得到了广泛应用，研究过memcached缓存数据库的人都知道，memcached服务器端本身不提供分布式cache的一致性，而是由客户端来提供，具体在计算一致性hash时采用如下步骤：

1) 首先求出memcached服务器（节点）的哈希值，并将其配置到```0~2^32```的圆(continuum)上；

2) 然后采用同样的方法求出存储数据的键的哈希值，并映射到相同的圆上；

3） 然后从数据映射到的位置开始顺时针查找，将数据保存到找到的第一个服务器上。如果超过2^32仍找不到服务器，就会保存到第一台memcached服务器上。

![hc-figure-1](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_1.png)

从上图的状态中添加一台memcached服务器。余数分布式算法由于保存键的服务器会发生巨大变化而影响缓存的命中率，但Consistent Hashing中，只有在圆(continuum)上增加服务器的地点逆时针方向的第一台服务器上的键会受到影响，如下图所示：

![hc-figure-2](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_2.png)


## 4. 一致性Hash性质
考虑到分布式系统每个节点都有可能失效，并且新的节点很可能动态的增加进来，如何保证当系统的节点数目发生变化时仍然能够对外提供良好的服务，这是值得考虑的，尤其是在设计分布式缓存系统时，如果某台服务器失效，对于整个系统来说如果不采用合适的算法来保证一致性，那么缓存于系统中的所有数据都都可能会失效（即由于系统节点数目变少，客户端在请求某一对象时需要重新计算其hash值（通常与系统中的节点数目有关），由于hash值已经改变，所以很可能找不到保存该对象的服务器节点），因此一致性hash就显得至关重要，良好的分布式cache系统中的一致性hash算法应该满足以下几个方面：


* **平衡性(Balance)**

平衡性是指哈希的结果能够尽可能分布到所有的缓冲中去，这样可以使得所有的缓冲空间都得到利用。很多哈希算法都能够满足这一条件。


* **单调性(Monotonicity)**

单调性是指如果已经有一些内容通过哈希分派到了相应的缓冲中，又有新的缓冲区加入到系统中，那么哈希的结果应能够保证原有已分配的内容可以被映射到新的缓冲区中去，而不会被映射到旧的缓冲集合中的其他缓冲区。简单的哈希算法往往不能满足单调性的要求，如最简单的线性哈希： x = (ax + b) mod (P)，在上式中，```P```表示全部缓冲的大小。不难看出，当缓冲大小发生变化时（从P1到P2)，原来所有的哈希结果均会发生变化，从而不满足单调性要求。哈希结果的变化意味着当缓冲空间发生变化时，所有的映射关系需要在系统内全部更新。而在P2P系统内，缓冲的变化等价于Peer的加入或退出系统，这一情况在P2P系统中会频繁发生，因此会带来极大计算和传输负荷。单调性就是要求哈希算法能够应对这种情况。


* **分散性(Spread)**

在分布式环境中，终端有可能看不到所有的缓冲，而是只能看到其中的一部分。当终端希望通过哈希过程将内容映射到缓冲上时，由于不同终端所见的缓冲范围有可能不同，从而导致哈希的结果不一致，最终的结果是相同的内容被不同的终端映射到不同的缓冲区中。这种情况显然是应该避免的，因为它导致相同内容被存储到不同的缓冲中去，降低了系统存储的效率。分散性的定义就是上述情况发生的严重程度。好的哈希算法应能够尽量避免不一致的情况发生，也就是尽量降低分散性。


* **负载(Load)**

负载问题实际上是从另一个角度看待分散性问题。既然不同的终端可能将相同的内容映射到不同的缓冲区中，那么对于一个特定的缓冲区而言，也可能被不同的用户映射为不同的内容。与分散性一样，这种情况也是应当避免的，因此好的哈希算法应能够尽量降低缓冲的负荷。


* **平滑性(Smoothness)**

平滑性是指缓存服务器的数目平滑改变和缓存对象的平滑改变是一致的。


## 5. 一致性Hash原理

一致性哈希算法(Consistent Hashing)最早在论文[<<Consistent Hashing and Random Trees: Distributed Caching Protocols for Relieving Hot Spots on the World Wide Web>>](https://dl.acm.org/citation.cfm?id=258660)中被提出。简单来说，一致性哈希将整个哈希值空间组织成一个虚拟的圆环，如假设某哈希函数```H```的值空间为```[0,2^32-1]```（即哈希值是一个32位无符号整形），整个哈希空间环如下：

![hc-figure-3](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_3.png)

整个空间按顺时钟方向组织。0和2^32-1在零点方向重合。

下一步将各个服务器使用Hash算法进行一次哈希计算，具体可以选择服务器的IP或主机名作为关键字进行哈希，这样每台机器就能确定其在哈希环上的位置。这里假设将上文中的四台服务器使用IP地址哈希后在环空间的位置如下：


![hc-figure-4](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_4.png)

接下来使用如下算法定位数据访问到相应服务器： 将数据```key```使用相同的Hash函数计算出哈希值，并确定此数据在环上的位置，从此位置沿环顺时钟```行走```，第一台遇到的服务器就是其应该定位到的服务器。

例如我们有Object A、Object B、Object C、Object D四个数据对象，经过哈希计算后，在环空间上的位置如下：

![hc-figure-5](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_5.png)

根据一致性哈希算法，数据A会被定位到```Node A```上， B被定位到```Node B```上， C被定位到```Node C```上， D被定位到```Node D```上。


下面分析一致性哈希算法的```容错性```和```可扩展性```。现假设```Node C```不幸宕机，可以看到此时对象A、B、D不会受到影响，只有C对象被定位到```Node D```。一般的，在一致性哈希算法中，如果一台服务器不可用，则受到影响的数据仅仅是此服务器到其环空间中前一台服务器（即沿着逆时针方向行走遇到的第一台服务器）之间的数据，其他不会受到影响。



下面考虑另外一种情况，如果在系统中增加一台服务器```Node X```，如下图所示：

![hc-figure-6](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_6.png)

此时对象Object A、B、D不受影响，只有对象C需要重定位到新的```Node X```。一般的，在一致性哈希算法中，如果增加一台服务器，则受影响的数据仅仅是新服务器到其环空间中前一台服务器（即沿着逆时针方向行走遇到的第一台服务器）之间的数据，其他数据也不会受到影响。



综上所述，一致性哈希算法对于节点的增减都只需重定位环空间中的一小部分数据，具有较好的容错性和可扩展性。


另外，一致性哈希算法在服务节点太少时，容易因为节点分布不均匀而造成数据倾斜问题。例如，系统中只有两台服务器，其环分布如下：


![hc-figure-7](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_7.png)

此时必然造成大量数据集中到```Node A```上，而只有极少量会定位到```Node B```上。为了解决这种数据倾斜问题，一致性哈希算法引入了虚拟节点机制，即对每一个服务节点计算多个哈希，每个计算结果位置都放置一个此服务节点，称为```虚拟节点```。具体做法可以在```服务器IP```或```主机名```的后面增加编号来实现。例如上面的情况，可以为每台服务器计算三个虚拟节点，于是可以分别计算```Node A#1```、```Node A#2```、```Node A#3```、```Node B#1```、```Node B#2```、```Node B#3```的哈希值，于是形成6个虚拟节点：

![hc-figure-8](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_8.png)

同时数据定位算法不变，只是多了一步虚拟节点到实际节点的映射。例如定位到```Node A#1```、```Node A#2```、```Node A#3```三个虚拟节点的数据均定位到```Node A```上。这样就解决了服务节点少时数据倾斜的问题。在实际应用中，通常将虚拟节点数设置为32甚至更大，因此即使很少的服务节点也能做到相对均匀的数据分布。

## 5. 一致性哈希的Java实现

### 5.1 简单情况
Java实现中用什么表示Hash环好呢？ 经对比，用TreeMap的时间复杂度是O(logN)，相对效率比较高，因为TreeMap使用了红黑树结构存储实体对象。

Hash算法的选择上，首先我们考虑简单的String.HashCode()方法，这个算法的缺点是，相似的字符串如N0(10.0.0.0:91001)、N1(10.0.0.0:91002)、N2(10.0.0.0:91003)，哈希值也很接近，造成的结果是节点在Hash环上的分布很紧密，导致大部分key值落到N0上，节点资源分布不均。一般我们采用```FNV1_32_HASH```、```KETAMA_HASH```等算法，KETAMA_HASH是MemCache集群默认的实现方法，这些算法效果要好很多，会使N0、N1、N2的Hash值更均匀的分布在环上。


下面我们用```KETAMA_HASH```算法实现一致性哈希（无虚拟节点方式），如下代码所示(```SimpleHashConsistency.java```)：
{% highlight string %}
package simple_hc;


import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.Map;
import java.util.TreeMap;


/**
 * Created by markcd on 2018/2/28.
 */
public class SimpleHashConsistency {

    private TreeMap<Long, String> realNodes = new TreeMap<>();
    private String[] nodes;

    
    public SimpleHashConsistency(String[] nodes){
        this.nodes = Arrays.copyOf(nodes, nodes.length);
        initalization();
    }

    
    /**
     * 初始化哈希环
     * 循环计算每个node名称的哈希值，将其放入treeMap
     */
    private void initalization(){
        for (String nodeName: nodes) {
            realNodes.put(hash(nodeName, 0), nodeName);
        }
    }

    
    /**
     * 根据资源key选择返回相应的节点名称
     * @param key
     * @return 节点名称
     */
    public String selectNode(String key){
        Long hashOfKey = hash(key, 0);
        if (! realNodes.containsKey(hashOfKey)) {
               //ceilingEntry()的作用是得到比hashOfKey大的第一个Entry
            Map.Entry<Long, String> entry = realNodes.ceilingEntry(hashOfKey);
            if (entry != null)
                return entry.getValue();
            else
                return nodes[0];
        }else
            return realNodes.get(hashOfKey);
    }

    
    private Long hash(String nodeName, int number) {
        byte[] digest = md5(nodeName);
        return (((long) (digest[3 + number * 4] & 0xFF) << 24)
                | ((long) (digest[2 + number * 4] & 0xFF) << 16)
                | ((long) (digest[1 + number * 4] & 0xFF) << 8)
                | (digest[number * 4] & 0xFF))
                & 0xFFFFFFFFL;
    }

    
    /**
     * md5加密
     *
     * @param str
     * @return
     */
    public byte[] md5(String str) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            md.reset();
            md.update(str.getBytes("UTF-8"));
            return md.digest();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            return null;
        }
    }

    
    private void printTreeNode(){
        if (realNodes != null && ! realNodes.isEmpty()){
            realNodes.forEach((hashKey, node) ->
                    System.out.println(
                            new StringBuffer(node)
                            .append(" ==> ")
                            .append(hashKey)
                    )
            );
        }else
            System.out.println("Cycle is Empty");
    }
    
    

    public static void main(String[] args){
        String[] nodes = new String[]{"192.168.2.1:8080", "192.168.2.2:8080", "192.168.2.3:8080", "192.168.2.4:8080"};
        SimpleHashConsistency consistentHash = new SimpleHashConsistency(nodes);
        consistentHash.printTreeNode();
    }
}
{% endhighlight %}
main()方法执行结果如下，可以看到，hash值分布的距离比较开阔:
{% highlight string %}
192.168.2.3:8080 ==> 1182102228
192.168.2.4:8080 ==> 1563927337
192.168.2.1:8080 ==> 2686712470
192.168.2.2:8080 ==> 3540412423
{% endhighlight %}


```KETAMA_HASH```解决了hash值分布不均匀的问题，但还存在一个问题，如下图所示，在没有```Node3```节点时，资源相对均匀的分布在{Node0, Node1, Node2}上。增加了Node3节点后，Node1到Node3节点中间的所有资源从Node2迁移到了Node3上。这样，Node0、Node1存储的资源多，Node2、Node3存储的资源少，资源分布不均匀。

![hc-figure-9](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_9.png)


### 5.2 带虚拟节点的一致性哈希
上面我们提到了资源分布不均匀的问题，要如何解决呢？ 我们引入虚拟节点概念，如将一个真实节点Node 0映射成100个虚拟节点分布在Hash环上，与这100个虚拟节点根据```KETAMA_HASH```哈希环相匹配的资源都存到真实节点```Node0```上。{Node0,Node1,Node2}以相同的方式拆分虚拟节点映射到Hash环上。当集群增加节点```Node3```时，在Hash环上增加Node3拆分的100个虚拟节点，这新增的100个虚拟节点更均匀的分布在了哈希环上，可能承担了{Node0,Node1,Node2}每个节点的部分资源，资源分布仍然保持均匀。


每个真实节点应该拆分成多少个虚拟节点？ 数量要合适才能保证负载分布的均匀，有一个大致的规律，如下图所示，```Y轴```表示真实节点的数目，```X轴```表示需拆分的虚拟节点数目：

![hc-figure-10](https://ivanzz1001.github.io/records/assets/img/distribute/hc_figure_10.png)

真实节点越少，所需拆分的虚拟节点越多。比如有10个真实节点，每个节点所需要拆分的虚拟节点个数可能是100~200个，才能达到真正的负载均衡。

下面贴出使用了虚拟节点的算法实现(```VirtualNodeHashConsistency.java```):
{% highlight string %}
package vnode_hc;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.LinkedList;
import java.util.Map;
import java.util.TreeMap;

/**
 * Created by markcd on 2018/2/28.
 */
public class VirtualNodeHashConsistency {

    private TreeMap<Long, String> virtualNodes = new TreeMap<>();
    private LinkedList<String> nodes;
        //每个真实节点对应的虚拟节点数
    private final int replicCnt;
    

    public VirtualNodeHashConsistency(LinkedList<String> nodes, int replicCnt){
        this.nodes = nodes;
        this.replicCnt = replicCnt;
        initalization();
    }

    /**
     * 初始化哈希环
     * 循环计算每个node名称的哈希值，将其放入treeMap
     */
    private void initalization(){
        for (String nodeName: nodes) {
            for (int i = 0; i < replicCnt/4; i++) {
                String virtualNodeName = getNodeNameByIndex(nodeName, i);
                for (int j = 0; j < 4; j++) {
                    virtualNodes.put(hash(virtualNodeName, j), nodeName);
                }
            }
        }
    }

    private String getNodeNameByIndex(String nodeName, int index){
        return new StringBuffer(nodeName)
                .append("&&")
                .append(index)
                .toString();
    }

    /**
     * 根据资源key选择返回相应的节点名称
     * @param key
     * @return 节点名称
     */
    public String selectNode(String key){
        Long hashOfKey = hash(key, 0);
        if (! virtualNodes.containsKey(hashOfKey)) {
            Map.Entry<Long, String> entry = virtualNodes.ceilingEntry(hashOfKey);
            if (entry != null)
                return entry.getValue();
            else
                return nodes.getFirst();
        }else
            return virtualNodes.get(hashOfKey);
    }

    private Long hash(String nodeName, int number) {
        byte[] digest = md5(nodeName);
        return (((long) (digest[3 + number * 4] & 0xFF) << 24)
                | ((long) (digest[2 + number * 4] & 0xFF) << 16)
                | ((long) (digest[1 + number * 4] & 0xFF) << 8)
                | (digest[number * 4] & 0xFF))
                & 0xFFFFFFFFL;
    }

    /**
     * md5加密
     *
     * @param str
     * @return
     */
    public byte[] md5(String str) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            md.reset();
            md.update(str.getBytes("UTF-8"));
            return md.digest();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            return null;
        }
    }

    public void addNode(String node){
        nodes.add(node);
        
        for (int i = 0; i < replicCnt/4; i++) {
        	String virtualNodeName = getNodeNameByIndex(node, i);
        	
            for (int j = 0; j < 4; j++) {
                virtualNodes.put(hash(virtualNodeName, j), node);
            }
        }
    }

    public void removeNode(String node){
        nodes.remove(node);
       
        for (int i = 0; i < replicCnt/4; i++) {
        	 String virtualNodeName = getNodeNameByIndex(node, i);
        	 
            for (int j = 0; j < 4; j++) {
                virtualNodes.remove(hash(virtualNodeName, j), node);
            }
        }
    }

    private void printTreeNode(){
        if (virtualNodes != null && ! virtualNodes.isEmpty()){
            virtualNodes.forEach((hashKey, node) ->
                    System.out.println(
                            new StringBuffer(node)
                                    .append(" ==> ")
                                    .append(hashKey)
                    )
            );
        }else
            System.out.println("Cycle is Empty");
    }

    
    public static void main(String[] args){
        LinkedList<String> nodes = new LinkedList<>();
        nodes.add("192.168.2.1:8080");
        nodes.add("192.168.2.2:8080");
        nodes.add("192.168.2.3:8080");
        nodes.add("192.168.2.4:8080");
        VirtualNodeHashConsistency consistentHash = new VirtualNodeHashConsistency(nodes, 160);
        consistentHash.printTreeNode();
    }
}
{% endhighlight %}

以上main方法执行的结果如下：
{% highlight string %}
192.168.2.4:8080 ==> 18075595
192.168.2.1:8080 ==> 18286704
192.168.2.1:8080 ==> 35659769
192.168.2.2:8080 ==> 43448858
192.168.2.1:8080 ==> 44075453
192.168.2.3:8080 ==> 47625378
192.168.2.4:8080 ==> 52449361
192.168.2.2:8080 ==> 53176589
192.168.2.4:8080 ==> 53206362
192.168.2.2:8080 ==> 54789163
192.168.2.3:8080 ==> 78933624
192.168.2.2:8080 ==> 84809132
192.168.2.1:8080 ==> 116518130
192.168.2.2:8080 ==> 116682394
....
....
{% endhighlight %}


<br />
<br />

**[参看]**

1. [一致性哈希算法原理](https://www.cnblogs.com/lpfuture/p/5796398.html)

2. [分布式算法(一致性Hash算法)](https://www.cnblogs.com/moonandstar08/p/5405991.html)

3. [一致性哈希算法原理分析及实现](https://www.cnblogs.com/markcd/p/8476237.html)

4. [对一致性Hash算法，Java代码实现的深入研究](https://www.cnblogs.com/xrq730/p/5186728.html)

<br />
<br />
<br />


