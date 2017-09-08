---
layout: post
title: Linux中pkg-config的使用
tags:
- LinuxOps
categories: linux
description: Linux中pkg-config的使用
---

在通过源代码安装tesseract，在执行configure命令时一直提示：
<pre>
Leptonica 1.74 or higher is required. Try to install libleptonica-dev package.
</pre>
<!-- more -->
而实际上，我们已经通过源代码的方式成功的安装了Leptonica 1.74.4。最后没办法只能查看configure源代码，发现原来是pkg-config找不到我们安装的leptonica。因此，学习了一下pkg-config及其使用方法，在此做一个记录。


## 1. pkg-config简单介绍

```pkg-config```在编译应用程序和库的时候作为一个工具来使用。例如你在命令行通过如下命令编译程序时：
{% highlight string %}
gcc -o test test.c `pkg-config --libs --cflags glib-2.0`
{% endhighlight %}
pkg-config可以帮助你插入正确的编译选项，而不需要你通过硬编码的方式来找到glib(或其他库）。

```--cflags```一般用于指定头文件，```--libs```一般用于指定库文件。

大家应该都知道一般用第三方库的时候，就少不了要使用到第三方的头文件和库文件。我们在编译、链接的时候，必须要指定这些头文件和库文件的位置。对于一个比较大的第三方库，其头文件和库文件的数量是比较多的，如果我们一个个手动地写，那将是相当的麻烦的。因此，pkg-config就应运而生了。pkg-config能够把这些头文件和库文件的位置指出来，给编译器使用。pkg-config主要提供了下面几个功能：

* 检查库的版本号。 如果所需要的库的版本不满足要求，它会打印出错误信息，避免链接错误版本的库文件
* 获得编译预处理参数，如宏定义、头文件的位置
* 获得链接参数，如库及依赖的其他库的位置，文件名及其他一些链接参数
* 自动加入所依赖的其他库的设置 


pkg-config命令的基本用法如下：
{% highlight string %}
# pkg-config <options> <library-name>
{% endhighlight %}
例如，我们可以通过如下命令来查看当前安装了哪些库：
<pre>

[root@localhost pkgconfig]# pkg-config --list-all
zlib                      zlib - zlib compression library
gio-unix-2.0              GIO unix specific APIs - unix specific headers for glib I/O library
inputproto                InputProto - Input extension headers
cairo-xcb                 cairo-xcb - XCB surface backend for cairo graphics library
gio-2.0                   GIO - glib I/O library
//后续省略
</pre>


## 2. 配置环境变量

事实上，pkg-config只是一个工具，所以不是你安装了一个第三方库，pkg-config就能知道第三方库的头文件和库文件的位置的。为了让pkg-config可以得到一个库的信息，就要求库的提供者提供一个.pc文件。默认情况下，比如执行如下命令：
<pre>
# pkg-config --libs --cflags glib-2.0
</pre>
pkg-config会到```/usr/lib/pkconfig/```目录下去寻找glib-2.0.pc文件。也就是说在此目录下的.pc文件，pkg-config是可以自动找到的。然而假如我们安装了一个库，其生成的.pc文件并不在这个默认目录中的话，pkg-config就找不到了。此时我们需要通过```PKG_CONFIG_PATH```环境变量来指定pkg-config还应该在哪些地方去寻找.pc文件。

我们可以通过如下命令来设置```PKG_CONFIG_PATH```环境变量：
{% highlight string %}
# export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig/
{% endhighlight %}
这样pkg-config就会在/usr/local/lib/pkgconfig/目录下寻找.pc文件了。我们在文章开头提到的找不到Leptonica 1.74.4的原因也正是因为其找不到lept.pc，因此我们只需要将对应的目录设置到PKG_CONFIG_PATH环境变量中即可。

另外还需要注意的是,上述环境变量的设置只对当前的终端窗口有效。为了让其永久生效，我们可以将上述命令写入到/etc/bash.bashrc等文件中，以方便后续使用。

## 3. pkg-config与LD_LIBRARY_PATH

pkg-config与LD_LIBRARY_PATH在使用时有些类似，都可以帮助找到对应的库（静态库和共享库）。这里我们重点介绍一下它们两者的区别。我们知道一个程序从源代码，然后编译连接，最后再执行这一基本过程。这里我们列出pkg-config与LD_LIBRARY_PATH的主要工作阶段：

* pkg-config: 编译时、 链接时
* LD_LIBRARY_PATH: 链接时、 运行时

pkg-config主要是在编译时会用到其来查找对应的头文件、链接库等；而LD_LIBRARY_PATH环境变量则在 链接时 和 运行时 会用到。程序编译出来之后，在程序加载执行时也会通过LD_LIBRARY_PATH环境变量来查询所需要的库文件。

下面我们来讲述一下LD_LIBRARY_PATH及ldconfig命令：

库文件在链接（静态库和共享库）和运行（仅限于使用共享库的程序）时被使用，其搜索路径是在系统中进行设置的。一般Linux系统把/lib和/usr/lib这两个目录作为默认的库搜索路径，所以使用这两个目录中的库时不需要进行设置搜索路径即可直接使用。对于处于默认库搜索路径之外的库，需要将库的位置添加到库的搜索路径之中。设置库文件的搜索路径有下列两种方式，可任选其中一种使用：

* 在环境变量LD_LIBRARY_PATH中指明库的搜索路径
* 在/etc/ld.so.conf文件中添加库的搜索路径

将自己可能存放库文件的路径都加入到/etc/ld.so.conf中是明智的选择。添加方法也及其简单，将库文件的绝对路径直接写进去就OK了，一行一个。比如：
{% highlight string %}
/usr/X11R6/lib
/usr/local/lib
/opt/lib
{% endhighlight %}

需要注意的是：第二种搜索路径的设置方式对于程序链接时的库（包括共享库和静态库）的定位已经足够了。但是对于使用了共享库的程序的执行还是不够的，这是因为为了加快程序执行时对共享库的定位速度，避免使用搜索路径查找共享库的低效率，所以是直接读取库列表文件/etc/ld.so.cache的方式从中进行搜索。/etc/ld.so.cache是一个非文本的数据文件，不能直接编辑，它是根据/etc/ld.so.conf中设置的搜索路径由/sbin/ldconfig命令将这些搜索路径下的共享库文件集中在一起而生成的（ldconfig命令要以root权限执行）。因此为了保证程序执行时对库的定位，在/etc/ld.so.conf中进行了库搜索路径的设置之后，还必须要运行/sbin/ldconfig命令更新/etc/ld.so.cache文件之后才可以。

ldconfig，简单的说，它的作用就是将/etc/ld.so.conf列出的路径下的库文件缓存到/etc/ld.so.cache以供使用。因此当安装完一些库文件（例如刚安装好glib)，或者修改ld.so.conf增加新的库路径之后，需要运行一下/sbin/ldconfig使所有的库文件都被缓存到ld.so.cache中。如果没有这样做，即使库文件明明就在/usr/lib下的，也是不会被使用的，结果在编译过程中报错。

在程序链接时，对于库文件（静态库和共享库）的搜索路径，除了上面的设置方式之外，还可以通过-L参数显示指定。因为用-L设置的路径将被优先搜索，所以在链接的时候通常都会以这种方式直接指定要链接的库的路径。


前面已经说明过了，库搜索路径的设置有两种方式：在环境变量 LD_LIBRARY_PATH 中设置以及在 /etc/ld.so.conf 文件中设置。其中，第二种设置方式需要 root 权限，以改变 /etc/ld.so.conf 文件并执行 /sbin/ldconfig 命令。而且，当系统重新启动后，所有的基于 GTK2 的程序在运行时都将使用新安装的 GTK+ 库。不幸的是，由于 GTK+ 版本的改变，这有时会给应用程序带来兼容性的问题，造成某些程序运行不正常。为了避免出现上面的这些情况，在 GTK+ 及其依赖库的安装过程中对于库的搜索路径的设置将采用第一种方式进行。这种设置方式不需要 root 权限，设置也简单：
{% highlight string %}
# export LD_LIBRARY_PATH=/opt/gtk/lib:$LD_LIBRARY_PATH

# echo $LD_LIBRARY_PATH
{% endhighlight %}


## 4. pc文件书写规范
这里我们首先来看一个例子：
{% highlight string %}
[root@localhost pkgconfig]# cat libevent.pc 
#libevent pkg-config source file

prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: libevent
Description: libevent is an asynchronous notification event loop library
Version: 2.0.22-stable
Requires:
Conflicts:
Libs: -L${libdir} -levent
Libs.private: 
Cflags: -I${includedir}
{% endhighlight %}

这是libevent库的一个真实的例子。下面我们简单描述一下pc文件中的用到的一些关键词：

* Name: 一个针对library或package的便于人阅读的名称。这个名称可以是任意的，它并不会影响到pkg-config的使用，pkg-config是采用pc文件名的方式来工作的。
* Description: 对package的简短描述
* URL: 人们可以通过该URL地址来获取package的更多信息或者package的下载地址
* Version: 指定package版本号的字符串
* Requires: 本库所依赖的其他库文件。所依赖的库文件的版本号可以通过使用如下比较操作符指定：=,<,>,<=,>=
* Requires.private: 本库所依赖的一些私有库文件，但是这些私有库文件并不需要暴露给应用程序。这些私有库文件的版本指定方式与Requires中描述的类似。
* Conflicts: 是一个可选字段，其主要用于描述与本package所冲突的其他package。版本号的描述也与Requires中的描述类似。本字段也可以取值为同一个package的多个不同版本实例。例如: Conflicts: bar < 1.2.3, bar >= 1.3.0
* Cflags: 编译器编译本package时所指定的编译选项，和其他并不支持pkg-config的library的一些编译选项值。假如所需要的library支持pkg-config,则它们应该被添加到Requires或者Requires.private中
* Libs: 链接本库时所需要的一些链接选项，和其他一些并不支持pkg-config的library的链接选项值。与Cflags类似
* Libs.private: 本库所需要的一些私有库的链接选项。



## 5. 示例

如下我们给出一个使用pkg-config的程序例子(test_event.cpp)：
{% highlight string %}
#include <iostream>  
#include <event.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <netinet/in.h>  
#include <string.h>  
#include <fcntl.h>  
  
using namespace std;  
  
struct event_base* main_base;  
  
static const char MESSAGE[] ="Hello, World!\n";  
  
void accept_handle(const int sfd, const short event, void *arg)  
{  
    cout<<"accept handle"<<endl;  
  
    struct sockaddr_in addr;  
  
    socklen_t addrlen = sizeof(addr);  
  
    int fd = accept(sfd, (struct sockaddr *) &addr, &addrlen); //处理连接  
  
    struct bufferevent* buf_ev;  
    buf_ev = bufferevent_new(fd, NULL, NULL, NULL, NULL);  
  
    buf_ev->wm_read.high = 4096;  
  
    cout<<"event write"<<endl;  
    bufferevent_write(buf_ev, MESSAGE, strlen(MESSAGE));  
}  
  
int main()  
{  
    cout<<"hello man!"<<endl;  
  
    // 1. 初始化EVENT  
    main_base = event_init();  
    if(main_base)  
        cout<<"init event ok!"<<endl;  
  
    // 2. 初始化SOCKET  
    int sListen;  
  
    // Create listening socket  
    sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
  
    // Bind  
    struct sockaddr_in server_addr;  
    bzero(&server_addr,sizeof(struct sockaddr_in));  
    server_addr.sin_family=AF_INET;  
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);  
    int portnumber = 8080;  
    server_addr.sin_port = htons(portnumber);  
  
    /* 捆绑sockfd描述符  */  
    if(bind(sListen,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)  
    {  
        cout<<"error!"<<endl;  
        return -1;  
    }  
  
    // Listen  
    ::listen(sListen, 3);  
    cout<<"Server is listening!\n"<<endl;  
  
    /*将描述符设置为非阻塞*/  
    int flags = ::fcntl(sListen, F_GETFL);  
  
    flags |= O_NONBLOCK;  
  
    fcntl(sListen, F_SETFL, flags);  
  
    // 3. 创建EVENT 事件  
    struct event ev;  
    event_set(&ev, sListen, EV_READ | EV_PERSIST, accept_handle, (void *)&ev);  
  
    // 4. 事件添加与删除  
    event_add(&ev, NULL);  
  
    // 5. 进入事件循环  
    event_base_loop(main_base, 0);  
  
    cout<<"over!"<<endl;  
}
{% endhighlight %}

执行如下命令编译程序：
<pre>
# gcc -o test_event test_event.cpp -lstdc++ `pkg-config --cflags --libs libevent`
</pre>

运行程序：
<pre>
[root@localhost test-src]# ./test_event 
hello man!
init event ok!
Server is listening!
</pre>

开启另外一个终端，采用nc命令连接test_event服务端程序：
<pre>
[root@localhost ~]# nc 127.0.0.1 8080
Hello, World!
</pre>
可以看到运行成功。



**[参看]:**

1. [pkg-config官网](https://www.freedesktop.org/wiki/Software/pkg-config/)

2. [ldconfig命令](http://man.linuxde.net/ldconfig)

3. [PKG_CONFIG_PATH变量 与 ld.so.conf 文件](http://www.cnblogs.com/s_agapo/archive/2012/04/24/2468925.html)


<br />
<br />
<br />





