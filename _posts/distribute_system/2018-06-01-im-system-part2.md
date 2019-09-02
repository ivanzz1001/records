---
layout: post
title: IM服务器设计--消息存储(转)
tags:
- 分布式系统
categories: distribute-systems
description: IM服务器设计
---

这部分专门讲述IM消息存储的设计。消息存储的难度在于，要考虑以下场景：

* 离线消息存储。即发送消息时对方不在线该怎么处理

* 单聊、群聊消息

* 随着用户量越来越大，以后应该如何扩展



>说明： 工作多年，自己也亲身参与过一款IM系统相关模块的设计，但是对比本文有一些地方还是略有不足。本文转载自[IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)，主要是为了进一步从更高层次理解IM；另一方面也方便自己的后续查找，防止文章丢失。

<!-- more -->

## 1. 读扩散 VS 写扩散
消息同步模型中，有读扩散和写扩散两种模型。在开始讨论之前需要先了解两个相关的概念：

* 收件箱(inbox): 该用户收到的消息

* 发件箱(outbox): 该用户发出的消息

### 1.1 写扩散(push)
写扩散就是经常说的push模式，即每个消息都直接发送到该用户的收件箱中。其优缺点如下：

* 优点： 读优化，用户每次只需要去读取自己收件箱中的消息即可。

* 缺点： 写很重，如果这个消息是一条群消息，那么一个群成员发出去的消息将拷贝到所有其余群成员的收件箱中

![im-msg-push.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-msg-push.png)


### 1.2 读扩散(pull)

读扩散就是pull模式，用户每次到消息发送者的发件箱去拉取消息，优缺点如下：

* 优点： 写优化，每次发送的消息只需要写到一个地方，由收件者自己去拉取消息即可

* 缺点： 读操作很重，假设一个用户有一千个好友，重新登录时需要拉取这些好友所有的离线消息

![im-msg-pull.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-msg-pull.png)

最终选择的是以pull模式为主的模式，理由在于：

* IM业务属于```写多读少```类型的业务，如果使用push模式，将造成消息的大量冗余

* pull模式读操作较重的缺陷可以通过其他方式来优化解决

下面来看具体的设计。

## 2. 表设计
在数据库设计中，仅使用一个发送消息表来存储消息的具体内容，而另外有一个消息接收表用来存储消息的ID信息而不是具体内容，这样用户查询消息时，大体流程如下：

* 首先拉取接收消息表中的信息

* 根据接收消息表中的ID以及发送者ID信息到发送信息表来具体查询消息

![im-msg.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-msg.png)

### 2.1 用户发送消息表
无论是单聊还是群聊消息，都使用这个表来存储发送出去的消息:
<pre>
im_message_send（msg_id,msg_from,msg_to,msg_seq,msg_content,send_time,msg_type）
</pre>
其中：

* msg_id: 消息ID

* msg_from: 消息发送者UID

* msg_to: 消息接收者。如果是单聊消息那么就是用户UID，如果是群聊消息就是群ID

* msg_seq: 客户端发送消息时带上的序列号，主要用于消息排重以及通知客户端消息发送成功之用

* msg_content: 消息内容

* send_time: 消息发送时间

* msg_type: 消息类型，如单聊、群聊消息等

### 2.2 用户接收消息表
<pre>
im_message_recieve（id, msg_from, msg_to, msg_id, flag）
</pre>
其中：

* id: 这个表的ID，自增

* msg_from: 消息发送者ID

* msg_to: 消息接收者ID

* msg_id: 消息ID，对应发送消息表中的ID

* flag: 标志位，表示该消息是否已读

接收消息表的信息并没有很多，因为主体部分如消息内容、发送消息时间等都在发送消息表中。

![im-msg-table.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-msg-table.png)


## 3. 分库分表及访问策略
发送消息表，根据msg_from字段作为分库分表的依据，而接收消息表则使用msg_to字段作为分库分表的依据。

另外，还需要添加缓存将群聊消息进行缓存，缓存的key为msg_to和msg_id的组合，这样查询具体群聊消息的时候就可以根据组ID查询一条具体的消息了。

以上需要对存储之上的业务完全透明，因此加上一个db proxy来处理消息的读写，除了应付这套流程以外，proxy的引入还有这些好处：

* 无状态，可以横向扩展

* 下面的存储服务扩容的时候，proxy可以感知到变化，而这些对上面的应用层而言都是透明的

有了这一层proxy之后，消息的读写流程如下。

### 3.1 写消息
收到用户发送过来的消息，db proxy做如下处理：

* 根据msg_from查询到哪个存储服务存储该发送消息，写入消息，同时得到写入成功之后的消息ID

* 如果这条消息是一个群聊消息，此时根据群ID以及第一步返回的消息ID，写一条键为群ID以及消息ID组合，而值为消息内容的缓存数据到缓存中

* 根据msg_to查询是哪个存储服务存储该接收消息，写入这部分信息

![im-msg-write.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-msg-write.png)

### 3.2 读消息
读消息的过程反之：

* 根据msg_to查询是哪个存储服务存储该接收消息，查询到该消息之后就知道对应的msg_id

* 根据第一步查询到msg_from以及msg_id来去发送消息表中查询消息，如果是群聊消息的话，可以首先组合这两个字段到缓存中查询，查询不到再查询数据库

* 如果上面第二步中的群聊消息，在缓存中没有查询到，需要一个策略来向缓存中写入一份该群聊消息

* 如果接收到用户已读该消息的应答，那么还需要再次根据msg_to查询该消息将flag字段变成用户已读状态

## 4. 登录之后拉取离线消息的优化
在第一篇```基础篇```中已经给出了拉取离线消息的基本流程，在这里还需要进行一些优化。

在实际的应用中，离线的群聊消息并不是需要每次登录都完整拉取下来的，因此这里可以做一个优化： 登录时针对群聊消息仅拉取每个群的未读消息数量，用于客户端的展示，而实际消息内容的加载，可以等到用户真的点到这个群查看消息或者可以后台加载，总之不影响登录总流程即可。

细化了消息存储部分之后的整体架构如下图所示：

![im-arch-detail.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-arch-detail.png)

## 5. 总结
* 采用pull模式为主的消息发送存储方式

* 为了解决pull模式的读消息叫重的问题，引入了以下组件：

>db proxy来解决整个读写逻辑，这部分对业务层完全透明，同时proxy可以感知下面存储服务的扩缩容变更等
>
>群聊消息根据消息ID以及群ID写入缓存一份，不必每次都到存储服务器上面拉取消息
>
>使用另一个消息计数表来存储未读消息数量，登录之后群聊消息仅展示未读消息数量，这部分群聊消息可以延迟拉取或者后台拉取不影响客户端登录主流程


<br />
<br />

**[参看]:**

1. [IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)

2. [单点登录（SSO），从原理到实现](https://cloud.tencent.com/developer/article/1166255)

3. [单点登录（SSO）的设计与实现](https://ken.io/note/sso-design-implement)

4. [一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)

<br />
<br />
<br />


