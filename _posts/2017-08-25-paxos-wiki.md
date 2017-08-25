---
layout: post
title: paxos算法 - wiki
tags:
- paxos
categories: paxos
description: paxos算法 - wiki
---

如下paxos算法主要是对[paxos维基百科](https://zh.wikipedia.org/zh-cn/Paxos%E7%AE%97%E6%B3%95)的整理。

<!-- more -->


## 1. 实例
我们用实际的例子来更清晰的描述上述过程：

有A1、A2、A3、A4、A5共5位议员，就税率问题进行决议。议员A1决定将税率定为10%，因此它向所有人发出一个草案。这个草案内容是：
{% highlight string %}
现有的税率是什么？ 如果没有决定，则建议将其定为10%。 时间：本届议会第3年3月15日； 提案者：A1
{% endhighlight %}

在最简单的情况下，没有人与其竞争；信息能及时顺利的传达到其他议员处。于是，A2-A5回应：
{% highlight string %}
我已收到你的提案，等待最终批准。
{% endhighlight %}
而A1在收到2份回复后就发布最终决议：
{% highlight string %}
税率已定为10%，新的提案不再讨论本问题.
{% endhighlight %}

这实际上退化为二阶段提交协议。







<br />
<br />
<br />


