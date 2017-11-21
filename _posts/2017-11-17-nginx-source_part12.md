---
layout: post
title: os/unix/ngx_errno.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要介绍一下nginx中对ngx errno的的处理。

<!-- more -->


## 1. os/unix/ngx_errno.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ERRNO_H_INCLUDED_
#define _NGX_ERRNO_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef int               ngx_err_t;

#define NGX_EPERM         EPERM
#define NGX_ENOENT        ENOENT
#define NGX_ENOPATH       ENOENT
#define NGX_ESRCH         ESRCH
#define NGX_EINTR         EINTR
#define NGX_ECHILD        ECHILD
#define NGX_ENOMEM        ENOMEM
#define NGX_EACCES        EACCES
#define NGX_EBUSY         EBUSY
#define NGX_EEXIST        EEXIST
#define NGX_EEXIST_FILE   EEXIST
#define NGX_EXDEV         EXDEV
#define NGX_ENOTDIR       ENOTDIR
#define NGX_EISDIR        EISDIR
#define NGX_EINVAL        EINVAL
#define NGX_ENFILE        ENFILE
#define NGX_EMFILE        EMFILE
#define NGX_ENOSPC        ENOSPC
#define NGX_EPIPE         EPIPE
#define NGX_EINPROGRESS   EINPROGRESS
#define NGX_ENOPROTOOPT   ENOPROTOOPT
#define NGX_EOPNOTSUPP    EOPNOTSUPP
#define NGX_EADDRINUSE    EADDRINUSE
#define NGX_ECONNABORTED  ECONNABORTED
#define NGX_ECONNRESET    ECONNRESET
#define NGX_ENOTCONN      ENOTCONN
#define NGX_ETIMEDOUT     ETIMEDOUT
#define NGX_ECONNREFUSED  ECONNREFUSED
#define NGX_ENAMETOOLONG  ENAMETOOLONG
#define NGX_ENETDOWN      ENETDOWN
#define NGX_ENETUNREACH   ENETUNREACH
#define NGX_EHOSTDOWN     EHOSTDOWN
#define NGX_EHOSTUNREACH  EHOSTUNREACH
#define NGX_ENOSYS        ENOSYS
#define NGX_ECANCELED     ECANCELED
#define NGX_EILSEQ        EILSEQ
#define NGX_ENOMOREFILES  0
#define NGX_ELOOP         ELOOP
#define NGX_EBADF         EBADF

#if (NGX_HAVE_OPENAT)
#define NGX_EMLINK        EMLINK
#endif

#if (__hpux__)
#define NGX_EAGAIN        EWOULDBLOCK
#else
#define NGX_EAGAIN        EAGAIN
#endif


#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err


u_char *ngx_strerror(ngx_err_t err, u_char *errstr, size_t size);
ngx_int_t ngx_strerror_init(void);


#endif /* _NGX_ERRNO_H_INCLUDED_ */
{% endhighlight %}


### 1.1 相关宏定义

头文件主要是对相应的错误进行了重新的定义。在ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_HAVE_OPENAT
#define NGX_HAVE_OPENAT  1
#endif
</pre>
因此，这里会将NGX_EMLINK定义为EMLINK。当前我们并没有```__hpux__```定义，因此这里NGX_AGAIN被定义为EAGAIN。

<br />

### 1.2 相关函数的声明
{% highlight string %}
#define ngx_errno                  errno
#define ngx_socket_errno           errno
#define ngx_set_errno(err)         errno = err
#define ngx_set_socket_errno(err)  errno = err


u_char *ngx_strerror(ngx_err_t err, u_char *errstr, size_t size);
ngx_int_t ngx_strerror_init(void);
{% endhighlight %}
这里由于操作系统提供的:
<pre>
char *strerror(int errnum);

int strerror_r(int errnum, char *buf, size_t buflen);
           /* XSI-compliant */

char *strerror_r(int errnum, char *buf, size_t buflen);
</pre>
都不是```异步信号安全```的，因此这里实现自身的ngx_strerror()函数。

这里我们来分析一下原生的strerror()函数：
{% highlight string %}
// glibc-2.14\string\strerror.c
#include <errno.h>
 
/* Return a string describing the errno code in ERRNUM.
   The storage is good only until the next call to strerror.
   Writing to the storage causes undefined behavior.  */
libc_freeres_ptr (static char *buf);
 
char *
strerror (int errnum)
{
	char *ret = __strerror_r (errnum, NULL, 0);
	int saved_errno;

	if (__builtin_expect (ret != NULL, 1))
		return ret;

	saved_errno = errno;
	if (buf == NULL)
		buf = malloc (1024);
	
	__set_errno (saved_errno);
	if (buf == NULL)
		return _("Unknown error");
	
	return __strerror_r (errnum, buf, 1024);
}

// glibc-2.14\string\_strerror.c
/* Return a string describing the errno code in ERRNUM.  */
char *
__strerror_r (int errnum, char *buf, size_t buflen)
{
	if (__builtin_expect (errnum < 0 || errnum >= _sys_nerr_internal
		|| _sys_errlist_internal[errnum] == NULL, 0))
	{
		/* Buffer we use to print the number in.  For a maximum size for
		`int' of 8 bytes we never need more than 20 digits.  */
		char numbuf[21];
		const char *unk = _("Unknown error ");
		size_t unklen = strlen (unk);
		char *p, *q;
		bool negative = errnum < 0;

		numbuf[20] = '\0';
		p = _itoa_word (abs (errnum), &numbuf[20], 10, 0);

		/* Now construct the result while taking care for the destination
		buffer size.  */
		q = __mempcpy (buf, unk, MIN (unklen, buflen));
		if (negative && unklen < buflen)
		{
			*q++ = '-';
			++unklen;
		}
		
		if (unklen < buflen)
			memcpy (q, p, MIN ((size_t) (&numbuf[21] - p), buflen - unklen));

		/* Terminate the string in any case.  */
		if (buflen > 0)
		buf[buflen - 1] = '\0';

		return buf;
	}

	return (char *) _(_sys_errlist_internal[errnum]);
}

weak_alias (__strerror_r, strerror_r)
libc_hidden_def (__strerror_r)
{% endhighlight %}


## 2. os/unix/ngx_errno.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * The strerror() messages are copied because:
 *
 * 1) strerror() and strerror_r() functions are not Async-Signal-Safe,
 *    therefore, they cannot be used in signal handlers;
 *
 * 2) a direct sys_errlist[] array may be used instead of these functions,
 *    but Linux linker warns about its usage:
 *
 * warning: `sys_errlist' is deprecated; use `strerror' or `strerror_r' instead
 * warning: `sys_nerr' is deprecated; use `strerror' or `strerror_r' instead
 *
 *    causing false bug reports.
 */


static ngx_str_t  *ngx_sys_errlist;
static ngx_str_t   ngx_unknown_error = ngx_string("Unknown error");


u_char *
ngx_strerror(ngx_err_t err, u_char *errstr, size_t size)
{
    ngx_str_t  *msg;

    msg = ((ngx_uint_t) err < NGX_SYS_NERR) ? &ngx_sys_errlist[err]:
                                              &ngx_unknown_error;
    size = ngx_min(size, msg->len);

    return ngx_cpymem(errstr, msg->data, size);
}


ngx_int_t
ngx_strerror_init(void)
{
    char       *msg;
    u_char     *p;
    size_t      len;
    ngx_err_t   err;

    /*
     * ngx_strerror() is not ready to work at this stage, therefore,
     * malloc() is used and possible errors are logged using strerror().
     */

    len = NGX_SYS_NERR * sizeof(ngx_str_t);

    ngx_sys_errlist = malloc(len);
    if (ngx_sys_errlist == NULL) {
        goto failed;
    }

    for (err = 0; err < NGX_SYS_NERR; err++) {
        msg = strerror(err);
        len = ngx_strlen(msg);

        p = malloc(len);
        if (p == NULL) {
            goto failed;
        }

        ngx_memcpy(p, msg, len);
        ngx_sys_errlist[err].len = len;
        ngx_sys_errlist[err].data = p;
    }

    return NGX_OK;

failed:

    err = errno;
    ngx_log_stderr(0, "malloc(%uz) failed (%d: %s)", len, err, strerror(err));

    return NGX_ERROR;
}

{% endhighlight %}


<br />
<br />

**[参考]：**

1. [Nginx源码完全注释（8）ngx_errno.c](http://blog.csdn.net/Poechant/article/details/8032389)

2. [可重入性 线程安全 Async-Signal-Safe](http://blog.csdn.net/ldong2007/article/details/4271685)

3. [strerror线程安全分析](http://blog.csdn.net/aquester/article/details/23839619)

4. [gcc的__builtin_函数介绍](http://blog.csdn.net/jasonchen_gbd/article/details/44948523)

<br />
<br />
<br />

