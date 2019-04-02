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

![cpp-linux-semphore](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_linux_semphore.jpg)

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

### 1.5 特殊键值IPC_PRIVATE
semget()的调用者可以给其key参数传递一个特殊的键值IPC_PRIVATE（其值为0），这样无论该信号量是否已经存在，semget()都将创建一个新的信号量。使用该键值创建的信号量并非像它的名字声称的那样是进程私有的。其他进程，尤其是子进程，也有方法访问这个信号量。所以semget()的man手册的BUGS部分上说，使用名字IPC_PRIVATE有些误导（历史原因），应该称为IPC_NEW。比如下面的代码就在父、子进程间使用一个IPC_PRIVATE信号量来同步(ipc_private.c)：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>


union semun {
	int              val;
	struct semid_ds *buf;
	unsigned short  *array;
	struct seminfo  *__buf;
};


/*
 * op为-1时执行P操作，op为1时执行V操作
 */
int pv(int semid, int op){
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = op;
	sem_b.sem_flg = SEM_UNDO;

	return semop(semid, &sem_b, 1);
}

int main(int argc, char *argv[]){
	union semun sem_un;
	int ret;
	pid_t pid;

	int semid = semget(IPC_PRIVATE, 1, 0666);
	if (semid < 0){
		printf("create semphore failure(%d)\n", errno);
		return -1;
	}

	sem_un.val = 1;
	ret = semctl(semid, 0, SETVAL, sem_un);
	if (ret < 0){
		printf("set semphore value failure(%d)\n", errno);
		goto END;
	}


	pid = fork();
	if(pid == 0){
		//child process
		printf("child try to get binary sem\n");

		/*
		 * 在父子进程间共享IPC_PRIVATE信号量的关键就在于二者都
		 * 可以操作该信号量的标识符semid
		 */

		ret = pv(semid, -1);
		if (ret < 0){
			printf("child get sem failure(%d: %d)\n", ret, errno);
			exit(-1);
		}
		printf("child get the sem and would release it after 5 seconds\n");
		sleep(5);

		ret = pv(semid, 1);
		if(ret < 0){
			printf("child release sem failure(%d: %d)\n", ret, errno);
			exit(-1);
		}
		printf("child success release the sem\n");

		exit(0);

	}else if(pid > 0){
		//parent process
		printf("parent try to get binary sem\n");

		ret = pv(semid, -1);
		if (ret < 0){
			printf("parent get sem failure(%d: %d)\n", ret, errno);
			exit(-1);
		}
		printf("parent get the sem and would release it after 5 seconds\n");
		sleep(5);

		ret = pv(semid, 1);
		if(ret < 0){
			printf("parent release sem failure(%d: %d)\n", ret, errno);
			exit(-1);
		}
		printf("parent success release the sem\n");

		exit(0);

	}else{
		printf("fork failure(%d)", errno);
		goto END;
	}
	waitpid(pid, NULL, 0);

END:
	ret = semctl(semid, 0, IPC_RMID, sem_un);
	if (ret < 0){
		printf("remove semphore failure(%d)\n", errno);
		return -1;
	}

	return 0x0;
}

{% endhighlight %}
编译运行：
<pre>
# gcc -o ipc_private ipc_private.c
# ./ipc_private 
parent try to get binary sem
child try to get binary sem
child get the sem and would release it after 5 seconds
child success release the sem
parent get the sem and would release it after 5 seconds
parent success release the sem
</pre>

另外一个例子是： 工作在prefork模式下的httpd网页服务器程序使用1个IPC_PRIVATE信号量来同步各子进程对epoll_wait()的调用权限。

## 2. 共享内存
共享内存是最高效的IPC机制，因为它不涉及进程之间的任何数据传输。这种高效率带来的问题是，我们必须使用其他辅助手段来同步对共享内存的访问，否则会产生竞态条件。因此，共享内存通常和其他进程间通信方式一起使用。

Linux共享内存API都定义在sys/shm.h头文件中，包括4个系统调用： shmget、shmat、shmdt和shmctl。我们将依次讨论之。

### 2.1 shmget系统调用
shmget()系统调用创建一段新的共享内存，或者获取一段已存在的共享内存。其定义如下：
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
{% endhighlight %}
和semget()系统调用一样，key参数是一个键值，用来标识一段全局唯一的共享内存。size参数指定共享内存的大小，单位是字节。如果是创建新的共享内存，则size值必须被指定。如果是获取已存在的共享内存，则可以把size设置为0.

shmflg参数的使用和含义与semget()系统调用的```semflg```参数相同。不过shmget()支持两个额外的标志————SHM_HUGETLB和SHM_MORESERVE。它们的含义如下：

* SHM_HUGETLB: 类似于mmap()的MAP_HUGETLB标志，系统将使用```大页面```来为共享内存分配空间

* SHM_MORESERVE: 类似于mmap()的MAP_MORESERVE标志，不为共享内存保留交换分区(swap空间)。这样，当物理内存不足的时候，对该共享内存执行写操作将触发SIGSEGV信号。

shmget()成功时返回一个正整数值，它是共享内存的标识符。shmget()失败时返回-1，并设置errno。

如果shmget()用于创建共享内存，则这段共享内存的所有字节都被初始化为0，与之关联的内核数据结构shmid_ds将被创建并初始化。shmid_ds结构体的定义如下：
{% highlight string %}
struct shmid_ds {
	struct ipc_perm shm_perm;    /* 共享内存的操作权限*/
	size_t          shm_segsz;   /* 共享内存的大小，单位为字节 */
	time_t          shm_atime;   /* 对这段共享内存最后一次调用shmat()的时间*/
	time_t          shm_dtime;   /* 对这段共享内存最后一次调用shmdt()的时间*/
	time_t          shm_ctime;   /* 对这段共享内存最后一次电泳shmctl()的时间*/
	pid_t           shm_cpid;    /* 创建者的PID*/
	pid_t           shm_lpid;    /* 最后一次执行shmat()或shmdt()操作的进程的PID*/
	shmatt_t        shm_nattch;  /* 目前关联到此共享内存的进程数量*/
	...                          /* 省略一些填充字段*/
};
{% endhighlight %}
shmget()对shmid_ds结构体的初始化包括：

* 将shm_perm.cuid和shm_perm.uid设置为调用进程的有效用户ID

* 将shm_perm.cgid和shm_perm.gid设置为调用进程的有效组ID

* 将shm_perm.mode的最低9位设置为shmflg参数的最低9位

* 将shm_segsz设置为size

* 将shm_lpid、shm_nattach、shm_atime、shm_dtime设置为0

* 将shm_ctime设置为当前时间

### 2.2 shmat和shmdt系统调用
共享内存被创建/获取之后，我们不能立即访问它，而是需要先将它关联到进程的地址空间中。使用完共享内存之后，我们也需要将它从进程地址空间中分离。这两项任务分别由如下两个系统调用实现：
{% highlight string %}
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);

int shmdt(const void *shmaddr);
{% endhighlight %}
其中```shmid```是由shmget()调用返回的共享内存标识符。shmaddr参数指定将共享内存关联到进程的哪块地址空间，最终的效果还受到shmflg参数的可选标志SHM_RND的影响：

* 如果shmaddr为NULL，则被关联的地址由操作系统选择。这是推荐做法，以确保代码的可移植性

* 如果shmaddr非空，并且SHM_RND标志未被设置，则共享内存被关联到addr指定的地址处。

* 如果shmaddr非空，并且设置了SHM_RND标志，则被关联的地址是[shmaddr-(shmaddr % SHMLBA)]。SHMLBA的含义是“段低端边界地址倍数”(Segment Low Boundary Address Multiple)，它必须是内存页面大小(PAGE_SIZE)的整数倍。现在的Linux内核中，它等于一个内存页面大小。SHM_RND的含义是圆整(round)，即将共享内存被关联的地址向下圆整到离shmaddr最近的SHMLBA的整数倍地址处。

除了SHM_RND标志外，shmflg参数还支持如下标志：

* SHM_RDONLY: 进程只能读取共享内存中的内容。若没有指定该标志，则进程可同时对共享内存进行读写操作（当然，还需要在创建共享内存的时候指定其读写权限）

* SHM_REMAP: 如果地址shmaddr已经被关联到一段共享内存上，则重新关联

* SHM_EXEC: 它指定对共享内存段的执行权限。对共享内存而言，执行权限实际上和读权限是一样的。

shmat()成功时返回共享内存被关联到的地址，失败时则返回(void *)-1并设置errno。shmat()成功时，将修改内核数据结构shmid_ds的部分字段，如下：

* 将shm_nattach加1

* 将shm_lpid设置为调用进程的PID

* 将shm_atime设置为当前时间

shmdt()函数将关联到shmaddr处的共享内存从进程中分离。它成功时返回0，失败时则返回-1并设置errno。shmdt()在成功调用时将修改内核数据结构shmid_ds的部分字段，如下：

* 将shm_nattach减1

* 将shm_lpid设置为调用进程的PID

* 将shm_dtime设置为当前时间

### 2.3 shmctl系统调用
shmctl()系统调用控制共享内存的某些属性，其定义如下：
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
{% endhighlight %}
其中，shmid参数是由shmget()调用返回的共享内存标识符。cmd参数指定要执行的命令。shmctl()支持的所有命令如下表所示：
<pre>
   命令                                    含义                                              shmctl成功时的返回值
--------------------------------------------------------------------------------------------------------------------------
IPC_STAT            将共享内存相关的内核数据结构复制到buf(第3个参数，下同）中                               0
IPC_SET             将buf中的部分成员复制到共享内存相关的内核数据结构中，同时内核数据结构                     0
                    中的shmid_ds.shm_ctime将被更新 

IPC_RMID            将共享内存打上删除标记。这样当最后一个使用它的进程调用shmdt()将它从进程                  0
                    中分离时，该共享内存就被删除了

IPC_INFO            获取系统共享内存资源配置信息，将结果存储在buf中。应用程序需要将buf转换成            内核共享内存信息数组中已经 
                    shminfo结构体类型来读取这些系统信息。shminfo结构体与seminfo类似，这里不再         被使用的项的最大索引值
                    赘述。

SHM_INFO            与IPC_INFO类似。不过返回的是已经分配的共享内存占用的资源信息。应用程序需要            同IPC_INFO
                    将buf转换成shm_info结构体类型来读取这些信息。shm_info结构体与shminfo类似，
                    这里不再赘述

SHM_STAT            与IPC_STAT类似。不过此时shmid参数不是用来表示共享内存标识符，而是内核中共享        内核共享内存信息数组中索引值为
                    内存信息数组的索引（每个共享内存的信息都是该数组中的一项）                         shm_id的共享内存的标识符

SHM_LOCK            禁止共享内存被移动至交换分区                                                         0
SHM_UNLOCK          允许共享内存被移动至交换分区                                                         0
</pre>
shmctl()成功时的返回值取决于cmd参数, 如上表所示。shmctl()失败时返回-1，并设置errno。

### 2.4 共享内存的POSIX方法

我们知道mmap()，可以利用它的MAP_ANONYMOUS标志我们可以实现父、子进程之间的匿名内存共享。通过打开同一个文件，mmap()也可以实现无关进程之间的内存共享。Linux提供了另外一种利用mmap()在无关进程之间共享内存的方式。这种方式无须任何文件支持，但它需要先使用如下函数来创建或打开一个POSIX共享内存对象：
{% highlight string %}
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int shm_open(const char *name, int oflag, mode_t mode);
{% endhighlight %}
shm_open()的使用方法与open()系统调用完全相同。name参数指定要创建/打开的共享内存对象。从可移植性的角度考虑，该参数应该使用```/somename```格式： 以```/```开始，后接多个字符，且这些字符都不是```/```；以```\0```结尾，长度不超过NAME_MAX(通常是255)

oflag参数指定创建方式。它可以是下列标志中的一个或多个的按位或：

* O_RDONLY: 以只读方式打开共享内存对象

* O_RDWR: 以可读、可写方式打开共享内存对象

* O_CREAT: 如果共享内存对象不存在，则创建之。此时mode参数的最低9位将指定该共享内存对象的访问权限。共享内存对象被创建的时候，其初始长度为0

* O_EXCL: 和O_CREAT一起使用，如果由name指定的共享内存对象已经存在，则shm_open()调用返回错误，否则就创建一个新的共享内存对象。

* O_TRUNC: 如果共享内存对象已经存在，则把它截断，使其长度为0

shm_open()调用成功时返回一个文件描述符。该文件描述符可用于后续的mmap()调用，从而将共享内存关联到调用进程。shm_open()失败时返回-1，并设置errno。

和打开的文件最后需要关闭一样，由shm_open()创建的共享内存对象使用完之后也需要被删除，这个过程是通过如下函数实现的：
{% highlight string %}
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int shm_unlink(const char *name);
{% endhighlight %}

该函数将name参数指定的共享内存对象标记为等待删除。当所有使用该共享内存对象的进程都使用unmap()将它从进程中分离之后，系统将销毁这个共享内存对象所占据的资源。

如果代码中使用了上述POSIX共享内存函数，则编译的时候需要指定链接选项```-lrt```。

## 3. 消息队列
消息队列是在两个进程之间传递二进制块数据的一种简单有效的方式。每个数据块都有一个特定的类型，接收方可以根据类型来有选择地接收数据，而不一定像管道和命名管道那样必须以先进先出的方式接收数据。

Linux消息队列的API都定义在sys/msg.h头文件中，包括4个系统调用： msgget()、msgsnd()、msgrcv()和msgctl()。我们将依次讨论之。

### 3.1 msgget系统调用
msgget()系统调用创建一个消息队列，或者获取一个已有的消息队列。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msgget(key_t key, int msgflg);
{% endhighlight %}
和semget()系统调用一样，key参数是一个键值，用来标识一个全局唯一的消息队列。msgflg参数的使用和含义与semget()系统调用的```semflg```参数相同。

msgget()成功时返回一个正整数值，它是消息队列的标识符。msgget()失败时返回-1，并设置errno。

如果msgget()用于创建消息队列，则与之关联的内核数据结构msgid_ds将被创建并初始化。msgid_ds结构体的定义如下：
{% highlight string %}
struct msqid_ds {
	struct ipc_perm msg_perm;     /*消息队列的操作权限*/
	time_t          msg_stime;    /*最后一次调用msgsnd()的时间*/
	time_t          msg_rtime;    /*最后一次调用msgrcv()的时间*/
	time_t          msg_ctime;    /*最后一次被修改的时间*/
	unsigned long   __msg_cbytes; /*消息队列中已有的字节数*/
	msgqnum_t       msg_qnum;     /*消息队列中已有的消息数*/

	msglen_t        msg_qbytes;   /*消息队列允许的最大字节数*/
	pid_t           msg_lspid;    /*最后执行msgsnd()的进程的PID*/
	pid_t           msg_lrpid;    /*最后执行msgrcv()的进程的PID*/
};
{% endhighlight %}

### 3.2 msgsnd系统调用
msgsnd()系统调用把一条消息添加到消息队列中。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
{% endhighlight %}
msgid参数是由msgget()调用返回的消息队列标识符。```msgp```参数指向一个准备发送的消息，消息必须被定义为如下类型：
{% highlight string %}
struct msgbuf {
	long mtype;       /* 消息类型*/
	char mtext[1];    /* 消息数据*/
};
{% endhighlight %}
其中mtype成员指定消息的类型，它必须是一个正整数。ntext是消息数据。msgsz参数是消息的数据部分(mtext)的长度。这个长度可以为0，表示没有消息数据。

msgflg参数控制msgsnd()的行为。它通常仅支持IPC_NOWAIT标志，即以非阻塞的方式发送消息。默认情况下，发送消息时如果消息队列满了，则msgsnd()将阻塞。若IPC_NOWAIT标志被指定，则msgsnd()将立即返回并设置errno为EAGAIN。

处于阻塞状态的msgsnd()调用可能被如下两种异常情况所中断：

* 消息队列被移除，此时msgsnd()调用将立即返回并设置errno为EIDRM

* 程序接收到信号，此时msgsnd()调用立即返回并设置errno为EINTR

msgsnd()成功时返回0，失败则返回-1并设置errno。msgsnd()成功时将修改内核数据结构msgid_ds的部分字段，如下所示：

* 将msg_qnum加1

* 将msg_lspid设置为调用进程的PID

* 将msg_stime设置为当前的时间


### 3.3 msgrcv系统调用
msgrcv()系统调用从消息队列中获取消息。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
              int msgflg);
{% endhighlight %}
msgid参数是由msgget()调用返回的消息队列标识符。```msgp```参数用于存储接收的消息， msgsz参数指的是消息数据部分的长度。msgtyp参数指定接收何种类型的消息。我们可以使用如下几种方式来指定消息类型：

* msgtyp等于0： 读取消息队列中的第一个消息

* msgtyp大于0： 读取消息队列中第一个类型为msgtyp的消息（除非指定了标志MSG_EXCEPT，见后文）

* msgtyp小于0： 读取消息队列中第一个类型值比msgtyp绝对值小的消息；

参数msgflg控制msgrcv()函数的行为，它可以是如下一些标志的按位或：

* IPC_NOWAIT: 如果消息队列中没有消息，则msgrcv调用立即返回并设置errno为EAGAIN;

* MSG_EXCEPT: 如果msgtyp大于0，则接收消息队列中第一个非msgtyp类型的消息；

* MSG_NOERROR: 如果消息数据部分的长度超过了msg_sz，就将它截断

处于阻塞状态的msgrcv调用还可能被如下两种异常情况所中断：

* 消息队列被移除，此时msgrcv()调用将立即返回并设置errno为EIDRM

* 程序接收到信号，此时msgrcv()调用立即返回并设置errno为EINTR

msgrcv()成功时返回实际接收的消息长度(接收进mtext数组中数据的字节数），失败则返回-1并设置errno。msgrcv()成功时将修改内核数据结构msgid_ds的部分字段，如下所示：

* 将msg_qnum减1

* 将msg_lspid设置为调用进程的PID

* 将msg_rtime设置为当前的时间


### 3.4 msgctl系统调用
msgctl系统调用控制消息队列的某些属性。其定义如下：
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msgctl(int msqid, int cmd, struct msqid_ds *buf);
{% endhighlight %}
msgid参数是由msgget()调用返回的消息队列标识符。cmd参数指定要执行的命令。msgctl()所支持的命令如下表所示：
<pre>
   命令                                    含义                                              msgctl成功时的返回值
--------------------------------------------------------------------------------------------------------------------------
IPC_STAT           将消息队列关联的内核数据结构复制到buf(第3个参数，下同）中                            0
IPC_SET            将buf中的部分成员复制到消息队列关联的内核数据结构中，同时内核数据结构中的              0
                   msgid_ds.msg_ctime被更新

IPC_RMID           立即移除消息队列，唤醒所有等待读消息和写消息的进程（这些调用立即返回并设置              0
                   errno为EIDRM)

IPC_INFO           获取系统消息队列资源配置信息，将结果存储在buf中。应用程序需要将buf转换成         内核消息队列数组中已经被使用的项的
                   msginfo结构体类型来读取这些系统信息。msginfo结构体与seminfo类似，这里          最大索引值
                   不再赘述

MSG_INFO           与IPC_INFO类似。不过buf返回的是已经分配的消息队列占用的资源信息                     同IPC_INFO
MSG_STAT           与IPC_STAT类似，不过此时msgid参数不是用来表示消息队列标识符，而是内核消息       内核消息队列信息数组中索引值为msgid
                   队列信息数组的索引（每个消息队列的信息都是该数组中的一项）                      的消息队列的标识符
</pre>
msgctl()成功时的返回值取决于cmd参数，如上表所示。msgctl()函数失败时返回-1，并设置errno。

### 3.5 消息队列使用示例
消息发送端(msgqueue_send.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>


struct msg_st{
	long mtype;
	char mtext[BUFSIZ];   /*BUFSIZ: default 8192*/
};

int main(int argc, char *argv[]){
	int msgid;
	struct msg_st data;

	key_t key = ftok("/tmp", 66);
	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
	if (msgid == -1){
		printf("create msg queue(%d) failure(%d)\n",key, errno);
		return -1;
	}

	while(1){
		char msg[512];
		memset(msg, 0x0, sizeof(msg));
		data.mtype = 1;

		printf("input message: ");
		fgets(msg, sizeof(msg), stdin);
		strcpy(data.mtext, msg);

		if(msgsnd(msgid, &data, strlen(msg) + 1, 0) < 0){
			printf("send msg failure(%d)\n", errno);
			break;
		}

		if(strncmp(msg, "QUIT", 4) == 0)
			break;
	}

	if(msgctl(msgid, IPC_RMID, NULL) < 0){
		printf("remove msg failure(%d)\n", errno);
		return -1;
	}

	return 0x0;
}
{% endhighlight %}
消息接收端(msgqueue_recv.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>


struct msg_st{
	long mtype;
	char mtext[BUFSIZ];   /*BUFSIZ: default 8192*/
};

int main(int argc, char *argv[]){
	int msgid;
	struct msg_st data;

	key_t key = ftok("/tmp", 66);
	msgid = msgget(key, IPC_EXCL | 0666);
	if (msgid == -1){
		printf("get msg queue(%d) failure(%d)\n",key, errno);
		return -1;
	}

	while(1){
		if(msgrcv(msgid, &data, BUFSIZ, 1, 0x0) < 0){
			printf("recv msg failure(%d)\n", errno);
			break;
		}
		printf("data: %s\n", data.mtext);
		if (strncmp(data.mtext, "QUIT", 4) == 0)
			break;
	}
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o msgqueue_send msgqueue_send.c 
# gcc -o msgqueue_recv msgqueue_recv.c 
# ./msgqueue_send
input message: hello
input message: world
input message: QUIT

# ./msgqueue_recv
data: hello

data: world

data: QUIT
</pre>

## 4. IPC命令
上述三种System V IPC进程间通信方式都使用一个全局唯一的键值（key)来描述一个共享资源。当程序调用semget()、shmget()或者msgget()时，就创建了这些共享资源的一个实例。Linux提供了```ipcs```命令，以观察当前系统上拥有哪些共享资源实例。例如：
<pre>
# ipcs

------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages    

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00000000 196608     ivan1001   600        524288     2          dest         
0x00000000 229377     ivan1001   600        4194304    2          dest         
0x00000000 327682     ivan1001   600        4194304    2          dest         
0x00000000 360451     ivan1001   600        1048576    2          dest         

------ Semaphore Arrays --------
key        semid      owner      perms      nsems     
0x00000000 0          root       666        1 
</pre>
输出结果分段显示了系统拥有的消息队列、共享内存和信号量资源。可见该系统尚未使用任何消息队列，却使用了一组键值为0(IPC_PRIVATE)的共享内存和信号量。

此外，我们可以使用ipcrm命令来删除遗留在系统中的共享资源。

















<br />
<br />

**[参看]**





<br />
<br />
<br />


