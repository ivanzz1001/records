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
            election epoch 24424, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2158: 9 osds: 9 up, 9 in
            flags sortbitwise,require_jewel_osds
      pgmap v5819315: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76113 MB used, 225 GB / 299 GB avail
                 632 active+clean
</pre>
集群的初始状态为```active+clean```状态，PG的个数有632个。

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


1） **获取初始状态下的osdmap**

通过如下命令获取出当前状态的osdmap信息：
{% highlight string %}
# ceph osd dump > osdmap_active_clean.txt
# cat osdmap_active_clean.txt 
epoch 2158
fsid 5341b139-15dc-4c68-925a-179797d894d3
created 2018-05-11 16:11:51.479785
modified 2020-09-08 16:12:03.446719
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
osd.0 up   in  weight 1 up_from 2153 up_thru 2156 down_at 2142 last_clean_interval [2116,2141) 10.17.155.113:6800/17857 10.17.155.113:6801/17857 10.17.155.113:6802/17857 10.17.155.113:6803/17857 exists,up 67990973-0316-4f57-b721-ce61d886572c
osd.1 up   in  weight 1 up_from 2119 up_thru 2153 down_at 2114 last_clean_interval [2045,2113) 10.17.155.113:6808/5393 10.17.155.113:6809/5393 10.17.155.113:6810/5393 10.17.155.113:6811/5393 exists,up e6c55932-820c-474d-aff7-0ed4081f2a33
osd.2 up   in  weight 1 up_from 2118 up_thru 2153 down_at 2093 last_clean_interval [2045,2092) 10.17.155.113:6804/5391 10.17.155.113:6805/5391 10.17.155.113:6806/5391 10.17.155.113:6807/5391 exists,up dfe500d8-5778-4379-9c48-1f1390fa8f0a
osd.3 up   in  weight 1 up_from 2123 up_thru 2154 down_at 2095 last_clean_interval [2041,2093) 10.17.155.114:6800/894 10.17.155.114:6801/894 10.17.155.114:6802/894 10.17.155.114:6803/894 exists,up e00c8fe5-d49e-42d1-9bfb-4965b9ab75b3
osd.4 up   in  weight 1 up_from 2125 up_thru 2154 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6804/6327 10.17.155.114:6805/6327 10.17.155.114:6806/6327 10.17.155.114:6807/6327 exists,up dede8fcc-0b34-4296-83e2-a48966b22c36
osd.5 up   in  weight 1 up_from 2127 up_thru 2154 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6808/6391 10.17.155.114:6809/6391 10.17.155.114:6810/6391 10.17.155.114:6811/6391 exists,up c2cde97e-c27a-4560-a46c-68695be79ff1
osd.6 up   in  weight 1 up_from 2040 up_thru 2154 down_at 2039 last_clean_interval [2020,2038) 10.17.155.115:6800/820 10.17.155.115:6801/820 10.17.155.115:6802/820 10.17.155.115:6803/820 exists,up d12f28d8-8fff-4a77-b344-d5d0b3e6949c
osd.7 up   in  weight 1 up_from 2072 up_thru 2154 down_at 2051 last_clean_interval [2015,2038) 10.17.155.115:6804/26346 10.17.155.115:6805/26346 10.17.155.115:6806/26346 10.17.155.115:6807/26346 exists,up 26035aa5-6759-4856-8bdb-1507f5b052e6
osd.8 up   in  weight 1 up_from 2087 up_thru 2154 down_at 2085 last_clean_interval [2074,2086) 10.17.155.115:6808/26484 10.17.155.115:6812/1026484 10.17.155.115:6813/1026484 10.17.155.115:6814/1026484 exists,up bca13e21-d64c-433f-87e4-4d5ea309f28a
{% endhighlight %}

2) **获取初始状态下的pgmap**

使用如下命令获取初始状态的pgmap信息：
{% highlight string %}
# ceph pg dump > pgmap_active_clean.txt
# cat pgmap_active_clean.txt 
version 5819294
stamp 2020-09-08 17:47:57.116295
last_osdmap_epoch 2158
last_pg_scan 2158
full_ratio 0.95
nearfull_ratio 0.85
pg_stat objects mip     degr    misp    unf     bytes   log     disklog state   state_stamp     v       reported        up      up_primary      acting  acting_primary  last_scrub      scrub_stamp     last_deep_scrub deep_scrub_stamp
23.67   10      0       0       0       0       1095624955      56      56      active+clean    2020-09-08 13:06:01.953213      1956'56 2158:347        [1,7]   1       [1,7]   1       1956'56 2020-09-08 13:06:01.953174      1956'56 2020-09-02 05:54:57.973143
22.66   0       0       0       0       0       0       0       0       active+clean    2020-09-08 16:30:13.871826      0'0     2158:899        [8,2]   8       [8,2]   8       0'0     2020-09-08 16:30:13.871727      0'0     2020-09-06 09:41:03.238164
23.64   24      0       0       0       0       2194721422      99      99      active+clean    2020-09-08 08:20:44.896224      1962'99 2158:213        [3,2]   3       [3,2]   3       1962'99 2020-09-08 08:20:44.896151      1962'99 2020-09-03 07:27:12.251315
22.65   0       0       0       0       0       0       0       0       active+clean    2020-09-07 21:25:37.037135      0'0     2158:272        [0,8]   0       [0,8]   0       0'0     2020-09-07 21:25:37.037045      0'0     2020-09-05 09:58:02.887318
23.65   23      0       0       0       0       59042501        100     100     active+clean    2020-09-07 17:12:38.600461      1956'100        2158:2199       [4,7]   4       [4,7]   4       1956'100        2020-09-07 17:12:38.600346      1956'100       2020-09-06 16:56:41.273324
22.64   1       0       0       0       0       0       3000    3000    active+clean    2020-09-08 00:58:31.014725      972'4247        2158:5839       [2,3]   2       [2,3]   2       972'4247        2020-09-08 00:58:31.014644      972'4247      2020-09-05 15:30:14.870384
23.62   19      0       0       0       0       24117248        68      68      active+clean    2020-09-08 07:13:37.265108      1956'68 2158:300        [3,1]   3       [3,1]   3       1956'68 2020-09-08 07:13:37.265015      1956'68 2020-09-06 19:42:35.356916
22.63   1       0       0       0       0       0       2       2       active+clean    2020-09-08 02:11:32.448038      515'2   2158:276        [5,8]   5       [5,8]   5       515'2   2020-09-08 02:11:32.447972      515'2   2020-09-08 02:11:32.447972
23.63   15      0       0       0       0       27802844        57      57      active+clean    2020-09-07 21:26:11.841224      1956'57 2158:235        [3,7]   3       [3,7]   3       1956'57 2020-09-07 21:26:11.841118      1956'57 2020-09-02 17:39:31.045819
22.62   1       0       0       0       0       0       2       2       active+clean    2020-09-07 17:54:53.963798      201'2   2158:2422       [5,1]   5       [5,1]   5       201'2   2020-09-07 17:54:53.963734      201'2   2020-09-02 00:14:05.137488
23.60   28      0       0       0       0       3267227406      107     107     active+clean    2020-09-07 16:27:24.864408      1956'107        2158:302        [2,4]   2       [2,4]   2       1956'107        2020-09-07 16:27:24.864327      1956'107       2020-09-07 16:27:24.864327
22.61   0       0       0       0       0       0       0       0       active+clean    2020-09-08 15:40:23.405406      0'0     2158:891        [6,3]   6       [6,3]   6       0'0     2020-09-08 15:40:23.405299      0'0     2020-09-03 16:13:14.659013
23.61   25      0       0       0       0       31142253        67      67      active+clean    2020-09-07 20:41:57.004701      1956'67 2158:338        [0,4]   0       [0,4]   0       1956'67 2020-09-07 19:35:02.848429      1956'67 2020-09-05 09:24:54.042182
22.60   0       0       0       0       0       0       0       0       active+clean    2020-09-08 12:41:53.131421      0'0     2158:257        [7,5]   7       [7,5]   7       0'0     2020-09-08 12:41:53.131348      0'0     2020-09-08 12:41:53.131348
23.5e   21      0       0       0       0       1085440156      82      82      active+clean    2020-09-08 04:22:20.038621      1956'82 2158:308        [6,4]   6       [6,4]   6       1956'82 2020-09-08 04:22:20.038544      1956'82 2020-09-01 21:58:57.523530
22.5f   1       0       0       0       0       0       2       2       active+clean    2020-09-08 01:20:08.967847      525'2   2158:1881       [5,7]   5       [5,7]   5       525'2   2020-09-08 01:20:08.967769      525'2   2020-09-08 01:20:08.967769
23.5f   17      0       0       0       0       40385627        88      88      active+clean    2020-09-08 10:26:04.055691      1956'88 2158:2591       [4,1]   4       [4,1]   4       1956'88 2020-09-08 10:26:04.055631      1956'88 2020-09-07 03:34:11.800848
22.5e   0       0       0       0       0       0       0       0       active+clean    2020-09-08 17:13:21.905463      0'0     2158:1808       [4,8]   4       [4,8]   4       0'0     2020-09-08 17:13:21.905391      0'0     2020-09-05 23:26:35.067631
23.5c   6       0       0       0       0       9961472 24      24      active+clean    2020-09-07 12:12:49.907482      1956'24 2158:1944       [5,7]   5       [5,7]   5       1956'24 2020-09-07 12:12:49.907399      1956'24 2020-09-05 05:38:09.723392
22.5d   1       0       0       0       0       0       11      11      active+clean    2020-09-07 20:44:33.446559      741'11  2158:185        [3,6]   3       [3,6]   3       741'11  2020-09-07 20:44:33.446491      741'11  2020-09-06 17:25:41.760980
23.5d   12      0       0       0       0       7864320 90      90      active+clean    2020-09-08 06:26:24.812691      972'90  2158:2671       [5,1]   5       [5,1]   5       972'90  2020-09-08 06:26:24.812624      972'90  2020-09-05 22:08:07.539734
22.5c   0       0       0       0       0       0       0       0       active+clean    2020-09-08 17:13:21.275901      0'0     2158:799        [8,4]   8       [8,4]   8       0'0     2020-09-08 17:13:21.275795      0'0     2020-09-06 02:39:51.249894
23.5a   15      0       0       0       0       23097124        46      46      active+clean    2020-09-07 16:25:38.187875      1956'46 2158:370        [1,7]   1       [1,7]   1       1956'46 2020-09-07 16:25:38.187751      1956'46 2020-09-06 08:15:47.221954
22.5b   1       0       0       0       0       0       2       2       active+clean    2020-09-08 02:29:14.059006      515'2   2158:1043       [7,1]   7       [7,1]   7       515'2   2020-09-08 02:29:14.058958      515'2   2020-09-05 18:22:15.647682
23.5b   18      0       0       0       0       3250061312      71      71      active+clean    2020-09-08 08:52:27.189672      1962'71 2158:2338       [5,2]   5       [5,2]   5       1962'71 2020-09-08 08:52:27.189581      1962'71 2020-09-05 18:49:15.233786
22.5a   0       0       0       0       0       0       0       0       active+clean    2020-09-08 05:31:07.682502      0'0     2158:832        [6,4]   6       [6,4]   6       0'0     2020-09-08 05:31:07.682430      0'0     2020-09-08 05:31:07.682430
23.58   14      0       0       0       0       2164260864      59      59      active+clean    2020-09-07 14:20:16.268687      1956'59 2158:1179       [7,2]   7       [7,2]   7       1956'59 2020-09-07 14:20:16.268604      1956'59 2020-09-06 05:36:59.981873
22.59   0       0       0       0       0       0       0       0       active+clean    2020-09-08 13:47:50.358754      0'0     2158:303        [6,3]   6       [6,3]   6       0'0     2020-09-08 13:47:50.358687      0'0     2020-09-08 13:47:50.358687
23.59   15      0       0       0       0       3237485605      87      87      active+clean    2020-09-07 14:05:59.129504      1962'87 2158:492        [6,3]   6       [6,3]   6       1962'87 2020-09-07 14:05:59.129465      1962'87 2020-09-01 09:37:10.433056
22.58   0       0       0       0       0       0       0       0       active+clean    2020-09-08 04:01:49.446214      0'0     2158:1854       [4,7]   4       [4,7]   4       0'0     2020-09-08 04:01:49.446142      0'0     2020-09-08 04:01:49.446142
23.56   13      0       0       0       0       26949183        47      47      active+clean    2020-09-08 05:19:21.007986      1956'47 2158:317        [8,4]   8       [8,4]   8       1956'47 2020-09-08 05:19:21.007865      1956'47 2020-09-08 05:19:21.007865
22.57   0       0       0       0       0       0       0       0       active+clean    2020-09-08 05:33:15.311613      0'0     2158:187        [3,7]   3       [3,7]   3       0'0     2020-09-08 05:33:15.311537      0'0     2020-09-07 00:48:46.681187
23.57   17      0       0       0       0       19550580        52      52      active+clean    2020-09-07 22:11:25.602340      1956'52 2158:894        [6,5]   6       [6,5]   6       1956'52 2020-09-07 22:11:25.602260      1956'52 2020-09-03 14:15:49.966506
22.56   0       0       0       0       0       0       0       0       active+clean    2020-09-07 17:59:47.007147      0'0     2158:154        [3,1]   3       [3,1]   3       0'0     2020-09-07 17:59:47.007081      0'0     2020-09-07 17:59:47.007081
23.54   32      0       0       0       0       2189254725      149     149     active+clean    2020-09-08 04:04:39.486715      1962'149        2158:2754       [5,2]   5       [5,2]   5       1962'149        2020-09-08 04:04:39.486637      1962'149       2020-09-05 11:14:05.204444
22.55   0       0       0       0       0       0       0       0       active+clean    2020-09-08 00:07:39.314649      0'0     2158:2112       [4,2]   4       [4,2]   4       0'0     2020-09-08 00:07:39.314599      0'0     2020-09-04 16:29:23.553768
23.55   26      0       0       0       0       43803237        51      51      active+clean    2020-09-08 11:22:21.997487      1956'51 2158:660        [2,3]   2       [2,3]   2       1956'51 2020-09-08 11:22:21.997412      1956'51 2020-09-07 10:18:16.561822
22.54   0       0       0       0       0       0       0       0       active+clean    2020-09-08 13:51:57.400325      0'0     2158:273        [8,0]   8       [8,0]   8       0'0     2020-09-08 13:51:57.400240      0'0     2020-09-03 12:38:33.708448
23.52   12      0       0       0       0       1085840249      41      41      active+clean    2020-09-08 13:39:15.282687      1956'41 2158:343        [8,1]   8       [8,1]   8       1956'41 2020-09-08 13:39:15.282618      1956'41 2020-09-03 05:05:37.198707
....
{% endhighlight %}
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
# ceph pg ls-by-primary 0
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

修改成功后，我们将osd.3的日志重定向到一个新的文件中(osd3_watch.txt):
<pre>
# tail -f /var/log/ceph/ceph-osd.3.log > ./osd3_watch.txt
</pre>

## 2. osd.0处于(in+down)状态
1) **关闭osd.0，并观察集群状态**

执行如下命令手动关闭```osd.0```，并观察集群状态：
<pre>
# systemctl stop ceph-osd@0
# ceph -s
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_ERR
            40 pgs are stuck inactive for more than 300 seconds
            99 pgs degraded
            40 pgs peering
            40 pgs stuck inactive
            138 pgs stuck unclean
            99 pgs undersized
            recovery 918/11720 objects degraded (7.833%)
            1/9 in osds are down
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24424, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2160: 9 osds: 8 up, 9 in; 139 remapped pgs
            flags sortbitwise,require_jewel_osds
      pgmap v5819798: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76110 MB used, 225 GB / 299 GB avail
            918/11720 objects degraded (7.833%)
                 493 active+clean
                  99 active+undersized+degraded
                  40 peering
recovery io 0 B/s, 17 keys/s, 0 objects/s
</pre>
可以看到停止osd.0之后，集群马上出现```HEALTH_ERR```状态。接着再执行```ceph -s```命令：
<pre>
# ceph -w
    cluster 5341b139-15dc-4c68-925a-179797d894d3
     health HEALTH_WARN
            139 pgs degraded
            139 pgs stuck unclean
            139 pgs undersized
            recovery 1301/11720 objects degraded (11.101%)
            1/9 in osds are down
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24424, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2160: 9 osds: 8 up, 9 in; 139 remapped pgs
            flags sortbitwise,require_jewel_osds
      pgmap v5819806: 632 pgs, 15 pools, 206 GB data, 5860 objects
            76110 MB used, 225 GB / 299 GB avail
            1301/11720 objects degraded (11.101%)
                 493 active+clean
                 139 active+undersized+degraded

2020-09-08 19:32:41.953782 mon.0 [INF] pgmap v5819806: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 76110 MB used, 225 GB / 299 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-08 19:33:19.620972 mon.0 [INF] pgmap v5819807: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 76110 MB used, 225 GB / 299 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-08 19:33:25.531104 mon.0 [INF] HEALTH_WARN; 139 pgs degraded; 139 pgs stuck unclean; 139 pgs undersized; recovery 1301/11720 objects degraded (11.101%); 1/9 in osds are down
2020-09-08 19:34:00.393548 mon.0 [INF] pgmap v5819808: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 76110 MB used, 225 GB / 299 GB avail; 1301/11720 objects degraded (11.101%)
2020-09-08 19:34:17.323444 mon.0 [INF] pgmap v5819809: 632 pgs: 139 active+undersized+degraded, 493 active+clean; 206 GB data, 76110 MB used, 225 GB / 299 GB avail; 1301/11720 objects degraded (11.101%)
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
            72 pgs degraded
            3 pgs recovering
            69 pgs recovery_wait
            72 pgs stuck unclean
            recovery 2564/11720 objects degraded (21.877%)
     monmap e3: 3 mons at {node7-1=10.17.155.113:6789/0,node7-2=10.17.155.114:6789/0,node7-3=10.17.155.115:6789/0}
            election epoch 24424, quorum 0,1,2 node7-1,node7-2,node7-3
     osdmap e2167: 9 osds: 8 up, 8 in
            flags sortbitwise,require_jewel_osds
      pgmap v5819849: 632 pgs, 15 pools, 206 GB data, 5860 objects
            71995 MB used, 196 GB / 266 GB avail
            2564/11720 objects degraded (21.877%)
                 560 active+clean
                  69 active+recovery_wait+degraded
                   3 active+recovering+degraded
recovery io 18252 kB/s, 9 objects/s
  client io 7386 B/s rd, 0 B/s wr, 7 op/s rd, 4 op/s wr

2020-09-08 19:37:53.132802 mon.0 [INF] pgmap v5819848: 632 pgs: 3 active+recovering+degraded, 69 active+recovery_wait+degraded, 560 active+clean; 206 GB data, 71931 MB used, 196 GB / 266 GB avail; 4379 B/s rd, 0 B/s wr, 7 op/s; 2564/11720 objects degraded (21.877%); 18034 kB/s, 9 objects/s recovering
2020-09-08 19:37:54.149839 mon.0 [INF] pgmap v5819849: 632 pgs: 3 active+recovering+degraded, 69 active+recovery_wait+degraded, 560 active+clean; 206 GB data, 71995 MB used, 196 GB / 266 GB avail; 7386 B/s rd, 0 B/s wr, 12 op/s; 2564/11720 objects degraded (21.877%); 18252 kB/s, 9 objects/s recovering
2020-09-08 19:37:55.208008 mon.0 [INF] pgmap v5819850: 632 pgs: 3 active+recovering+degraded, 69 active+recovery_wait+degraded, 560 active+clean; 206 GB data, 71969 MB used, 196 GB / 266 GB avail; 16261 B/s rd, 0 B/s wr, 26 op/s; 2564/11720 objects degraded (21.877%)
2020-09-08 19:37:56.238560 mon.0 [INF] pgmap v5819851: 632 pgs: 3 active+recovering+degraded, 69 active+recovery_wait+degraded, 560 active+clean; 206 GB data, 72505 MB used, 196 GB / 266 GB avail; 8882 B/s rd, 0 B/s wr, 14 op/s; 2564/11720 objects degraded (21.877%)
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
epoch 2158                                                    | epoch 2160
modified 2020-09-08 16:12:03.446719                           | modified 2020-09-08 19:32:30.271816
osd.0 up   in  weight 1 up_from 2153 up_thru 2156 down_at 214 | osd.0 down in  weight 1 up_from 2153 up_thru 2156 down_at 215
osd.3 up   in  weight 1 up_from 2123 up_thru 2154 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2159 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2154 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2159 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2154 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2159 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2154 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2159 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2154 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2159 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2154 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2159 down_at 208
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
从上面我们可以看到首先是osdmap的epoch值发生了变动，由```2158```变成了```2160```。然后再是osdmap显示```osd.0```上的139个PG产生了pg_temp。

2） **```in+down```状态与```out+down```状态osdmap对比**
{% highlight string %}
# diff osdmap_in_down.txt osdmap_out_down.txt -y --suppress-common-lines
epoch 2160                                                    | epoch 2167
modified 2020-09-08 19:32:30.271816                           | modified 2020-09-08 19:37:39.203158
osd.0 down in  weight 1 up_from 2153 up_thru 2156 down_at 215 | osd.0 down out weight 0 up_from 2153 up_thru 2156 down_at 215
osd.1 up   in  weight 1 up_from 2119 up_thru 2153 down_at 211 | osd.1 up   in  weight 1 up_from 2119 up_thru 2162 down_at 211
osd.2 up   in  weight 1 up_from 2118 up_thru 2153 down_at 209 | osd.2 up   in  weight 1 up_from 2118 up_thru 2164 down_at 209
osd.3 up   in  weight 1 up_from 2123 up_thru 2159 down_at 209 | osd.3 up   in  weight 1 up_from 2123 up_thru 2166 down_at 209
osd.4 up   in  weight 1 up_from 2125 up_thru 2159 down_at 212 | osd.4 up   in  weight 1 up_from 2125 up_thru 2162 down_at 212
osd.5 up   in  weight 1 up_from 2127 up_thru 2159 down_at 212 | osd.5 up   in  weight 1 up_from 2127 up_thru 2164 down_at 212
osd.6 up   in  weight 1 up_from 2040 up_thru 2159 down_at 203 | osd.6 up   in  weight 1 up_from 2040 up_thru 2162 down_at 203
osd.7 up   in  weight 1 up_from 2072 up_thru 2159 down_at 205 | osd.7 up   in  weight 1 up_from 2072 up_thru 2163 down_at 205
osd.8 up   in  weight 1 up_from 2087 up_thru 2159 down_at 208 | osd.8 up   in  weight 1 up_from 2087 up_thru 2162 down_at 208
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
pg_temp 14.d [0,5]                                            <
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
从上面我们可以看到首先是osdmap的epoch值发生了变动，由```2160```变成了```2167```，并且此时pg_temp已经消失。

## 4. pgmap对比分析
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
14.d    [5]     5       [5]     5                             | 14.d    [2,5]   2       [2,5]   2
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






<br />
<br />
<br />


