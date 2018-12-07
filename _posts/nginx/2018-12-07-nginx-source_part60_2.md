---
layout: post
title: core/ngx_shmtx.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx中进程之间互斥锁的实现。


<!-- more -->







<br />
<br />

**[参看]**

1. [nginx源码分析1———进程间的通信机制一（信号量）](https://blog.csdn.net/sina_yangyang/article/details/47011303)

2. [Nginx的锁的实现以及惊群的避免](https://www.cnblogs.com/549294286/p/6058811.html)

3. [Nginx源代码分析之锁的实现（十八）](https://blog.csdn.net/namelcx/article/details/52447027)

4. [Nginx---进程锁的实现](https://blog.csdn.net/bytxl/article/details/24580801)

5. [linux进程锁](https://blog.csdn.net/zyembed/article/details/79884211)

6. [进程间同步（进程间互斥锁、文件锁）](https://blog.csdn.net/qq_35396127/article/details/78942245?utm_source=blogxgwz9)

7. [nginx之共享内存](https://blog.csdn.net/evsqiezi/article/details/51785093)

<br />
<br />
<br />

