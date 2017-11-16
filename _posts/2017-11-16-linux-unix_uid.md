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

**(1)** 若进程具有超级用户权限，则setuid函数将实际用户ID、有效用户ID以及保存的设置用户ID(saved set-user-ID)设置为UID。

**(2)** 若进程没有超级用户特权，但是uid等于实际用户ID或保存的设置用户ID，则setuid只将有效用户ID设置为uid。不更改实际用户ID和保存的设置用户ID。

**(3)** 如果上面两个条件都不满足，则errno设置为EPERM，并返回-1。

在此假定```_POSIX_SAVED_IDS```为真。如果没有提供这种功能，则上面所说的关于保存的设置用户ID部分都无效。
<pre>
在POSIX.1 2001版中，保存的ID是强制性功能。而在较早的版本中，它们是可选择的。为了弄清楚某种实现是否
支持这一功能，应用程序在编译时可以测试常量_POSIX_SAVED_IDS，或者在运行时以_SC_SAVED_IDS参数调用
sysconf函数。
</pre>

<br />

关于内核所维护的3个用户ID






<br />
<br />
<br />


