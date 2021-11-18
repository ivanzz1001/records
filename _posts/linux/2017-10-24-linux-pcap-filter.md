---
layout: post
title: Linux pcap filter的使用
tags:
- LinuxOps
categories: linux
description: Linux pcap filter的使用
---


本节我们讲述一下包过滤表达式的语法。在tcpdump中就会使用到这里讲述到的pcap filter表达式。



<!-- more -->


## 1. Description

**pcap_compile()**被用于将一个字符串编译进filter程序中。产生的filter程序可以用于过滤哪些数据包可以传递给**pcap_loop()**,**pcap_dispatch()**,**pcap_next()**,和**pcap_next_ex()**.

filter表达式通常由一个或多个原语构成。而原语通常是由带一个或多个带前缀修饰符的id(name or number)所构成。有三种不同类型的修饰符：

* *type*
 
type限定符用于指明当前要引用哪一种类型的id name 或 id number。可选值有： **host**, **net**, **port**, **portrange**。例如: ```host foo```, ```net 128.3```, ```port 20```, ```portrange 6000-6008```。 假如没有指定类型限定符的话，默认值为host。

* *dir*

dir限定符用于指明传输方向。可选的方向有： **src**, **dst**, **src or dst**, **src and dst**, **ra**, **ta**, **addr1**, **addr2**, **addr3**, **addr4**。 例如：```src foo```, ```dst net 128.3```, ```src or dst port ftp-data```。 假若并没有指定方向限定符的话，默认值为**src or dst**。而**ra**, **ta**, **addr1**, **addr2**, **addr3**, 和**addr4**限定符只在IEEE 802.11无线网络链路层有效。

* *proto*

proto限定符用于设置匹配的协议类型。可选值有： **ether**, **fddi**, **tr**, **wlan**, **ip**, **ip6**, **arp**, **rarp**, **decnet**, **tcp**和**udp**。 例如： ```ether src foo```, ```arp net 128.3```, ```tcp port 21```, ```udp portrange 7000-7009```, ```wlan addr2 0:2:3:4:5:6```。 假如没有指定proto限定符，则默认值为所有兼容该**type**的协议类型。 例如：```src foo```表示为```(ip or arp or rarp) src foo```; ```net bar```表示为```(ip or arp or rarp) net bar```; ```port 53```表示为```(tcp or udp) port 53```。



除上述之外，也有一些并不满足上述模式的```primitive```关键词： **gateway**, **broadcast**, **less**, **greater**和算术表达式。我们会在后面进行介绍。

更复杂的过滤表达式可以使用**and**, **or**, **not** 来组合上述primitives。例如：
<pre>
host foo and not port ftp and not port ftp-data
</pre>

### 1.1 pcap primitives

pcap所支持的primitives有(这里只介绍一些比较常用的)：

1) **dst host** host 

假如ipv4/ipv6数据包的目标地址为```host```时，本表达式为true。 ```host```可以为主机名或者ip地址。

2) **src host** host 

假如ipv4/ipv6数据包的原地址为```host```时，本表达式为true

3) **host** host 

假若ipv4/ipv6数据包的源或目标地址为host时，本表达式为true.

>说明：上述所有host表达式都可以加上前缀：ip, arp,rarp,ip6。例如：
>
> ip host 'host'

4) **dst net** net 

5) **src net** net 

6) **net** net 

7) **dst port** port 

8) **src port** port 

9) **port** port 


10) **dst portrange** port1-port2 

11) **src portrange** port1-port2

12) **portrange** port1-port2 

13) **less** length 

假如包的长度小于等于length的话，本表达式值为true.

14) **greater** length 

假如包的长度大于等于length的话，本表达式值为true.

###### 1.1.1 primitive组合
原语之间可以使用如下进行组合：
{% highlight string %}
A parenthesized group of primitives and operators.

Negation (`!' or `not').

Concatenation (`&&' or `and').

Alternation (`||' or `or').
{% endhighlight %}


### 1.3 示例

1) To select all packets arriving at or departing from `sundown'
{% highlight string %}
host sundown 
{% endhighlight %}

2) To select traffic between `helios' and either `hot' or `ace'
{% highlight string %}
host helios and (hot or ace)
{% endhighlight %}

3) To select all IPv4 packets between `ace' and any host except `helios'
{% highlight string %}
ip host ace and not helios
{% endhighlight %}

4) To select all traffic between local hosts and hosts at Berkeley
{% highlight string %}
net ucb-ether
{% endhighlight %}

5) To select all FTP traffic through Internet gateway `snup':
{% highlight string %}
gateway snup and (port ftp or ftp-data)
{% endhighlight %}

6) To select IPv4 traffic neither sourced from nor destined for local hosts (if you gateway to one other net, this stuff should never make it onto your local net).
{% highlight string %}
ip and not net localnet
{% endhighlight %}


7) To select the start and end packets (the SYN and FIN packets) of each TCP conversation that involves a non-local host.
{% highlight string %}
tcp[tcpflags] & (tcp-syn|tcp-fin) != 0 and not src and dst net localnet
{% endhighlight %}

8) To select the TCP packets with flags RST and ACK both set. (i.e. select only the RST and ACK flags in the flags field, and if the result is "RST and ACK both set", match)
{% highlight string %}
tcp[tcpflags] & (tcp-rst|tcp-ack) == (tcp-rst|tcp-ack)
{% endhighlight %}

9) To select all IPv4 HTTP packets to and from port 80, i.e. print only packets that contain data, not, for example, SYN and FIN packets and ACK-only packets. (IPv6 is left as an exercise for the reader.)
{% highlight string %}
tcp port 80 and (((ip[2:2] - ((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)
{% endhighlight %}

10) To select IPv4 packets longer than 576 bytes sent through gateway `snup':
{% highlight string %}
gateway snup and ip[2:2] > 576
{% endhighlight %}

11) To select IPv4 broadcast or multicast packets that were not sent via Ethernet broadcast or multicast:
{% highlight string %}
ether[0] & 1 = 0 and ip[16] >= 224
{% endhighlight %}

12) To select all ICMP packets that are not echo requests/replies (i.e., not ping packets):
{% highlight string %}
icmp[icmptype] != icmp-echo and icmp[icmptype] != icmp-echoreply
icmp6[icmp6type] != icmp6-echo and icmp6[icmp6type] != icmp6-echoreply
{% endhighlight %}


<br />
<br />

**[参看]:**

1. [pcap filter manual](http://www.tcpdump.org/manpages/pcap-filter.7.html)




<br />
<br />
<br />





