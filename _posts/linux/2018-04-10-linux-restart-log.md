---
layout: post
title: Linux死机重启后怎么查看实际日志？
tags:
- LinuxOps
categories: linuxOps
description: Linux死机重启后怎么查看实际日志
---



在程序的日常运维中，有时候也会遇到操作系统死机重启的情况，此时我们可能需要对此进行诊断。本文主要介绍如何查看Linux死机重启后的系统日志。

具体操作系统信息如下：
<pre>
# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

# uname -a
Linux midea-oss-hk-07 3.10.0-1062.12.1.el7.x86_64 #1 SMP Tue Feb 4 23:02:59 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux

</pre>

<!-- more -->

## 1. 查看系统重启日志


1) **进入系统所在日志目录**

执行如下命令进入系统所在日志目录：
<pre>
# cd /var/log
# ls
anaconda           boot.log-20200911  btmp-20201101   cloud-init-output.log  cron-20201101  grubby              maillog-20201018  messages           messages-20201108   rhsm             secure-20201025  spooler-20201018  tallylog
audit              boot.log-20200917  ceph            cron                   cron-20201108  grubby_prune_debug  maillog-20201025  messages-20201018  multi-queue-hw.log  samba            secure-20201101  spooler-20201025  tuned
boot.log           boot.log-20200923  chrony          cron-20201018          dmesg          lastlog             maillog-20201101  messages-20201025  nginx               secure           secure-20201108  spooler-20201101  wtmp
boot.log-20200820  btmp               cloud-init.log  cron-20201025          dmesg.old      maillog             maillog-20201108  messages-20201101  ppp                 secure-20201018  spooler          spooler-20201108  yum.log
</pre>

2) **查看过去重启日志**

执行如下命令查看过去Linux操作系统的重启日志：
<pre>
# last | grep reboot
reboot   system boot  3.10.0-1062.12.1 Thu Nov 12 06:35 - 11:09  (04:33)    
reboot   system boot  3.10.0-1062.12.1 Tue Sep 22 20:39 - 11:09 (50+14:29)  
reboot   system boot  3.10.0-1062.12.1 Tue Sep 22 18:19 - 20:39  (02:20)    
reboot   system boot  3.10.0-1062.12.1 Wed Sep 16 15:55 - 20:39 (6+04:44)   
reboot   system boot  3.10.0-1062.12.1 Thu Sep 10 16:41 - 15:54 (5+23:13)   
reboot   system boot  3.10.0-1062.12.1 Thu Sep 10 16:38 - 16:41  (00:02)    
reboot   system boot  3.10.0-1062.12.1 Wed Aug 19 11:28 - 16:38 (22+05:10)  
</pre>

>注： ```Last```  searches  back  through the file /var/log/wtmp (or the file designated by the -f flag) and displays a list of all users logged in (and out) since that file was created.

3) **查看最近一条重启记录**

执行如下命令查看最近一条Linux操作系统重启记录：
<pre>
# last reboot | head -1
reboot   system boot  3.10.0-1062.12.1 Thu Nov 12 06:35 - 11:13  (04:37)   
</pre>

4) **查看上一次关机日期和时间**

为了排查问题，也可以执行如下命令来查看上一次关机的日期和时间：
<pre>
# last -x|grep shutdown | head -1
shutdown system down  3.10.0-1062.12.1 Tue Sep 22 20:39 - 20:39  (00:00)  
</pre>
但是请注意，使用上面这条命令可能并不能够找出系统Linux操作系统```非正常关机```的情形。

5) **执行history查看系统执行了哪些操作**

我们可以使用```history```命令查看系统最近执行了哪些操作：
<pre>
# history
  236    2020-11-12 10:55:29   uname -a
  237    2020-11-12 11:00:23   cd /var/log
  238    2020-11-12 11:00:25   ls -al
  239    2020-11-12 11:00:32   ls
  240    2020-11-12 11:00:58   cat boot.log
  241    2020-11-12 11:01:03   cat boot.log | more
  242    2020-11-12 11:02:14   ls
  243    2020-11-12 11:02:24   cat dmesg
  244    2020-11-12 11:09:19   last | grep reboot
  245    2020-11-12 11:10:37   last --help
  246    2020-11-12 11:10:44   man last
  247    2020-11-12 11:13:18   last reboot | head -1
  248    2020-11-12 11:14:50   last -x|grep shutdown | head -1
  249    2020-11-12 11:17:46   history
</pre>


6) **查看/var/log/message**

此外，我们还可以查看/var/log/message文件，以进一步分析操作系统重启的一些信息：
<pre>
# cat /var/log/messages | more
Nov  8 03:37:19 midea-oss-hk-07 rsyslogd: [origin software="rsyslogd" swVersion="8.24.0-41.el7_7.4" x-pid="1816" x-info="http://www.rsyslog.com"] rsyslogd was HUPed
Nov  8 03:37:19 compile-machine systemd: Removed slice User Slice of root.
Nov  8 03:38:01 compile-machine systemd: Created slice User Slice of root.
Nov  8 03:38:01 compile-machine systemd: Started Session 68942 of user root.
Nov  8 03:38:01 compile-machine systemd: Removed slice User Slice of root.
Nov  8 03:39:01 compile-machine systemd: Created slice User Slice of root.
Nov  8 03:39:01 compile-machine systemd: Started Session 68943 of user root.
Nov  8 03:39:01 compile-machine systemd: Removed slice User Slice of root.
Nov  8 03:40:01 compile-machine systemd: Created slice User Slice of root.
Nov  8 03:40:01 compile-machine systemd: Started Session 68944 of user root.
Nov  8 03:40:01 compile-machine systemd: Removed slice User Slice of root.
Nov  8 03:41:01 compile-machine systemd: Created slice User Slice of root.
Nov  8 03:41:01 compile-machine systemd: Started Session 68945 of user root.
</pre>

<br />
<br />
<br />


