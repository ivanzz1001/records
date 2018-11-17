---
layout: post
title: core/ngx_queue.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章讲述一下nginx中队列的实现。


<!-- more -->


## 1. 函数ngx_queue_middle()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * find the middle queue element if the queue has odd number of elements
 * or the first element of the queue's second part otherwise
 */

ngx_queue_t *
ngx_queue_middle(ngx_queue_t *queue)
{
    ngx_queue_t  *middle, *next;

    middle = ngx_queue_head(queue);

    if (middle == ngx_queue_last(queue)) {
        return middle;
    }

    next = ngx_queue_head(queue);

    for ( ;; ) {
        middle = ngx_queue_next(middle);

        next = ngx_queue_next(next);

        if (next == ngx_queue_last(queue)) {
            return middle;
        }

        next = ngx_queue_next(next);

        if (next == ngx_queue_last(queue)) {
            return middle;
        }
    }
}

{% endhighlight %}
本函数较为简单，一个基本的思想是附设两个指针， 第一个指针以```1倍速```往后移动， 第二个指针以```2倍速```往后移动，直到第二个指针到达对列尾， 此时第一个```1倍速```指针刚好移动到中间。如下图所示：

![ngx-queue-middle](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_queue_middle.jpg)

<pre>
上面每经过一次大循环， middle向后移动一步， 而next向后移动两步。假设经过了x轮循环， 则middle移动到的位置为: 1+x； 
而next移动到的位置为: 1+2x。此时有两种情况：

1) 1+2x刚好是对列的末尾， 则对列个数是奇数个， 此时 1+x 刚好为中间那个元素

2) 1+2x还差一格到对列的末尾， 则需要再进行半步循环操作， 则next移动到 1+2x+1这个末尾时， middle移动到 1+x+1。 此种
   情况下对列总的长度为 2+2x， 对列长度为偶数。此种情况刚好对应上面for循环的前半部分 
</pre>


## 2. 函数ngx_queue_sort()
{% highlight string %}
/* the stable insertion sort */

void
ngx_queue_sort(ngx_queue_t *queue,
    ngx_int_t (*cmp)(const ngx_queue_t *, const ngx_queue_t *))
{
    ngx_queue_t  *q, *prev, *next;

    q = ngx_queue_head(queue);

    if (q == ngx_queue_last(queue)) {
        return;
    }

    for (q = ngx_queue_next(q); q != ngx_queue_sentinel(queue); q = next) {

        prev = ngx_queue_prev(q);
        next = ngx_queue_next(q);

        ngx_queue_remove(q);

        do {
            if (cmp(prev, q) <= 0) {
                break;
            }

            prev = ngx_queue_prev(prev);

        } while (prev != ngx_queue_sentinel(queue));

        ngx_queue_insert_after(prev, q);
    }
}
{% endhighlight %}
此函数较为简单，直接采用稳定的的插入排序算法来进行排序。







<br />
<br />

**[参看]**





<br />
<br />
<br />

