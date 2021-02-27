---
layout: post
title: time_wait的快速回收和重用（转）
tags:
- tcpip
categories: tcpip
description: time_wait的快速回收和重用
---


本章我们介绍一下time_wait的产生原因，以及如何处理大量连接处于```time_wait```这一状态的情况。


<!-- more -->

## 1. time_wait产生的原因及作用

下面我们先来简单回顾一下TCP连接关闭动作：

![tcp-close](https://ivanzz1001.github.io/records/assets/img/tcpip/tcp-close.png)

在Linux环境下我们可以如下的方式来统计TCP连接的情况：
<pre>
# netstat -nat | awk '/^tcp/ {++S[$NF]} END{for(a in S) print S[a], "\t", a}'
1        LAST_ACK
57       LISTEN
113953   ESTABLISHED
5        FIN_WAIT1
16       FIN_WAIT2
2        SYN_SENT
559      TIME_WAIT
</pre>
上面我们看到，大部分的连接处于```ESTABLISHED```状态，目前比较少的处于```TIME_WAIT```状态，当前机器看起来还算是正常。

1） **time_wait状态如何产生？**

通过上面的变迁图，首先调用```close()```发起主动关闭的一方，在发送最后一个ACK之后会进入time_wait的状态，也就是说该发送方会保持```2MSL```时间之后才会回到初始状态。```MSL```指的是数据包在网络中的最大生存时间。在这样一个```2MSL```长的等待时间内，定义这个连接的四元组(客户端IP/Port，服务端IP/Port)不能被使用。


2）**time_wait状态产生的原因**

* 为实现TCP全双工连接的可靠释放

由TCP状态变迁图可知，假设发起主动关闭的一方(client)最后发送的ACK在网络中丢失，由于TCP协议的重传机制，执行被动关闭的一方(server)将会重发其```FIN```，在该FIN到达client之前， client必须维护这条连接状态，也就是说这条TCP连接所对应的资源(client方的local_ip/local_port)不能被立即释放或重新分配，直到另一方重发的FIN达到之后，client重发ACK后，经过2MSL时间周期没有再收到另一方的```FIN```之后，该TCP连接才能恢复初始的**CLOSED**状态。如果主动关闭的一方不维护这样一个```TIME_WAIT```状态，那么当被动关闭一方重发的```FIN```到达时，主动关闭一方的TCP传输层会用**RST**包响应对方，这会被对方认为是有错误发生，然而这事实上这只是正常的关闭连接过程，并非异常。
<pre>
确保被动关闭方收到ACK，连接正常关闭，且不因被动关闭方重传 FIN 影响下一个新连接。
</pre>

* 为使旧的数据包在网络因过期而消失

为说明这个问题，我们先假设TCP协议中不存在```TIME_WAIT```状态的限制，再假设当前有一条TCP连接(local_ip/local_port, remote_ip/remote_port)，因某些原因，我们先关闭，接着很快以相同的四元组建立一条新连接。本文前面介绍过，TCP连接由四元组唯一标识，因此，在我们假设的情况中，TCP协议栈是无法区分前后两条TCP连接的不同的，在它看来，这根本就是同一条连接，中间先释放再建立连接的过程对其来说是```感知```不到的。这样就可能发生这样的情况：前一条TCP连接由local peer发送的数据到达remote peer后，会被该remote peer的TCP传输层当做当前TCP连接的正常数据接收并向上传递至应用层（而事实上，在我们假设的场景下，这些旧数据到达remote peer前，旧连接已断开并且一条由相同四元组构成的新TCP连接已建立，因此，这些旧数据是不应该被向上传递至应用层的），从而引起数据错乱而导致各种无法预知的诡异现象。作为一种可靠的传输协议，TCP必须在协议层面考虑并避免这种情况的发生，这正是```TIME_WAIT```状态存在的第2个原因。

<pre>
2MSL：报文最大生存时间，确保旧的数据不会影响新连接
</pre>

**总结：**

具体而言，local peer主动调用close后，此时的TCP连接进入```TIME_WAIT```状态，处于该状态下的TCP连接不能立即以同样的四元组建立新连接，即发起active close的那方占用的local port在```TIME_WAIT```期间不能再被重新分配。由于```TIME_WAIT```状态持续时间为**2MSL**， 这样保证了旧TCP连接双工链路中旧数据包因过期（超过MSL)而消失，此后，就可以用相同的四元组建立一条新连接而不会发生前后两次连接数据错乱的情况。



## 2. 处理系统上的TIME_WAIT连接过多问题

一般来说，当系统有较大的并发短连接压力时，都会出现少量的```TIME_WAIT```连接，这是正常的。但是有时候系统上会出现大量的```TIME_WAIT```状态的连接，从而导致再也没有可用端口来建立新的TCP连接。下面我们这对这一情况来进行讲解。

### 2.1 查看系统网络和当前TCP状态

在定位并处理应用程序出现的网络问题时，了解系统当前的网络配置是非常必要的。以x86_64平台Centos7.3为例，ipv4网络协议的默认配置可以在/proc/sys/net/ipv4/下查看，其中与TCP协议相关的配置项均以```tcp_xxx```命名，关于这些配置项的含义，请参考[这里](https://blog.csdn.net/li_101357/article/details/78415461)的文档，此外，还可以查看Linux源码树中提供的官方文档（src/linux/Documentation/ip-sysctl.txt)。下面列出我机器上几个需重点关注的配置项及其默认值：
<pre>
# cat /proc/sys/net/ipv4/ip_local_port_range
32768   60999
# cat /proc/sys/net/ipv4/tcp_max_syn_backlog 
2048
# cat /proc/sys/net/ipv4/tcp_syn_retries
6
# cat /proc/sys/net/ipv4/tcp_syncookies
1
# cat /proc/sys/net/ipv4/tcp_max_tw_buckets
5000
# cat /proc/sys/net/ipv4/tcp_tw_recycle
1
# cat /proc/sys/net/ipv4/tcp_tw_reuse 
1
# cat /proc/sys/net/ipv4/tcp_fin_timeout 
30
# cat /proc/sys/net/ipv4/tcp_timestamps 
1
</pre>

下面对这些参数进行一个简单的说明：

1） **ip_local_port_range**

该项说明了local port的分配范围，从上面可以看到默认的可用端口数不到3W。

2） **tcp_max_syn_backlog**

incomplete connection queue的最大长度

3) **tcp_syn_retries**

三次握手时SYN的最大重试次数

4)  **tcp_syncookies**

本选项用于控制是否开启SYN Cookies，为1时表示开启，为0时表示关闭。当出现SYN等待队列溢出时，启用cookies来处理，可防范少量SYN攻击。

5) **tcp_max_tw_buckets**

关于该字段的描述：
<pre>
Maximal number of time wait sockets held by system simultaneously. If this number is exceeded TIME_WAIT socket is 
immediately destroyed and warning is printed. This limit exists only to prevent simple DoS attacks, you must not 
lower the limit artificially, but rather increase it (probably, after increasing installed memory), if network 
conditions require more than default value (180000).
</pre>
即表示系统允许同时存在的处于```TIME_WAIT```状态的socket数量。该配置项可以用来防范简单的Dos攻击，在某些情况下可以适当调大，但绝对不应当调小，否则后果自负

6) **tcp_tw_recycle**

该配置项可用于快速回收处于TIME_WAIT状态的socket以便重新分配。默认是关闭的，必要时可以开启该配置项。但是开启该配置项后，有一些需要注意的地方，本文后面会提到。

7） **tcp_tw_reuse**

开启该选项后，kernel会复用处于```TIME_WAIT```状态的socket，当然复用的前提是“从协议角度来看，复用是安全的”。关于“在什么情况下，协议认为复用是安全的”这个问题，[这篇文章](https://blog.csdn.net/yunhua_lee/article/details/8146856)从Linux Kernel源码中挖出了答案，感兴趣的同学可以查看。

<pre>
关于什么情况下，协议认为复用是安全的？

从代码来看，tcp_tw_reuse选项和tcp_timestamps选项也必须同时打开；否则tcp_tw_reuse就不起作用另外，所谓的“协议安全”，从代码来看应该是收到最后一个包后超过1s。

另外，关于本字段，官方手册对于我们的建议为 It should not be changed without advice/request of technical experts
</pre>

8) **tcp_fin_timeout**

对于本端断开的socket连接，TCP保持在```FIN_WAIT_2```状态的时间。对方可能会断开连接或一直不结束连接或不可预料的进程死亡。默认值为60秒。过去在2.2版本的内核中是180秒。您可以设置该值，但是需要注意，如果你的机器为负载很重的Web服务器，你可能要冒内存被大量无效数据报填满的风险。```FIN_WAIT_2``` sockets的危险性低于```FIN_WAIT_1```，因为它们最多只吃1.5K的内存，但是它们存在时间更长。

9） **tcp_timestamps**

为1表示开启TCP时间戳，用来计算往返时间RTT（Round-Trip Time）和防止序列号回绕


### 2.2 网络问题定位思路
参考 [前篇笔记](https://blog.csdn.net/slvher/article/details/8941873) 开始处描述的线上实际问题，收到某台机器无法对外建立新连接的报警时，排查定位问题过程如下：

用如下命令
<pre>
# netstat -nat | grep TIME_WAIT
</pre>
统计发现，当时出问题的那台机器上共有10W+处于```TIME_WAIT```状态的TCP连接，进一步分析发现，由报警模块引起的TIME_WAIT连接有2W+。将netstat输出的统计结果重定位到文件中继续分析，一般会看到本机的port被大量占用。

由本文前面介绍的系统配置项可知，tcp_max_tw_buckets默认值为18W，而ip_local_port_range范围不到3W，大量的```TIME_WAIT```状态使得local port在```TIME_WAIT```持续期间不能被再次分配，即没有可用的local port，这将是导致新建连接失败的最大原因。


在这里提醒大家： 上面的结论只是我们的初步判断，具体原因还需要根据代码的异常返回值(如socket api的返回值及errno等）和模块日志做进一步确认。无法建立新连接的原因可能是被其他模块列入黑名单了。本人就有这方面的教训： 程序中使用libcurl api请求下游模块失败，初步定位发现机器```TIME_WAIT```状态很多，于是没有仔细分析curl输出日志就认为是```TIME_WAIT```引起的问题，导致浪费了很多时间，折腾了半天发现不对劲后才想起，下游模块有防攻击机制，而发起请求的机器IP不在下游模块的访问白名单内，高峰期上游模块通过curl请求下游的次数太过频繁被列入黑名单，新建连接时被下游模块的TCP层直接以RST包断开连接，导致curl api返回 “Recv failure: Connection reset by peer” 的错误，惨痛的教训呀！

另外，关于何时发送RST包，《Unix Network Programming Volume 1》第4.3节做了说明，作为笔记，摘出如下：
<pre>
An RST is a type of TCP segment that is sent by TCP when somethingis wrong.Three conditions that generatean RST are:            
1) when a SYN arrives for a port that has no listening server;
2) when TCP wants to abort an existing connection;
3) when TCP receives a segment for a connection that does not exist. (TCPv1 [pp.246–250] contains additional information.)
</pre>

### 2.3 解决方法

可以用两种思路来解决机器```TIME_WAIT```过多导致无法对外建立新TCP连接的问题。

1） **修改系统配置**

* 修改tcp_max_tw_buckets

将```tcp_max_tw_buckets```调大。从本文第一部分可知，其默认值为18w（不同内核可能有所不同，需以机器实际配置为准），根据文档，我们可以适当调大，至于上限是多少，文档没有给出说明，我也不清楚。个人认为这种方法只能对TIME_WAIT过多的问题起到缓解作用，随着访问压力的持续，该出现的问题迟早还是会出现，治标不治本。


* 开启tcp_tw_recycle选项

我们可以通过使用如下命令来开启该配置项：
{% highlight string %}
# echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle
{% endhighlight %}
需要明确的是，其实```TIME_WAIT```状态的socket是否被快速回收是由**tcp_tw_recycle**和**tcp_timestamps**两个配置项共同决定的，只不过由于 **tcp_timestamps**默认就是开启的，故大多数文章只提到设置**tcp_tw_recycle**为1。

注意： 关于tcp_tw_recycle参数，TCP有一种行为，可以缓存每个连接最新的时间戳，后续请求中如果时间戳小于缓存的时间戳，即视为无效，相应的数据包会被丢弃。Linux是否启用这种行为取决于tcp_timestamps和tcp_tw_recycle，因为tcp_timestamps缺省就是开启的，所以当tcp_tw_recycle被开启后，实际上这种行为就被激活了。在nat环境中会出现时间戳错乱的情况，后面的数据包就被丢弃了，具体的表现通常是是客户端明明发送的SYN，但服务端就是不响应ACK。因为NAT设备将数据包的源IP地址都改成了一个地址(或者少量的IP地址)，但是却基本上不修改TCP包的时间戳，则会导致时间戳混乱。建议：如果前端部署了三/四层NAT设备，尽量关闭快速回收，以免发生NAT背后真实机器由于时间戳混乱导致的SYN拒绝问题。

* 开启tcp_tw_reuse选项

我们可以通过使用如下命令来开启该配置项：
{% highlight string %}
# echo1 > /proc/sys/net/ipv4/tcp_tw_reuse
{% endhighlight %}
该选项也是与tcp_timestamps共同起作用的，另外socket reuse也是有条件的：协议认为复用是安全的。与**tcp_tw_recycle**选项相比，本选项一般不会带来可能的```副作用```。



2) **修改应用程序**

具体来说，可以细分为两种方式:

* 将TCP短连接改造为长连接

通常情况下，如果发起连接的目标也是自己可控制的服务器时，它们自己的TCP通信最好采用长连接，避免大量TCP短连接每次建立/释放产生的各种开销；如果建立连接的目标是不受自己控制的机器时，能否使用长连接就需要考虑对方机器是否支持长连接方式了


* 快速关闭socket
       
通过getsockopt/setsockoptapi设置socket的SO_LINGER选项，关于SO_LINGER选项的设置方法，《UNP Volume1》一书7.5节给出了详细说明，想深入理解的同学可以去查阅该教材，也可以参考[这篇文章](https://blog.csdn.net/yunhua_lee/article/details/8146837)，讲的还算清楚



<br />
<br />

**[参看]**

1. [time_wait的快速回收和重用](https://www.cnblogs.com/LUO77/p/8555103.html)

2. [大量TIME_WAIT的终极详解和解决方案](https://blog.csdn.net/bingqingsuimeng/article/details/52064841)

3. [linux 大量的TIME_WAIT解决办法](https://www.cnblogs.com/softidea/p/6062147.html)

4. [ip-sysctl.txt](https://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt)

5. [Linux上的TIME_WAIT和tcp_fin_timeout](https://blog.csdn.net/Aquester/article/details/79969937)
<br />
<br />
<br />

