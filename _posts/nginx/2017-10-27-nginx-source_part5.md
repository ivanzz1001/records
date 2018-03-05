---
layout: post
title: core/ngx_cpuinfo源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文主要介绍一下ngx_cpuinfo.


<!-- more -->


## 1. core/ngx_cpuinfo.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (( __i386__ || __amd64__ ) && ( __GNUC__ || __INTEL_COMPILER ))


static ngx_inline void ngx_cpuid(uint32_t i, uint32_t *buf);


#if ( __i386__ )

static ngx_inline void
ngx_cpuid(uint32_t i, uint32_t *buf)
{

    /*
     * we could not use %ebx as output parameter if gcc builds PIC,
     * and we could not save %ebx on stack, because %esp is used,
     * when the -fomit-frame-pointer optimization is specified.
     */

    __asm__ (

    "    mov    %%ebx, %%esi;  "

    "    cpuid;                "
    "    mov    %%eax, (%1);   "
    "    mov    %%ebx, 4(%1);  "
    "    mov    %%edx, 8(%1);  "
    "    mov    %%ecx, 12(%1); "

    "    mov    %%esi, %%ebx;  "

    : : "a" (i), "D" (buf) : "ecx", "edx", "esi", "memory" );
}


#else /* __amd64__ */


static ngx_inline void
ngx_cpuid(uint32_t i, uint32_t *buf)
{
    uint32_t  eax, ebx, ecx, edx;

    __asm__ (

        "cpuid"

    : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (i) );

    buf[0] = eax;
    buf[1] = ebx;
    buf[2] = edx;
    buf[3] = ecx;
}


#endif


/* auto detect the L2 cache line size of modern and widespread CPUs */

void
ngx_cpuinfo(void)
{
    u_char    *vendor;
    uint32_t   vbuf[5], cpu[4], model;

    vbuf[0] = 0;
    vbuf[1] = 0;
    vbuf[2] = 0;
    vbuf[3] = 0;
    vbuf[4] = 0;

    ngx_cpuid(0, vbuf);

    vendor = (u_char *) &vbuf[1];

    if (vbuf[0] == 0) {
        return;
    }

    ngx_cpuid(1, cpu);

    if (ngx_strcmp(vendor, "GenuineIntel") == 0) {

        switch ((cpu[0] & 0xf00) >> 8) {

        /* Pentium */
        case 5:
            ngx_cacheline_size = 32;
            break;

        /* Pentium Pro, II, III */
        case 6:
            ngx_cacheline_size = 32;

            model = ((cpu[0] & 0xf0000) >> 8) | (cpu[0] & 0xf0);

            if (model >= 0xd0) {
                /* Intel Core, Core 2, Atom */
                ngx_cacheline_size = 64;
            }

            break;

        /*
         * Pentium 4, although its cache line size is 64 bytes,
         * it prefetches up to two cache lines during memory read
         */
        case 15:
            ngx_cacheline_size = 128;
            break;
        }

    } else if (ngx_strcmp(vendor, "AuthenticAMD") == 0) {
        ngx_cacheline_size = 64;
    }
}

#else


void
ngx_cpuinfo(void)
{
}


#endif
{% endhighlight %}


## 2. 相应宏测试

在具体分析源代码之前，我们先来对其中的一些宏定义进行测试(test4.c)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* pread(), pwrite(), gethostname() */
#endif


int main(int argc,char *argv[])
{
    #if (__i386__)
      printf(" __i386__\n");
    #endif


    #if (__i386)
      printf(" __i386\n");
    #endif

    #if (__amd64__)
      printf(" __amd64__\n");
    #endif

    #if (__amd64)
      printf(" __amd64\n");
    #endif
   

    #if (__GNUC__)
      printf(" __GNUC__\n");
    #endif

    #if (__INTEL_COMPILER)
      printf(" __INTEL_COMPILER\n");
    #endif

    return 0;

}
{% endhighlight %}
编译运行：
<pre>
root@ubuntu:~/test-src# gcc -o test4 test4.c
root@ubuntu:~/test-src# ./test4
 __i386__
 __i386
 __GNUC__
</pre>
由上我们看到对于我们当前操作系统环境，定义了如下宏：

* __i386__

* __i386

* __GNUC__

## 3. 测试ngx_cpuid()函数

通过上面```相应宏测试```我们再对相应的ngx_cpuid()函数进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* pread(), pwrite(), gethostname() */
#endif

#ifndef ngx_inline
#define ngx_inline      inline
#endif

static ngx_inline void ngx_cpuid(uint32_t i, uint32_t *buf)
{

    /*
     * we could not use %ebx as output parameter if gcc builds PIC,
     * and we could not save %ebx on stack, because %esp is used,
     * when the -fomit-frame-pointer optimization is specified.
     */

    __asm__ (

    "    mov    %%ebx, %%esi;  "

    "    cpuid;                "
    "    mov    %%eax, (%1);   "
    "    mov    %%ebx, 4(%1);  "
    "    mov    %%edx, 8(%1);  "
    "    mov    %%ecx, 12(%1); "

    "    mov    %%esi, %%ebx;  "

    : : "a" (i), "D" (buf) : "ecx", "edx", "esi", "memory" );
}

int main(int argc,char *argv[])
{
   u_char    *vendor;
    uint32_t   vbuf[5], cpu[4], model;

    vbuf[0] = 0;
    vbuf[1] = 0;
    vbuf[2] = 0;
    vbuf[3] = 0;
    vbuf[4] = 0;

    ngx_cpuid(0, vbuf);

    vendor = (u_char *) &vbuf[1];

    if (vbuf[0] == 0) {
        return -1;
    }

    ngx_cpuid(1, cpu);

    printf("vbuf[0]: %u\n",vbuf[0]);
    printf("vendor: %s\n",vendor);
    printf("cpu[0]: %u\n",cpu[0]);
   
    return 0;
}
{% endhighlight %}

编译运行：
<pre>
root@ubuntu:~/test-src# gcc -o test5 test5.c 
root@ubuntu:~/test-src# ./test5
vbuf[0]: 13
vendor: AuthenticAMD
cpu[0]: 6360833
</pre>


## 3. ngx_cpuinfo()函数
{% highlight string %}
/* auto detect the L2 cache line size of modern and widespread CPUs */

void
ngx_cpuinfo(void)
{
    u_char    *vendor;
    uint32_t   vbuf[5], cpu[4], model;

    vbuf[0] = 0;
    vbuf[1] = 0;
    vbuf[2] = 0;
    vbuf[3] = 0;
    vbuf[4] = 0;

    ngx_cpuid(0, vbuf);

    vendor = (u_char *) &vbuf[1];

    if (vbuf[0] == 0) {
        return;
    }

    ngx_cpuid(1, cpu);

    if (ngx_strcmp(vendor, "GenuineIntel") == 0) {

        switch ((cpu[0] & 0xf00) >> 8) {

        /* Pentium */
        case 5:
            ngx_cacheline_size = 32;
            break;

        /* Pentium Pro, II, III */
        case 6:
            ngx_cacheline_size = 32;

            model = ((cpu[0] & 0xf0000) >> 8) | (cpu[0] & 0xf0);

            if (model >= 0xd0) {
                /* Intel Core, Core 2, Atom */
                ngx_cacheline_size = 64;
            }

            break;

        /*
         * Pentium 4, although its cache line size is 64 bytes,
         * it prefetches up to two cache lines during memory read
         */
        case 15:
            ngx_cacheline_size = 128;
            break;
        }

    } else if (ngx_strcmp(vendor, "AuthenticAMD") == 0) {
        ngx_cacheline_size = 64;
    }
}
{% endhighlight %}
由上我们可知，vendor为```AuthenticAMD```, 最后求得ngx_cacheline_size为64。

<br />
<br />

[参看]:

1. [linux C语言调用Intel处理器CPUID指令的实例](http://blog.csdn.net/subfate/article/details/50789905)

2. [Linux下C编程 -- 得到系统的CPU信息(cpuid)](http://blog.chinaunix.net/uid-25808509-id-2467177.html)
 
3. [ngxin源代码完全注释](http://blog.csdn.net/Poechant/article/details/7960215)

4. [进程异常退出导致死锁的解决办法](http://blog.csdn.net/wangzuxi/article/details/44775231)

5. [最牛X的GCC 内联汇编](http://www.cnblogs.com/big-tree/p/5884543.html)

6. [Inline assembly for x86 in Linux](https://www.ibm.com/developerworks/library/l-ia/index.html)

7. [GCC-Inline-Assembly-HOWTO](http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html)

8. [C语言函数调用及栈帧结构](http://blog.csdn.net/qq_29403077/article/details/53205010)

9. [深入理解C语言的函数调用过程](http://blog.chinaunix.net/uid-23069658-id-3981406.html)

10. [深入理解C语言的函数调用过程](http://www.lupaworld.com/article-260205-1.html)
<br />
<br />
<br />

