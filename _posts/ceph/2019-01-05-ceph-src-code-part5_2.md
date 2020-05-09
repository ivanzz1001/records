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
{% endhighlight %}






<br />
<br />

**[参看]**

1. [Ceph 的物理和逻辑结构](https://www.cnblogs.com/sammyliu/p/4836014.html)

2. [小甲陪你一起看Ceph](https://cloud.tencent.com/developer/article/1428004)



<br />
<br />
<br />

