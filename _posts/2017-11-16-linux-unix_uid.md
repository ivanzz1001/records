---
layout: post
title: Linux进程控制之uid/euid/suid
tags:
- LinuxOps
categories: linux
description: Linux进程控制之uid/euid/suid
---

本章我们简单讨论一下Linux进程控制之```实际用户ID```，```有效用户ID```，```设置用户ID```.


<!-- more -->

## 1. 进程的用户ID

在Unix系统中，```特权```以及```访问控制```是基于用户ID和组ID的。当程序需要增加特权，或者需要访问当前并不允许访问的资源时，我们需要更换自己的用户ID或组ID，使得新ID具有合适的特权或访问权限。与此类似，当程序需要降低其特权或阻止对某些资源的访问时，也需要更换用户ID或组ID，新ID不具有相应特权或访问这些资源的能力。

一般而言，在设计应用时，我们总是试图使用最小特权（least privilege)模型。依照此模型，我们的程序应当只具有为完成给定任务所需的最小特权。这降低了由恶意用户试图哄骗我们的程序以未料的方式使用特权造成的安全性风险。

可以用```setuid```函数设置实际用户ID、有效用户ID。与此类似，可以用```setgid```函数设置实际组ID和有效组ID。
{% highlight string %}
#include <sys/types.h>
#include <unistd.h>

int setuid(uid_t uid);
int setgid(gid_t gid);

两个函数返回值：若成功，返回0；若出错，返回-1
{% endhighlight %}

关于谁能更改ID有若干规则。现在优先考虑更改用户ID的规则（关于用户ID我们所说明的一切都适用于组ID）：

* 若进程具有超级用户权限，则setuid函数将实际用户ID、有效用户ID以及保存的设置用户ID(saved set-user-ID)设置为UID。

* 若进程没有超级用户特权，但是uid等于实际用户ID或保存的设置用户ID，则setuid只将有效用户ID设置为uid。不更改实际用户ID和保存的设置用户ID。

* 如果上面两个条件都不满足，则errno设置为EPERM，并返回-1。

在此假定```_POSIX_SAVED_IDS```为真。如果没有提供这种功能，则上面所说的关于保存的设置用户ID部分都无效。
<pre>
在POSIX.1 2001版中，保存的ID是强制性功能。而在较早的版本中，它们是可选择的。为了弄清楚某种实现是否
支持这一功能，应用程序在编译时可以测试常量_POSIX_SAVED_IDS，或者在运行时以_SC_SAVED_IDS参数调用
sysconf函数。
</pre>

<br />

关于内核所维护的3个用户ID，还需要注意以下几点：

* 只有超级用户进程可以更改实际用户ID。通常，实际用户ID是在用户登录时，由login程序设置的，而且绝不会改变它。因为login是一个超级用户进程，当它调用setuid时，设置所有3个用户ID。

* 仅当对程序文件设置了设置用户ID位时，exec才设置有效用户ID。如果设置用户ID位没有设置，exec函数不会改变有效用户ID，而将维持其现有值。任何时候都可以调用setuid，将有效用户ID设置为实际用户ID或保存的设置用户ID。自然地，不能将有效用户ID设置为任一随机值。

* 保存的设置用户ID是由exec复制有效用户ID而得到的。如果设置了文件的设置用户ID位，则在exec根据文件的用户ID设置了进程的有效用户ID以后， 这个副本就被保存起来。

下表总结了更改这3个用户ID的不同方法：

![change uid](https://ivanzz1001.github.io/records/assets/img/linux/change_uid_method.jpg)


**注意：** ```getuid```和```geteuid```函数只能获得实际的用户ID和有效公户ID的当前值。我们没有可移植的方法去获得保存的设置用户ID的当前值。
<pre>
FreeBSD 8.0 和 Linux 3.2.0提供了getresuid和getresgid函数，它们可以分别用于获取保存的设置用户ID和保存的设置组ID。
</pre>

### 1.1 函数setreuid和setregid

历史上，BSD支持setreuid函数，其功能是```交换```实际用户ID和有效用户ID的值。
{% highlight string %}
#include <sys/types.h>
#include <unistd.h>

int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);

描述：
setreuid()设置调用进程的实际用户ID和有效用户ID。

假如上述函数参数的任何一个值为-1的话，则保持该对应的ID不变。

* 对于非特权进程，只能将有效用户ID设置为实际用户ID、有效用户ID或者保存的设置用户ID
* 对于非特权用户，只能将实际用户ID设置为实际用户ID或者有效用户ID

正是由于上述这两点，所以前面才会讲 “功能是'交换'实际用户ID和有效用户ID的值”

注： 假如实际用户ID或有效用户ID被设置为不同于原实际用户ID的话，保存的设置用户ID(saved set-user-ID)会被设置为
新的有效用户ID。
{% endhighlight %}
规则很简单：一个非特权用户总能交换实际用户ID和有效用户ID。这就允许一个设置用户ID程序交换成普通的用户权限，以后又可再次交换回设置用户ID权限。POSIX.1引进了保存的设置用户ID特性后，其规则也相应加强，它允许一个非特权用户将其有效用户ID设置为保存的设置用户ID。

### 1.2 函数seteuid和setegid

POSIX.1包含了两个函数seteuid和setegid。它们类似于setuid和setgid，但只能更改有效用户ID和有效组ID。
{% highlight string %}
#include <sys/types.h>
#include <unistd.h>

int seteuid(uid_t euid);
int setegid(gid_t egid);

这两个函数返回值： 若成功，返回0；若出错，返回-1
{% endhighlight %}

一个非特权用户可以将其有效用户ID设置为其实际用户ID或其保存的设置用户ID。对于一个特权用户则可将有效用户ID设置为uid。（这区别于setuid函数，它更改所有3个用户ID。）

<br />

下图给出了上面所述的更改3个不同用户ID的各个函数：

![setuid methods](https://ivanzz1001.github.io/records/assets/img/linux/set_uid_methods.jpg)

注：可以通过如下命令设置```设置用户ID位```
<pre>
# chmod o+s file
# chmod g+s file

# chmod o-s file
# chmod g-s file
</pre>


## 2. 示例-1

如下sample.c源文件：
{% highlight string %}
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>

int main()
{
    struct passwd* ruser = NULL;
    struct passwd* euser = NULL;
    struct passwd* suser = NULL;
    uid_t uid, euid, suid;
    
    /* get uid euid suid */
    if(getresuid(&uid, &euid, &suid) != 0)
    {
        perror(0);
        return 1;
    }

    /* get name of id */
    ruser = getpwuid(uid);
    printf("real user: %s\n", ruser->pw_name);
    euser = getpwuid(euid);
    printf("effective user: %s\n", euser->pw_name);
    suser = getpwuid(suid);
    printf("saved set-user-id user: %s\n", suser->pw_name);
    return 0;
}
{% endhighlight %}
编译：
<pre>
[ivan1001@localhost test-src]$ gcc -o sample sample.c 
</pre>

**(1) 普通用户权限执行**
<pre>
[ivan1001@localhost test-src]$ ls -al sample
-rwxrwxr-x. 1 ivan1001 ivan1001 8720 Nov 16 17:46 sample

[ivan1001@localhost test-src]$ ./sample
real user: ivan1001
effective user: ivan1001
saved set-user-id user: ivan1001
</pre> 

**(2) 特权用户权限执行**
<pre>
[ivan1001@localhost test-src]$ su
Password: 
[root@localhost test-src]# ls -al sample
-rwxrwxr-x. 1 ivan1001 ivan1001 8720 Nov 16 17:46 sample

[root@localhost test-src]# ./sample
real user: root
effective user: root
saved set-user-id user: root
</pre>

**(3) 设置用户ID执行**
<pre>
[root@localhost test-src]# chmod u+s ./sample
[root@localhost test-src]# ls -al sample
-rwsrwxr-x. 1 ivan1001 ivan1001 8720 Nov 16 17:46 sample

[root@localhost test-src]# ./sample
real user: root
effective user: ivan1001
saved set-user-id user: ivan1001
</pre>


## 3. 示例-2

at程序(略）

## 4. 总结
每个Linux进程都包含如下这些属性，这些属性功能决定了该进程访问文件的权限：
<pre>
1. real user id

2. real user group

3. effective user id

4. effective user group

5. saved set-user-id

6. saved set-user-group
</pre>
*  1、2简称ruid、rgid， 由启动进程的用户决定，通常是当前登录用户（运行可执行文件的用户）；

*  3、4简称euid、egid，一般在进程启动时，直接由1、2复制而来；或者当进程对应的可执行文件的set-user-id/set-group-id (chmod u+s)标志位为true时，为该文件的所属用户/所属组。这组属性决定了进程访问文件的权限。

*  5、6简称suid、sgid，从euid、egid复制。

下面这张图描述了进程启动时，这些属性是怎么赋值的：

![linux process uid](https://ivanzz1001.github.io/records/assets/img/linux/linux-process-uid.png)

* step 1-2: 由登录用户启动运行可执行文件，启动进程；

* step 3: 设置进程的rid/rgid为当前用户的uid/gid

* step 4: 设置进程的euid/egid，根据可执行文件的set-user-id/set-group-id属性（红色：0 ， 紫色：1），为1时，设为可执行文件的uid/gid，否则从ruid/rgid拷贝。





<br />
<br />
**参看：**

1. [Linux进程权限的研究](http://blog.csdn.net/ybxuwei/article/details/23563423)


<br />
<br />
<br />


