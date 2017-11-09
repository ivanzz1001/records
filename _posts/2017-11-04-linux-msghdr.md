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

这里```msg_control```指针: points to a buffer for other protocol control-related messages or miscellaneous ancillary data（指向与协议控制相关的消息或者辅助数据）. 而```msg_controllen```为msg_control所指向的这块缓冲的长度。

```msg_control```是一个struct cmsghdr结构，下面我们会介绍。


## 2. cmsghdr结构

cmsghdr结构如下：
<pre>
struct cmsghdr {
   size_t cmsg_len;    /* Data byte count, including header
                          (type is socklen_t in POSIX) */
   int    cmsg_level;  /* Originating protocol */
   int    cmsg_type;   /* Protocol-specific type */
/* followed by
   unsigned char cmsg_data[]; */
};
</pre>
```cmsg_level```一般为原始的协议级别，```cmsg_type```为前面原始协议下的某一个子类型。要访问此辅助数据结构，一般会用到如下几个函数：
{% highlight string %}
#include <sys/socket.h>

struct cmsghdr *CMSG_FIRSTHDR(struct msghdr *msgh);
struct cmsghdr *CMSG_NXTHDR(struct msghdr *msgh, struct cmsghdr
*cmsg);
size_t CMSG_ALIGN(size_t length);
size_t CMSG_SPACE(size_t length);
size_t CMSG_LEN(size_t length);
unsigned char *CMSG_DATA(struct cmsghdr *cmsg);
{% endhighlight %}


这些宏用于创建和访问控制消息(msg_control)，也称为辅助数据，其并不作为socket净荷数据的一部分，净荷数据保存在msg_iov中（参见上述struct msghdr)。 这些辅助数据可能包括：

* 所收到的packet的网卡接口

* 一些不太常用的头部字段

* 一个扩展的错误描述

* 一个文件描述符集合

* UNIX credentials

例如用辅助数据可以发送一些额外的头部字段(eg. IP options)。

<br />

要访问一系列的cmsghdr结构，我们必须使用如下这些宏，而不要直接访问：

* **CMSG_FIRSTHDR()**: 返回msghdr辅助数据部分指向第一个cmsghdr的指针

* **CMSG_NXTHDR()**: 返回参数中cmsghdr的下一个有效cmsghdr。当msg_control buffer中没有足够剩余的空间的时候，返回NULL

* **CMSG_ALIGN()**:  给定一个长度，其会返回对齐后相应的长度。它是一个常量表达式，其一般实现如下：
<pre>
#define CMSG_ALIGN(len)  ( ((len)+sizeof(long)-1) & ~(sizeof(long)-1) ) 
</pre>

* **CMSG_SPACE()**: 返回辅助数据及其所传递的净荷数据的总长度。即sizeof(cmsg_len) + sizeof(cmsg_level) + sizeof(cmsg_type) + len(cmsg_data)长度进行CMSG_ALIGN后的值.

* **CMSG_DATA()**: 返回cmsghdr的净荷数据部分 

* **CMSG_LEN()**: 返回净荷数据长度进行CMSG_ALIGN后的值，一般赋值给cmsghdr.cmsg_len。


为了创建辅助数据，首先初始化msghdr.msg_controllen字段。 在msghdr上使用```CMSG_FIRSTHDR()```来获取第一个控制消息，然后使用```CMSG_NXTHDR()```来获取后续的控制消息。在每一个控制消息中，使用```CMSG_LEN()```来初始化cmsghdr.cmsg_len，使用```CMSG_DATA()```来初始化cmsghdr.cmsg_data部分。



## 3. 例子
如下代码片段用于寻找辅助数据中的IP_TTL选项值：
{% highlight string %}
struct msghdr msgh;
struct cmsghdr *cmsg;
int *ttlptr;
int received_ttl;

/* Receive auxiliary data in msgh */

for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
       cmsg = CMSG_NXTHDR(&msgh, cmsg)) {
   if (cmsg->cmsg_level == IPPROTO_IP
           && cmsg->cmsg_type == IP_TTL) {
       ttlptr = (int *) CMSG_DATA(cmsg);
       received_ttl = *ttlptr;
       break;
   }
}

if (cmsg == NULL) {
   /* Error: IP_TTL not enabled or small buffer or I/O error */
}
{% endhighlight %}


如下代码片段在unix域socket上使用```SCM_RIGHTS```传递文件句柄数组：
{% highlight string %}
struct msghdr msg = { 0 };
struct cmsghdr *cmsg;
int myfds[NUM_FD];  /* Contains the file descriptors to pass */
int *fdptr;
char iobuf[1];
struct iovec io = {
   .iov_base = iobuf,
   .iov_len = sizeof(iobuf)
};
union {         /* Ancillary data buffer, wrapped in a union
				  in order to ensure it is suitably aligned */
   char buf[CMSG_SPACE(sizeof(myfds))];
   struct cmsghdr align;
} u;

msg.msg_iov = &io;
msg.msg_iovlen = 1;
msg.msg_control = u.buf;
msg.msg_controllen = sizeof(u.buf);
cmsg = CMSG_FIRSTHDR(&msg);
cmsg->cmsg_level = SOL_SOCKET;
cmsg->cmsg_type = SCM_RIGHTS;
cmsg->cmsg_len = CMSG_LEN(sizeof(int) * NUM_FD);
fdptr = (int *) CMSG_DATA(cmsg);    /* Initialize the payload */
memcpy(fdptr, myfds, NUM_FD * sizeof(int));
{% endhighlight %}



<br />
<br />

**[参看]:**

1. [Linux Programmer's Manual CMSG(3)](http://www.man7.org/linux/man-pages/man3/cmsg.3.html)


<br />
<br />
<br />


