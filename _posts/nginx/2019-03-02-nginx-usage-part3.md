---
layout: post
title: nginx缓冲区优化
tags:
- nginx
categories: nginx
description: nginx使用基础
---


本章我们介绍一下Nginx中用到的一些缓冲，通过合理的设置缓冲区大小，可以改善整个程序的性能。

<!-- more -->


## 1. nginx缓存

1) **proxy_buffering**

我们可以在ngx_http_proxy_module模块找到关于proxy_buffering的描述。该指令用于控制是否开启对后端代理服务器(proxied server)响应的缓冲。当通过设置为```on```开启缓冲之后，nginx会尽快的从后端代理服务器接收响应数据，然后将其存放到由proxy_buffer_size与proxy_buffers指令设置的缓存中。假如整个响应数据超出了内存缓冲的大小，其中的一部分响应数据会被存放到硬盘的零时文件(temporary file)中。该临时文件由proxy_max_temp_file_size与proxy_temp_file_write_size指令所控制。

通过本指令设置为```off```来禁用响应缓存后，则当nginx收到后端代理服务器的响应信息时，会马上将响应信息同步的发送给客户端。nginx并不会尝试从后端代理服务器读取整个响应信息。nginx一次性可以从后端服务器接收的数据大小为proxy_buffer_size。

注意，我们也可以通过```X-Accel-Buffering```响应头(response header)的yes或no值来控制启用或禁用**响应缓存**。当然，如果我们要禁用```X-Accel-Buffering```功能，可以使用proxy_ignore_headers指令来做到。

2) **proxy_buffer_size**



<br />
<br />

**[参看]**

1. [nginx缓冲区优化](https://www.cnblogs.com/me115/p/5698787.html)

2. [nginx优化缓冲缓存](https://www.cnblogs.com/bethal/p/5606062.html)

3. [nginx下载大文件失败的解决方法](https://blog.51cto.com/zhukeqiang/2286462)

4. [时间request_time和upstream_response_time](https://happyqing.iteye.com/blog/2384266)

<br />
<br />
<br />

