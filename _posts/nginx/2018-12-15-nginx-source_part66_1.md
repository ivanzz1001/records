---
layout: post
title: core/ngx_times.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


nginx中很多地方都需要用到时间戳信息，但是如果每一次都直接调用系统函数来获取，虽然可以保证时间的精确性，但是却会严重降低系统的性能。考虑到Nginx中很多地方用到的时间戳并不需要十分精确，从系统性能方面考虑，nginx采用缓存时间戳的方法来处理。


<!-- more -->

## 1. ngx_time_t数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_TIMES_H_INCLUDED_
#define _NGX_TIMES_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    time_t      sec;
    ngx_uint_t  msec;
    ngx_int_t   gmtoff;
} ngx_time_t;
{% endhighlight %}
```ngx_time_t```用于当前的时间：

* sec: 当前时间的秒数(UTC时间）

* msec: 当前时间的毫秒数（UTC时间）

* gmtoff: 指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负```分钟```数。这一点与```struct tm```中的```tm_gmtoff```是不一样的
{% highlight string %}
struct tm 
{ 　
int tm_sec;		 /* 秒–取值区间为[0,59] */ 　　
int tm_min; 		 /* 分 - 取值区间为[0,59] */ 　　
int tm_hour; 	         /* 时 - 取值区间为[0,23] */ 　　
int tm_mday;		 /* 一个月中的日期 - 取值区间为[1,31] */ 　
int tm_mon;		 /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */ 
int tm_year; 	         /* 年份，其值从1900开始 */ 　
int tm_wday; 	         /* 星期–取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */ 　
int tm_yday; 	         /* 从每年的1月1日开始的天数–取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 */ 　
int tm_isdst; 	         /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。*/ 　
long int tm_gmtoff;	 /*指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数*/ 　　
const char *tm_zone;     /*当前时区的名字(与环境变量TZ有关)*/ 　
}; 
{% endhighlight %}


## 2. 相关函数声明
{% highlight string %}
//用于初始化nginx时间缓存
void ngx_time_init(void);

//更新nginx时间缓存
void ngx_time_update(void);

//通过nginx中产生信号来更新时间(setitimer()会周期性的产生超时，然后产生SIGALARM信号；
//对于其他信号，默认也会更新nginx缓存时间)
void ngx_time_sigsafe_update(void);

//将时间t格式化到buf中，形成http格式的时间戳信息
u_char *ngx_http_time(u_char *buf, time_t t);


//将时间t格式化到buf中，形成cookie格式的时间戳信息
u_char *ngx_http_cookie_time(u_char *buf, time_t t);

//将时间t转化成gm时间
void ngx_gmtime(time_t t, ngx_tm_t *tp);

//将当前本地时间加上when，形成新的时间返回
time_t ngx_next_time(time_t when);
#define ngx_next_time_n      "mktime()"
{% endhighlight %}


## 3. 相关全局变量声明
{% highlight string %}
extern volatile ngx_time_t  *ngx_cached_time;

#define ngx_time()           ngx_cached_time->sec
#define ngx_timeofday()      (ngx_time_t *) ngx_cached_time

extern volatile ngx_str_t    ngx_cached_err_log_time;
extern volatile ngx_str_t    ngx_cached_http_time;
extern volatile ngx_str_t    ngx_cached_http_log_time;
extern volatile ngx_str_t    ngx_cached_http_log_iso8601;
extern volatile ngx_str_t    ngx_cached_syslog_time;

/*
 * milliseconds elapsed since epoch and truncated to ngx_msec_t,
 * used in event timers
 */
extern volatile ngx_msec_t  ngx_current_msec;
{% endhighlight %}

* ngx_cached_time: 当前nginx所缓存的时间

* ngx_time(): 获取当前Nginx缓存时间的秒数（精度为秒级）

* ngx_timeofday(): 获取当前nginx缓存的时间

* ngx_cached_err_log_time: 当前缓存的错误日志时间

* ngx_cached_http_time: 当前缓存的http时间

* ngx_cached_http_log_time: 当前缓存的http log时间

* ngx_cached_ttp_log_iso8601: 当前缓存的http log的ios8601格式的时间

* ngx_cached_syslog_time: 当前缓存的syslog格式的时间

* ngx_current_msec： 当前时间的毫秒数。只用作nginx中的相对时间的计算，因为```msec```所能保存的时间范围较小。




<br />
<br />

**[参看]**




<br />
<br />
<br />

