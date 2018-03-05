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

<br />
<br />

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

这里可以看到，对于strerror(int errnum)函数，如果参数errnum是一个已知的errno，其直接从一个预先定义好的数组中返回相应的字符串，因此是绝对安全的。但是假如errnum参数所表示的错误码并未在系统内部定义的话，则会执行malloc()开辟一段内存空间。malloc()函数本身是线程安全的，但是却不是```异步信号安全```的。这里strerror()函数之所以线程不安全，是因为两次调用strerror()函数，后一次调用会将前一次调用所分配的空间释放掉。
<pre>
异步信号安全，它其实也很简单，就是一个函数可以在信号处理函数中被安全地调用。看起来它似乎和线程安全类似，其 实不然，我们知道信号是异
步产生的，但是，信号处理函数是打断主函数（相对于信号处理函数）后执行，执行完后又返回主函数中去的。也就是说，它不是 并发的！
</pre>
<br />

这里在对上面函数中```__builtin_expect()```做一个简单的介绍：

__builtin_expect(long exp, long c):  用来引导gcc进行条件分支预测。在一条指令执行时，由于流水线的作用，CPU可以完成下一条指令的取指，这样可以提高CPU的利用率。在执行一条条件分支指令时，CPU也会预取下一条执行，但是如果条件分支跳转到了其他指令，那CPU预取的下一条指令就没用了，这样就降低了流水线的效率。内核中likely()和unlikely()就是通过```__builtin_expect```来实现的。

__builtin_expect(long exp, long c)函数可以优化程序编译后的指令序列，使指令尽可能的顺序执行，从而提高CPU的预取指令的正确率。该函数的第二个参数可以取0和1。 例如：
<pre>
if (__builtin_expect(x,0))
	foo();
</pre>
表示x的值大部分情况下可能为0，因此foo()函数得到执行的机会较少。gcc就不必将foo()函数的汇编指令紧挨着if条件跳转指令。

由于第二个参数只能取整数，所以如果要判断指针或字符串，可以像下面这样写：
<pre>
if(__builtin_expect(ptr != NULL, 1))
	foo(*ptr);
</pre>
表示ptr一般不会为NULL，所以foo函数得到执行的概率较大，gcc会将foo函数的汇编指令放在紧挨着if跳转执行的位置。


<br />



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


这里之所以用ngx_strerror()，有两点原因。上面代码开头部分已经解释很清楚，这里不再赘述。

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

