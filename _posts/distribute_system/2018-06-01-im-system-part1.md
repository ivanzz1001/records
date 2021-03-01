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

整体架构中，分为如下几个部分：

* 客户端： 支持IO、Android系统

* 接入层： 负责维护与客户端之间的长连接

* 逻辑层： 负责IM系统中各逻辑功能的实现

* 存储层： 存储IM系统相关的数据，主要包括Redis缓存系统（用于保存用户状态及路由数据）、消息数据

![im-arch](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-arch.png)

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

1: 客户端通过统一登录系统验证登录密码等；

2: SSO验证客户端用户名、密码之后，生成登录token并返回给客户端；

3: 客户端使用UID和返回的token向gate发起授权验证请求；

4: gate同步调用logic server的验证接口

5: logic server请求SSO系统验证token合法性:
<pre>
SSO向auth系统返回验证token结果

如果验证成功，auth系统在redis中存储客户端的路由信息，即客户端在哪个gate上登录
</pre>
6: auth系统向gate返回验证登录结果

7: gate向客户端返回授权结果



###### 3.1.2 登出(logout)

![im-logout](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-logout.png)
1. 客户端向gate发出logout请求

2. gate设置客户端UID对应的peer无效，然后应答客户端登出成功

3. gate向logic server发出登出请求

4. 处理该类请求的c2s服务器，清除redis中的客户端路由信息


###### 3.1.3 踢人(kickout)
用户请求授权时，可能在另一个设备（同类型设备，比如一台苹果手机登录时发现一台安卓手机也在登录这个账号）开着软件处于登录状态。这种情况需要系统将那个设备踢下线。

![im-kickout](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-kickout.png)

新的客户端登录流程同上面的登录认证流程，只不过在auth模块完成认证之后，会做如下的操作：

* 根据UID到redis中查询路由数据，如果不存在说明前面没有登录过，那么就像登录流程一样返回即可

* 否则说明前面已经有其他设备登录了，将向前面的gate发送踢人请求，然后保存新的路由信息到redis中

* gate接收到踢人请求，踢掉客户端之后断掉与客户端的连接


### 3.2 客户端上报消息
![im-c2smsg](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-c2smsg.png)

1. 客户端向gate发送c2s消息数据

2. gate应答客户端

3. gate向逻辑服务器发送c2s消息

4. logic server的c2s模块，将消息发送到MQ消息总线中

5. appserver消费MQ消息做处理

### 3.3 应用服务器推送消息(s2c消息)
![im-s2cmsg.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-s2cmsg.png)

1. 业务服务器向逻辑服务器发送s2c消息

2. 逻辑服务器的s2c模块从redis中查询UID的路由数据，知道该用户在哪个gate上面登录

3. 逻辑服务器向gate发送s2c消息

4. 客户端收到之后向gate ack消息

5. gate向逻辑服务器ack s2c消息

### 3.4 单对单聊天(c2c消息)
![im-c2cmsg.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-c2cmsg.png)

1. 客户端向gate发送c2c消息

2. gate向逻辑服务器发送c2c消息

3. 逻辑服务器的c2c模块保存消息到消息存储中，此时会该将消息的未读标志置位表示未读

4. 逻辑服务器应答gate，说明已经保存了该消息，即客户端发送成功

5. gate应答客户端，表示c2c消息发送成功

6. 逻辑服务器的c2c模块，查询redis服务看该c2c消息的目标客户端的路由信息，如果不在线就直接返回

7. 否则说明该消息的目的客户端在线，向所在gate发送c2c消息

8. gate向客户端转发c2c消息

9. 客户端向gate应答收到c2c消息

10. gate向逻辑服务器应答客户端已经收到c2c消息

11. 逻辑服务器的c2c模块，在消息存储中清空该消息的未读标志表示消息已读

注意第7步中，逻辑服务器的c2c模块在向gate转发c2c消息消息之后，需要加上定时器，如果在指定时间没有收到最后客户端的应答，需要重发。尝试几次重发都失败则放弃，等待下次用户登录了拉取离线消息

### 3.5 群聊消息(c2g消息）
![im-c2gmsg.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-c2gmsg.png)

1. 客户端向gate发送c2g消息

2. gate向逻辑服务器发送c2g消息

3. 逻辑服务器的c2g模块将消息保存到SendMsg DB中，这部分消息将根据消息的发送者ID水平扩展

4. c2g模块从cache中查询该群组的用户ID列表，如果查不到会到存放群组信息的DB中查询

5. 遍历获取到的群组ID，保存消息到RecvMsg DB中，这部分消息将根据接受者ID水平扩展

6. 查询redis，知道哪些群组用户当前在线

7. 向当前在线的用户所在gate发送c2g消息

8. gate转发给客户端c2g消息

9. 客户端应答gate c2g消息

10. gate应答逻辑服务器的c2g模块，用户已经收到c2g消息

11. c2g模块修改发送消息库置该消息为已读

### 3.6 登录后拉取离线消息流程
![im-offlinemsg.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-offlinemsg.png)

1. 客户端请求离线消息，其中会带上的字段是： 客户端uid、当前客户端上保存的最大消息id(msgid)、每次最多获取多少离线消息(size)。当msgid为0的时候，由服务器自行查询当前的离线消息返回给客户端；否则服务器只会返回该消息id以后的消息。在这个例子中，假设第一次请求时，msgid为0，即由服务器查询需要给客户端返回哪些离线消息

2. im服务器查询uid为100的用户的前10(因为size=10)的离线消息，具体来说就是去消息接收表中查询uid=100且read flag为false的前10条消息。这里假设第一次查询返回的消息中，最大消息id为100。

3. 向客户端返回最新离线消息，同时带上最大离线消息id 100

4. 客户端收到离线消息之后，由于收到的消息数量等于size，说明可能还有没有读取的离线消息，因此再次向服务器查询，这一次带上的消息id为100，表示请求该id之后的未读消息

5. IM服务器收到这一次拉取离线消息请求之后，由于msgid不为0，因此首先会将uid=100且msgid在100之前的未读消息全部置为已读

6. 获取uid=100且msgid>100的未读消息返回给客户端

如果每次拉取的离线消息都等于拉取离线消息数量，客户端会一直重复拉取离线消息流程，直到拉取完毕。

## 4. 协议设计
### 4.1 协议格式
![im-protocol-format.png](https://ivanzz1001.github.io/records/assets/img/distribute/im/im-protocol-format.png)
协议分为包头和包体两部分，其中包体为固定的大小，包括：

* version(4字节): 协议版本号

* cmd(4字节): 协议类型

* seq(4字节): 序列号

* timestamp(8字节): 消息的时间戳

* body length(4字节): 包体大小

其中，包体部分使用protobuf来定义，以下介绍不同命令的包体格式。

### 4.2 认证(auth) 
{% highlight string %}
message AuthRequest {
  string token = 1; // 从SSO服务器返回的登录token，登录之后保存在客户端
  srting uid = 2;   // 用户ID
}
message AuthResponse {
  int32 status = 1; // 应答状态码，0表示成功，其他表示失败
  string err_msg = 2; // 错误描述信息
}
{% endhighlight %}

### 4.3 登出(logout)
{% highlight string %}
message LogoutRequest {
  string token = 1; // 从SSO服务器返回的登录token，登录之后保存在客户端
  srting uid = 2;   // 用户ID
}
message LogoutResponse {
}
{% endhighlight %}

### 4.4 踢人(kickout)
{% highlight string %}
message KickoutRequest {
  enum Reason {
    OTHER_LOGIN = 1; // 其他设备登录
  }
  int32 reason = 1; // 踢人原因
}
message KickoutResponse {
}
{% endhighlight %}

### 4.5 心跳包
无包体

### 4.6 单对单消息(c2c)
{% highlight string %}
// 发送者发送消息的协议
message C2CSendRequest {
  string from = 1; // 发送者
  string to = 2; // 接收者
  string content = 3; // 消息内容
}
message C2CSendResponse {
  int64 msgid = 1; // 落地的消息ID
}
// 推送给接收者的协议
message C2CPushRequest {
  string from = 1;
  string content = 2;
  int64 msgid = 3;
}
message C2CPushResponse {
  int64 msgid = 1;  // 消息id，服务器收到这个id可以去置位这个消息已读
}
{% endhighlight %}

### 4.7 群聊(c2g)
{% highlight string %}
// 发送者发送群消息协议
message C2GSendRequest {
  string from = 1; // 发送者
  string group = 2; // 群
  string content = 3; // 消息内容
}
message C2GSendResponse {
  int64 msgid = 1; // 落地的消息ID
}
// 推送给其他群成员消息协议
message C2GPushRequest {
  string from = 1; // 发送者
  string group = 2; // 群
  string content = 3; // 消息内容
  int64 msgid = 4; // 落地的消息ID
}
message C2GPushResponse {
  int64 msgid = 1; // 落地的消息ID
}
{% endhighlight %}

### 4.8 拉取离线消息(pull)
{% highlight string %}
message C2SPullMessageRequest {
  string uid = 1;
  int64 msgid = 2;  // 拉取该消息id以后的离线消息，为0由服务器自行判断
  int32 limit = 3; //  单次拉取离线消息的数量
}
message PullMsg {
  string from = 1;  // 发送者
  int64 group = 2;  // 目的群
  string content = 3; // 消息内容
  int64 msgid = 4;  // 消息编号
  int64 send_time = 5;  // 服务器接收消息时间
}
message C2SPullMessageResponse {
  repeated PullMsg msg = 1; // 离线消息数组
}
{% endhighlight %}

<br />
<br />

**[参看]:**

1. [IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)

2. [单点登录（SSO），从原理到实现](https://cloud.tencent.com/developer/article/1166255)

3. [单点登录（SSO）的设计与实现](https://ken.io/note/sso-design-implement)

4. [一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)

5. [解密融云IM产品的聊天消息ID生成策略](http://www.52im.net/thread-2747-1-1.html)

<br />
<br />
<br />


