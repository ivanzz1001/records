---
layout: post
title: core/ngx_hash.c源文件分析(2)
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

## 1. 函数ngx_hash_wildcard_init()
{% highlight string %}
ngx_int_t
ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts)
{
    size_t                len, dot_len;
    ngx_uint_t            i, n, dot;
    ngx_array_t           curr_names, next_names;
    ngx_hash_key_t       *name, *next_name;
    ngx_hash_init_t       h;
    ngx_hash_wildcard_t  *wdc;

    if (ngx_array_init(&curr_names, hinit->temp_pool, nelts,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (ngx_array_init(&next_names, hinit->temp_pool, nelts,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    for (n = 0; n < nelts; n = i) {

#if 0
        ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                      "wc0: \"%V\"", &names[n].key);
#endif

        dot = 0;

        for (len = 0; len < names[n].key.len; len++) {
            if (names[n].key.data[len] == '.') {
                dot = 1;
                break;
            }
        }

        name = ngx_array_push(&curr_names);
        if (name == NULL) {
            return NGX_ERROR;
        }

        name->key.len = len;
        name->key.data = names[n].key.data;
        name->key_hash = hinit->key(name->key.data, name->key.len);
        name->value = names[n].value;

#if 0
        ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                      "wc1: \"%V\" %ui", &name->key, dot);
#endif

        dot_len = len + 1;

        if (dot) {
            len++;
        }

        next_names.nelts = 0;

        if (names[n].key.len != len) {
            next_name = ngx_array_push(&next_names);
            if (next_name == NULL) {
                return NGX_ERROR;
            }

            next_name->key.len = names[n].key.len - len;
            next_name->key.data = names[n].key.data + len;
            next_name->key_hash = 0;
            next_name->value = names[n].value;

#if 0
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "wc2: \"%V\"", &next_name->key);
#endif
        }

        for (i = n + 1; i < nelts; i++) {
            if (ngx_strncmp(names[n].key.data, names[i].key.data, len) != 0) {
                break;
            }

            if (!dot
                && names[i].key.len > len
                && names[i].key.data[len] != '.')
            {
                break;
            }

            next_name = ngx_array_push(&next_names);
            if (next_name == NULL) {
                return NGX_ERROR;
            }

            next_name->key.len = names[i].key.len - dot_len;
            next_name->key.data = names[i].key.data + dot_len;
            next_name->key_hash = 0;
            next_name->value = names[i].value;

#if 0
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "wc3: \"%V\"", &next_name->key);
#endif
        }

        if (next_names.nelts) {

            h = *hinit;
            h.hash = NULL;

            if (ngx_hash_wildcard_init(&h, (ngx_hash_key_t *) next_names.elts,
                                       next_names.nelts)
                != NGX_OK)
            {
                return NGX_ERROR;
            }

            wdc = (ngx_hash_wildcard_t *) h.hash;

            if (names[n].key.len == len) {
                wdc->value = names[n].value;
            }

            name->value = (void *) ((uintptr_t) wdc | (dot ? 3 : 2));

        } else if (dot) {
            name->value = (void *) ((uintptr_t) name->value | 1);
        }
    }

    if (ngx_hash_init(hinit, (ngx_hash_key_t *) curr_names.elts,
                      curr_names.nelts)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    return NGX_OK;
}
{% endhighlight %}

本函数用于初始化带通配符的hash表。在具体介绍本函数之前，我们先给出一副整体的示意图：

![ngx-wildcard-hash](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_wildcard_hash.jpg)

上图只显示了二级Hash，实际上可能会出现多级Hash的情况。这里我们再举一个例子来加深理解，假设有下面的键值对<key,value>:
{% highlight string %}
<*.com, "220.181.111.147">,<*.baidu.com, "220.181.111.147">,<*.baidu.com.cn, "220.181.111.147">,<*.google.com，"58.63.236.35">
{% endhighlight %}

再调用后面的```ngx_hash_add_key()```函数添加到```dns_wc_head```数组中时，该数组值最后会被处理成如下所示：
<pre>
{key = ("com.", 4 ), key_hash = 0, value = "220.181.111.147"}
{key = ("cn.com.baidu.", 13), key_hash = 0, value = "220.181.111.147"}
{key = ("com.baidu.", 10), key_hash = 0, value = "220.181.111.147"}
{key = ("com.google.", 11), key_hash = 0, value = "58.63.236.35"}
</pre>

然后将```dns_wc_head```数组中的值添加到hash表中之前，一般都会先调用```qsort()```函数进行排序，处理成为：
<pre>
{key = ("cn.com.baidu.", 13), key_hash = 0, value = "220.181.111.147"}
{key = ("com.", 4 ), key_hash = 0, value = "220.181.111.147"}
{key = ("com.baidu.", 10), key_hash = 0, value = "220.181.111.147"}
{key = ("com.google.", 11), key_hash = 0, value = "58.63.236.35"}
</pre>
这样处理完成后，所有相同前缀的元素都会排在一起。



接下来我们分成好几个部分来讲解一下ngx_hash_wildcard_init()函数：

1) 初始化两个数组
{% highlight string %}
ngx_int_t
ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts)
{
	if (ngx_array_init(&curr_names, hinit->temp_pool, nelts,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (ngx_array_init(&next_names, hinit->temp_pool, nelts,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }
}
{% endhighlight %}
其中第一个数组```curr_names```用于处理上图中的```一级Hash元素```; 而```next_names```用于处理```子级Hash元素```(二级以上）。

2） 构建Hash表
{% highlight string %}
ngx_int_t
ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts)
{
    // 这里for循环时，n可能不是每次+1， 这是因为在处理到有相同前缀的元素时，会构造子hash，后面这些元素都交由子Hash来处理
	for (n = 0; n < nelts; n = i) {
      
         //1) 寻找到names[n]中的第一个'.'字符（例如上面cn.com.baidu.)
   
         //2) 将寻找到的元素放入curr_names数组中(这里查找的元素为cn)

         //3) 假如查找到的元素长度不等于names[n]的总长度, 则初始化next_names数组(例如，上面len(cn) != len(cn.com.baidu))
      
        //4) 从names[n+1]开始，判断后续是否有与上面查找到的元素同前缀的names， 如果有则加到next_names数组中

        //5） 假如next_names数组长度不为0，则递归调用ngx_hash_wildcard_init()函数，构建子级Hash
         if (next_names.nelts) {

            h = *hinit;
            h.hash = NULL;

            if (ngx_hash_wildcard_init(&h, (ngx_hash_key_t *) next_names.elts,
                                       next_names.nelts)
                != NGX_OK)
            {
                return NGX_ERROR;
            }

            wdc = (ngx_hash_wildcard_t *) h.hash;

            //全匹配的Hash值在wdc->value处
            if (names[n].key.len == len) {
                wdc->value = names[n].value;
            }

            //这里dot来区分该元素是否匹配到了最末尾
            //这里2表示当前已经到了一个匹配的结尾，即已经获得了一个全量匹配，不需要再匹配下去了
            name->value = (void *) ((uintptr_t) wdc | (dot ? 3 : 2));

        } else if (dot) {

            //并没有子级Hash的情况
            name->value = (void *) ((uintptr_t) name->value | 1);
        }
    }

    //6) 初始化一级Hash表

}
{% endhighlight %}


下面给出上面所举例子的一个Hash图：

![ngx-wildcard-hash-eg](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_wildcard_hash._eg.jpg)

## 2. 函数ngx_hash_key() 
{% highlight string %}
ngx_uint_t
ngx_hash_key(u_char *data, size_t len)
{
    ngx_uint_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        key = ngx_hash(key, data[i]);
    }

    return key;
}
{% endhighlight %}
本函数用于对一个字符串求Hash：
<pre>
key = (ngx_uint_t) key * 31 + data[i];
</pre>

## 3. 函数ngx_hash_key_lc()
{% highlight string %}
ngx_uint_t
ngx_hash_key_lc(u_char *data, size_t len)
{
    ngx_uint_t  i, key;

    key = 0;

    for (i = 0; i < len; i++) {
        key = ngx_hash(key, ngx_tolower(data[i]));
    }

    return key;
}
{% endhighlight %}

这里将data字符串转换为小写来求Hash

## 4. 函数ngx_hash_strlow()
{% highlight string %}
ngx_uint_t
ngx_hash_strlow(u_char *dst, u_char *src, size_t n)
{
    ngx_uint_t  key;

    key = 0;

    while (n--) {
        *dst = ngx_tolower(*src);
        key = ngx_hash(key, *dst);
        dst++;
        src++;
    }

    return key;
}
{% endhighlight %}
这里在求Hash的同时，将src转化成小写.

## 5. 函数ngx_hash_keys_array_init()
{% highlight string %}
ngx_int_t
ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type)
{
    ngx_uint_t  asize;

    if (type == NGX_HASH_SMALL) {
        asize = 4;
        ha->hsize = 107;

    } else {
        asize = NGX_HASH_LARGE_ASIZE;
        ha->hsize = NGX_HASH_LARGE_HSIZE;
    }

    if (ngx_array_init(&ha->keys, ha->temp_pool, asize, sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (ngx_array_init(&ha->dns_wc_head, ha->temp_pool, asize,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (ngx_array_init(&ha->dns_wc_tail, ha->temp_pool, asize,
                       sizeof(ngx_hash_key_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    ha->keys_hash = ngx_pcalloc(ha->temp_pool, sizeof(ngx_array_t) * ha->hsize);
    if (ha->keys_hash == NULL) {
        return NGX_ERROR;
    }

    ha->dns_wc_head_hash = ngx_pcalloc(ha->temp_pool,
                                       sizeof(ngx_array_t) * ha->hsize);
    if (ha->dns_wc_head_hash == NULL) {
        return NGX_ERROR;
    }

    ha->dns_wc_tail_hash = ngx_pcalloc(ha->temp_pool,
                                       sizeof(ngx_array_t) * ha->hsize);
    if (ha->dns_wc_tail_hash == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}
{% endhighlight %}

本函数用于初始化```ngx_hash_keys_arrays_t```数据结构， 分别初始化如下数据：

* ```ha->keys```: 不带通配符的```ngx_hash_key_t```数组

* ```ha->dns_wc_head```: 带前向通配符的```ngx_hash_key_t```数组

* ```ha->dns_wc_tail```: 带后向通配符的```ngx_hash_key_t```数组

* ```ha->keys_hash```:  这是一个二维数组。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复

* ```ha->dns_wc_head_hash```: 这是一个二维数组。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复

* ```ha->dns_wc_tail_hash```: 这是一个二维数组。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复


## 6. 函数ngx_hash_add_key()
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
    size_t           len;
    u_char          *p;
    ngx_str_t       *name;
    ngx_uint_t       i, k, n, skip, last;
    ngx_array_t     *keys, *hwc;
    ngx_hash_key_t  *hk;

    last = key->len;

    if (flags & NGX_HASH_WILDCARD_KEY) {

        /*
         * supported wildcards:
         *     "*.example.com", ".example.com", and "www.example.*"
         */

        n = 0;

        for (i = 0; i < key->len; i++) {

            if (key->data[i] == '*') {
                if (++n > 1) {
                    return NGX_DECLINED;
                }
            }

            if (key->data[i] == '.' && key->data[i + 1] == '.') {
                return NGX_DECLINED;
            }

            if (key->data[i] == '\0') {
                return NGX_DECLINED;
            }
        }

        if (key->len > 1 && key->data[0] == '.') {
            skip = 1;
            goto wildcard;
        }

        if (key->len > 2) {

            if (key->data[0] == '*' && key->data[1] == '.') {
                skip = 2;
                goto wildcard;
            }

            if (key->data[i - 2] == '.' && key->data[i - 1] == '*') {
                skip = 0;
                last -= 2;
                goto wildcard;
            }
        }

        if (n) {
            return NGX_DECLINED;
        }
    }

    /* exact hash */

    k = 0;

    for (i = 0; i < last; i++) {
        if (!(flags & NGX_HASH_READONLY_KEY)) {
            key->data[i] = ngx_tolower(key->data[i]);
        }
        k = ngx_hash(k, key->data[i]);
    }

    k %= ha->hsize;

    /* check conflicts in exact hash */

    name = ha->keys_hash[k].elts;

    if (name) {
        for (i = 0; i < ha->keys_hash[k].nelts; i++) {
            if (last != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data, name[i].data, last) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4,
                           sizeof(ngx_str_t))
            != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(&ha->keys_hash[k]);
    if (name == NULL) {
        return NGX_ERROR;
    }

    *name = *key;

    hk = ngx_array_push(&ha->keys);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key = *key;
    hk->key_hash = ngx_hash_key(key->data, last);
    hk->value = value;

    return NGX_OK;


wildcard:

    /* wildcard hash */

    k = ngx_hash_strlow(&key->data[skip], &key->data[skip], last - skip);

    k %= ha->hsize;

    if (skip == 1) {

        /* check conflicts in exact hash for ".example.com" */

        name = ha->keys_hash[k].elts;

        if (name) {
            len = last - skip;

            for (i = 0; i < ha->keys_hash[k].nelts; i++) {
                if (len != name[i].len) {
                    continue;
                }

                if (ngx_strncmp(&key->data[1], name[i].data, len) == 0) {
                    return NGX_BUSY;
                }
            }

        } else {
            if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4,
                               sizeof(ngx_str_t))
                != NGX_OK)
            {
                return NGX_ERROR;
            }
        }

        name = ngx_array_push(&ha->keys_hash[k]);
        if (name == NULL) {
            return NGX_ERROR;
        }

        name->len = last - 1;
        name->data = ngx_pnalloc(ha->temp_pool, name->len);
        if (name->data == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(name->data, &key->data[1], name->len);
    }


    if (skip) {

        /*
         * convert "*.example.com" to "com.example.\0"
         *      and ".example.com" to "com.example\0"
         */

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        len = 0;
        n = 0;

        for (i = last - 1; i; i--) {
            if (key->data[i] == '.') {
                ngx_memcpy(&p[n], &key->data[i + 1], len);
                n += len;
                p[n++] = '.';
                len = 0;
                continue;
            }

            len++;
        }

        if (len) {
            ngx_memcpy(&p[n], &key->data[1], len);
            n += len;
        }

        p[n] = '\0';

        hwc = &ha->dns_wc_head;
        keys = &ha->dns_wc_head_hash[k];

    } else {

        /* convert "www.example.*" to "www.example\0" */

        last++;

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        ngx_cpystrn(p, key->data, last);

        hwc = &ha->dns_wc_tail;
        keys = &ha->dns_wc_tail_hash[k];
    }


    /* check conflicts in wildcard hash */

    name = keys->elts;

    if (name) {
        len = last - skip;

        for (i = 0; i < keys->nelts; i++) {
            if (len != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data + skip, name[i].data, len) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(keys, ha->temp_pool, 4, sizeof(ngx_str_t)) != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(keys);
    if (name == NULL) {
        return NGX_ERROR;
    }

    name->len = last - skip;
    name->data = ngx_pnalloc(ha->temp_pool, name->len);
    if (name->data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(name->data, key->data + skip, name->len);


    /* add to wildcard hash */

    hk = ngx_array_push(hwc);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key.len = last - 1;
    hk->key.data = p;
    hk->key_hash = 0;
    hk->value = value;

    return NGX_OK;
}
{% endhighlight %}

本函数用于添加元素到```ngx_hash_keys_arrays_t```数据结构中。下面我们分几个部分来说明一下该函数：

**1) 判断元素添加到哪一种类的Hash表中**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
	last = key->len;

    if (flags & NGX_HASH_WILDCARD_KEY) {

        /*
         * supported wildcards:
         *     "*.example.com", ".example.com", and "www.example.*"
         */

        n = 0;

        for (i = 0; i < key->len; i++) {

            if (key->data[i] == '*') {
                if (++n > 1) {
                    return NGX_DECLINED;
                }
            }

            if (key->data[i] == '.' && key->data[i + 1] == '.') {
                return NGX_DECLINED;
            }

            if (key->data[i] == '\0') {
                return NGX_DECLINED;
            }
        }

        if (key->len > 1 && key->data[0] == '.') {
            skip = 1;
            goto wildcard;
        }

        if (key->len > 2) {

            if (key->data[0] == '*' && key->data[1] == '.') {
                skip = 2;
                goto wildcard;
            }

            if (key->data[i - 2] == '.' && key->data[i - 1] == '*') {
                skip = 0;
                last -= 2;
                goto wildcard;
            }
        }

        if (n) {
            return NGX_DECLINED;
        }
    }
}
{% endhighlight %}
这里首先根据```flags```提示当前是否要添加的```key```元素是否含有通配。如果有的话，则执行if条件的相关操作。这里支持的通配含如下三种形式：
<pre>
/*
* supported wildcards:
*     "*.example.com", ".example.com", and "www.example.*"
*/
</pre>

然后判断所传入的带通配的```key```元素是否合法，如果合法是属于上述三种的哪一种通配：

* ```skip=1```: 表示```.example.com```这种通配类型

* ```skip=2```: 表示```*.example.com```这种通配类型

* ```skip=0```: 表示```www.example.*```这种通配类型，此时将```last```值进行调整```last-=2```，即减去后面```.*```两个字符的长度。

**2) 插入元素到exact hash中**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
	 /* exact hash */

    k = 0;

    for (i = 0; i < last; i++) {
        if (!(flags & NGX_HASH_READONLY_KEY)) {
            key->data[i] = ngx_tolower(key->data[i]);
        }
        k = ngx_hash(k, key->data[i]);
    }

    k %= ha->hsize;

    /* check conflicts in exact hash */

    name = ha->keys_hash[k].elts;

    if (name) {
        for (i = 0; i < ha->keys_hash[k].nelts; i++) {
            if (last != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data, name[i].data, last) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4,
                           sizeof(ngx_str_t))
            != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(&ha->keys_hash[k]);
    if (name == NULL) {
        return NGX_ERROR;
    }

    *name = *key;

    hk = ngx_array_push(&ha->keys);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key = *key;
    hk->key_hash = ngx_hash_key(key->data, last);
    hk->value = value;

    return NGX_OK;

}
{% endhighlight %}

这里插入步骤如下：

* 求可以的hash值。 这里如果```flags```标明的该key不是```NGX_HASH_READONLY_KEY```不是readonly类型的话，会先将该key转换成小写，然后再求hash值。

* 判断该```key```在```ha->keys```数组当中是否有重复。 这里可以看到```ha->hash_keys```哈希表的用处了，就是用于快速判断是否有重复。

* 如果没有重复，则将当前元素```<key,value>```插入到无通配符的hash表中

**3） 检查```.example.com```类型通配是否在exact hash中**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
wildcard:

    /* wildcard hash */

    k = ngx_hash_strlow(&key->data[skip], &key->data[skip], last - skip);

    k %= ha->hsize;

    if (skip == 1) {

        /* check conflicts in exact hash for ".example.com" */

        name = ha->keys_hash[k].elts;

        if (name) {
            len = last - skip;

            for (i = 0; i < ha->keys_hash[k].nelts; i++) {
                if (len != name[i].len) {
                    continue;
                }

                if (ngx_strncmp(&key->data[1], name[i].data, len) == 0) {
                    return NGX_BUSY;
                }
            }

        } else {
            if (ngx_array_init(&ha->keys_hash[k], ha->temp_pool, 4,
                               sizeof(ngx_str_t))
                != NGX_OK)
            {
                return NGX_ERROR;
            }
        }

        name = ngx_array_push(&ha->keys_hash[k]);
        if (name == NULL) {
            return NGX_ERROR;
        }

        name->len = last - 1;
        name->data = ngx_pnalloc(ha->temp_pool, name->len);
        if (name->data == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(name->data, &key->data[1], name->len);
    }

}
{% endhighlight %}

这里首先求出所要通配的key的hash值，然后看其映射到哪一个桶。如果```skip=1```,即key是属于```.example.com```这种通配类型的时候，需要判断是否该key也存在于```exact hash```中。 如果在```exact hash```中也存在，直接返回```NGX_BUSY```； 否则将其在```ha->keys_hash```中做一个标记，后续禁止往```ha->keys```中插入该key(注意这里并没有插入```ha->keys```)。

**4) 对通配字符串进行相应的格式转换**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
	 if (skip) {

        /*
         * convert "*.example.com" to "com.example.\0"
         *      and ".example.com" to "com.example\0"
         */

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        len = 0;
        n = 0;

        for (i = last - 1; i; i--) {
            if (key->data[i] == '.') {
                ngx_memcpy(&p[n], &key->data[i + 1], len);
                n += len;
                p[n++] = '.';
                len = 0;
                continue;
            }

            len++;
        }

        if (len) {
            ngx_memcpy(&p[n], &key->data[1], len);
            n += len;
        }

        p[n] = '\0';

        hwc = &ha->dns_wc_head;
        keys = &ha->dns_wc_head_hash[k];

    } else {

        /* convert "www.example.*" to "www.example\0" */

        last++;

        p = ngx_pnalloc(ha->temp_pool, last);
        if (p == NULL) {
            return NGX_ERROR;
        }

        ngx_cpystrn(p, key->data, last);

        hwc = &ha->dns_wc_tail;
        keys = &ha->dns_wc_tail_hash[k];
    }

}
{% endhighlight %}
这里对通配字符串进行格式转换：

* ```skip>0```: 表示前置通配字符串。此时```*.example.com```会被转换成```com.example.\0```; ```.example.com```会被转换成 ```com.example\0```。
* 
* ```skip=0```: 表示后置通配符。此时```www.example.*```会被转换成```www.example\0```。

**5） 在wildcard hash中检查是否有冲突**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
	 /* check conflicts in wildcard hash */

    name = keys->elts;

    if (name) {
        len = last - skip;

        for (i = 0; i < keys->nelts; i++) {
            if (len != name[i].len) {
                continue;
            }

            if (ngx_strncmp(key->data + skip, name[i].data, len) == 0) {
                return NGX_BUSY;
            }
        }

    } else {
        if (ngx_array_init(keys, ha->temp_pool, 4, sizeof(ngx_str_t)) != NGX_OK)
        {
            return NGX_ERROR;
        }
    }

    name = ngx_array_push(keys);
    if (name == NULL) {
        return NGX_ERROR;
    }

    name->len = last - skip;
    name->data = ngx_pnalloc(ha->temp_pool, name->len);
    if (name->data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(name->data, key->data + skip, name->len);
}
{% endhighlight %}

注意在```ha->dns_wc_head_hash```或```ha->dns_wc_head_hash```中存放的是没有经过上述```4)步骤```所转换的key。例如对于：

* ```*.example.com```: 存放的在```ha->dns_wc_head_hash```中的值为```example.com```

* ```.example.com```: 存放在```ha->dns_wc_head_hash```中的值也为```example.com```

* ```www.example.*```: 存放在```ha->dns_wc_tail_hash```中的值为```www.example```

**6) 添加转换后的通配字符串到数组中**
{% highlight string %}
ngx_int_t
ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key, void *value,
    ngx_uint_t flags)
{
    /* add to wildcard hash */

    hk = ngx_array_push(hwc);
    if (hk == NULL) {
        return NGX_ERROR;
    }

    hk->key.len = last - 1;
    hk->key.data = p;
    hk->key_hash = 0;
    hk->value = value;

    return NGX_OK;
}
{% endhighlight %}
这里添加到```hk```中的是经过```步骤4)```所转换过的字符串。例如对于：

* ```*.example.com```: 添加到```hk```中为```com.example.```

* ```.example.com```: 添加到```hk```中为```com.example```

* ```www.example.*```: 添加到```hk```中为```www.example```



<br />
<br />

**[参看]**

1. [nginx源码分析之hash的实现](https://www.cnblogs.com/chengxuyuancc/p/3782808.html)

2. [nginx通配符哈希表](https://blog.csdn.net/apelife/article/details/53106140)

<br />
<br />
<br />

