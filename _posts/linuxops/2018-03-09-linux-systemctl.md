---
layout: post
title: Linux下systemd的用法
tags:
- LinuxOps
categories: linuxOps
description: Linux下systemd的用法
---

历史上，Linux启动一直采用```init```进程。下面的命令用来启动服务：
<pre>
# sudo /etc/init.d/apache2 start

//或者
# service apache2 start
</pre>

<!-- more -->

上面的启动方法有两中缺点：

* 启动时间长： init进程是串行启动的，只有前一个进程启动完，才会启动下一个进程

* 启动脚本复杂： init进程只是执行启动脚本，不管其他事情。脚本需要自己处理各种情况，这往往使得脚本变的很长。

## 1. systemd概述

Systemd是Linux系统中最新的初始化系统(init)， 它主要的设计目标是克服sysvinit固有的缺点，提高系统的启动速度，为系统的启动和管理提供一套完整的解决方案。 根据Linux惯例，字母```d```是守护进程(daemon)的缩写。Systemd这个名字的含义，就是它要守护整个系统。


使用了Systemd，就不需要再用```init```了。Systemd取代了```initd```，成为系统的第一个进程（pid等于1)，其他进程都是它的子进程。我们可以通过如下的命令来查看Systemd的版本：
<pre>
# systemctl --version
systemd 219
+PAM +AUDIT +SELINUX +IMA -APPARMOR +SMACK +SYSVINIT +UTMP +LIBCRYPTSETUP +GCRYPT +GNUTLS +ACL +XZ -LZ4 -SECCOMP +BLKID +ELFUTILS +KMOD +IDN
</pre>

Systemd的优点是功能强大，使用方便，缺点是体系庞大，非常复杂。事实上，现在还有很多人反对使用systemd，理由就是它过于复杂，与操作系统的其他部分强耦合，违反```keep simple,keep stupid```的(unix哲学)[http://www.ruanyifeng.com/blog/2009/06/unix_philosophy.html]。

![lops-systemd-arch](https://ivanzz1001.github.io/records/assets/img/linuxops/lops_systemd_arch.png)

## 2. 系统管理

Systemd并不是一个命令，而是一组命令，涉及到系统管理的方方面面。

### 2.1 systemctl

```systemctl```是Systemd的主命令，用于管理系统。从功能上来说，systemctl是将```service```及```chkconfig```这两个命令组合到一起：

|        任务       |        旧指令                         |     新指令                                 |
|:------------------|:-------------------------------------|:------------------------------------------|
| 使某任务自动启动    | chkconfig ```--level``` 3 httpd on   | systemctl enable httpd.service            |
| 使某服务不自动启动  | chkconfig ```--level``` 3 httpd off  | systemctl disable httpd.service           |
| 检查服务状态       | service httpd status                 | systemctl status httpd.service(服务详细信息) systemctl is-active httpd.service(仅显示是否Active)  |
| 显示所有已启动服务  | chkconfig ```--list```               | systemctl list-units ```--type=service``` |
| 启动某服务         | service httpd start                  | systemctl start httpd.service             |
| 停止某服务         | service httpd stop                   | systemctl stop httpd.service              |
| 重启某服务         | service httpd restart                | systemctl restart httpd.service           |

下面我们列出一些常用的```systemctl```系统管理命令：
<pre>
// 重启系统
# sudo systemctl reboot

//关闭系统，切断电源
# sudo systemctl poweroff

//cpu停止工作
# sudo systemctl halt

//暂停系统
# sudo systemctl suspend

//让系统进入冬眠状态
# sudo systemctl hibernate

//让系统进入交互式休眠状态
# sudo systemctl hybrid-sleep

//启动进入救援状态（单用户状态)
# sudo systemctl rescue
</pre>

### 2.2 systemd-analyze
```systemd-analyze```命令用于查看系统启动耗时：
<pre>
//查看系统启动耗时
# systemd-analyze

//查看每个服务的启动耗时
# systemd-analyze blame

//显示瀑布状的启动过程流
# systemd-analyze critical-chain

//显示指定服务的启动流
# systemd-analyze critical-chain atd.service
</pre>

### 2.3 hostnamectl
```hostnamectl```命令用于查看当前主机的信息：
<pre>
//显示当前主机的信息
# hostnamectl 

//设置主机名
# sudo hostnamectl set-hostname rhel7
</pre>

### 2.4 localectl
```localectl```命令用于查看本地化设置：
<pre>
//查看本地化设置
# localectl

//设置本地化参数
# sudo localectl set-locale LANG=en_GB.utf8
# sudo localectl set-keymap en_GB
</pre>

### 2.4 timedatectl
```timedatectl```命令用于查看当前时区设置：
<pre>
//查看当前时区设置
# timedatectl

//显示所有可用时区
# timedatectl list-timezones

//设置当前时区
# sudo timedatectl set-timezone America/New_York
# sudo timedatectl set-date YYYY-MM-DD
# sudo timedatectl set-time HH:MM:SS
</pre>

### 2.5 loginctl
```loginctl```命令用于查看当前登录的用户：
<pre>
//列出当前session
# loginctl list-sessions

//列出当前登录用户
# loginctl list-users

//显示指定用户的信息
# loginctl show-user ruanyf
</pre>

## 3. Unit
### 3.1 含义
```Systemd```可以管理所有系统资源。不同的资源统称为```Unit```。Unit一共分为12种：

* Service unit: 系统服务

* Target unit: 多个unit构成一个组

* Device unit: 硬件设备

* Mount unit: 文件系统的挂载点

* Automount unit: 自动挂载点

* Path unit: 文件或路径

* Scope unit: 不是由Systemd启动的外部进程

* Slice unit: 进程组

* Snapshot unit: Systemd快照，可以切回某个快照

* Socket unit: 进程间通信的socket

* Swap unit: swap文件

* Timer unit: 定时器

```systemctl list-units```命令可以查看当前系统的所有unit:
<pre>

</pre>




**[参看]:**

1. [systemctl命令](http://man.linuxde.net/systemctl)

2. [CentOS 7之Systemd详解之服务单元设置system.service](http://blog.csdn.net/yuesichiu/article/details/51485147)

3. [Systemd 入门教程：命令篇](http://www.ruanyifeng.com/blog/2016/03/systemd-tutorial-commands.html)

4. [systemd](http://blog.51cto.com/12550795/1948348)
<br />
<br />
<br />


