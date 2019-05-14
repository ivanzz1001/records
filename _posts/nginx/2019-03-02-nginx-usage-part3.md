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

1) **proxy_buffering指令**

proxy_buffering指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_buffering on | off;
Default:	
proxy_buffering on;
Context:	http, server, location
{% endhighlight %}

我们可以在ngx_http_proxy_module模块找到关于proxy_buffering的描述。该指令用于控制是否开启对后端代理服务器(proxied server)响应的缓冲。当通过设置为```on```开启缓冲之后，nginx会尽快的从后端代理服务器接收响应数据，然后将其存放到由proxy_buffer_size与proxy_buffers指令设置的缓存中。假如整个响应数据超出了内存缓冲的大小，其中的一部分响应数据会被存放到硬盘的零时文件(temporary file)中。该临时文件由proxy_max_temp_file_size与proxy_temp_file_write_size指令所控制。

通过本指令设置为```off```来禁用响应缓存后，则当nginx收到后端代理服务器的响应信息时，会马上将响应信息同步的发送给客户端。nginx并不会尝试从后端代理服务器读取整个响应信息。nginx一次性可以从后端服务器接收的数据大小为proxy_buffer_size。

注意，我们也可以通过```X-Accel-Buffering```响应头(response header)的yes或no值来控制启用或禁用**响应缓存**。当然，如果我们要禁用```X-Accel-Buffering```功能，可以使用proxy_ignore_headers指令来做到。


2) **proxy_buffer_size指令**

proxy_buffer_size指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_buffer_size size;
Default:	
proxy_buffer_size 4k|8k;
Context:	http, server, location
{% endhighlight %}
我们可以在ngx_http_proxy_module模块找到proxy_buffer_size的描述。此指令用于设置从后端代理服务器读取到的**初始响应部分**(first part of the response)所使用的缓存大小。该**初始响应部分**通常包含一个小的响应头。默认情况下，缓存的大小等于一个内存页(memory page)的大小。根据操作系统平台的不同，通常为4K或者8K。然而，我们也可以通过本指令将缓存设的更小。
<pre>
注： proxy_buffer_size通常只作为响应头的缓冲区，没必要设置的太大，一般设置为4K就够了
</pre>

3) **proxy_buffers指令**

proxy_buffers指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_buffers number size;
Default:	
proxy_buffers 8 4k|8k;
Context:	http, server, location
{% endhighlight %}

我们可以在ngx_http_proxy_module模块找到proxy_buffers的描述。此指令针对每一个单独的连接，用于设置从后端代理服务器读取响应所使用的缓存的**个数**(number)及**大小**(size)。默认情况下，缓存的大小等于一个内存页(memory page)的大小。根据操作系统平台的不同，通常为4K或8K。

proxy_buffers的缓冲区大小一般会设置的比较大，以应付大网页。proxy_buffers由缓冲区数量和缓冲区大小组成的，总的大小为```number * size```。若某些请求的响应过大，则超过proxy_buffers的部分将被缓冲到硬盘（缓冲目录由proxy_temp_path指令指定），当然这将会使读取响应的速度减慢，影响用户体验，可以使用proxy_max_temp_file_size指令关闭磁盘缓冲。

4) **proxy_busy_buffers_size指令**

proxy_busy_buffers_size指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_busy_buffers_size size;
Default:	
proxy_busy_buffers_size 8k|16k;
Context:	http, server, location
{% endhighlight %}
我们可以在ngx_http_proxy_module模块找到本指令的相关描述。当使用```proxy_buffering```指令启用了对后端代理服务器的响应缓冲时，通过本指令来**限制**在后端响应未全部读取完的情况下，最多size大小的缓冲处于busy状态以将响应返回给客户端。在这段时间内，剩余的缓冲可以被用于继续读取后端代理服务器的响应，甚至在必要的情况下将响应数据缓存到临时文件。默认情况下，size的大小受到指令proxy_buffer_size以及指令proxy_buffers中size参数的限制。

proxy_busy_buffers_size不是独立的空间，它是proxy_buffers和proxy_buffer_size的一部分。nginx会在没有完全读完后端响应的时候就开始向客户端传送数据，所以它会划出一部分缓冲区来专门向客户端传送数据（这部分的大小是由proxy_busy_buffers_size来控制，建议为proxy_buffers中单个缓冲区大小的2倍），然后它继续从后端取数据，缓冲区满了之后就写到磁盘的临时文件中。

5) **proxy_max_temp_file_size指令**

proxy_max_temp_file_size指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_max_temp_file_size size;
Default:	
proxy_max_temp_file_size 1024m;
Context:	http, server, location
{% endhighlight %}
我们可以在ngx_http_proxy_module模块找到本指令的相关描述。当使用```proxy_buffering```指令启用了对后端代理服务器的响应缓冲时，假如后端代理服务器的响应超出了**proxy_buffer_size**以及**proxy_buffers**指令设置的缓冲区大小，则有一部分响应会被保存到临时文件中。本指令用于设置临时文件的最大大小，而每一次向临时文件写入的数据大小由proxy_temp_file_write_size指令控制。

如果超过了这个值, Nginx将与Proxy服务器同步的传递内容, 而不再缓冲到硬盘；如果我们将proxy_max_temp_file_size的大小设置为0，则会禁止将响应写入到临时文件。

<pre>
注： 本限制并不对那些cached(proxy_cache)到硬盘上的响应产生影响
</pre>

6) **proxy_temp_file_write_size指令**

proxy_temp_file_write_size指令的基本语法格式如下：
{% highlight string %}
Syntax:	proxy_temp_file_write_size size;
Default:	
proxy_temp_file_write_size 8k|16k;
Context:	http, server, location
{% endhighlight %}
我们可以在ngx_http_proxy_module模块找到本指令的相关描述。当启用了将后端响应缓存到临时文件时，本指令用于限制每次向临时文件写入的数据大小。

默认情况下，size会受到proxy_buffer_size以及proxy_buffers这两个缓冲的影响。


## 2. nginx代理缓存配置示例

* 通用网站配置
{% highlight string %}
proxy_buffer_size 4k;            #设置代理服务器（nginx）保存用户头信息的缓冲区大小
proxy_buffers 4 32k;             #proxy_buffers缓冲区，网页平均在32k以下的设置
proxy_busy_buffers_size 64k;     #高负荷下缓冲大小（proxy_buffers*2）
proxy_temp_file_write_size 64k;
#设定缓存文件夹大小，大于这个值，将从upstream服务器传
{% endhighlight %}

* docker registry的配置 这个每次传输至少都是9M以上的内容，缓冲区配置大
{% highlight string %}
proxy_buffering on;
proxy_buffer_size 4k; 
proxy_buffers 8 1M;
proxy_busy_buffers_size 2M;
proxy_max_temp_file_size 0;
{% endhighlight %}


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

