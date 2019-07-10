---
layout: post
title: HAProxy的安装及使用
tags:
- lb
categories: lb
description: HAProxy的安装及使用
---

本文主要介绍一下HAProxy的安装及使用。

<!-- more -->
## 1. HAProxy简介
HAProxy是一个使用C语言编写的自由及开放源代码软件，其提供高可用性、负载均衡，以及基于TCP和HTTP的应用程序代理。

HAProxy特别适用于那些负载大的Web站点，这些站点通常又需要会话保持或七层处理。HAProxy运行在当前的硬件上，完全可以支持数以万计的并发连接。并且它的运行模式使得它可以很简单安全地整合进您当前的架构中，同时可以保护你的Web服务器不被暴露到网络上。

HAProxy实现了一种事件驱动，单一进程模型，此模型支持非常大的并发连接数。多进程或多线程模型受内存限制、系统调度器限制以及无处不在的锁限制，很少能处理数千并发连接。事件驱动模型因为在有更好的资源和时间管理的用户空间(user space)实现所有这些任务，所有没有这些问题。此模型的弊端是，在多核系统上，这些程序通常扩展性较差。这就是为什么它们必须进行优化以使每个CPU时间片(Cycle)做更多的工作。

## 2. HAProxy、Nginx、LVS三者的优缺点

### 2.1 Nginx优缺点
* **Nginx优点**

1) 工作在7层网络之上，可针对HTTP应用做一些分流的策略，如针对域名、目录结构，它的正则规则比HAProxy更为强大和灵活，所以目前为止广为流行。

2） Nginx对网络稳定性的依赖非常小，理论上能ping通就能进行负载功能

3） Nginx安装与配置比较简单，测试也比较方便，基本能把错误日志打印出来

4） 可以承担高负载压力且稳定，硬件不差的情况下一般每秒能支持几万次的并发量，但负载度比LVS小

5） Nginx可以通过端口检测到服务器内部的故障，如根据服务器处理网页返回的状态码、超时等，并会把返回错误的请求重新提交到另一个节点

6） 不仅仅是优秀的负载均衡、反向代理软件，同时也是强大的Web应用服务器。LNMP也是今年非常流行的Web架构，在高流量环境中稳定性也很好

7） 可以作为中间的反向代理使用

8） 可作为静态网页和图片服务器

9） Nginx社区活跃，第三方模块非常多，相关的资料在网上比比皆是


* **Nginx缺点**

1） 适用范围较小，仅能支持http、https、email协议

2） 对后端服务器的健康检查，只支持通过端口检测，不支持URL来检测。比如用户正在上传一个文件，而处理该上传的节点刚好在上传过程中出现故障，Nginx会把上传切换到另一台服务器重新处理，而LVS就直接断掉了，如果是上传一个很大的文件或者很重要的文件的话，用户可能会因此不满。




### 2.2 LVS优缺点

* **LVS优点**

1) 抗负载能力强，工作在4层网络之上仅做分发之用，没有其他额外流量的产生，这个特点也决定了它在负载均衡软件里性能是最强的，对内存和CPU资源消耗比较低。

2） 配置性比较低，这是一个优点同时也是一个缺点，因为没有太多可配置的东西，所以并不需要太多接触就能使用，大大减少了人为出错的几率

3） 工作稳定，因为其本身抗负载能力很强，自身有完整的双机热备方案，如LVS+Keepalived，不过我们在项目实施中用得最多的还是LVS/DR + Keepalived

4） 无流量，LVS只分发请求，而流量并不从它本身出去，这点保证了均衡器IO的性能不会受到大流量的影响

* **LVS的缺点**

1) 软件本身并不支持正则表达式处理，不能做动静分离；而现在许多网站在这方面都有较强的需求，这个是Nginx/HAProxy + Keepalived的优势所在

2） 如果是网站应用比较庞大的话，LVS/DR + Keepalived实施起来就比较复杂了，特别是后面有Windows Server的机器的话，实施及配置还有维护过程就比较复杂了，相对而言，Nginx/HAProxy + Keepalived就简单多了。

### 2.3 HAProxy优缺点

* **HAProxy优点**

1) HAProxy是支持虚拟主机的，可以工作在4层、7层网络

2） HAProxy的优点能够补充Nginx的一切缺点，比如支持Session的保持，Cookie的引导；同时支持通过获取指定的url来检测后端服务器的状态

3） HAProxy跟LVS类似，本身就只是一款负载均衡软件；单纯从效率上来讲，HAProxy会比Nginx有更出色的负载均衡速度，在并发处理上也是由于Nginx的

4） HAProxy支持TCP协议的负载均衡转发

5） HAProxy负载均衡策略非常多，HAProxy的负载现在具有有如下8种
<pre>
1. roundrobin
表示简单的轮询，每个服务器根据权重轮流使用，在服务器的处理时间平均分配的情况下这是最流畅和公平的算法。该算法是动态的，对于
实例启动慢的服务器权重会在运行中调整。最大支持4095个后端主机。

2. leastconn
连接数最少的服务器优先接收连接。leastconn建议用于长会话服务，例如LDAP、SQL、TSE等，而不适合短会话协议，如HTTP。该算法是
动态的，对于实例启动慢的服务器权重会在运行中调整

3. static-rr
每个服务器根据权重轮流使用，类似roundrobin，但它是静态的，意味着运行时修改权重是无效的。另外，它对服务器的数量没有限制。通常
我们不使用该算法

4. source

5. uri

6. url_pattern

7. hdr(name)

8. rdp-cookie(name)
</pre>


* **HAProxy缺点**

1) 不支持POP/SMTP协议

2） 不支持SPDY协议

3） 不支持HTTP Cache。现在不少的开源lb项目，都或多或少的具备HTTP Cache功能

4) 重载配置的功能需要重启进程，虽然也是soft restart，但没有nginx的reload那般平滑和友好

5） 多进程模式支持不够好


## 3. HAProxy的安装

1） **下载并解压HAProxy**

我们可以到[HAProxy官网](https://www.haproxy.org/)去下载最新版本的HAProxy(可能需要翻墙），然后进行安装。
<pre>
# mkdir haproxy-inst
# cd haproxy-inst/
# wget https://www.haproxy.org/download/2.0/src/haproxy-2.0.1.tar.gz
# tar -zxvf haproxy-2.0.1.tar.gz
# cd haproxy-2.0.1/
</pre>

2) **安装HAProxy**
<pre>
# mkdir -p /usr/local/HAProxy
# make TARGET=linux-glibc PREFIX=/usr/local/HAProxy SBINDIR=/usr/local/sbin ARCH=x86_64
# make install PREFIX=/usr/local/HAProxy SBINDIR=/usr/local/sbin
# tree /usr/local/HAProxy/
/usr/local/HAProxy/
├── doc
│   └── haproxy
│       ├── 51Degrees-device-detection.txt
│       ├── architecture.txt
│       ├── close-options.txt
│       ├── configuration.txt
│       ├── cookie-options.txt
│       ├── DeviceAtlas-device-detection.txt
│       ├── intro.txt
│       ├── linux-syn-cookies.txt
│       ├── lua.txt
│       ├── management.txt
│       ├── netscaler-client-ip-insertion-protocol.txt
│       ├── network-namespaces.txt
│       ├── peers.txt
│       ├── peers-v2.0.txt
│       ├── proxy-protocol.txt
│       ├── regression-testing.txt
│       ├── seamless_reload.txt
│       ├── SOCKS4.protocol.txt
│       ├── SPOE.txt
│       └── WURFL-device-detection.txt
└── share
    └── man
        └── man1
            └── haproxy.1

5 directories, 21 files

# tree /usr/local/sbin
/usr/local/sbin
├── haproxy

0 directories, 1 files
</pre>

3) **生成配置文件**

默认情况下```haproxy```会加载*/etc/haproxy/haproxy.cfg*文件，当然我们也可以通过```-f```选项来指定加载别处的配置文件。新版的HAProxy在编译安装后并不会默认为我们生成```haproxy.cfg```，但是在安装源代码的```examples```目录下有一些示例可以参考，我们将其都复制到*/etc/haproxy*目录下：
<pre>
# mkdir -p /etc/haproxy
# cp -ar examples/* /etc/haproxy/
# tree /etc/haproxy/
/etc/haproxy/
├── acl-content-sw.cfg
├── content-sw-sample.cfg
├── errorfiles
│   ├── 400.http
│   ├── 403.http
│   ├── 408.http
│   ├── 500.http
│   ├── 502.http
│   ├── 503.http
│   ├── 504.http
│   └── README
├── haproxy.init
├── option-http_proxy.cfg
├── socks4.cfg
├── transparent_proxy.cfg
└── wurfl-example.cfg

1 directory, 15 files
</pre>

4） **配置启动脚本**

将安装源代码的```examples```目录下的```haproxy.init```文件复制到```/etc/init.d/```目录下，并改名为```haproxy```，并赋予可执行权限:
<pre>
# cp examples/haproxy.init /etc/init.d/haproxy
# chmod 0755 /etc/init.d/haproxy
</pre>
注意此处需要根据我们上面编译时的安装目录，对*/etc/init.d/haproxy*稍作修改：
<pre>
#BIN=/usr//sbin/$BASENAME
BIN=/usr/local/sbin/$BASENAME
</pre>

## 4. Haproxy配置文件介绍
HAProxy的配置参数的来源主要有如下3种：

* 通过命令行参数 （具有最高优先级，会覆盖配置文件中的配置）

* 通过global section，用于设置进程级别的参数

* proxy section可以通过如下关键字来指定： ```defaults```，```listen```，```frontend```，```backend```

配置文件的语法为： 每行以一个关键字开始，后接一个或多个参数，参数与参数之间以空格分割。注释行以```#```开头。HAProxy配置文件根据功能和用途，主要有5个部分组成，但有些部分并不是必须的，可以根据需要选择相应的部分进行配置：

1) **global部分**

用来设定全局配置参数，属于进程级别的配置，通常和操作系统配置有关。

2） **defaults部分**

默认参数的配置部分。在此部分设置的参数值，默认会自动引用到下面的```frontend```、```backend```、```listen```部分中。因此，如果有些参数属于公用的配置，只需在defaults部分添加一次即可。而如果在```frontend```、```backend```或```listen```部分也配置了与defaults部分一样的参数，那么defaults部分参数对应的值将自动被覆盖。

3） **frontend部分**

此部分用于设置接收用户请求的前端虚拟节点。frontend是在HAProxy1.3版本之后才引入的一个组件，同时引入的还有backend组件。通过引入这些组件，在很大程度上简化了HAProxy配置文件的复杂性。frontend可以根据ACL规则直接指定要使用的后端。

4） **backend部分**

此部分用于设置后端服务器集群，也就是用来添加一组真实服务器，以处理前端用户的请求。添加的真实服务器类似于LVS的real server节点。

5) **listen部分**

此部分是```frontend```部分和```backend```部分的结合体。在HAProxy1.3版本之前，HAProxy的所有配置选项都在这个部分中设置。为了保持兼容性，HAProxy新的版本仍然保留了```listen```组件的配置方式。目前在HAProxy中，两种配置方式任选其一即可。

### 3.1 HAProxy配置示例
{% highlight string %}
global
	log 127.0.0.1 local0 info 
	maxconn 4096
	user nobody 
	group nobody 
	daemon 
	nbproc 1
	pidfile /usr/local/haproxy/logs/haproxy.pid 
defaults
	mode http 
	retries 3
	timeout connect 10s 
	timeout client 20s 
	timeout server 30s 
	timeout check 5s
frontend www
	bind *:80 
	mode	http
	option	httplog 
	option	forwardfor 
	option	httpclose 
	log	global
	default_backend htmpool
backend htmpool
	mode	http 
	option	redispatch
	option	abortonclose 
	balance	roundrobin 
	cookie	SERVERID
	option	httpchk GET /index.php
	server	web1 10.200.34.181:80	cookie server1 weight 6 check inter 2000 rise 2 fall 3
	server	web2 10.200.34.182:8080 cookie server2 weight 6 check inter 2000 rise 2 fall 3
listen admin_stats
	bind 0.0.0.0:9188
	mode http
	log 127.0.0.1 
	local0 err stats 
	refresh 30s
	stats uri /haproxy-status
	stats realm welcome login\ Haproxy
	stats auth admin:admin123
	stats hide-version 
	stats admin if TRUE
{% endhighlight %}
1) **global配置**

* log: 全局日志配置， local0是日志设备，info表示日志级别。其中日志级别有err、warning、info、debug四种可选。这个配置表示使用127.0.0.1上的```rsyslog```服务中的local0日志设备，记录日志等级为info。

* maxconn: 设定每个haproxy进程的最大并发连接数，

* user/group: 设置运行haproxy进程的用户和组，也可以使用```用户```和```组```的uid和gid值类替代。

* daemon: 设置HAProxy进程进入后台运行。这是推荐的运行模式

* nbproc: 设置HAProxy启动时刻创建的进程数，此参数要求将HAProxy运行模式设置为```daemon```。默认情况下，只会创建一个进程，这也是我们所推荐的操作模式。对于有一些系统，可能会限制一个进程只能打开少量的文件描述符，此时可能需要创建过个daemons。不推荐使用多进程方式

* pidfile: 指定HAProxy进程的pid文件。启动进程的用户必须具有访问此文件的权限

2） **defaults部分**

* mode: 设置HAProxy实例默认的运行模式，有tcp、http、health三个可选值
<pre>
tcp模式： 在此模式下，客户端和服务器端之间将建立一个全双工的连接，不会对七层报文作做任何类型的检查，默认为tcp模式。经常用于
         SSL、SSH、SMTP等应用

http模式： 在此模式下，客户端请求在转发至后端服务器之前将会被深度分析，所有不与RFC兼容的请求都将会被拒绝。

health模式： 在此模式下，其只会对来自客户端的连接响应'OK'，然后就断开连接。另外，假如设置了'httpchk'选项，将会响应'HTTP/1.0 200 OK'。
            在任何情况下，此种模式都不会进行日志记录。当前本模式已经过时，不再使用。
</pre>

* retries: 设置连接后端服务器的失败重试次数，连接失败的次数如果超过这里设置的值，HAProxy会将对应的后端服务器标记为不可用。此参数也可在后面部分进行设置。

* timeout connect: 设置成功连接到一台服务器的最长等待时间，默认单位是毫秒。但是也可以使用其他的时间单位后缀

* timeout client: 设置连接客户端发送数据时最长等待时间，默认单位是毫秒。但是也可以使用其他的时间单位后缀

* timeout server: 设置服务器端回应客户端数据发送的最长等待时间，默认单位是毫秒。但是也可以使用其他的时间单位后缀

* timeout check: 设置多后端服务器的检测超时时间，默认单位是毫秒。但是也可以使用其他的时间单位后缀

3) **frontend部分**

* bind: 此选项只能在frontend和listen部分进行定义，用于定义一个或多个监听的套接字。bind的语法格式为：
{% highlight string %}
bind [<address>]:<port_range> [, ...] [param*]
bind /<path> [, ...] [param*]
{% endhighlight %}
其中address为可选选项，其可以为主机名或IP地址，如果将其设置为```*```或者```0.0.0.0```，则将监听当前系统的所有IPv4地址；port_range可以是一个特定的TCP端口，也可以是一个端口范围，小于1024的端口需要有特定权限的用户才能使用。

* option httplog: 在默认情况下，haproxy是不记录HTTP请求的，这样很不方便HAProxy问题的排查与监控。通过此选项可以启用日志记录HTTP请求。

* option forwardfor: 如果后端服务器需要获得客户端的真实IP，就需要配置此参数。由于HAProxy工作于反向代理模式，因此发往后端真实服务器的请求中客户端IP均为HAProxy主机的IP，而非真正的客户端地址，这就导致真实服务器无法记录客户端真正请求来源的IP，而```X-Forwarded-For```则可以用于解决此问题。通过使用```forwardfor```选项，HAProxy就可以向每个发往后端真实服务器的请求添加```X-Forwarded-For```记录，这样后端真实服务器日志可以通过```X-Forward-For```信息来记录客户端来源IP。

* option httpclose: 此选项表示在客户端和服务器完成一次连接请求后，HAProxy将主动关闭http连接。这是对性能非常有帮助的一个参数。

* log global: 表示使用全局的日志配置。这里的```global```表示引用HAProxy配置文件global部分中定义的log选项配置格式。

* default_backend: 指定默认的后端服务器池，也就是指定一组后端真实服务器，而这些真实服务器组将在backend段进行定义。这里的```htmlpool```就是一个后端服务器组。

4） **backend部分**

* option redispatch： 此参数用于cookie保持的环境中。在默认情况下，HAProxy会将其请求的后端服务器的ServerID插入到cookie中，以保证会话的Session持久性。而如果后端服务器出现故障，客户端的cookie是不会刷新的，这就出现了问题。此时，如果设置此参数，就会将客户端的请求强制定向到另外一个健康的后端服务器上，以保证服务的正常。

* option abortonclose: 如果设置了此参数，可以在服务器负载很高的情况下，自动结束掉当前队列中处理时间比较长的连接。

* balance: 此关键字用来定义负载均衡算法

* cookie: 表示允许向cookie插入```SERVERID```，每台服务器的```SERVERID```可在下面的server关键字中使用cookie关键字定义

* option httpchk: 此选项表示启用HTTP的服务状态检测功能。HAProxy作为一款专业的负载均衡器，它支持对backend部分指定的后端服务节点进行健康检查，以保证在后端backend中某个节点不能服务时，把从frontend端进来的客户端请求分配至backend中其他健康节点上，从而保证整体服务的可用性。```option httpchk```的语法格式如下：
{% highlight string %}
option httpchk <method> <uri> <version>

其中：
method  --   表示HTTP请求的方式，常用的有OPTIONS、GET、HEAD几种方式。一般的健康检查可以采用HEAD方式进行，而不是采用GET方式，这是因为HEAD
             方式没有数据返回，仅检查Response的HEAD是不是200状态。因此，相对于GET来说，HEAD方式更快更简单

uri     --   表示叫检测的url地址，通过执行此url，可以获取后端服务器的运行状态。在正常情况下将返回状态码200，返回其他状态码均为异常状态。

version --   指定心跳检测时的HTTP版本号
{% endhighlight %}

* server: 这个关键字用来定义多个后端真实服务器，不能用于defaults和frontend部分。使用格式为：
{% highlight string %}
server <name> <address>[:port] [param*] 

例： 
server	web1 10.200.34.181:80	cookie server1 weight 6 check inter 2000 rise 2 fall 3
{% endhighlight %}
```[param*]```为是server的一系列参数,下面对上面示例中server的各项参数进行简单说明：
<pre>
check   --  表示启用对后端服务器执行健康检查
inter   --  设置监看检查的时间间隔，单位为毫秒
rise    --  设置从故障状态转换至正常状态需要成功检查的次数。例如： 'rise 2'表示2次检查成功就认为此服务器可用
fall    --  设置从正常状态转换为不可用状态需要检查的次数。例如： 'fall 3'表示3次检查失败就认为此服务器不可用
cookie  --  为指定的后端服务器设定cookie值，此处指定的值将在请求入站时被检查，第一次为此值挑选的后端服务器将在后面被选中
</pre>

5) **listen部分**

这个部分通过listen关键字定义了一个名为```admin_stats```的实例，其实就是定义了一个 HAProxy 的监控页面，每个选项的含义如下：

* stats refresh：设置 HAProxy 监控统计页面自动刷新的时间。

* stats uri：设置 HAProxy 监控统计页面的URL 路径，可随意指定。例如：指定*stats uri /haproxy-status*，就可以过http://IP:9188/haproxy-status 查看。

* stats realm：设置登录 HAProxy 统计页面时密码框上的文本提示信息。

* stats auth：设置登录 HAProxy 统计页面的用户名和密码。用户名和密码通过冒号分割。可为监控页面设置多个用户名和密码，每行一个。

* stats hide-version：用来隐藏统计页面上 HAProxy 的版本信息。

* stats admin if TRUE：通过设置此选项，可以在监控页面上手工启用或禁用后端真实服务器，仅在 haproxy1.4.9 以后版本有效。

## 5. HAProxy使用示例

如下我们给出一个HAProxy的使用实例： 使用Haproxy作为负载均衡器访问后端的Web服务。当前我们的示例环境如下：
<pre>
haproxy主机：          192.168.79.128  

http服务器器1(nginx):  192.168.79.129
http服务器器2(nginx):  192.168.79.131
</pre>
在进行具体工作之前，我们最好先关闭```SELinux```：
<pre>
# setenforce 0
setenforce: SELinux is disabled
</pre>

1) **修改配置文件**

修改默认配置文件*/etc/haproxy/haproxy.cfg*:
{% highlight string %}
#全局配置
global
	#设置日志
	log 127.0.0.1 local3 info
	#用户与用户组
	user nobody
	group nobody
	#守护进程启动
	daemon
	#最大连接数
	maxconn 4000
	pidfile /var/run/haproxy.pid

#默认配置
defaults
	log global
	mode http
	option httplog
	option dontlognull
	timeout connect 5000
	timeout client 50000
	timeout server 50000

#前端配置，http_front名称可自定义
frontend http_front
	#发起http请求到10080端口，会被转发到设置的ip及端口
	bind *:10080
	#haproxy的状态管理页面，通过/haproxy?stats来访问
	stats uri /haproxy?stats
	default_backend http_back

#后端配置，http_back名称可自定义
backend http_back
	#负载均衡方式
	#source 根据请求源IP
	#static-rr 根据权重
	#leastconn 最少连接者先处理
	#uri 根据请求的uri
	#url_param 根据请求的url参数
	#rdp-cookie 据据cookie(name)来锁定并哈希每一次请求
	#hdr(name) 根据HTTP请求头来锁定每一次HTTP请求
	#roundrobin 轮询方式
	balance roundrobin
	#设置健康检查页面
	option httpchk GET /index.html
	#传递客户端真实IP
	option forwardfor header X-Forwarded-For
	# inter 2000 健康检查时间间隔2秒
	# rise 3 检测多少次才认为是正常的
	# fall 3 失败多少次才认为是不可用的
	# weight 30 权重
	# 需要转发的ip及端口
	server web1 192.168.79.129:80 check inter 2000 rise 3 fall 3 weight 30
	server web2 192.168.79.131:80 check inter 2000 rise 3 fall 3 weight 30
{% endhighlight %}
注意这里我们采用```nobody/nobody```来启动HAProxy，如果要采用其他用户/组来启动，可能需要先创建。

2) **配置HAProxy日志**

HAProxy默认是不记录日志的，需要借助```rsyslog```来记录。这里有两种方法：

* 方法1

直接在*/etc/rsyslog.conf*文件中配置：
{% highlight string %}
# 对如下两行取消注释
$ModLoad imudp
$UDPServerRun 514

# 在末尾添加如下行
local3.* /var/log/haproxy.log
{% endhighlight %}
此外，为了使haproxy的日志不会记录到*/var/log/messages*中，我们还需要找到*/etc/rsyslog.conf*中的如下行，然后在其末尾加上```local3.none```:
{% highlight string %}
*.info;mail.none;authpriv.none;cron.none;local3.none     /var/log/messages
{% endhighlight %}

最后执行如下的命令重启rsyslog服务：
<pre>
# systemctl restart rsyslog
</pre>

* 方法2

因为我们从*/etc/rsyslog.conf*中我们可以看到rsyslog会读取*/etc/rsyslog.d/*目录下所有以```.conf```结尾的配置文件，因此我们在*/etc/rsyslog.d*目录下添加```haproxy_log.conf```配置文件：
{% highlight string %}
$ModLoad imudp
$UDPServerRun 514
local3.* /var/log/haproxy.log
{% endhighlight %}
同样与上面```方法1```相似，我们也要避免haproxy被记录到*/var/log/messages*中，因此需要加上```local3.none```。最后执行如下命令重启rsyslog服务：
<pre>
# systemctl restart rsyslog
</pre>

在这里我们采用```方法2```来进行配置。
 

3) **启动相关服务器**

在*192.168.79.129*与*192.168.79.131*服务器上启动nginx服务。在*192.168.79.128*主机上通过如下命令启动HAProxy:
<pre>
# service haproxy start
Starting haproxy (via systemctl):  [  OK  ]
# ps -ef | grep haproxy
nobody   123057      1  0 04:46 ?        00:00:00 /usr/local/sbin/haproxy -D -f /etc/haproxy/haproxy.cfg -p /var/run/haproxy.pid
</pre>

第一次启动haproxy先使用```service```命令，以后就可以使用systemctl命令来操作了：
<pre>
# systemctl daemon-reload
# systemctl restart haproxy
</pre>

4) **测试haproxy服务**

我们可以通过Web浏览器请求*http://192.168.79.128:10080/*，可以看到能够正常请求成功。另外，可以查看*/var/log/haproxy.log*文件看相应的请求日志。

## 6. Session绑定
HAProxy可以有两种方法保持客户端```Session```一致，这里我们简单说明一下：

1） **用户IP识别**

haroxy 将用户 IP 经过 hash 计算后 指定到固定的真实服务器上（类似于 nginx 的 IP hash 指令）:
<pre>
backend htmpool
    mode http 
    option redispatch
    option abortonclose 
    balance source 
    cookie SERVERID
    option httpchk GET /index.jsp
    server web1 192.168.81.237:8080 cookie server1 weight 6 check inter 2000 rise 2 fall 3
    server web2 192.168.81.234:8080 cookie server2 weight 3 check inter 2000 rise 2 fall 3
</pre>
上面我们通过```balance source```配置指令实现。

2) **cookie识别**

haproxy 将WEB 服务端发送给客户端的 cookie 中插入到haproxy 定义的后端的服务器COOKIE ID：
{% highlight string %}
backend htmpool
    mode http 
    option	redispatch
    option	abortonclose 
    balance  static-rr 
    cookie	SESSION_COOKIE insert indirect nocache   #cookie参数
    option	httpchk GET /index.jsp
    server	web1 192.168.81.237:8080 cookie server1 weight 6 check inter 2000 rise 2 fall 3   #server里面的cookie参数
    server	web2 192.168.81.234:8080 cookie server2 weight 3 check inter 2000 rise 2 fall 3   #server里面的cookie参数
{% endhighlight %}
上面我们通过*cookie	SESSION_COOKIE insert indirect nocache*配置指令实现



<br />
<br />

**[参看]**

1. [HAProxy官网](https://www.haproxy.org/)

2. [HAProxy Ducumentation](https://cbonte.github.io/haproxy-dconv/1.8/intro.html)

3. [haproxy安装配置](https://blog.csdn.net/qq_28710983/article/details/82194404)

4. [HAProxy 入门使用、安装](https://blog.csdn.net/u011186019/article/details/79998653)

<br />
<br />
<br />


