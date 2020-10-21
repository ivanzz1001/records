---
layout: post
title: OSD从down到out过程中osdmap的变化
tags:
- ceph
categories: ceph
description: ceph pg
---



本文主要讲述一个初始状态为active+clean了ceph集群，从OSDmap及PGMap的视角观察一个OSD从(in + up)状态到(in + down)状态，再到(out + down)状态这一过程的变化，并结合OSD的日志信息以期对PG peering过程有一个初步的了解。

<!-- more --> 


## 1. 集群环境介绍
当前我们有如下9个OSD组成的ceph集群：
<pre>
# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.34998 failure-domain sata-00                                                     
 -9 1.34998     replica-domain replica-0                                               
 -8 0.44998         host-domain host-group-0-rack-01                                   
 -2 0.44998             host node7-1                                                   
  2 0.14999                 osd.2                         up  1.00000          1.00000 
  0 0.14999                 osd.0                         up  1.00000          1.00000 
  1 0.14999                 osd.1                         up  1.00000          1.00000 
-11 0.45000         host-domain host-group-0-rack-02                                   
 -4 0.45000             host node7-2                                                   
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
-12 0.45000         host-domain host-group-0-rack-03                                   
 -6 0.45000             host node7-3                                                   
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 -1 1.34998 root default                                                               
 -3 0.44998     rack rack-01                                                           
 -2 0.44998         host node7-1                                                       
  2 0.14999             osd.2                             up  1.00000          1.00000 
  0 0.14999             osd.0                             up  1.00000          1.00000 
  1 0.14999             osd.1                             up  1.00000          1.00000 
 -5 0.45000     rack rack-02                                                           
 -4 0.45000         host node7-2                                                       
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 -7 0.45000     rack rack-03                                                           
 -6 0.45000         host node7-3                                                       
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
# ceph -s
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_OK
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24440, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2222: 9 osds: 9 up, 9 in
            flags sortbitwise,require_jewel_osds
      pgmap v5839062: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76122 MB used, 225 GB / 299 GB avail
                 632 active+clean

</pre>
集群的初始状态为```active+clean```状态，PG的个数有632个。当前osdmap的版本为```e2222```，pgmap的版本为```v5839062```。


另外所采用的crush如下：
{% highlight string %}
$ ceph osd getcrushmap -o ./oss_crushmap.bin
$ crushtool -d ./oss_crushmap.bin -o ./oss_crushmap.txt

$ cat ./oss_crushmap.txt
# begin crush map
tunable choose_local_tries 0
tunable choose_local_fallback_tries 0
tunable choose_total_tries 50
tunable chooseleaf_descend_once 1
tunable chooseleaf_vary_r 1
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
        item osd.2 weight 0.150
        item osd.0 weight 0.150
        item osd.1 weight 0.150
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
        ruleset 2
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
{% endhighlight %}

之后我们在集群的三台主机上创建本次数据抓取工作目录：
<pre>
# mkdir workdir
# cd workdir
</pre>


1） **获取初始状态下的osdmap**

在node7-1主机上，通过如下命令获取出当前状态的osdmap信息：
{% highlight string %}
# ceph osd dump > osdmap_active_clean.txt
# cat osdmap_active_clean.txt 
epoch 2222
fsid 5341b139-15dc-4c68-925a-179797d894d3
created 2018-05-11 16:11:51.479785
modified 2020-09-11 12:13:01.076048
flags sortbitwise,require_jewel_osds
pool 11 '.rgw.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 170 flags hashpspool stripe_width 0
pool 12 'default.rgw.control' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 172 flags hashpspool stripe_width 0
pool 13 'default.rgw.data.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 174 flags hashpspool stripe_width 0
pool 14 'default.rgw.gc' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 1142 flags hashpspool stripe_width 0
pool 15 'default.rgw.log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 178 flags hashpspool stripe_width 0
pool 16 'default.rgw.intent-log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 180 flags hashpspool stripe_width 0
pool 17 'default.rgw.usage' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 182 flags hashpspool stripe_width 0
pool 18 'default.rgw.users.keys' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 184 flags hashpspool stripe_width 0
pool 19 'default.rgw.users.email' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 186 flags hashpspool stripe_width 0
pool 20 'default.rgw.users.swift' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 188 flags hashpspool stripe_width 0
pool 21 'default.rgw.users.uid' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 190 flags hashpspool stripe_width 0
pool 22 'default.rgw.buckets.index' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 192 flags hashpspool stripe_width 0
pool 23 'default.rgw.buckets.data' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 194 flags hashpspool stripe_width 0
pool 24 'default.rgw.meta' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 197 flags hashpspool stripe_width 0
pool 25 'default.rgw.buckets.non-ec' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 200 flags hashpspool stripe_width 0
max_osd 10
osd.0 up   in  weight 1 up_from 2220 up_thru 2221 down_at 2212 last_clean_interval [2206,2211) 10.17.155.113:6800/29412 10.17.155.113:6801/29412 10.17.155.113:6802/29412 10.17.155.113:6803/29412 exists,up 67990973-0316-4f57-b721-ce61d886572c
osd.1 up   in  weight 1 up_from 2119 up_thru 2220 down_at 2114 last_clean_interval [2045,2113) 10.17.155.113:6808/5393 10.17.155.113:6809/5393 10.17.155.113:6810/5393 10.17.155.113:6811/5393 exists,up e6c55932-820c-474d-aff7-0ed4081f2a33
osd.2 up   in  weight 1 up_from 2118 up_thru 2220 down_at 2093 last_clean_interval [2045,2092) 10.17.155.113:6804/5391 10.17.155.113:6805/5391 10.17.155.113:6806/5391 10.17.155.113:6807/5391 exists,up dfe500d8-5778-4379-9c48-1f1390fa8f0a
osd.3 up   in  weight 1 up_from 2123 up_thru 2221 down_at 2095 last_clean_interval [2041,2093) 10.17.155.114:6800/894 10.17.155.114:6801/894 10.17.155.114:6802/894 10.17.155.114:6803/894 exists,up e00c8fe5-d49e-42d1-9bfb-4965b9ab75b3
osd.4 up   in  weight 1 up_from 2125 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6804/6327 10.17.155.114:6805/6327 10.17.155.114:6806/6327 10.17.155.114:6807/6327 exists,up dede8fcc-0b34-4296-83e2-a48966b22c36
osd.5 up   in  weight 1 up_from 2127 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6808/6391 10.17.155.114:6809/6391 10.17.155.114:6810/6391 10.17.155.114:6811/6391 exists,up c2cde97e-c27a-4560-a46c-68695be79ff1
osd.6 up   in  weight 1 up_from 2040 up_thru 2221 down_at 2039 last_clean_interval [2020,2038) 10.17.155.115:6800/820 10.17.155.115:6801/820 10.17.155.115:6802/820 10.17.155.115:6803/820 exists,up d12f28d8-8fff-4a77-b344-d5d0b3e6949c
osd.7 up   in  weight 1 up_from 2072 up_thru 2221 down_at 2051 last_clean_interval [2015,2038) 10.17.155.115:6804/26346 10.17.155.115:6805/26346 10.17.155.115:6806/26346 10.17.155.115:6807/26346 exists,up 26035aa5-6759-4856-8bdb-1507f5b052e6
osd.8 up   in  weight 1 up_from 2087 up_thru 2221 down_at 2085 last_clean_interval [2074,2086) 10.17.155.115:6808/26484 10.17.155.115:6812/1026484 10.17.155.115:6813/1026484 10.17.155.115:6814/1026484 exists,up bca13e21-d64c-433f-87e4-4d5ea309f28a
{% endhighlight %}
上面看到当前的osdmap的版本号为```2222```，总共有9个osd，都处于```up + in```状态。

2) **获取初始状态下的pgmap**

使用如下命令获取初始状态的pgmap信息：
{% highlight string %}
# ceph pg dump > pgmap_active_clean.txt
# cat pgmap_active_clean.txt 
version 5839062
stamp 2020-09-11 13:57:11.859797
last_osdmap_epoch 2222
last_pg_scan 2222
full_ratio 0.95
nearfull_ratio 0.85
pg_stat objects mip     degr    misp    unf     bytes   log     disklog state   state_stamp     v       reported        up      up_primary      acting  acting_primary  last_scrub      scrub_stamp     last_deep_scrub deep_scrub_stamp
23.67   10      0       0       0       0       1095624955      56      56      active+clean    2020-09-11 04:33:30.434795      1956'56 2214:361        [1,7]   1       [1,7]   1       1956'56 2020-09-11 04:33:30.434723      1956'56 2020-09-09 18:30:40.201407
22.66   0       0       0       0       0       0       0       0       active+clean    2020-09-11 04:27:20.276408      0'0     2214:913        [8,2]   8       [8,2]   8       0'0     2020-09-11 04:27:20.276336      0'0     2020-09-06 09:41:03.238164
23.64   24      0       0       0       0       2194721422      99      99      active+clean    2020-09-10 16:23:37.876876      1962'99 2214:227        [3,2]   3       [3,2]   3       1962'99 2020-09-10 16:23:37.876778      1962'99 2020-09-10 16:23:37.876778
22.65   0       0       0       0       0       0       0       0       active+clean    2020-09-11 12:13:01.285076      0'0     2222:311        [0,8]   0       [0,8]   0       0'0     2020-09-10 08:49:02.107556      0'0     2020-09-05 09:58:02.887318
23.65   23      0       0       0       0       59042501        100     100     active+clean    2020-09-10 06:11:37.649942      1956'100        2214:2213       [4,7]   4       [4,7]   4       1956'100        2020-09-10 06:11:37.649843      1956'100 2020-09-06 16:56:41.273324
22.64   1       0       0       0       0       0       3000    3000    active+clean    2020-09-10 18:52:08.198539      972'4247        2214:5853       [2,3]   2       [2,3]   2       972'4247        2020-09-10 18:52:08.198492      972'4247        2020-09-10 18:52:08.198492
23.62   19      0       0       0       0       24117248        68      68      active+clean    2020-09-10 19:26:25.938796      1956'68 2214:314        [3,1]   3       [3,1]   3       1956'68 2020-09-10 19:26:25.938730      1956'68 2020-09-09 17:38:24.900539
22.63   1       0       0       0       0       0       2       2       active+clean    2020-09-11 08:56:04.770887      515'2   2214:292        [5,8]   5       [5,8]   5       515'2   2020-09-11 08:56:04.770809      515'2   2020-09-08 02:11:32.447972
23.63   15      0       0       0       0       27802844        57      57      active+clean    2020-09-11 09:53:32.145516      1956'57 2214:251        [3,7]   3       [3,7]   3       1956'57 2020-09-11 09:53:32.145446      1956'57 2020-09-10 08:38:25.599452
22.62   1       0       0       0       0       0       2       2       active+clean    2020-09-11 10:36:05.768238      201'2   2214:2438       [5,1]   5       [5,1]   5       201'2   2020-09-11 10:36:05.768173      201'2   2020-09-11 10:36:05.768173
23.60   28      0       0       0       0       3267227406      107     107     active+clean    2020-09-10 06:04:01.345274      1956'107        2214:316        [2,4]   2       [2,4]   2       1956'107        2020-09-10 06:04:01.345229      1956'107 2020-09-10 06:04:01.345229
22.61   0       0       0       0       0       0       0       0       active+clean    2020-09-10 20:25:28.624057      0'0     2214:905        [6,3]   6       [6,3]   6       0'0     2020-09-10 20:25:28.623983      0'0     2020-09-10 20:25:28.623983
23.61   25      0       0       0       0       31142253        67      67      active+clean    2020-09-11 12:13:29.734713      1956'67 2222:590        [0,4]   0       [0,4]   0       1956'67 2020-09-11 09:58:39.802565      1956'67 2020-09-11 09:58:39.802565
22.60   0       0       0       0       0       0       0       0       active+clean    2020-09-10 23:03:35.867956      0'0     2214:271        [7,5]   7       [7,5]   7       0'0     2020-09-10 23:03:35.867741      0'0     2020-09-08 12:41:53.131348
23.5e   21      0       0       0       0       1085440156      82      82      active+clean    2020-09-10 18:02:11.253557      1956'82 2214:322        [6,4]   6       [6,4]   6       1956'82 2020-09-10 18:02:11.253467      1956'82 2020-09-09 12:43:46.534538
....
{% endhighlight %}
上面pgmap的版本号为```5839062```。


接着通过如下命令获得```osd.0```上的PG个数（结果为139）：
<pre>
# ceph pg ls-by-osd 0
</pre>
查看ceph数目目录下的current文件夹，发现其中的```_head```文件的个数也刚好是139:
<pre>
# pwd
/var/lib/ceph/osd/ceph-0/current
# ls -al | grep _head | wc -l
139
</pre>

然后我们再获取以```osd.0```作为主OSD的PG个数(结果为68)：
<pre>
# ceph pg ls-by-primary 0 | awk '{print $1}'
pg_stat
11.4
13.4
13.9
13.d
14.3
14.5
14.8
14.d
18.2
18.4
19.2
20.3
21.1
22.e
22.21
22.2c
22.3a
22.44
22.4a
22.65
22.6b
22.73
22.79
22.92
22.96
22.a2
22.a4
22.a7
22.c4
22.ca
22.cb
22.ce
22.d7
22.e2
22.ec
22.f1
23.b
23.d
23.1e
23.26
23.30
23.37
23.38
23.61
23.72
23.75
23.76
23.79
23.7a
23.7b
23.7e
23.89
23.96
23.a1
23.a7
23.ae
23.bb
23.be
23.cb
23.cf
23.d2
23.d6
23.d7
23.e0
23.e4
23.ea
23.fa
24.5
</pre>


3) **修改OSD3的日志打印级别**

首先执行如下命令查看当前osd3的日志打印级别：
<pre>
# ceph daemon osd.3 config show | grep debug_osd
    "debug_osd": "0\/5",
# ceph daemon osd.3 config show | grep debug_monc
    "debug_monc": "0\/10",
</pre>
接着执行如下命令更改打印级别：
<pre>
# ceph daemon osd.3 config set debug_osd 30/30
{
    "success": ""
}

# ceph daemon osd.3 config set debug_monc 30/30
{
    "success": ""
}

# ceph daemon osd.3 config show | grep debug_osd
    "debug_osd": "30\/30",
# ceph daemon osd.3 config show | grep debug_monc
    "debug_monc": "30\/30",

</pre>
可以看到当前已经修改成功。我们再检查一下osd.3的日志，以确保日志级别已经修改成功。


4） **修改osd0的日志打印级别**

同上，我们也修改osd0的日志打印级别：
<pre>
# ceph daemon osd.0 config set debug_osd 30/30
{
    "success": ""
}

# ceph daemon osd.0 config set debug_monc 30/30
{
    "success": ""
}

# ceph daemon osd.0 config show | grep debug_osd
    "debug_osd": "30\/30",
# ceph daemon osd.0 config show | grep debug_monc
    "debug_monc": "30\/30",
</pre>

4） **修改mon的日志打印级别**

首先执行如下命令查看当前mon的leader:
<pre>
# ceph mon_status
{"name":"node7-1","rank":0,"state":"leader","election_epoch":24434,"quorum":[0,1,2],"outside_quorum":[],"extra_probe_peers":[],"sync_provider":[],"monmap":{"epoch":3,"fsid":"5341b139-15dc-4c68-925a-179797d894d3","modified":"2018-05-11 17:10:27.830117","created":"2018-05-11 14:31:17.538703","mons":[{"rank":0,"name":"node7-1","addr":"10.17.155.113:6789\/0"},{"rank":1,"name":"node7-2","addr":"10.17.155.114:6789\/0"},{"rank":2,"name":"node7-3","addr":"10.17.155.115:6789\/0"}]}}
</pre>
这里我们看到leader是```node7-1```，因此这里我们先查看该monitor的日志级别：
<pre>
# ceph daemon mon.node7-1 config show | grep debug_mon
    "debug_mon": "1\/5",
    "debug_monc": "0\/10",
</pre>
接着执行如下命令更改monitor的日志打印级别：
<pre>
# ceph daemon mon.node7-1 config set debug_mon 30/30
{
    "success": ""
}
</pre>
可以看到当前已经修改成功。我们再检查一下mon.node7-1的日志，以确保日志级别已经修改成功。


## 2. 准备阶段
在我们完成上面的日志级别的调整后，我们重定向相关日志：

1） **重定向osd.0日志**

我们将osd.0的日志重定向到一个新文件中(osd0_watch.txt):
{% highlight string %}
# tail -f /var/log/ceph/ceph-osd.0.log > ./osd0_watch.txt
{% endhighlight %}

2) **重定向osd.3日志**

我们将osd.3的日志重定向到一个新的文件中(osd3_watch.txt):
{% highlight string %}
# tail -f /var/log/ceph/ceph-osd.3.log > ./osd3_watch.txt
{% endhighlight %}

3) **重定向mon.node7-1日志**

我们将mon.node7-1的日志重定向到一个新的文件中(mon-node7-1.txt):
{% highlight string %}
# tail -f /var/log/ceph/ceph-mon.node7-1.log > ./mon-node7-1.txt
{% endhighlight %}

## 3. osd.0处于(in+down)状态
1) **关闭osd.0，并观察集群状态**

执行如下命令手动关闭```osd.0```，并观察集群状态：
<pre>
# systemctl stop ceph-osd@0
# ceph -s
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_ERR
            17 pgs are stuck inactive for more than 300 seconds
            119 pgs degraded
            20 pgs peering
            17 pgs stuck inactive
            133 pgs stuck unclean
            119 pgs undersized
            recovery 1169/11720 objects degraded (9.974%)
            1/9 in osds are down
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24440, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2225: 9 osds: 8 up, 9 in; 139 remapped pgs
            flags sortbitwise,require_jewel_osds
      pgmap v5839107: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76124 MB used, 225 GB / 299 GB avail
            1169/11720 objects degraded (9.974%)
                 493 active+clean
                 119 active+undersized+degraded
                  20 peering
recovery io 0 B/s, 5 keys/s, 0 objects/s
</pre>
可以看到停止osd.0之后，集群马上出现```HEALTH_ERR```状态。接着再执行```ceph -w```命令：
<pre>
# ceph -w
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_WARN
            139 pgs degraded
            133 pgs stuck unclean
            139 pgs undersized
            recovery 1301/11720 objects degraded (11.101%)
            1/9 in osds are down
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24440, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2225: 9 osds: 8 up, 9 in; 139 remapped pgs
            flags sortbitwise,require_jewel_osds
      pgmap v5839112: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76124 MB used, 225 GB / 299 GB avail
            1301/11720 objects degraded (11.101%)
                 493 active+clean
                 139 active+undersized+degraded

2020-09-11 14:05:32.824053 mon.0 [INF] pgmap v5839112: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 76124 MB used, 225 GB / 299 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-11 14:05:37.849532 mon.0 [INF] HEALTH_WARN; 139 pgs degraded; 133 pgs stuck unclean; 139 pgs undersized; recovery 1301/11720 objects degraded (11.101%); 1/9 in osds are down
2020-09-11 14:06:37.850136 mon.0 [INF] HEALTH_WARN; 139 pgs degraded; 135 pgs stuck unclean; 139 pgs undersized; recovery 1301/11720 objects degraded (11.101%); 1/9 in osds are down
</pre>
可以看到这个时候集群状态变为```HEALTH_WARN```状态，其中处于```degraded```状态的PG个数是139，这刚好就是osd.0上PG的个数。

2) **导出集群当前osdmap及pgmap信息**

接着我们马上执行如下命令导出当前的osdmap及pgmap信息：
{% highlight string %}
# ceph osd dump > osdmap_in_down.txt
# ceph pg dump > pgmap_in_down.txt
dumped all in format plain
{% endhighlight %}


## 3. osd.0处于(out+down)状态
1） **查看ceph集群状态**

在osd.0停止约```5分钟```后，monitor会将osd.0标记为out状态：
<pre>
# ceph -w
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_WARN
            1 pgs backfill_wait
            76 pgs degraded
            5 pgs recovering
            70 pgs recovery_wait
            76 pgs stuck unclean
            1 pgs undersized
            recovery 2462/11720 objects degraded (21.007%)
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24440, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2231: 9 osds: 8 up, 8 in; 1 remapped pgs
            flags sortbitwise,require_jewel_osds
      pgmap v5839147: 632 pgs, 15 pools, 206 GB data, 5860 objects
            72034 MB used, 196 GB / 266 GB avail
            2462/11720 objects degraded (21.007%)
                 556 active+clean
                  70 active+recovery_wait+degraded
                   5 active+recovering+degraded
                   1 active+undersized+degraded+remapped+wait_backfill
recovery io 25022 kB/s, 25 objects/s

2020-09-11 14:10:18.902675 mon.0 [INF] osd.0 out (down for 300.095972)
2020-09-11 14:10:18.942928 mon.0 [INF] osdmap e2226: 9 osds: 8 up, 8 in
2020-09-11 14:10:18.961850 mon.0 [INF] pgmap v5839131: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 70540 MB used, 197 GB / 266 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-11 14:10:19.967084 mon.0 [INF] osdmap e2227: 9 osds: 8 up, 8 in
2020-09-11 14:10:19.973245 osd.7 [INF] 17.0 starting backfill to osd.2 from (0'0,0'0] MAX to 1745'17827
2020-09-11 14:10:19.988115 mon.0 [INF] pgmap v5839132: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 70540 MB used, 197 GB / 266 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-11 14:10:21.004820 mon.0 [INF] osdmap e2228: 9 osds: 8 up, 8 in
2020-09-11 14:10:21.042746 mon.0 [INF] pgmap v5839134: 632 pgs: 1 active+undersized+degraded+remapped+wait_backfill, 2 active+undersized+remapped, 2 active+undersized+degraded+remapped+backfilling, 60 peering, 74 active+undersized+degraded, 493 active+clean; 206 GB data, 70540 MB used, 197 GB / 266 GB avail; 743/11720 objects degraded (6.340%); 0 B/s, 1603 keys/s, 43 objects/s recovering
2020-09-11 14:10:19.971030 osd.8 [INF] 14.5 starting backfill to osd.2 from (0'0,0'0] MAX to 2222'235208
2020-09-11 14:10:22.047015 mon.0 [INF] osdmap e2229: 9 osds: 8 up, 8 in
2020-09-11 14:10:22.099375 mon.0 [INF] pgmap v5839136: 632 pgs: 4 active+recovery_wait+degraded, 2 active+recovering+degraded, 15 activating, 1 active+undersized+degraded+remapped+wait_backfill, 1 active+undersized+remapped, 3 active+undersized+degraded+remapped+backfilling, 61 peering, 17 active+undersized+degraded, 31 activating+degraded, 497 active+clean; 206 GB data, 70541 MB used, 197 GB / 266 GB avail; 351/11720 objects degraded (2.995%)
2020-09-11 14:10:19.967348 osd.5 [INF] 14.3 starting backfill to osd.2 from (0'0,0'0] MAX to 2222'417838
2020-09-11 14:10:19.975356 osd.5 [INF] 14.d starting backfill to osd.2 from (0'0,0'0] MAX to 2222'321710
2020-09-11 14:10:19.975419 osd.5 [INF] 14.8 starting backfill to osd.3 from (0'0,0'0] MAX to 2222'230561
2020-09-11 14:10:19.976943 osd.5 [INF] 14.8 starting backfill to osd.7 from (0'0,0'0] MAX to 2222'230561
2020-09-11 14:10:20.025833 osd.3 [INF] 15.5 starting backfill to osd.1 from (0'0,0'0] MAX to 2225'9330365
2020-09-11 14:10:23.142904 mon.0 [INF] osdmap e2230: 9 osds: 8 up, 8 in
2020-09-11 14:10:23.201075 mon.0 [INF] pgmap v5839138: 632 pgs: 5 active+recovery_wait+degraded, 3 active+recovering+degraded, 21 activating, 1 active+undersized+degraded+remapped+wait_backfill, 1 active+undersized+remapped, 3 active+undersized+degraded+remapped+backfilling, 61 peering, 39 activating+degraded, 498 active+clean; 206 GB data, 70541 MB used, 197 GB / 266 GB avail; 311/11720 objects degraded (2.654%)
2020-09-11 14:10:24.173223 mon.0 [INF] osdmap e2231: 9 osds: 8 up, 8 in
2020-09-11 14:10:24.192977 mon.0 [INF] pgmap v5839139: 632 pgs: 5 active+recovery_wait+degraded, 3 active+recovering+degraded, 21 activating, 1 active+undersized+degraded+remapped+wait_backfill, 1 active+undersized+remapped, 3 active+undersized+degraded+remapped+backfilling, 61 peering, 39 activating+degraded, 498 active+clean; 206 GB data, 70541 MB used, 197 GB / 266 GB avail; 311/11720 objects degraded (2.654%)
</pre>
这个过期时间可以通过如下命令查出:
<pre>
# ceph daemon mon.node7-2 config show | grep mon_osd_down_out_interval
    "mon_osd_down_out_interval": "300",
</pre>

再仔细观察ceph集群状态，可以发现开始出现```recovering```的动作。

接着使用```ceph osd tree```命令查看：
<pre>
# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.34998 failure-domain sata-00                                                     
 -9 1.34998     replica-domain replica-0                                               
 -8 0.44998         host-domain host-group-0-rack-01                                   
 -2 0.44998             host node7-1                                                   
  2 0.14999                 osd.2                         up  1.00000          1.00000 
  0 0.14999                 osd.0                       down        0          1.00000 
  1 0.14999                 osd.1                         up  1.00000          1.00000 
-11 0.45000         host-domain host-group-0-rack-02                                   
 -4 0.45000             host node7-2                                                   
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
-12 0.45000         host-domain host-group-0-rack-03                                   
 -6 0.45000             host node7-3                                                   
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 -1 1.34998 root default                                                               
 -3 0.44998     rack rack-01                                                           
 -2 0.44998         host node7-1                                                       
  2 0.14999             osd.2                             up  1.00000          1.00000 
  0 0.14999             osd.0                           down        0          1.00000 
  1 0.14999             osd.1                             up  1.00000          1.00000 
 -5 0.45000     rack rack-02                                                           
 -4 0.45000         host node7-2                                                       
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 -7 0.45000     rack rack-03                                                           
 -6 0.45000         host node7-3                                                       
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
</pre>
可以看到osd.0的权重也降为0了。

2) **重新导出osdmap及pgmap**

在osd.0停止后大约第8分钟，我们再导出osdmap及pgmap:
{% highlight string %}
# ceph osd dump > osdmap_out_down.txt
# ceph pg dump > pgmap_out_down.txt
dumped all in format plain
{% endhighlight %}

## 4. osdmap对比分析
1） **```active+clean```状态与```in+down```状态osdmap对比**
{% highlight string %}
# diff osdmap_active_clean.txt osdmap_in_down.txt -y --suppress-common-lines
epoch 2222                                                    | epoch 2225
modified 2020-09-11 12:13:01.076048                           | modified 2020-09-11 14:05:20.831242
osd.0 up   in  weight 1 up_from 2220 up_thru 2221 down_at 221 | osd.0 down in  weight 1 up_from 2220 up_thru 2221 down_at 222
osd.3 up   in  weight 1 up_from 2123 up_thru 2221 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2223 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2221 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2223 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2221 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2224 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2221 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2223 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2221 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2223 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2221 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2223 down_at 208
                                                              > pg_temp 11.0 [6,0]
                                                              > pg_temp 11.4 [0,3]
                                                              > pg_temp 11.6 [3,0]
                                                              > pg_temp 13.1 [8,0]
                                                              > pg_temp 13.4 [0,6]
                                                              > pg_temp 13.9 [0,4]
                                                              > pg_temp 13.c [4,0]
                                                              > pg_temp 13.d [0,7]
                                                              > pg_temp 14.3 [0,5]
                                                              > pg_temp 14.5 [0,8]
                                                              > pg_temp 14.8 [0,5]
                                                              > pg_temp 14.d [0,5]
                                                              > pg_temp 15.5 [3,0]
                                                              > pg_temp 16.2 [8,0]
                                                              > pg_temp 17.0 [7,0]
                                                              > pg_temp 17.4 [6,0]
                                                              > pg_temp 18.1 [8,0]
                                                              > pg_temp 18.2 [0,4]
                                                              > pg_temp 18.3 [4,0]
                                                              > pg_temp 18.4 [0,6]
                                                              > pg_temp 19.1 [3,0]
                                                              > pg_temp 19.2 [0,4]
                                                              > pg_temp 20.3 [0,7]
                                                              > pg_temp 20.5 [5,0]
                                                              > pg_temp 21.1 [0,4]
                                                              > pg_temp 21.3 [8,0]
                                                              > pg_temp 21.6 [5,0]
                                                              > pg_temp 22.0 [3,0]
                                                              > pg_temp 22.e [0,3]
                                                              > pg_temp 22.19 [5,0]
                                                              > pg_temp 22.1e [4,0]
                                                              > pg_temp 22.21 [0,7]
                                                              > pg_temp 22.24 [7,0]
                                                              > pg_temp 22.2a [3,0]
                                                              > pg_temp 22.2c [0,3]
                                                              > pg_temp 22.2f [8,0]
                                                              > pg_temp 22.37 [7,0]
                                                              > pg_temp 22.3a [0,5]
                                                              > pg_temp 22.3b [4,0]
                                                              > pg_temp 22.44 [0,3]
                                                              > pg_temp 22.49 [8,0]
                                                              > pg_temp 22.4a [0,4]
                                                              > pg_temp 22.4b [7,0]
                                                              > pg_temp 22.4e [8,0]
                                                              > pg_temp 22.54 [8,0]
                                                              > pg_temp 22.65 [0,8]
                                                              > pg_temp 22.6b [0,8]
                                                              > pg_temp 22.73 [0,8]
                                                              > pg_temp 22.79 [0,8]
                                                              > pg_temp 22.7a [5,0]
                                                              > pg_temp 22.7f [8,0]
                                                              > pg_temp 22.85 [5,0]
                                                              > pg_temp 22.87 [6,0]
                                                              > pg_temp 22.8b [3,0]
                                                              > pg_temp 22.92 [0,7]
                                                              > pg_temp 22.96 [0,4]
                                                              > pg_temp 22.9a [7,0]
                                                              > pg_temp 22.a2 [0,8]
                                                              > pg_temp 22.a4 [0,3]
                                                              > pg_temp 22.a7 [0,6]
                                                              > pg_temp 22.b0 [6,0]
                                                              > pg_temp 22.b5 [3,0]
                                                              > pg_temp 22.b8 [6,0]
                                                              > pg_temp 22.b9 [5,0]
                                                              > pg_temp 22.bf [6,0]
                                                              > pg_temp 22.c0 [5,0]
                                                              > pg_temp 22.c4 [0,5]
                                                              > pg_temp 22.c8 [6,0]
                                                              > pg_temp 22.ca [0,3]
                                                              > pg_temp 22.cb [0,7]
                                                              > pg_temp 22.ce [0,4]
                                                              > pg_temp 22.d0 [3,0]
                                                              > pg_temp 22.d5 [4,0]
                                                              > pg_temp 22.d7 [0,8]
                                                              > pg_temp 22.df [7,0]
                                                              > pg_temp 22.e2 [0,8]
                                                              > pg_temp 22.ec [0,3]
                                                              > pg_temp 22.f1 [0,8]
                                                              > pg_temp 22.f5 [3,0]
                                                              > pg_temp 22.fa [5,0]
                                                              > pg_temp 23.2 [4,0]
                                                              > pg_temp 23.b [0,3]
                                                              > pg_temp 23.d [0,3]
                                                              > pg_temp 23.f [3,0]
                                                              > pg_temp 23.13 [3,0]
                                                              > pg_temp 23.1e [0,5]
                                                              > pg_temp 23.26 [0,6]
                                                              > pg_temp 23.30 [0,3]
                                                              > pg_temp 23.37 [0,8]
                                                              > pg_temp 23.38 [0,4]
                                                              > pg_temp 23.4b [5,0]
                                                              > pg_temp 23.4f [8,0]
                                                              > pg_temp 23.61 [0,4]
                                                              > pg_temp 23.6d [3,0]
                                                              > pg_temp 23.72 [0,7]
                                                              > pg_temp 23.73 [5,0]
                                                              > pg_temp 23.75 [0,7]
                                                              > pg_temp 23.76 [0,4]
                                                              > pg_temp 23.79 [0,8]
                                                              > pg_temp 23.7a [0,4]
                                                              > pg_temp 23.7b [0,7]
                                                              > pg_temp 23.7e [0,6]
                                                              > pg_temp 23.7f [6,0]
                                                              > pg_temp 23.83 [4,0]
                                                              > pg_temp 23.87 [7,0]
                                                              > pg_temp 23.89 [0,7]
                                                              > pg_temp 23.95 [4,0]
                                                              > pg_temp 23.96 [0,5]
                                                              > pg_temp 23.97 [6,0]
                                                              > pg_temp 23.9d [8,0]
                                                              > pg_temp 23.a1 [0,6]
                                                              > pg_temp 23.a4 [7,0]
                                                              > pg_temp 23.a7 [0,4]
                                                              > pg_temp 23.ad [4,0]
                                                              > pg_temp 23.ae [0,4]
                                                              > pg_temp 23.bb [0,3]
                                                              > pg_temp 23.bc [6,0]
                                                              > pg_temp 23.be [0,5]
                                                              > pg_temp 23.c1 [5,0]
                                                              > pg_temp 23.cb [0,7]
                                                              > pg_temp 23.cf [0,6]
                                                              > pg_temp 23.d2 [0,7]
                                                              > pg_temp 23.d5 [6,0]
                                                              > pg_temp 23.d6 [0,8]
                                                              > pg_temp 23.d7 [0,4]
                                                              > pg_temp 23.dc [6,0]
                                                              > pg_temp 23.de [4,0]
                                                              > pg_temp 23.df [5,0]
                                                              > pg_temp 23.e0 [0,4]
                                                              > pg_temp 23.e4 [0,6]
                                                              > pg_temp 23.e9 [3,0]
                                                              > pg_temp 23.ea [0,8]
                                                              > pg_temp 23.ef [4,0]
                                                              > pg_temp 23.f3 [8,0]
                                                              > pg_temp 23.fa [0,6]
                                                              > pg_temp 23.fc [3,0]
                                                              > pg_temp 24.4 [3,0]
                                                              > pg_temp 24.5 [0,8]
                                                              > pg_temp 25.5 [3,0]

{% endhighlight %}
从上面我们可以看到首先是osdmap的epoch值发生了变动，由```2222```变成了```2225```。然后再是osdmap显示```osd.0```上的139个PG产生了pg_temp。

2） **```in+down```状态与```out+down```状态osdmap对比**
{% highlight string %}
# diff osdmap_in_down.txt osdmap_out_down.txt -y --suppress-common-lines
epoch 2225                                                    | epoch 2231
modified 2020-09-11 14:05:20.831242                           | modified 2020-09-11 14:10:24.139331
osd.0 down in  weight 1 up_from 2220 up_thru 2221 down_at 222 | osd.0 down out weight 0 up_from 2220 up_thru 2221 down_at 222
osd.1 up   in  weight 1 up_from 2119 up_thru 2220 down_at 211 | osd.1 up   in  weight 1 up_from 2119 up_thru 2227 down_at 211
osd.2 up   in  weight 1 up_from 2118 up_thru 2220 down_at 209 | osd.2 up   in  weight 1 up_from 2118 up_thru 2230 down_at 209
osd.3 up   in  weight 1 up_from 2123 up_thru 2223 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2228 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2223 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2227 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2224 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2227 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2223 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2227 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2223 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2229 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2223 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2229 down_at 208
pg_temp 11.0 [6,0]                                            <
pg_temp 11.4 [0,3]                                            <
pg_temp 11.6 [3,0]                                            <
pg_temp 13.1 [8,0]                                            <
pg_temp 13.4 [0,6]                                            <
pg_temp 13.9 [0,4]                                            <
pg_temp 13.c [4,0]                                            <
pg_temp 13.d [0,7]                                            <
pg_temp 14.3 [0,5]                                            <
pg_temp 14.5 [0,8]                                            <
pg_temp 14.8 [0,5]                                            <
pg_temp 15.5 [3,0]                                            <
pg_temp 16.2 [8,0]                                            <
pg_temp 17.0 [7,0]                                            <
pg_temp 17.4 [6,0]                                            <
pg_temp 18.1 [8,0]                                            <
pg_temp 18.2 [0,4]                                            <
pg_temp 18.3 [4,0]                                            <
pg_temp 18.4 [0,6]                                            <
pg_temp 19.1 [3,0]                                            <
pg_temp 19.2 [0,4]                                            <
pg_temp 20.3 [0,7]                                            <
pg_temp 20.5 [5,0]                                            <
pg_temp 21.1 [0,4]                                            <
pg_temp 21.3 [8,0]                                            <
pg_temp 21.6 [5,0]                                            <
pg_temp 22.0 [3,0]                                            <
pg_temp 22.e [0,3]                                            <
pg_temp 22.19 [5,0]                                           <
pg_temp 22.1e [4,0]                                           <
pg_temp 22.21 [0,7]                                           <
pg_temp 22.24 [7,0]                                           <
pg_temp 22.2a [3,0]                                           <
pg_temp 22.2c [0,3]                                           <
pg_temp 22.2f [8,0]                                           <
pg_temp 22.37 [7,0]                                           <
pg_temp 22.3a [0,5]                                           <
pg_temp 22.3b [4,0]                                           <
pg_temp 22.44 [0,3]                                           <
pg_temp 22.49 [8,0]                                           <
pg_temp 22.4a [0,4]                                           <
pg_temp 22.4b [7,0]                                           <
pg_temp 22.4e [8,0]                                           <
pg_temp 22.54 [8,0]                                           <
pg_temp 22.65 [0,8]                                           <
pg_temp 22.6b [0,8]                                           <
pg_temp 22.73 [0,8]                                           <
pg_temp 22.79 [0,8]                                           <
pg_temp 22.7a [5,0]                                           <
pg_temp 22.7f [8,0]                                           <
pg_temp 22.85 [5,0]                                           <
pg_temp 22.87 [6,0]                                           <
pg_temp 22.8b [3,0]                                           <
pg_temp 22.92 [0,7]                                           <
pg_temp 22.96 [0,4]                                           <
pg_temp 22.9a [7,0]                                           <
pg_temp 22.a2 [0,8]                                           <
pg_temp 22.a4 [0,3]                                           <
pg_temp 22.a7 [0,6]                                           <
pg_temp 22.b0 [6,0]                                           <
pg_temp 22.b5 [3,0]                                           <
pg_temp 22.b8 [6,0]                                           <
pg_temp 22.b9 [5,0]                                           <
pg_temp 22.bf [6,0]                                           <
pg_temp 22.c0 [5,0]                                           <
pg_temp 22.c4 [0,5]                                           <
pg_temp 22.c8 [6,0]                                           <
pg_temp 22.ca [0,3]                                           <
pg_temp 22.cb [0,7]                                           <
pg_temp 22.ce [0,4]                                           <
pg_temp 22.d0 [3,0]                                           <
pg_temp 22.d5 [4,0]                                           <
pg_temp 22.d7 [0,8]                                           <
pg_temp 22.df [7,0]                                           <
pg_temp 22.e2 [0,8]                                           <
pg_temp 22.ec [0,3]                                           <
pg_temp 22.f1 [0,8]                                           <
pg_temp 22.f5 [3,0]                                           <
pg_temp 22.fa [5,0]                                           <
pg_temp 23.2 [4,0]                                            <
pg_temp 23.b [0,3]                                            <
pg_temp 23.d [0,3]                                            <
pg_temp 23.f [3,0]                                            <
pg_temp 23.13 [3,0]                                           <
pg_temp 23.1e [0,5]                                           <
pg_temp 23.26 [0,6]                                           <
pg_temp 23.30 [0,3]                                           <
pg_temp 23.37 [0,8]                                           <
pg_temp 23.38 [0,4]                                           <
pg_temp 23.4b [5,0]                                           <
pg_temp 23.4f [8,0]                                           <
pg_temp 23.61 [0,4]                                           <
pg_temp 23.6d [3,0]                                           <
pg_temp 23.72 [0,7]                                           <
pg_temp 23.73 [5,0]                                           <
pg_temp 23.75 [0,7]                                           <
pg_temp 23.76 [0,4]                                           <
pg_temp 23.79 [0,8]                                           <
pg_temp 23.7a [0,4]                                           <
pg_temp 23.7b [0,7]                                           <
pg_temp 23.7e [0,6]                                           <
pg_temp 23.7f [6,0]                                           <
pg_temp 23.83 [4,0]                                           <
pg_temp 23.87 [7,0]                                           <
pg_temp 23.89 [0,7]                                           <
pg_temp 23.95 [4,0]                                           <
pg_temp 23.96 [0,5]                                           <
pg_temp 23.97 [6,0]                                           <
pg_temp 23.9d [8,0]                                           <
pg_temp 23.a1 [0,6]                                           <
pg_temp 23.a4 [7,0]                                           <
pg_temp 23.a7 [0,4]                                           <
pg_temp 23.ad [4,0]                                           <
pg_temp 23.ae [0,4]                                           <
pg_temp 23.bb [0,3]                                           <
pg_temp 23.bc [6,0]                                           <
pg_temp 23.be [0,5]                                           <
pg_temp 23.c1 [5,0]                                           <
pg_temp 23.cb [0,7]                                           <
pg_temp 23.cf [0,6]                                           <
pg_temp 23.d2 [0,7]                                           <
pg_temp 23.d5 [6,0]                                           <
pg_temp 23.d6 [0,8]                                           <
pg_temp 23.d7 [0,4]                                           <
pg_temp 23.dc [6,0]                                           <
pg_temp 23.de [4,0]                                           <
pg_temp 23.df [5,0]                                           <
pg_temp 23.e0 [0,4]                                           <
pg_temp 23.e4 [0,6]                                           <
pg_temp 23.e9 [3,0]                                           <
pg_temp 23.ea [0,8]                                           <
pg_temp 23.ef [4,0]                                           <
pg_temp 23.f3 [8,0]                                           <
pg_temp 23.fa [0,6]                                           <
pg_temp 23.fc [3,0]                                           <
pg_temp 24.4 [3,0]                                            <
pg_temp 24.5 [0,8]                                            <
pg_temp 25.5 [3,0]                                            <
{% endhighlight %}
从上面我们可以看到首先是osdmap的epoch值发生了变动，由```2225```变成了```2231```，并且此时pg_temp已经消失。

## 5. pgmap对比分析
这里我们主要是对pgmap立面的```pg_stat```、```up```、```up_primary```、```acting```、```acting_primary```这几列比较感兴趣，下面我们分别对前面保存的pgmap_active_clean.txt、pgmap_in_down.txt、pgmap_out_down.txt做如下处理：
{% highlight string %}
# cat pgmap_active_clean.txt | awk '{print $1 "\t" $15 "\t" $16 "\t" $17 "\t" $18}' > pgmap_active_clean_fix.txt
# cat pgmap_in_down.txt | awk '{print $1 "\t" $15 "\t" $16 "\t" $17 "\t" $18}' > pgmap_in_down_fix.txt
# cat pgmap_out_down.txt | awk '{print $1 "\t" $15 "\t" $16 "\t" $17 "\t" $18}' > pgmap_out_down_fix.txt
{% endhighlight %}

1) **```active+clean```状态与```in+down```状态pgmap对比**
{% highlight string %}
# diff pgmap_active_clean_fix.txt pgmap_in_down_fix.txt -y --suppress-common-lines
pg_stat  up   up_primary acting acting_primary                  pg_stat  up   up_primary acting acting_primary
22.65   [0,8]   0       [0,8]   0                             | 22.65   [8]     8       [8]     8
23.61   [0,4]   0       [0,4]   0                             | 23.61   [4]     4       [4]     4
22.54   [8,0]   8       [8,0]   8                             | 22.54   [8]     8       [8]     8
23.4f   [8,0]   8       [8,0]   8                             | 23.4f   [8]     8       [8]     8
22.4e   [8,0]   8       [8,0]   8                             | 22.4e   [8]     8       [8]     8
22.4b   [7,0]   7       [7,0]   7                             | 22.4b   [7]     7       [7]     7
23.4b   [5,0]   5       [5,0]   5                             | 23.4b   [5]     5       [5]     5
22.4a   [0,4]   0       [0,4]   0                             | 22.4a   [4]     4       [4]     4
22.49   [8,0]   8       [8,0]   8                             | 22.49   [8]     8       [8]     8
22.44   [0,3]   0       [0,3]   0                             | 22.44   [3]     3       [3]     3
22.3b   [4,0]   4       [4,0]   4                             | 22.3b   [4]     4       [4]     4
22.3a   [0,5]   0       [0,5]   0                             | 22.3a   [5]     5       [5]     5
23.38   [0,4]   0       [0,4]   0                             | 23.38   [4]     4       [4]     4
22.37   [7,0]   7       [7,0]   7                             | 22.37   [7]     7       [7]     7
23.37   [0,8]   0       [0,8]   0                             | 23.37   [8]     8       [8]     8
23.30   [0,3]   0       [0,3]   0                             | 23.30   [3]     3       [3]     3
22.2f   [8,0]   8       [8,0]   8                             | 22.2f   [8]     8       [8]     8
22.2c   [0,3]   0       [0,3]   0                             | 22.2c   [3]     3       [3]     3
22.2a   [3,0]   3       [3,0]   3                             | 22.2a   [3]     3       [3]     3
23.26   [0,6]   0       [0,6]   0                             | 23.26   [6]     6       [6]     6
22.24   [7,0]   7       [7,0]   7                             | 22.24   [7]     7       [7]     7
22.21   [0,7]   0       [0,7]   0                             | 22.21   [7]     7       [7]     7
14.d    [0,5]   0       [0,5]   0                             | 14.d    [5]     5       [5]     5
13.d    [0,7]   0       [0,7]   0                             | 13.d    [7]     7       [7]     7
13.c    [4,0]   4       [4,0]   4                             | 13.c    [4]     4       [4]     4
14.8    [0,5]   0       [0,5]   0                             | 14.8    [5]     5       [5]     5
13.9    [0,4]   0       [0,4]   0                             | 13.9    [4]     4       [4]     4
23.13   [3,0]   3       [3,0]   3                             | 23.13   [3]     3       [3]     3
13.1    [8,0]   8       [8,0]   8                             | 13.1    [8]     8       [8]     8
14.3    [0,5]   0       [0,5]   0                             | 14.3    [5]     5       [5]     5
11.6    [3,0]   3       [3,0]   3                             | 11.6    [3]     3       [3]     3
22.19   [5,0]   5       [5,0]   5                             | 22.19   [5]     5       [5]     5
11.4    [0,3]   0       [0,3]   0                             | 11.4    [3]     3       [3]     3
22.1e   [4,0]   4       [4,0]   4                             | 22.1e   [4]     4       [4]     4
13.4    [0,6]   0       [0,6]   0                             | 13.4    [6]     6       [6]     6
23.1e   [0,5]   0       [0,5]   0                             | 23.1e   [5]     5       [5]     5
15.5    [3,0]   3       [3,0]   3                             | 15.5    [3]     3       [3]     3
14.5    [0,8]   0       [0,8]   0                             | 14.5    [8]     8       [8]     8
11.0    [6,0]   6       [6,0]   6                             | 11.0    [6]     6       [6]     6
18.2    [0,4]   0       [0,4]   0                             | 18.2    [4]     4       [4]     4
17.0    [7,0]   7       [7,0]   7                             | 17.0    [7]     7       [7]     7
18.3    [4,0]   4       [4,0]   4                             | 18.3    [4]     4       [4]     4
19.2    [0,4]   0       [0,4]   0                             | 19.2    [4]     4       [4]     4
20.5    [5,0]   5       [5,0]   5                             | 20.5    [5]     5       [5]     5
16.2    [8,0]   8       [8,0]   8                             | 16.2    [8]     8       [8]     8
19.1    [3,0]   3       [3,0]   3                             | 19.1    [3]     3       [3]     3
21.6    [5,0]   5       [5,0]   5                             | 21.6    [5]     5       [5]     5
18.1    [8,0]   8       [8,0]   8                             | 18.1    [8]     8       [8]     8
21.1    [0,4]   0       [0,4]   0                             | 21.1    [4]     4       [4]     4
17.4    [6,0]   6       [6,0]   6                             | 17.4    [6]     6       [6]     6
23.2    [4,0]   4       [4,0]   4                             | 23.2    [4]     4       [4]     4
22.0    [3,0]   3       [3,0]   3                             | 22.0    [3]     3       [3]     3
21.3    [8,0]   8       [8,0]   8                             | 21.3    [8]     8       [8]     8
18.4    [0,6]   0       [0,6]   0                             | 18.4    [6]     6       [6]     6
20.3    [0,7]   0       [0,7]   0                             | 20.3    [7]     7       [7]     7
22.6b   [0,8]   0       [0,8]   0                             | 22.6b   [8]     8       [8]     8
23.6d   [3,0]   3       [3,0]   3                             | 23.6d   [3]     3       [3]     3
23.73   [5,0]   5       [5,0]   5                             | 23.73   [5]     5       [5]     5
23.72   [0,7]   0       [0,7]   0                             | 23.72   [7]     7       [7]     7
22.73   [0,8]   0       [0,8]   0                             | 22.73   [8]     8       [8]     8
23.75   [0,7]   0       [0,7]   0                             | 23.75   [7]     7       [7]     7
23.76   [0,4]   0       [0,4]   0                             | 23.76   [4]     4       [4]     4
23.79   [0,8]   0       [0,8]   0                             | 23.79   [8]     8       [8]     8
22.79   [0,8]   0       [0,8]   0                             | 22.79   [8]     8       [8]     8
23.7b   [0,7]   0       [0,7]   0                             | 23.7b   [7]     7       [7]     7
22.7a   [5,0]   5       [5,0]   5                             | 22.7a   [5]     5       [5]     5
23.7a   [0,4]   0       [0,4]   0                             | 23.7a   [4]     4       [4]     4
23.7f   [6,0]   6       [6,0]   6                             | 23.7f   [6]     6       [6]     6
23.7e   [0,6]   0       [0,6]   0                             | 23.7e   [6]     6       [6]     6
22.7f   [8,0]   8       [8,0]   8                             | 22.7f   [8]     8       [8]     8
23.83   [4,0]   4       [4,0]   4                             | 23.83   [4]     4       [4]     4
22.85   [5,0]   5       [5,0]   5                             | 22.85   [5]     5       [5]     5
23.87   [7,0]   7       [7,0]   7                             | 23.87   [7]     7       [7]     7
22.87   [6,0]   6       [6,0]   6                             | 22.87   [6]     6       [6]     6
23.89   [0,7]   0       [0,7]   0                             | 23.89   [7]     7       [7]     7
22.8b   [3,0]   3       [3,0]   3                             | 22.8b   [3]     3       [3]     3
22.92   [0,7]   0       [0,7]   0                             | 22.92   [7]     7       [7]     7
23.95   [4,0]   4       [4,0]   4                             | 23.95   [4]     4       [4]     4
23.97   [6,0]   6       [6,0]   6                             | 23.97   [6]     6       [6]     6
22.96   [0,4]   0       [0,4]   0                             | 22.96   [4]     4       [4]     4
23.96   [0,5]   0       [0,5]   0                             | 23.96   [5]     5       [5]     5
22.9a   [7,0]   7       [7,0]   7                             | 22.9a   [7]     7       [7]     7
23.9d   [8,0]   8       [8,0]   8                             | 23.9d   [8]     8       [8]     8
25.5    [3,0]   3       [3,0]   3                             | 25.5    [3]     3       [3]     3
24.4    [3,0]   3       [3,0]   3                             | 24.4    [3]     3       [3]     3
23.b    [0,3]   0       [0,3]   0                             | 23.b    [3]     3       [3]     3
23.a1   [0,6]   0       [0,6]   0                             | 23.a1   [6]     6       [6]     6
22.a2   [0,8]   0       [0,8]   0                             | 22.a2   [8]     8       [8]     8
22.a4   [0,3]   0       [0,3]   0                             | 22.a4   [3]     3       [3]     3
23.a4   [7,0]   7       [7,0]   7                             | 23.a4   [7]     7       [7]     7
23.a7   [0,4]   0       [0,4]   0                             | 23.a7   [4]     4       [4]     4
22.a7   [0,6]   0       [0,6]   0                             | 22.a7   [6]     6       [6]     6
23.ad   [4,0]   4       [4,0]   4                             | 23.ad   [4]     4       [4]     4
23.ae   [0,4]   0       [0,4]   0                             | 23.ae   [4]     4       [4]     4
24.5    [0,8]   0       [0,8]   0                             | 24.5    [8]     8       [8]     8
22.b0   [6,0]   6       [6,0]   6                             | 22.b0   [6]     6       [6]     6
22.b5   [3,0]   3       [3,0]   3                             | 22.b5   [3]     3       [3]     3
22.b8   [6,0]   6       [6,0]   6                             | 22.b8   [6]     6       [6]     6
22.b9   [5,0]   5       [5,0]   5                             | 22.b9   [5]     5       [5]     5
23.bb   [0,3]   0       [0,3]   0                             | 23.bb   [3]     3       [3]     3
23.bc   [6,0]   6       [6,0]   6                             | 23.bc   [6]     6       [6]     6
23.be   [0,5]   0       [0,5]   0                             | 23.be   [5]     5       [5]     5
22.bf   [6,0]   6       [6,0]   6                             | 22.bf   [6]     6       [6]     6
23.d    [0,3]   0       [0,3]   0                             | 23.d    [3]     3       [3]     3
23.c1   [5,0]   5       [5,0]   5                             | 23.c1   [5]     5       [5]     5
22.c0   [5,0]   5       [5,0]   5                             | 22.c0   [5]     5       [5]     5
22.c4   [0,5]   0       [0,5]   0                             | 22.c4   [5]     5       [5]     5
22.c8   [6,0]   6       [6,0]   6                             | 22.c8   [6]     6       [6]     6
23.cb   [0,7]   0       [0,7]   0                             | 23.cb   [7]     7       [7]     7
22.ca   [0,3]   0       [0,3]   0                             | 22.ca   [3]     3       [3]     3
22.cb   [0,7]   0       [0,7]   0                             | 22.cb   [7]     7       [7]     7
23.cf   [0,6]   0       [0,6]   0                             | 23.cf   [6]     6       [6]     6
22.ce   [0,4]   0       [0,4]   0                             | 22.ce   [4]     4       [4]     4
22.d0   [3,0]   3       [3,0]   3                             | 22.d0   [3]     3       [3]     3
23.d2   [0,7]   0       [0,7]   0                             | 23.d2   [7]     7       [7]     7
23.d5   [6,0]   6       [6,0]   6                             | 23.d5   [6]     6       [6]     6
22.d5   [4,0]   4       [4,0]   4                             | 22.d5   [4]     4       [4]     4
23.d7   [0,4]   0       [0,4]   0                             | 23.d7   [4]     4       [4]     4
23.d6   [0,8]   0       [0,8]   0                             | 23.d6   [8]     8       [8]     8
22.d7   [0,8]   0       [0,8]   0                             | 22.d7   [8]     8       [8]     8
23.dc   [6,0]   6       [6,0]   6                             | 23.dc   [6]     6       [6]     6
23.df   [5,0]   5       [5,0]   5                             | 23.df   [5]     5       [5]     5
23.de   [4,0]   4       [4,0]   4                             | 23.de   [4]     4       [4]     4
22.df   [7,0]   7       [7,0]   7                             | 22.df   [7]     7       [7]     7
23.f    [3,0]   3       [3,0]   3                             | 23.f    [3]     3       [3]     3
22.e    [0,3]   0       [0,3]   0                             | 22.e    [3]     3       [3]     3
23.e0   [0,4]   0       [0,4]   0                             | 23.e0   [4]     4       [4]     4
22.e2   [0,8]   0       [0,8]   0                             | 22.e2   [8]     8       [8]     8
23.e4   [0,6]   0       [0,6]   0                             | 23.e4   [6]     6       [6]     6
23.e9   [3,0]   3       [3,0]   3                             | 23.e9   [3]     3       [3]     3
23.ea   [0,8]   0       [0,8]   0                             | 23.ea   [8]     8       [8]     8
22.ec   [0,3]   0       [0,3]   0                             | 22.ec   [3]     3       [3]     3
23.ef   [4,0]   4       [4,0]   4                             | 23.ef   [4]     4       [4]     4
22.f1   [0,8]   0       [0,8]   0                             | 22.f1   [8]     8       [8]     8
23.f3   [8,0]   8       [8,0]   8                             | 23.f3   [8]     8       [8]     8
22.f5   [3,0]   3       [3,0]   3                             | 22.f5   [3]     3       [3]     3
22.fa   [5,0]   5       [5,0]   5                             | 22.fa   [5]     5       [5]     5
23.fa   [0,6]   0       [0,6]   0                             | 23.fa   [6]     6       [6]     6
23.fc   [3,0]   3       [3,0]   3                             | 23.fc   [3]     3       [3]     3
{% endhighlight %}
上面我们看到受影响的行数刚好就是存在于osd.0上pg的个数```139```。

2) **```in+down```状态与```out+down```状态pgmap对比**
{% highlight string %}
# diff pgmap_in_down_fix.txt pgmap_out_down_fix.txt -y --suppress-common-lines
pg_stat  up   up_primary acting acting_primary                  pg_stat  up   up_primary acting acting_primary
22.65   [8]     8       [8]     8                             | 22.65   [8,2]   8       [8,2]   8
23.61   [4]     4       [4]     4                             | 23.61   [3,2]   3       [3,2]   3
22.54   [8]     8       [8]     8                             | 22.54   [8,5]   8       [8,5]   8
23.4f   [8]     8       [8]     8                             | 23.4f   [8,2]   8       [8,2]   8
22.4e   [8]     8       [8]     8                             | 22.4e   [8,5]   8       [8,5]   8
22.4b   [7]     7       [7]     7                             | 22.4b   [7,4]   7       [7,4]   7
23.4b   [5]     5       [5]     5                             | 23.4b   [5,7]   5       [5,7]   5
22.4a   [4]     4       [4]     4                             | 22.4a   [4,6]   4       [4,6]   4
22.49   [8]     8       [8]     8                             | 22.49   [8,5]   8       [8,5]   8
22.44   [3]     3       [3]     3                             | 22.44   [2,3]   2       [2,3]   2
22.3b   [4]     4       [4]     4                             | 22.3b   [4,1]   4       [4,1]   4
22.3a   [5]     5       [5]     5                             | 22.3a   [3,7]   3       [3,7]   3
23.38   [4]     4       [4]     4                             | 23.38   [3,1]   3       [3,1]   3
22.37   [7]     7       [7]     7                             | 22.37   [7,5]   7       [7,5]   7
23.37   [8]     8       [8]     8                             | 23.37   [7,5]   7       [7,5]   7
23.30   [3]     3       [3]     3                             | 23.30   [3,7]   3       [3,7]   3
22.2f   [8]     8       [8]     8                             | 22.2f   [8,4]   8       [8,4]   8
22.2c   [3]     3       [3]     3                             | 22.2c   [5,7]   5       [5,7]   5
22.2a   [3]     3       [3]     3                             | 22.2a   [3,6]   3       [3,6]   3
23.26   [6]     6       [6]     6                             | 23.26   [1,6]   1       [1,6]   1
22.24   [7]     7       [7]     7                             | 22.24   [7,5]   7       [7,5]   7
22.21   [7]     7       [7]     7                             | 22.21   [8,1]   8       [8,1]   8
14.d    [5]     5       [5]     5                             | 14.d    [2,5]   2       [5]     5
13.d    [7]     7       [7]     7                             | 13.d    [1,7]   1       [1,7]   1
13.c    [4]     4       [4]     4                             | 13.c    [4,7]   4       [4,7]   4
14.8    [5]     5       [5]     5                             | 14.8    [3,7]   3       [3,7]   3
13.9    [4]     4       [4]     4                             | 13.9    [5,2]   5       [5,2]   5
23.13   [3]     3       [3]     3                             | 23.13   [3,1]   3       [3,1]   3
13.1    [8]     8       [8]     8                             | 13.1    [8,2]   8       [8,2]   8
14.3    [5]     5       [5]     5                             | 14.3    [2,5]   2       [2,5]   2
11.6    [3]     3       [3]     3                             | 11.6    [3,2]   3       [3,2]   3
22.19   [5]     5       [5]     5                             | 22.19   [5,6]   5       [5,6]   5
11.4    [3]     3       [3]     3                             | 11.4    [3,2]   3       [3,2]   3
22.1e   [4]     4       [4]     4                             | 22.1e   [4,6]   4       [4,6]   4
13.4    [6]     6       [6]     6                             | 13.4    [8,5]   8       [8,5]   8
23.1e   [5]     5       [5]     5                             | 23.1e   [5,1]   5       [5,1]   5
15.5    [3]     3       [3]     3                             | 15.5    [3,1]   3       [3,1]   3
14.5    [8]     8       [8]     8                             | 14.5    [2,8]   2       [2,8]   2
11.0    [6]     6       [6]     6                             | 11.0    [6,1]   6       [6,1]   6
18.2    [4]     4       [4]     4                             | 18.2    [2,4]   2       [2,4]   2
17.0    [7]     7       [7]     7                             | 17.0    [7,2]   7       [7,2]   7
18.3    [4]     4       [4]     4                             | 18.3    [4,2]   4       [4,2]   4
19.2    [4]     4       [4]     4                             | 19.2    [3,2]   3       [3,2]   3
20.5    [5]     5       [5]     5                             | 20.5    [5,6]   5       [5,6]   5
16.2    [8]     8       [8]     8                             | 16.2    [8,5]   8       [8,5]   8
19.1    [3]     3       [3]     3                             | 19.1    [3,7]   3       [3,7]   3
21.6    [5]     5       [5]     5                             | 21.6    [5,8]   5       [5,8]   5
18.1    [8]     8       [8]     8                             | 18.1    [8,3]   8       [8,3]   8
21.1    [4]     4       [4]     4                             | 21.1    [4,1]   4       [4,1]   4
17.4    [6]     6       [6]     6                             | 17.4    [6,3]   6       [6,3]   6
23.2    [4]     4       [4]     4                             | 23.2    [4,1]   4       [4,1]   4
22.0    [3]     3       [3]     3                             | 22.0    [3,1]   3       [3,1]   3
21.3    [8]     8       [8]     8                             | 21.3    [8,1]   8       [8,1]   8
18.4    [6]     6       [6]     6                             | 18.4    [6,4]   6       [6,4]   6
20.3    [7]     7       [7]     7                             | 20.3    [8,4]   8       [8,4]   8
22.6b   [8]     8       [8]     8                             | 22.6b   [7,3]   7       [7,3]   7
23.6d   [3]     3       [3]     3                             | 23.6d   [3,6]   3       [3,6]   3
23.73   [5]     5       [5]     5                             | 23.73   [5,2]   5       [5,2]   5
23.72   [7]     7       [7]     7                             | 23.72   [8,1]   8       [8,1]   8
22.73   [8]     8       [8]     8                             | 22.73   [8,3]   8       [8,3]   8
23.75   [7]     7       [7]     7                             | 23.75   [8,5]   8       [8,5]   8
23.76   [4]     4       [4]     4                             | 23.76   [4,1]   4       [4,1]   4
23.79   [8]     8       [8]     8                             | 23.79   [7,1]   7       [7,1]   7
22.79   [8]     8       [8]     8                             | 22.79   [6,1]   6       [6,1]   6
23.7b   [7]     7       [7]     7                             | 23.7b   [7,4]   7       [7,4]   7
22.7a   [5]     5       [5]     5                             | 22.7a   [5,1]   5       [5,1]   5
23.7a   [4]     4       [4]     4                             | 23.7a   [5,2]   5       [5,2]   5
23.7f   [6]     6       [6]     6                             | 23.7f   [6,5]   6       [6,5]   6
23.7e   [6]     6       [6]     6                             | 23.7e   [7,2]   7       [7,2]   7
22.7f   [8]     8       [8]     8                             | 22.7f   [8,2]   8       [8,2]   8
23.83   [4]     4       [4]     4                             | 23.83   [4,7]   4       [4,7]   4
22.85   [5]     5       [5]     5                             | 22.85   [5,6]   5       [5,6]   5
23.87   [7]     7       [7]     7                             | 23.87   [7,4]   7       [7,4]   7
22.87   [6]     6       [6]     6                             | 22.87   [6,1]   6       [6,1]   6
23.89   [7]     7       [7]     7                             | 23.89   [2,7]   2       [2,7]   2
22.8b   [3]     3       [3]     3                             | 22.8b   [3,2]   3       [3,2]   3
22.92   [7]     7       [7]     7                             | 22.92   [1,7]   1       [1,7]   1
23.95   [4]     4       [4]     4                             | 23.95   [4,7]   4       [4,7]   4
23.97   [6]     6       [6]     6                             | 23.97   [6,1]   6       [6,1]   6
22.96   [4]     4       [4]     4                             | 22.96   [5,1]   5       [5,1]   5
23.96   [5]     5       [5]     5                             | 23.96   [4,8]   4       [4,8]   4
22.9a   [7]     7       [7]     7                             | 22.9a   [7,4]   7       [7,4]   7
23.9d   [8]     8       [8]     8                             | 23.9d   [8,1]   8       [8,1]   8
25.5    [3]     3       [3]     3                             | 25.5    [3,7]   3       [3,7]   3
24.4    [3]     3       [3]     3                             | 24.4    [3,2]   3       [3,2]   3
23.b    [3]     3       [3]     3                             | 23.b    [3,2]   3       [3,2]   3
23.a1   [6]     6       [6]     6                             | 23.a1   [8,3]   8       [8,3]   8
22.a2   [8]     8       [8]     8                             | 22.a2   [7,5]   7       [7,5]   7
22.a4   [3]     3       [3]     3                             | 22.a4   [1,3]   1       [1,3]   1
23.a4   [7]     7       [7]     7                             | 23.a4   [7,5]   7       [7,5]   7
23.a7   [4]     4       [4]     4                             | 23.a7   [5,2]   5       [5,2]   5
22.a7   [6]     6       [6]     6                             | 22.a7   [8,5]   8       [8,5]   8
23.ad   [4]     4       [4]     4                             | 23.ad   [4,6]   4       [4,6]   4
23.ae   [4]     4       [4]     4                             | 23.ae   [5,7]   5       [5,7]   5
24.5    [8]     8       [8]     8                             | 24.5    [6,4]   6       [6,4]   6
22.b0   [6]     6       [6]     6                             | 22.b0   [6,5]   6       [6,5]   6
22.b5   [3]     3       [3]     3                             | 22.b5   [3,2]   3       [3,2]   3
22.b8   [6]     6       [6]     6                             | 22.b8   [6,2]   6       [6,2]   6
22.b9   [5]     5       [5]     5                             | 22.b9   [5,1]   5       [5,1]   5
23.bb   [3]     3       [3]     3                             | 23.bb   [4,6]   4       [4,6]   4
23.bc   [6]     6       [6]     6                             | 23.bc   [6,4]   6       [6,4]   6
23.be   [5]     5       [5]     5                             | 23.be   [2,5]   2       [2,5]   2
22.bf   [6]     6       [6]     6                             | 22.bf   [6,4]   6       [6,4]   6
23.d    [3]     3       [3]     3                             | 23.d    [4,2]   4       [4,2]   4
23.c1   [5]     5       [5]     5                             | 23.c1   [5,2]   5       [5,2]   5
22.c0   [5]     5       [5]     5                             | 22.c0   [5,7]   5       [5,7]   5
22.c4   [5]     5       [5]     5                             | 22.c4   [2,5]   2       [2,5]   2
22.c8   [6]     6       [6]     6                             | 22.c8   [6,2]   6       [6,2]   6
23.cb   [7]     7       [7]     7                             | 23.cb   [6,5]   6       [6,5]   6
22.ca   [3]     3       [3]     3                             | 22.ca   [5,7]   5       [5,7]   5
22.cb   [7]     7       [7]     7                             | 22.cb   [6,3]   6       [6,3]   6
23.cf   [6]     6       [6]     6                             | 23.cf   [7,4]   7       [7,4]   7
22.ce   [4]     4       [4]     4                             | 22.ce   [1,4]   1       [1,4]   1
22.d0   [3]     3       [3]     3                             | 22.d0   [3,7]   3       [3,7]   3
23.d2   [7]     7       [7]     7                             | 23.d2   [7,1]   7       [7,1]   7
23.d5   [6]     6       [6]     6                             | 23.d5   [6,2]   6       [6,2]   6
22.d5   [4]     4       [4]     4                             | 22.d5   [4,6]   4       [4,6]   4
23.d7   [4]     4       [4]     4                             | 23.d7   [4,2]   4       [4,2]   4
23.d6   [8]     8       [8]     8                             | 23.d6   [1,8]   1       [1,8]   1
22.d7   [8]     8       [8]     8                             | 22.d7   [8,4]   8       [8,4]   8
23.dc   [6]     6       [6]     6                             | 23.dc   [6,1]   6       [6,1]   6
23.df   [5]     5       [5]     5                             | 23.df   [5,7]   5       [5,7]   5
23.de   [4]     4       [4]     4                             | 23.de   [4,2]   4       [4,2]   4
22.df   [7]     7       [7]     7                             | 22.df   [7,2]   7       [7,2]   7
23.f    [3]     3       [3]     3                             | 23.f    [3,8]   3       [3,8]   3
22.e    [3]     3       [3]     3                             | 22.e    [3,8]   3       [3,8]   3
23.e0   [4]     4       [4]     4                             | 23.e0   [4,7]   4       [4,7]   4
22.e2   [8]     8       [8]     8                             | 22.e2   [1,8]   1       [1,8]   1
23.e4   [6]     6       [6]     6                             | 23.e4   [8,4]   8       [8,4]   8
23.e9   [3]     3       [3]     3                             | 23.e9   [3,1]   3       [3,1]   3
23.ea   [8]     8       [8]     8                             | 23.ea   [7,4]   7       [7,4]   7
22.ec   [3]     3       [3]     3                             | 22.ec   [3,1]   3       [3,1]   3
23.ef   [4]     4       [4]     4                             | 23.ef   [4,7]   4       [4,7]   4
22.f1   [8]     8       [8]     8                             | 22.f1   [8,3]   8       [8,3]   8
23.f3   [8]     8       [8]     8                             | 23.f3   [8,1]   8       [8,1]   8
22.f5   [3]     3       [3]     3                             | 22.f5   [3,2]   3       [3,2]   3
22.fa   [5]     5       [5]     5                             | 22.fa   [5,8]   5       [5,8]   5
23.fa   [6]     6       [6]     6                             | 23.fa   [8,3]   8       [8,3]   8
23.fc   [3]     3       [3]     3                             | 23.fc   [3,2]   3       [3,2]   3
{% endhighlight %}
可以看到，在```osd.0``` out之后，PG进行了重新的映射。

## 6. 过程分析

下面我们结合上面抓取到的osd3_watch.txt、monitor日志及上面的osdmap、pgmap信息，来分析一下osd0从```in+active```到```in+down```再到```out+down```这一过程当中，ceph集群究竟执行了哪些操作，从而窥探出ceph的整体工作原理。

###### 6.1 in+down状态下osdmap中出现的pg_temp
在上面```in+down```状态下，我们通过对比osdmap，发现出现了139个pg_temp， 对于这一点我们感觉到十分疑惑。这里我们先来看一下网上相关文章对```临时PG```的说明：

>假设一个PG的acting set为[0,1,2]列表。此时如果osd0出现故障，导致CRUSH算法重新分配该PG的acting set为[3,1,2]。此时osd3为该PG的主OSD，但是osd3为新加入的OSD，并不能负担该PG上的读操作。所以PG向Monitor申请一个临时的PG，osd1为临时的主OSD，这时up set变为[1,3,2]，acting set依然为[3,1,2]，导致acting set和up set不同。当osd3完成Backfill过程之后，临时PG被取消，该PG的up set修复为acting set，此时acting set和up set都为[3,1,2]列表。

通过上面的描述，我们可以认为```acting set```是实际进行服务的一个映射，但是假如说该映射的primary osd当前并不能正常提供服务的话，那么此时就需要申请一个```临时PG```。查询Ceph源代码，发现如下函数会申请pg_temp:
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound);
{% endhighlight %}

choose_acting()函数会在两个地方被调用：

* PG::RecoveryState::GetLog::GetLog(): 在PG进行peering获取权威日志时，GetLog()被调用

* PG::RecoveryState::Recovered::Recovered(): 在数据恢复完成时，Recovered()被调用

因为这里我们的PG的副本数设置的是两副本，因此对于那些映射为[0,3]或[3,0]的PG，在osd0停止后的```in+down```阶段，只有可能是osd3申请了临时PG，所以我们这里跟踪osd3_watch.txt日志信息。通过仔细跟踪osd3_watch.txt日志信息，并未发现osd3使用choose_acting()函数对所有与osd0、osd3相关的PG进行```临时PG```申请。

另一方面，由于osd0被我们使用```systemctl stop ceph-osd@0```命令停止，按理说处于degraded状态下的139个PG中，只有68个PG的主OSD是osd0，因而在acting set不能正常提供服务的情况下，最多也只有68个PG会申请pg temp。


那在```in+down```状态下osdmap中的139个pg_temp是如何而来的呢？如下是我们结合Monitor日志分析结果：

1） **osd0端**

osd0接收到关闭信号，后调用shutdown()函数：
{% highlight string %}
int OSD::shutdown()
{
	if (!service.prepare_to_stop())
		return 0; // already shutting down

	....
}
{% endhighlight %}
在shutdown()函数中调用prepare_to_stop()函数向Monitor发送```MOSDMarkMeDown```消息：
{% highlight string %}
bool OSDService::prepare_to_stop()
{
	Mutex::Locker l(is_stopping_lock);
	if (get_state() != NOT_STOPPING)
		return false;
	
	OSDMapRef osdmap = get_osdmap();
	if (osdmap && osdmap->is_up(whoami)) {
		dout(0) << __func__ << " telling mon we are shutting down" << dendl;
		set_state(PREPARING_TO_STOP);
		monc->send_mon_message(new MOSDMarkMeDown(monc->get_fsid(),
			osdmap->get_inst(whoami),
			osdmap->get_epoch(),
			true  // request ack
			));
		utime_t now = ceph_clock_now(cct);
		utime_t timeout;
		timeout.set_from_double(now + cct->_conf->osd_mon_shutdown_timeout);
		while ((ceph_clock_now(cct) < timeout) &&
		  (get_state() != STOPPING)) {
			is_stopping_cond.WaitUntil(is_stopping_lock, timeout);
		}
	}
	dout(0) << __func__ << " starting shutdown" << dendl;
	set_state(STOPPING);
	return true;
}
{% endhighlight %}

2) **Monitor端**

在Monitor端(src/mon/Monitor.cc)，Monitor类继承自Dispatcher，因此可以分发消息：
{% highlight string %}
void Monitor::_ms_dispatch(Message *m){
	...

	if ((is_synchronizing() || (s->global_id == 0 && !exited_quorum.is_zero())) &&
	   !src_is_mon && m->get_type() != CEPH_MSG_PING) {
		waitlist_or_zap_client(op);
	} else {
		dispatch_op(op);
	}
}


void Monitor::dispatch_op(MonOpRequestRef op){
	...

	switch (op->get_req()->get_type()) {
	
		// OSDs
		case CEPH_MSG_MON_GET_OSDMAP:
		case MSG_OSD_MARK_ME_DOWN:
		case MSG_OSD_FAILURE:
		case MSG_OSD_BOOT:
		case MSG_OSD_ALIVE:
		case MSG_OSD_PGTEMP:
		case MSG_REMOVE_SNAPS:
			paxos_service[PAXOS_OSDMAP]->dispatch(op);
			break;

		...
	}

	...
}
{% endhighlight %}
这里来自OSD的消息会调用OSDMonitor的dispatch来进行处理：
{% highlight string %}
bool PaxosService::dispatch(MonOpRequestRef op){
	....

	// update
	if (prepare_update(op)) {
		double delay = 0.0;
		if (should_propose(delay)) {
			if (delay == 0.0) {
				propose_pending();
			} else {
				// delay a bit
				if (!proposal_timer) {
					/**
					* Callback class used to propose the pending value once the proposal_timer
					* fires up.
					*/
					proposal_timer = new C_MonContext(mon, [this](int r) {
						proposal_timer = 0;
						if (r >= 0)
							propose_pending();
						else if (r == -ECANCELED || r == -EAGAIN)
							return;
						else
							assert(0 == "bad return value for proposal_timer");
					});
					dout(10) << " setting proposal_timer " << proposal_timer << " with delay of " << delay << dendl;
					mon->timer.add_event_after(delay, proposal_timer);
				} else { 
					dout(10) << " proposal_timer already set" << dendl;
				}
			}

		} else {
			dout(10) << " not proposing" << dendl;
		}
	}   

	return true;  
}
{% endhighlight %}
之后调用prepare_update()来进行处理：
{% highlight string %}
bool OSDMonitor::prepare_update(MonOpRequestRef op)
{
	...

	switch (m->get_type()) {
		// damp updates
		case MSG_OSD_MARK_ME_DOWN:
			return prepare_mark_me_down(op);
		....
	}
	
	return false;
}
{% endhighlight %}

这里我们看到了实际的对于MOSDMarkMeDown消息的处理函数prepare_mark_me_down()函数：
{% highlight string %}
bool OSDMonitor::prepare_mark_me_down(MonOpRequestRef op)
{
	op->mark_osdmon_event(__func__);
	MOSDMarkMeDown *m = static_cast<MOSDMarkMeDown*>(op->get_req());
	int target_osd = m->get_target().name.num();
	
	assert(osdmap.is_up(target_osd));
	assert(osdmap.get_addr(target_osd) == m->get_target().addr);
	
	mon->clog->info() << "osd." << target_osd << " marked itself down\n";
	pending_inc.new_state[target_osd] = CEPH_OSD_UP;
	if (m->request_ack)
		wait_for_finished_proposal(op, new C_AckMarkedDown(this, op));
	return true;
}
{% endhighlight %}
如上，在函数中构造了一个proposal: C_AckMarkedDown，然后插入到waiting_for_finished_proposal列表中，准备对提议进行表决。因为Paxos需要对每一个提议进行持久化，因此这里会调用：
{% highlight string %}
void OSDMonitor::encode_pending(MonitorDBStore::TransactionRef t){
	...

	if (g_conf->mon_osd_prime_pg_temp)
		maybe_prime_pg_temp();

	...
}
{% endhighlight %}
跟踪到这里，我们发现osdmap中的pg_temp原来是由maybe_prime_pg_temp()来生成的：
{% highlight string %}
void OSDMonitor::maybe_prime_pg_temp(){
	...

	if(all){
		...
	}else{
		dout(10) << __func__ << " " << osds.size() << " interesting osds" << dendl;
		for (set<int>::iterator p = osds.begin(); p != osds.end(); ++p) {
			n -= prime_pg_temp(next, pg_map, *p);
			if (--n <= 0) {
				n = chunk;
				if (ceph_clock_now(NULL) > stop) {
					dout(10) << __func__ << " consumed more than "
						<< g_conf->mon_osd_prime_pg_temp_max_time
						<< " seconds, stopping"
						<< dendl;
					break;
				}
			}
		}
	}
}
{% endhighlight %}
上面调用prime_pg_temp()来完成osdmap中的pg_temp的构建。

###### 6.2 osdmap从```active+clean```时的e2222变成```in+down```时的e2225
在```active+clean```状态时，我们查看到osdmap的版本号为e2222，但是到了```in+down```状态时，我们查看到osdmap的版本号变为了e2225。这中间到底发生了一个什么样的过程呢？ 下面我们结合在Monitor上面抓取到的日志```mon-node7-1.txt```来进行分析。


执行如下命令导出e2222到e2225的osdmap:
{% highlight string %}
# ceph osd dump 2222 >> osdmap_2222.txt
# ceph osd dump 2223 >> osdmap_2223.txt
# ceph osd dump 2224 >> osdmap_2224.txt
# ceph osd dump 2225 >> osdmap_2225.txt
{% endhighlight %}


1） **osdmap从e2222变为e2223**

osd0在接收到关闭信号后，在OSDService::prepare_to_stop()函数中向OSDMonitor发送MOSDMarkMeDown消息。在OSDMonitor接收到MOSDMarkDown消息后，生成一个proposal来进行表决，其中proposal的内容为: e2223版本的osdmap。在该proposal表决通过之后，就形成了一个权威的```e2223```版本的osdmap，并正式标记osd0处于DOWN状态(此时此时我们查询ceph集群状态则为HEALTH_WARN)，然后OSDMonitor正式对外发布该版本的osdmap。

执行如下命令对比osdmap e2222和e2223:
{% highlight string %}
# diff osdmap_2222.txt osdmap_2223.txt -y --suppress-common-lines
epoch 2222                                                    | epoch 2223
modified 2020-09-11 12:13:01.076048                           | modified 2020-09-11 14:05:18.778903
osd.0 up   in  weight 1 up_from 2220 up_thru 2221 down_at 221 | osd.0 down in  weight 1 up_from 2220 up_thru 2221 down_at 222
                                                              > pg_temp 11.0 [6,0]
                                                              > pg_temp 11.4 [0,3]
                                                              > pg_temp 11.6 [3,0]
                                                              > pg_temp 13.1 [8,0]
                                                              > pg_temp 13.4 [0,6]
                                                              > pg_temp 13.9 [0,4]
                                                              > pg_temp 13.c [4,0]
                                                              > pg_temp 13.d [0,7]
                                                              > pg_temp 14.3 [0,5]
                                                              > pg_temp 14.5 [0,8]
                                                              > pg_temp 14.8 [0,5]
                                                              > pg_temp 14.d [0,5]
                                                              > pg_temp 15.5 [3,0]
                                                              > pg_temp 16.2 [8,0]
                                                              > pg_temp 17.0 [7,0]
                                                              > pg_temp 17.4 [6,0]
                                                              > pg_temp 18.1 [8,0]
                                                              > pg_temp 18.2 [0,4]
                                                              > pg_temp 18.3 [4,0]
                                                              > pg_temp 18.4 [0,6]
                                                              > pg_temp 19.1 [3,0]
                                                              > pg_temp 19.2 [0,4]
                                                              > pg_temp 20.3 [0,7]
                                                              > pg_temp 20.5 [5,0]
                                                              > pg_temp 21.1 [0,4]
                                                              > pg_temp 21.3 [8,0]
                                                              > pg_temp 21.6 [5,0]
                                                              > pg_temp 22.0 [3,0]
                                                              > pg_temp 22.e [0,3]
                                                              > pg_temp 22.19 [5,0]
                                                              > pg_temp 22.1e [4,0]
                                                              > pg_temp 22.21 [0,7]
                                                              > pg_temp 22.24 [7,0]
                                                              > pg_temp 22.2a [3,0]
                                                              > pg_temp 22.2c [0,3]
                                                              > pg_temp 22.2f [8,0]
                                                              > pg_temp 22.37 [7,0]
                                                              > pg_temp 22.3a [0,5]
                                                              > pg_temp 22.3b [4,0]
                                                              > pg_temp 22.44 [0,3]
                                                              > pg_temp 22.49 [8,0]
                                                              > pg_temp 22.4a [0,4]
                                                              > pg_temp 22.4b [7,0]
                                                              > pg_temp 22.4e [8,0]
                                                              > pg_temp 22.54 [8,0]
                                                              > pg_temp 22.65 [0,8]
                                                              > pg_temp 22.6b [0,8]
                                                              > pg_temp 22.73 [0,8]
                                                              > pg_temp 22.79 [0,8]
                                                              > pg_temp 22.7a [5,0]
                                                              > pg_temp 22.7f [8,0]
                                                              > pg_temp 22.85 [5,0]
                                                              > pg_temp 22.87 [6,0]
                                                              > pg_temp 22.8b [3,0]
                                                              > pg_temp 22.92 [0,7]
                                                              > pg_temp 22.96 [0,4]
                                                              > pg_temp 22.9a [7,0]
                                                              > pg_temp 22.a2 [0,8]
                                                              > pg_temp 22.a4 [0,3]
                                                              > pg_temp 22.a7 [0,6]
                                                              > pg_temp 22.b0 [6,0]
                                                              > pg_temp 22.b5 [3,0]
                                                              > pg_temp 22.b8 [6,0]
                                                              > pg_temp 22.b9 [5,0]
                                                              > pg_temp 22.bf [6,0]
                                                              > pg_temp 22.c0 [5,0]
                                                              > pg_temp 22.c4 [0,5]
                                                              > pg_temp 22.c8 [6,0]
                                                              > pg_temp 22.ca [0,3]
                                                              > pg_temp 22.cb [0,7]
                                                              > pg_temp 22.ce [0,4]
                                                              > pg_temp 22.d0 [3,0]
                                                              > pg_temp 22.d5 [4,0]
                                                              > pg_temp 22.d7 [0,8]
                                                              > pg_temp 22.df [7,0]
                                                              > pg_temp 22.e2 [0,8]
                                                              > pg_temp 22.ec [0,3]
                                                              > pg_temp 22.f1 [0,8]
                                                              > pg_temp 22.f5 [3,0]
                                                              > pg_temp 22.fa [5,0]
                                                              > pg_temp 23.2 [4,0]
                                                              > pg_temp 23.b [0,3]
                                                              > pg_temp 23.d [0,3]
                                                              > pg_temp 23.f [3,0]
                                                              > pg_temp 23.13 [3,0]
                                                              > pg_temp 23.1e [0,5]
                                                              > pg_temp 23.26 [0,6]
                                                              > pg_temp 23.30 [0,3]
                                                              > pg_temp 23.37 [0,8]
                                                              > pg_temp 23.38 [0,4]
                                                              > pg_temp 23.4b [5,0]
                                                              > pg_temp 23.4f [8,0]
                                                              > pg_temp 23.61 [0,4]
                                                              > pg_temp 23.6d [3,0]
                                                              > pg_temp 23.72 [0,7]
                                                              > pg_temp 23.73 [5,0]
                                                              > pg_temp 23.75 [0,7]
                                                              > pg_temp 23.76 [0,4]
                                                              > pg_temp 23.79 [0,8]
                                                              > pg_temp 23.7a [0,4]
                                                              > pg_temp 23.7b [0,7]
                                                              > pg_temp 23.7e [0,6]
                                                              > pg_temp 23.7f [6,0]
                                                              > pg_temp 23.83 [4,0]
                                                              > pg_temp 23.87 [7,0]
                                                              > pg_temp 23.89 [0,7]
                                                              > pg_temp 23.95 [4,0]
                                                              > pg_temp 23.96 [0,5]
                                                              > pg_temp 23.97 [6,0]
                                                              > pg_temp 23.9d [8,0]
                                                              > pg_temp 23.a1 [0,6]
                                                              > pg_temp 23.a4 [7,0]
                                                              > pg_temp 23.a7 [0,4]
                                                              > pg_temp 23.ad [4,0]
                                                              > pg_temp 23.ae [0,4]
                                                              > pg_temp 23.bb [0,3]
                                                              > pg_temp 23.bc [6,0]
                                                              > pg_temp 23.be [0,5]
                                                              > pg_temp 23.c1 [5,0]
                                                              > pg_temp 23.cb [0,7]
                                                              > pg_temp 23.cf [0,6]
                                                              > pg_temp 23.d2 [0,7]
                                                              > pg_temp 23.d5 [6,0]
                                                              > pg_temp 23.d6 [0,8]
                                                              > pg_temp 23.d7 [0,4]
                                                              > pg_temp 23.dc [6,0]
                                                              > pg_temp 23.de [4,0]
                                                              > pg_temp 23.df [5,0]
                                                              > pg_temp 23.e0 [0,4]
                                                              > pg_temp 23.e4 [0,6]
                                                              > pg_temp 23.e9 [3,0]
                                                              > pg_temp 23.ea [0,8]
                                                              > pg_temp 23.ef [4,0]
                                                              > pg_temp 23.f3 [8,0]
                                                              > pg_temp 23.fa [0,6]
                                                              > pg_temp 23.fc [3,0]
                                                              > pg_temp 24.4 [3,0]
                                                              > pg_temp 24.5 [0,8]
                                                              > pg_temp 25.5 [3,0]

{% endhighlight %}
可以看到osdmap e2222与e2223相比，变化主要有两点：

* osd0的状态由up状态变为down状态

* 为osd0上的所有PG都生成了一个pg_temp


>注： osdmap发生变化，也同时会触发对pgmap的检查，导致pgmap也发生改变，接着触发检查是否需要创建新的PG等操作。

在上面osdmap e2223成功发布后，OSD、PGMonitor等接收到新版本的osdmap，就会触发相应的动作。这里我们介绍一下PGMonitor的大概执行路径。PGMonitor会逐个检查集群中的各个OSD，看是否需要向其发送创建pg的请求；接着检查那些处于down状态的PG，并更新对应PG的状态为stale，形成新的PGmap版本：
<pre>
2020-09-11 14:05:19.834895 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103 check_down_pgs last_osdmap_epoch 2223
2020-09-11 14:05:19.835145 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.65 stale (acting_primary 0)
2020-09-11 14:05:19.835167 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.61 stale (acting_primary 0)
2020-09-11 14:05:19.835192 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.4a stale (acting_primary 0)
2020-09-11 14:05:19.835203 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.44 stale (acting_primary 0)
2020-09-11 14:05:19.835216 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.3a stale (acting_primary 0)
2020-09-11 14:05:19.835222 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.38 stale (acting_primary 0)
2020-09-11 14:05:19.835228 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.37 stale (acting_primary 0)
2020-09-11 14:05:19.835237 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.30 stale (acting_primary 0)
2020-09-11 14:05:19.835244 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.2c stale (acting_primary 0)
2020-09-11 14:05:19.835253 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.26 stale (acting_primary 0)
2020-09-11 14:05:19.835261 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.21 stale (acting_primary 0)
2020-09-11 14:05:19.835270 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 14.d stale (acting_primary 0)
2020-09-11 14:05:19.835276 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 13.d stale (acting_primary 0)
2020-09-11 14:05:19.835284 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 14.8 stale (acting_primary 0)
2020-09-11 14:05:19.835313 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 13.9 stale (acting_primary 0)
2020-09-11 14:05:19.835336 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 14.3 stale (acting_primary 0)
2020-09-11 14:05:19.835347 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 11.4 stale (acting_primary 0)
2020-09-11 14:05:19.835358 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 13.4 stale (acting_primary 0)
2020-09-11 14:05:19.835363 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.1e stale (acting_primary 0)
2020-09-11 14:05:19.835371 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 14.5 stale (acting_primary 0)
2020-09-11 14:05:19.835381 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 18.2 stale (acting_primary 0)
2020-09-11 14:05:19.835389 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 19.2 stale (acting_primary 0)
2020-09-11 14:05:19.835401 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 21.1 stale (acting_primary 0)
2020-09-11 14:05:19.835411 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 18.4 stale (acting_primary 0)
2020-09-11 14:05:19.835419 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 20.3 stale (acting_primary 0)
2020-09-11 14:05:19.835428 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.6b stale (acting_primary 0)
2020-09-11 14:05:19.835437 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.72 stale (acting_primary 0)
2020-09-11 14:05:19.835441 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.73 stale (acting_primary 0)
2020-09-11 14:05:19.835446 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.75 stale (acting_primary 0)
2020-09-11 14:05:19.835452 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.76 stale (acting_primary 0)
2020-09-11 14:05:19.835456 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.79 stale (acting_primary 0)
2020-09-11 14:05:19.835461 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.79 stale (acting_primary 0)
2020-09-11 14:05:19.835465 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.7b stale (acting_primary 0)
2020-09-11 14:05:19.835470 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.7a stale (acting_primary 0)
2020-09-11 14:05:19.835476 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.7e stale (acting_primary 0)
2020-09-11 14:05:19.835489 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.89 stale (acting_primary 0)
2020-09-11 14:05:19.835501 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.92 stale (acting_primary 0)
2020-09-11 14:05:19.835508 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.96 stale (acting_primary 0)
2020-09-11 14:05:19.835512 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.96 stale (acting_primary 0)
2020-09-11 14:05:19.835524 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.b stale (acting_primary 0)
2020-09-11 14:05:19.835529 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.a1 stale (acting_primary 0)
2020-09-11 14:05:19.835535 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.a2 stale (acting_primary 0)
2020-09-11 14:05:19.835541 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.a4 stale (acting_primary 0)
2020-09-11 14:05:19.835546 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.a7 stale (acting_primary 0)
2020-09-11 14:05:19.835551 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.a7 stale (acting_primary 0)
2020-09-11 14:05:19.835560 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.ae stale (acting_primary 0)
2020-09-11 14:05:19.835565 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 24.5 stale (acting_primary 0)
2020-09-11 14:05:19.835577 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.bb stale (acting_primary 0)
2020-09-11 14:05:19.835586 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.be stale (acting_primary 0)
2020-09-11 14:05:19.835592 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.d stale (acting_primary 0)
2020-09-11 14:05:19.835600 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.c4 stale (acting_primary 0)
2020-09-11 14:05:19.835608 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.cb stale (acting_primary 0)
2020-09-11 14:05:19.835613 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.ca stale (acting_primary 0)
2020-09-11 14:05:19.835618 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.cb stale (acting_primary 0)
2020-09-11 14:05:19.835622 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.cf stale (acting_primary 0)
2020-09-11 14:05:19.835627 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.ce stale (acting_primary 0)
2020-09-11 14:05:19.835636 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.d2 stale (acting_primary 0)
2020-09-11 14:05:19.835642 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.d7 stale (acting_primary 0)
2020-09-11 14:05:19.835647 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.d6 stale (acting_primary 0)
2020-09-11 14:05:19.835652 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.d7 stale (acting_primary 0)
2020-09-11 14:05:19.835663 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.e stale (acting_primary 0)
2020-09-11 14:05:19.835669 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.e0 stale (acting_primary 0)
2020-09-11 14:05:19.835675 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.e2 stale (acting_primary 0)
2020-09-11 14:05:19.835696 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.e4 stale (acting_primary 0)
2020-09-11 14:05:19.835705 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.ea stale (acting_primary 0)
2020-09-11 14:05:19.835711 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.ec stale (acting_primary 0)
2020-09-11 14:05:19.835720 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 22.f1 stale (acting_primary 0)
2020-09-11 14:05:19.835731 7f8f9533c700 10 mon.node7-1@0(leader).pg v5839103  marking pg 23.fa stale (acting_primary 0)
</pre>
上面刚好是所有以osd0作为主OSD的PG。


2） **osdmap从e2223变为e2224**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2223.txt osdmap_2224.txt -y --suppress-common-lines
epoch 2223                                                    | epoch 2224
modified 2020-09-11 14:05:18.778903                           | modified 2020-09-11 14:05:19.804576
osd.3 up   in  weight 1 up_from 2123 up_thru 2221 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2223 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2221 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2223 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2221 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2223 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2221 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2223 down_at 205
{% endhighlight %}
从上面可以看到，这两个版本的osdmap的变化主要是相应osd的up_thru发生了改变。

在OSD端，申请up_thru是通过如下函数来进行的：
{% highlight string %}
void OSD::queue_want_up_thru(epoch_t want)
{
	map_lock.get_read();
	epoch_t cur = osdmap->get_up_thru(whoami);
	Mutex::Locker l(mon_report_lock);
	if (want > up_thru_wanted) {
		dout(10) << "queue_want_up_thru now " << want << " (was " << up_thru_wanted << ")"<< ", currently " << cur<< dendl;
		up_thru_wanted = want;
		send_alive();
	} else {
		dout(10) << "queue_want_up_thru want " << want << " <= queued " << up_thru_wanted<< ", currently " << cur<< dendl;
	}
	map_lock.put_read();
}

void OSD::send_alive()
{
	assert(mon_report_lock.is_locked());
	if (!osdmap->exists(whoami))
		return;
	epoch_t up_thru = osdmap->get_up_thru(whoami);
	dout(10) << "send_alive up_thru currently " << up_thru << " want " << up_thru_wanted << dendl;
	if (up_thru_wanted > up_thru) {
		dout(10) << "send_alive want " << up_thru_wanted << dendl;
		monc->send_mon_message(new MOSDAlive(osdmap->get_epoch(), up_thru_wanted));
	}
}
{% endhighlight %}
在OSDMonitor端，如下是其处理流程：
{% highlight string %}
void Monitor::_ms_dispatch(Message *m){
	...

	if ((is_synchronizing() || (s->global_id == 0 && !exited_quorum.is_zero())) &&
	   !src_is_mon && m->get_type() != CEPH_MSG_PING) {
		waitlist_or_zap_client(op);
	} else {
		dispatch_op(op);
	}
}


void Monitor::dispatch_op(MonOpRequestRef op){
	...

	switch (op->get_req()->get_type()) {
	
		// OSDs
		case CEPH_MSG_MON_GET_OSDMAP:
		case MSG_OSD_MARK_ME_DOWN:
		case MSG_OSD_FAILURE:
		case MSG_OSD_BOOT:
		case MSG_OSD_ALIVE:
		case MSG_OSD_PGTEMP:
		case MSG_REMOVE_SNAPS:
			paxos_service[PAXOS_OSDMAP]->dispatch(op);
			break;

		...
	}

	...
}
bool OSDMonitor::preprocess_query(MonOpRequestRef op)
{
	....
	switch (m->get_type()) {
	...

	case MSG_OSD_ALIVE:
		return preprocess_alive(op);

	...
	}
}
{% endhighlight %}
接下来我们来看对```MSG_OSD_ALIVE```的处理：

* 预处理阶段
{% highlight string %}
bool OSDMonitor::preprocess_alive(MonOpRequestRef op)
{
	op->mark_osdmon_event(__func__);
	MOSDAlive *m = static_cast<MOSDAlive*>(op->get_req());
	int from = m->get_orig_source().num();
	
	// check permissions, ignore if failed
	MonSession *session = m->get_session();
	if (!session)
		goto ignore;
	if (!session->is_capable("osd", MON_CAP_X)) {
		dout(0) << "attempt to send MOSDAlive from entity with insufficient privileges:"<< session->caps << dendl;
		goto ignore;
	}
	
	if (!osdmap.is_up(from) ||
		osdmap.get_inst(from) != m->get_orig_source_inst()) {
		dout(7) << "preprocess_alive ignoring alive message from down " << m->get_orig_source_inst() << dendl;
		goto ignore;
	}
	
	if (osdmap.get_up_thru(from) >= m->want) {
	// yup.
		dout(7) << "preprocess_alive want up_thru " << m->want << " dup from " << m->get_orig_source_inst() << dendl;
		_reply_map(op, m->version);
		return true;
	}
	
	dout(10) << "preprocess_alive want up_thru " << m->want<< " from " << m->get_orig_source_inst() << dendl;
	return false;
	
ignore:
	return true;
}
{% endhighlight %}
如果该请求可以在本阶段完成，直接返回true，否则返回false。这里如果所请求的osd的up_thru已经大于m->want，那么直接将对应的osdmap返回给OSD即可。否则需要下一步的处理。

* 后续处理阶段
{% highlight string %}
bool OSDMonitor::prepare_update(MonOpRequestRef op)
{
	...

	switch (m->get_type()) {
	...

	case MSG_OSD_ALIVE:
    return prepare_alive(op);
	...
	}
}

bool OSDMonitor::prepare_alive(MonOpRequestRef op)
{
	op->mark_osdmon_event(__func__);
	MOSDAlive *m = static_cast<MOSDAlive*>(op->get_req());
	int from = m->get_orig_source().num();
	
	if (0) {  // we probably don't care much about these
	mon->clog->debug() << m->get_orig_source_inst() << " alive\n";
	}
	
	dout(7) << "prepare_alive want up_thru " << m->want << " have " << m->version<< " from " << m->get_orig_source_inst() << dendl;
	
	update_up_thru(from, m->version); // set to the latest map the OSD has
	wait_for_finished_proposal(op, new C_ReplyMap(this, op, m->version));
	return true;
}
{% endhighlight %}
可以看到这里直接生成一个proposal，提交表决，表决通过后将响应返回给OSD。





3) **osdmap从e2224变为e2225**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2224.txt osdmap_2225.txt -y --suppress-common-lines
epoch 2224                                                    | epoch 2225
modified 2020-09-11 14:05:19.804576                           | modified 2020-09-11 14:05:20.831242
osd.5 up   in  weight 1 up_from 2127 up_thru 2221 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2224 down_at 212
osd.8 up   in  weight 1 up_from 2087 up_thru 2221 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2223 down_at 208
{% endhighlight %}
上面我们看到，osdmap的变化也是up_thru发生了更改。


###### 6.2 osdmap从```in+down```时的e2225变成```active+clean```时的e2231
在e2225时，osd0处于down状态已经达到了5分钟，然后OSDMonitor自动将osd0标记为out状态，触发新的peering操作。这里我们将这一过程中的osdmap dump出来：
{% highlight string %}
# ceph osd dump 2226 >> osdmap_2226.txt
# ceph osd dump 2227 >> osdmap_2227.txt
# ceph osd dump 2228 >> osdmap_2228.txt
# ceph osd dump 2229 >> osdmap_2229.txt
# ceph osd dump 2230 >> osdmap_2230.txt
# ceph osd dump 2231 >> osdmap_2231.txt
{% endhighlight %}

1） **osdmap从e2225变为e2226**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2225.txt osdmap_2226.txt -y --suppress-common-lines
epoch 2225                                                    | epoch 2226
modified 2020-09-11 14:05:20.831242                           | modified 2020-09-11 14:10:18.902836
osd.0 down in  weight 1 up_from 2220 up_thru 2221 down_at 222 | osd.0 down out weight 0 up_from 2220 up_thru 2221 down_at 222
{% endhighlight %}
可以看到，osdmap的变化仅仅是osd.0的状态由down+in变为down+out。

2） **osdmap从e2226变为e2227**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2226.txt osdmap_2227.txt -y --suppress-common-lines
epoch 2226                                                    | epoch 2227
modified 2020-09-11 14:10:18.902836                           | modified 2020-09-11 14:10:19.937451
osd.3 up   in  weight 1 up_from 2123 up_thru 2223 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2226 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2223 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2226 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2224 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2226 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2223 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2226 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2223 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2226 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2223 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2226 down_at 208
pg_temp 11.0 [6,0]                                            <
pg_temp 11.4 [0,3]                                            <
pg_temp 11.6 [3,0]                                            <
pg_temp 13.1 [8,0]                                            <
pg_temp 13.4 [0,6]                                            <
pg_temp 13.9 [0,4]                                            <
pg_temp 13.c [4,0]                                            <
pg_temp 13.d [0,7]                                            <
pg_temp 16.2 [8,0]                                            <
pg_temp 17.4 [6,0]                                            <
pg_temp 18.1 [8,0]                                            <
pg_temp 18.2 [0,4]                                            <
pg_temp 18.3 [4,0]                                            <
pg_temp 18.4 [0,6]                                            <
pg_temp 19.1 [3,0]                                            <
pg_temp 19.2 [0,4]                                            <
pg_temp 20.3 [0,7]                                            <
pg_temp 20.5 [5,0]                                            <
pg_temp 21.1 [0,4]                                            <
pg_temp 21.3 [8,0]                                            <
pg_temp 21.6 [5,0]                                            <
pg_temp 22.0 [3,0]                                            <
pg_temp 22.e [0,3]                                            <
pg_temp 22.19 [5,0]                                           <
pg_temp 22.1e [4,0]                                           <
pg_temp 22.21 [0,7]                                           <
pg_temp 22.24 [7,0]                                           <
pg_temp 22.2a [3,0]                                           <
pg_temp 22.2c [0,3]                                           <
pg_temp 22.2f [8,0]                                           <
pg_temp 22.37 [7,0]                                           <
pg_temp 22.3a [0,5]                                           <
pg_temp 22.3b [4,0]                                           <
pg_temp 22.44 [0,3]                                           <
pg_temp 22.49 [8,0]                                           <
pg_temp 22.4a [0,4]                                           <
pg_temp 22.4b [7,0]                                           <
pg_temp 22.4e [8,0]                                           <
pg_temp 22.54 [8,0]                                           <
pg_temp 22.65 [0,8]                                           <
pg_temp 22.6b [0,8]                                           <
pg_temp 22.73 [0,8]                                           <
pg_temp 22.79 [0,8]                                           <
pg_temp 22.7a [5,0]                                           <
pg_temp 22.7f [8,0]                                           <
pg_temp 22.85 [5,0]                                           <
pg_temp 22.87 [6,0]                                           <
pg_temp 22.8b [3,0]                                           <
pg_temp 22.92 [0,7]                                           <
pg_temp 22.96 [0,4]                                           <
pg_temp 22.9a [7,0]                                           <
pg_temp 22.a2 [0,8]                                           <
pg_temp 22.a4 [0,3]                                           <
pg_temp 22.a7 [0,6]                                           <
pg_temp 22.b0 [6,0]                                           <
pg_temp 22.b5 [3,0]                                           <
pg_temp 22.b8 [6,0]                                           <
pg_temp 22.b9 [5,0]                                           <
pg_temp 22.bf [6,0]                                           <
pg_temp 22.c0 [5,0]                                           <
pg_temp 22.c4 [0,5]                                           <
pg_temp 22.c8 [6,0]                                           <
pg_temp 22.ca [0,3]                                           <
pg_temp 22.cb [0,7]                                           <
pg_temp 22.ce [0,4]                                           <
pg_temp 22.d0 [3,0]                                           <
pg_temp 22.d5 [4,0]                                           <
pg_temp 22.d7 [0,8]                                           <
pg_temp 22.df [7,0]                                           <
pg_temp 22.e2 [0,8]                                           <
pg_temp 22.ec [0,3]                                           <
pg_temp 22.f1 [0,8]                                           <
pg_temp 22.f5 [3,0]                                           <
pg_temp 22.fa [5,0]                                           <
pg_temp 23.2 [4,0]                                            <
pg_temp 23.b [0,3]                                            <
pg_temp 23.d [0,3]                                            <
pg_temp 23.f [3,0]                                            <
pg_temp 23.13 [3,0]                                           <
pg_temp 23.1e [0,5]                                           <
pg_temp 23.26 [0,6]                                           <
pg_temp 23.30 [0,3]                                           <
pg_temp 23.37 [0,8]                                           <
pg_temp 23.38 [0,4]                                           <
pg_temp 23.4b [5,0]                                           <
pg_temp 23.4f [8,0]                                           <
pg_temp 23.61 [0,4]                                           <
pg_temp 23.6d [3,0]                                           <
pg_temp 23.72 [0,7]                                           <
pg_temp 23.73 [5,0]                                           <
pg_temp 23.75 [0,7]                                           <
pg_temp 23.76 [0,4]                                           <
pg_temp 23.79 [0,8]                                           <
pg_temp 23.7a [0,4]                                           <
pg_temp 23.7b [0,7]                                           <
pg_temp 23.7e [0,6]                                           <
pg_temp 23.7f [6,0]                                           <
pg_temp 23.83 [4,0]                                           <
pg_temp 23.87 [7,0]                                           <
pg_temp 23.89 [0,7]                                           <
pg_temp 23.95 [4,0]                                           <
pg_temp 23.96 [0,5]                                           <
pg_temp 23.97 [6,0]                                           <
pg_temp 23.9d [8,0]                                           <
pg_temp 23.a1 [0,6]                                           <
pg_temp 23.a4 [7,0]                                           <
pg_temp 23.a7 [0,4]                                           <
pg_temp 23.ad [4,0]                                           <
pg_temp 23.ae [0,4]                                           <
pg_temp 23.bb [0,3]                                           <
pg_temp 23.bc [6,0]                                           <
pg_temp 23.be [0,5]                                           <
pg_temp 23.c1 [5,0]                                           <
pg_temp 23.cb [0,7]                                           <
pg_temp 23.cf [0,6]                                           <
pg_temp 23.d2 [0,7]                                           <
pg_temp 23.d5 [6,0]                                           <
pg_temp 23.d6 [0,8]                                           <
pg_temp 23.d7 [0,4]                                           <
pg_temp 23.dc [6,0]                                           <
pg_temp 23.de [4,0]                                           <
pg_temp 23.df [5,0]                                           <
pg_temp 23.e0 [0,4]                                           <
pg_temp 23.e4 [0,6]                                           <
pg_temp 23.e9 [3,0]                                           <
pg_temp 23.ea [0,8]                                           <
pg_temp 23.ef [4,0]                                           <
pg_temp 23.f3 [8,0]                                           <
pg_temp 23.fa [0,6]                                           <
pg_temp 23.fc [3,0]                                           <
pg_temp 24.4 [3,0]                                            <
pg_temp 24.5 [0,8]                                            <
pg_temp 25.5 [3,0]                                            <
{% endhighlight %}
从上面我们可以看到，osd3、osd4、osd5、osd6、osd7、osd8的up_thru都发生了改变，同时删除了与osd0相关的pg_temp。

3） **osdmap从e2227变为e2228**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2227.txt osdmap_2228.txt -y --suppress-common-lines
epoch 2227                                                    | epoch 2228
modified 2020-09-11 14:10:19.937451                           | modified 2020-09-11 14:10:20.962661
osd.1 up   in  weight 1 up_from 2119 up_thru 2220 down_at 211 | osd.1 up   in  weight 1 up_from 2119 up_thru 2227 down_at 211
osd.2 up   in  weight 1 up_from 2118 up_thru 2220 down_at 209 | osd.2 up   in  weight 1 up_from 2118 up_thru 2227 down_at 209
osd.3 up   in  weight 1 up_from 2123 up_thru 2226 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2227 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2226 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2227 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2226 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2227 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2226 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2227 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2226 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2227 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2226 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2227 down_at 208
pg_temp 14.3 [0,5]                                            <
pg_temp 14.8 [0,5]                                            <
pg_temp 15.5 [3,0]                                            < 
{% endhighlight %}
从上面我们可以看到，osd.1~osd.8的up_thru都发生了改变，同时剩下的3个pg_temp也进行了删除。

4） **osdmap从e2228到e2229**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2228.txt osdmap_2229.txt -y --suppress-common-lines
epoch 2228                                                    | epoch 2229
modified 2020-09-11 14:10:20.962661                           | modified 2020-09-11 14:10:22.008209
osd.2 up   in  weight 1 up_from 2118 up_thru 2227 down_at 209 | osd.2 up   in  weight 1 up_from 2118 up_thru 2228 down_at 209
osd.3 up   in  weight 1 up_from 2123 up_thru 2227 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2228 down_at 209
osd.7 up   in  weight 1 up_from 2072 up_thru 2227 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2228 down_at 205
pg_temp 17.0 [7,0] 
{% endhighlight %}

5) **osdmap从e2229到e2230**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2229.txt osdmap_2230.txt -y --suppress-common-lines
epoch 2229                                                    | epoch 2230
modified 2020-09-11 14:10:22.008209                           | modified 2020-09-11 14:10:23.079014
osd.7 up   in  weight 1 up_from 2072 up_thru 2228 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2229 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2227 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2229 down_at 208
pg_temp 14.5 [0,8] 
{% endhighlight %}

6) **osdmap从e2230到e2231**

执行如下命令对比这两个版本的osdmap:
{% highlight string %}
# diff osdmap_2230.txt osdmap_2231.txt -y --suppress-common-lines
epoch 2230                                                    | epoch 2231
modified 2020-09-11 14:10:23.079014                           | modified 2020-09-11 14:10:24.139331
osd.2 up   in  weight 1 up_from 2118 up_thru 2228 down_at 209 | osd.2 up   in  weight 1 up_from 2118 up_thru 2230 down_at 209
{% endhighlight %}


###### 6.3 osd端工作流程

这里我们通过osd3的日志```osd3_watch.txt```来分析一下OSD端的工作流程。

OSD3通过OSD::ms_dispatch()函数接收到OSDMonitor发送过来的新版本的osdmap(e2223)，触发调用OSD::handle_osd_map()调用：
{% highlight string %}
void OSD::handle_osd_map(MOSDMap *m)
{
	....

	// superblock and commit
	write_superblock(t);
	store->queue_transaction(
		service.meta_osr.get(),
		std::move(t),
		new C_OnMapApply(&service, pinned_maps, last),
		new C_OnMapCommit(this, start, last, m), 0);


	service.publish_superblock(superblock);
}
{% endhighlight %}
在osdmap成功保存到硬盘之后，回调C_OnMapCommit()函数：
{% highlight string %}
struct C_OnMapCommit : public Context {
  OSD *osd;
  epoch_t first, last;
  MOSDMap *msg;
  C_OnMapCommit(OSD *o, epoch_t f, epoch_t l, MOSDMap *m)
    : osd(o), first(f), last(l), msg(m) {}
  void finish(int r) {
    osd->_committed_osd_maps(first, last, msg);
    msg->put();
  }
};

void OSD::_committed_osd_maps(epoch_t first, epoch_t last, MOSDMap *m){

	...
	
	// yay!
	consume_map();
	
	....
}
{% endhighlight %}
在_committed_osd_maps()中consume_map()是一个关键的核心函数，起到触发相关PG的Peering过程。通常会在如下两种情况下触发PG的peering过程：

* OSD启动时，加载所有PG，对于Primary是本OSD的PG，通过MakePrimary()进入Primary状态，然后默认直接进入Peering过程；

* 集群正常启动后，处于active+clean状态，接着某个OSD出现故障，引起osdmap发生改变，触发PG的peering过程

关于第一种情况，我们在《OSD启动流程》相关文章中已有说明，这里我们主要介绍一下第二种情形。

1） **consume_map()函数**
{% highlight string %}
void OSD::consume_map()
{
	...

	// scan pg's
	{
		RWLock::RLocker l(pg_map_lock);
		for (ceph::unordered_map<spg_t,PG*>::iterator it = pg_map.begin(); it != pg_map.end();++it) {
			PG *pg = it->second;
			pg->lock();
			pg->queue_null(osdmap->get_epoch(), osdmap->get_epoch());
			pg->unlock();
		}
	
		logger->set(l_osd_pg, pg_map.size());
	}
}
{% endhighlight %}
consume_map()会向所有PG发送一个NullEvt事件，接着将自己(PG)添加到对应OSD的```peering_wq```中。

>注：在osd3_watch.txt日志中，有大量类似于如下的打印信息
>
>2020-09-11 14:05:19.127331 7fba49ec6700  7 osd.3 2223 consume_map version 2223
>
>2020-09-11 14:05:19.129777 7fba49ec6700 30 osd.3 pg_epoch: 2222 pg[22.2c( empty local-les=2222 n=0 ec=155 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2222 pi=2207-2220/5 crt=0'0 active] lock
>
>2020-09-11 14:05:19.129793 7fba49ec6700 10 osd.3 pg_epoch: 2222 pg[22.2c( empty local-les=2222 n=0 ec=155 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2222 pi=2207-2220/5 crt=0'0 active] null
>
> 这些打印信息就是来自于上面的pg->lock()



2） **process_peering_events()函数**

OSD对应的```osd_tp```会检查peering_wq是否为空，如果不为空，触发调用process_peering_events():
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...

	if (!advance_pg(curmap->get_epoch(), pg, handle, &rctx, &split_pgs)) {
		// we need to requeue the PG explicitly since we didn't actually
		// handle an event
		peering_wq.queue(pg);
	} else {
		assert(!pg->peering_queue.empty());
		PG::CephPeeringEvtRef evt = pg->peering_queue.front();
		pg->peering_queue.pop_front();
		pg->handle_peering_event(evt, &rctx);
	}
	....
}
{% endhighlight %}
在process_peering_events()函数中调用advance_pg()完成PG OSDMap的追赶。

3） **advance_pg()函数**
{% highlight string %}
bool OSD::advance_pg(
  epoch_t osd_epoch, PG *pg,
  ThreadPool::TPHandle &handle,
  PG::RecoveryCtx *rctx,
  set<boost::intrusive_ptr<PG> > *new_pgs)
{
	....

	for (;
		next_epoch <= osd_epoch && next_epoch <= max;
		++next_epoch) {
	
		vector<int> newup, newacting;
		int up_primary, acting_primary;
		
		nextmap->pg_to_up_acting_osds(
			pg->info.pgid.pgid,
			&newup, &up_primary,
			&newacting, &acting_primary);
		
		pg->handle_advance_map(
			nextmap, lastmap, newup, up_primary,
			newacting, acting_primary, rctx);

		...

	}
	....

	pg->handle_activate_map(rctx);
	if (next_epoch <= osd_epoch) {
		dout(10) << __func__ << " advanced to max " << max
			<< " past min epoch " << min_epoch
			<< " ... will requeue " << *pg << dendl;
		return false;
	}
	return true;
}
{% endhighlight %}
在advance_pg()中，for循环逐步完成PG对osdmap的追赶。里面涉及到的两个比较关键的函数是pg->handle_advance_map()以及pg->handle_activate_map()，下面我们分别介绍。

4） **handle_advance_map()函数**
{% highlight string %}
void PG::handle_advance_map(
  OSDMapRef osdmap, OSDMapRef lastmap,
  vector<int>& newup, int up_primary,
  vector<int>& newacting, int acting_primary,
  RecoveryCtx *rctx)
{
	....
	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);

	recovery_state.handle_event(evt, rctx);
	...
}
{% endhighlight %}
在handle_advance_map()函数中会发起一个AdvMap事件，并调用recovery_state来进行处理。我们搜索AdvMap事件，发现在如下状态下都可接受该事件：

* Reset状态

* Started状态

* WaitActingChange状态

* Peering状态

* Active状态

* GetLog状态

* Incomplete状态

之后我们在查看各个状态的react()处理函数，发现基本上都是会触发Peering操作。例如：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const AdvMap& advmap){
	...

	return forward_event();
}
{% endhighlight %}
Active状态下收到AdvMap事件后，先进行一些其他的前置操作，之后调用forward_event()将事件交由父状态机处理。结合状态机转换图，Active状态机的父状态机为Started：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap)
{
	dout(10) << "Started advmap" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->check_full_transition(advmap.lastmap, advmap.osdmap);
	if (pg->should_restart_peering(
		advmap.up_primary,
		advmap.acting_primary,
		advmap.newup,
		advmap.newacting,
		advmap.lastmap,
		advmap.osdmap)) {
		dout(10) << "should_restart_peering, transitioning to Reset" << dendl;
		post_event(advmap);
		return transit< Reset >();
	}

	pg->remove_down_peer_info(advmap.osdmap);
	return discard_event();
}
{% endhighlight %}
上面我们可以看到，如果需要重启进行Peering的话，那么就会转到Reset状态。

5） **handle_activate_map()函数**
{% highlight string %}
void PG::handle_activate_map(RecoveryCtx *rctx)
{
	dout(10) << "handle_activate_map " << dendl;
	ActMap evt;
	recovery_state.handle_event(evt, rctx);
	if (osdmap_ref->get_epoch() - last_persisted_osdmap_ref->get_epoch() >
		cct->_conf->osd_pg_epoch_persisted_max_stale) {
		dout(20) << __func__ << ": Dirtying info: last_persisted is "
			<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;
		dirty_info = true;
	} else {
		dout(20) << __func__ << ": Not dirtying info: last_persisted is "
			<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;
	}
	if (osdmap_ref->check_new_blacklist_entries()) check_blacklisted_watchers();
}
{% endhighlight %}

handle_activate_map()函数会向recovery_state投递一个ActMap事件，并进行处理。我们搜索ActMap事件，发现在如下状态下都可接受该事件：

* Reset状态

* Primary状态

* Active状态

* ReplicaActive状态

* Stray状态

* WaitUpThru状态

之后我们在查看各个状态的react()处理函数，发现基本上都是调用pg->take_waiters()向OSD投递一些因peering而延后的事件，即这些事件需要等待peering完成之后来处理。例如：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const ActMap&)
{
	...

	return forward_event();
}
{% endhighlight %}
Active状态下收到ActMap事件后，检查PG是否处于clean状态，如果不是，则将自己加入到recovery队列等待恢复（注： peering完成后，PG并不一定处于clean状态）。之后再调用forward_event()将事件交由父状态机处理。结合状态机转换图，Active状态的父状态为Primary:
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
在Primary::react()中对于ActMap主要是处理一些Peering完成后的事件。

>实际上，有上面4）、5）的分析，我们发现，```AdvMap```主要用于触发Peering过程，通常到PG::start_peering_interval()就结束；而```ActMap```主要用于触发处理Peering完成后的一些事件。AdvMap是Advance OSDMap的缩写，而ActMap是Activate OSDMap的缩写，在处理actmap事件时都会调用pg->take_waiters()来等待相关的请求处理完成。

到这里，整个Peering的触发过程，我们基本上有了一个比较清晰的了解，而关于Peering的一些细节问题，我们会在后续相关的章节中继续讲解。




<br />
<br />
<br />
