---
layout: post
title: ceph配置文件恢复
tags:
- ceph
categories: ceph
description: ceph配置文件恢复
---

本文主要讲述在ceph集群由于某种原因导致/etc/ceph目录下的配置文件丢失的情况下，如何通过一个当前正常运行的ceph线上环境来进行配置文件恢复。
<!-- more -->

## 1. ceph配置文件

要根据线上ceph运行环境进行配置文件恢复，我们需要了解在一般情况下，ceph配置文件有哪些，以及一些ceph的默认配置值。

(1) /etc/ceph目录
<pre>
[root@mceph-node1 lzy]# ls /etc/ceph/
ceph.client.admin.keyring  ceph.client.radosgw.keyring  ceph.conf  rbdmap
</pre>


(2) /var/lib/ceph目录
<pre>
[root@mceph-node1 lzy]# ls /var/lib/ceph/
bootstrap-mds  bootstrap-osd  bootstrap-rgw  mds  mon  osd  radosgw  tmp
[root@mceph-node1 lzy]# ls /var/lib/ceph/mon/ceph-mceph-node1/
done  keyring  store.db  sysvinit
[root@mceph-node1 lzy]# ls /var/lib/ceph/osd/ceph-0/
ceph_fsid  current  fsid  keyring  magic  ready  store_version  superblock  whoami
</pre>

建立ceph集群时，默认cluster为```ceph```，在配置文件中可以通过$cluster来引用；默认的管理用户为```client.admin```，在配置文件中可以通过$name来引用。

这里我们主要考虑/etc/ceph目录下的配置的恢复。/var/lib/ceph主要是与运行时相关的数据，这里暂时不考虑。通过上面我们看到，在/etc/ceph/ 目录下,主要有keyring文件及ceph.conf配置文件。


现假如我们有如下线上环境：
<pre>
[root@mceph-node1 lzy]# ceph osd tree
ID  WEIGHT  TYPE NAME           UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-12 1.50000 root ssd                                              
 -7 0.50000     host node1-ssd                                    
  0 0.50000         osd.0            up  1.00000          1.00000 
 -8 0.50000     host node2-ssd                                    
  4 0.50000         osd.4            up  1.00000          1.00000 
 -9 0.50000     host node3-ssd                                    
  8 0.50000         osd.8            up  1.00000          1.00000 
-11 4.50000 root sata                                             
 -2 1.50000     host node1-sata                                   
  1 0.50000         osd.1            up  1.00000                0 
  2 0.50000         osd.2            up  1.00000                0 
  3 0.50000         osd.3            up  1.00000                0 
 -4 1.50000     host node2-sata                                   
  5 0.50000         osd.5            up  1.00000                0 
  6 0.50000         osd.6            up  1.00000                0 
  7 0.50000         osd.7            up  1.00000                0 
 -6 1.50000     host node3-sata                                   
  9 0.50000         osd.9            up  1.00000                0 
 10 0.50000         osd.10           up  1.00000                0 
 11 0.50000         osd.11           up  1.00000                0 
-10 6.00000 root all                                              
 -1 2.00000     host node1-all                                    
  0 0.50000         osd.0            up  1.00000          1.00000 
  1 0.50000         osd.1            up  1.00000                0 
  2 0.50000         osd.2            up  1.00000                0 
  3 0.50000         osd.3            up  1.00000                0 
 -3 2.00000     host node2-all                                    
  4 0.50000         osd.4            up  1.00000          1.00000 
  5 0.50000         osd.5            up  1.00000                0 
  6 0.50000         osd.6            up  1.00000                0 
  7 0.50000         osd.7            up  1.00000                0 
 -5 2.00000     host node3-all                                    
  8 0.50000         osd.8            up  1.00000          1.00000 
  9 0.50000         osd.9            up  1.00000                0 
 10 0.50000         osd.10           up  1.00000                0 
 11 0.50000         osd.11           up  1.00000                0 
</pre>
我们需要据此来恢复丢失的/etc/ceph目录。

## 2. 恢复keyring文件

首先查看当前ceph集群名称：
<pre>
[root@mceph-node1 ~]# ceph daemon osd.0 config show | grep cluster
    "cluster": "ceph",
    "cluster_addr": "172.20.55.8:0\/0",
    "cluster_network": "172.20.55.0\/24",
    "mon_cluster_log_to_syslog": "default=false",
    "mon_cluster_log_to_syslog_level": "info",
    "mon_cluster_log_to_syslog_facility": "daemon",
    "mon_cluster_log_file": "default=\/var\/log\/ceph\/ceph.$channel.log cluster=\/var\/log\/ceph\/ceph.log",
    "mon_cluster_log_file_level": "info",
    "auth_cluster_required": "cephx",
    "cephx_cluster_require_signatures": "false",
    "osd_rollback_to_cluster_snap": "",
</pre>
这里我们cluster为```ceph```。


在/etc/ceph目录下一般会存放client.admin的keyring文件，和rgw用户的keyring文件。下面我们对此keyring文件进行恢复。运行如下命令，查看当前ceph认证系统中有哪些用户：
<pre>
[root@mceph-node1 lzy]# ceph auth list
installed auth entries:

osd.0
        key: AQDmUFtZj5bCERAAVJjmFfYXnQKK56VHcoxWHw==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.1
        key: AQD9UFtZbhwBOhAAsqC2c83knmTiOr1uKd6lyg==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.10
        key: AQChUltZCG5lAhAAG/BKXfNmgZVIK1iv6Tv9FA==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.11
        key: AQCsUltZjg/mNhAAhxERjbbyokWWhRCpakLU/g==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.2
        key: AQAMUVtZpZ+uERAAMadVdbApJHjWbo/t5fqAoA==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.3
        key: AQAZUVtZ/5dUMRAAUPq9zLCk5VNkdnw/sQXlew==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.4
        key: AQBAUltZe0VSGBAAJVHlUzIBD3Wl2PvIKD5BMA==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.5
        key: AQBNUltZibFXFxAASeUv7vGNxdyPfl3s/k8V0A==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.6
        key: AQDAXVtZD6jCARAAuEaXgEx5saLaDDY133nD/A==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.7
        key: AQBoUltZRp3iIhAA/wpdn8RSr7He4908XgwZtQ==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.8
        key: AQCIUltZ20cSGBAA5Y4y1l0ATz711z3HIARMwQ==
        caps: [mon] allow rwx
        caps: [osd] allow *
osd.9
        key: AQCUUltZ5NMDGRAA1k2olJomiBW5stfjRdLNng==
        caps: [mon] allow rwx
        caps: [osd] allow *
client.admin
        key: AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg==
        auid: 0
        caps: [mds] allow
        caps: [mon] allow *
        caps: [osd] allow *
client.bootstrap-mds
        key: AQB0SVtZk5lwGRAAtTRzLSeIO+erS9467ME98Q==
        caps: [mon] allow profile bootstrap-mds
client.bootstrap-osd
        key: AQBzSVtZH7TzORAA4IEJESn92+izktW13IUfOg==
        caps: [mon] allow profile bootstrap-osd
client.bootstrap-rgw
        key: AQB0SVtZepbJCxAA3IYrgOCDCugOUZUGZGMXFA==
        caps: [mon] allow profile bootstrap-rgw
client.radosgw.mceph-node1
        key: AQB3ZltZt+4jFxAAehnC7BX+Rkya0xWzBFwEwA==
        caps: [mon] allow rwx
        caps: [osd] allow rwx
client.radosgw.mceph-node2
        key: AQAiaFtZQK9SMxAA7SgPnWA9C5mDoouZzjJjXA==
        caps: [mon] allow rwx
        caps: [osd] allow rwx
client.radosgw.mceph-node3
        key: AQAObFtZjWy6NxAAcGe5owlY9EtSVstndqJAGw==
        caps: [mon] allow rwx
        caps: [osd] allow rwx
</pre>

如上我们看到，当前cephx认证系统中存储有很多用户，我们可以将这些用户的信息全部导出。这里导出client.admin及client.radosgw.mceph-node1。
{% highlight string %}
# ceph auth export client.admin -o ${cluster}.client.admin.keyring

# ceph auth export client.admin -o ${cluster}.client.radosgw.keyring
{% endhighlight %}

请用上面具体的cluster名称替换${cluster}。例如：
<pre>
[root@mceph-node1 lzy]# ceph auth export client.admin -o ceph.client.admin.keyring
export auth(auid = 0 key=AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg== with 3 caps)
[root@mceph-node1 lzy]# 
[root@mceph-node1 lzy]# ceph auth export client.admin -o ceph.client.radosgw.keyring
export auth(auid = 0 key=AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg== with 3 caps)
[root@mceph-node1 lzy]# 
[root@mceph-node1 lzy]# cat ceph.client.admin.keyring 
[client.admin]
        key = AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
[root@mceph-node1 lzy]# cat ceph.client.radosgw.keyring 
[client.admin]
        key = AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
</pre>









<br />
<br />
<br />