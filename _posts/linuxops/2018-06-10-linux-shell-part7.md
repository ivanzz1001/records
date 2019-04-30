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

usermod命令是用户账户修改工具中最强大的一个。它能用来修改/etc/passwd文件中的大部分字段，只需用与修改的字段对应的命令行参数就可以了。参数大部分跟useradd命令的参数一样，比如```-c```用来修改备注字段， ```-e```用来修改过期日期，```-g```用来修改默认的登录组。但还有一些实用的额外参数：

* -l 用来修改用户账户的登录名

* -L 用来锁定账户，这样用户就无法登录了；

* -p 用来修改账户的密码

* -U 用来解除锁定，解除后用户就能登录了

```-L```参数尤其实用。用这个参数就把账户锁定，用户就无法登录了，而不用删除账户和用户的数据。要让账户恢复正常，只要加```-U```参数就行了。

2） **passwd和chpasswd**

改变用户密码的一个简便方法就是用passwd命令：
<pre>
# passwd test
Changing password for user test.
New password: 
Retype new password: 
passwd: all authentication tokens updated successfully.
</pre>

如果只用passwd命令，它会改变你自己的密码。系统上的任何用户都能改变他自己的密码，但只有root用户才有权限改变别人的密码。

```-e```选项能强制用户下次登录时修改密码。你可以先给用户设置一个简单的密码，之后再强制他们在下次登录时改成他们能记住的更复杂的密码。

如果需要为系统中的大量用户来修改密码，chpasswd命令能让事情简单许多。chpasswd命令能从标准输入自动读取登录名和密码对（由冒号分割）列表，给密码加密，然后为用户账户设置。你也可以用重定向命令来将含有```userid:passwd```对的文件重定向给命令：
{% highlight string %}
# chpasswd < users.txt

# echo "test:testAa@123" | chpasswd

# chpasswd << EOF
> test:testAaBb@123
> EOF
{% endhighlight %}


3) **chsh、chfn和chage**

chsh、chfn和chage工具专门用来修改特定的账户信息。chsh命令用来快速修改默认的用户登录shell。使用时必须用shell的全路径名作为参数，不能只用shell名：
{% highlight string %}
# chsh -s /bin/csh test
Changing shell for test.
Shell changed.
{% endhighlight %}

chfn命令提供了在/etc/passwd文件的备注字段中存储信息的标准方法。chfn命令会将unix的finger命令用到的信息存进备注字段，而不是简单的存入一些随机文本（比如 昵称 之类的），或是将备注字段留空。finger命令可以用来简单地查看Linux系统上的用户信息：
{% highlight string %}
# finger rich
Login: rich                       Name: Rich Blum
Directory: /home/rich             Shell: /bin/bash
On since Thu Sep 20 18:03 (EDT) on pts/0 from 192.168.1.2
No mail.
No Plan.
{% endhighlight %}
<pre>
说明： 出于安全性考虑，很多Linux系统管理员会在系统上禁用finger命令
</pre>

如在使用chfn命令时不加参数，它会向你询问要存进备注字段的恰当值：
{% highlight string %}
# chfn test
Changing finger information for test.
Name []: Ima Test
Office []: Director of Technology
Office Phone []: (123)555-1234
Home Phone []: (123)555-9876

Finger information changed.

# finger test
Login: test                       Name: Ima Test
Directory: /home/test             Shell: /bin/csh
Office: Director of Technology    Office Phone: (123)555-1234
Home Phone: (123)555-9876
Never logged in.
No mail.
No Plan.
{% endhighlight %}
查看/etc/passwd文件中的记录，你会看到下面这样的结果：
{% highlight string %}
# cat /etc/passwd | grep test
test:x:1001:1002:Ima Test,Director of Technology,(123)555-1234,(123)555-9876:/home/test:/bin/csh
{% endhighlight %}

所有的人物信息现在都存在了/etc/passwd文件中了。

最后，chage命令用来帮助管理用户账户的有效期。它有一些参数可以用来设置每个值，如下表所示：
<pre>
               表： chage命令参数

参 数                   描     述
---------------------------------------------------------------------------------------
 -d               设置上次修改密码到现在的天数
 -E               设置密码过期的日期
 -I               设置密码过期到锁定账户的天数
 -m               设置修改密码之间最少要多少天
 -W               设置密码过期前多久开始出现提醒信息
</pre>
chage命令的日期值可以用下面两种方式中的任意一种：

* YYYY-MM-DD格式的日期

* 代表从1970年1月1日起到该日期天数的数值

chage命令中有个好用的功能是设置账户的过期日期。通过它，你就能创建临时用户了。设定的日期一过，临时账户就会自动过期，而不需要记住在那天去删除这些账户。过期的账户跟锁定的账户很相似： 账户仍然存在，当用户无法用它登录。

## 2. 使用Linux组
用户账户在控制单个用户安全性方面很管用，但他们在允许一组用户共享资源时就捉襟见肘了。为了达到这个目的，Linux系统用了另外一个概念————组（group)。


组权限允许多个用户共享一组共用的权限来访问系统上的对象，比如文件、目录或设备之类的。

Linux发行版在处理默认组的归属关系时略有差异。有些Linux发行版会创建一个组，把所有用户都当做这个组的成员；遇到这种情况要特别小心，因为文件很可能对其他用户也是可读的。有些发行版会为每个用户创建一个单独的组，这样可以更安全一些。

每个组都有一个唯一的GID————跟UID类似，在系统上这是个唯一的数值。和GID一起的，每个组还有一个唯一的组名。Linux系统上有一些组工具可以用来创建和管理你自己的组。本节将详述组信息是如何保存的，以及如何用组工具来创建新组、修改已有的组。

1） **/etc/group文件**

类似于用户账户，组信息也保存在系统的一个文件中。/etc/group文件包含系统上用到的每个组的信息。下面是一些来自Linux系统上/etc/group文件中的典型例子：
{% highlight string %}
# cat /etc/group
root:x:0:
daemon:x:1:
bin:x:2:
sys:x:3:
adm:x:4:syslog,ivan1001
tty:x:5:
disk:x:6:
lp:x:7:
...
{% endhighlight %}
类似于UID，GID也是用特有的格式分配的。系统账户用的组通常会分配低于500的GID值，而用户组的GID则会从500开始分配。/etc/group文件有4个字段：

* 组名

* 组密码

* GID

* 属于该组的用户列表

组密码允许非组内成员通过它临时性地成为该组成员。这个功能并不是十分通用，但确实存在。

千万不能直接修改/etc/group文件来添加用户到一个组，而要通过usermod命令来添加。在添加用户到不同的组之前，首先得创建组。

<pre>
说明： 用户账户列表某种意义上有些误导人。你会发现在列表中，有些组并没有列出用户。这并不是说，这些组
      没有成员。当一个用户在/etc/passwd文件中指定某个组作为默认组时，用户账户不会该组成员再出现在
      /etc/group文件中。多年来被这个问题难倒过的系统管理员可不是一个两个。
</pre>

2） **创建新组**

groupadd命令用来在系统上创建新组：
{% highlight string %}
# /usr/sbin/groupadd shared
# tail /etc/group
scanner:x:122:saned
colord:x:123:
pulse:x:124:
pulse-access:x:125:
rtkit:x:126:
saned:x:127:
ivan1001:x:1000:
sambashare:x:128:ivan1001
test:x:1001:
shared:x:1002:
{% endhighlight %}

在创建新组时，默认没有用户属于该组成员。groupadd命令没有提供将用户添加到组的选项，但可以用usermod命令来添加用户到该组：
{% highlight string %}
# /usr/sbin/usermod -G shared test
# tail -f /etc/group
scanner:x:122:saned
colord:x:123:
pulse:x:124:
pulse-access:x:125:
rtkit:x:126:
saned:x:127:
ivan1001:x:1000:
sambashare:x:128:ivan1001
test:x:1001:
shared:x:1002:test
{% endhighlight %}
shared组现在有一个成员test。usermod命令的```-G```参数会把这个新组添加到该用户账户的组列表里。

<pre>
说明： 如果更改了已登录系统账户所属的用户组，该用户必须登出系统后在登录，组关系的更改才能生效。

警告： 在将组添加到用户账户时要格外小心。如果加了-g参数，指定的组名会替换掉该账户的默认组。-G参数
      则将该组添加到用户的属主的列表里，而不会影响默认组
</pre>

3) **修改组**

我们在/etc/group文件中看到，组信息并不多，也没什么可修改的。groupmod命令可以修改已有组的GID（加```-g```参数）或组名（加```-n```参数）：
{% highlight string %}
# /usr/sbin/groupmod -n sharing shared
# tail /etc/group
scanner:x:122:saned
colord:x:123:
pulse:x:124:
pulse-access:x:125:
rtkit:x:126:
saned:x:127:
ivan1001:x:1000:
sambashare:x:128:ivan1001
test:x:1001:
sharing:x:1002:test
{% endhighlight %}
修改组名时，GID和组成员不会变，只有组名会改变。由于所有的安全权限都是基于GID的，你可以随意改变组名而不会影响文件的安全性。

## 3. 理解文件权限

现在你已经了解了用户和组，可以进一步了解ls命令输出的神秘文件权限了。本节将会介绍如何对权限码进行解码以及它们的来历。

### 3.1 使用文件权限符
Linux中ls命令可以用来查看系统上的文件、目录和设备的权限：
<pre>
# ls -l
total 20
-rw-rw-r-- 1 ivan1001 ivan1001    0 Apr 29 07:52 file1
-rw-rw-r-- 1 ivan1001 ivan1001    0 Apr 29 07:52 file2
-rw-rw-r-- 1 ivan1001 ivan1001    0 Apr 29 07:52 file3
-rwxrwxr-x 1 ivan1001 ivan1001 7348 Apr 29 07:53 myproc
-rw-rw-r-- 1 ivan1001 ivan1001  126 Apr 29 07:53 myproc.c
drwxrwxr-x 2 ivan1001 ivan1001 4096 Apr 29 07:54 test1
drwxrwxr-x 2 ivan1001 ivan1001 4096 Apr 29 07:54 test2
</pre>
输出结果的第一个字段是描述文件和目录权限的码。这个字段的第一个字符代表了对象的类型：

* - 代表文件

* d 代表目录

* l 代表链接

* c 代表字符型设备

* b 代表块设备

* n 代表网络设备

如果没有某种权限，在该权限位出现单破折线。则3组三字码分别对应对象的3个安全级别：

* 对象的属主

* 对象的属组

* 系统其它用户

下面以myproc文件为例：
<pre>
# ls -l myproc
-rwxrwxr-x 1 ivan1001 ivan1001 7348 Apr 29 07:53 myproc
</pre>
文件myproc有下面3组权限：

* rgw: 文件的属主（设为登录名ivan1001)

* rgw: 文件的属组（设为组名ivan1001)

* r-x: 系统上其它人

这些权限说明登录名为ivan1001的用户可以读取、写入以及执行这个文件（可以看做有全部权限）。类似地，属组ivan1001的成员也可以读取、写入和执行这个文件。然而不属于ivan1001组的其他用户只能读取和执行这个文件： w被单破折线取代了，说明这个安全级别没有写入权限。

### 3.2 默认文件权限

你可能会问这些文件权限从何而来，答案是umask。umask命令用来设置用户创建文件和目录的默认权限：
{% highlight string %}
$ touch newfile
$ ls -al newfile
-rw-rw-r-- 1 ivan1001 ivan1001 0 Apr 29 08:18 newfile
{% endhighlight %}
touch命令用分配给我的账户的默认权限创建了这个文件。umask命令可以显示和设置这个默认权限：
<pre>
$ umask
0002
</pre>
遗憾的是，umask命令的设置不是那么简单明了；想明白它如何工作，更是让人一头雾水。第一位代表了一项特别的安全特性，叫做```粘着位```（sticky bit)。关于这部分内容，我们在第5节会再讲。

后面的3位表示文件或目录的umask的八进制值。要理解umask是怎么工作的，先得理解八进制模式的安全性设置。

八进制模式的安全性设置先获取这3组```rwx```权限的值，然后将其转换成3位二进制值来表示一个八进制值。在这个二进制表示中，每个位置代表一个二进制位。因此，如果读权限是唯一置位的权限，权限值就是```r--```，转换成二进制值就是```100```，代表的八进制值是4。下表列出了可能会遇到的组合：
<pre>
                表： Linux文件权限码

 权 限       二进制值        八进制值                描述
----------------------------------------------------------------------
 ---          000              0                没有任何权限
 --x          001              1                只有执行权限
 -w-          010              2                只有写入权限
 -wx          011              3                有写入和执行权限
 r--          100              4                只有读取权限
 r-x          101              5                有读取和执行权限
 rw-          110              6                有读取和写入权限
 rwx          111              7                有全部权限             
</pre>
八进制模式先取得权限的八进制值，然后再把这3组安全级别（属主、属组和其他用户）的八进制值顺序列出。因此，八进制模式的值664代表属主和属组成员都有读取和写入权限，而其他用户都只有读取权限。

现在你了解了八进制模式权限是怎么工作的，umask值反而更叫人困惑了。我的Linux系统上默认的八进制的umask值是```0002```，而我所创建的文件的八进制权限却是```664```，这是如何得来的呢？

umask值只是个掩码，它会屏蔽掉不想授予该安全级别的权限。接下来我们稍微进行一点八进制运算来把这个原因讲完。

umask值会从对象的全权限值中减掉，对文件来说，全权限值是```666```(所有用户都有读和写的权限）； 而对目录来说是```777```(所有用户都有读、写、执行的权限）。所以，在上面的例子中，文件一开始的权限是```666```，然后umask值```002```作用后，剩下的文件权限就成了```664```。

umask值通常会在/etc/profile启动文件中设置。你可以用umask命令为默认umask设置指定一个新值：
{% highlight string %}
$ umask 026 
$ touch newfile2
$ ls -l newfile2
-rw-r----- 1 ivan1001 ivan1001 0 Apr 29 08:46 newfile2
{% endhighlight %}
在把umask值设成```026```之后，默认的文件权限变成了```640```，因此新文件现在对属组成员来说是只读的，而系统里的其他成员则没有任何权限。

umask值同样会作用在创建目录上：
{% highlight string %}
$ ls -l 
drwxr-x--x 2 ivan1001 ivan1001 4096 Apr 29 08:50 newdir
{% endhighlight %}
由于目录的默认权限是```777```，umask作用后生成的目录权限不同于生成的文件权限。umask值```026```会从```777```中减去，留下来```751```作为目录权限设置。

## 4. 改变安全性设置
如果你已经创建了一个目录或文件而需要改变它的安全性设置，在Linux系统上有一些不同的工具能完成这个功能。本节将告诉你如何更改文件和目录的已有权限、默认文件属主以及默认属组。


### 4.1 改变权限

chmod命令用来改变文件和目录的安全性设置。chmod命令的格式如下：
{% highlight string %}
chmod options mode file
{% endhighlight %}
mode参数后可跟个八进制模式或符号模式来设置安全性设置。八进制模式设置非常直接，直接用期望赋予文件的标准3位八进制权限码：
<pre>
# chmod 760 newfile
# ls -l newfile
-rwxrw----. 1 root root 0 Apr 30 10:08 newfile
</pre>

八进制文件权限会自动应用到指定的文件上。符号模式的权限就没这么简单了。

与通常用到的3组三字符权限字符不同，chmod用了另外一种实现。下面是在符号模式下指定权限的格式：
{% highlight string %}
[ugoa...] [+-=][rwxXstugo...]
{% endhighlight %}
非常有意义，不是吗？ 第一组字符定义了权限作用的对象：

* u代表用户

* g代表组

* o代表其他

* a代表上述所有

下一步，后面跟着的符号表示你是想在现有权限的基础上增加权限（+)，还是在现有权限的基础上移除权限(-)，还是将权限设置成后面的值(=)。

最后，第3个符号代表作用到设置上的权限。你会发现，这个值要比```rwx```多。额外的设置有以下几项：

* X: 如果对象是目录或者它已有执行权限，赋予执行权限

* s: 运行时重新设置UID和GID

* t: 保留文件或目录

* u: 将权限设置为跟属主一样

* g: 将权限设置为跟属组一样

* o: 将权限设置为跟其他用户一样

这么使用这些权限：
<pre>
# ls -l newfile
-rwxrw----. 1 root root 0 Apr 30 10:08 newfile
# chmod o+r newfile
# ls -l newfile
-rwxrw-r--. 1 root root 0 Apr 30 10:08 newfile
</pre>

不管其他用户在这一安全级别之前都有什么权限，```o+r```给这一级别添加了读取权限。
<pre>
# chmod u-x newfile
# ls -l newfile
-rw-rw-r--. 1 root root 0 Apr 30 10:08 newfile
</pre>

```u-x```移除了属主已有的执行权限。回顾一下之前讲过的ls命令的那个参数```-F```，能够在具有执行权限的文件名后加一个星号。可用在这里检查前后的变化：
<pre>
# chmod u-x newfile
# ls -lF ./newfile
-rw-rw-r--. 1 root root 0 Apr 30 10:08 ./newfile
# chmod u+x newfile
# ls -lF ./newfile
-rwxrw-r--. 1 root root 0 Apr 30 10:08 ./newfile*
</pre>

options参数为chmod命令提供了另外一些功能。```-R```参数可以让权限的改变递归地作用到文件和子目录。可以在指定文件名时用通配符将权限的更改通过一个命令作用到多个文件上。

### 4.2 改变所属关系
有时你要改变文件的属主，比如有人离职或开发人员创建了一个在产品环境中运行时需要归属在系统账户下的应用。Linux提供了两个命令来完成这个功能： chown命令用来改变文件的属主，chgrp命令用来改变文件的默认属组。

chown命令的格式如下：
{% highlight string %}
chown [OPTION]... [OWNER][:[GROUP]] FILE...
{% endhighlight %}
可以用登录名或UID来指定文件的新属主：
<pre>
# chown ivan1001 newfile
# ls -l newfile
-rw-r--r--. 1 ivan1001 root 0 Apr 30 10:51 newfile
</pre>

非常简单。chown命令也支持同时改变文件的属主和属组：
{% highlight string %}
# chown ivan1001:ivan1001 newfile
# ls -l newfile
-rw-r--r--. 1 ivan1001 ivan1001 0 Apr 30 10:51 newfile
{% endhighlight %}
如果你不嫌麻烦，那么你可以这么改变一个目录的默认属组：
{% highlight string %}
# chown :ivan1001 ./newfile
# ls -l newfile
-rw-r--r--. 1 root ivan1001 0 Apr 30 10:54 newfile
{% endhighlight %}
最后，如果你的Linux系统采用和登录名匹配的组名，你可以只用一个条目就改变二者：
{% highlight string %}
# chown ivan1001: newfile
# ls -l newfile
-rw-r--r--. 1 ivan1001 ivan1001 0 Apr 30 11:00 newfile
{% endhighlight %}

chown命令采用一些不同的选项参数。```-R```参数加通配符可以递归地改变子目录和文件的所属关系。```-h```参数可以改变该文件的所有符号链接文件的所属关系。

<pre>
说明： 只有root用户能够改变文件的属主。任何属主都可以改变文件的	属组，但前提是属主必须是源和目标属组的成员。
</pre>

chgrp命令可以很方便的更改目录或文件的默认属组：
{% highlight string %}
# chgrp ivan1001 newfile
# chmod g+w newfile
# ls -l newfile
-rw-rw-r--. 1 root ivan1001 0 Apr 30 11:08 newfile
{% endhighlight %}

现在ivan1001组的任意一个成员都可以写这个文件了。这是Linux系统共享文件的一个途径。然而，在系统上给一组人共享一个文件很复杂。下一节会介绍如何做。

## 5. 共享文件
可能你已经猜到了，创建组是Linux系统上共享文件访问权限的方法。但在一个完整的共享文件的环境中，事情会复杂得多。

在第3节中你已经看到，创建新文件时，Linux会用默认UID和GID来给文件分配权限。想让其他人也能访问文件，你要么改变其他用户所在安全组的访问权限，要么就给文件分配一个新的包含其他用户的默认属组。

如果你想在大的环境中创建文档并将文档与人共享，这会很繁琐。幸好有解决这个问题的简单方法。

Linux还为每个文件和目录存储了3个额外的信息位：

* 设置用户ID(SUID): 当文件被用户使用时，程序会以文件属主的权限运行

* 设置组ID(SGID): 对文件来说，程序会以文件属组的权限运行； 对目录来说，目录中创建的新文件会以目录的默认属组作为默认属组。

* 粘着位: 进程结束后文件还会在内存中

SGID位对文件共享非常重要。使能了SGID位，你能让在一个共享目录下创建的新文件都属于该目录的属组，也就是每个用户的组。

SGID可通过chmod命令设置。它会加到标准3位八进制值之前（组成4位八进制值），或者在符号模式下用符号```s```。

如果你用的是八进制模式，你需要知道这些位的位置，如下表所示：

<pre>
                表： chmod SUID、SGID和粘着位的八进制值

二进制值               八进制值                    描述
-------------------------------------------------------------------------------
 000                     0                   所有位都清零
 001                     1                   粘着位置位
 010                     2                   SGID位置位
 011                     3                   SGID位和粘着位都置位
 100                     4                   SUID位置位
 101                     5                   SUID为和粘着位置位
 110                     6                   SUID位和SGID位置位
 111                     7                   所有位都置位       
</pre>
因此，要创建一个共享目录，使目录里的新文件都能沿用目录的数组，你只需将该目录的SGID位置位：
{% highlight string %}
# groupadd shared
# mkdir /tmp/testdir
# ls -l /tmp/
drwxr-xr-x. 2 root     root        6 Apr 30 11:31 testdir

# chgrp shared /tmp/testdir
# chmod g+s /tmp/testdir
# chmod g+w /tmp/testdir
# ls -l /tmp
drwxrwsr-x. 2 root     shared      6 Apr 30 11:31 testdir

# umask 002
# cd /tmp/testdir
# touch testfile
# ls -l
total 0
-rw-rw-r--. 1 root shared 0 Apr 30 11:38 testfile
{% endhighlight %}

首先，用mkdir命令来创建希望共享的目录。其次，通过chgrp命令将该目录的默认属组改为含有所有需要共享文件用户的组。最后，将目录的SGID位置位，以保证目录中新建文件都用shared作为默认属组。

为了让这个环境能正常工作，所有组成员都需把它们的umask值设置成文件对属组成员可写。在前面的例子中，umask改成了```002```，所以文件对属组是可写的。

做完了这些，组成员就能到共享目录下创建新文件了。跟期望的一样，新文件会沿用目录的属组，而不是用户的默认属组。现在shared组的所有用户都能访问这个文件了。










<br />
<br />

**[参看]**






<br />
<br />
<br />


