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

现在，Linux系统将用户密码保存在另一个单独的文件中（称为shadow文件，位置在/etc/shadow)。只有特定的程序才能访问这个文件，比如登录程序。

你已经看到了，/etc/passwd是标准的文本文件。你可以用任何文本编辑器来直接手动地在/etc/passwd文件里进行用户管理，比如添加、修改或删除用户账户。但这样做及其危险，如果/etc/passwd文件损坏了，系统就无法读取它的内容了，这样用户就无法正常登录了，甚至连root用户也会无法登录。用标准的Linux用户管理工具去执行这些用户管理功能就会安全许多。

### 1.2 /etc/shadow文件

/etc/shadow文件能对Linux系统如何管理密码有更多的控制。只有root用户才能访问/etc/shadow文件，这让它比起/etc/passwd来安全许多。

/etc/shadow文件为系统上的每个用户账户保存了一条记录。记录就像下面这样：
<pre>
# cat /etc/shadow
root:$1$WiHVDIwj$L934QNDKQu3qNSePb5oYh.::0:99999:7:::
bin:*:17110:0:99999:7:::
daemon:*:17110:0:99999:7:::
</pre>
在/etc/shadow文件的每条记录中有9个字段：

* 与/etc/passwd文件名中的登录名对应的登录名

* 加密后的密码

* 自1970年1月1日（上次修改密码的日期）到当天的天数

* 多少天后才能更改密码

* 多少天后必须更改密码

* 密码过期前提前多少天提醒用户更改密码

* 密码过期后多少天禁用用户账户

* 用户账户被禁用的日期。用自1970年1月1日到当天的天数表示

* 预留字段，给将来使用

使用shadow密码系统后，Linux系统可以更好地控制用户密码了。它可以控制用户多久更改一次密码，以及密码未更新的话多久后禁用该用户账户。

### 1.3 添加新用户

添加新用户到Linux系统的工具是useradd。这个命令提供了一次性创建新用户账户及设置用户HOME目录结构的简便方法。useradd命令使用系统的默认值以及命令行参数来设置用户账户。可以用useradd命令加```-D```参数来查看你的Linux系统的系统默认值：
{% highlight string %}
# /usr/sbin/useradd -D
GROUP=100
HOME=/home
INACTIVE=-1
EXPIRE=
SHELL=/bin/bash
SKEL=/etc/skel
CREATE_MAIL_SPOOL=yes
{% endhighlight %}
<pre>
说明： 一些Linux发行版会把Linux用户和组工具放在/usr/sbin目录下，有可能不在PATH环境变量里。如果你的Linux系统是这样
      的话，可以将这个目录添加进PATH环境变量，或者使用绝对路径名来运行这个程序
</pre>
```-D```参数显示了在创建新用户时如果你不在命令行指定的话，useradd命令使用的默认值。这个例子列出了这些默认值：

* 新用户会被添加到GID为100的公共组；

* 新用户的HOME目录将会位于/home/loginname

* 新用户账户密码在过期后不会被禁用

* 新用户账户未被设置为某个日期后就过期

* 新用户账户将bash shell作为默认的shell

* 系统会将/etc/skel目录下的内容复制到用户的HOME目录下；

* 系统为该用户账户在mail目录下创建一个用于接收邮件的文件

倒数第二个值很有意思。useradd命令允许管理员创建一份默认的HOME目录配置，然后把它作为创建新用户HOME目录的模板。这样，就能自动在每个新用户的HOME目录里放置默认的系统文件。在Centos7.3 Linux系统上，/etc/skel目录下有以下文件：
<pre>
# ls -al /etc/skel/ 
total 24
drwxr-xr-x.   3 root root   78 Oct 21  2017 .
drwxr-xr-x. 138 root root 8192 Apr 18 17:23 ..
-rw-r--r--.   1 root root   18 Aug  3  2016 .bash_logout
-rw-r--r--.   1 root root  193 Aug  3  2016 .bash_profile
-rw-r--r--.   1 root root  231 Aug  3  2016 .bashrc
drwxr-xr-x.   4 root root   39 Oct 21  2017 .mozilla
</pre>
通过这些文件的名字，我们大概可以看出这些文件是做什么的。它们是bash shell环境的标准启动文件。系统会自动将这些默认文件复制到你创建的每个用户的HOME目录。




<br />
<br />

**[参看]**






<br />
<br />
<br />


