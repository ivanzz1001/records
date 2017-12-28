---
layout: post
title: os/unix/ngx_process_cycle.c源代码分析(附录)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本文主要包含ngx_process_cycle.c中涉及到的一些知识点： 信号屏蔽函数sigprocmask()、nginx平滑升级时的reconfigure操作等。


<!-- more -->

## 1. 进程信号处理

在程序的编写过程中往往要对关心的信号进行处理。这里我们介绍几个函数：

### 1.1 函数sigprocmask()
{% highlight string %}
#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
{% endhighlight %}
```sigprocmask()```函数被用于获取和更改调用线程的信号掩码。信号掩码是指当前信号投递会被阻塞的一组信号集合。本函数的实际效果主要是受```how```参数所影响：

* **SIG_BLOCK:** 最后会被阻塞的信号集为当前被阻塞的信号集与set参数指定的信号集的并集。

* **SIG_UNBLOCK:** 最后会被阻塞的信号集为当前被阻塞的信号集与set参数指定的信号集的差集。移除一个当前并不在阻塞集合中的信号也是被允许的。

* **SIG_SETMASK:** 最后会被阻塞的信号集为set参数指定的信号集。

对于参数```oldset```，如果不为NULL，则会用此参数保留调用sigprocmask()函数之前的被阻塞的信号集。假如```set```参数为NULL，则最后被阻塞的信号集保持不变，而此时仍可以通过```oldset```参数返回此种情况下被阻塞的信号集。

<pre>
对于在拥有多个线程的进程中使用sigprocmask()函数，其效果是未定的。具有多线程的进程的信号处理，请参看pthread_sigmask()
</pre>

sigprocmask()函数在成功时返回0，失败时返回-1。

<br />

下面我们给出一个sigprocmask()的使用例子：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static void sighandler(int sig)
{
   char buf[] = "sighandler\n";
   write(1,buf,sizeof(buf)-1);
}

int main(int argc,char *argv[])
{
    struct sigaction   sa;
    sigset_t           set;
    int count = 10;
 
    memset(&sa,0x0,sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);

    if(sigaction(SIGINT,&sa,NULL) == -1)
       exit(-1);
   
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    if(sigprocmask(SIG_BLOCK,&set,NULL) == -1)
        exit(-2);

    if(sigismember(&set,SIGINT) == 1)
    {
        printf("SIGINT is a member of set\n");
    }
    else{
        printf("SIGINT is not a member of set\n");
    }
    
    printf("in 10s, the SIGINT keeped blocked\n");
    while(count)
    {
        printf("%d\n",count--);
        sleep(1);
    }

    sigemptyset(&set);
    sigsuspend(&set);

    printf("10s later, we will unblock SIGINT\n");
    while(count < 10)
    { 
       printf("%d\n",count++);
       sleep(1);
    }
   
    sigemptyset(&set);
    sigaddset(&set,SIGINT);
    if(sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
        exit(-3);

    printf("unblock SIGINT successful\n");
    while(1)
    {
       sleep(1);
    }
    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
SIGINT is a member of set
in 10s, the SIGINT keeped blocked
10
9
8
7
^C6
5
^C4
3
2
1
sighandler               //此处调用了sigsuspend()
10s later, we will unblock SIGINT
0
1
2
3
^C4
5
^C6
7
8
9
sighandler             //此处调用了sigprocmask(SIG_UNBLOCK,&set,NULL)
unblock SIGINT successful
^Csighandler
^Csighandler
Killed               //此处我们在另一个终端向本进程发送了kill -9信号
</pre>

### 1.2 函数sigsuspend()

{% highlight string %}
#include <signal.h>

int sigsuspend(const sigset_t *mask);
{% endhighlight %}
sigsuspend()函数临时用```mask```信号集替换当前调用进程的信号阻塞掩码，然后挂起进程的执行，直到收到一个需要调用处理函数的信号(处理函数不能为SIG_IGN)或者进程终止。

假如该信号终止了进程，则sigsuspend()函数并不会返回；假如信号被捕捉到，则在信号处理函数返回后sigsuspend()函数才会返回，并且当前被阻塞的信号掩码会被重置为调用sigsuspend()函数之前的状态。

<pre>
注意：并不能阻塞SIGKILL和SIGSTOP信号。即使在信号掩码中指定这两个信号，都不会实质影响到进程的信号掩码。
</pre>

通常情况下，sigsuspend()函数搭配sigprocmask()函数一起使用，以此来保护关键代码段在执行期间并不会被投递我们屏蔽的信号。调用者一般是先调用sigprocmask()函数屏蔽一些信号，然后执行关键代码段，再接着调用sigsuspend()函数等待相应需要处理的信号的到来。如下：
<pre>
sigprocmask()

critical_code

sigsuspend()
</pre>



<br />
<br />

## 2. nginx平滑升级时reconfigure
{% highlight string %}
root@ubuntu:/usr/local/nginx# ./nginx
root@ubuntu:/usr/local/nginx# 
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      6960     1  0 07:48 ?        00:00:00 nginx: master process ./nginx
nobody    6961  6960  0 07:48 ?        00:00:00 nginx: worker process
root      6963  6929  0 07:48 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# kill -s SIGUSR2 6960
root@ubuntu:/usr/local/nginx# ls 
client_body_temp        koi-utf             nginx.conf.default   uwsgi_params
fastcgi.conf            koi-win             nginx.pid            uwsgi_params.default
fastcgi.conf.default    logs                nginx.pid.oldbin     uwsgi_temp
fastcgi_params          mime.types          proxy_temp           win-utf
fastcgi_params.default  mime.types.default  scgi_params
fastcgi_temp            nginx               scgi_params.default
html                    nginx.conf          scgi_temp
root@ubuntu:/usr/local/nginx# ./nginx
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] still could not bind()
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      6960     1  0 07:48 ?        00:00:00 nginx: master process ./nginx
nobody    6961  6960  0 07:48 ?        00:00:00 nginx: worker process
root      6964  6960  0 07:49 ?        00:00:00 nginx: master process ./nginx
nobody    6965  6964  0 07:49 ?        00:00:00 nginx: worker process
root      6982  6929  0 07:50 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# ls
client_body_temp        koi-utf             nginx.conf.default   uwsgi_params
fastcgi.conf            koi-win             nginx.pid            uwsgi_params.default
fastcgi.conf.default    logs                nginx.pid.oldbin     uwsgi_temp
fastcgi_params          mime.types          proxy_temp           win-utf
fastcgi_params.default  mime.types.default  scgi_params
fastcgi_temp            nginx               scgi_params.default
html                    nginx.conf          scgi_temp
root@ubuntu:/usr/local/nginx# cat nginx.pid
6964
root@ubuntu:/usr/local/nginx# kill -s SIGHUP 6960
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      6960     1  0 07:48 ?        00:00:00 nginx: master process ./nginx
nobody    6961  6960  0 07:48 ?        00:00:00 nginx: worker process
root      6964  6960  0 07:49 ?        00:00:00 nginx: master process ./nginx
nobody    6965  6964  0 07:49 ?        00:00:00 nginx: worker process
nobody    6985  6960  0 07:53 ?        00:00:00 nginx: worker process
root      6987  6929  0 07:53 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# ./nginx -s reload
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      6960     1  0 07:48 ?        00:00:00 nginx: master process ./nginx
nobody    6961  6960  0 07:48 ?        00:00:00 nginx: worker process
root      6964  6960  0 07:49 ?        00:00:00 nginx: master process ./nginx
nobody    6985  6960  0 07:53 ?        00:00:00 nginx: worker process
nobody    6990  6964  0 07:54 ?        00:00:00 nginx: worker process
root      6992  6929  0 07:54 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# ls
client_body_temp        koi-utf             nginx.conf.default   uwsgi_params
fastcgi.conf            koi-win             nginx.pid            uwsgi_params.default
fastcgi.conf.default    logs                nginx.pid.oldbin     uwsgi_temp
fastcgi_params          mime.types          proxy_temp           win-utf
fastcgi_params.default  mime.types.default  scgi_params
fastcgi_temp            nginx               scgi_params.default
html                    nginx.conf          scgi_temp
root@ubuntu:/usr/local/nginx# kill -s SIGHUP 6960
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      6960     1  0 07:48 ?        00:00:00 nginx: master process ./nginx
nobody    6961  6960  0 07:48 ?        00:00:00 nginx: worker process
root      6964  6960  0 07:49 ?        00:00:00 nginx: master process ./nginx
nobody    6985  6960  0 07:53 ?        00:00:00 nginx: worker process
nobody    6990  6964  0 07:54 ?        00:00:00 nginx: worker process
nobody    6994  6960  0 07:57 ?        00:00:00 nginx: worker process
root      6996  6929  0 07:57 pts/8    00:00:00 grep --color=auto nginx
{% endhighlight %}

<br />
<br />
<br />

