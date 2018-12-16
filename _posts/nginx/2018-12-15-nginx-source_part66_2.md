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

1. [TM结构体详解](https://blog.csdn.net/csghydiaoke/article/details/8435010)

2. [nginx的时间管理](http://blog.itpub.net/15480802/viewspace-1344713)

3. [nginx + lua环境下时间戳更新不及时的问题](http://songran.net/2016/06/17/nginx-lua%E7%8E%AF%E5%A2%83%E4%B8%8B%E6%97%B6%E9%97%B4%E6%88%B3%E6%9B%B4%E6%96%B0%E4%B8%8D%E5%8F%8A%E6%97%B6%E7%9A%84%E9%97%AE%E9%A2%98/)

4. [深入剖析nginx时间缓存](https://mp.weixin.qq.com/s?__biz=MzIxNzg5ODE0OA%3D%3D&mid=2247483704&idx=1&sn=b23376a56b598f0e461cc3ff4522af3f&chksm=97f38cf3a08405e5ac672cbfd7d56b83398825372b5eaa6570403dbc5928f0f9a461a830a27e)

<br />
<br />
<br />

