---
layout: post
title: Linux服务器性能问题排查思路(转)
tags:
- LinuxOps
categories: linuxOps
description: Linux服务器性能问题排查思路
---


一个基于Linux操作系统的服务器运行的同时，也会表征出各种各样的参数信息。通常来说，运维人员、系统管理员会对这些数据极为敏感，但是这些参数对于开发者来说也十分重要，尤其当你的程序非正常工作的时候，这些蛛丝马迹往往会帮助快速定位跟踪问题。

这里只是一些简单的工具查看系统的相关参数，当然很多工具也是通过分析加工/proc、/sys下的数据来工作的，而那些更加细致、专业的性能监测和调优，可能还需要更加专业的工具(perf、systemtap等)和技术才能完成。


<!-- more -->


## 1. CPU和内存类
## 1.1 top命令
{% highlight string %}
# top
top - 10:09:16 up 483 days, 22:50, 21 users,  load average: 4.82, 5.66, 5.90
Tasks: 749 total,   5 running, 744 sleeping,   0 stopped,   0 zombie
%Cpu(s):  3.7 us,  3.5 sy,  0.0 ni, 92.2 id,  0.4 wa,  0.0 hi,  0.2 si,  0.0 st
KiB Mem : 26385816+total, 49007424 free, 97353928 used, 11749680+buff/cache
KiB Swap:  8388604 total,  7647364 free,   741240 used. 13031131+avail Mem 

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                                                                              
 103749 root      20   0  124844   8864    780 R 100.0  0.0   0:01.88 netstat                                                                                              
2000736 root      20   0  145312  33308    960 S  52.5  0.0 281331:14 nginx_log.sh                                                                                         
 103743 root      20   0  158400   2724   1496 R  35.8  0.0   0:00.75 top                                                                                                  
 104144 zabbix    20   0  151064   1816   1424 R  35.0  0.0   0:00.42 ps                                                                                                   
     33 root      rt   0       0      0      0 S  28.3  0.0  19290:01 migration/5                                                                                          
    229 root      rt   0       0      0      0 S  25.0  0.0  28459:40 migration/44                                                                                         
4009664 zabbix    20   0   84940   6032   5124 R  21.7  0.0 956:47.86 zabbix_agentd                                                                                        
4009669 zabbix    20   0   85036   2708   1648 R  10.8  0.0 438:49.42 zabbix_agentd                                                                                        
1174623 root      20   0 11.720g 1.732g 673348 S  10.0  0.7 107958:29 ceph-osd                                                                                             
3403995 ceph      20   0 8338516 1.374g  68500 S   8.3  0.5  63973:53 ceph-mon                                                                                             
4009667 zabbix    20   0   85048   3100   2036 D   8.3  0.0 444:52.26 zabbix_agentd                                                                                        
     13 root      rt   0       0      0      0 S   4.2  0.0  21193:41 migration/1                                                                                          
    174 root      rt   0       0      0      0 S   3.3  0.0   5759:55 migration/33                                                                                         
    184 root      rt   0       0      0      0 S   1.7  0.0   3503:26 migration/35                                                                                         
    230 root      20   0       0      0      0 S   0.8  0.0  55:57.57 ksoftirqd/44                                                                                         
 585426 root      20   0  111848   2576   1420 S   0.8  0.0   6:05.88 keepalived                                                                                           
2351396 root      20   0       0      0      0 S   0.8  0.0   0:02.14 kworker/16:0                                                                                         
2427137 nobody    20   0   50000  15012   3048 S   0.8  0.0   1:07.51 nginx                                                                                                
2427147 nobody    20   0   50000  14868   3100 S   0.8  0.0   1:03.99 nginx                                                                                                
      1 root      20   0  191256   3720   1948 S   0.0  0.0 839:55.80 systemd  
{% endhighlight %}

1） **第一行**

第一行后面的三个值是系统在之前1分钟、5分钟、15分钟的平均负载，也可以看出系统负载是上升、平稳、下降的趋势，当这个值超过CPU可执行单元的数目，则表示CPU的性能已经饱和和成为瓶颈了。


2) **第二行**

第二行统计了系统的任务状态信息。running很自然不必多说，包括正在CPU上运行的和将要被调度运行的； sleeping通常是等待事件（比如IO操作）完成的任务，细分可以包括interruptible和uniterruptible的类型；stopped是一些被暂停的任务，通常发送```SIGSTOP```或者对一个前台任务操作```CTRL+Z```可以将其暂停；zombie僵尸任务，虽然进程终止资源会被自动回收，但是含有退出任务的task descriptor需要父进程访问后才能释放，这种进程显示为defunct状态，无论是因为父进程提前退出还是未wait调用，出现这种进程都应该格外注意程序是否设计有误。

3) **第三行**

第三行CPU占用率根据类型有以下几种情况：

* (us)user: CPU在低nice值(高优先级)用户态所占用的时间（nice<=0)。正常情况下只要CPU不是很闲，那么大部分的CPU时间应该都在此执行这类程序。

* (sy)system: CPU处于内核状态所占用的时间，操作系统通过系统调用(system call)从用户态转入内核态，以执行特定的服务。通常情况下该值会比较小，但是当服务器执行的IO比较密集的时候，该值会比较大。

* (ni)nice: CPU在高nice值(低优先级）用户态以低优先级运行时占用的CPU时间(nice>0)。默认新启动的进程nice=0，是不会记录到这里的，除非手动通过renice或者setpriority()的方式修改程序的nice值。

* (id)idle: CPU在空闲状态（执行kernel idle handler)所占用的时间

* (wa)iowait: 等待IO完成所占用的时间

* (hi)irq: 系统处理硬件中断所消耗的时间

* (si)softirq: 系统处理软件中断所消耗的时间。记住软中断分为softirq、tasklets(其实是前者的特例）、work queues，不知道这里统计的是哪些时间，毕竟work queues的执行已经不是中断上下文了。

* (st)steal: 在虚拟机情况下才有意义，因为虚拟机下CPU也是共享物理CPU的，所以这段时间表明虚拟机等待hypervisor调度CPU的时间，也意味着这段时间hypervisor将CPU调度给别的CPU执行，这个时段的CPU资源被```stolen```了。这个值在我KVM的VPS机器上是不为0的，但也只有0.1这个数量级，是不是可以用来判断VPS超售的情况？


CPU占用率高很多情况下意味着一些东西，这也给服务器CPU使用率过高情况下指明了相应的排查思路：

* 当user占用率过高的时候，通常是某些个别的进程占用了大量的CPU，这时候很容易通过top找到该程序。此时，如果怀疑程序异常，可以通过perf等思路找出热点调用函数来进一步排查。

* 当system占用率过高的时候，如果IO操作（包括终端IO）比较多，可能会造成这部分的CPU占用率高，比如在file server、database server等类型的服务器上，否则(比如>20%)很可能有些部分的内核、驱动模块有问题。

* 当nice占用率过高的时候，通常是有意行为，当进程的发起者知道某些进程占用较高的CPU，会设置其nice值确保不会淹没其他进程对CPU的使用请求

* 当iowait占用率过高的时候，通常意味着某些程序的IO操作效率很低，或者IO对应设备的性能很低以至于读写操作需要很长时间来完成；

* 当irq/softirq占用率过高的时候，很可能某些外设出现问题，导致产生大量的irq请求，这时候通过检查/proc/interrupts文件来深究问题所在；

* 当steal占用率过高的时候，黑心厂商虚拟机超售了吧！

4） **第四、五行**

第四行和第五行是物理内存和虚拟内存（交换分区）的信息。total = free + used + buff/cache，现在```buffers```和```cached mem```信息总和到一起了，但是buffers和cached mem的关系很多地方没说清楚。其实通过对比数据，这两个值就是/proc/meminfo中的Buffers和Cached字段： Buffers是针对raw disk的块缓存，主要是以raw block的方式缓存文件系统的元数据（比如超级块信息等)，这个值一般比较小（20M左右）；而Cached是针对某些具体的文件进行读缓存，以增加文件的访问效率而使用的，可以说是用于文件系统中文件缓存使用。


而```avail Mem```是一个新的参数值，用于指示在不进行交换的情况下，可以给新开的程序多少内存空间。大致和free+buff/cached相当，而这也印证了上面的说法，free+buff/cache才是真正可用的物理内存。并且，使用交换分区不见得是坏事情，所以交换分区使用率不是什么严重参数，但是频繁的swap in/out就不是好事情了，这种情况需要注意，通常表示物理内存紧缺的情况。

5） **其他行**

最后是每个程序的资源占用列表，其中CPU的使用率是所有CPU core占用率的总和。通常执行top的时候，本身该程序会大量的读取/proc操作，所以基本该top程序本身也会是名列前茅的。

<br />

top虽然非常强大，但是通常用于控制台实时监测系统信息，不适合长时间(几天、几个月）监测系统的负载信息，同时对于短命的进程也会遗漏无法给出统计信息。


### 1.2 vmstat
vmstat是除top之外另一个常用的系统监测工具。下面是我用```./b2 -j4```编译boost时的系统负载：
<pre>
# vmstat
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 5  0      0 2421236 1149228 7637000    0    0     0     6    1    0  1  0 98  0  0
</pre>
* ``` r ```表示可运行进程(running+waiting to run)的数目，数据大致相符； 

* ``` b ```表示的是uninterruptible睡眠进程的数目；

*  ```swpd ```表示使用到的虚拟内存的数量，跟top命令中swap-used的数值是一个含义，而如手册所说，通常情况下buffers数目要比cached mem小的多，buffers一般是20MB这么个数量级；

* io域的bi、bo表明每秒钟向磁盘接收和发送的块数目（blocks/s)

* system域的in表明每秒钟的系统中断数（包括时钟中断），cs表明因为进程切换导致上下文切换的数目；

<br />
说到这里，想到以前很多人纠结编译linux kernel的时候，```-j```参数究竟是CPU Core还是CPU Core+1？ 通过上面修改```-j```参数值编译boost和linux kernel的同时开启vmstat监控，发现两种情况下context switch基本没有变化，且也只有显著增加```-j```值后context switch才会显著增加，看来不必过于纠结这个参数了，虽然具体编译时间长度我还没测试。资料说如果不是在系统启动或者benchmark状态，参数context switch > 100000程序肯定有问题。


### 1.3 pidstat
如果想对某个进程进行全面具体的追踪，没有什么比pidstat更合适的了————栈空间、缺页情况、主被动切换等信息尽收眼底。这个命令最有用的参数是```-t```，可以将进程中各个线程的详细信息罗列出来。
<pre>
# ps -ef | grep rados | grep -v grep
root     2644056       1  1 Feb20 ?        02:02:13 radosgw -c /etc/ceph/ceph.conf -n client.radosgw.ceph-node001
# pidstat -t -p 2644056 | more
Linux 3.10.0-514.el7.x86_64 (ceph-node001)       02/27/2019      _x86_64_        (56 CPU)

02:09:31 PM   UID      TGID       TID    %usr %system  %guest    %CPU   CPU  Command
02:09:31 PM     0   2644056         -    0.01    0.02    0.00    0.03    23  radosgw
02:09:31 PM     0         -   2644056    0.00    0.00    0.00    0.00    23  |__radosgw
02:09:31 PM     0         -   2644057    0.00    0.00    0.00    0.00     7  |__log
02:09:31 PM     0         -   2644060    0.00    0.00    0.00    0.00    14  |__service
02:09:31 PM     0         -   2644061    0.00    0.00    0.00    0.00    50  |__admin_socket
02:09:31 PM     0         -   2644062    0.00    0.00    0.00    0.00     9  |__radosgw
02:09:31 PM     0         -   2644063    0.00    0.00    0.00    0.00    22  |__ms_dispatch
02:09:31 PM     0         -   2644064    0.00    0.00    0.00    0.00    50  |__ms_local
02:09:31 PM     0         -   2644065    0.00    0.00    0.00    0.00    24  |__ms_reaper
02:09:31 PM     0         -   2644066    0.00    0.00    0.00    0.00    15  |__safe_timer
02:09:31 PM     0         -   2644067    0.00    0.00    0.00    0.00    50  |__fn_anonymous
02:09:31 PM     0         -   2644068    0.00    0.00    0.00    0.00    21  |__ms_pipe_write
02:09:31 PM     0         -   2644069    0.00    0.00    0.00    0.00    37  |__ms_pipe_read
</pre>

pidstat有许多选项。下面我们对几个常用的简单进行说明：

1) **-r选项**

显示缺页错误和内存使用状况，缺页错误是程序需要访问映射在虚拟内存空间中但是还尚未被加载到物理内存中的一个分页，缺页错误两个主要类型是:

* minflt/s: 指的是minor faults，当需要访问的```物理页面```因为某些原因（比如共享页面、缓存机制）已经存在于物理内存中了，只是在当前进程的页表中没有引用，MMU只需要设置对应的entry就可以了，这个代价是相当小的。

* majflt/s: 指的是major faults，MMU需要在当前可用物理内存中申请一块空闲的```物理页面```(如果没有可用的空闲页面，则需要将别的物理页面切换到交换空间去以释放得到空闲物理页面），然后从外部加载数据到该物理页面中，并设置好对应的entry，这个代价是相当高的，和前者有几个数据级的差异。

下面给出运行示例：
<pre>
# pidstat -r
Linux 3.10.0-514.el7.x86_64 (ceph-node001)       02/27/2019      _x86_64_        (56 CPU)

02:32:25 PM   UID       PID  minflt/s  majflt/s     VSZ    RSS   %MEM  Command
02:32:25 PM     0         1      0.43      0.00  191368   3888   0.00  systemd
02:32:25 PM     0       903     91.45      0.00  269896 156936   0.06  systemd-journal
02:32:25 PM     0       930      0.00      0.00  192640    740   0.00  lvmetad
02:32:25 PM     0      1400      0.00      0.00   55432   1464   0.00  auditd
02:32:25 PM     0      1427      0.92      0.00   21740   1108   0.00  irqbalance
02:32:25 PM    81      1432      0.02      0.00   25740   2136   0.00  dbus-daemon
02:32:25 PM     0      1434      0.00      0.00  195044    564   0.00  gssproxy
02:32:25 PM     0      1438      0.00      0.00  442284   4116   0.00  NetworkManager
02:32:25 PM   998      1439      0.00      0.00  534240   3040   0.00  polkitd
02:32:25 PM     0      1460      0.03      0.00  126232   1068   0.00  crond
02:32:25 PM     0      1470      0.00      0.00  110048    672   0.00  agetty
02:32:25 PM     0      1926      0.00      0.00  560264   7612   0.00  tuned
02:32:25 PM     0      2672      0.00      0.00   89540   1196   0.00  master
02:32:25 PM    89      2797      0.00      0.00   89820   3144   0.00  qmgr
02:32:25 PM     0      5172      0.20      0.00  106000   3232   0.00  sshd
02:32:25 PM     0      8690    242.90      0.01 11868200 1708864   0.65  ceph-osd
02:32:25 PM     0      8726    262.47      0.01 12089936 1543824   0.59  ceph-osd
02:32:25 PM     0      8753    267.99      0.01 11894960 1611164   0.61  ceph-osd
</pre>

2) **-s 选项**

栈使用状况，包括StkSize为线程保留的栈空间，以及StkRef实际使用的栈空间，使用```ulimit -s```发现Centos 7.3上面默认栈空间大小是8192K

下面给出运行示例：
<pre>
# pidstat -p 2644056 -s 1 
Linux 3.10.0-514.el7.x86_64 (ceph-node001)       02/27/2019      _x86_64_        (56 CPU)

02:46:13 PM   UID       PID StkSize  StkRef  Command
02:46:22 PM     0   2644056    136      12  radosgw
02:46:29 PM     0   2644056    136      12  radosgw
</pre>

3) **-u 选项**

CPU使用率情况。例如：
<pre>
# pidstat -p 2644056 -u 1 1
Linux 3.10.0-514.el7.x86_64 (ceph-node001)       02/27/2019      _x86_64_        (56 CPU)

02:48:36 PM   UID       PID    %usr %system  %guest    %CPU   CPU  Command
02:48:37 PM     0   2644056    1.00    1.00    0.00    2.00    23  radosgw
Average:        0   2644056    1.00    1.00    0.00    2.00     -  radosgw
</pre>

4) **-w 选项**

上下文切换情况。还分为主动切换和被动切换：

* cswch/s： 每秒任务主动(自愿的)切换上下文的次数，当某一任务处于阻塞等待状态，将主动让出自己的CPU资源

* nvcswch/s: 每秒任务被动(不自愿的）切换上下文的次数，CPU分配给某一任务的时间片已经用完，因此将强迫该进程让出CPU的执行权

参看如下示例：
<pre>
# pidstat -p 2644056 -w 1 3
Linux 3.10.0-514.el7.x86_64 (ceph-node001)       02/27/2019      _x86_64_        (56 CPU)

02:53:02 PM   UID       PID   cswch/s nvcswch/s  Command
02:53:03 PM     0   2644056      0.00      0.00  radosgw
02:53:04 PM     0   2644056      0.00      0.00  radosgw
02:53:05 PM     0   2644056      0.00      0.00  radosgw
Average:        0   2644056      0.00      0.00  radosgw
</pre>



<br />
<br />

**[参看]:**

1. [Linux 服务器性能问题排查思路](https://www.cnblogs.com/wqcheng/p/7764926.html)



<br />
<br />
<br />


