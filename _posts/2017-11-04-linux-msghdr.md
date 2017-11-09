---
layout: post
title: Linux msghdr结构讲解
tags:
- LinuxOps
categories: linux
description: Linux msghdr结构讲解
---


本文我们主要Linux msghdr这一重要的数据结构，其广泛应用于如文件描述符传递，数字证书传递等方面。



<!-- more -->


## 1. msghdr结构

msghdr结构一般会用于如下两个函数中：
{% highlight string %}
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
{% endhighlight %}
它主要用于向一个socket发送消息，或从一个socket中接收消息。此处很重要的一个作用就是用在unix域中传递一个文件描述符。struct msghdr结构如下：
<pre>
#include <sys/socket.h>

struct iovec {                    /* Scatter/gather array items */
   void  *iov_base;              /* Starting address */
   size_t iov_len;               /* Number of bytes to transfer */
};

struct msghdr {
   void         *msg_name;       /* optional address */
   socklen_t     msg_namelen;    /* size of address */
   struct iovec *msg_iov;        /* scatter/gather array */
   size_t        msg_iovlen;     /* # elements in msg_iov */
   void         *msg_control;    /* ancillary data, see below */
   size_t        msg_controllen; /* ancillary data buffer len */
   int           msg_flags;      /* flags on received message */
};
</pre>




<br />
<br />

**[参看]:**

1. [Linux Programmer's Manual CMSG(3)](http://www.man7.org/linux/man-pages/man3/cmsg.3.html)


<br />
<br />
<br />


