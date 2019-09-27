---
layout: post
title: 一套海量在线用户的移动端IM架构设计实践分享(转)
tags:
- 分布式系统
categories: distribute-systems
description: IM服务器设计
---

本文主要介绍一下海量IM架构的设计，文章转载自[一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)，主要是为了进一步从更高层次理解IM；另一方面也方便自己的后续查找，防止文章丢失。

<!-- more -->


## 1. 写在前面
### 1.1 引言
如果在没有太多经验可借鉴的情况下，要设计一套完整可用的移动端IM架构，难度是相当大的。原因在于，IM系统（尤其是移动端IM系统）是多种技术和领域知识的横向应用综合体：网络编程、通信安全、高并发编程、移动端开发等，如果要包含实时音视频聊天的话，还要加上难度更大的音视频编解码技术（内行都知道，把音视频编解码及相关技术玩透的，博士学位都可以混出来了），凡此种种，加上移动网络的特殊性、复杂性，设计和开发难度不言而喻。

本文分享了一套完整的海量在线用户的移动端IM架构设计，来自于作者的真实项目实践总结，包含了详细的算法原理图、数据结构定义、表结构定义等等。

>**即时通讯网注：**本文中的架构设计从实际应用的角度看，其实并不完美，多处设计对于高吞吐高并发的IM应用来说也是存在单点性能瓶颈的（比如：提供消息交换逻辑的msg_logic服务、提供全局用户状态查询的单点Redis等），另外IM协议设计可能也稍显混乱（但这是仁者见仁智者见智的事了，不能一概而论）。但文章中的大部分算法原理、协议设计等都是值得借鉴的，总之没必要照搬，但至少能给你自己的方案设计带来灵感，我想这也是本文或即时通讯网的其他类似文章的真正价值所在。

另外，如果您正打算从零开发移动端IM，则建议您从[《新手入门一篇就够：从零开发移动端IM》](http://www.52im.net/thread-464-1-1.html)一文开始，此文按照IM开发所需的知识和技能要求，拟定了详尽的学习提纲和建议等。

### 1.2 参考资料

1) **IM架构方面的文章**

[《浅谈IM系统的架构设计》](http://www.52im.net/thread-307-1-1.html)

[《简述移动端IM开发的那些坑：架构设计、通信协议和客户端》](http://www.52im.net/thread-289-1-1.html)

[《一套海量在线用户的移动端IM架构设计实践分享(含详细图文)》](http://www.52im.net/thread-812-1-1.html)

[《一套原创分布式即时通讯(IM)系统理论架构方案》](http://www.52im.net/thread-151-1-1.html)

[《从零到卓越：京东客服即时通讯系统的技术架构演进历程》](http://www.52im.net/thread-152-1-1.html)

[《蘑菇街即时通讯/IM服务器开发之架构选择》](http://www.52im.net/thread-31-1-1.html)

[《腾讯QQ1.4亿在线用户的技术挑战和架构演进之路PPT》](http://www.52im.net/thread-158-1-1.html)

[《微信后台基于时间序的海量数据冷热分级架构设计实践》](http://www.52im.net/thread-895-1-1.html)

[《微信技术总监谈架构：微信之道——大道至简(演讲全文)》](http://www.52im.net/thread-200-1-1.html)

[《如何解读《微信技术总监谈架构：微信之道——大道至简》》](http://www.52im.net/thread-201-1-1.html)

[《快速裂变：见证微信强大后台架构从0到1的演进历程（一）》](http://www.52im.net/thread-168-1-1.html)

[《17年的实践：腾讯海量产品的技术方法论》](http://www.52im.net/thread-159-1-1.html)

[《移动端IM中大规模群消息的推送如何保证效率、实时性？》](http://www.52im.net/thread-1221-1-1.html)

[《现代IM系统中聊天消息的同步和存储方案探讨》](http://www.52im.net/thread-1230-1-1.html)

[《IM开发基础知识补课(二)：如何设计大量图片文件的服务端存储架构？》](http://www.52im.net/thread-1356-1-1.html)

[《IM开发基础知识补课(三)：快速理解服务端数据库读写分离原理及实践建议》](http://www.52im.net/thread-1366-1-1.html)

[《IM开发基础知识补课(四)：正确理解HTTP短连接中的Cookie、Session和Token》](http://www.52im.net/thread-1525-1-1.html)

[《WhatsApp技术实践分享：32人工程团队创造的技术神话》](http://www.52im.net/thread-1542-1-1.html)

[《微信朋友圈千亿访问量背后的技术挑战和实践总结》](http://www.52im.net/thread-1569-1-1.html)

[《王者荣耀》2亿用户量的背后：产品定位、技术架构、网络方案等》](http://www.52im.net/thread-1595-1-1.html)

>> [更多同类文章 ……](http://www.52im.net/forum.php?mod=collection&action=view&ctid=7)

2) **IM热点问题总结文章**

[《移动端IM开发者必读(一)：通俗易懂，理解移动网络的“弱”和“慢”》](http://www.52im.net/thread-1587-1-1.html)

[《移动端IM开发者必读(二)：史上最全移动弱网络优化方法总结》](http://www.52im.net/thread-1588-1-1.html)

[《从客户端的角度来谈谈移动端IM的消息可靠性和送达机制》](http://www.52im.net/thread-1470-1-1.html)

[《现代移动端网络短连接的优化手段总结：请求速度、弱网适应、安全保障》](http://www.52im.net/thread-1413-1-1.html)

[《腾讯技术分享：社交网络图片的带宽压缩技术演进之路》](http://www.52im.net/thread-1391-1-1.html)

[《IM开发基础知识补课：正确理解前置HTTP SSO单点登陆接口的原理》](http://www.52im.net/thread-1351-1-1.html)

[《移动端IM中大规模群消息的推送如何保证效率、实时性？》](http://www.52im.net/thread-1221-1-1.html)

[《移动端IM开发需要面对的技术问题》](http://www.52im.net/thread-133-1-1.html)

[《开发IM是自己设计协议用字节流好还是字符流好？》](http://www.52im.net/thread-150-1-1.html)

[《请问有人知道语音留言聊天的主流实现方式吗？》](http://www.52im.net/thread-175-1-1.html)

[《IM消息送达保证机制实现(一)：保证在线实时消息的可靠投递》](http://www.52im.net/thread-294-1-1.html)

[《IM消息送达保证机制实现(二)：保证离线消息的可靠投递》](http://www.52im.net/thread-594-1-1.html)

[《如何保证IM实时消息的“时序性”与“一致性”？》](http://www.52im.net/thread-714-1-1.html)

[《一个低成本确保IM消息时序的方法探讨》](http://www.52im.net/thread-866-1-1.html)

[《IM单聊和群聊中的在线状态同步应该用“推”还是“拉”？》](http://www.52im.net/thread-715-1-1.html)

[《IM群聊消息如此复杂，如何保证不丢不重？》](http://www.52im.net/thread-753-1-1.html)

[《谈谈移动端 IM 开发中登录请求的优化》](http://www.52im.net/thread-282-1-1.html)

[《移动端IM登录时拉取数据如何作到省流量？》](http://www.52im.net/thread-787-1-1.html)

[《浅谈移动端IM的多点登陆和消息漫游原理》](http://www.52im.net/thread-867-1-1.html)

[《完全自已开发的IM该如何设计“失败重试”机制？》](http://www.52im.net/thread-280-1-1.html)

[《通俗易懂：基于集群的移动端IM接入层负载均衡方案分享》](http://www.52im.net/thread-802-1-1.html)

[《微信对网络影响的技术试验及分析（论文全文）》](http://www.52im.net/thread-195-1-1.html)

[《即时通讯系统的原理、技术和应用（技术论文）》](http://www.52im.net/thread-218-1-1.html)

[《开源IM工程“蘑菇街TeamTalk”的现状：一场有始无终的开源秀》](http://www.52im.net/thread-447-1-1.html)

[《QQ音乐团队分享：Android中的图片压缩技术详解（上篇）》](http://www.52im.net/thread-1208-1-1.html)

[《QQ音乐团队分享：Android中的图片压缩技术详解（下篇）》](http://www.52im.net/thread-1212-1-1.html)

[《腾讯原创分享(一)：如何大幅提升移动网络下手机QQ的图片传输速度和成功率》](http://www.52im.net/thread-675-1-1.html)

[《腾讯原创分享(二)：如何大幅压缩移动网络下APP的流量消耗（上篇）》](http://www.52im.net/thread-696-1-1.html)

[《腾讯原创分享(三)：如何大幅压缩移动网络下APP的流量消耗（下篇）》](http://www.52im.net/thread-697-1-1.html)

[《如约而至：微信自用的移动端IM网络层跨平台组件库Mars已正式开源》](http://www.52im.net/thread-684-1-1.html)

[《基于社交网络的Yelp是如何实现海量用户图片的无损压缩的？》](http://www.52im.net/thread-1191-1-1.html)

[《腾讯技术分享：腾讯是如何大幅降低带宽和网络流量的(图片压缩篇)》](http://www.52im.net/thread-1559-1-1.html)

[《腾讯技术分享：腾讯是如何大幅降低带宽和网络流量的(音视频技术篇)》](http://www.52im.net/thread-1560-1-1.html)

[《为什么说即时通讯社交APP创业就是一个坑？》](http://www.52im.net/thread-1619-1-1.html)

>> [更多同类文章 …… ](http://www.52im.net/forum.php?mod=collection&action=view&ctid=10)

## 2. 服务器端设计

### 2.1 总体架构设计
总体架构包括5个层级，具体内容如下图：

![52im-arch](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im_arch.jpg)

各层级说明如下：

* 用户端： 重点是移动端，支持IOS/Android系统，包括IM App，嵌入消息功能的瓜子App，未来还可能接入客服系统

* 移动端API： 针对TCP协议，提供IOS/Android开发SDK。对于H5页面，提供WebSocket接口

* 接入层： 接入层主要任务是保持海量用户连接（接入）、攻击防护、将海量连接整流成少量TCP连接与逻辑层通信

* 逻辑层： 逻辑层负责IM系统各项功能的核心逻辑实现。包括单聊(c2c)、上报(c2s)、推送(s2c)、群聊(c2g)、离线消息、登录授权、组织机构树等等内容；

* 存储层：存储层负责缓存或存储IM系统相关数据，主要包括用户状态及路由（缓存），消息数据（MySQL也可以采用NoSQL，如MangoDB)、文件数据（文件服务器)

### 2.2 典型算法逻辑
典型算法逻辑部分描述IM系统核心组件及其协作关系，结构图如下：

![52im-logic](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im_logic.jpg)
客户端从Iplist服务获取接入层IP地址（也可以采用域名的方式解析得到接入层IP地址)，建立与接入层的连接（可能为短连接），从而实现客户端与IM服务器的数据交互；业务线服务器可以通过服务器端API建立与IM服务器的联系，向客户端推送消息；客户端上报到业务服务器的消息，IM服务器会通过mq投递给业务服务器。

以下将对各子业务的工作原理进行逐一介绍。

###### 登录授权(auth)流程原理
![52im-auth](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-auth.jpg)

1) 客户端通过统一登陆系统实现登陆，得到token

2） 客户端用uid和token向msg-gate发起授权验证请求

3）msg-gate同步调用msg-logic的验证接口

4) msg-logic请求sso系统验证token的合法性 

5) msg-gate得到登陆结果后，设置session状态，并向客户端返回授权结果

###### 登出(logout)流程原理
![52im-logout](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-logout.jpg)

1) 客户端发起logout请求，msg-gate设置对应peer为未登录状态

2) msg-gate给客户端一个ack响应

3） msg-gate通知msg-logic用户登出

###### 踢人(kickout)流程原理
用户请求授权时，可能在另一个设备（同类型设备）开着软件处于登陆状态，这种情况需要系统将那个设备踢下线，如下图：

![52im-kickout](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-kickout.jpg)

1) 1~5步，参看Auth流程

2) logic检索Redis，查看是否该用户在其他地方登陆

3） 如果在其他地方登陆，发起kickout命令。（如果没有登陆，整个流程结束）

4) Gate向用户发起kickout请求，并在短时间内（确保客户端收到kickout数据）关闭socket连接

###### 上报(c2s)流程原理

![52im-c2s](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-c2s.jpg)

1) 客户端向gate发送数据

2） gate回一个ack包，向客户端确认已经收到数据；

3） gate将数据包传递给logic

4） Logic根据数据投递目的地，选择对应的mq队列进行投递

5） 业务服务器得到数据

###### 推送(s2c)流程原理

![52im-s2c](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-s2c.jpg)
1) 业务线调用push数据接口sendMsg

2) Logic向redis检索目标用户状态。如果目标用户不在线，丢弃数据（未来可根据业务场景定制化逻辑）；如果用户在线，查询到用户连接的接入层gate

3) Logic向用户所在gate发送数据

4) Gate向用户推送数据。（如果用户不在线，通知logic用户不在线）

5） 客户端收到数据后向gate发送ack反馈

6） gate将ack信息传递给logic层，用于其他可能的逻辑处理（如日志、确认送达等）

###### 单对单聊天(c2c)流程原理
![52im-c2c](https://ivanzz1001.github.io/records/assets/img/distribute/im/52im-c2c.jpg)

1) App1向gate1发送信息（信息最终要发给App2）

2） Gate1将信息投递给logic

3） Logic收到信息后，将信息进行存储

4） 存储成功后，logic向gate1发送ack

5） Gate1将ack信息发送给App1

6） Logic检索Redis，查找APP2在线状态。如果App2未登录，流程结束

7） 如果App2登录到了gate2，logic将消息发往gate2

8) Gate2将消息发给App2（如果发现App2不在线，丢弃消息即可，这种概率极低，后续离线消息可保证消息不丢）

9） App2向Gate2发送ack

10） Gate2将ack信息发送给logic

11) Logic将消息状态设置为已送达


注： 在第6步和第7步之间，启动计时器(DelayedQueue或哈希环，时间如5秒)，计时时间到后，探测该条消息状态，如果消息未送达，考虑通过APNS、米推、个推进行推送。


###### 群聊(c2g)流程原理

这里我们采用```扩散写```(而非扩散读）的方式。

群聊是多人社交的基本诉求，一个群友在群内发了一条消息：
<pre>
1) 在线的群友能第一时间收到消息

2） 离线的群友能在登录后收到消息
</pre> 

由于```“消息风暴扩散系数”```的存在，群消息的复杂度要远高于单对单消息。如下是群聊里涉及到的一些数据库表设计：

* 群基础表： 用来描述一个群的基本信息
{% highlight string %}
im_group_msgs(group_id, group_name,create_user, owner, announcement, create_time)
{% endhighlight %}

* 群成员表: 用来描述一个群中的成员信息
{% highlight string %}
im_group_users(group_id, user_id)
{% endhighlight %}

* 用户接收消息表： 用来描述一个用户的所有收到的群消息（与单对单消息表是同一个表）
{% highlight string %}
im_message_recieve（msg_id,msg_from,msg_to, group_id，msg_seq, msg_content, send_time, msg_type, deliverd, cmd_id）
{% endhighlight %}

* 用户发送消息表： 用来描述一个用户发送了哪些消息
{% highlight string %}
im_message_send (msg_id,msg_from,msg_to, group_id，msg_seq, msg_content, send_time, msg_type, cmd_id)
{% endhighlight %}

**业务场景举例：**

* 1) 一个群中有X, A, B, C, D共5个成员，成员X发了一条消息；

* 2) 成员A和B在线，期望实时收到消息；

* 3) 成员C与D离线，期望未来拉取到离线消息


**群聊流程如下图所示：**

![52im-c2g](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-c2g.jpg)

**群聊流程详细说明：**

* 1) X向gate发送信息（信息最终要发给这个群，A、B在线）

* 2) Gate将消息发给logic

* 3) 存储消息到im_message_send表，按照msg_from水平分库

* 4) 回ack

* 5) 回ack

* 6) Logic检索数据库（需要使用缓存），获得群成员列表

* 7) 存储每个用户的消息数据（用户视图），按照msg_to水平分库（并发、批量写入）

* 8) 查询用户在线状态及位置

* 9) Logic向Gate投递消息

* 10) Gate向用户投递消息

* 11) APP返回收到消息的ack信息

* 12) Gate向logic传递ack信息

* 13) 向缓存(Hash)中更新收到ack的消息ID。然后再通过一个定时任务，每隔一定时间，将数据更新到数据库。

###### 拉取离线消息流程原理

下图中，将Gate和Logic合并为im-server，拉取离线消息流程如下：

![52im-pull](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-pull.jpg)

* 1) App端登录成功后（或业务触发拉取离线消息），向IM系统发起拉离线消息的请求。传递3个主要参数：```uid```表明用户；msgid表明当前收到的最大消息id(如果没有收到过消息，或拿不到最大消息id则msgid=0即可)；size表示每次拉取条数（这个值也可以由服务器端控制）。

* 2) 假设msgid==0，什么都不做。（参看第6步骤）

* 3) im-server查询用户前10条离线消息

* 4) 将离线消息推给用户。假设这10条离线消息最大msgid=110

* 5) App得到数据，判断得到的数据不为空（表明可能没有拉取完离线数据，不用<10条做判断拉完条件，因为服务器端需要下次拉离线的请求来确定这次数据已经送达)，继续发起拉取操作。msgid=110(取得离线消息中最大的msgid)

* 6) im-server删除该用户msgid<=110的离线消息（或者标记为已送达）

* 7) 查询msgid>110的前10条离线数据

* 8) 返回给App

* ...

* N-1) 查询msgid>140的离线数据，0条（没有离线数据了）

* N) 将数据返回App，App判断拉取到0条数据，结束离线拉取过程

### 2.3 后台PUSH（推送）

IOS采用APNS、Android真后台保活，同时增加米推、个推。基本思路：push提示信息，App通过拉离线获得真实消息

## 3. 协议设计
### 3.1 IM协议总体定义

TCP的数据协议如下图所示，包括header和body两部分：

![52im-protocol](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-protocol.jpg)

消息头总共20个字节，具体信息如下表：

![52im-detail](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-detaill.jpg)


### 3.2 各具体的IM协议体定义

消息体协议采用ProtocolBuffer(谷歌)协议(详见文章[《Protobuf通信协议详解：代码演示、详细原理介绍等》](http://www.52im.net/thread-323-1-1.html))，版本3.0.0，该协议在序列化效率、压缩、可扩展方面都具有优势。以下为主要流程涉及的协议。

###### 认证(auth)

![52im-auth-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-auth-msg.jpg)

###### 登出(logout)

![52im-logout-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-logout-msg.jpg)

###### 踢人(kickout)

![52im-kickout-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-kickout-msg.jpg)

###### 心跳(keepalive, noop)

心跳包消息体为空。

###### 单对单聊天(c2c)

![52im-c2c-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-c2c-msg.jpg)


###### 群聊(c2g)

![52im-c2g-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-c2g-msg.jpg)

###### 拉离线(pull)

![52im-pull-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-pull-msg.jpg)


###### 控制类(ctrl)协议

![52im-ctrl-msg](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-ctrl-msg.jpg)

## 4. 存储设计

### 4.1 MySQL数据库

MySQL数据库采用utf8mb4编码格式（emoji字符问题）

### 4.2 主要表结构

**发送消息表：**

保存某个用户发送了哪些消息，用于复现用户聊天场景（消息漫游功能需要)

![52im-send-table](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-send-table.jpg)


**推送消息表：**

保存某个用户收到了哪些消息。

![52im-recv-table](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-recv-table.jpg)

**群基本信息表：**

![52im-group-basic](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-group-basic.jpg)


**群用户关系表：**

![52im-group-relationship](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-group-relationship.jpg)

### 4.3 水平分库

![52im-table-split](https://ivanzz1001.github.io/records/assets/img/distribute/im/51im-table-split.jpg)


### 4.4 Redis缓存

**用户状态及路由信息：**

Redis缓存以uid为key，检索channel(socketid)、last_packet_time等。

Gate层， Session以channel(socketid)为key，检索uid及其他信息。

交互接口： gate->logic，通过将channel转换为uid作为key； logic->gate，将uid转换为channel作为key。

**其他缓存信息：**

你觉得该怎么存就怎么存。

### 4.5 文件及图片存储

采用商用云存储。

### 4.6 数据归档

可考虑采用HBase、HDFS作为数据归档，或者相关云存储服务。

>注： 安全部分略，其他非核心功能略。




<br />
<br />

**[参看]:**

1. [一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)

<br />
<br />
<br />


