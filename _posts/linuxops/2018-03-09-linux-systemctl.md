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
//列出正在运行的unit
# systemctl list-units

//列出所有unit，包括没有找到配置文件的或者启动失败的
# systemctl list-units --all

//列出所有没有运行的unit
# systemctl list-units --all --state=inactive

//列出所有加载失败的unit
# systemctl list-units --failed

//列出所有正在运行的类型为service 的unit
# systemctl list-units --type=service
</pre>


### 3.2 Unit状态
```systemctl status```命令用于查看系统状态和单个Unit的状态：
<pre>
//显示系统状态
# systemctl status 

//显示单个unit的状态
# systemctl status bluetooth.service

//显示远程主机的某个unit的状态
# systemctl status -H root@rhel7.example.com status httpd.service
</pre>

此外，除了```status```命令，```systemctl```还提供了三个查询状态的简单方法，主要供脚本内部的判断语句使用。
<pre>
//显示某个unit服务是否正在运行
# systemctl is-active application.service

//显示某个unit服务是否处于启动失败状态
# systemctl is-failed application.service

//显示某个unit服务是否建立了启动链接
# systemctl is-enabled application.service
</pre>

### 3.3 Unit管理
对于用户来说，最常用的是下面这些命令，用于启动和停止Unit（主要是Service）：
<pre>
//立即启动一个服务
# sudo systemctl start apache.service

//立即停止一个服务
# sudo systemctl stop apache.service

//重启一个服务
# sudo systemctl restart apache.service

//杀死一个服务的所有子进程
# suso systemctl kill apache.service

//重新加载一个服务的配置文件
# suso systemctl reload apache.service

//重载所有修改过的配置文件
# sudo systemctl daemon-reload

//显示某个unit的所有底层参数
# systemctl show httpd.service

//显示某个unit的指定属性的值
# systemctl show -p CPUShares httpd.service

//设置某个unit的指定属性
# systemctl set-property httpd.service CPUShares=500
</pre>

### 3.4 依赖关系
Unit之间存在依赖关系： A依赖与B，就意味着Systemd在启动A的时候，同时会去启动B。

```systemctl list-dependencies```命令列出一个unit的所有依赖：
<pre>
# systemctl list-dependencies nginx.service
</pre>

上面命令的输出结果之中，有些依赖是```Target```类型，默认并不会展开显示。如果要展开Target，就需要使用```--all```选项：
<pre>
# systemctl list-dependencies --all nginx.service
</pre>

## 4. Unit配置文件
### 4.1 概述
每一个Unit都有一个配置文件，告诉Systemd怎么启动这个Unit。```Systemd```默认从```/etc/systemd/system/```目录读取配置文件。但是，里面存放的大部分文件都是符号链接，指向目录```/usr/lib/systemd/system/```目录下的文件，因此真正的文件存放在那个目录下。

```systemctl enable```命令用于在上面两个目录之间建立符号链接关系：
<pre>
# sudo systemctl enable clamd@scan.service

//等价于
# sudo ln -s /usr/lib/systemd/system/clamd@scan.service /etc/systemd/system/clamd@scan.service
</pre>

如果配置文件里面设置了开机启动，```systemctl enable```就相当于激活开机启动。与之对应的，```systemctl disable```命令用于在两个目录之间撤销符号链接关系，相当于撤销开机启动：
<pre>
# sudo systemctl disable clamd@scan.service
</pre>

配置文件名称的后缀，就是该```Unit```的种类，比如```sshd.socket```。如果省略，Systemd默认后缀名为```.service```。所以```sshd```会被理解成```sshd.service```。

### 4.2 配置文件的状态
```systemctl list-unit-files```命令用于列出所有配置文件：
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


