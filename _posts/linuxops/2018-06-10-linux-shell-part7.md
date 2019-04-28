---
layout: post
title: 理解Linux文件权限
tags:
- LinuxOps
categories: linuxOps
description: 理解Linux文件权限
---



缺乏安全性的系统不是完整的系统。系统上必须要有一套保护文件不被非授权用户访问或修改的机制。Linux沿用了Unix文件权限的办法，即允许用户和组基于每个文件和目录的一组安全性设置来访问文件。本章将介绍如何用Linux文件安全系统来在需要时共享数据和保护数据。

<!-- more -->


## 1. Linux的安全性
Linux安全系统的核心是用户账户。每个能进入Linux系统的用户都会被分配一个唯一的用户账户。用户对系统上对象的访问权限取决于他们登录系统时用的账户。

用户权限是通过创建用户时分配的用户ID(User ID，通常缩写为UID)来跟踪的。UID是个数值，每个用户都有个唯一的UID。但在登录系统时不是用UID来登录，而是用登录名(login name)。登录名是用户用来登录系统的最长8字符的字符串（字符可以是数字或字母），同时会关联一个对应的密码。

Linux系统使用特定的文件和工具来跟踪和管理系统上的用户账户。在我们讨论文件权限之前， 先来看一下Linux是怎样处理用户账户的。本节会介绍管理用户账户需要的文件和工具，这样在处理文件权限问题时，你就知道如何使用它们了。

### 1.1 /etc/passwd文件

Linux系统使用一个专门的文件来将用户的登录名匹配到对应的UID值。这个文件就是/etc/passwd文件，它包含了一些与用户有关的信息。下面是Linux系统上典型的/etc/passwd文件的一个例子：
{% highlight string %}
# more /etc/passwd
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
daemon:x:2:2:daemon:/sbin:/sbin/nologin
shutdown:x:6:0:shutdown:/sbin:/sbin/shutdown
halt:x:7:0:halt:/sbin:/sbin/halt
mail:x:8:12:mail:/var/spool/mail:/sbin/nologin
operator:x:11:0:operator:/root:/sbin/nologin
games:x:12:100:games:/usr/games:/sbin/nologin
ftp:x:14:50:FTP User:/var/ftp:/sbin/nologin
nobody:x:99:99:Nobody:/:/sbin/nologin
ivan1001:x:1000:1000:Centos7.01:/home/ivan1001:/bin/bash
mysql:x:27:27:MySQL Server:/var/lib/mysql:/bin/false
....
{% endhighlight %}
root用户账户是Linux系统的管理员，通常分配给它的UID是0。就像上例中显示的，Linux系统会为各种各样的功能创建不同的用户账户，而这些账户并不是真的用户。这些账户称作系统账户，是系统上运行的各种服务进程访问资源用的特殊账户。所有运行在后台的服务都需要用一个系统用户账户登录到Linux系统上。

在安全成为一个大问题之前，这些服务经常会用根账户登录。遗憾的是，如果有非授权的用户攻入了这些服务中的一个，他就能作为root用户进入整个系统了。为了防止这种情况发生，现在几乎每个Linux服务器上后台运行的服务都有自己的用户账户。这样，即使有人攻入了某个服务，他也无法访问整个系统。

Linux为系统账户预留了500以下的UID值。有些服务甚至要特定的UID才能正常工作。为普通用户创建账户时，大多数Linux系统会将500起始的第一个可用UID分配给这个账户（这未必适用于所有的Linux发行版）。

你可能已经注意到/etc/passwd文件中还有很多用户名和UID之外的信息。/etc/passwd文件的字段包含了如下信息：

* 用户登录名

* 用户密码

* 用户账户的UID

* 用户账户的GID

* 用户账户的文本描述（称为备注字段）

* 用户HOME目录的位置

* 用户的默认shell

/etc/passwd文件中的密码字段都被设置成了```x```，这并不是说所有的用户账户都用相同的密码。在早期Linux上， /etc/passwd文件里有加密后的用户密码。但鉴于很多程序都需要访问/etc/passwd文件获取用户信息，这就成了一个安全隐患。随着用来破解加密过的密码的工具的不断演进，用心不良的人开始忙于破解存储在/etc/passwd文件中的密码。Linux开发人员需要重新构思这个策略。








<br />
<br />

**[参看]**






<br />
<br />
<br />


