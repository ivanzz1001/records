---
layout: post
title: core/ngx_times.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


nginx中很多地方都需要用到时间戳信息，但是如果每一次都直接调用系统函数来获取，虽然可以保证时间的精确性，但是却会严重降低系统的性能。考虑到Nginx中很多地方用到的时间戳并不需要十分精确，从系统性能方面考虑，nginx采用缓存时间戳的方法来处理。


<!-- more -->


## 1. 



<br />
<br />

**[参看]**




<br />
<br />
<br />

