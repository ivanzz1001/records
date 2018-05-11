---
layout: post
title: Linux可打开最大文件数
tags:
- LinuxOps
categories: linuxOps
description: Linux可打开最大文件数
---


在Linux下有时会遇到```socket/file: can't open so many files```的问题。其实Linux是有文件句柄限制的，而且Linux默认一般都是1024（阿里云主机默认是65535）。在生产环境中很容易达到这个值，因此这里就会成为系统的瓶颈。

<!-- more -->


## 1. 相关概念说明

下面我们主要会介绍:

* nr_open

* file_max/file_nr

* ulimit -n

### 1.1 nr_open说明
<pre>
This denotes the maximum number of file-handles a process can
allocate. Default value is 1024*1024 (1048576) which should be
enough for most machines. Actual limit depends on RLIMIT_NOFILE
resource limit.
</pre>
用于指示单个进程可以分配的最大文件描述符。默认值为1048576, 这对于大多数机器是够用的。而一个进程实际可以打开的文件句柄数还会受```RLIMIT_NOFILE```的限制。

可以通过通过如下命令来查看：
<pre>
# cat /proc/sys/fs/nr_open
1048576
</pre>


### 1.2 file_max/file_nr说明
执行如下命令查看file_max/file_nr的说明：
{% highlight string %}
# man 5 proc
/proc/sys/fs/file-max
              This  file defines a system-wide limit on the number of open files for all processes.  (See also setrlimit(2), which can be used by a process to set the
              per-process limit, RLIMIT_NOFILE, on the number of files it may open.)  If you get lots of error messages in the kernel log about running  out  of  file
              handles (look for "VFS: file-max limit <number> reached"), try increasing this value:

                  echo 100000 > /proc/sys/fs/file-max

              The kernel constant NR_OPEN imposes an upper limit on the value that may be placed in file-max.

              If  you  increase /proc/sys/fs/file-max, be sure to increase /proc/sys/fs/inode-max to 3-4 times the new value of /proc/sys/fs/file-max, or you will run
              out of inodes.

              Privileged processes (CAP_SYS_ADMIN) can override the file-max limit.

       /proc/sys/fs/file-nr
              This (read-only) file contains three numbers: the number of allocated file handles (i.e., the number of files presently opened); the number of free file
              handles;  and  the  maximum number of file handles (i.e., the same value as /proc/sys/fs/file-max).  If the number of allocated file handles is close to
              the maximum, you should consider increasing the maximum.  Before Linux 2.6, the kernel allocated file handles  dynamically,  but  it  didn't  free  them
              again.  Instead the free file handles were kept in a list for reallocation; the "free file handles" value indicates the size of that list.  A large num‐
              ber of free file handles indicates that there was a past peak in the usage of open file handles.  Since Linux 2.6, the kernel does deallocate freed file
              handles, and the "free file handles" value is always zero.
{% endhighlight %}
```file_max```用于限制整个系统能够分配的文件句柄数。而关于```file-nr```，再看如下说明：
<pre>
Historically,the kernel was able to allocate file handles
dynamically, but not to free them again. The three values in
file-nr denote the number of allocated file handles, the number
of allocated but unused file handles, and the maximum number of
file handles. Linux 2.6 always reports 0 as the number of free
file handles -- this is not an error, it just means that the
number of allocated file handles exactly matches the number of
used file handles.
</pre>
由于历史原因，Linux内核能够动态的分配文件句柄，并且在分配之后并不会进行释放。 file-nr总共有3个数据组成： 第一列表示当前已分配且正在使用的文件句柄数； 第二列表示已分配但目前未使用的文件句柄数（当前操作系统一般不会用到此特性了，因此值一般为0）； 第三列一般表示为最大的文件句柄数，通常等于```file-max```。

我们可以通过如下命令来查看这些值：
<pre>
# cat /proc/sys/fs/file-max
1584856

# cat /proc/sys/fs/file-nr
1984    0       1584856
</pre>


### 1.3 ```ulimit -n```说明
<pre>
Provides control over the resources available to the shell and to processes started by it, 
on systems that allow such control
</pre>
```ulimit -n```提供了使用shell来控制资源数量的方法。关于```ulimit -n```,需要注意如下两点：

* 提供对shell及其启动的进程的可用资源（包括文件句柄，进程数量，core文件大小等）的控制

* 这是进程级别的，也就是说系统中某个session及其启动的每个进程能打开多少个文件描述符，能fork出多少个子进程等。

我们可以通过```ulimit -a```或者```ulimit -n```来进行查看：
<pre>
# ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 62474
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1000000
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 4096
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
</pre>
上面可以看到： ```open files    (-n) 1000000```,这是Linux操作系统对一个进程打开的文件句柄数量的限制（也包含打开的套接字数量）。这1000000个文件中还得除去每个进程必然打开的标准输入、标准输出、标准错误、服务器监听socket、进程间通信的unix域socket等文件，那么剩下的可用于客户端socket连接和其他的文件句柄。

其实可打开的文件句柄数还有一个```soft limit```、```hard limit```的概念。其中```soft limit```称为软限制，当达到该限制数时会产生警告； 而```hard limit```称为硬限制，可打开的文件句柄数不能超过该值。```soft limit```必须小于等于```hard limit```。

综合上面的说明， 当个进程可以打开的文件句柄首先不能超过系统总的文件句柄数```file-max```，其次还不能超过```nr_open```数目， 然后还不能超过```hard limit```限制数, 并且最好还不能超过```soft limit```，否则会产生相应的警告。


## 2. 修改方法


### 2.1 修改soft limit及hard limit
**1) 临时生效**
<pre>
# ulimit -SHn 10000
</pre>
其实ulimit 命令身是分软限制和硬限制，加```-H```就是硬限制，加```-S```就是软限制。默认显示的是软限制，如果运行```ulimit```命令修改时没有加上```-H```或```-S```，就是两个参数一起改变。

{% highlight string %}
软限制和硬限制的区别？

硬限制就是实际的限制，而软限制是警告限制，它只会给出警告。
{% endhighlight %}


**2) 永久生效**

要想ulimits 的数值永久生效，必须修改配置文件/etc/security/limits.conf 
在该配置文件中添加
<pre>
* soft nofile 65535

* hard nofile 65535  
</pre>

可以通过如下的命令来添加：
{% highlight string %}
# echo "* soft nofile 65535" >> /etc/security/limits.conf

# echo "* hard nofile 65535" >> /etc/security/limits.conf
{% endhighlight %}

上面```*```表示所用的用户。

注意： 有时```*```指定所有用户可能在不重启机器的情况下并不能生效， 此时可能需要指定具体的用户，例如root用户
<pre>
root soft nofile 1800000
root hard nofile 2000000
</pre>

另外可能修改在```/etc/pam.d/login```文件中添加如下行：
<pre>
session required pam_limits.so
</pre>


### 2.2 修改系统总限制
其实上的修改都是对一个进程打开的文件句柄数量的限制，我们还需要设置系统的总限制才可以。

假如，我们设置进程打开的文件句柄数是1024 ，但是系统总线制才500，所以所有进程最多能打开文件句柄数量500。从这里我们可以看出只设置进程的打开文件句柄的数量是不行的。所以需要修改系统的总限制才可以。

**1） 修改file-max**

* 临时生效方法（重启机器后会失效）
<pre>
# echo  6553560 > /proc/sys/fs/file-max
</pre>

* 永久生效方法： 修改/etc/sysctl.conf文件，加入
<pre>
fs.file-max = 6553560 
</pre>

**2) 修改nr_open**

另外,我们可能还需要修改```nr_open```:

* 临时生效方法（重启机器会失效）
<pre>
# echo 2000000 > /proc/sys/fs/nr_open
</pre>

* 永久生效方法: 修改/etc/sysctl.conf文件，加入
<pre>
fs.nr_open = 2000000
</pre>
也可以通过如下命令来修改：
<pre>
# sysctl -w fs.nr_open=2000000
</pre>






<br />
<br />

1. [Linux最大文件打开数](https://www.cnblogs.com/pangguoping/p/5791432.html)

2. [设置Linux最大打开文件数和进程数](https://blog.csdn.net/hellozpc/article/details/47952867)

3. [Linux下设置最大文件打开数nofile及nr_open、file-max](https://www.cnblogs.com/zengkefu/p/5635153.html)

4. [nr_open , file_max & file_nr区别](https://blog.csdn.net/google0802/article/details/52304776)

5. [Documentation for /proc/sys/fs/*](https://www.kernel.org/doc/Documentation/sysctl/fs.txt)

6. [ulimit -a最大打开文件数显示1024，但是/etc/security/limits.conf显示10000处理方法](http://blog.itpub.net/25462274/viewspace-2123294/)
<br />
<br />
<br />


