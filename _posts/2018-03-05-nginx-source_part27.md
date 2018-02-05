---
layout: post
title: os/unix/ngx_time.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节主要讲述一下nginx中对底层时间的处理。


<!-- more -->


## 1. 基本概念
在Linux中有几个不同意义上的时间表示，在这里我们将要介绍一下它们。

**1） UTC时间**

UTC(universal time coordinated)又称为世界统一时间、世界标准时间、国际协调时间。UTC是以```原子时```秒长为基础，在时刻上尽量接近于```世界时(UT)```的一种时间计量系统。

```国际原子时```的准确度为每日数纳秒，而```世界时(UT)```的准确度为每日数毫秒。许多应用部门要求时间系统接近```世界时(UT)```，对于这种情况，一种称为```世界协调时```的折衷时标于1972年面世。为确保世界协调时与世界时相差不会超过0.9秒，在有需要的情况下会在协调世界时内加上正或负闺秒。
<pre>
从名字上来看，UTC时间是为了协调UT时间而诞生的。	
</pre>

**2) UT时间**

UT(universal time)又称为世界时间，是以本初子午线的平子夜起算的平太阳时。又称```格林尼治平时```或```格林尼治时间```。各地的```地方平时```与世界时只差等于该地的地理经度。1960年以前曾作为基本时间计量系统被广泛应用。由于地球自转速度变化的影响，它不是一种均匀的时间系统。后来世界时被原子时所替代，但是在日常生活、天文导航、大地观测和宇宙飞行等方面仍属必须；同时，世界时反应地球自转速率的变化，是地球自转参数之一，仍为天文学和地球物理学的基本资料。

**3) GMT时间**

GMT(GreenWich Mean Time)又称格林尼治标准时间，是指位于伦敦郊区的皇家格林尼治天文台的标准时间，因为本初子午线被定义在通过那里的经线。理论上来说，格林尼治标准时间的正午是指当太阳横穿格林尼治子午线时的时间。由于地球在它的椭圆轨道里的运动速度不均匀，这个时刻可能和实际太阳时相差16分钟。地球每天的自转是有些不规则的，而且正在缓慢减速。所以格林尼治标准时间已经不再作为标准时间使用。现在的标准时间——世界协调时(UTC)——由原子时钟提供。自1924年2月5日开始， 格林尼治天文台每隔一小时会向全世界发放调时信息，而UTC是基于标准的GMT提供的准确时间。
<pre>
为了方便，在不需要精确到秒的情况下，通常将GMT和UTC视作等同。但UTC更加科学精确，它是以原子时作为基础，在时刻上尽量接近世界时的一种
时间计量系统。它的出现是现代社会对于精确计时的需要。
</pre>



  

## 2. os/unix/ngx_time.h头文件
头文件源代码如下：
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_TIME_H_INCLUDED_
#define _NGX_TIME_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef ngx_rbtree_key_t      ngx_msec_t;
typedef ngx_rbtree_key_int_t  ngx_msec_int_t;

typedef struct tm             ngx_tm_t;

#define ngx_tm_sec            tm_sec
#define ngx_tm_min            tm_min
#define ngx_tm_hour           tm_hour
#define ngx_tm_mday           tm_mday
#define ngx_tm_mon            tm_mon
#define ngx_tm_year           tm_year
#define ngx_tm_wday           tm_wday
#define ngx_tm_isdst          tm_isdst

#define ngx_tm_sec_t          int
#define ngx_tm_min_t          int
#define ngx_tm_hour_t         int
#define ngx_tm_mday_t         int
#define ngx_tm_mon_t          int
#define ngx_tm_year_t         int
#define ngx_tm_wday_t         int


#if (NGX_HAVE_GMTOFF)
#define ngx_tm_gmtoff         tm_gmtoff
#define ngx_tm_zone           tm_zone
#endif


#if (NGX_SOLARIS)

#define ngx_timezone(isdst) (- (isdst ? altzone : timezone) / 60)

#else

#define ngx_timezone(isdst) (- (isdst ? timezone + 3600 : timezone) / 60)

#endif


void ngx_timezone_update(void);
void ngx_localtime(time_t s, ngx_tm_t *tm);
void ngx_libc_localtime(time_t s, struct tm *tm);
void ngx_libc_gmtime(time_t s, struct tm *tm);

#define ngx_gettimeofday(tp)  (void) gettimeofday(tp, NULL);
#define ngx_msleep(ms)        (void) usleep(ms * 1000)
#define ngx_sleep(s)          (void) sleep(s)


#endif /* _NGX_TIME_H_INCLUDED_ */

{% endhighlight %}








<br />
<br />

**[参看]**:

1. [关于linux的时间表示函数：localtime、gmtime、ctime、strftime](https://www.cnblogs.com/maowen/p/5073084.html)

2. [Linux/CentOS下的CST和UTC时间的区别以及不一致的解决方法](https://www.cnblogs.com/aguncn/p/4316093.html)

3. [UTC和GMT什么关系？](https://www.zhihu.com/question/27052407)

<br />
<br />
<br />

