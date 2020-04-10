---
layout: post
title: ceph本地对象存储(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

在本地对象中，日志是实现操作一致性的机制。在介绍Filestore之前，首先需要了解Journal的机制。本章首先介绍Journal的对外接口，通过对外提供的功能接口，可以了解日志的基本功能。然后详细介绍FileJournal的实现。


<!-- more -->


## 1. Journal对外接口

通过Ceph中的test_filejournal.cc的一段测试程序，执行如下命令编译：
<pre>
# cd src
# make ceph_test_filejournal
</pre>

可以比较清楚地了解Journal的外部接口，通过这些外部接口可以了解Journal的功能，以及如何使用Journal:
{% highlight string %}
FileJournal j(fsid, finisher, &sync_cond, path, directio, aio, faio);

j.create();
j.make_writeable();

vector<ObjectStore::Transaction> tls;
bufferlist bl;
bl.append("small");
int orig_len = j.prepare_entry(tls, &bl);    //获得日志的原始长度
j.reserve_throttle_and_backoff(bl.length());
j.submit_entry(1, bl, orig_len, new C_SafeCond(&wait_lock, &cond, &done));

wait();
j.close();
{% endhighlight %}
通过上述代码可以了解到，日志的基本使用方法如下所示：

1） 创建一个FileJournal类型的日志对象，其构造函数path为日志文件的路径；

2) 调用日志的create方法创建日志，并调用函数make_writable()设置为可写状态；

3） 在bufferlist中添加一段数据作为测试的日志数据，调用submit_entry提交一条日志；

4) 等待日志提交完成；

5) 关闭日志

## 2. FileJournal
类FileJournal以文件（包括块设备文件看做特殊文件）作为日志，实现了Journal的接口。下面先介绍FileJournal的数据结构和日志的格式，然后介绍日志的三个阶段： 日志的提交(journal submit)、日志的应用(journal apply)、日志的同步(journal sync或者commit、trim)及其具体的实现过程。

### 2.1 FileJournal数据结构
FileJournal数据结构如下：


1） **write_item部分**

{% highlight string %}
//FileJournal用于在文件或块设备层面实现日志功能

class FileJournal :
  public Journal,
  public md_config_obs_t {

public:
  struct write_item {
    uint64_t seq;
    bufferlist bl;
    uint32_t orig_len;
    TrackedOpRef tracked_op;
    write_item(uint64_t s, bufferlist& b, int ol, TrackedOpRef opref) :
      seq(s), orig_len(ol), tracked_op(opref) {
      bl.claim(b, buffer::list::CLAIM_ALLOW_NONSHAREABLE); // potential zero-copy
    }
    write_item() : seq(0), orig_len(0) {}
  };
	

  Mutex writeq_lock;               //writeq相关的锁
  Cond writeq_cond;                //writeq相关的条件变量
  list<write_item> writeq;         //保存write_item的队列
};
{% endhighlight %}

结构体write_item封装了一条提交的日志，seq为提交日志的序号，bl保存了提交的日志数据，也就是提交的事务或事务的集合序列化后的数据。Journal并不关心日志数据的内容，它认为一条日志就是一段写入的数据，只负责把数据正确写入日志盘。

<br />
<br />

**[参看]**




<br />
<br />
<br />

