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


## 1.3 函数pthread_sigmask()
前面我们说道，sigprocmask()函数是针对进程中只有一个线程的情况。针对一个进程中有多个线程这一情景，我们用pthread_sigmask()函数：
{% highlight string %}
#include <signal.h>

int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
{% endhighlight %}
pthread_sigmask()函数用法与sigprocmask()相似，但是主要是用在多线程这一情形中。pthread_sigmask()是线程安全的。

本函数成功时返回0，失败时返回对应的错误号。

<pre>
说明：一个新创建的线程会继承其创建者的信号掩码
</pre>

如下给出一个例子。该例子会在主线程中阻塞一些信号，然后在创建一个线程通过sigwait()函数来获取这些信号：
{% highlight string %}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* Simple error handling functions */

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static void *
sig_thread(void *arg)
{
   sigset_t *set = arg;
   int s, sig;

   for (;;) {
       s = sigwait(set, &sig);
       if (s != 0)
           handle_error_en(s, "sigwait");
       printf("Signal handling thread got signal %d\n", sig);
   }
}

int
main(int argc, char *argv[])
{
   pthread_t thread;
   sigset_t set;
   int s;

   /* Block SIGQUIT and SIGUSR1; other threads created by main()
      will inherit a copy of the signal mask. */

   sigemptyset(&set);
   sigaddset(&set, SIGQUIT);
   sigaddset(&set, SIGUSR1);
   s = pthread_sigmask(SIG_BLOCK, &set, NULL);
   if (s != 0)
       handle_error_en(s, "pthread_sigmask");

   s = pthread_create(&thread, NULL, &sig_thread, (void *) &set);
   if (s != 0)
       handle_error_en(s, "pthread_create");

   /* Main thread carries on to create other threads and/or do
      other work */

   pause();            /* Dummy pause so we can test program */
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test3 test3.c -lpthread
[root@localhost test-src]# ./test3 &
[1] 4985
[root@localhost test-src]# kill -QUIT %1
Signal handling thread got signal 3
[root@localhost test-src]# kill -USR1 %1
Signal handling thread got signal 10
[root@localhost test-src]# kill -TERM %1
[root@localhost test-src]# ps -ef | grep test3
[1]+  Terminated              ./test3
</pre>

### 1.4 多线程程序信号投递
在一个运行有多个线程的进程中，信号具体会投递到哪一个线程是未知的，请参看如下程序：
{% highlight string %}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* Simple error handling functions */

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static void *
sig_thread(void *arg)
{
   sigset_t *set = arg;
   int s, sig;
   pthread_t pid;
   pid = pthread_self();

   for (;;) {
       s = sigwait(set, &sig);
       if (s != 0)
           handle_error_en(s, "sigwait");
       printf("Signal handling thread[%d] got signal %d\n",pid, sig);
   }
}

int
main(int argc, char *argv[])
{
   pthread_t thread[5];
   sigset_t set;
   int s;
   int sig;
   int i;

   /* Block SIGQUIT and SIGUSR1; other threads created by main()
      will inherit a copy of the signal mask. */

   sigemptyset(&set);
   sigaddset(&set, SIGQUIT);
   sigaddset(&set, SIGUSR1);
   s = pthread_sigmask(SIG_BLOCK, &set, NULL);
   if (s != 0)
       handle_error_en(s, "pthread_sigmask");

   for(i = 0;i<5;i++)
   {
      s = pthread_create(&thread[i], NULL, &sig_thread, (void *) &set);
      if (s != 0)
         handle_error_en(s, "pthread_create");
   }

   /* Main thread carries on to create other threads and/or do
      other work */

   //pause();            /* Dummy pause so we can test program */
    for (;;) {
        s = sigwait(&set, &sig);
        if (s != 0)
            handle_error_en(s, "sigwait");
        printf("Signal handling thread(main) got signal %d\n", sig);
    }
    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test3 test3.c -lpthread
[root@localhost test-src]# for i in {1..20}; do kill -QUIT %1; done
Signal handling thread(main) got signal 3
Signal handling thread[1332807424] got signal 3
Signal handling thread[1332807424] got signal 3
Signal handling thread[1332807424] got signal 3
Signal handling thread(main) got signal 3
Signal handling thread[1299236608] got signal 3
Signal handling thread(main) got signal 3
Signal handling thread(main) got signal 3
Signal handling thread[1332807424] got signal 3
Signal handling thread[1332807424] got signal 3
Signal handling thread(main) got signal 3
Signal handling thread(main) got signal 3
Signal handling thread[1299236608] got signal 3
Signal handling thread[1299236608] got signal 3
Signal handling thread[1332807424] got signal 3
</pre>
**说明：**

1) 这里我们采用脚本来向test3发送信号，否则可能比较难以见到信号被投递到子线程

2) 我们循环20次，发送了20个QUIT信号，但是```Signal handling...```只打印了15次，这是因为在信号中断的过程中一般会屏蔽接收相同的信号，直到从上一次信号中断返回为止，因此这里我们的打印次数一定<=20。我们可以在每次调用之后睡眠一段时间，这时候就可以看到打印次数为20。


## 2. Linux中setitimer()定时器
{% highlight string %}
#include <sys/time.h>

int getitimer(int which, struct itimerval *curr_value);
int setitimer(int which, const struct itimerval *new_value,
             struct itimerval *old_value);
{% endhighlight %}
系统为每一个进程提供了3种间隔的定时器，每一种都在其特定的时间域内进行递减。当任何一个定时器到期之后，都会有一个信号发送到进程，然后根据设置定时器可能就会再进行重启。这三种类型的定时器分别为：

* **ITIMER_REAL**: 在实际时间域进行递减，到期后会投递一个SIGALRM信号。

* **ITIMER_VIRTUAL**: 以该进程在用户态花费的时间来计算，到期后会投递一个SIGVALRM信号。

* **ITIMER_PROF**: 以该进程在用户态和内核态花费的时间来计算，到期后会投递一个ITIMER_PROF信号。

Timer值由如下结构体定义：
{% highlight string %}
struct itimerval {
   struct timeval it_interval; /* next value */
   struct timeval it_value;    /* current value */
};

struct timeval {
   time_t      tv_sec;         /* seconds */
   suseconds_t tv_usec;        /* microseconds */
};
{% endhighlight %}
函数**getitimer()**会用当前的设定值填充```curr_value```指向的结构体。curr_value->it_value会被设置为当前定时器的剩余时间，或者在该定时器被disable之后会被设置为0。相似的,curr_value->it_interval会被设置为重置值。

函数**setitimter()**会设置指定的定时器(REAL,VIRTUAL,PROF)的值为```new_value```。假如```old_value```不为NULL，则该定时器的原来的值会存放于该参数。

定时器(REAL,VIRTUAL,PROF)会从it_value一直递减到0，然后定时间隔会被重置为it_interval。如果一个定时器其it_value被设置为0，则该定时器就会停止。

函数成功时返回0，失败时返回-1.

<br />
请参看如下的例子程序：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>


sig_atomic_t interrupt;

static void sighandler(int sig)
{
    switch(sig)
    {
        case SIGINT:
           interrupt = 1;
           write(1,"RECV SIGINT\n",strlen("RECV SIGINT\n"));
           break;
        case SIGALRM:
           write(1,"RECV SIGALRM\n",strlen("RECV SIGALRM\n"));
           break;
    }
}

int main(int argc,char *argv[])
{
    sigset_t set;
    struct sigaction sa;
    struct itimerval   itv;
    int delay;

    memset(&sa,0x0,sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGINT,&sa,NULL) == -1)
    {
        printf("sigaction(SIGINT) failed\n");
        exit(1);
    }

    memset(&sa,0x0,sizeof(struct sigaction));
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGALRM,&sa,NULL) == -1)
    {
        printf("sigaction(SIGALRM) failed\n");
        exit(1);
    }
    
    sigemptyset(&set);
    sigaddset(&set,SIGTERM);
    sigaddset(&set,SIGALRM);
    if(sigprocmask(SIG_BLOCK,&set,NULL) == -1)
    {
        printf("sigprocmask failed\n");
        exit(2);
    }
    
    sigemptyset(&set);

   #if 0
    delay = 8000;
    itv.it_interval.tv_sec = 4;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = delay / 1000;
    itv.it_value.tv_usec = (delay % 1000 ) * 1000;
    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
       printf("setitimer failed\n");
    }
   #endif

    for(;;)
    {
      #if 1
        //当定时器过期之后，定时器会被重置为it_interval(这里为0，因此该定时器会停止,可认为本定时器是一个
        //一次性定时器)，然后产生SIGALRM信号，sigsuspend()接收到该信号，调用该信号的处理函数sighandler(),
        //在处理函数执行完毕后，sigsuspend()返回。返回后执行interrupt条件，然后再开启一次新的循环。
        delay = 8000;
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = 0;
        itv.it_value.tv_sec = delay / 1000;
        itv.it_value.tv_usec = (delay % 1000 ) * 1000;
        if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
           printf("setitimer failed\n");
        }
       #endif
       
        sigsuspend(&set);
        
        if(interrupt)
        {
            interrupt = 0;
            if(getitimer(ITIMER_REAL,&itv) == 0)
            {
                printf("it_interval: %u %u\n",itv.it_interval.tv_sec,itv.it_interval.tv_usec);
                printf("it_value: %u %u\n", itv.it_value.tv_sec,itv.it_value.tv_usec);
            }
        }
    }

    return 0x0;

}
{% endhighlight %}

编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
^CRECV SIGINT
it_interval: 0 0
it_value: 5 778717
^CRECV SIGINT
it_interval: 0 0
it_value: 6 32245
RECV SIGALRM
RECV SIGALRM
RECV SIGALRM
Killed
</pre>


## 3. nginx平滑升级时reconfigure
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
root@ubuntu:/usr/local/nginx# kill -s SIGHUP 6960                                //重新加载配置
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
nobody    6985  6960  0 07:53 ?        00:00:00 nginx: worker process              //对比上面如上pid，进程id发送了改变
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

上面我们首先运行nginx，然后向master进程发送SIGUSR2信号执行平滑升级，这时我们可以看到有两个master进程同时在运行，这时我们发送SIGHUP信号重新加载配置，可以看到会新创建出一个worker process。
<pre>
这里注意到，执行./nginx -s reload命令，并没有增加子进程。这时因为此时nginx.pid文件已经发生了改变，我们这里是向新升级的
nginx发送了SIGHUP信号，因此该新升级的master处理该信号时是先将其worker子进程shutdown掉，然后重新创建子进程。因此我们会
发现子进程pid发生了改变。
</pre>




<br />
<br />
<br />

