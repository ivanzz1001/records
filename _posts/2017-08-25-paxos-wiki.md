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
现在我们假设在A1提出提案的同时，A5决定将税率定为20%：
{% highlight string %}
现有的税率是什么？ 如果没有决定，则建议将其定为20%。 时间：本届议会第3年3月15日； 提案者：A5
{% endhighlight %}

草案要通过侍从送到其他议员的案头。 A1的草案将由4位侍从送到A2-A5那里。 现在，负责A2和A3的侍从将草案顺利送达，负责A4和A5的侍从则不上班。A5的草案则顺利的送至A4和A3手中。如下图所示：

![实例](https://ivanzz1001.github.io/records/assets/img/distribute/paxos-wiki-example.png)

现在, A1、A2、A3收到了A1的提案； A3、A4、A5收到了A5的提案。 按照协议， A1、A2将接受A1的提案， A4、A5将接受A5的提案，侍从将拿着
{% highlight string %}
我已收到你的提案，等待最终批准
{% endhighlight %}
的回复回到提案者那里。```而A3的行为将决定批准哪一个```







<br />
<br />
<br />


