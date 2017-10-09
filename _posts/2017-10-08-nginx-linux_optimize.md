---
layout: post
title: Linux内核参数针对nginx的优化
tags:
- nginx
categories: nginx
description: nginx内核参数针对nginx的优化
---


为了使nginx能够支持更多的并发，达到更高的性能，我们这里对Linux内核参数进行优化。




<!-- more -->


## 1. Linux内核参数的优化
由于默认的Linux内核参数考虑的是最通用的场景，这明显不符合支持高并发访问的Web服务器的定义，所以需要修改Linux内核参数，使得Nginx可以拥有更高的性能。

在优化内核时，可以做的事情很多，不过我们通常会根据业务特点来进行调整，当Nginx作为静态Web内容服务器、```反向代理服务器```或是提供图片缩略功能（实时压缩图片）的服务器时，其内核参数的调整都是不同的。这里只针对最通用的、使Nginx支持更多并发请求的TCP网络参数做简单说明。

首先，需要修改/etc/sysctl.conf来更改内核参数。例如,最常用的配置：

{% highlight string %}
fs.file-max = 999999
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_keepalive_time = 600
net.ipv4.tcp_fin_timeout = 30
net.ipv4.tcp_max_tw_buckets = 5000
net.ipv4.ip_local_port_range = 1024 61000
net.ipv4.tcp_rmem = 4096 32768 262142
net.ipv4.tcp_wmem = 4096 32768 262142
net.core.netdev_max_backlog = 8096
net.core.rmem_default = 262144
net.core.wmem_default = 262144
net.core.rmem_max = 2097152
net.core.wmem_max = 2097152
net.ipv4.tcp_syncookies = 1
net.ipv4.tcp_max_syn.backlog = 1024
{% endhighlight %}
然后执行```sysctl -p```命令，使上述修改生效。

上面的参数意义解释如下：

* file-max: 这个参数表示进程（比如一个worker进程）可以同时打开的最大句柄数，这个参数直接限制最大并发连接数，需根据实际情况配置。


* tcp_tw_reuse: 这个参数设置为1，表示允许将TIME_WAIT状态的socket重新用于新的TCP连接，这对于服务器来说很有意义，因为服务器上总会有大量TIME_WAIT状态的连接。

* tcp_keepalive_time: 这个参数表示当keepalive启用时，TCP发送keepalive消息的频度。默认是2小时，若将其设置得小一些，可以更快地清理无效的连接。

* tcp_fin_timeout: 这个参数表示当服务器主动关闭连接时，socket保持在FIN-WAIT-2状态的最大时间。

* tcp_max_tw_buckets: 这个参数表示操作系统允许TIME_WAIT套接字数量的最大值，如果超过这个数字，TIME_WAIT套接字将立刻被清除并打印警告信息。该参数默认为180000, 过多的TIME_WAIT套接字会使Web服务器变慢。

* tcp_max_syn_backlog: 这个参数表示TCP三次握手建立阶段接收SYN请求队列的最大长度，默认为1024，将其设置得大一些可以使出现Nginx繁忙来不及accept新连接的情况时，Linux不至于丢失客户端发起的连接请求。

* ip_local_port_range: 这个参数定义了UDP和TCP连接中本地（不包括连接的远端）端口的取值范围。

* net.ipv4.tcp_rmem: 这个参数定义了TCP接收缓存(用于TCP接收滑动窗口）的最小值、默认值、最大值。

* net.ipv4.tcp_wmem: 这个参数定义了TCP发送缓存(用于TCP发送滑动窗口）的最小值、默认值、最大值。

* netdev_max_backlog: 当网卡接收数据包的速度大于内核处理的速度时，会有一个队列保存这些数据包。这个参数表示该队列的最大值。

* rmem_default: 这个参数表示内核套接字接收缓存区默认的大小。

* wmem_default: 这个参数表示内核套接字发送缓存区默认的大小。

* rmem_max: 这个参数表示内核套接字接收缓存区的最大大小。

* wmem_max: 这个参数表示内核套接字发送缓存区的最大大小。
<pre>
注意： 滑动窗口的大小与套接字缓存区会在一定程度上影响并发连接连接的数目。每个TCP连接
都会为维护TCP滑动窗口而消耗内存，这个窗口会根据服务器的处理速度收缩或扩张。

参数wmem_max的设置，需要平衡内存的总大小、Nginx并发处理的最大连接数量（由nginx.conf中的worker_processes和worker_connections参数决定）而确定。当然，如果仅仅为了提高并发量使服务器不出现Out Of Memory(OOM)问题而去降低滑动窗口大小，那么并不合适，因为滑动窗口过小会影响大数据量的传输速度。rmem_default、wmem_default、rmem_max、wmem_max这4个参数的设置需要根据我们的业务特性以及实际的硬件成本来综合考虑。
</pre>

* tcp_syncookies: 该参数与性能无关，用于解决TCP的SYN攻击。

<br />

```说明：```

![proxy](https://ivanzz1001.github.io/records/assets/img/nginx/nginx_proxy.jpg)
正向代理中，proxy和client同属一个LAN, proxy对server透明。
反向代理中，proxy和server同属一个LAN，proxy对client透明。







<br />
<br />

参看：

1. [centos7优化内核参数详解](http://blog.csdn.net/xiegh2014/article/details/52132863)


<br />
<br />
<br />

