---
layout: post
title: nginx超时设置
tags:
- nginx
categories: nginx
description: nginx使用基础
---

nginx中涉及到大量的超时方面的设置，本章我们介绍一下这方面的内容。

<!-- more -->

## 1. nginx超时设置

我们通常使用nginx来作为反向代理服务器，工作的典型情况如下：

![ngx-timeout-set](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_timeout_set.jpg)

超时方面的设置通常也涉及到client与nginx、nginx与proxied server之间的超时。下面我们简要介绍一下：

### 1.1 client与nginx之间的超时
1) **keepalive_timeout指令**

在ngx_http_core_module模块及ngx_http_upstream_module模块均有keepalive_timeout指令。这里我们讲述的是工作在core模块的该指令。其基本语法如下：
{% highlight string %}
Syntax:	keepalive_timeout timeout [header_timeout];
Default:	
keepalive_timeout 75s;
Context:	http, server, location
{% endhighlight %}
第一个参数用于设置对于一个keep-alive类型的客户端连接，服务器端会保持为打开状态的超时时间。假如设置为0，则会禁止keep-alive客户端连接。第二个可选参数用于设置"Keep-Alive: timeout=time"响应头。这两个参数值是可以不同的。

Mozilla以及Konqueror可以识别"Keep-Alive: timeout=time"响应头。而MSIE会在60s内由其自己关闭连接。


HTTP是一种无状态协议，客户端向服务器发送一个TCP请求，服务端响应完毕后断开连接。如果客户端向服务器发送多个请求，每个请求都要建立各自独立的连接以传输数据。HTTP有一个KeepAlive模式，它告诉webserver在处理完一个请求后保持这个TCP连接的打开状态。若接收到来自客户端的其他请求，服务端会利用这个未被关闭的连接，而不需要再建立一个连接。

KeepAlive在一段时间内保持打开状态，它们会在这段时间内占用资源。占用过多就会影响性能。关于http的keep-alive，请参看：**浅谈HTTP长连接和Keep-Alive**

2) **client_header_timeout指令**

我们可以在ngx_http_core_module模块找到client_header_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	client_header_timeout time;
Default:	
client_header_timeout 60s;
Context:	http, server
{% endhighlight %}
用于指定nginx读取客户端```请求头```(request header)的超时时间。假如客户端在该超时时间之内未传输完整个http头，则请求会被终止并返回一个408(Request Time-out)错误。

3) **client_body_timeout指令**

我们可以在ngx_http_core_module模块找到client_body_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	client_body_timeout time;
Default:	
client_body_timeout 60s;
Context:	http, server, location
{% endhighlight %}
用于指定nginx读取客户端```请求体```(request body)的超时时间。该超时时间是指**两次连续的读**操作之间的时间间隔，而不是整个请求体（request body)的传输时间。假如在超时时间之内，客户端并没有传递任何数据，则请求会被终止并返回一个408(Request Time-out)错误。

4） **send_timeout指令**

我们可以在ngx_http_core_module模块找到send_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	send_timeout time;
Default:	
send_timeout 60s;
Context:	http, server, location
{% endhighlight %}
用于设置nginx发送响应倒客户端的超时时间。该超时时间是指**两次连续的写**操作之间的时间间隔，而不是整个响应的传输时间。假如客户端在该超时时间内未收到任何数据，则连接会被关闭。

5) **lingering_timeout指令**

我们可以在ngx_http_core_module模块找到lingering_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	lingering_timeout time;
Default:	
lingering_timeout 5s;
Context:	http, server, location
{% endhighlight %}
当```lingering_close```开启之后，本指令用于设置在连接关闭之前最长等待多长时间以接受更多的客户端数据。假如在这段时间之内并没有数据到达，连接将会被关闭。否则，数据会被读取并忽略，nginx开始等待更多的数据到达。```wait-read-ignore```循环会一直重复，但最长不会超过lingering_time时间。
<pre>
注： 默认情况下lingering_close是被启用的。
</pre>


### 1.2 nginx与proxied server之间的超时

1) **proxy_connect_timeout指令**

我们可以在ngx_http_proxy_module模块找到proxy_connect_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	proxy_connect_timeout time;
Default:	
proxy_connect_timeout 60s;
Context:	http, server, location
{% endhighlight %}
定义nginx与后端代理服务器(proxied server)建立连接时的超时时间。我们需要注意的是，该超时时间通常不应该超过```75s```。

2) **proxy_read_timeout指令**

我们可以在ngx_http_proxy_module模块找到proxy_read_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	proxy_read_timeout time;
Default:	
proxy_read_timeout 60s;
Context:	http, server, location
{% endhighlight %}
定义nginx读取后端代理服务器(proxied server)响应的超时时间。该超时时间是指**两次连续的读**操作之间的时间间隔，而不是整个响应的传输时间。假如代理服务器在该超时时间段内没有传递任何数据，连接将被关闭。

3) **proxy_send_timeout指令**

我们可以在ngx_http_proxy_module模块找到proxy_send_timeout指令，其基本语法如下：
{% highlight string %}
Syntax:	proxy_send_timeout time;
Default:	
proxy_send_timeout 60s;
Context:	http, server, location
{% endhighlight %}
用于设置将请求发送到代理服务器(proxied server)的超时时间。该超时时间是指**两次连续的写**操作之间的时间间隔，而不是整个请求的传输时间。假如代理服务器(proxied server)在该超时时间之内没有收到任何数据，则连接会被关闭。

### 1.3 其他超时时间

1） **resolver_timeout指令**

resolver_timeout指令在ngx_http_core_module模块、ngx_mail_core_module模块以及ngx_stream_core_module模块均可找到，在三个模块中的语法也类似。我们以http模块为例：
{% highlight string %}
Syntax:	resolver_timeout time;
Default:	
resolver_timeout 30s;
Context:	http, server, location
{% endhighlight %}
用于设置解析一个名称(name)的超时时间。


## 2. nginx日志中常用的时间参数
在nginx日志中，有时为了排查问题，我们需要加上时间戳信息。通常我们会按如下方式打印日志信息：
{% highlight string %}
error_log /var/log/nginx/error.log info;

http {
        log_format  main  '$remote_addr - $remote_user [$time_local]  [$request_time] [$upstream_response_time] "$request" '
                                        '$status $body_bytes_sent "$http_referer" '
                                        '"$http_user_agent" "$http_x_forwarded_for" "$upstream_addr" ';
}
{% endhighlight %}
上面涉及到3个时间戳：

1) **time_local**

在ngx_http_core_module、ngx_http_log_module以及ngx_stream_core_module模块中均可以找到time_local变量。在上面例子中，```$time_local```对应的是ngx_http_log_module模块中的变量。它是nginx处理完打印日志的时间，而不是请求发出的时间。

2) **request_time**

请求的处理时间，分辨率为毫秒。该时间是从nginx接收到来自客户端第一个字节开始，到nginx向客户端返回最后一个字节后写日志时的时间间隔。


3) **upstream_response_time**

我们可以在ngx_http_upstream_module模块找到本变量。用于指示从Nginx向后端(proxied server)建立连接开始到接收完数据然后关闭连接为止的时间，分辨率为毫秒。


## 3. 附录： 浅谈HTTP长连接和Keep-Alive

1) **Keep-Alive模式**

我们知道http协议采用“请求-应答”模式，当使用普通模式，即非Keep-Alive模式时，对于每个请求/应答，客户端和服务器都要新建一个连接 ，完成之后立即断开连接； 当使用Keep-Alive模式时，Keep-Alive功能使客户端到服务器端的连接持续有效，当出现对服务器的后继请求时，Keep-Alive功能避免了建立或重新建立连接。

在HTTP1.0中默认是关闭的，需要在http头加入"Connection: Keep-Alive"才能启用Keep-Alive；

在HTTP1.1中默认启用Keep-Alive，如果加入"Connection: close"才关闭。目前大部分浏览器都是用http1.1协议，也就是说默认都会发起Keep-Alive的连接请求了，所以是否能完成一个完整的Keep-Alive连接就看服务器设置情况。下图是**普通模式**和**长连接模式**的请求对比：


![ngx-http-keepalive](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_http_keepalive.png)


2) **开启Keep-Alive的优缺点**

* 优点： Keep-Alive模式更加高效，因为避免了连接建立和释放的开销；

* 缺点： 长时间的tcp连接容易导致系统资源无效占用，浪费系统资源；

3） **当保持长连接时，如何判断一次请求已经完成？**

* Content-Length: 表示实体内容的长度。浏览器通过这个字段来判断当前请求的数据是否已经全部接收。所以，当浏览器请求的是一个静态资源时，即服务器能明确知道返回内容的长度时，可以设置Content-Length来控制请求的结束。但当服务器并不知道请求结果的长度时，如一个动态的页面或者数据，Content-Length就无法解决上面的问题，这个时候就需要用到Transfer-Encoding字段。

* Transfer-Encoding: 是指传输编码，在上面的问题中，当服务器无法知道实体内容的长度时，就可以通过指定```Transfer-Encoding: chunked```来告知浏览器当前的编码是将数据分成一块一块传递的。当然，还可以指定"Transfer-Encoding: gzip, chunked"，表明实体内容不仅是gzip压缩的，还是分块传递的。最后，当浏览器接收到一个长度为0的chunked时，知道当前请求内容已全部接收。

4） **Keep-Alive timeout**

http守护进程一般都提供了keep-alive timeout时间设置参数。比如nginx的keepalive_timeout， 和Apache的KeepAliveTimeout。这个keepalive_timeout时间意味着： 一个http产生的tcp连接在传送完最后一个响应后，还需要hold住keepalive_timeout秒后，才开始关闭这个连接。

当http守护进程发送完一个响应后，理应马上主动关闭响应的TCP连接，设置keepalive_timeout后，http守护进程会想说：“再等等， 看看浏览器还有没有请求过来”，这一等，便是keepalive_timeout时间。如果守护进程在这个等待的时间里，一直没有收到浏览器发过来的http请求，则关闭这个http连接。

5） **TCP的keepalive**

连接建立之后，如果客户端一直不发送数据，或者隔很长时间才发送一次数据，当连接很久没有数据报文传输时如何去确定对方还在线，到底是掉线了还是确实没有数据传输，连接还需不需要保持，这种情况在TCP协议设计中是需要考虑的。

TCP协议通过一种巧妙的方式去解决这个问题，当超过一段时间之后，TCP自动发送一个数据为空的报文（侦测包）给对方，如果对方回应了这个报文，说明对方还在线，连接可以继续保持； 如果对方没有报文返回，并且重试了多次之后则认为链接丢失，没必要保持连接。

tcp keep-alive是TCP的一种检测TCP连接状况的保鲜机制。 tcp keep-alive保鲜定时器支持三个系统内核配置参数：
<pre>
# ls -l /proc/sys/net/ipv4/tcp_keepalive_*
-rw-r--r-- 1 root root 0 May 15 19:36 /proc/sys/net/ipv4/tcp_keepalive_intvl
-rw-r--r-- 1 root root 0 May 15 19:36 /proc/sys/net/ipv4/tcp_keepalive_probes
-rw-r--r-- 1 root root 0 May 15 19:36 /proc/sys/net/ipv4/tcp_keepalive_time

# cat /proc/sys/net/ipv4/tcp_keepalive_intvl
75
# cat /proc/sys/net/ipv4/tcp_keepalive_probes
9
# cat /proc/sys/net/ipv4/tcp_keepalive_time
7200
</pre>
keepalive是TCP保鲜定时器，当网络两端建立了TCP连接之后，闲置（双方没有任何数据流发送往来）了tcp_keepalive_time后，服务器就会尝试向客户端发送侦测包，来判断TCP连接状况（可能客户端崩溃、强制关闭了应用、主机不可达等等）。如果没有收到对方的回答(ack包），则会在tcp_keepalive_intvl后再次尝试发送侦测包，直到收到对方的ack。如果一直没有收到对方的ack，一共会尝试tcp_keepalive_probes次，每次的时间间隔分别是75s、150s、225s、300s。如果尝试tcp_keepalive_probes次，依然没收到对方的ack包，则会丢弃该tcp连接。TCP连接默认闲置时间是2小时，一般设置为30分钟足够了。

<pre>
注： 谁想定期检查连接状况,谁就启用keep alive。另一端可以不起，只是被动地对探测包进行响应，这种响应是
    tcp协议的基本要求，跟keep alive无关。并不需要客户端和服务器端都开启keep alive
</pre>


<br />
<br />

**[参看]**

1. [nginx中的超时设置，请求超时、响应等待超时等](https://www.cnblogs.com/lemon-flm/p/8352194.html)

2. [Nginx的超时timeout配置详解](https://www.jb51.net/article/131868.htm)

4. [优化Nginx连接参数，调整连接超时时间](https://blog.51cto.com/13673885/2299770)

5. [nginx指令](http://nginx.org/en/docs/dirindex.html)

6. [ngin日志time_local解释](https://blog.csdn.net/mental_derangement/article/details/81779795)

<br />
<br />
<br />

