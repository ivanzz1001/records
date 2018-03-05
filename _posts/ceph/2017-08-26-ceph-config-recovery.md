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
[root@mceph-node1 lzy]# ceph mon dump
dumped monmap epoch 1
epoch 1
fsid 1d5e7f3d-8e4a-43b6-9787-4c55196f8b1b
last_changed 2017-07-04 15:19:55.711176
created 2017-07-04 15:19:55.711176
0: 172.20.30.224:6789/0 mon.mceph-node1
1: 172.20.30.225:6789/0 mon.mceph-node2
2: 172.20.30.226:6789/0 mon.mceph-node3

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

# ceph auth export client.admin -o ${cluster}.client.radosrgw.keyring
{% endhighlight %}

请用上面具体的cluster名称替换${cluster}。例如：
<pre>
[root@mceph-node1 lzy]# ceph auth export client.admin -o ceph.client.admin.keyring
export auth(auid = 0 key=AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg== with 3 caps)
[root@mceph-node1 lzy]# 
[root@mceph-node1 lzy]# ceph auth export client.admin -o ceph.client.radosrgw.keyring
export auth(auid = 0 key=AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg== with 3 caps)
[root@mceph-node1 lzy]# 
[root@mceph-node1 lzy]# cat ceph.client.admin.keyring 
[client.admin]
        key = AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
[root@mceph-node1 lzy]# cat ceph.client.radosrgw.keyring 
[client.admin]
        key = AQB/QFtZ2uMFLBAA3Wh7ykaL9WYQQiupkYBcmg==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
</pre>

这样我们就恢复了ceph.client.admin.keyring及ceph.client.radosrgw.keyring两个文件。后续我们只需要将这两个文件拷贝到/etc/ceph/目录下即可。

## 恢复ceph.conf文件

/etc/ceph/ceph.conf是ceph启动时默认加载的配置文件。ceph配置文件中一般有如下section:
* global section
* 全局mon section
* 具体mon section 
* 全局osd section
* 具体osd section
* rgw section

因为ceph配置参数较多，总共约900个。因此大部分参数系统都已经给其指定了默认值。我们可以分别获取osd节点、mon节点、rgw节点的当前值，然后分别与默认值进行比较，找出其中的不同来恢复丢失的ceph.conf文件。


（1） 获取所有配置参数的默认值

我们可以通过如下命令来获取所有配置参数的默认值：
<pre>
[root@mceph-node1 lzy]# ceph --show-config > ceph_default.txt
[root@mceph-node1 lzy]# cat ceph_default.txt
name = client.admin
cluster = ceph
debug_none = 0/5
debug_lockdep = 0/0
debug_context = 0/0
debug_crush = 0/0
debug_mds = 0/0
debug_mds_balancer = 0/0
debug_mds_locker = 0/0
debug_mds_log = 0/0
debug_mds_log_expire = 0/0
debug_mds_migrator = 0/0
debug_buffer = 0/0
debug_timer = 0/0
debug_filer = 0/0
debug_striper = 0/1
debug_objecter = 0/0
debug_rados = 0/0

//后续省略
</pre>

（2） 获取osd节点当前值

我们可以通过如下命令获取osd节点（例如osd.0)的当前值:
<pre>
[root@mceph-node1 lzy]# ceph daemon osd.0 config show > ceph_osd.0.txt
[root@mceph-node1 lzy]# cat ceph_osd.0.txt
{
    "name": "osd.0",
    "cluster": "ceph",
    "debug_none": "0\/5",
    "debug_lockdep": "0\/0",
    "debug_context": "0\/0",
    "debug_crush": "0\/0",
    "debug_mds": "0\/0",
    "debug_mds_balancer": "0\/0",
    "debug_mds_locker": "0\/0",
    "debug_mds_log": "0\/0",
    "debug_mds_log_expire": "0\/0",
    "debug_mds_migrator": "0\/0",
    "debug_buffer": "0\/0",
    "debug_timer": "0\/0",
    "debug_filer": "0\/0",
    "debug_striper": "0\/1",
    "debug_objecter": "0\/0",
    "debug_rados": "0\/0",
    "debug_rbd": "0\/0",
    "debug_rbd_replay": "0\/5",

//后续省略
</pre>

（2） 获取mon节点的当前值

我们可以通过如下命令获取mon节点(例如mon.mceph-node1)的当前值：
<pre>
[root@mceph-node1 lzy]# ceph daemon mon.mceph-node1 config show > mon.mceph-node1.txt
[root@mceph-node1 lzy]# cat mon.mceph-node1.txt 
{
    "name": "mon.mceph-node1",
    "cluster": "ceph",
    "debug_none": "0\/5",
    "debug_lockdep": "0\/0",
    "debug_context": "0\/0",
    "debug_crush": "0\/0",
    "debug_mds": "0\/0",
    "debug_mds_balancer": "0\/0",
    "debug_mds_locker": "0\/0",
    "debug_mds_log": "0\/0",

//后续省略
</pre>

(3) 获取rgw节点的当前值

我们可以通过如下命令获取rgw节点（例如client.radosgw.mceph-node1)的当前值：
<pre>
[root@mceph-node1 lzy]# ceph daemon client.radosgw.mceph-node1 config show > rgw.mceph-node1.txt
[root@mceph-node1 lzy]# cat rgw.mceph-node1.txt | less
{
    "name": "client.radosgw.mceph-node1",
    "cluster": "ceph",
    "debug_none": "0\/5",
    "debug_lockdep": "0\/0",
    "debug_context": "0\/0",
    "debug_crush": "0\/0",
    "debug_mds": "0\/0",
    "debug_mds_balancer": "0\/0",
    "debug_mds_locker": "0\/0",
    "debug_mds_log": "0\/0",
    "debug_mds_log_expire": "0\/0",
    "debug_mds_migrator": "0\/0",
    "debug_buffer": "0\/0",
    "debug_timer": "0\/0",
    "debug_filer": "0\/0",
    "debug_striper": "0\/1",
    "debug_objecter": "0\/0",
    "debug_rados": "0\/0",
    "debug_rbd": "0\/0",
    "debug_rbd_replay": "0\/5",
    "debug_journaler": "0\/0",
    "debug_objectcacher": "0\/0",

//后续省略
</pre>


<br />
在获取了上述ceph配置参数默认值、osd当前值、mon当前值、rgw当前值之后，对数据进行适当的处理，然后通过相应的工具找出不同，将这些不同的数据提取出来，再进行后续的调整即可恢复出ceph.conf文件。但是这可能会比较麻烦，我们可以直接通过如下命令找出与默认值不同的参数（以osd.0, mon.mceph-node1, client.radosgw.mceph-node1节点为例）：
<pre>
[root@mceph-node1 lzy]# ceph daemon osd.0 config diff > osd.0.diff
[root@mceph-node1 lzy]# ceph daemon mon.mceph-node1 config diff > mon.mceph-node1.diff
[root@mceph-node1 lzy]# ceph daemon client.radosgw.mceph-node1 config diff > rgw.mceph-node1.diff

[root@mceph-node1 lzy]# cat osd.0.diff
{
    "diff": {
        "current": {
            "auth_client_required": "cephx",
            "auth_supported": "cephx",
            "cluster_addr": "172.20.55.8:0\/0",
            "cluster_network": "172.20.55.0\/24",
            "filestore_fd_cache_shards": "2048",
            "filestore_fd_cache_size": "131072",
            "filestore_fiemap": "true",
            "filestore_max_inline_xattrs": "6",
            "filestore_max_sync_interval": "300",
            "filestore_min_sync_interval": "30",
            "filestore_omap_header_cache_size": "204800",
//后续省略

[root@mceph-node1 lzy]# cat mon.mceph-node1.diff 
{
    "diff": {
        "current": {
            "auth_client_required": "cephx",
            "auth_supported": "cephx",
            "cluster_network": "172.20.55.0\/24",
            "fsid": "1d5e7f3d-8e4a-43b6-9787-4c55196f8b1b",
            "internal_safe_to_start_threads": "true",
            "leveldb_block_size": "65536",
            "leveldb_cache_size": "536870912",
            "leveldb_compression": "false",
            "leveldb_log": "",
            "leveldb_write_buffer_size": "33554432",
            "log_to_stderr": "false",
            "mon_cluster_log_file": "\/var\/log\/ceph\/ceph.log",
            "mon_cluster_log_to_syslog": "False",
            "mon_host": "172.20.30.224,172.20.30.225,172.20.30.226",
            "mon_initial_members": "mceph-node1,mceph-node2,mceph-node3",
            "mon_osd_adjust_down_out_interval": "false",
            "mon_osd_adjust_heartbeat_grace": "false",
            "mon_osd_allow_primary_affinity": "true",
            "mon_osd_down_out_interval": "43200",
            "mon_pg_warn_max_per_osd": "0",
            "mon_warn_on_legacy_crush_tunables": "false",
//后续省略

[root@mceph-node1 lzy]# cat rgw.mceph-node1.diff 
{
    "diff": {
        "current": {
            "admin_socket": "\/var\/run\/ceph\/radosgw-mceph-node1.asok",
            "auth_client_required": "cephx",
            "auth_supported": "cephx",
            "cluster_network": "172.20.55.0\/24",
            "daemonize": "true",
            "fsid": "1d5e7f3d-8e4a-43b6-9787-4c55196f8b1b",
            "internal_safe_to_start_threads": "true",
            "keyring": "\/etc\/ceph\/ceph.client.radosgw.keyring",
            "log_file": "\/var\/log\/ceph\/radosgw-mceph-node1.log",
            "log_to_stderr": "false",
            "mon_host": "172.20.30.224,172.20.30.225,172.20.30.226",
            "mon_initial_members": "mceph-node1,mceph-node2,mceph-node3",
            "mon_pg_warn_max_per_osd": "200",
            "osd_crush_chooseleaf_type": "0",
            "osd_pool_default_crush_rule": "5",
            "osd_pool_default_min_size": "2",
            "public_network": "172.20.30.0\/24",
            "rgw_enable_usage_log": "true",
            "rgw_frontends": "civetweb port=7480",
            "rgw_multipart_min_part_size": "524288",
//后续省略

</pre>

将这些diff提取出来之后，我们可以直接创建出ceph-preprocess.conf文件，将这些differ的当前值分别填入配置文件的各个section下。例如：
<pre>
[root@mceph-node1 lzy]# cat ceph-preprocess.conf 
[mon.mceph-node1]
# monitor mceph-node1 differ 处理后的值



[osd.0]
# osd.0 differ 处理后的值


[client.radosgw.mceph-node1]
# rgw mceph-node1 differ 处理后的值


[root@mceph-node1 lzy]# 
</pre>
然后对这些节点的值进行整理，看哪些字段可以提升为[global] section，哪些字段可以提升为全局[mon] section，哪些字段可以提升为全局[osd] section。调整完之后就可以形成最后的ceph.conf文件，拷贝到/etc/ceph/目录即可。一般ceph.conf配置文件各个section下有哪些字段，这里给出一个大体的参考：
<pre>
[global]
# fsid 字段
# public network 
# cluster network


# auth_service_required
# auth_supported
# auth_cluster_required
# auth_client_required

# mon_initial_members = mceph-node1,mceph-node2,mceph-node3
# mon_host = 172.20.30.220,172.20.30.221,172.20.30.222
# mon_osd_full_ratio 
# mon_osd_nearfull_ratio 

# osd_pool_default_size = 3
# osd_pool_default_min_size = 2

# debug相关字段


[mon]
# mon_cluster_log_file
# mon_pg_warn_max_per_osd
# mon_warn_on_legacy_crush_tunables
# mon_osd_down_out_interval
# mon_osd_adjust_heartbeat_grace

[osd]
# osd相关
# journal日志相关
# leveldb相关
# filestore相关


</pre>



## 3. 总结

ceph在运行过程中内存中保存有所有相关的必要信息，并且自身也提供了很方便的接口/工具将这些数据导出。在配置文件丢失的情况下，我们也可以据此很容易的进行恢复。






<br />
<br />
<br />