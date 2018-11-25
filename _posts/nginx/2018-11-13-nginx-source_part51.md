---
layout: post
title: core/ngx_parse_time.c(h)文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们介绍一下ngx_parse_time.c(h)文件，其主要用于将字符串解析称为time_t类型。




<!-- more -->

   
## 1. ngx_parse_time.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PARSE_TIME_H_INCLUDED_
#define _NGX_PARSE_TIME_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


time_t ngx_parse_http_time(u_char *value, size_t len);

/* compatibility */
#define ngx_http_parse_time(value, len)  ngx_parse_http_time(value, len)


#endif /* _NGX_PARSE_TIME_H_INCLUDED_ */
{% endhighlight %}
头文件声明了解析函数的原型。

## 2. ngx_parse_time.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_uint_t  mday[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

time_t
ngx_parse_http_time(u_char *value, size_t len)
{
    u_char      *p, *end;
    ngx_int_t    month;
    ngx_uint_t   day, year, hour, min, sec;
    uint64_t     time;
    enum {
        no = 0,
        rfc822,   /* Tue, 10 Nov 2002 23:50:13   */
        rfc850,   /* Tuesday, 10-Dec-02 23:50:13 */
        isoc      /* Tue Dec 10 23:50:13 2002    */
    } fmt;

    fmt = 0;
    end = value + len;

#if (NGX_SUPPRESS_WARN)
    day = 32;
    year = 2038;
#endif

    for (p = value; p < end; p++) {
        if (*p == ',') {
            break;
        }

        if (*p == ' ') {
            fmt = isoc;
            break;
        }
    }

    for (p++; p < end; p++)
        if (*p != ' ') {
            break;
        }

    if (end - p < 18) {
        return NGX_ERROR;
        }

    if (fmt != isoc) {
        if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9') {
            return NGX_ERROR;
        }

        day = (*p - '0') * 10 + *(p + 1) - '0';
        p += 2;

        if (*p == ' ') {
            if (end - p < 18) {
                return NGX_ERROR;
            }
            fmt = rfc822;

        } else if (*p == '-') {
            fmt = rfc850;

        } else {
            return NGX_ERROR;
        }

        p++;
    }

    switch (*p) {

    case 'J':
        month = *(p + 1) == 'a' ? 0 : *(p + 2) == 'n' ? 5 : 6;
        break;

    case 'F':
        month = 1;
        break;

    case 'M':
        month = *(p + 2) == 'r' ? 2 : 4;
        break;

    case 'A':
        month = *(p + 1) == 'p' ? 3 : 7;
        break;

    case 'S':
        month = 8;
        break;

    case 'O':
        month = 9;
        break;

    case 'N':
        month = 10;
        break;

    case 'D':
        month = 11;
        break;

    default:
        return NGX_ERROR;
    }

    p += 3;

    if ((fmt == rfc822 && *p != ' ') || (fmt == rfc850 && *p != '-')) {
        return NGX_ERROR;
    }

    p++;

    if (fmt == rfc822) {
        if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9'
            || *(p + 2) < '0' || *(p + 2) > '9'
            || *(p + 3) < '0' || *(p + 3) > '9')
        {
            return NGX_ERROR;
        }

        year = (*p - '0') * 1000 + (*(p + 1) - '0') * 100
               + (*(p + 2) - '0') * 10 + *(p + 3) - '0';
        p += 4;

    } else if (fmt == rfc850) {
        if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9') {
            return NGX_ERROR;
        }

        year = (*p - '0') * 10 + *(p + 1) - '0';
        year += (year < 70) ? 2000 : 1900;
        p += 2;
    }

    if (fmt == isoc) {
        if (*p == ' ') {
            p++;
        }

        if (*p < '0' || *p > '9') {
            return NGX_ERROR;
        }

        day = *p++ - '0';

        if (*p != ' ') {
            if (*p < '0' || *p > '9') {
                return NGX_ERROR;
            }

            day = day * 10 + *p++ - '0';
        }

        if (end - p < 14) {
            return NGX_ERROR;
        }
    }

    if (*p++ != ' ') {
        return NGX_ERROR;
    }

    if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9') {
        return NGX_ERROR;
    }

    hour = (*p - '0') * 10 + *(p + 1) - '0';
    p += 2;

    if (*p++ != ':') {
        return NGX_ERROR;
    }

    if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9') {
        return NGX_ERROR;
    }

    min = (*p - '0') * 10 + *(p + 1) - '0';
    p += 2;

    if (*p++ != ':') {
        return NGX_ERROR;
    }

    if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9') {
        return NGX_ERROR;
    }

    sec = (*p - '0') * 10 + *(p + 1) - '0';

    if (fmt == isoc) {
        p += 2;

        if (*p++ != ' ') {
            return NGX_ERROR;
        }

        if (*p < '0' || *p > '9' || *(p + 1) < '0' || *(p + 1) > '9'
            || *(p + 2) < '0' || *(p + 2) > '9'
            || *(p + 3) < '0' || *(p + 3) > '9')
        {
            return NGX_ERROR;
        }

        year = (*p - '0') * 1000 + (*(p + 1) - '0') * 100
               + (*(p + 2) - '0') * 10 + *(p + 3) - '0';
    }

    if (hour > 23 || min > 59 || sec > 59) {
        return NGX_ERROR;
    }

    if (day == 29 && month == 1) {
        if ((year & 3) || ((year % 100 == 0) && (year % 400) != 0)) {
            return NGX_ERROR;
        }

    } else if (day > mday[month]) {
        return NGX_ERROR;
    }

    /*
     * shift new year to March 1 and start months from 1 (not 0),
     * it is needed for Gauss' formula
     */

    if (--month <= 0) {
        month += 12;
        year -= 1;
    }

    /* Gauss' formula for Gregorian days since March 1, 1 BC */

    time = (uint64_t) (
            /* days in years including leap years since March 1, 1 BC */

            365 * year + year / 4 - year / 100 + year / 400

            /* days before the month */

            + 367 * month / 12 - 30

            /* days before the day */

            + day - 1

            /*
             * 719527 days were between March 1, 1 BC and March 1, 1970,
             * 31 and 28 days were in January and February 1970
             */

            - 719527 + 31 + 28) * 86400 + hour * 3600 + min * 60 + sec;

#if (NGX_TIME_T_SIZE <= 4)

    if (time > 0x7fffffff) {
        return NGX_ERROR;
    }

#endif

    return (time_t) time;
}
{% endhighlight %}

这里我们首先介绍一下```time_t```类型，其一般被定义为long或者int64类型。如果是long类型的话，它所表示的时间不能晚于2038年1月18日19时14分07秒。这里支持解析三种日期格式的时间：

* rfc822: 格式为 'Tue, 10 Nov 2002 23:50:13'

* rfc850: 格式为 'Tuesday, 10-Dec-02 23:50:13'

* isoc: 格式为 'Tue Dec 10 23:50:13 2002' 


注意，这里我们是从1970年3月1日作为起始来计算的，并将这一天作为一年的第一天。



<br />
<br />

**[参看]**

1. [C语言中time_t数据类型详细介绍](https://www.cnblogs.com/joeblackzqq/archive/2012/07/10/2584140.html)

2. [Gregorian calendar](https://en.wikipedia.org/wiki/Gregorian_calendar)

<br />
<br />
<br />

