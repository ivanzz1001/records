---
layout: post
title: event/ngx_event.c源文件(惊群的处理)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx event模块对惊群的处理。


<!-- more -->




<br />
<br />

**[参看]**


1. [Nginx的accept_mutex配置](https://blog.csdn.net/adams_wu/article/details/51669203)

2. [“惊群”，看看nginx是怎么解决它的](https://blog.csdn.net/russell_tao/article/details/7204260)

3. [Nginx的锁的实现以及惊群的避免](http://www.cnblogs.com/549294286/p/6058811.html)

4. [nginx平台初探](http://tengine.taobao.org/book/chapter_2.html)


<br />
<br />
<br />

