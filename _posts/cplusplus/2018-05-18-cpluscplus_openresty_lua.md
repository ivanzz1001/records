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

如下我们列出nginx lua相关的一些指令：
<pre>
lua_load_resty_core   lua_capture_error_log    lua_use_default_type
lua_malloc_trim
lua_code_cache
lua_regex_cache_max_entries
lua_regex_match_limit
lua_package_path
lua_package_cpath
init_by_lua
init_by_lua_block
init_by_lua_file
init_worker_by_lua
init_worker_by_lua_block
init_worker_by_lua_file
set_by_lua
set_by_lua_block
set_by_lua_file
content_by_lua
content_by_lua_block
content_by_lua_file
rewrite_by_lua
rewrite_by_lua_block
rewrite_by_lua_file
access_by_lua
access_by_lua_block
access_by_lua_file
header_filter_by_lua
header_filter_by_lua_block
header_filter_by_lua_file
body_filter_by_lua
body_filter_by_lua_block
body_filter_by_lua_file
log_by_lua
log_by_lua_block
log_by_lua_file
balancer_by_lua_block
balancer_by_lua_file
lua_need_request_body
ssl_certificate_by_lua_block
ssl_certificate_by_lua_file
ssl_session_fetch_by_lua_block
ssl_session_fetch_by_lua_file
ssl_session_store_by_lua_block
ssl_session_store_by_lua_file
lua_shared_dict
lua_socket_connect_timeout
lua_socket_send_timeout
lua_socket_send_lowat
lua_socket_read_timeout
lua_socket_buffer_size
lua_socket_pool_size
lua_socket_keepalive_timeout
lua_socket_log_errors
lua_ssl_ciphers
lua_ssl_crl
lua_ssl_protocols
lua_ssl_trusted_certificate
lua_ssl_verify_depth
lua_http10_buffering
rewrite_by_lua_no_postpone
access_by_lua_no_postpone
lua_transform_underscores_in_response_headers
lua_check_client_abort
lua_max_pending_timers
lua_max_running_timers
lua_sa_restart
</pre>


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




<br />
<br />
<br />





