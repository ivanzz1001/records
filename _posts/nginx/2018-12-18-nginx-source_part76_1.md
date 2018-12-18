---
layout: post
title: event/modules/ngx_epoll_module.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下nginx epoll模块的实现。


<!-- more -->





<br />
<br />

**[参看]**

1. [nginx events](https://nginx.org/en/docs/dev/development_guide.html#events)

2. [nginx event 模块解析](https://blog.csdn.net/jackywgw/article/details/48676643)

3. [Nginx学习笔记(十八)：事件处理框架](https://blog.csdn.net/fzy0201/article/details/23171207)

4. [事件和连接](https://blog.csdn.net/nestler/article/details/37570401)

5. [eventfd 的分析与具体例子](https://blog.csdn.net/tanswer_/article/details/79008322)

6. [EVENTFD](http://man7.org/linux/man-pages/man2/eventfd.2.html)

7. [事件模块（二）ngx_epoll_module详解](https://blog.csdn.net/ws891033655/article/details/25643465)

8. [Linux下的I/O复用与epoll详解](https://www.cnblogs.com/lojunren/p/3856290.html)

<br />
<br />
<br />

