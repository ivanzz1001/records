---
layout: post
title: 微信PaxosStore：深入浅出Paxos算法协议
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->




<br />
<br />
**参看：**

1. [Paxos从理论到实践 ](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw?)

2. [NWR协议](https://www.cnblogs.com/liyulong1982/p/5999169.html)

3. [PaxosStore解读](https://blog.csdn.net/chdhust/article/details/77750327)

4. [paxos 视频](https://video.tudou.com/v/XMTcwNTQyNDU4MA==.html?spm=a2h0k.8191414.0.0&from=s1.8-1-1.2)

5. [用paxos实现多副本日志系统--basic paxos部分](https://blog.csdn.net/u011750989/article/details/81814700)

6. [用paxos实现多副本日志系统--multi paxos部分](https://blog.csdn.net/u011750989/article/details/81814704)

<br />
<br />
<br />


