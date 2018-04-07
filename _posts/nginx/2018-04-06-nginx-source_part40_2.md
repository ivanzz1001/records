---
layout: post
title: core/ngx_hash.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中hash。nginx中hash有三种类型：

* 不带通配符的hash

* 带前向通配符的hash

* 带后向通配符的hash

<pre>
注意： 不支持同时带有前向通配符与后向通配符的hash
</pre>


<!-- more -->

## 1. 函数ngx_hash_find()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


void *
ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len)
{
    ngx_uint_t       i;
    ngx_hash_elt_t  *elt;

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "hf:\"%*s\"", len, name);
#endif

    elt = hash->buckets[key % hash->size];

    if (elt == NULL) {
        return NULL;
    }

    while (elt->value) {
        if (len != (size_t) elt->len) {
            goto next;
        }

        for (i = 0; i < len; i++) {
            if (name[i] != elt->name[i]) {
                goto next;
            }
        }

        return elt->value;

    next:

        elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                               sizeof(void *));
        continue;
    }

    return NULL;
}
{% endhighlight %}

这里函数实现较为简单，我们主要看一下ngx_align_ptr宏，该宏的core/ngx_config.h头文件中：
<pre>
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
</pre>
主要是实现```a```字节对齐，这里是进行```sizeof(void *)```字节对齐。

## 2. 函数ngx_hash_find_wc_head()
{% highlight string %}
void *
ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    void        *value;
    ngx_uint_t   i, n, key;

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "wch:\"%*s\"", len, name);
#endif

    n = len;

    while (n) {
        if (name[n - 1] == '.') {
            break;
        }

        n--;
    }

    key = 0;

    for (i = n; i < len; i++) {
        key = ngx_hash(key, name[i]);
    }

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "key:\"%ui\"", key);
#endif

    value = ngx_hash_find(&hwc->hash, key, &name[n], len - n);

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "value:\"%p\"", value);
#endif

    if (value) {

        /*
         * the 2 low bits of value have the special meaning:
         *     00 - value is data pointer for both "example.com"
         *          and "*.example.com";
         *     01 - value is data pointer for "*.example.com" only;
         *     10 - value is pointer to wildcard hash allowing
         *          both "example.com" and "*.example.com";
         *     11 - value is pointer to wildcard hash allowing
         *          "*.example.com" only.
         */

        if ((uintptr_t) value & 2) {

            if (n == 0) {

                /* "example.com" */

                if ((uintptr_t) value & 1) {
                    return NULL;
                }

                hwc = (ngx_hash_wildcard_t *)
                                          ((uintptr_t) value & (uintptr_t) ~3);
                return hwc->value;
            }

            hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);

            value = ngx_hash_find_wc_head(hwc, name, n - 1);

            if (value) {
                return value;
            }

            return hwc->value;
        }

        if ((uintptr_t) value & 1) {

            if (n == 0) {

                /* "example.com" */

                return NULL;
            }

            return (void *) ((uintptr_t) value & (uintptr_t) ~3);
        }

        return value;
    }

    return hwc->value;
}
{% endhighlight %}

在介绍本函数之前，我们先来看一下对于```前向通配符字符串```加入hash表时其是如何处理的：

* 对于```*.example.com```在插入前向hash表时会转换成```com.example.\0```

* 对于```.example.com```在插入前向hash表时会转换成```com.example\0```

现在如果要检索```www.example.com```是否匹配```*.example.com```，直接使用nginx的ngx_hash_find_wc_head()方法查询。该方法会把要查询的```www.example.com```转化为```com.example.```再开始查询。


下面我们就来介绍一下具体的查找过程：
{% highlight string %}
//功能: 在前置通配符哈希表中，查找key对应的value值。  
//参数: name 关键字key  
//     len  关键字key对应的长度  
//例如: 查找www.example.com是否匹配*.example.com
//返回值: key对应的value值 

void *
ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    //1) 从后往前查找以.分割的每一个单词。  
    //例如: 查找www.example.com是否匹配*.example.com

   
    //2) 计算单词哈希值，并进行hash查找
    for (i = n; i < len; i++)  
    {  
        key = ngx_hash(key, name[i]);  
    }
    value = ngx_hash_find(&hwc->hash, key, &name[n], len - n);

    //3) 根据value返回结果进行处理
    //注意这里对应的value有可能是最终关键字对应的value，也有可能是下一个子哈希表的指针
    if(value)
    {
         //这里根据value的最低2bit判断value属于哪一种情况：
         // 00 --- 表示value指向的是一个data pointer, 同时匹配"example.com" 与 "*.example.com";
         // 01 --- 表示value指向的是一个data pointer，只匹配"*.example.com"
         // 10 --- 表示value指向的是下一个子wildcard hash，允许匹配"example.com" 与 "*.example.com";
         // 11 --- 表示value指向的是下一个子wildcard hash， 只允许匹配"*.example.com"

         if ((uintptr_t) value & 2) 
         {
            //这里value指向的是下一个子wildcard hash

            if(n == 0)
            {
                /*n=0表示匹配到了name的开始, 即实现了全匹配 "example.com"*/
               
                 if ((uintptr_t) value & 1) {
                    return NULL;       //value低两位为11的情况，不符合
                 }
                
               hwc = (ngx_hash_wildcard_t *)
                                          ((uintptr_t) value & (uintptr_t) ~3);
                return hwc->value;
            }

            //递归调用下一个子wildcard hash
            hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);

            value = ngx_hash_find_wc_head(hwc, name, n - 1);

            if (value) {
                return value;
            }

            return hwc->value;
         }

     
         //value指向的是data pointer
         if ((uintptr_t) value & 1) {

            if (n == 0) {

                /*n=0表示匹配到了name的开始, 即实现了全匹配 "example.com"*/

                return NULL;
            }

            return (void *) ((uintptr_t) value & (uintptr_t) ~3);
        }

        return value;
    }

   return hwc->value;
}
{% endhighlight %}
例如要在通配符哈希表中查找www.domain.com是否匹配*.domain.com，则从后往前查找每一个关键词。则查找过程如图所示:

![ngx-findwc-head](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_findwc_head.jpg)


对于www.domain.com，则先在根哈希表中查找com，而com指向一级哈希表。然后在一级哈希表中查找domain,因为domain是叶子节点了，domain指向的空间就是用户数据，查找过程结束

<br />
<br />

**[参看]**

1. [nginx源码分析之hash的实现](https://www.cnblogs.com/chengxuyuancc/p/3782808.html)

2. [nginx通配符哈希表](https://blog.csdn.net/apelife/article/details/53106140)

<br />
<br />
<br />

