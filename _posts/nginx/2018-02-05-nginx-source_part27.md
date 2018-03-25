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

* **UTC时间**

UTC(universal time coordinated)又称为世界统一时间、世界标准时间、国际协调时间。UTC是以```原子时```秒长为基础，在时刻上尽量接近于```世界时(UT)```的一种时间计量系统。

```国际原子时```的准确度为每日数纳秒，而```世界时(UT)```的准确度为每日数毫秒。许多应用部门要求时间系统接近```世界时(UT)```，对于这种情况，一种称为```世界协调时```的折衷时标于1972年面世。为确保世界协调时与世界时相差不会超过0.9秒，在有需要的情况下会在协调世界时内加上正或负闰秒。
<pre>
从名字上来看，UTC时间是为了协调UT时间而诞生的。	
</pre>

* **UT时间**

UT(universal time)又称为世界时间，是以本初子午线的平子夜起算的平太阳时。又称```格林尼治平时```或```格林尼治时间```。各地的```地方平时```与世界时只差等于该地的地理经度。1960年以前曾作为基本时间计量系统被广泛应用。由于地球自转速度变化的影响，它不是一种均匀的时间系统。后来世界时被原子时所替代，但是在日常生活、天文导航、大地观测和宇宙飞行等方面仍属必须；同时，世界时反应地球自转速率的变化，是地球自转参数之一，仍为天文学和地球物理学的基本资料。

* **GMT时间**

```GMT```(GreenWich Mean Time)又称格林尼治标准时间，是指位于伦敦郊区的皇家格林尼治天文台的标准时间，因为本初子午线被定义在通过那里的经线。理论上来说，格林尼治标准时间的正午是指当太阳横穿格林尼治子午线时的时间。由于地球在它的椭圆轨道里的运动速度不均匀，这个时刻可能和实际太阳时相差16分钟。地球每天的自转是有些不规则的，而且正在缓慢减速。所以格林尼治标准时间已经不再作为标准时间使用。现在的标准时间——世界协调时(UTC)——由原子时钟提供。自1924年2月5日开始， 格林尼治天文台每隔一小时会向全世界发放调时信息，而UTC是基于标准的GMT提供的准确时间。
<pre>
为了方便，在不需要精确到秒的情况下，通常将GMT和UTC视作等同。但UTC更加科学精确，它是以原子时作为基础，在时刻上尽量接近世界时的一种
时间计量系统。它的出现是现代社会对于精确计时的需要。
</pre>

* **日历时间(Calendar Time)**

```Calendar Time```是用“从一个标准时间点到此时的时间经过的秒数”来表示的时间。无论哪一个时区，在同一时刻对同一个标准时间点来说，日历时间都是一样的。日历时间返回自1970-1-1 00：00：00以来所经过的秒数累计值。因此，不论服务器是在哪个时区（国家），同一时刻，日历时间总是一样的，都是相对于1970-1-1 00:00:00以来的秒数，理解这一点很重要。

* **时区**

为了克服时间上的混乱，1884年在华盛顿召开的一次国际经度会议（又称国际子午线会议）上，规定将全球划分为24个时区（东、西各12个时区）。规定英国（格林尼治天文台旧址）为中时区（零时区）、东1-12区、西1-12区。每个时区横跨经度15度，时间正好是1小时。最后的东、西第12区各跨经度7.5度，以东西经180度为界。每个时区的中央经线上的时间就是这个时区内统一采用的时间，称为区时，相邻两个时区的时间相差1小时。例如：中国东8区的时间总比泰国东7区的时间早1小时，而比日本东9区的时间迟1小时。因此，出过旅行的人，必须随时调整自己的手表，才能和当地时间一致。

* **夏令时**

所谓【夏日节约时间】Daylight Saving Time(简称DST)，是指在夏天太阳升起的比较早时，将时钟拨快一小时，以提早日光的使用，在英国则称为夏令时间(Summer Time)。这个构想于1784年由美国班杰明·富兰克林提出来，1915年德国成为第一个正式实施夏令日光节约时间的国家，以削减灯光照明和耗电开支。自此以后，全球以欧洲和北美为主的约70个国家都引用这个做法。目前被划分成两个时区的印度也正在商讨是否全国该统一实行夏令日光节约时间。欧洲手机上也有很多GSM系统的基地台，除了会传送当地时间外也包括夏令日光节约时间，作为手机的时间标准，使用者可以自行决定要开启或者关闭。


  

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

下面我们对上述头文件进行简单的分析：

### 2.1 相关数据类型定义
{% highlight string %}
typedef ngx_rbtree_key_t      ngx_msec_t;
typedef ngx_rbtree_key_int_t  ngx_msec_int_t;
{% endhighlight %}
如上数据类型在src/core/ngx_rbtree.h头文件中定义为如下：
<pre>
typedef ngx_uint_t  ngx_rbtree_key_t;
typedef ngx_int_t   ngx_rbtree_key_int_t;
</pre>
之所以这么定义， 是因为后续nginx中用到的定时器都是存放于红黑树结构中。

### 2.2 字段tm_gmtoff与tm_zone
{% highlight string %}
#if (NGX_HAVE_GMTOFF)
#define ngx_tm_gmtoff         tm_gmtoff
#define ngx_tm_zone           tm_zone
#endif
{% endhighlight %}
在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_GMTOFF
#define NGX_HAVE_GMTOFF  1
#endif
</pre>
因此，当前我们编译系统是支持tm.tm_gmtoff与tm.tm_zone字段的。

### 2.3 获得时区值
{% highlight string %}
#if (NGX_SOLARIS)

#define ngx_timezone(isdst) (- (isdst ? altzone : timezone) / 60)

#else

#define ngx_timezone(isdst) (- (isdst ? timezone + 3600 : timezone) / 60)

#endif
{% endhighlight %}
在Linux操作系统下的```time.h```头文件中，一般会有一个timezone全局变量，用于记录当前所设置的timezone值。请参看如下代码：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main(int argc,char *argv[])
{
    tzset();
    printf("zone: %d\n",timezone);

    #if 0
      printf("altzone: %d\n", altzone);
    #endif

    printf("time: %lu\n",(unsigned long)time(NULL));
    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c

# export TZ='Asia/Shanghai'
# ./test
zone: -28800                  //  28800/60/60=8，即东8区
time: 1517923134

# export TZ='America/New_York'   
# ./test
zone: 18000                  // 18000/60/60 = 5，即西5区
time: 1517923508

# export TZ='Europe/Moscow'
# ./test
zone: -10800
time: 1517923235
</pre>
注意```tzset()```函数会依赖与当前的```TZ```环境变量来设置当前运行程序的timezone值。

### 2.4 相关函数声明
{% highlight string %}
void ngx_timezone_update(void);
void ngx_localtime(time_t s, ngx_tm_t *tm);
void ngx_libc_localtime(time_t s, struct tm *tm);
void ngx_libc_gmtime(time_t s, struct tm *tm);

#define ngx_gettimeofday(tp)  (void) gettimeofday(tp, NULL);
#define ngx_msleep(ms)        (void) usleep(ms * 1000)
#define ngx_sleep(s)          (void) sleep(s)
{% endhighlight %}



## 3. os/unix/ngx_time.c源文件

### 3.1 函数ngx_timezone_update()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * FreeBSD does not test /etc/localtime change, however, we can workaround it
 * by calling tzset() with TZ and then without TZ to update timezone.
 * The trick should work since FreeBSD 2.1.0.
 *
 * Linux does not test /etc/localtime change in localtime(),
 * but may stat("/etc/localtime") several times in every strftime(),
 * therefore we use it to update timezone.
 *
 * Solaris does not test /etc/TIMEZONE change too and no workaround available.
 */

void
ngx_timezone_update(void)
{
#if (NGX_FREEBSD)

    if (getenv("TZ")) {
        return;
    }

    putenv("TZ=UTC");

    tzset();

    unsetenv("TZ");

    tzset();

#elif (NGX_LINUX)
    time_t      s;
    struct tm  *t;
    char        buf[4];

    s = time(0);

    t = localtime(&s);

    strftime(buf, 4, "%H", t);

#endif
}
{% endhighlight %}
ngx_timezone_update()函数用于更新时区。在不同的操作系统上，更新时区的方法也有些许不同：

* **FreeBsd操作系统**： FreeBSD并不会检测```/etc/localtime```文件的变化。然而，我们可以通过调用tzset()的方式来达到更新时区。具体操作如下：
<pre>
// 1) 设置TZ环境变量
    putenv("TZ=UTC")

// 2) 调用tzset()
    tzset()

// 3) 清空TZ环境变量
    unsetenv("TZ")

// 4) 调用tzset()
    tzset()
</pre>
这个更新时区的小技巧从FreeBSD 2.1.0版本开始有效。


* **Linux操作系统**: Linux操作系统在调用localtime()的时候也并不会检测/etc/localtime的更改情况，但是在每一次调用strftime()函数的时候会多次stat("/etc/localtime")，因此我们可以此来更新timezone。在ngx_auto_headers.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_LINUX
#define NGX_LINUX  1
#endif
</pre>

* **Solaris操作系统**: 对于Solaris操作系统，也并不会检测/etc/localtime文件的变化情况，到目前为止并没有一些技巧来感知到其变化情况。

### 3.2 函数ngx_localtime()
{% highlight string %}
void
ngx_localtime(time_t s, ngx_tm_t *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) localtime_r(&s, tm);

#else
    ngx_tm_t  *t;

    t = localtime(&s);
    *tm = *t;

#endif

    tm->ngx_tm_mon++;
    tm->ngx_tm_year += 1900;
}
{% endhighlight %}

我们在ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_HAVE_LOCALTIME_R
#define NGX_HAVE_LOCALTIME_R  1
#endif
</pre>
函数localtime()不是线程安全的，而localtime_r()是属于线程安全的。这里当执行else分支时，ngx_localtime()函数也是线程不安全的。
<pre>
ngx_localtime()函数将日历时间转换为本地时间，处理后的结果值便于人们阅读，因此在ngx_tm_year及ngx_tm_mon字段上做了相应的处理。
</pre>

### 3.3 函数ngx_libc_localtime()
{% highlight string %}
void
ngx_libc_localtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) localtime_r(&s, tm);

#else
    struct tm  *t;

    t = localtime(&s);
    *tm = *t;

#endif
}
{% endhighlight %}
函数较为简单，这里不再赘述。

### 3.4 函数ngx_libc_gmtime()
{% highlight string %}
void
ngx_libc_gmtime(time_t s, struct tm *tm)
{
#if (NGX_HAVE_LOCALTIME_R)
    (void) gmtime_r(&s, tm);

#else
    struct tm  *t;

    t = gmtime(&s);
    *tm = *t;

#endif
}
{% endhighlight %}
本函数将日历时间```time_t s```转换为UTC时间(格林尼治标准时间)。

<br />
<br />

**[参看]**:

1. [关于linux的时间表示函数：localtime、gmtime、ctime、strftime](https://www.cnblogs.com/maowen/p/5073084.html)

2. [Linux/CentOS下的CST和UTC时间的区别以及不一致的解决方法](https://www.cnblogs.com/aguncn/p/4316093.html)

3. [UTC和GMT什么关系？](https://www.zhihu.com/question/27052407)

4. [世界协调时间 (UTC)](http://zh.thetimenow.com/utc/coordinated_universal_time)

5. [linux应用time和timezone](https://www.cnblogs.com/embedded-linux/p/7087558.html)

<br />
<br />
<br />

