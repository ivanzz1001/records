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

倒数第二个值很有意思。useradd命令允许管理员创建一份默认的HOME目录配置，然后把它作为创建新用户HOME目录的模板。这样，就能自动在每个新用户的HOME目录里放置默认的系统文件。在Ubuntu16.04 Linux系统上，/etc/skel目录下有以下文件：
{% highlight string %}
# ls -al /etc/skel/
total 40
drwxr-xr-x   2 root root  4096 Feb 23 22:17 .
drwxr-xr-x 131 root root 12288 Apr 28 06:15 ..
-rw-r--r--   1 root root   220 Aug 31  2015 .bash_logout
-rw-r--r--   1 root root  3771 Aug 31  2015 .bashrc
-rw-r--r--   1 root root  8980 Apr 20  2016 examples.desktop
-rw-r--r--   1 root root   655 Jun 24  2016 .profile
{% endhighlight %}
通过这些文件的名字，我们大概可以看出这些文件是做什么的。它们是bash shell环境的标准启动文件。系统会自动将这些默认文件复制到你创建的每个用户的HOME目录。

你可以用默认系统参数创建一个新用户账户来试一下，并检查一下新用户的HOME目录：
{% highlight string %}
# useradd -m test
# ls -al /home/test
total 32
drwxr-xr-x 2 test test 4096 Apr 28 09:16 .
drwxr-xr-x 4 root root 4096 Apr 28 09:16 ..
-rw-r--r-- 1 test test  220 Aug 31  2015 .bash_logout
-rw-r--r-- 1 test test 3771 Aug 31  2015 .bashrc
-rw-r--r-- 1 test test 8980 Apr 20  2016 examples.desktop
-rw-r--r-- 1 test test  655 Jun 24  2016 .profile
{% endhighlight %}

默认情况下，useradd命令不会创建HOME目录，但是```-m```命令行选项会叫它创建HOME目录。你能在例子中看到，useradd命令创建了新的HOME目录，并将/etc/skel目录中的文件复制了过来。
<pre>
说明： 运行本章中提到的用户账户管理命令，需要以root用户账户登录或通过sudo命令以root用户
      账户身份运行这些命令。
</pre>

要想在创建用户时改变默认值或默认行文，可以使用命令行参数。下表列出了这些参数：
<pre>
            表： useradd命令行参数

   参  数                        描   述
--------------------------------------------------------------------------------
-c comment           给新用户添加备注
-d home_dir          为主目录指定一个名字（如果不想用登录名作为主目录名的话）
-e expire_date       用YYYY-MM-DD格式指定一个账户过期的日期

-f inactive_days     指定这个账户密码过期后多少天这个账户被禁用；0表示密码一过期就立即禁用，
                     -1表示禁用这个功能

-g initial_group     指定用户登录组的GID或组名
-G group             指定用户除登录组之外所属的一个或多个附加组
-k                   必须和-m一起使用，将/etc/skel目录的内容复制到用户的HOME目录

-m                   创建用户的HOME目录
-M                   不创建用户的HOME目录（当默认设置里指定创建时，才用到）
-n                   创建一个同用户登录名同名的新组
-r                   创建系统账户
-p passwd            为用户账户指定默认密码
-s shell             指定默认的登录shell
-u uid               为账户指定一个唯一的UID
</pre>

你会发现，在创建新用户账户时使用命令行参数，可以更改系统指定的默认值。但如果发现你一直需要修改一个值时，最好修改系统的默认值。

你可以用```-D```参数后跟一个代表要修改的值的参数，来修改系统默认的新用户值。这些参数如下表所示：
<pre>
                表：  useradd更改默认值的参数

 参 数                          描 述
------------------------------------------------------------------------------------------------------
-b default_home          更改默认的创建用户HOME目录的位置
-e expire_date           更改默认的新账户的过期日期
-f inactive              更改默认的新用户从密码过期到账户被禁用的天数
-g group                 更改默认的组名称或GID
-s shell                 更改默认的登录shell
</pre>
更改默认值非常简单：
{% highlight string %}
# useradd -D -s /bin/tsch
# useradd -D
GROUP=100
HOME=/home
INACTIVE=-1
EXPIRE=
SHELL=/bin/tsch
SKEL=/etc/skel
CREATE_MAIL_SPOOL=yes
{% endhighlight %}
现在useradd命令会将tsch作为所有新建用户的默认登录shell。注意，这里我们只是展示如何更改系统默认值，不要轻易的更改。

### 1.4 删除用户

如果你想从系统中删除用户，userdel可以满足这个需求。默认情况下，userdel命令只会删除/etc/passwd文件中的用户信息，而不会删除系统中属于该账户的任何文件。

如果加上```-r```参数，userdel会删除用户的HOME目录以及mail目录。然而，系统上仍可能存有归已删除用户所有的其他文件。这在有些环境中会造成问题。

下面是用userdel命令删除已有用户账户的例子：
{% highlight string %}
# /usr/sbin/userdel -r test
# ls -al /home/test
ls: cannot access /home/test: No such file or directory
{% endhighlight %}
加了```-r```参数之后，用户之前的那个/home/test目已经不存在了。
<pre>
警告： 在有大量用户的环境中使用-r参数时要特别小心。你永远不知道用户是否在他的HOME目录下存放了其他用户或其他程序
      要使用的重要文件。记住在删除用户的HOME目录之前一定要检查清楚。
</pre>

### 1.5 修改用户
Linux提供了一些不同的工具来修改已有用户账户的信息。下表列出了这些工具：
<pre>
              表： 用户账户修改工具

命 令                       描 述
----------------------------------------------------------------------------------------
usermod           修改用户账户的字段，并可以指定主要组以及附加组的所属关系
passwd            修改已有的用户密码
chpasswd          从文件中读取登录名密码对，并更新密码
chage             修改密码的过期日期
chfn              修改用户账户的备注信息
chsh              修改用户账户的默认登录shell
</pre>
这些工具中的每个都会提供修改用户账户信息的一个专门的功能。下面的几节将详细介绍每一个工具：

1） **usermod**



<br />
<br />

**[参看]**






<br />
<br />
<br />


