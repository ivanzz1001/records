---
layout: post
title: Linux crontab的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux crontab的使用
---


本节我们简要介绍一下Linux环境下crontab的使用。


<!-- more -->


## 1. 使用格式
<pre>
crontab [-u user] [-l | -r | -e] [-i] [-s]
</pre>

上面是最常用的一种使用格式。其中：

* ```crontab -l```: 查看现有定时任务
* ```crontab -e```: 编辑定时任务

<pre>
#MIN	HOUR	DAY		MONTH	DAYOFWEEK	COMMAND
*	     *		*		*		*			command     
</pre> 
* 第一列表示分钟0~59，每分钟用```*```或者```*/1```表示，```*/10```表示每10分钟
* 第二列表示小时0~23
* 第三列表示日期1~31
* 第四列表示月份1~12
* 第五列表示星期0~7,其中0或7代表星期天
* 第六列表示要运行的命令

<pre>
* : (星号)表示所有的值，如 * 16 5 6 *, 表示6月5日16点到17点之间的每分钟执行一次
, : (逗号)表示可选的值，如 3,4,5 16 5 6表示6月5日16点3分、4分、5分中各执行一次
- : (中划线)表示范围，如5-10 16 5 6表示6月5日16点5分到10分，每分钟执行一次
/ : (正斜杠)表示频率，如*/10 * * * *表示每10分钟执行一次
</pre>


## 2. crontab的用户权限问题

Linux下可以通过创建文件/etc/cron.allow或者/etc/cron.deny来控制权限: 如果/etc/cron.allow文件存在，那么只有这个文件中列出的用户可以使用crontab，同时/etc/cron.deny被忽略； 如果/etc/cron.allow文件不存在，那么文件/etc/cron.deny中列出的用户将不能使用crontab.


## 3. crontab服务的启动与停止
<pre>
[root@localhost langdata]# service crond status
Redirecting to /bin/systemctl status  crond.service
● crond.service - Command Scheduler
   Loaded: loaded (/usr/lib/systemd/system/crond.service; enabled; vendor preset: enabled)
   Active: active (running) since Wed 2017-10-11 19:06:05 PDT; 4 days ago
 Main PID: 861 (crond)
   CGroup: /system.slice/crond.service
           └─861 /usr/sbin/crond -n

Oct 11 19:06:05 localhost.localdomain systemd[1]: Started Command Scheduler.
Oct 11 19:06:05 localhost.localdomain systemd[1]: Starting Command Scheduler...
Oct 11 19:06:05 localhost.localdomain crond[861]: (CRON) INFO (RANDOM_DELAY will be scaled with factor 84% if used.)
Oct 11 19:06:06 localhost.localdomain crond[861]: (CRON) INFO (running with inotify support)



[root@localhost langdata]# service crond stop
Redirecting to /bin/systemctl stop  crond.service


[root@localhost langdata]# service crond start
Redirecting to /bin/systemctl start  crond.service


[root@localhost langdata]# service crond restart
Redirecting to /bin/systemctl restart  crond.service


# 重新载入配置
[root@localhost langdata]# service crond reload
Redirecting to /bin/systemctl reload  crond.service
</pre>

## 4. 示例

**(1) 每分钟向/root/test.txt中打印数据**
<pre>
[root@localhost langdata]# cat /etc/cron.deny 
[root@localhost langdata]# cat /etc/crontab 
SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root

# For details see man 4 crontabs

# Example of job definition:
# .---------------- minute (0 - 59)
# |  .------------- hour (0 - 23)
# |  |  .---------- day of month (1 - 31)
# |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
# |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
# |  |  |  |  |
# *  *  *  *  * user-name  command to be executed

*  *  *  *  *  root  echo "hello,world" >> /root/test.txt 

[root@localhost langdata]# service crond reload
Redirecting to /bin/systemctl reload  crond.service

[root@localhost langdata]# cat /root/test.txt
hello,world
hello,world
hello,world
hello,world
hello,world
</pre>

**(2) 每2小时执行一次脚本删除过期文件**
<pre>
[root@localhost script]# cat clear.sh
#!/bin/sh


#echo "remove checkpoint files" >> /root/tesseract-src/script/rm_result.txt
count=`find /root/tesseract-src/tesstutorial/chi_simoutput/ -amin +2160 -name "*.checkpoint" | wc -l`
total=`find /root/tesseract-src/tesstutorial/chi_simoutput/ -amin +0 -name "*.checkpoint" | wc -l`
left=$(($total - $count))


if [ $count -gt 8 ] && [ $left -gt 12 ]
then
   echo "remove $count checkpoint files" >> /root/tesseract-src/script/rm_result.txt
   find /root/tesseract-src/tesstutorial/chi_simoutput/ -amin +2160 -name "*.checkpoint" -exec rm {} \;
fi
sed -i '1,10000d' /root/tesseract-src/tesstutorial/chi_simoutput/basetrain.log

#find /root/tesseract-src/tesstutorial/chi_simoutput/ -amin +1200 -name "*.checkpoint" -exec rm {} \;
#sed -i '1,10000d' /root/tesseract-src/tesstutorial/chi_simoutput/basetrain.log
[root@localhost script]# chmod +x clear.sh


[root@localhost script]# cat /etc/crontab 
SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root

# For details see man 4 crontabs

# Example of job definition:
# .---------------- minute (0 - 59)
# |  .------------- hour (0 - 23)
# |  |  .---------- day of month (1 - 31)
# |  |  |  .------- month (1 - 12) OR jan,feb,mar,apr ...
# |  |  |  |  .---- day of week (0 - 6) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
# |  |  |  |  |
# *  *  *  *  * user-name  command to be executed

*  */2  *  *  *  root  /root/tesseract-src/script/clear.sh

[root@localhost langdata]# service crond restart
Redirecting to /bin/systemctl restart  crond.service

[root@localhost langdata]# service crond status
</pre>





<br />
<br />







<br />
<br />
<br />


