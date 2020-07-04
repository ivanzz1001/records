---
layout: post
title: OpenResty lua模块指令
tags:
- cplusplus
categories: cplusplus
description: openresty lua模块指令
---

本章我们介绍一下OpenResty Lua模块的指令，并对其中一些指令的用法进行说明。



<!-- more -->

## 1. lua指令

使用Lua来构建nginx脚本就是通过一条条指令来完成的，指令常用于指定 Lua 代码是什么时候执行的以及如何使用运行的结果。下图展示了指令执行的顺序：

![ngx-lua-order](https://ivanzz1001.github.io/records/assets/img/cplusplus/ngx_lua_order.png)

下面我们再给出Nginx http的11个处理阶段的图，以做比较：

![ngx-proc-phase](https://ivanzz1001.github.io/records/assets/img/cplusplus/ngx_proc_phase.jpg)


如下我们列出nginx lua相关的一些指令：
<pre>
lua_load_resty_core          lua_capture_error_log              lua_use_default_type            lua_malloc_trim                  lua_code_cache
lua_regex_cache_max_entries  lua_regex_match_limit              lua_package_path                lua_package_cpath                init_by_lua   
init_by_lua_block            init_by_lua_file                   init_worker_by_lua              init_worker_by_lua_block         init_worker_by_lua_file
set_by_lua                   set_by_lua_block                   set_by_lua_file                 content_by_lua                   content_by_lua_block
content_by_lua_file          rewrite_by_lua                     rewrite_by_lua_block            rewrite_by_lua_file              access_by_lua                
access_by_lua_block          access_by_lua_file                 header_filter_by_lua            header_filter_by_lua_block       header_filter_by_lua_file
body_filter_by_lua           body_filter_by_lua_block           body_filter_by_lua_file         log_by_lua                       log_by_lua_block
log_by_lua_file              balancer_by_lua_block              balancer_by_lua_file            lua_need_request_body            ssl_certificate_by_lua_block
ssl_certificate_by_lua_file  ssl_session_fetch_by_lua_block     ssl_session_fetch_by_lua_file   ssl_session_store_by_lua_block   ssl_session_store_by_lua_file
lua_shared_dict              lua_socket_connect_timeout         lua_socket_send_timeout         lua_socket_send_lowat            lua_socket_read_timeout
lua_socket_buffer_size       lua_socket_pool_size               lua_socket_keepalive_timeout    lua_socket_log_errors            lua_ssl_ciphers
lua_ssl_crl                  lua_ssl_protocols                  lua_ssl_trusted_certificate     lua_ssl_verify_depth             lua_http10_buffering
rewrite_by_lua_no_postpone   access_by_lua_no_postpone          lua_transform_underscores_in_response_headers
lua_check_client_abort       lua_max_pending_timers             lua_max_running_timers          lua_sa_restart
</pre>


下面我们简单的介绍一下其中一些lua指令：

1) **lua_load_resty_core**
{% highlight string %}
syntax: lua_load_resty_core on|off

default: lua_load_resty_core on

context: http
{% endhighlight %}
自```v0.10.16```版本起该指令就在本模块失效了。当前```resty.core```模块会在Lua VM初始化的时候被强制加载。在当前的版本，使用此指令不会用任何的作用。

2) **lua_capture_error_log**
{% highlight string %}
syntax: lua_capture_error_log size

default: none

context: http
{% endhighlight %}
使用一个指定大小的缓存来捕获所有的nginx错误日志（注： 不仅包括http、http subsystem所产生的错误日志，而是所有），而不是保存到文件或磁盘中。

在设置缓冲区大小时可以使用```k```、```m```等单位，例如：
<pre>
lua_capture_error_log 100k;
</pre>

根据我们的实践经验，一个```4k```的缓存大概可以容纳20条典型的错误日志信息。因此，假如我们需要容纳10000条错误日志，那大概就需要2000K大小的缓存空间。

注意事项：

* 当我们设置好缓存之后，该空间就不会增大了。当缓存满时，新的错误日志就会覆盖缓存中最老的日志条目。

* 缓存的大小必须大于单条错误日志消息的最大长度（openresty中单条错误日志消息的长度最大可以是4K， nginx生产版本单条错误日志消息的最大长度可以是2k)

* 可以通过lua-resty-core库的ngx.errlog模块的get_logs()函数来读取Lua land缓冲区的消息。这个lua API将返回捕获的错误日志消息，并且将读取到的消息从全局捕获缓冲区中移除，从而为任何新的错误日志消息腾出空间。基于这个原因，如果可以保证用户读取缓冲区中的错误日志消息足够快的话，则我们可以不必将缓冲区空间配置的过大。

* error_log指令指定的日志级别将会影响该缓冲区的捕获，该缓冲区仅捕获不低于error_log指令指定的日志级别的消息。用户可以通过Lua API函数errlog.set_filter_level()来动态设置更高的过滤日志级别，这会比静态的error_log指令更灵活。


* 如果没有使用```./configure```选项```--with-debug```来编译openresty或nginx，则无法捕获调试日志。由于高昂的开销，在生产版本中强烈建议禁用调试日志。

3） **lua_code_cache**
{% highlight string %}
syntax: lua_code_cache on | off

default: lua_code_cache on

context: http, server, location, location if
{% endhighlight %}

```启用```或```禁用```对Lua代码的缓存。当启用对Lua代码的缓存时，其可以缓存由```*_by_lua_file```指令(如set_by_lua_file、content_by_lua_file)Lua代码，也可以缓存Lua模块中的脚本代码。

* 从```0.9.3```版本开始，当关闭lua_code_cache时，通过ngx_lua处理的每个请求都将在一个单独的Lua VM实例中运行。因此，由set_by_lua_file、content_by_lua_file、access_by_lua_file等指令所引用的Lua files都不会被缓存，所有使用到的Lua模块都将从头开始加载。通过该指令，开发人员就可以采用edit-and-refresh模式来调试。

* 注意，当我们编辑nginx.conf配置文件中的内联Lua代码(比如set_by_lua、content_by_lua、access_by_lua、rewrite_by_lua指令中的Lua代码）时，运行中的nginx将不能够动态的感知到这些更新，这是因为只有nginx配置文件解析器才能够解析nginx.conf配置文件。因此当我们更改这些内联lua代码时，我们可能必须得重启nginx。

* 即使是启用了代码缓存，由```*_by_lua_file```指令中的```dofile```或```loadfile```所加载的Lua脚本也不能被缓存（除非你自己缓存了结果）。通常我们可以使用init_by_lua或者init_by_lua_file指令来加载所有这些文件，又或者使这些Lua文件成为真正的Lua模块，然后通过require来进行加载。

* ngx_lua模块不支持Apache mod_lua模块可用的stat模式；

* 强烈建议在生产环境中启用Lua代码缓存，在调试环境中如果不会太影响性能的话我们可以不启用。例如，在禁用lua代码缓存后，```hello, world``` Lua示例的性能可能会下降一个数量级。

4） **lua_package_path**
{% highlight string %}
syntax: lua_package_path <lua-style-path-str>

default: The content of LUA_PATH environment variable or Lua's compiled-in defaults.

context: http
{% endhighlight %}
设置```set_by_lua```、```content_by_lua```等指令中搜索Lua模块的路径。这个路径字符串是标准的 lua 路径格式，```;;```常用于表示原始的搜索路径。

从发行版 v0.5.0rc29 开始，可以在搜索路径中使用特殊符号 ```$prefix``` 或 ```${prefix}```来指示服务器前缀的路径，通常在 Nginx 服务器启动时通过```-p PATH```命令行选项来指定server prefix。

5) **lua_package_cpath**
{% highlight string %}
syntax: lua_package_cpath <lua-style-cpath-str>

default: The content of LUA_CPATH environment variable or Lua's compiled-in defaults.

context: http
{% endhighlight %}
设置```set_by_lua```、```content_by_lua```等指令中Lua C模块的搜索路径。这个```cpath```字符串是标准的lua C路径格式，```;;```常用于表示原始的cpath搜索路径。

从发行版 v0.5.0rc29 开始，可以在搜索路径中使用特殊符号 ```$prefix``` 或 ```${prefix}```来指示服务器前缀的路径，通常在 Nginx 服务器启动时通过```-p PATH```命令行选项来指定server prefix。


6） **init_by_lua**
{% highlight string %}
syntax: init_by_lua <lua-script-str>

context: http

phase: loading-config
{% endhighlight %}
>注：注意在 v0.9.17 发行版以后不鼓励使用该指令，应使用init_by_lua_block指令代替

* 当Nginx Master进程(如果有）加载Nginx配置文件的时候，在全局Lua VM级别上运行由参数```<lua-script-str>```指定的Lua代码；

* 当Nginx接收到```HUP```信号并开始重新加载配置文件时，Lua VM将会被重新创建，且init_by_lua也将在新的VM上再次运行。如果lua_code_cache是关闭的（默认打开）， init_by_lua处理程序将在每个请求上都运行一次，因此在这种特殊模式下，总是每个请求创建一个独立的Lua VM。

* 通常，你可以通过该hook在服务启动时预加载```一些```所用到的Lua模块，并充分利用现代操作系统的写时复制(COW)优化。如下是一个预加载Lua modules的例子：
<pre>
 # this runs before forking out nginx worker processes:
 init_by_lua_block { require "cjson" }

 server {
     location = /api {
         content_by_lua_block {
             -- the following require() will just  return
             -- the already loaded module from package.loaded:
             ngx.say(require "cjson".encode{dog = 5, cat = 6})
         }
     }
 }
</pre>

也可以在这个阶段初始化```lua_shared_dict shm```共享内存。如下是一个例子：
<pre>
 lua_shared_dict dogs 1m;

 init_by_lua_block {
     local dogs = ngx.shared.dogs;
     dogs:set("Tom", 56)
 }

 server {
     location = /api {
         content_by_lua_block {
             local dogs = ngx.shared.dogs;
             ngx.say(dogs:get("Tom"))
         }
     }
 }
</pre>
但需要注意的是，lua_shared_dict shm共享内存并不能够通过重新加载配置文件(如发送HUP信号)来来进行清除。因此，假如你并不想在```init_by_lua```代码中重新初始化shm共享内存的话，那么你可以在共享内存中自定义一个flag标志，然后在```init_by_lua```代码中检查该标志即可。

因为该Lua代码在Nginx fork出 worker子进程之前运行，所以在这里加载的数据或代码将享受到操作系统的COW特性，在worker进程创建时并不会真正的复制数据，从而节省大量的内存。

* 不要在此上下文中初始化你自己的Lua全局变量，因为使用Lua全局变量将会造成一定的性能损失，并可能造成全局名称空间(namespace)污染（请参看[Lua Variable Scope](https://github.com/openresty/lua-nginx-module#lua-variable-scope)章节)。我们推荐的做法是使用适当的```Lua module```文件（但不要使用标准Lua函数模块去定义Lua模块，因为它也会污染全局命名空间），同时在 init_by_lua 或其他上下文中调用 require 去加载你自己的模块文件。（require() 会将加载的模块缓存在全局的 Lua 注册表 package.loaded 中，因此你的模块仅在整个 Lua VM 实例中加载一次）


* 在该上下文中，仅支持一小部分Nginx Lua API:

  * 日志API: ngx.log()和print()

  * 共享字典API： ngx.shared.DICT

在未来用户请求中，该上下文将支持更多的 Lua 的 Nginx API。


基本上，你可以在此上下文中安全地使用```阻塞IO```的Lua库，因为在服务器启动期间阻塞master进程完全是可以的。甚至Nginx内核也会在配置加载阶段阻塞（至少在解析 upstream 域名时）。


此外，应该非常小心在此上下文中注册的Lua代码中潜在的安全漏洞，因为Nginx master进程通常在 root 用户下运行。


7） **init_by_lua_block**
{% highlight string %}
syntax: init_by_lua_block { lua-script }

context: http

phase: loading-config
{% endhighlight %}
与```init_by_lua```指令类似，不同之处在于此指令直接在一对花括号（{}）内部而不是在 Nginx 字符串（需要特殊字符串转义）中内联 lua 代码。例如：
<pre>
init_by_lua_block {
     print("I need no extra escaping here, for example: \r\nblah")
 }
</pre>

8） **init_by_lua_file**
{% highlight string %}
syntax: init_by_lua_file <path-to-lua-script-file>

context: http

phase: loading-config
{% endhighlight %}
与```init_by_lua```等价，除了这里是通过path-to-lua-script-file来指定所要执行的Lua脚本外。

注：当我们使用类似于```foo/bar.lua```这样的相对路径时，会转换成相对于```server prefix```所指定的绝对路径。我们可以在Nginx启动的时候通过```-p Path```来指定server prefix path。

9) **init_worker_by_lua**
{% highlight string %}
syntax: init_worker_by_lua <lua-script-str>

context: http

phase: starting-worker
{% endhighlight %}
>注：注意在 v0.9.17 发行版以后不鼓励使用该指令，应使用init_worker_by_lua_block指令代替

当启用了Nginx master进程时，则通过```init_worker_by_lua```所指定的Lua代码会在每个Nginx worker进程启动时执行； 而如果禁用了Nginx master，那么此hook将会在```init_by_lua*```指令之后执行。

本hook常用于创建每个worker重复发生的计时器（通过ngx.timer.at Lua API），用于后端运行状况检查或其他定时工作。参看如下示例：
<pre>
 init_worker_by_lua '
     local delay = 3  -- in seconds
     local new_timer = ngx.timer.at
     local log = ngx.log
     local ERR = ngx.ERR
     local check

     check = function(premature)
         if not premature then
             -- do the health check or other routine work
             local ok, err = new_timer(delay, check)
             if not ok then
                 log(ERR, "failed to create timer: ", err)
                 return
             end
         end

         -- do something in timer
     end

     local hdl, err = new_timer(delay, check)
     if not hdl then
         log(ERR, "failed to create timer: ", err)
         return
     end

     -- other job in init_worker_by_lua
 ';
</pre>
自```v0.10.12```版本以来，该钩子不再在 cache manager 和 cache loader 进程中运行。

10) **init_worker_by_lua_block**
{% highlight string %}
syntax: init_worker_by_lua_block { lua-script }

context: http

phase: starting-worker
{% endhighlight %}
与init_worker_by_lua指令类似，不同之处在于此指令直接在一对花括号（{}）内部而不是在NGINX字符串文字（需要特殊字符转义）中内联Lua代码。例如：
<pre>
init_worker_by_lua_block {
 	print("I need no extra escaping here, for example: \r\nblah")
}
</pre>

自```v0.10.12```版本以来，该钩子不再在 cache manager 和 cache loader 进程中运行

11） **init_worker_by_lua_file**
{% highlight string %}
syntax: init_worker_by_lua_file <lua-file-path>

context: http

phase: starting-worker
{% endhighlight %}
与```init_worker_by_lua```等价，除了这里是通过lua-file-path来指定所要执行的Lua脚本外。

12) **set_by_lua**
{% highlight string %}
syntax: set_by_lua $res <lua-script-str> [$arg1 $arg2 ...]

context: server, server if, location, location if

phase: rewrite
{% endhighlight %}
>注：注意在 v0.9.17 发行版以后不鼓励使用该指令，应使用set_by_lua_block指令代替


13) **content_by_lua**
{% highlight string %}
syntax: content_by_lua <lua-script-str>

context: location, location if

phase: content
{% endhighlight %}
>注：注意在 v0.9.17 发行版以后不鼓励使用该指令，应使用content_by_lua_block指令代替

作为一个```content handler```，其会为每一个请求执行```<lua-script-str>```所指定的Lua 代码。该lua代码可能会进行API调用，并且在独立的全局环境（例如 sandbox）中作为一个新派生的协程执行。

不要在同一个location中同时使用该指令和其他的content handler指令。如，该指令和 proxy_pass指令不应该在同一个 location 中出现。

14) **content_by_lua_block**
{% highlight string %}
syntax: content_by_lua_block { lua-script }

context: location, location if

phase: content
{% endhighlight %}


15) **access_by_lua**
{% highlight string %}
syntax: access_by_lua <lua-script-str>

context: http, server, location, location if

phase: access tail
{% endhighlight %}
>注：注意在 v0.9.17 发行版以后不鼓励使用该指令，应使用access_by_lua_block指令代替

作为一个access阶段的handler，为每一个请求执行```lua-script-str```所指定的Lua代码。该lua代码可能会进行API调用，并且在独立的全局环境（例如 sandbox）中作为一个新派生的协程执行。

* 本handler会在ngx_http_access_module模块之后运行，因此如下代码可以工作正常：
<pre>
 location / {
     deny    192.168.1.1;
     allow   192.168.1.0/24;
     allow   10.1.1.0/16;
     deny    all;

     access_by_lua '
         local res = ngx.location.capture("/mysql", { ... })
         ...
     ';

     # proxy_pass/fastcgi_pass/...
 }
</pre>
上述代码中，假如client IP在blacklist中，那么在执行```access_by_lua```指令查询MySQL进行更复杂的身份认证之前，该请求就会被拒绝掉。


* 注意， ngx_auth_request模块可以近似于采用如下代码来实现：
<pre>
 location / {
     auth_request /auth;

     # proxy_pass/fastcgi_pass/postgres_pass/...
 }
</pre>
采用ngx_lua的实现类似如下：
<pre>
 location / {
     access_by_lua '
         local res = ngx.location.capture("/auth")

         if res.status == ngx.HTTP_OK then
             return
         end

         if res.status == ngx.HTTP_FORBIDDEN then
             ngx.exit(res.status)
         end

         ngx.exit(ngx.HTTP_INTERNAL_SERVER_ERROR)
     ';

     # proxy_pass/fastcgi_pass/postgres_pass/...
 }
</pre>

* 与其他的access phase handlers一样，access_by_lua也不能够运行在subrequests中





<br />
<br />

**[参考]**

1. [OpenResty® Linux 包](http://openresty.org/cn/linux-packages.html)

2. [OpenResty官方网站](http://openresty.org/cn/)

3. [OpenResty installation](http://openresty.org/cn/installation.html)

4. [openresty之指令与常用API](https://www.cnblogs.com/jimodetiantang/p/9257819.html)

5. [OpenResty中LUA指令的执行顺序](https://blog.csdn.net/wwsl123/article/details/103308295)

6. [nginx api for lua](https://github.com/openresty/lua-nginx-module#nginx-api-for-lua)

7. [Nginx Lua 模块指令](https://www.cnblogs.com/babycomeon/p/11109499.html)

8. [Nginx 请求的11个阶段](https://www.cnblogs.com/jackey2015/p/10701489.html)

9. [nginx的十一个阶段处理](https://blog.csdn.net/esion23011/article/details/24057633)



<br />
<br />
<br />





