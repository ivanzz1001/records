---
layout: post
title: os/unix/ngx_linux_init.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


linux也是属于类Unix操作系统，但是其具有一些特殊的特性，因此这里在初始化时会专门针对Linux做一些特殊的处理。


<!-- more -->

## 1. os/unix/ngx_linux_init.c源文件

文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


u_char  ngx_linux_kern_ostype[50];
u_char  ngx_linux_kern_osrelease[50];


static ngx_os_io_t ngx_linux_io = {
    ngx_unix_recv,
    ngx_readv_chain,
    ngx_udp_unix_recv,
    ngx_unix_send,
    ngx_udp_unix_send,
#if (NGX_HAVE_SENDFILE)
    ngx_linux_sendfile_chain,
    NGX_IO_SENDFILE
#else
    ngx_writev_chain,
    0
#endif
};


ngx_int_t
ngx_os_specific_init(ngx_log_t *log)
{
    struct utsname  u;

    if (uname(&u) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "uname() failed");
        return NGX_ERROR;
    }

    (void) ngx_cpystrn(ngx_linux_kern_ostype, (u_char *) u.sysname,
                       sizeof(ngx_linux_kern_ostype));

    (void) ngx_cpystrn(ngx_linux_kern_osrelease, (u_char *) u.release,
                       sizeof(ngx_linux_kern_osrelease));

    ngx_os_io = ngx_linux_io;

    return NGX_OK;
}


void
ngx_os_specific_status(ngx_log_t *log)
{
    ngx_log_error(NGX_LOG_NOTICE, log, 0, "OS: %s %s",
                  ngx_linux_kern_ostype, ngx_linux_kern_osrelease);
}
{% endhighlight %}


### 1.1 Linux操作系统环境下IO操作
{% highlight string %}
static ngx_os_io_t ngx_linux_io = {
    ngx_unix_recv,
    ngx_readv_chain,
    ngx_udp_unix_recv,
    ngx_unix_send,
    ngx_udp_unix_send,
#if (NGX_HAVE_SENDFILE)
    ngx_linux_sendfile_chain,
    NGX_IO_SENDFILE
#else
    ngx_writev_chain,
    0
#endif
};
{% endhighlight %}
这里```NGX_HAVE_SENDFILE```为1，```NGX_IO_SENDFILE```也在ngx_os.h头文件中被定义为1，表明当前针对```sendfile```有专用的高效发送函数。

## 1.2 获得操作系统内核类型及版本号

{% highlight string %}
ngx_int_t
ngx_os_specific_init(ngx_log_t *log)
{
    struct utsname  u;

    if (uname(&u) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "uname() failed");
        return NGX_ERROR;
    }

    (void) ngx_cpystrn(ngx_linux_kern_ostype, (u_char *) u.sysname,
                       sizeof(ngx_linux_kern_ostype));

    (void) ngx_cpystrn(ngx_linux_kern_osrelease, (u_char *) u.release,
                       sizeof(ngx_linux_kern_osrelease));

    ngx_os_io = ngx_linux_io;

    return NGX_OK;
}
{% endhighlight %}

我们也可以通过如下命令来进行获取：
<pre>
# uname -s

# uname -r
</pre>

针对当前我们当前的环境，写如下程序进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>



int main(int argc,char *argv[])
{
    struct utsname  u;

    if (uname(&u) == -1) {
      return -1;
    }


    printf("sysname: %s\n",u.sysname);
    printf("release: %s\n",u.release);

    return 0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test1 test1.c
# ./test1
sysname: Linux
release: 4.10.0-35-generic
</pre>



## 1.3 打印操作系统类型及版本相关信息
{% highlight string %}
void
ngx_os_specific_status(ngx_log_t *log)
{
    ngx_log_error(NGX_LOG_NOTICE, log, 0, "OS: %s %s",
                  ngx_linux_kern_ostype, ngx_linux_kern_osrelease);
}
{% endhighlight %}


<br />
<br />
<br />

