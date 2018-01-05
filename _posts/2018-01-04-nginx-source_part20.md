---
layout: post
title: os/unix/ngx_setaffinity.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要介绍一下nginx中，进程cpu亲和性设置相关的实现。


<!-- more -->


## 1. os/unix/ngx_setaffinity.h头文件
{% highlight string %}

/*
 * Copyright (C) Nginx, Inc.
 */

#ifndef _NGX_SETAFFINITY_H_INCLUDED_
#define _NGX_SETAFFINITY_H_INCLUDED_


#if (NGX_HAVE_SCHED_SETAFFINITY || NGX_HAVE_CPUSET_SETAFFINITY)

#define NGX_HAVE_CPU_AFFINITY 1

#if (NGX_HAVE_SCHED_SETAFFINITY)

typedef cpu_set_t  ngx_cpuset_t;

#elif (NGX_HAVE_CPUSET_SETAFFINITY)

#include <sys/cpuset.h>

typedef cpuset_t  ngx_cpuset_t;

#endif

void ngx_setaffinity(ngx_cpuset_t *cpu_affinity, ngx_log_t *log);

#else

#define ngx_setaffinity(cpu_affinity, log)

typedef uint64_t  ngx_cpuset_t;

#endif


#endif /* _NGX_SETAFFINITY_H_INCLUDED_ */
{% endhighlight %}
当前我们在ngx_auto_config.h头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_SCHED_SETAFFINITY
#define NGX_HAVE_SCHED_SETAFFINITY  1
#endif
</pre>
因此这里我们实际执行的是：
{% highlight string %}

#ifndef _NGX_SETAFFINITY_H_INCLUDED_
#define _NGX_SETAFFINITY_H_INCLUDED_

#define NGX_HAVE_CPU_AFFINITY 1

typedef cpu_set_t  ngx_cpuset_t;

void ngx_setaffinity(ngx_cpuset_t *cpu_affinity, ngx_log_t *log);
{% endhighlight %}


## 2. os/unix/ngx_setaffinity.c源文件
{% highlight string %}

/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_CPUSET_SETAFFINITY)

void
ngx_setaffinity(ngx_cpuset_t *cpu_affinity, ngx_log_t *log)
{
    ngx_uint_t  i;

    for (i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, cpu_affinity)) {
            ngx_log_error(NGX_LOG_NOTICE, log, 0,
                          "cpuset_setaffinity(): using cpu #%ui", i);
        }
    }

    if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, -1,
                           sizeof(cpuset_t), cpu_affinity) == -1)
    {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      "cpuset_setaffinity() failed");
    }
}

#elif (NGX_HAVE_SCHED_SETAFFINITY)

void
ngx_setaffinity(ngx_cpuset_t *cpu_affinity, ngx_log_t *log)
{
    ngx_uint_t  i;

    for (i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, cpu_affinity)) {
            ngx_log_error(NGX_LOG_NOTICE, log, 0,
                          "sched_setaffinity(): using cpu #%ui", i);
        }
    }

    if (sched_setaffinity(0, sizeof(cpu_set_t), cpu_affinity) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      "sched_setaffinity() failed");
    }
}

#endif
{% endhighlight %}

当前我们有定义```NGX_HAVE_SCHED_SETAFFINITY```宏，因此这里我们使用的是ngx_setaffinity()的后一个实现。程序较为简单，首先会打印一下当前进程绑定的CPU集合：
{% highlight string %}
for (i = 0; i < CPU_SETSIZE; i++) {
    if (CPU_ISSET(i, cpu_affinity)) {
        ngx_log_error(NGX_LOG_NOTICE, log, 0,
                      "sched_setaffinity(): using cpu #%ui", i);
    }
}
{% endhighlight %}

然后再调用sched_setaffinity()函数设置进程的亲和性。下面我们介绍一下亲和性方面的一些内容：

### 2.1 设置和获取一个线程的亲和性
{% highlight string %}
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>

int sched_setaffinity(pid_t pid, size_t cpusetsize,
                     const cpu_set_t *mask);

int sched_getaffinity(pid_t pid, size_t cpusetsize,
                     cpu_set_t *mask);
{% endhighlight %}

一个线程的CPU亲和性掩码决定了该线程跑在哪个（些）CPU上比较合适。在一个多核系统上，设置CPU亲和性掩码可以获得较好的性能。例如，将一个CPU单独贡献给一个线程（其他线程绑定到其他CPU上），这样就能够保证该线程获得最大的执行速度。限制线程运行在一个单独的CPU上可以避免线程因切换CPU导致的cache失效而产生的性能损失。

一个CPU的亲和性掩码由一个cpu_set_t结构来表示。

* **sched_setaffinity()**: 设置指定线程(pid)的cpu亲和性掩码为mask。假若pid为0，则表明是设置当前调用线程。参数cpusetsize用于指定mask指向的数据结构的长度，通常这个参数可取值为sizeof(cpu_set_t)。

* **sched_getaffinity()**: 用于获取指定线程的cpu亲和性掩码。假若pid为0，则表明是获取当前调用线程的cpu亲和性。

### 2.2 相关辅助宏定义
{% highlight string %}
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>

//1: 清空集合
void CPU_ZERO(cpu_set_t *set);    

//2: 添加一个CPU到set集合中
void CPU_SET(int cpu, cpu_set_t *set);

//3: 从set集合中移除一个CPU
void CPU_CLR(int cpu, cpu_set_t *set);

//4: 用于检测一个CPU是否在set集合中
int  CPU_ISSET(int cpu, cpu_set_t *set);

//5: 返回set集合中CPU的个数
int  CPU_COUNT(cpu_set_t *set);

//6: 通过destset返回srcset1与srcset2的交集
void CPU_AND(cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);

//7: 通过destset返回srcset1与srcset2的并集
void CPU_OR(cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);

//8: 通过destsct返回在srcset1或srcset2中的CPU集合（但是不能同时在srcset1和srcset2中)
void CPU_XOR(cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);

//9: 用于判断两个集合是否相等
int  CPU_EQUAL(cpu_set_t *set1, cpu_set_t *set2);



//10: 分配一个足够大的cpuset集合，以使能够容纳num_cpus个cpu
cpu_set_t *CPU_ALLOC(int num_cpus);

//11: 释放CPU_ALLOC()分配的空间
void CPU_FREE(cpu_set_t *set);

//12: 计算容纳num_cpus个CPU所需要的空间大小（字节)
size_t CPU_ALLOC_SIZE(int num_cpus);

//13: 如下函数与上面类似，只不过其是用于动态分配(CPU_ALLOC())的cpuset
void CPU_ZERO_S(size_t setsize, cpu_set_t *set);

void CPU_SET_S(int cpu, size_t setsize, cpu_set_t *set);
void CPU_CLR_S(int cpu, size_t setsize, cpu_set_t *set);
int  CPU_ISSET_S(int cpu, size_t setsize, cpu_set_t *set);

int  CPU_COUNT_S(size_t setsize, cpu_set_t *set);

void CPU_AND_S(size_t setsize, cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);
void CPU_OR_S(size_t setsize, cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);
void CPU_XOR_S(size_t setsize, cpu_set_t *destset,
            cpu_set_t *srcset1, cpu_set_t *srcset2);

int  CPU_EQUAL_S(size_t setsize, cpu_set_t *set1, cpu_set_t *set2);
{% endhighlight %}

另外有一个常量```CPU_SETSIZE```，当前一般被定义为1024。大部分情况下，其已经远远超过了我们当前电脑CPU的个数了。

### 2.3 示例

**1) 示例1**

程序test.c:
{% highlight string %}
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

int main(int argc,char *argv[])
{
     int size;
     cpu_set_t st;
     unsigned char *pt;
     int i;

     size = CPU_ALLOC_SIZE(0);
     printf("cpu count: %d , alloc size: %d\n",0,size);

     size = CPU_ALLOC_SIZE(1);
     printf("cpu count: %d , alloc size: %d\n",1,size);

     size = CPU_ALLOC_SIZE(10);
     printf("cpu count: %d , alloc size: %d\n",10,size);
    
     size = CPU_ALLOC_SIZE(100);
     printf("cpu count: %d , alloc size: %d\n",100,size);

     size = CPU_ALLOC_SIZE(1000);
     printf("cpu count: %d , alloc size: %d\n",1000,size);

     size = CPU_ALLOC_SIZE(1023);
     printf("cpu count: %d , alloc size: %d\n",1023,size);

     size = CPU_ALLOC_SIZE(1024);
     printf("cpu count: %d , alloc size: %d\n",1024,size);

     size = CPU_ALLOC_SIZE(1025);
     printf("cpu count: %d , alloc size: %d\n",1025,size);

     printf("sizeof(cpu_set_t): %d\n",sizeof(cpu_set_t));
     CPU_ZERO(&st);
     
     CPU_SET(0,&st);
     CPU_SET(1,&st);
     CPU_SET(7,&st);
     CPU_SET(15,&st);

     for(i = 0, pt = (unsigned char *)&st;i<sizeof(cpu_set_t);i++)
     {
         printf("%02x%s%s",*(pt+i),(i+1)%8==0?" ":"", (i+1)%16==0?"\n":" ");
     }
     return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
cpu count: 0 , alloc size: 0
cpu count: 1 , alloc size: 8
cpu count: 10 , alloc size: 8
cpu count: 100 , alloc size: 16
cpu count: 1000 , alloc size: 128
cpu count: 1023 , alloc size: 128
cpu count: 1024 , alloc size: 128
cpu count: 1025 , alloc size: 136
sizeof(cpu_set_t): 128
83 80 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
</pre>


**2) 示例2**

程序test2.c:
{% highlight string %}
#define _GNU_SOURCE  
#include <stdio.h>  
#include <stdlib.h>  
#include <sched.h>  
#include <unistd.h>  
#include <sys/times.h>  
  
void set_cpu(int id);  
  
int main(void)  
{  
    puts("cpu affinity set");  
    set_cpu(0x0003);  
  
    while(1)  
    {  
        //sleep(10);  
        //printf("in the while\n");  
    }  
  
    return 1;  
}  
  
// setup the cpu set of this program to run on  
void set_cpu(int id)  
{  
    cpu_set_t mask;  
    CPU_ZERO(&mask);  
    CPU_SET(id, &mask);  
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1)  
    {  
        fprintf(stderr, "warning: could not set CPU affinity/n");  
    }  
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test2 test2.c
[root@localhost test-src]# ./test2
cpu affinity set



在另外一个窗口执行top命令：
[root@localhost test-src]# top            //按下数字键1，查看每个逻辑CPU使用率
top - 05:13:28 up  3:32,  3 users,  load average: 0.95, 0.59, 0.29
Tasks: 163 total,   2 running, 161 sleeping,   0 stopped,   0 zombie
%Cpu0  :  0.0 us,  0.3 sy,  0.0 ni, 99.7 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
%Cpu1  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
%Cpu2  :  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
%Cpu3  :100.0 us,  0.0 sy,  0.0 ni,  0.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem : 10058704 total,  9185284 free,   397576 used,   475844 buff/cache
KiB Swap:  5112828 total,  5112828 free,        0 used.  9328352 avail Mem 

   PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                                                                               
 10497 root      20   0    4160    340    272 R 100.0  0.0   1:11.27 test2                                                                                                 
   312 root      20   0       0      0      0 S   0.3  0.0   0:03.65 xfsaild/sda3                                                                                          
  8864 root      20   0       0      0      0 S   0.3  0.0   0:01.35 kworker/0:0                                                                                           
 10498 root      20   0  157724   2292   1556 R   0.3  0.0   0:00.06 top                                                                                                   
     1 root      20   0  128096   6704   3952 S   0.0  0.1   0:03.95 systemd                                                                                               
     2 root      20   0       0      0      0 S   0.0  0.0   0:00.01 kthreadd                                                                                              
     3 root      20   0       0      0      0 S   0.0  0.0   0:00.02 ksoftirqd/0                                                                                           
     5 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/0:0H                                                                                          
     6 root      20   0       0      0      0 S   0.0  0.0   0:00.36 kworker/u256:0                                                                                        
     7 root      rt   0       0      0      0 S   0.0  0.0   0:00.08 migration/0                                                                                           
     8 root      20   0       0      0      0 S   0.0  0.0   0:00.00 rcu_bh                                                                                                
     9 root      20   0       0      0      0 S   0.0  0.0   0:02.35 rcu_sched                                                                                             
    10 root      rt   0       0      0      0 S   0.0  0.0   0:00.04 watchdog/0                                                                                            
    11 root      rt   0       0      0      0 S   0.0  0.0   0:00.03 watchdog/1                                                                                            
    12 root      rt   0       0      0      0 S   0.0  0.0   0:00.12 migration/1                                                                                           
    13 root      20   0       0      0      0 S   0.0  0.0   0:00.06 ksoftirqd/1 
</pre>
我们可以看到test2进程被绑定到cpu3上，cpu3的使用率为100%。

<br />
<br />
**[参看]:**

1. [线程绑定CPU核-sched_setaffinity](http://blog.csdn.net/lanyzh0909/article/details/50404664)

2. [c语言设置cpu affinity](http://blog.csdn.net/watkinsong/article/details/28418911)


<br />
<br />
<br />

