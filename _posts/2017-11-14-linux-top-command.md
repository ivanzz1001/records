---
layout: post
title: Linux top命令的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux top命令的使用
---


本文件简单介绍一下Linux环境下top命令的使用。

<!-- more -->

## 1. top命令

top命令输出：
<pre>
# top
top - 14:39:02 up 1 day,  2:46,  4 users,  load average: 0.00, 0.00, 0.00
Tasks: 243 total,   1 running, 242 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.2 us,  0.2 sy,  0.0 ni, 99.4 id,  0.2 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem:   8093596 total,  7201400 used,   892196 free,     1000 buffers
KiB Swap:  8000508 total,      880 used,  7999628 free.  1732676 cached Mem

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                                                                                
 2079 midea     20   0 1598784 124000  71432 S   1.3  1.5   8:59.30 compiz                                                                                                 
    1 root      20   0   33760   4256   2728 S   0.0  0.1   0:01.33 init                                                                                                   
    2 root      20   0       0      0      0 S   0.0  0.0   0:00.02 kthreadd                                                                                               
    3 root      20   0       0      0      0 S   0.0  0.0   0:00.94 ksoftirqd/0                                                                                            
    5 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/0:0H                                                                                           
    7 root      20   0       0      0      0 S   0.0  0.0   0:24.76 rcu_sched                                                                                              
    8 root      20   0       0      0      0 S   0.0  0.0   0:00.00 rcu_bh                                                                                                 
    9 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/0                                                                                            
   10 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/0                                                                                             
   11 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/1                                                                                             
   12 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/1                                                                                            
   13 root      20   0       0      0      0 S   0.0  0.0   0:01.03 ksoftirqd/1                                                                                            
   15 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/1:0H                                                                                           
   16 root      rt   0       0      0      0 S   0.0  0.0   0:00.32 watchdog/2                                                                                             
   17 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/2                                                                                            
   18 root      20   0       0      0      0 S   0.0  0.0   0:00.72 ksoftirqd/2                                                                                            
   20 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/2:0H                                                                                           
   21 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/3                                                                                             
   22 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/3                                                                                            
   23 root      20   0       0      0      0 S   0.0  0.0   0:01.28 ksoftirqd/3                                                                                            
   26 root      20   0       0      0      0 S   0.0  0.0   0:00.00 kdevtmpfs  
</pre>




<br />
<br />







<br />
<br />
<br />


