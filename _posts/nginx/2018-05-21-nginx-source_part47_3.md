---
layout: post
title: core/ngx_open_file_cache.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---





<!-- more -->






## 1. direct io方式读写文件（附录）
所谓direct io， 即不通过操作系统缓冲， 使用磁盘IO(或者DMA)直接将硬盘上的数据读入用户空间buffer， 或者将用户空间buffer中的数据通过磁盘IO(或者DMA)直接写到硬盘上。这样避免内核缓冲的消耗与CPU拷贝（数据在内核空间和用户空间之间的拷贝）的消耗。

### 1.1 direct io使用场景

direct io一般是通过DMA的方式来读取文件的。 通过direct io读取文件之前，一般需要初始化DMA, 因此一般使用direct io来读取大文件； 如果是读取小文件，初始化DMA的时间比系统读小文件的时间还长， 所以小文件使用direct io没有优势。对于大文件也只是在只读一次，并且后续没有其他应用再次读取此文件的时候，才有优势， 如果后续还有其他应用需要使用， 这个时候DirectIO也没有优势。


### 1.2 direct io使用示例
direct io方式读写文件， 只需在打开文件时选上```O_DIRECT```选项就行， 但必须在所有```include```前加上:
<pre>
#define _GNU_SOURCE
</pre> 
另外，以direct io方式读写时，开辟的buffer必须是系统页大小的整数倍而且必须以页大小为标准对齐， 例如linux2.6下每页大小是4096字节（函数```getpagesize()```)，申请的buffer大小只能是4096的整数倍才能获得最大的性能。 参看如下例子test.c:
{% highlight string %}

#define _GNU_SOURCE

#define BUFFER_SIZE 8192

int fd = open("testfile", O_CREAT|O_RDWR|O_DIRECT);
int pagesize = getpagesize();

char *readbuf = (char *)malloc(BUFFER_SIZE + pagesize);

char *alignedReadBuf = (char *) ((((uintptr)readbuf + pagesize -1)/pagesize) * pagesize);

read(fd, alignedReadBuf, BUFFER_SIZE);

free(readbuf);

{% endhighlight %}


### 1.3 nginx aio与direct io
关于nginx aio， 有如下一段说明：
<pre>
On Linux, AIO can be used starting from kernel version 2.6.22. Also, it is necessary to enable directio, or otherwise reading will be blocking:

location /video/ {
    aio            on;
    directio       512;
    output_buffers 1 128k;
}
</pre>


而当AIO与sendfile一起使用时， 有如下一段说明：
<pre>
When both AIO and sendfile are enabled on Linux, AIO is used for files that are larger than or equal to the size specified in the directio directive, while sendfile is used for files of smaller sizes or when directio is disabled.

location /video/ {
    sendfile       on;
    aio            on;
    directio       8m;
}
</pre>

当aio为```threads```时，有如下说明：
<pre>
Finally, files can be read and sent using multi-threading (1.7.11), without blocking a worker process:

location /video/ {
    sendfile       on;
    aio            threads;
}
</pre>



<br />
<br />

**[参看]**

1. [DirectIO方式读写文件](https://blog.csdn.net/zhangxinrun/article/details/6874143)

2. [nginx aio](http://nginx.org/en/docs/http/ngx_http_core_module.html#aio)



<br />
<br />
<br />

