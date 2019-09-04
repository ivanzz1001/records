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


## 1. 现在前面
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



<br />
<br />

**[参看]:**

1. [一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)

<br />
<br />
<br />


