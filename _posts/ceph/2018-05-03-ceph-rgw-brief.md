---
layout: post
title: rgw简要介绍
tags:
- ceph
categories: ceph
description: rgw简要介绍
---

本文简要介绍一下RGW。

<!-- more -->


## 1. RGW层次结构


![ceph-rgw](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/ceph-rgw-level.png)

RGW是建立在librados上的一个对象存储接口，支持两种类型的接口： s3-compatible、swift-compatible。由于RGW兼容s3/swift类型接口，因此RGW本身提供了其自身的用户管理功能。


## 2. MULTI-SITE

```MULTI-SITE```功能是从```Jewel```版本开始支持的。一个```single zone```配置通常是由一个```zone group```组成，该zone group包含一个zone和多个用于负载均衡的RGW实例。然而,Kraken为RGW提供了一些multi-site的配置选项。

* **Multi-zone**: A more advanced configuration consists of one zone group and multiple zones，每一个zone都包含着多个rgw实例。每一个zone后端都对应着其自己的Ceph Storage Cluster。一个zone group中的多个zone提供了容灾恢复。在Kraken中，zone group中的每一个zone都是active状态的，并且都会接受写操作。另外在灾难恢复时，所有处于active状态的zone都可以作为恢复源。

* **Multi-zone-group**: 在以前的话叫做```regions```。RGW也支持多个zone groups，每一个zone group包含多个zone。如果两个zone group的realm相同的话，则所存储的对象也会使用相同的全局namespace。	

* **Multiple Realms**： 在Kraken中, 支持每个zone group都有独立的realm； 也支持多个zone group共享一个realm； 也支持全局一个realm。

![rgw-multi-zone](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/rgw-multi-zone.jpg)


## 3 配置一个Multi-site
一个multi-site配置通常需要至少两个ceph storage cluster，建议每一个集群都采用不同的名字。而针对每一个ceph storage cluster都需至少一个rgw。本文假设在两个不同的地理位置有至少两个ceph集群，每个集群对应着一个rgw，分别为: rgw1和rgw2。

一个multi-site配置需要有一个master zone group和一个master zone， 在本例子中rgw1在master zone group的master zone中提供相应的服务； 而rgw2在master zone group的secondary zone中提供相应的服务。

![zone-sync2](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/zone-sync2.png)

### 3.1 配置一个master zone
在一个multi-site配置当中的所有rgw都会接收来自master zone group中的mater zone对应的ceph-radosgw的相关配置。因此首先我们必须先配置一个master zone group和一个master zone。

**1) 创建REALM**

一个realm包含multi-site配置中的```zone groups```以及```zones```， 并且在该realm中作为全局唯一的名称空间。我们在需要被设为```master zone group```的宿主机上执行如下命令：
<pre>
# radosgw-admin realm create --rgw-realm={realm-name} [--default]
</pre>
例如：
<pre>
# radosgw-admin realm create --rgw-realm=movies --default
</pre>

假如该realm只用于本集群的话，则添加```--default```选项。假如添加了```--default```选项，则radosgw-admin就默认会使用该realm。假如```--default```选项没有被设置的话，就可以通过```--rgw-realm```选项或者```--realm-id```选项添加其他的```zone-groups```和```zones```.

**2) 创建MASTER ZONE GROUP**

一个realm必须至少有一个master zone group。我们在需要被设为```master zone group```的宿主机上执行如下命令：
<pre>
# radosgw-admin zonegroup create --rgw-zonegroup={name} --endpoints={url} [--rgw-realm={realm-name}|--realm-id={realm-id}] --master --default
</pre>

例如：
<pre>
# radosgw-admin zonegroup create --rgw-zonegroup=us --endpoints=http://rgw1:80 --rgw-realm=movies --master --default
</pre>

假如该realm只有一个zone group的话，则指定```--default```选项。假如```--default```选项被指定了的话，则在添加新的```zones```时就会默认添加到该zone group中。假如```--default```选项未被指定的话，可以通过```--rgw-zonegroup```选项或者```--zonegroup-id```选项来指定需要添加到哪一个group。

**3） 创建Master Zone**

为一个multi-site配置添加新的master zone，可以采用如下命令：
<pre>
# radosgw-admin zone create --rgw-zonegroup={zone-group-name} \
                            --rgw-zone={zone-name} \
                            --master --default \
                            --endpoints={http://fqdn}[,{http://fqdn}]
</pre>
例如：
<pre>
# radosgw-admin zone create --rgw-zonegroup=us --rgw-zone=us-east \
                            --master --default \
                            --endpoints={http://fqdn}[,{http://fqdn}]
</pre>

**说明**： 这里并未指定```--access-key```与```--secret```。在下面的步骤我们创建用户的时候会自动的把这些设置添加到zone中。



**4) 删除默认的zone group与zone**

假如存在```default``` zone的话，首先确保要移除该默认的zone group:
<pre>
# radosgw-admin zonegroup remove --rgw-zonegroup=default --rgw-zone=default
# radosgw-admin period update --commit
# radosgw-admin zone delete --rgw-zone=default
# radosgw-admin period update --commit
# radosgw-admin zonegroup delete --rgw-zonegroup=default
# radosgw-admin period update --commit
</pre>
最后，删除ceph存储集群中的```default``` pools(**说明:** 下面的操作步骤是假设你要配置的multi-site当前未存储任何数据，如果当前default zone group存储有数据，删除将会导致数据丢失)：
<pre>
# rados rmpool default.rgw.control default.rgw.control --yes-i-really-really-mean-it
# rados rmpool default.rgw.data.root default.rgw.data.root --yes-i-really-really-mean-it
# rados rmpool default.rgw.gc default.rgw.gc --yes-i-really-really-mean-it
# rados rmpool default.rgw.log default.rgw.log --yes-i-really-really-mean-it
# rados rmpool default.rgw.users.uid default.rgw.users.uid --yes-i-really-really-mean-it
</pre>

**5） 创建一个系统用户**

```ceph-radosgw```守护进程来拉取realm以及period信息之前必须进行认证。在master zone中，创建一个系统用户来在不同daemon之间完成认证：
{% highlight string %}
# radosgw-admin user create --uid="{user-name}" --display-name="{Display Name}" --system
{% endhighlight %}
例如：
<pre>
# radosgw-admin user create --uid="synchronization-user" --display-name="Synchronization User" --system
</pre>
这里说明一下，当```secondary zones```需要和master zone完成认证时，需要```access_key```与```secret_key```。

最后，将系统用户添加到master zone中：
<pre>
# radosgw-admin zone modify --rgw-zone=us-east --access-key={access-key} --secret={secret}
# radosgw-admin period update --commit
</pre>

**6) 更新period**

在更新完```master zone```配置之后，需要更新period:
<pre>
# radosgw-admin period update --commit
</pre>

注意： 执行上述更新命令时，会更新epoch，然后确保其他的zone也会收到该配置。

**7) 更新ceph配置文件**

在```master zone```宿主机上更新ceph配置文件： 添加```rgw_zone```选项，该选项的值为master zone的名称.
{% highlight string %}
[client.rgw.{instance-name}]
...
rgw_zone={zone-name}
{% endhighlight %}

例如：
<pre>
[client.rgw.rgw1]
host = rgw1
rgw frontends = "civetweb port=80"
rgw_zone=us-east
</pre>

**8) 启动RGW**

在rgw宿主机上启动并使能RGW服务：
<pre>
# systemctl start ceph-radosgw@rgw.`hostname -s`
# systemctl enable ceph-radosgw@rgw.`hostname -s`
</pre>



### 3.2 配置secondary zones

secondary zone用于从一个master zone中同步数据。当创建一个```secondary zone```时，在一台提供secondary zone服务的宿主机上执行如下操作。
<pre>
若我们要添加一个third zone，操作与添加secondary zone类似，只需要修改zone的名称
</pre>

**1) 拉取realm**

使用master zone group中master zone的```URL路径```、```access key```以及```secret key```来拉取realm到secondary zone对应的宿主机上。如果要拉取一个非默认的realm，请使用```--rgw-realm```或```--realm-id```选项：
<pre>
# radosgw-admin realm pull --url={url-to-master-zone-gateway} --access-key={access-key} --secret={secret}
</pre>

假如该realm是默认realm，或者是唯一的realm，执行如下命令使其成为默认：
<pre>
# radosgw-admin realm default --rgw-realm={realm-name}
</pre>

**2) 拉取period**

使用master zone group中master zone的```URL路径```、```access key```以及```secret key```来拉取period到secondary zone对应的宿主机上。如果是从一个非默认的realm中拉取period，请使用```--rgw-realm```或```--realm-id```选项：
<pre>
# radosgw-admin period pull --url={url-to-master-zone-gateway} --access-key={access-key} --secret={secret}
</pre>

注意：拉取period能够为realm获得最新版本的zone group配置和zone配置


**3) 创建一个secondary zone**
<pre>
注意： 所要创建的zone必须使用处于该zone范围内的RGW来创建
</pre>

我们必须用提供该secondary zone服务的RGW来创建secondary zone。在创建时，需要指定zone group ID，新的zone name，和该zone的一个endpoint。请不要使用```--master```或```--default```选项。在Kraken中，默认情况下所有的zone都是以active-active配置方式运行，即一个RGW客户端可以向任何一个zone写数据，这个zone会向处于同一个group中的其他zone复制数据。假如secondary zone并不能接受写操作的话，请指定```--read-only```选项来创建一个active-passive配置的zone。另外，需要提供master zone中的```access key```以及```secret key```。执行如下的命令：
<pre>
# radosgw-admin zone create --rgw-zonegroup={zone-group-name}\
                            --rgw-zone={zone-name} --endpoints={url} \
                            --access-key={system-key} --secret={secret}\
                            --endpoints=http://{fqdn}:80 \
                            [--read-only]
</pre>
例如：
<pre>
# radosgw-admin zone create --rgw-zonegroup=us --rgw-zone=us-west \
                            --access-key={system-key} --secret={secret} \
                            --endpoints=http://rgw2:80
</pre>


注意：如下的操作步骤需要在一个全新的未存储数据的系统上来配置multi-site。假如在默认的zone上面已经存在了数据，则执行删除操作会导致丢失所有的数据，并且不能再进行恢复。

删除默认的zone:
<pre>
# radosgw-admin zone delete --rgw-zone=default
</pre>

最后，删除ceph集群默认的pool:
<pre>
# rados rmpool default.rgw.control default.rgw.control --yes-i-really-really-mean-it
# rados rmpool default.rgw.data.root default.rgw.data.root --yes-i-really-really-mean-it
# rados rmpool default.rgw.gc default.rgw.gc --yes-i-really-really-mean-it
# rados rmpool default.rgw.log default.rgw.log --yes-i-really-really-mean-it
# rados rmpool default.rgw.users.uid default.rgw.users.uid --yes-i-really-really-mean-it
</pre>

**4) 更新ceph配置文件**

在secondary zone对应的宿主机上修改ceph配置文件，添加```rgw_zone```配置选项:
{% highlight string %}
[client.rgw.{instance-name}]
...
rgw_zone={zone-name}
{% endhighlight %}

例如：
<pre>
[client.rgw.rgw2]
host = rgw2
rgw frontends = "civetweb port=80"
rgw_zone=us-west
</pre>

**5) 更新period**

在更新完master zone配置之后，更新peroid:
<pre>
# radosgw-admin period update --commit
</pre>

注意： 执行上述更新命令时，会更新epoch，然后确保其他的zone也会收到该配置。

**6) 启动RGW**

通过如下命令启动RGW服务：
<pre>
# systemctl start ceph-radosgw@rgw.`hostname -s`
# systemctl enable ceph-radosgw@rgw.`hostname -s`
</pre>

**7) 检查同步状态**

一旦```secondary zone```建立起来并成功运行之后，可以检查相应的同步状态。同步操作会拷贝在master zone中创建的users及buckets到secondary zone中。
{% highlight string %}
# radosgw-admin sync status
{% endhighlight %}
如下是一个输出示例：
<pre>
realm f3239bc5-e1a8-4206-a81d-e1576480804d (earth)
    zonegroup c50dbb7e-d9ce-47cc-a8bb-97d9b399d388 (us)
         zone 4c453b70-4a16-4ce8-8185-1893b05d346e (us-west)
metadata sync syncing
              full sync: 0/64 shards
              metadata is caught up with master
              incremental sync: 64/64 shards
    data sync source: 1ee9da3e-114d-4ae3-a8a4-056e8a17f532 (us-east)
                      syncing
                      full sync: 0/128 shards
                      incremental sync: 128/128 shards
                      data is caught up with source
</pre>

注意： 虽然secondary zone可以接收bucket operations，但其实是通过将该操作转发给master zone来进行处理的，然后再将处理后的结果同步到secondary zone中。而假如master zone不能正常工作的话，在secondary zone中执行的bucket operations将会失败。但是```object operations```是可以成功的。

## 4. 维护

**1) 检查同步状态**
<pre>
$ radosgw-admin sync status
        realm b3bc1c37-9c44-4b89-a03b-04c269bea5da (earth)
    zonegroup f54f9b22-b4b6-4a0e-9211-fa6ac1693f49 (us)
         zone adce11c9-b8ed-4a90-8bc5-3fc029ff0816 (us-2)
        metadata sync syncing
              full sync: 0/64 shards
              incremental sync: 64/64 shards
              metadata is behind on 1 shards
              oldest incremental change not applied: 2017-03-22 10:20:00.0.881361s
    data sync source: 341c2d81-4574-4d08-ab0f-5a2a7b168028 (us-1)
                      syncing
                      full sync: 0/128 shards
                      incremental sync: 128/128 shards
                      data is caught up with source
              source: 3b5d1a3f-3f27-4e4a-8f34-6072d4bb1275 (us-3)
                      syncing
                      full sync: 0/128 shards
                      incremental sync: 128/128 shards
                      data is caught up with source
</pre>

**2) changing the metadata master zone**

一般需要在相应的元数据完全同步完成之后，才能够进行修改。否则可能造成数据丢失：
<pre>
# radosgw-admin zone modify --rgw-zone=us-2 --master
# radosgw-admin zonegroup modify --rgw-zonegroup=us --master
# radosgw-admin period update --commit
</pre>

## 5. 故障切换和灾难恢复

假如master zone当前失败的话，切换到secondary zone以进行容灾恢复：
	
1) 使secondary zone成为master zone及默认的zone:
<pre>
# radosgw-admin zone modify --rgw-zone={zone-name} --master --default
</pre>
默认情况下，RGW会运行于active-active配置模式下。假如当前cluster被配置为active-passive模式，则secondary zone是一个只读zone。此时要移除```--read-only```状态以允许该secondary zone接受写操作：
<pre>
# radosgw-admin zone modify --rgw-zone={zone-name} --master --default \
                            --read-only=False
</pre>

2) 更新period使修改生效
<pre>
# radosgw-admin period update --commit
</pre>

3) 最后，重启RGW
<pre>
# systemctl restart ceph-radosgw@rgw.`hostname -s`
</pre>

假如前述的master zone恢复了之后，再执行如下的反操作：

1) 从恢复的zone拉取当前master zone的period
<pre>
# radosgw-admin period pull --url={url-to-master-zone-gateway} \
                            --access-key={access-key} --secret={secret}
</pre>

2) 是恢复后的zone成为master zone及default zone
<pre>
# radosgw-admin zone modify --rgw-zone={zone-name} --master --default
</pre>

3) 更新period以使修改生效
<pre>
# radosgw-admin period update --commit
</pre>

4) 重启恢复后的zone的rgw
<pre>
# systemctl restart ceph-radosgw@rgw.`hostname -s`
</pre>

5) 假如secondary zone需要被配置为read-only，执行如下更新secondary zone
<pre>
# radosgw-admin zone modify --rgw-zone={zone-name} --read-only
</pre>

6) 更新period以使修改生效
<pre>
# radosgw-admin period update --commit
</pre>

7) 最后，重启secondary zone中的rgw
<pre>
# systemctl restart ceph-radosgw@rgw.`hostname -s`
</pre>


## 6. 将一个single site系统迁移到Multi-site
假如要将一个使用默认```zone group```single site系统迁移到一个multi-site系统，使用如下的步骤：

1). 创建Realm。 请使用realm name替换```<name>```
{% highlight string %}
# radosgw-admin realm create --rgw-realm=<name> --default
{% endhighlight %}

2) 重命名default zone及zonegroup。 请使用```<name>```替换zonegroup name及zone name
{% highlight string %}
# radosgw-admin zonegroup rename --rgw-zonegroup default --zonegroup-new-name=<name>
# radosgw-admin zone rename --rgw-zone default --zone-new-name us-east-1 --rgw-zonegroup=<name>
{% endhighlight %}

3) 配置master zonegroup。请使用```<name>```替换realm name及zonegroup name。
{% highlight string %}
# radosgw-admin zonegroup modify --rgw-realm=<name> --rgw-zonegroup=<name> --endpoints http://<fqdn>:80 --master --default
{% endhighlight %}

4) 配置master zone
{% highlight string %}
# radosgw-admin zone modify --rgw-realm=<name> --rgw-zonegroup=<name> \
                            --rgw-zone=<name> --endpoints http://<fqdn>:80 \
                            --access-key=<access-key> --secret=<secret-key> \
                            --master --default
{% endhighlight %}


5) 创建一个系统用户
{% highlight string %}
# radosgw-admin user create --uid=<user-id> --display-name="<display-name>"\
                            --access-key=<access-key> --secret=<secret-key> --system
{% endhighlight %}

6) 提交更新的配置
{% highlight string %}
# radosgw-admin period update --commit
{% endhighlight %}

7) 最后，重启RGW
{% highlight string %}
# systemctl restart ceph-radosgw@rgw.`hostname -s`
{% endhighlight %}


在完成上面这些操作之后，就可以再配置一个secondary zone了。


<br />
<br />

**[参看]**

1. [MULTI-SITE](http://docs.ceph.com/docs/master/radosgw/multisite/)


<br />
<br />
<br />

