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

### 1.2 nginx与proxied server之间的超时




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

* Transfer-Encoding: 



<br />
<br />

**[参看]**

1. [nginx中的超时设置，请求超时、响应等待超时等](https://www.cnblogs.com/lemon-flm/p/8352194.html)

2. [Nginx的超时timeout配置详解](https://www.jb51.net/article/131868.htm)

4. [优化Nginx连接参数，调整连接超时时间](https://blog.51cto.com/13673885/2299770)

5. [nginx指令](http://nginx.org/en/docs/dirindex.html)

6. []()

<br />
<br />
<br />

