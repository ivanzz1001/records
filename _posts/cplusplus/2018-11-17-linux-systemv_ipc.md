---
layout: post
title: 多进程之间的通信
tags:
- cplusplus
categories: cplusplus
description: 多进程之间的通信
---


多进程之间有很多种方式，比如管道、socket、System V IPC等。本章我们讲述一下三种System V IPC：

* 信号量

* 共享内存

* 消息队列


<!-- more -->

## 1. 信号量

### 1.1 信号量原语

当多个进程同时访问系统上的某个资源的时候，比如同时写一个数据库的某条记录，或者同时修改某个文件，就需要考虑进程的同步问题，以确保任一时刻只有一个进程可以拥有对资源的独占式访问。通常，程序对共享资源的访问的代码只是很短的一段，但就是这一段代码引发了进程之间的竞态条件。我们称这段代码为```关键代码段```，或者```临界区```。对进程同步，也就是确保任一时刻只有一个进程能进入关键代码段。

要编写具有通用目的的代码，以确保关键代码段的独占式访问是非常困难的。有两个名为```Dekker算法```和```Peterson算法```的解决方案，它们试图从语言本身（不需要内核支持）解决并发问题。但它们依赖于忙等待，即进程要持续不断地等待某一个内存位置状态的改变。这种方式下CPU利用率太低，显然是不可取的。

Dijkstra提出的信号量(Semaphore)概念是并发编程领域迈出的重要一步。信号量是一种特殊的变量，它只能取```自然数值```并且只支持两种操作: 等待(wait)和信号(signal)。不过在Linux/Unix中，```等待```和```信号```都已经具有特殊的含义，所以对信号量的这两种操作更通常称呼是**P**、**V**操作。这两个字母来自于荷兰语单词```passeren```(传递，就好像进入临界区）和```vrijgeven```(释放，就好像退出临界区)。假设有信号量SV，则对它的**P**、**V**操作含义如下：

<pre>
P(SV): 如果SV的值大于0，就将它减1； 如果SV的值等于0，则挂起进程的执行。

V(SV): 如果有其他进程因为等待SV而挂起，则唤醒之； 如果没有，则将SV加1.
</pre>

信号量的取值可以是任何自然数。但最常用的、最简单的信号量是二进制信号量，它只能取0和1两个数值。这里我们也仅讨论二进制信号量。使用二进制信号量同步两个进程，以确保关键代码段的独占式访问的一个典型例子如下图所示：



在上图中，当关键代码段可用时，二进制信号量SV的值为1，进程A和B都有机会进入关键代码段。如果此时进程A执行了```P(SV)```操作将SV减1，则进程B若再执行```P(SV)```操作就会被挂起。直到进程A离开关键代码段，并执行```V(SV)```操作将SV加1，关键代码段才重新变得可用。如果此时进程B因为等待SV而处于挂起状态，则它将被唤醒，并进入关键代码段。同样，这时进程A如果再执行```P(SV)```操作，则也只能被操作系统挂起以等待进程B退出关键代码段。
{% highlight string %}
注意： 使用一个普通变量来模拟二进制信号量是行不通的，因为所有高级语言都没有一个原子操作可以同时
      完成如下两步操作： 检测变量是否为true/false，如果是则再将它设置为false/true。
{% endhighlight %}

Linux信号量的API都定义在sys/sem.h头文件中，主要包含3个系统调用： semget()、setop()和semctl()。它们都被设计为操作一组信号量，即```信号量集```，而不是单个信号量，因此这些接口看上去多少比我们期望的要复杂一点。我们将分成3个部分来依次讨论之：

### 1.2 semget系统调用
semget系统调用创建一个新的```信号量集```,或者获取一个已存在的信号量集。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semget(key_t key, int nsems, int semflg);
{% endhighlight %}

* key参数是一个键值，用来标识一个全局的信号量集，就像文件名全局唯一地标识一个文件一样。要通过信号量通信的进程需要使用相同的键值来创建/获取该信号量。

* nsems参数指定要创建/获取的信号量集中信号量的数目。如果是创建信号量，则该值必须被指定； 如果是获取已存在的信号量，则可以设置为0.

* semflg参数指定一组标志。它低端的9个比特是该信号量的权限，其格式和含义都与系统调用open()的mode参数相同。此外，它还可以和```IPC_CREAT```标志做```按位或```运算以创建新的信号量集。此时即使信号量已存在，semget()也不会产生错误。我们还可以联合使用```IPC_CREAT```和```IPC_EXCL```标志来确保创建一组新的、唯一的信号量集。在这种情况下，如果信号量集已经存在，则semget()返回错误并设置errno为```EEXIST```。这种创建信号量的行为与用```O_CREAT```和```O_EXCL```标志调用open()来排他式地打开一个文件相似。

semget()成功时返回一个正整数值，它是信号量集的标识符；semget()失败时返回-1，并设置errno。

如果semget()用于创建信号量集，则与之关联的内核数据结构```semid_ds```将被创建并初始化。semid_ds结构体的定义如下：
{% highlight string %}
#include <sys/sem.h>

/*该结构体用于描述IPC对象（信号量、共享内存和消息队列）的权限*/
struct ipc_perm
{
	key_t key;      /*键值*/
	uid_t uid;      /*所有者的有效用户ID*/
	gid_t gid;      /*所有者的有效组ID*/
	uid_t cuid;     /*创建者的有效用户ID*/
	gid_t cgid;     /*创建者的有效组ID*/
	mode_t mode;    /*访问权限*/

	...             /*省略其他填充字段*/
};

struct semid_ds
{
	struct ipc_perm sem_perm;     /*信号量的操作权限*/
	unsigned long int sem_nsems;  /*该信号量集的信号量数目*/
	time_t sem_otime;             /*最后一次调用semop()的时间*/
	time_t sem_ctime;             /*最后一次电泳semctl()的时间*/

	...                           /*省略其他填充字段*/
};
{% endhighlight %}
semget()对semid_ds结构体的初始化包括：

* 将sem_perm.cuid和sem_perm.uid设置为调用进程的有效用户ID

* 将sem_perm.cgid和sem_perm.gid设置为调用进程的有效组ID

* 将sem_perm.mode的最低9位设置为sem_flags参数的最低9位

* 将sem_nsems设置为num_sems

* 将sem_otime设置为0

* 将sem_ctime设置为当前系统时间

### 1.3 semop系统调用
semop系统调用改变信号量的值，即执行P、V操作。在讨论semop()之前，我们需要先介绍与每个信号量关联的一些重要内核变量：
<pre>
unsigned short semval;      /*信号量的值*/
unsigned short semzcnt;     /*等待信号量值变为0的进程数量*/
unsigned short semncnt;     /*等待信号量值增加的进程数量*/
pid_t sempid;               /*最后一次执行semop操作的进程ID*/
</pre>

semop()对信号量的操作实际上就是对这些内核变量的操作。semop()的定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semop(int semid, struct sembuf *sops, unsigned nsops);
{% endhighlight %}

1) **semid参数**

sem_id参数是由semget()调用返回的信号量集标识符，用以指定被操作的目标信号量集。


2) **sops参数**

sops参数指向一个sembuf结构体类型的数组，sembuf结构体的定义如下：
{% highlight string %}
struct sembuf{
	unsigned short int sem_num;
	short int sem_op;
	short int sem_flg;
};
{% endhighlight %}
其中，sem_num成员是信号量集中信号量的编号，0表示信号量集中的第一个信号量。sem_op成员指定操作类型，其可选值为正整数、0和负整数。每种类型的操作的行为又受到sem_flg成员的影响。sem_flg的可选值为```IPC_NOWAIT```和```SEM_UNDO```。IPC_NOWAIT的含义是，无论信号量操作是否成功，semop()操作都将立即返回，则类似于非阻塞IO操作。SEM_UNDO的含义是，当进程退出时取消正在进行的semop()操作。具体来说，sem_op和sem_flg将按照如下方式来影响semop()的行为：

* 如果sem_op大于0，则semop()将被操作的信号量的值```semval```增加sem_op。该操作要求调用进程对被操作的信号量集拥有写权限。此时若设置了SEM_UNDO标志，则系统将更新进程的```semadj```变量（用以跟踪进程对信号量的修改情况）

* 如果sem_op等于0，则表示这是一个```等待0```(wait-for-zero)操作。该操作要求要求调用进程对被操作信号量集拥有读权限。如果此时信号量的值是0，则调用立即成功返回。如果信号量的值不为0，则semop()失败返回或者阻塞进程以等待信号量变为0.在这种情况下，当```IPC_NOWAIT```标志被指定时，semop()立即返回一个错误，并设置errno为EAGAIN。如果未指定IPC_NOWAIT标志，则信号量的```semzcnt```值加1，进程被投入睡眠直到下列3个条件之一发生：
<pre>
1. 信号量的值semval变为0，此时系统将该信号量的semzcnt减1；

2. 被操作信号量所在的信号量集被进程移除，此时semop()调用失败返回，errno被设置为EIDRM;

3. 调用被信号中断，此时semop()调用失败返回，errno被设置为EINTR，同时系统将该信号量的semzcnt值减1；
</pre>

* 如果sem_op小于0，则表示对信号量值进行减操作，即期望获得信号量。该操作要求调用进程对被操作信号量集拥有写权限。如果信号量的值semval大于或等于sem_op的值，则semop()操作成功，调用进程立即获得信号量，并且将该信号量的```semval```值减去sem_op的绝对值。此时如果设置了SEM_UNDO标志，则系统将更新进程的semadj变量。如果信号量的值semval小于sem_op的绝对值，则semop()失败返回或者阻塞以等待信号量可用。在这种情况下，当IPC_NOWAIT标志被指定时，semop()立即返回一个错误，并设置errno为EAGAIN。如果未指定IPC_NOWAIT标志，则信号量的```semncnt```值加1，进程被投入睡眠直到下列3个条件之一发生：
<pre>
1. 信号量的值semval变得大于或等于sem_op的绝对值，此时系统将该信号量的semncnt值减1，并将semval减去
   sem_op的绝对值，同时，如果SEM_UNDO标志被设置，则系统更新semadj变量；

2. 被操作信号量所在的信号量集被进程移除，此时semop()调用失败返回，errno被设置为EIDRM;

3. 调用被信号中断，此时semop()调用失败返回，errno被设置为EINTR，同时系统将该信号量的semncnt值减1；
</pre>

3） **nsops参数**

semop()系统调用的第3个参数nsops指定要执行的操作个数，即```sops```数组中元素的个数。semop()对数组```sops```中的每个成员按照数组依次执行操作，并且该过程是原子操作，以避免别的进程在同一时刻按照不同的顺序对该信号量集中的信号量执行semop()操作导致竞态条件。

semop()成功时返回0，失败则返回-1并设置errno。失败的时候，```sops```数组中指定的所有操作都不被执行。


### 1.4 semctl系统调用
semctl系统调用允许调用者对信号量进行直接控制。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semctl(int semid, int semnum, int cmd, ...);
{% endhighlight %}
semid参数是由semget()调用返回的信号量集标识符，用以指定被操作的信号量集。semnum参数指定被操作的信号量在信号量集中的编号。cmd参数指定要执行的命令。有的命令需要调用者传递第4个参数。第4个参数的类型由用户自己定义，但```sys/sem.h```头文件给出了它的推荐格式：
{% highlight string %}
union semun {
	int              val;    /*用于SETVAL命令*/
	struct semid_ds *buf;    /*用于IPC_STAT和IPC_SET命令*/
	unsigned short  *array;  /*用于GETALL和SETALL命令*/
	struct seminfo  *__buf;  /*用与IPC_INFO命令*/
};

struct  seminfo {
	int semmap;    /*在信号量map中有多少信号量集，当前Linux内核没有使用*/
	int semmni;    /*系统最多可允许的信号量集数目*/
	int semmns;    /*系统最多可拥有的信号量数目(所有信号量集中信号量的总数目）*/
	int semmnu;    /*系统级别所支持的最大undo数目，Linux内核没有使用*/
	int semmsl;    /*一个信号量集中最多允许包含的信号量数目*/
	int semopm;    /*semop一次最多能执行的sem_op操作数目*/
	int semume;    /*一个进程所支持的最大undo数目，Linux内核没有使用*/
	int semusz;    /*sem_undo结构体的大小*/
	int semvmx;    /*最大所允许的信号量值*/
	int semaem;    /*最多允许的undo次数（带SEM_UNDO 标志的semop操作的次数)*/
};
{% endhighlight %}

semctl支持的所有命令如下所示：
<pre>
   命令                                    含义                                     semctl成功时的返回值
--------------------------------------------------------------------------------------------------------------
IPC_STAT       将信号量集关联的内核数据结构复制到semun.buf中                                  0
IPC_SET        将semun.buf中的部分成员复制到信号量集关联的内核数据结构中，同时内核               0
               数据中的semid_ds.sem_ctime被更新                

IPC_RMID       立即移除信号量集，唤醒所有等待该信号量集的进程(semop返回错误，并设置               0
               errno为EIDRM)

IPC_INFO       获取系统信号量资源配置信息，将结果存储在semun.__buf中，这些信息的含义       内核信号量集数组中已被使用的项
               见结构体seminfo的注释部分                                              的最大索引值

SEM_INFO       与IPC_INFO类似，不过semun.__buf.semusz被设置为系统目前所拥有的信号量      同IPC_INFO
               集数目，而semun.__buf.semaem被设置为系统目前拥有的信号量数目 

SEM_STAT       与IPC_STAT类似，不过此时semid参数不是用来表示信号量集标识符，而是内核       内核信号量集数组中索引值为semid
               中信号量集数组的索引（系统中所有信号量集都是该数组中的一项)                 的信号量集的标识符

GETALL         将由semid标识的信号量集中的所有信号量的semval值导出到semun.array中         0
GETNCNT        获取信号量的semncnt值                                                  信号量的semncnt值

GETPID         获取信号量的sempid值                                                   信号量的sempid值
GETVAL         获取信号量的semval值                                                   信号量的semval值
GETZCNT        获取信号量的semzcnt值                                                  信号量的semzcnt值

SETALL         用semun.array中的数据填充由semid标识的信号量集中所有信号量的semval         0
               值，同时内核数据中的semid_ds.sem_ctime被更新

SETVAL         将信号量的semval值设置为semun.val，同时内核数据中的semid_ds.sem_ctime     0
               被更新
----------------------------------------------------------------------------------------------------------------


注意： 这些操作中，GETNCNT、GETPID、GETVAL、GETZCNT和SETVAL操作的是单个信号量，它是由标识符semid指定的信号量集中第semnum
      个信号量；而其他操作针对的是整个信号量集，此时semctl的参数sem_num被忽略。
</pre>

semctl成功时的返回值取决于cmd参数，如上表所示。semctl失败时，返回-1，并设置errno。




<br />
<br />

**[参看]**





<br />
<br />
<br />


