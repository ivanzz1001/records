---
layout: post
title: core/ngx_hash.c源文件分析(1)
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
例如要在通配符哈希表中查找```www.domain.com```是否匹配```*.domain.com```，则从后往前查找每一个关键词。则查找过程如图所示:

![ngx-findwc-head](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_findwc_head.jpg)


对于```www.domain.com```，则先在根哈希表中查找```com```，而```com```指向一级哈希表。然后在一级哈希表中查找```domain```,因为domain是叶子节点了，```domain```指向的空间就是用户数据，查找过程结束。


## 3. 函数ngx_hash_find_wc_tail()
{% highlight string %}
void *
ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    void        *value;
    ngx_uint_t   i, key;

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "wct:\"%*s\"", len, name);
#endif

    key = 0;

    for (i = 0; i < len; i++) {
        if (name[i] == '.') {
            break;
        }

        key = ngx_hash(key, name[i]);
    }

    if (i == len) {
        return NULL;
    }

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "key:\"%ui\"", key);
#endif

    value = ngx_hash_find(&hwc->hash, key, name, i);

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "value:\"%p\"", value);
#endif

    if (value) {

        /*
         * the 2 low bits of value have the special meaning:
         *     00 - value is data pointer;
         *     11 - value is pointer to wildcard hash allowing "example.*".
         */

        if ((uintptr_t) value & 2) {

            i++;

            hwc = (ngx_hash_wildcard_t *) ((uintptr_t) value & (uintptr_t) ~3);

            value = ngx_hash_find_wc_tail(hwc, &name[i], len - i);

            if (value) {
                return value;
            }

            return hwc->value;
        }

        return value;
    }

    return hwc->value;
}

{% endhighlight %}

后置通配符hash表查询操作，跟前置通配符查找操作基本类似。差别是后置通配符是从第一个关键字往后查询。例如查找```www.example.com```是否匹配```www.example.*```，则先查询```www```，接着查询```example```。函数```ngx_hash_find_wc_tail()```用来在后置通配符哈希表中查找到key对应的value值。

下面我们就来介绍一下具体的查找过程：
{% highlight string %}
void *
ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len)
{
    //1) 从前往后找，以.分隔每一个单词,并求得hash key值

    //2) 进行hash查找
    value = ngx_hash_find(&hwc->hash, key, name, i);

    if(value)
    {
        //3) 这里根据value的最低2bit判断value属于哪一种情况：
        // 00 --- value是一个data pointer
        // 11 --- value是一个wildcard hash pointer，指向的是"example.*"这样的子级hash 表
    }
}
{% endhighlight %}


## 4. 函数ngx_hash_find_combined()
{% highlight string %}
void *
ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key, u_char *name,
    size_t len)
{
    void  *value;

    if (hash->hash.buckets) {
        value = ngx_hash_find(&hash->hash, key, name, len);

        if (value) {
            return value;
        }
    }

    if (len == 0) {
        return NULL;
    }

    if (hash->wc_head && hash->wc_head->hash.buckets) {
        value = ngx_hash_find_wc_head(hash->wc_head, name, len);

        if (value) {
            return value;
        }
    }

    if (hash->wc_tail && hash->wc_tail->hash.buckets) {
        value = ngx_hash_find_wc_tail(hash->wc_tail, name, len);

        if (value) {
            return value;
        }
    }

    return NULL;
}
{% endhighlight %}
这里首先进行全匹配hash查找；然后再进行前置通配hash查找；最后进行后置通配hash查找。

## 5. NGX_HASH_ELT_SIZE宏定义
{% highlight string %}
#define NGX_HASH_ELT_SIZE(name)                                               \
    (sizeof(void *) + ngx_align((name)->key.len + 2, sizeof(void *)))
{% endhighlight %}
这里求```ngx_hash_elt_t```结构体的大小。注意会进行```sizeof(void *)```字节对齐。

## 6. 函数ngx_hash_init()
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
    u_char          *elts;
    size_t           len;
    u_short         *test;
    ngx_uint_t       i, n, key, size, start, bucket_size;
    ngx_hash_elt_t  *elt, **buckets;

    if (hinit->max_size == 0) {
        ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                      "could not build %s, you should "
                      "increase %s_max_size: %i",
                      hinit->name, hinit->name, hinit->max_size);
        return NGX_ERROR;
    }

    for (n = 0; n < nelts; n++) {
        if (hinit->bucket_size < NGX_HASH_ELT_SIZE(&names[n]) + sizeof(void *))
        {
            ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                          "could not build %s, you should "
                          "increase %s_bucket_size: %i",
                          hinit->name, hinit->name, hinit->bucket_size);
            return NGX_ERROR;
        }
    }

    test = ngx_alloc(hinit->max_size * sizeof(u_short), hinit->pool->log);
    if (test == NULL) {
        return NGX_ERROR;
    }

    bucket_size = hinit->bucket_size - sizeof(void *);

    start = nelts / (bucket_size / (2 * sizeof(void *)));
    start = start ? start : 1;

    if (hinit->max_size > 10000 && nelts && hinit->max_size / nelts < 100) {
        start = hinit->max_size - 1000;
    }

    for (size = start; size <= hinit->max_size; size++) {

        ngx_memzero(test, size * sizeof(u_short));

        for (n = 0; n < nelts; n++) {
            if (names[n].key.data == NULL) {
                continue;
            }

            key = names[n].key_hash % size;
            test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));

#if 0
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: %ui %ui \"%V\"",
                          size, key, test[key], &names[n].key);
#endif

            if (test[key] > (u_short) bucket_size) {
                goto next;
            }
        }

        goto found;

    next:

        continue;
    }

    size = hinit->max_size;

    ngx_log_error(NGX_LOG_WARN, hinit->pool->log, 0,
                  "could not build optimal %s, you should increase "
                  "either %s_max_size: %i or %s_bucket_size: %i; "
                  "ignoring %s_bucket_size",
                  hinit->name, hinit->name, hinit->max_size,
                  hinit->name, hinit->bucket_size, hinit->name);

found:

    for (i = 0; i < size; i++) {
        test[i] = sizeof(void *);
    }

    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    len = 0;

    for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        test[i] = (u_short) (ngx_align(test[i], ngx_cacheline_size));

        len += test[i];
    }

    if (hinit->hash == NULL) {
        hinit->hash = ngx_pcalloc(hinit->pool, sizeof(ngx_hash_wildcard_t)
                                             + size * sizeof(ngx_hash_elt_t *));
        if (hinit->hash == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }

        buckets = (ngx_hash_elt_t **)
                      ((u_char *) hinit->hash + sizeof(ngx_hash_wildcard_t));

    } else {
        buckets = ngx_pcalloc(hinit->pool, size * sizeof(ngx_hash_elt_t *));
        if (buckets == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }
    }

    elts = ngx_palloc(hinit->pool, len + ngx_cacheline_size);
    if (elts == NULL) {
        ngx_free(test);
        return NGX_ERROR;
    }

    elts = ngx_align_ptr(elts, ngx_cacheline_size);

    for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        buckets[i] = (ngx_hash_elt_t *) elts;
        elts += test[i];

    }

    for (i = 0; i < size; i++) {
        test[i] = 0;
    }

    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        elt = (ngx_hash_elt_t *) ((u_char *) buckets[key] + test[key]);

        elt->value = names[n].value;
        elt->len = (u_short) names[n].key.len;

        ngx_strlow(elt->name, names[n].key.data, names[n].key.len);

        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    for (i = 0; i < size; i++) {
        if (buckets[i] == NULL) {
            continue;
        }

        elt = (ngx_hash_elt_t *) ((u_char *) buckets[i] + test[i]);

        elt->value = NULL;
    }

    ngx_free(test);

    hinit->hash->buckets = buckets;
    hinit->hash->size = size;

#if 0

    for (i = 0; i < size; i++) {
        ngx_str_t   val;
        ngx_uint_t  key;

        elt = buckets[i];

        if (elt == NULL) {
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: NULL", i);
            continue;
        }

        while (elt->value) {
            val.len = elt->len;
            val.data = &elt->name[0];

            key = hinit->key(val.data, val.len);

            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: %p \"%V\" %ui", i, elt, &val, key);

            elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                                   sizeof(void *));
        }
    }

#endif

    return NGX_OK;
}
{% endhighlight %}
本函数根据```hint```及```name```数组完成精准匹配hash表初始化。下面我们分成几个部分来进行讲解：

1) 检查```max_size```及```bucket_size```是否合法

```max_size```用于指定整个hash表中桶的数目；而bucket_size用于指定每个桶的大小，它的大小必须要保证每个桶至少能存放一个```<key,value>```键值对。

{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
    if (hinit->max_size == 0) {
        ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                      "could not build %s, you should "
                      "increase %s_max_size: %i",
                      hinit->name, hinit->name, hinit->max_size);
        return NGX_ERROR;
    }

    for (n = 0; n < nelts; n++) {
        if (hinit->bucket_size < NGX_HASH_ELT_SIZE(&names[n]) + sizeof(void *))
        {
            ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                          "could not build %s, you should "
                          "increase %s_bucket_size: %i",
                          hinit->name, hinit->name, hinit->bucket_size);
            return NGX_ERROR;
        }
    }
}
{% endhighlight %}
上面for循环保证hash的桶至少能装一个```<key,value>```键值对。宏```NGX_HASH_ELT_SIZE```用于计算一个实际的键值对所占用的空间。之所以后面还要再加上```sizeof(void *)```,是因为每个桶都用一个值```NULL```的void *指针来标记结束。

2) 计算Hash中桶的个数
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
	test = ngx_alloc(hinit->max_size * sizeof(u_short), hinit->pool->log);
    if (test == NULL) {
        return NGX_ERROR;
    }

    bucket_size = hinit->bucket_size - sizeof(void *);

    start = nelts / (bucket_size / (2 * sizeof(void *)));
    start = start ? start : 1;

    if (hinit->max_size > 10000 && nelts && hinit->max_size / nelts < 100) {
        start = hinit->max_size - 1000;
    }

    for (size = start; size <= hinit->max_size; size++) {

        ngx_memzero(test, size * sizeof(u_short));

        for (n = 0; n < nelts; n++) {
            if (names[n].key.data == NULL) {
                continue;
            }

            key = names[n].key_hash % size;
            test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));

#if 0
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: %ui %ui \"%V\"",
                          size, key, test[key], &names[n].key);
#endif

            if (test[key] > (u_short) bucket_size) {
                goto next;
            }
        }

        goto found;

    next:

        continue;
    }

    size = hinit->max_size;

    ngx_log_error(NGX_LOG_WARN, hinit->pool->log, 0,
                  "could not build optimal %s, you should increase "
                  "either %s_max_size: %i or %s_bucket_size: %i; "
                  "ignoring %s_bucket_size",
                  hinit->name, hinit->name, hinit->max_size,
                  hinit->name, hinit->bucket_size, hinit->name);
}
{% endhighlight %}
这里我们先给出一个示意图：

![ngx-hash-buckets-count](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_hash_buckets_count.jpg)

这里nginx首先会分配一个最大的桶```hinit->max_size```，但是一般情况下我们并不需要那么大空间，这里我们会进行相应的运算，从而选出一个适合当前环境的桶大小。这里首先评估每一个```ngx_hash_elt_t```元素的大小为```2*sizeof(void *)```,则bucket[0]、bucket[1]...中每一个子桶能够容纳的元素个数约为```bucket_size/2*sizeof(void *)```,这样就评估出大概的start值。然后我们遍历```names```，分别将其hash进桶中，如果当前bucket_size能够容纳这些元素，则我们就可以采用该```size```大小作为桶数。

注意上面函数中有：
<pre>
bucket_size = hinit->bucket_size - sizeof(void *);
</pre>
这里减去```sizeof(void *)```是为了减去最后一个以```NULL```结尾的指针空间



3） 计算新创建Hash所占用的空间
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
found:

    for (i = 0; i < size; i++) {
        test[i] = sizeof(void *);
    }

    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    len = 0;

    for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        test[i] = (u_short) (ngx_align(test[i], ngx_cacheline_size));

        len += test[i];
    }

}
{% endhighlight %}
这里首先将test[i]的值都初始化为```sizeof(void *)```,即一个```NULL```结尾指针的大小。再接着将```names```所有元素hash进桶中，然后计算出每个子桶占用的空间大小； 最后在求出所有这些子桶总共占用的空间大小。

上面根据实际的<key,value>键值对来实际计算每个桶的大小，而不是所有桶的大小的设置成一样的，这样能很有效的节约内存空间，当然由于每个桶的大小是不固定的，所有每个桶的末尾需要一个额外空间（大小为sizeof(void*)）来标记桶的结束。并且每个桶大小满足cache行对齐，这样能加快访问速度，从这里也可以看出nginx无处不在优化程序的性能和资源的使用效率。

4) 分配一级桶空间
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{

    if (hinit->hash == NULL) {
        hinit->hash = ngx_pcalloc(hinit->pool, sizeof(ngx_hash_wildcard_t)
                                             + size * sizeof(ngx_hash_elt_t *));
        if (hinit->hash == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }

        buckets = (ngx_hash_elt_t **)
                      ((u_char *) hinit->hash + sizeof(ngx_hash_wildcard_t));

    } else {
        buckets = ngx_pcalloc(hinit->pool, size * sizeof(ngx_hash_elt_t *));
        if (buckets == NULL) {
            ngx_free(test);
            return NGX_ERROR;
        }
    }

}
{% endhighlight %}
这里如果```hinit->hash```为NULL时，则首先应该为```hash```(ngx_hash_t)本身分配空间，并且需要为hash所指向的桶分配空间。这里为hash本身分配空间时，使用的是```sizeof(ngx_hash_wildcard_t)```,多分配了一小点空间。分配后如下所示：

![ngx-hash-buckets-alloc](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_hash_buckets_alloc.jpg)


5) 分配二级子桶空间
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
	elts = ngx_palloc(hinit->pool, len + ngx_cacheline_size);
    if (elts == NULL) {
        ngx_free(test);
        return NGX_ERROR;
    }

    elts = ngx_align_ptr(elts, ngx_cacheline_size);
}
{% endhighlight %}
这里分配的空间为```len+ngx_cacheline_size```，这是因为我们下面需要进行```ngx_cacheline_size```大小对齐。

6） 初始化一级桶相关指针
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
	   for (i = 0; i < size; i++) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        buckets[i] = (ngx_hash_elt_t *) elts;
        elts += test[i];

    }
}
{% endhighlight %}
因为这里二级桶分配的是一块连续的内存空间，因此这里需要对一级桶指针做一个初始化。

7) 初始化hash表
{% highlight string %}
ngx_int_t
ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names, ngx_uint_t nelts)
{
	    for (i = 0; i < size; i++) {
        test[i] = 0;
    }

    for (n = 0; n < nelts; n++) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        elt = (ngx_hash_elt_t *) ((u_char *) buckets[key] + test[key]);

        elt->value = names[n].value;
        elt->len = (u_short) names[n].key.len;

        ngx_strlow(elt->name, names[n].key.data, names[n].key.len);

        test[key] = (u_short) (test[key] + NGX_HASH_ELT_SIZE(&names[n]));
    }

    for (i = 0; i < size; i++) {
        if (buckets[i] == NULL) {
            continue;
        }

        elt = (ngx_hash_elt_t *) ((u_char *) buckets[i] + test[i]);

        elt->value = NULL;
    }

    ngx_free(test);

    hinit->hash->buckets = buckets;
    hinit->hash->size = size;
}
{% endhighlight %}
这里对```names```数组中的每一个元素，将其映射到hash表中。第二个for循环用于将每个子桶中的最末尾位置置为NULL，以做结尾使用。

到此位置，就完成了Hash表的初始化，下面给出一幅Hash的大概图景：

![ngx-hash-init](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_hash_init.jpg)


<br />
<br />

**[参看]**

1. [nginx源码分析之hash的实现](https://www.cnblogs.com/chengxuyuancc/p/3782808.html)

2. [nginx通配符哈希表](https://blog.csdn.net/apelife/article/details/53106140)

<br />
<br />
<br />

