---
layout: post
title: linux logrotate命令
tags:
- LinuxOps
categories: linuxOps
description: linux logrotate命令
---


logrotate主要是用来方便地管理系统所产生的大量的日志文件。其可以实现自动回滚(automatic rotation)、压缩、移除、和邮寄(mail)日志文件。每一个日志文件都可以按天(daily)、周(weekly)、月(monthly)、甚至是按文件大小来进行处理。

通常情况下，logrotate是作为一个daily cron job来运行的。除非我们设置的规则是按日志大小来进行管理的，否则一般其一天只会对日志修改一次。

我们可以在命令行上指定任意数量的config文件，但通常后面的config文件中的选项会覆盖前面配置文件中对应的选项。

当前我们的系统环境为：
<pre>
# uname -a
Linux VM-0-4-centos 3.10.0-1127.19.1.el7.x86_64 #1 SMP Tue Aug 25 17:23:54 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux

# cat /etc/redhat-release 
CentOS Linux release 7.5.1804 (Core) 
</pre>

<!-- more -->

## 1. logrotate的安装
事实上大多数Linux发行版本都默认安装了```logratate```，可以通过以下命令来判断系统是否已经预装了logrotate:

* rpm包管理器
<pre>
# rpm -qa | grep logrotate
logrotate-3.8.6-19.el7.x86_64
</pre>

* dpkg包管理器
<pre>
# dpkg --list | grep logrotate
</pre>

* yum命令查看
<pre>
# yum list installed | grep logrotate
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
logrotate.x86_64                        3.8.6-19.el7                   @os  
</pre>

下面我们列出yum针对软件包操作常用命令：
{% highlight string %}
// 1) 使用yum查找软件包
# yum search <package-name>

//2) 列出所有可安装的软件包
# yum list 

//3) 列出所有可更新的软件包
# yum list updates

//4) 列出所有已安装的软件包
# yum list installed 

//5) 列出所有已安装但不再yum repository内的软件包
# yum list extras

//6) 获取软件包信息
# yum info <package-name>

//7) 列出所有可更新的软件包信息
# yum info updates

//8) 列出所有已安装的软件包信息
# yum info installed

//9) 列出所有已安装但不再yum repository内的软件包信息
# yum info extras

//10) 列出软件包提供哪些文件
# yum provodes <package-name>

//11) 卸载软件包
# yum remove <package-name>

//12) 列出软件包依赖
# yum deplist <package-name>
{% endhighlight %}

* 更简单粗暴的办法
<pre>
# which logrotate
</pre>

如果我们没有安装，可以执行如下命令进行安装：
<pre>
# yum install logrotate
</pre>
安装完成后我们看到在etc下会有如下两个文件:
<pre>
#  ls -a /etc/logrotate*
/etc/logrotate.conf

/etc/logrotate.d:
.  ..  2019-01-21  bootlog  chrony  ppp  syslog  wpa_supplicant  yum
</pre>



## 2. logrotate的使用
logrotate命令的基本用法如下所示：
<pre>
# logrotate [-dv] [-f|--force] [-s|--state file] config_file ..
</pre>

常用选项如下：
{% highlight string %}
-?, --help
	打印帮助信息

-d, --debug
	开始调试模式（暗含着会自动开启-v选项)。在调试模式下，并不会实际修改日志文件，也不会对logrotate的状态文件进行修改

-f, --force
	告诉logrotate强制执行rotate，即使在其认为不必要的情况下。

-m, --mail <command>
	告诉logrotate当需要mail日志文件时使用哪一个命令。该命令可以接受两个参数：1）消息的主题(subject) 2) 接收者

-s, --state <statefile>
	告诉logrotate使用一个额外的state file。默认的state file是/var/lib/logrotate/logrotate.status

--usage
	打印一个简短的使用消息

-v,--verbose
	开启verbose模式，比如在rotating时打印相关消息
{% endhighlight %}


## 3. logrotate基本工作原理
作为一个定时处理的管理工具，logrotate本身是个命令行的工具，然而它需要定时处理你的日志，所以显然logrotate还是得基于cron来工作地，否则就没有人来定时唤醒它让它干活了。正常情况下logrotate已经作为cron.daily的一个任务每天被定时在执行了，这个定时脚本位于/etc/cron.daily/logrotate:
{% highlight string %}
#!/bin/sh

/usr/sbin/logrotate -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf
EXITVALUE=$?
if [ $EXITVALUE != 0 ]; then
    /usr/bin/logger -t logrotate "ALERT exited abnormally with [$EXITVALUE]"
fi
exit 0
{% endhighlight %}
上面这段是centos7.5默认的logrotate脚本，各个不同版本上的logrotate脚本会有细微的不同，但是核心都是使用/etc/logrotate.conf配置脚本来启动logrotate。

然而，这段脚本只是位于这个目录里，为什么每天会调用呢？而且每天什么点被调用呢？可以查看cron的默认配置文件/etc/crontab(新版Centos中其默认配置文件位于/etc/anacrontab)的内容：
<pre>
# /etc/anacrontab: configuration file for anacron

# See anacron(8) and anacrontab(5) for details.

SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root
# the maximal random delay added to the base delay of the jobs
RANDOM_DELAY=45
# the jobs will be started during the following hours only
START_HOURS_RANGE=3-22

#period in days   delay in minutes   job-identifier   command
1       5       cron.daily              nice run-parts /etc/cron.daily
7       25      cron.weekly             nice run-parts /etc/cron.weekly
@monthly 45     cron.monthly            nice run-parts /etc/cron.monthly
</pre>
下面来解释一下这个文件：

* SHELL: 运行此计划任务的解释器

* PATH: 执行命令的环境变量

* MAILTO: 计划任务执行时发邮件到指定的用户

* RANDOM_DELAY: 任务执行的随机最大延迟（单位： 分钟）

* START_HOURS_RANGE: 任务执行的时间范围

* run-parts： 其是一个脚本，在/usr/bin/run-parts，作用是执行一个目录下的所有脚本/程序。

下面我们来看如下这条语句：
<pre>
#period in days   delay in minutes   job-identifier   command
1                 5                  cron.daily       nice run-parts /etc/cron.daily
</pre>
含义为：每```1```天都执行/etc/cron.daily目录下的脚本文件，真实的延迟为RANDOM_DELAY+delay。这里的delay是5分钟，加上RANDOM_DELAY的最大值为45，所以实际的延迟时间是5~50分钟之间。开始时间为3~22点。如果机器没有关机，那么一般就是在3:05~3:50之间执行。nice命令将该进程设置为nice=10，默认为0，即低优先级进程。

## 4. logrotate配置文件
我们来看一下默认的配置文件： /etc/logrotate.conf
{% highlight string %}
# see "man logrotate" for details
# rotate log files weekly
weekly

# keep 4 weeks worth of backlogs
rotate 4

# create new (empty) log files after rotating old ones
create

# use date as a suffix of the rotated file
dateext

# uncomment this if you want your log files compressed
#compress

# RPM packages drop log rotation information into this directory
include /etc/logrotate.d

# no packages own wtmp and btmp -- we'll rotate them here
/var/log/wtmp {
    monthly
    create 0664 root utmp
        minsize 1M
    rotate 1
}

/var/log/btmp {
    missingok
    monthly
    create 0600 root utmp
    rotate 1
}

# system-specific logs may be also be configured here.
{% endhighlight %} 
其实，默认配置文件中给出的是logrotate的一些默认配置选项，并给出了一些样例的配置（如wtmp,btmp)，这些配置的意义我们会在下面进行阐述。这个配置文件中还通过```include```包含了/etc/logrotate.d目录下的配置，这使得我们可以不用将所有的配置扎堆写入这个默认配置文件中，从而可以分散的管理在/etc/logrotate.d目录下的文件，事实上/etc/logrotate.d也已经包含了一些样例的配置文件供参考。

### 4.1 logrotate配置文件详解
如上面所说，logrotate会每天通过cron启动并执行，我们只需要在/etc/logrotate.d目录中添加我们需要的配置，就可以利用logrotate来自动管理我们需要的日志文件。

这里我们先来看一下目录下已经存在的样例配置(dpkg):
{% highlight string %}
/var/log/dpkg.log {
    monthly
    rotate 12
    compress
    delaycompress
    missingok
    notifempty
    create 644 root root
}

/var/log/alternatives.log {
    monthly
    rotate 12
    compress
    delaycompress
    missingok
    notifempty
    create 644 root root
}
{% endhighlight %}
正如其名称dpkg，这个配置是用来处理包管理器dpkg的默认日志的，处理的日志文件是/var/log/dpkg.log与/var/log/alternatives.log这两个文件。下面我们对配置文件中的一下选项做出解释：

* monthly: 按月处理日志

* rotate 12: 保留12份日志

* compress: 日志分割后会进行gzip压缩（可使用gunzip来解压)

* delaycompress: 日志压缩会给延迟到下次分割时进行

* missingok: 目标日志文件不存在程序也不会报错退出

* notifempty： 目标日志为空时不进行分割操作

* creat 644 root root: 以644也就是rw-r–r–权限来建立分割出来的文件，同时该分割文件所属用户为root，用户组为root

下面再来看另一个配置文件样例：
{% highlight string %}
 # sample logrotate configuration file
compress

/var/log/messages {
   rotate 5
   weekly
   postrotate
       /usr/bin/killall -HUP syslogd
   endscript
}

"/var/log/httpd/access.log" /var/log/httpd/error.log {
   rotate 5
   mail www@my.org
   size 100k
   sharedscripts
   postrotate
       /usr/bin/killall -HUP httpd
   endscript
}

/var/log/news/* {
   monthly
   rotate 2
   olddir /var/log/news/old
   missingok
   postrotate
       kill -HUP `cat /var/run/inn.pid`
   endscript
   nocompress
}

~/log/*.log {}
{% endhighlight %}
下面我们对配置文件中的一下选项做出解释：

* sharedscripts：正常情况下，prerotate与postrotate脚本会针对每一个被切割日志都执行，并且会将日志的绝对路径作为第一个参数传递给脚本。这就意味着每一个脚本对匹配的日志条目可能会运行多次（比如： 匹配的日志条目为```/var/log/news/*```)。假如指定了```sharedscripts```，则这些脚本只会运行一次，而不管多少日志匹配了该通配模式。然而，假如并没有日志需要切割，其中的脚本将不会被执行。假如脚本执行发生错误，剩下的操作操作也将不会被执行。此指令暗含```create```选项。

* postrotate/endscript： 在日志被切割完成之后，会使用/bin/sh来执行postrotate与endscript之间的脚本。正常情况下日志文件的绝对路径会作为第一个参数传递给脚本。假如指定了sharedscripts，则整个```pattern```(如上文的```/var/log/news/*```)会被传递给script。

* prerotate/endscript： 在日志被切割（且实际会执行切割动作）之前，会使用/bin/sh来执行prerotate与endscript之间的脚本。正常情况下日志文件的绝对路径会作为第一个参数传递给脚本。假如指定了sharedscripts，则整个```pattern```(如上文的```/var/log/news/*```)会被传递给script。

* copy: 对日志文件进行拷贝，并不对原始的日志文件做任何改变。比如我们只想要对日志文件做一个snapshot时，我们可以使用此选项；当本选项被使用时，create选项将不会起作用，因为老的日志文件仍然stays in place。

* copytruncate：在拷贝完成之后，对原日志文件执行truncate操作，而不是将老的日志文件移动到另一个位置并创建一个新的日志文件。有一些程序可能并不能被告知关闭其logfile，而是会被不断的追加写入到原日志文件中，因此通过此选项可以很好的处理这种情况。当被选项被使用时，create选项将不会起作用，因为老的日志文件仍然stays in place

### 4.2 为nginx创建日志切割配置
仿照上文的样例，我们可以配置一个类似的处理任务，这里处理一下nginx的log文件。一般web server的access.log都是日志重灾区，我们可以建立一个/etc/logrotate.d/nginx配置文件，内容如下：
{% highlight string %}
/home/wwwlogs/*.log {
    daily
    rotate 7
    dateext
    compress
    delaycompress    
    missingok
    notifempty
    create 644 root root
    sharedscripts
    postrotate
        kill -USR1 `cat /usr/local/nginx/logs/nginx.pid`
    endscript
}
{% endhighlight %}

与之前的样例配置类似，注意到声明日志文件位置时使用了```*```通配符，这意味着/home/wwwlogs/下的所有```.log```文件都会被logrotate处理。

* daily：每天处理日志

* dateext：备份的日志文件的后缀不再是默认的```.1```、```.2```、```.3```的递增格式，而是```.YYYYMMDD```的日期格式


* sharedscripts：配置该选项时，prerotate和postrotate段的脚本会在所有文件被处理结束（被处理前）统一执行，而不是每个文件前后分别执行一次

* postrotate：处理日志后钩子，postrotate和endscript构成一个shell语句块，在日志文件被处理后这部分语句会被执行，对应的还有prerotate语句块

这样nginx的log文件的处理配置就完成了，但是我们可能还需要确认一下配置的内容到底是否正确呢，这时候可以利用logrotate命令行的debug选项来进行测试，命令如下：
<pre>
# logrotate -d -f /etc/logrotate.d/nginx
</pre>
开启了debug选项时，logrotate会详细地给出处理日志过程中的处理信息，但是并不会真正地去处理日志文件，所以可以用来测试配置文件处理的是否正确，这行命令的输出如下：
{% highlight string %}
reading config file /etc/logrotate.d/nginx

Handling 1 logs

rotating pattern: /home/wwwlogs/*.log  forced from command line (7 rotations)
empty log files are not rotated, old logs are removed
considering log /home/wwwlogs/access.log
  log needs rotating
considering log /home/wwwlogs/jp01.sanaecon.com.log
  log needs rotating
considering log /home/wwwlogs/nginx_error.log
  log needs rotating
rotating log /home/wwwlogs/access.log, log->rotateCount is 7
dateext suffix '-20151108'
glob pattern '-[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]'
glob finding logs to compress failed
glob finding old rotated logs failed
rotating log /home/wwwlogs/jp01.sanaecon.com.log, log->rotateCount is 7
dateext suffix '-20151108'
glob pattern '-[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]'
glob finding logs to compress failed
glob finding old rotated logs failed
rotating log /home/wwwlogs/nginx_error.log, log->rotateCount is 7
dateext suffix '-20151108'
glob pattern '-[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]'
glob finding logs to compress failed
glob finding old rotated logs failed
renaming /home/wwwlogs/access.log to /home/wwwlogs/access.log-20151108
creating new /home/wwwlogs/access.log mode = 0644 uid = 0 gid = 0
renaming /home/wwwlogs/jp01.sanaecon.com.log to /home/wwwlogs/jp01.sanaecon.com.log-20151108
creating new /home/wwwlogs/jp01.sanaecon.com.log mode = 0644 uid = 0 gid = 0
renaming /home/wwwlogs/nginx_error.log to /home/wwwlogs/nginx_error.log-20151108
creating new /home/wwwlogs/nginx_error.log mode = 0644 uid = 0 gid = 0
running postrotate script
running script with arg /home/wwwlogs/*.log : "
        kill -USR1 `cat /usr/local/nginx/logs/nginx.pid`
{% endhighlight %}
可以看到logrotate如我们设想的那样在正常处理日志文件，至此一个logrotate配置就完成了。

logrotate命令行除了可以用来展示配置文件配置是否正确以外，还可以用来手动执行日志分割，使用以下命令行：
<pre>
# logrotate -f /etc/logrotate.d/nginx
</pre>
这样就会脱离CRON手动运行一次日志分割和处理任务，事实上通过这个方式也可以用第三方的其他程序来管理日志分割的频率。


## 5. 配置的logrotate cron任务不执行问题
在我们的使用过程中，有时会出现配置的logrotate cron任务不能正常执行。一般我们可以通过如下步骤进行排查：
1） 查询crond运行状态
<pre>
# systemctl status crond                //或service crond status
● crond.service - Command Scheduler
   Loaded: loaded (/usr/lib/systemd/system/crond.service; enabled; vendor preset: enabled)
   Active: active (running) since Mon 2018-05-14 14:23:39 CST; 3 years 5 months ago
 Main PID: 1236 (crond)
   CGroup: /system.slice/crond.service
           └─1236 /usr/sbin/crond -n

May 12 18:08:01 ceph001-node1 crond[1236]: (root) RELOAD (/var/spool/cron/root)
Sep 17 22:41:01 ceph001-node1 crond[1236]: (root) RELOAD (/var/spool/cron/root)
Warning: Journal has been rotated since unit was started. Log output is incomplete or unavailable.
</pre>

2） 检查crond权限
<pre>
# cat /etc/cron.deny              //文件为空的
# ll /usr/bin/crontab             //具备S权限位，正常
-rwsr-xr-x 1 root root 57656 Aug  9  2019 /usr/bin/crontab
</pre>

3) 检查PAM模块
<pre>
# cat /etc/pam.d/crond 
#
# The PAM configuration file for the cron daemon
#
#
# No PAM authentication called, auth modules not needed
account    required   pam_access.so
account    include    system-auth
session    required   pam_loginuid.so
session    include    system-auth
auth       include    system-auth
</pre>

4) 查看系统日志

我们检查/var/log/secure（或/var/log/cron):
<pre>
# cat /var/log/secure
Jun 28 17:02:01 host-01 crond[3665224]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:03:01 host-01 crond[3678038]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:03:01 host-01 crond[3678039]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:04:01 host-01 crond[3690188]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:05:01 host-01 crond[3703362]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:06:02 host-01 crond[3716699]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:06:02 host-01 crond[3716700]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:07:01 host-01 crond[3730002]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:08:01 host-01 crond[3743625]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:09:02 host-01 crond[3757559]: pam_unix(crond:account): expired password for user root (password aged)
Jun 28 17:09:02 host-01 crond[3757558]: pam_unix(crond:account): expired password for user root (password aged)
</pre>

这通常是root密码过期了。我们可以通过如下命令查看root的过期信息：
<pre>
# chage -l root
Last password change                                    : Jun 16, 2021
Password expires                                        : never
Password inactive                                       : never
Account expires                                         : never
Minimum number of days between password change          : 0
Maximum number of days between password change          : 99999
Number of days of warning before password expires       : 7
</pre>

但有的时候通过上述命令检查出来的信息可能不准确，我们有时还必须检查```/etc/login.defs```文件，看其是否设置了缺省的过期时间：
<pre>
# cat /etc/login.defs | grep -v ^#

MAIL_DIR        /var/spool/mail

PASS_MAX_DAYS   99999
PASS_MIN_DAYS   0
PASS_MIN_LEN    8
PASS_WARN_AGE   7

UID_MIN                  1000
UID_MAX                 60000
SYS_UID_MIN               201
SYS_UID_MAX               999

GID_MIN                  1000
GID_MAX                 60000
SYS_GID_MIN               201
SYS_GID_MAX               999


CREATE_HOME     yes

UMASK           077

USERGROUPS_ENAB yes

ENCRYPT_METHOD MD5

MD5_CRYPT_ENAB yes
</pre>



<br />
<br />

**[参看]**

1. [manage-linux-log-files-with-logrotate](https://www.techrepublic.com/article/manage-linux-log-files-with-logrotate/)

2. [用logrotate管理每日增长的日志](https://blog.csdn.net/baidu_zhongce/article/details/50393090)

3. [关于root账号下cron任务不执行问题处理](https://www.lezhizhe.net/archives/432/)

4. [crontab不允许访问](https://blog.csdn.net/junjunjiao/article/details/50732247)

5. [Linux下crond切换到非root用户不执行的问题解决方法](https://www.it300.com/article-15346.html)

6. [Linux系统的PAM模块认证文件含义说明总结](https://www.cnblogs.com/fengdejiyixx/p/14804741.html)

7. [github logrotate](https://github.com/logrotate/logrotate)

8. [CentOS 中用 Yum 安装、卸载软件](https://blog.csdn.net/sunylat/article/details/81869513)

9. [logrotate man page](http://linuxconfig.org/logrotate-8-manual-page)

10. [Linux权限s权限和t权限](https://www.cnblogs.com/yiyide266/p/10047340.html)

<br />
<br />
<br />


