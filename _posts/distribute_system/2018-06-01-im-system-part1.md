---
layout: post
title: IM服务器设计--基础(转)
tags:
- 分布式系统
categories: distribute-systems
description: IM服务器设计
---

IM作为非常经典的服务器系统，其设计时候的考量具备代表性，所以这一次花几个篇幅讨论其相关设计。

主要内容相当部分参考了[一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)一文，在此之上补充了更好的消息存储设计以及集群设计。


>说明： 工作多年，自己也亲身参与过一款IM系统相关模块的设计，但是对比本文有一些地方还是略有不足。本文转载自[IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)，主要是为了进一步从更高层次理解IM；另一方面也方便自己的后续查找，防止文章丢失。

<!-- more -->

## 1. 整体架构

![im-arch](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-arch.png)

以上架构图中，分为几个部分：

* 客户端： 支持IO、Android系统

* 接入层： 负责维护与客户端之间的长连接

* 逻辑层： 负责IM系统中各逻辑功能的实现

* 存储层： 存储IM系统相关的数据，主要包括Redis缓存系统（用于保存用户状态及路由数据）、消息数据

上图中几部分的交互如下：

* 客户端通过gate接入IM服务器。在这里，客户端与Gate之间保持TCP长连接，客户端使用DNS查询域名返回最近的gate地址进行连接

* Gate的作用： 保持与客户端之间的长连接，将请求数据转发给后面的逻辑服务LogicServer。LogicServer最上面是一个消息路由服务Router，根据请求的类型转发到后面具体的逻辑服务器。其中**c**代表客户端，**s**代表服务器，**g**代表群组，因此比如```c2c```服务就是处理客户端之间消息的服务器，而auth服务是处理客户端登录请求的服务器。

* 逻辑类服务器与存储层服务打交道，其中： redis用于存储用户在线状态、用户路由数据（用户路由数据就是指用户在哪个gate服务器上维护长连接），而DB用于存储用户的消息数据，这部分留待下一部分讲解。

* 以上的接入层、逻辑层由于本身不存储状态，因此都可以进行横向扩展。看似Gate维护着长连接，但是即使一个Gate宕机，客户端检测到之后可以重新发起请求接入到另一台Gate服务器。

## 2. 数据存储

* 路由数据： 存放在Redis中，格式为(UID,客户端在哪个gate登录)

* 消息数据： 存储在DB中，部分也会缓存在缓存中方便查询，这部分作为下一部分文章的重点来讲解，不在这部分展开讨论。

## 3. 核心交互流程

### 3.1 登录系统
###### 3.1.1 登录授权(auth)

![im-login](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-login.png)

1. 客户端通过统一登录系统验证登录密码等；

2. SSO验证客户端用户名、密码之后，生成登录token并返回给客户端；

3. 客户端使用UID和返回的token向gate发起授权验证请求；

4. gate同步调用logic server的验证接口

5. logic server请求SSO系统验证token合法性:

* SSO向auth系统返回验证token结果

* 如果验证成功，auth系统在redis中存储客户端的路由信息，即客户端在哪个gate上登录

6. auth系统向gate返回验证登录结果

7. gate向客户端返回授权结果



###### 3.1.2 登出(logout)

###### 3.1.3 踢人(kickout)





<br />
<br />

**[参看]:**

1. [IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)

<br />
<br />
<br />

