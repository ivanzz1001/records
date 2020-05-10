---
layout: post
title: ceph客户端
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章我们介绍Ceph的客户端实现。客户端是系统对外提供的功能接口，上层应用通过它来访问ceph存储系统。本章首先介绍librados和Osdc两个模块，通过它们可以直接访问RADOS对象存储系统。其次介绍Cls扩展模块，使用它们可方便地扩展现有的接口。最后介绍librbd模块。由于librados和librbd的多数实现流程都比较类似，本章在介绍相关数据结构后，只选取一些典型的操作流程介绍。

<!-- more -->

## 1. librados
librados是RADOS对象存储系统访问的接口库，它提供了pool的创建、删除，对象的创建、删除、读写等基本操作接口。架构如下图5-5所示：

![ceph-chapter5-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter5_5.jpg)


最上层是类RadosClient，它是Librados的核心管理类，处理整个RADOS系统层面以及pool层面的管理。类IoctxImpl实现单个pool层对象的读写等操作。OSDC模块实现了请求的封装和通过网络模块发送请求的逻辑，其核心类Objecter完成对象的地址计算、消息的发送等工作。



### 1.1 RadosClient
代码如下(src/librados/radosclient.h)：
{% highlight string %}
class librados::RadosClient : public Dispatcher
{
public:
  using Dispatcher::cct;
  md_config_t *conf;                                     //配置文件
private:
  enum {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
  } state;                                              //和monitor的网络连接状态

  MonClient monclient;                                  //Monitor客户端
  Messenger *messenger;                                 //网络消息接口

  uint64_t instance_id;                                 //rados客户端实例的ID
  
  Objecter *objecter;                                   //objecter对象指针

  Mutex lock;
  Cond cond;
  SafeTimer timer;                                     //定时器
  int refcnt;                                          //引用计数

  version_t log_last_version;
  rados_log_callback_t log_cb;
  void *log_cb_arg;
  string log_watch;
  
public:
  Finisher finisher;                                  //用于执行回调函数的Finisher类
  
  ...
};
{% endhighlight %}

通过RadosClient的成员函数，可以了解RadosClient的实现功能如下：

1） 网络连接

connect()函数是RadosClient的初始化函数，完成了许多的初始化工作：

&emsp;a) 调用函数monclient.build_initial_monmap()，从配置文件里检查是否有初始的Monitor地址信息；

&emsp;b) 创建网络通信模块messenger，并设置相关的Policy信息；

&emsp;c) 创建Objecter对象并初始化；
{% highlight string %}
objecter = new (std::nothrow) Objecter(cct, messenger, &monclient,
		  &finisher,
		  cct->_conf->rados_mon_op_timeout,
		  cct->_conf->rados_osd_op_timeout);
if (!objecter)
	goto out;
objecter->set_balanced_budget();

objecter->init();
{% endhighlight %}

&emsp;d) 调用monclient.init()函数初始化monclient
{% highlight string %}
err = monclient.init();
{% endhighlight %}

&emsp;e) Timer定时器初始化，Finisher对象初始化


2) pool的同步和异步创建
{% highlight string %}
int librados::RadosClient::pool_create(string& name, unsigned long long auid,
				       int16_t crush_rule);

int librados::RadosClient::pool_create_async(string& name, PoolAsyncCompletionImpl *c,
					     unsigned long long auid,
					     int16_t crush_rule);
{% endhighlight %}

&emsp;a) 函数pool_create同步创建pool。其实现过程为调用Objecter::create_pool()函数，构造PoolOp操作，通过monitor的客户端monc发送请求给Monitor创建一个pool，并同步等待请求的返回；

&emsp;b) 函数pool_create_async异步创建。与同步方式的区别在于注册了回调函数，当创建成功后，执行回调函数通知完成。

3） pool的同步和异步删除

函数delete_pool完成同步删除，函数delete_pool_async异步删除。其过程和pool的创建过程相同，向Monitor发送删除请求。

4） 查找pool和列举pool

函数lookup_pool()用于查找pool，函数pool_list用于列出所有的pool。pool相关的信息都保存在OsdMap中。

5) 获取pool和系统的信息

函数get_pool_stats()用于获取pool的统计信息，函数get_fs_stats()用于获取系统的统计信息。
{% highlight string %}
int librados::RadosClient::get_pool_stats(std::list<string>& pools,
					  map<string,::pool_stat_t>& result);

int librados::RadosClient::get_fs_stats(ceph_statfs& stats);
{% endhighlight %}



6) 命令处理

函数mon_command()处理Monitor相关的命令，它调用函数monclient.start_mon_command()把命令发送给monitor处理；函数osd_command处理OSD相关的命令，它调用函数objecter->osd_command()把命令发送给对应OSD处理。函数pg_command()处理PG相关命令，它调用函数objecter->pg_command()把命令发送给该PG的主OSD来处理。

7） 创建IoCtxImpl对象

函数create_ioctx()创建一个pool相关的山下文信息IoCtxImpl对象。


### 1.2 IoCtxImpl
类IoCtxImpl(src/librados/IoCtxImpl.h)是pool操作相关的上下文信息，一个IoCtxImpl对象对应着一个pool（注： 对于一个pool，我们可以创建多个IoCtxImpl对象)，可以在该pool里创建、删除对象，完成对象的数据读写等各种操作，包括同步和异步的实现。其处理过程都比较简单，而且过程类似：

1） 把请求封装成ObjectOperation类（该类定义在src/osdc/Objecter.h中）

2) 然后再添加pool的地址信息，封装成Objecter::Op对象

3） 调用函数objecter->op_submit发送给相应的OSD。如果是同步操作，就等待操作完成；如果是异步操作，就不用等待，直接返回。当操作完成后，调用相应的回调函数通知。


<br />
<br />

**[参看]**

1. [Ceph 的物理和逻辑结构](https://www.cnblogs.com/sammyliu/p/4836014.html)

2. [小甲陪你一起看Ceph](https://cloud.tencent.com/developer/article/1428004)



<br />
<br />
<br />

