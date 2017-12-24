---
layout: post
title: os/unix/ngx_automic.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文我们主要介绍一下与原子锁相关的部分：ngx_automic.h


<!-- more -->


## 1. os/unix/ngx_automic.h源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ATOMIC_H_INCLUDED_
#define _NGX_ATOMIC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_LIBATOMIC)

#define AO_REQUIRE_CAS
#include <atomic_ops.h>

#define NGX_HAVE_ATOMIC_OPS  1

typedef long                        ngx_atomic_int_t;
typedef AO_t                        ngx_atomic_uint_t;
typedef volatile ngx_atomic_uint_t  ngx_atomic_t;

#if (NGX_PTR_SIZE == 8)
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif

#define ngx_atomic_cmp_set(lock, old, new)                                    \
    AO_compare_and_swap(lock, old, new)
#define ngx_atomic_fetch_add(value, add)                                      \
    AO_fetch_and_add(value, add)
#define ngx_memory_barrier()        AO_nop()
#define ngx_cpu_pause()


#elif (NGX_DARWIN_ATOMIC)

/*
 * use Darwin 8 atomic(3) and barrier(3) operations
 * optimized at run-time for UP and SMP
 */

#include <libkern/OSAtomic.h>

/* "bool" conflicts with perl's CORE/handy.h */
#if 0
#undef bool
#endif


#define NGX_HAVE_ATOMIC_OPS  1

#if (NGX_PTR_SIZE == 8)

typedef int64_t                     ngx_atomic_int_t;
typedef uint64_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#define ngx_atomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap64Barrier(old, new, (int64_t *) lock)

#define ngx_atomic_fetch_add(value, add)                                      \
    (OSAtomicAdd64(add, (int64_t *) value) - add)

#else

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#define ngx_atomic_cmp_set(lock, old, new)                                    \
    OSAtomicCompareAndSwap32Barrier(old, new, (int32_t *) lock)

#define ngx_atomic_fetch_add(value, add)                                      \
    (OSAtomicAdd32(add, (int32_t *) value) - add)

#endif

#define ngx_memory_barrier()        OSMemoryBarrier()

#define ngx_cpu_pause()

typedef volatile ngx_atomic_uint_t  ngx_atomic_t;


#elif (NGX_HAVE_GCC_ATOMIC)

/* GCC 4.1 builtin atomic operations */

#define NGX_HAVE_ATOMIC_OPS  1

typedef long                        ngx_atomic_int_t;
typedef unsigned long               ngx_atomic_uint_t;

#if (NGX_PTR_SIZE == 8)
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif

typedef volatile ngx_atomic_uint_t  ngx_atomic_t;


#define ngx_atomic_cmp_set(lock, old, set)                                    \
    __sync_bool_compare_and_swap(lock, old, set)

#define ngx_atomic_fetch_add(value, add)                                      \
    __sync_fetch_and_add(value, add)

#define ngx_memory_barrier()        __sync_synchronize()

#if ( __i386__ || __i386 || __amd64__ || __amd64 )
#define ngx_cpu_pause()             __asm__ ("pause")
#else
#define ngx_cpu_pause()
#endif


#elif ( __i386__ || __i386 )

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
typedef volatile ngx_atomic_uint_t  ngx_atomic_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


#if ( __SUNPRO_C )

#define NGX_HAVE_ATOMIC_OPS  1

ngx_atomic_uint_t
ngx_atomic_cmp_set(ngx_atomic_t *lock, ngx_atomic_uint_t old,
    ngx_atomic_uint_t set);

ngx_atomic_int_t
ngx_atomic_fetch_add(ngx_atomic_t *value, ngx_atomic_int_t add);

/*
 * Sun Studio 12 exits with segmentation fault on '__asm ("pause")',
 * so ngx_cpu_pause is declared in src/os/unix/ngx_sunpro_x86.il
 */

void
ngx_cpu_pause(void);

/* the code in src/os/unix/ngx_sunpro_x86.il */

#define ngx_memory_barrier()        __asm (".volatile"); __asm (".nonvolatile")


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define NGX_HAVE_ATOMIC_OPS  1

#include "ngx_gcc_atomic_x86.h"

#endif


#elif ( __amd64__ || __amd64 )

typedef int64_t                     ngx_atomic_int_t;
typedef uint64_t                    ngx_atomic_uint_t;
typedef volatile ngx_atomic_uint_t  ngx_atomic_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)


#if ( __SUNPRO_C )

#define NGX_HAVE_ATOMIC_OPS  1

ngx_atomic_uint_t
ngx_atomic_cmp_set(ngx_atomic_t *lock, ngx_atomic_uint_t old,
    ngx_atomic_uint_t set);

ngx_atomic_int_t
ngx_atomic_fetch_add(ngx_atomic_t *value, ngx_atomic_int_t add);

/*
 * Sun Studio 12 exits with segmentation fault on '__asm ("pause")',
 * so ngx_cpu_pause is declared in src/os/unix/ngx_sunpro_amd64.il
 */

void
ngx_cpu_pause(void);

/* the code in src/os/unix/ngx_sunpro_amd64.il */

#define ngx_memory_barrier()        __asm (".volatile"); __asm (".nonvolatile")


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define NGX_HAVE_ATOMIC_OPS  1

#include "ngx_gcc_atomic_amd64.h"

#endif


#elif ( __sparc__ || __sparc || __sparcv9 )

#if (NGX_PTR_SIZE == 8)

typedef int64_t                     ngx_atomic_int_t;
typedef uint64_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#else

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#endif

typedef volatile ngx_atomic_uint_t  ngx_atomic_t;


#if ( __SUNPRO_C )

#define NGX_HAVE_ATOMIC_OPS  1

#include "ngx_sunpro_atomic_sparc64.h"


#else /* ( __GNUC__ || __INTEL_COMPILER ) */

#define NGX_HAVE_ATOMIC_OPS  1

#include "ngx_gcc_atomic_sparc64.h"

#endif


#elif ( __powerpc__ || __POWERPC__ )

#define NGX_HAVE_ATOMIC_OPS  1

#if (NGX_PTR_SIZE == 8)

typedef int64_t                     ngx_atomic_int_t;
typedef uint64_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)

#else

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)

#endif

typedef volatile ngx_atomic_uint_t  ngx_atomic_t;


#include "ngx_gcc_atomic_ppc.h"

#endif


#if !(NGX_HAVE_ATOMIC_OPS)

#define NGX_HAVE_ATOMIC_OPS  0

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
typedef volatile ngx_atomic_uint_t  ngx_atomic_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


static ngx_inline ngx_atomic_uint_t
ngx_atomic_cmp_set(ngx_atomic_t *lock, ngx_atomic_uint_t old,
    ngx_atomic_uint_t set)
{
    if (*lock == old) {
        *lock = set;
        return 1;
    }

    return 0;
}


static ngx_inline ngx_atomic_int_t
ngx_atomic_fetch_add(ngx_atomic_t *value, ngx_atomic_int_t add)
{
    ngx_atomic_int_t  old;

    old = *value;
    *value += add;

    return old;
}

#define ngx_memory_barrier()
#define ngx_cpu_pause()

#endif


void ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin);

#define ngx_trylock(lock)  (*(lock) == 0 && ngx_atomic_cmp_set(lock, 0, 1))
#define ngx_unlock(lock)    *(lock) = 0


#endif /* _NGX_ATOMIC_H_INCLUDED_ */

{% endhighlight %}



## 2. 执行NGX_HAVE_GCC_ATOMIC代码

由于我们在执行configure时生成的头文件中定义了```NGX_HAVE_GCC_ATOMIC```，因此这里我们执行如下：
{% highlight string %}
#if (NGX_HAVE_LIBATOMIC)

#elif (NGX_DARWIN_ATOMIC)


#elif (NGX_HAVE_GCC_ATOMIC)

/* GCC 4.1 builtin atomic operations */

#define NGX_HAVE_ATOMIC_OPS  1

typedef long                        ngx_atomic_int_t;
typedef unsigned long               ngx_atomic_uint_t;

#if (NGX_PTR_SIZE == 8)
#define NGX_ATOMIC_T_LEN            (sizeof("-9223372036854775808") - 1)
#else
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
#endif

typedef volatile ngx_atomic_uint_t  ngx_atomic_t;


#define ngx_atomic_cmp_set(lock, old, set)                                    \
    __sync_bool_compare_and_swap(lock, old, set)

#define ngx_atomic_fetch_add(value, add)                                      \
    __sync_fetch_and_add(value, add)

#define ngx_memory_barrier()        __sync_synchronize()

#if ( __i386__ || __i386 || __amd64__ || __amd64 )
#define ngx_cpu_pause()             __asm__ ("pause")
#else
#define ngx_cpu_pause()
#endif


#elif ( __i386__ || __i386 )


#elif ( __amd64__ || __amd64 )


#elif ( __sparc__ || __sparc || __sparcv9 )


#elif ( __powerpc__ || __POWERPC__ )


#endif
{% endhighlight %}

如上，```NGX_PTR_SIZE```在我们当前环境下为4,因此：
<pre>
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)
</pre>

在我们当前环境定义了```__i386__```与```__i386```，因此会执行如下：
<pre>
#define ngx_cpu_pause()             __asm__ ("pause")
</pre>

下面我们介绍一下gcc中的一些内置原子函数：

### 2.1 gcc内置原子函数

参看：《Using the GNU Compiler Collection》 p462

gcc从4.1.2开始提供了```__sync_*```系列的built-in函数，用于提供加减和逻辑运算的原子操作：

1） 
{% highlight string %}
type __sync_fetch_and_add (type *ptr, type value, ...)
type __sync_fetch_and_sub (type *ptr, type value, ...)
type __sync_fetch_and_or (type *ptr, type value, ...)
type __sync_fetch_and_and (type *ptr, type value, ...)
type __sync_fetch_and_xor (type *ptr, type value, ...)
type __sync_fetch_and_nand (type *ptr, type value, ...)
{% endhighlight %}

上面这组built-in原子函数会根据其名称所示执行相应的操作，并且返回内存中更新之前的值。即：
<pre>
{ tmp = *ptr; *ptr op= value; return tmp; }
{ tmp = *ptr; *ptr = ~(tmp & value); return tmp; } // nand
</pre>

**注意**：GCC 4.4及之后的版本```__sync_fetch_and_nand```的实现变为：*ptr = ~(tmp & value)， 而不是 *ptr = ~tmp & value .


2）
{% highlight string %}
type __sync_add_and_fetch (type *ptr, type value, ...)
type __sync_sub_and_fetch (type *ptr, type value, ...)
type __sync_or_and_fetch (type *ptr, type value, ...)
type __sync_and_and_fetch (type *ptr, type value, ...)
type __sync_xor_and_fetch (type *ptr, type value, ...)
type __sync_nand_and_fetch (type *ptr, type value, ...)
{% endhighlight %}

上面这组built-in原子函数会根据其名称所示执行相应的操作，并且返回内存中更新之后的值。即：
<pre>
{ *ptr op= value; return *ptr; }
{ *ptr = ~(*ptr & value); return *ptr; } // nand
</pre>

**注意**：GCC 4.4及之后的版本```__sync_nand_and_fetch```的实现变为：*ptr =~(*ptr & value)， 而不是 *ptr = ~*ptr & value .


3）
{% highlight string %}
bool __sync_bool_compare_and_swap (type *ptr, type oldval, type newval, ...)
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...)
{% endhighlight %}
上面这两个函数提供原子的比较和交换：如果 *ptr == oldval，就将 newval 写入 *ptr. 其中第一个函数在相等并写入的情况下返回true; 第二个函数返回操作之前的值。

<br />

<pre>
说明：

上述 “__sync_*” 函数中type可以是1,2,4或8字节长度的 “整数” 类型或 “浮点” 类型:
int8_t / uint8_t
int16_t / uint16_t
int32_t / uint32_t
int64_t / uint64_t

后面的可扩展参数(...)用来指出哪些变量需要memory barrier，因为目前gcc实现的是full barrier
(类似于linux kernel中的mb()，表示这个操作之前的所有内存操作不会重排序到这个操作之后), 所以
可以忽略这个参数。
</pre>


4) 
{% highlight string %}
__sync_synchronize (...)
{% endhighlight %}
函数发出一个full memory barrier。

关于memory barrier, cpu会对我们的指令进行排序，一般来说会提高程序的效率，但有时候可能造成我们不希望看到的结果。举一个例子，比如我们一个硬件设备，它有4个寄存器，当你发出一个操作指令的时候，一个寄存器存的是你的操作指令(比如READ)，两个寄存器存的是参数(比如addr和size)，最后一个寄存器是控制寄存器，在所有的参数都设置好之后向其发出指令，设备开始读取参数，执行命令。程序可能如下：
<pre>
write1(dev.register_size,size);
write1(dev.register_addr,addr);
write1(dev.register_cmd,READ);
write1(dev.register_control,GO);
</pre>

如果最后一条write1被换到了前几条语句之前，那么肯定不是我们所期望的，这时候我们可以在最后一条语句之前加入一个memory barrier，强制cpu执行完前面的写入以后再执行最后一条：
<pre>
write1(dev.register_size,size);
write1(dev.register_addr,addr);
write1(dev.register_cmd,READ);
__sync_synchronize();
write1(dev.register_control,GO);
</pre>


memory barrier有几种类型：

* acquire barrier: 不允许将barrier之后的内存读取指令移到barrier之前(linux kernel中的wmb())

* release barrier: 不允许将barrier之前的内存读取指令移到barrier之后(linux kernel中的rmb())

* full barrier: 以上两种barrier的合集(linux kernel中的mb())


5) 
{% highlight string %}
type __sync_lock_test_and_set (type *ptr, type value, ...)
void __sync_lock_release (type *ptr, ...)
{% endhighlight %}
上面第一个函数将```*ptr```设为value，并返回```*ptr```操作之前的值； 第二个函数将```*ptr```置为0.

6） 

示例：
{% highlight string %}
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

static int count = 0;


void *test_func(void *arg)
{
        int i=0;
        for(i=0;i<20000;++i){
                __sync_fetch_and_add(&count,1);
        }
        return NULL;
}

int main(int argc, const char *argv[])
{
        pthread_t id[20];
        int i = 0;

        for(i=0;i<20;++i){
                pthread_create(&id[i],NULL,test_func,NULL);
        }

        for(i=0;i<20;++i){
                pthread_join(id[i],NULL);
        }

        printf("%d\n",count);
        return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test8 test8.c -lpthread
[root@localhost test-src]# ./test8
count: 400000
</pre>


## 3. 其他
{% highlight string %}
#if !(NGX_HAVE_ATOMIC_OPS)

#define NGX_HAVE_ATOMIC_OPS  0

typedef int32_t                     ngx_atomic_int_t;
typedef uint32_t                    ngx_atomic_uint_t;
typedef volatile ngx_atomic_uint_t  ngx_atomic_t;
#define NGX_ATOMIC_T_LEN            (sizeof("-2147483648") - 1)


static ngx_inline ngx_atomic_uint_t
ngx_atomic_cmp_set(ngx_atomic_t *lock, ngx_atomic_uint_t old,
    ngx_atomic_uint_t set)
{
    if (*lock == old) {
        *lock = set;
        return 1;
    }

    return 0;
}


static ngx_inline ngx_atomic_int_t
ngx_atomic_fetch_add(ngx_atomic_t *value, ngx_atomic_int_t add)
{
    ngx_atomic_int_t  old;

    old = *value;
    *value += add;

    return old;
}

#define ngx_memory_barrier()
#define ngx_cpu_pause()

#endif


void ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin);

#define ngx_trylock(lock)  (*(lock) == 0 && ngx_atomic_cmp_set(lock, 0, 1))
#define ngx_unlock(lock)    *(lock) = 0
{% endhighlight %}
```NGX_HAVE_ATOMIC_OPS```在上一节已经定义为1，因此这里：
<pre>
#if !(NGX_HAVE_ATOMIC_OPS)

...

#endif
</pre>
并不会被执行。

关于ngx_spinlock()我们会在后续进行讲解。






<br />
<br />

**[参看]:**

1. [Nginx 源码完全剖析（11）ngx_spinlock](http://blog.csdn.net/poechant/article/details/8062969)

2. [GCC内联汇编（1）Get started](http://blog.csdn.net/poechant/article/details/7727751)

3. [GCC 提供的原子操作](http://www.cnblogs.com/woshare/p/5854428.html)


<br />
<br />
<br />

