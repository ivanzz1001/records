---
layout: post
title: Centos7离线安装gcc
tags:
- LinuxOps
categories: linuxOps
description: Centos7离线安装gcc
---

本节我们简要介绍一下Centos7操作系统环境下GCC的安装。当前我们的操作系统环境为：


<!-- more -->
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux sz-oss-01.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>


## 1. gcc安装包下载

结合我们当前的操作系统环境，查看```http://vault.centos.org/7.3.1611/os/x86_64/Packages/```地址， 里面有Centos7 所有rpm包 
下载图中的包。这里我们下载GCC安装的必要rpm包：
<pre>
# ls -al
total 41988
drwxr-xr-x 2 root root     4096 May 16 11:04 .
drwxr-xr-x 3 root root     4096 May 16 11:02 ..
-rwxrwxrwx 1 root root  6225248 May 16 11:03 cpp-4.8.5-11.el7.x86_64.rpm
-rwxrwxrwx 1 root root 16957504 May 16 11:03 gcc-4.8.5-11.el7.x86_64.rpm
-rwxrwxrwx 1 root root  3767280 May 16 11:04 glibc-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root 12038164 May 16 11:04 glibc-common-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root  1106292 May 16 11:04 glibc-devel-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root   684244 May 16 11:04 glibc-headers-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root  1599280 May 16 11:04 glibc-static-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root   213324 May 16 11:04 glibc-utils-2.17-157.el7.x86_64.rpm
-rwxrwxrwx 1 root root    51732 May 16 11:04 libmpc-1.0.1-3.el7.x86_64.rpm
-rwxrwxrwx 1 root root    32904 May 16 11:04 libmpc-devel-1.0.1-3.el7.x86_64.rpm
-rwxrwxrwx 1 root root   208316 May 16 11:04 mpfr-3.1.1-4.el7.x86_64.rpm
-rwxrwxrwx 1 root root    69904 May 16 11:04 mpfr-devel-3.1.1-4.el7.x86_64.rpm
</pre>

## 2. 安装gcc
使用如下的命令进行安装：
<pre>
# rpm -ivh *.rpm --nodeps --force

//或使用如下的命令
# yum localinstall *.rpm
</pre>

## 3. 验证是否安装成功
<pre>
# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-11)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

## 4. 安装gcc-c++
如果要安装支持编译c++的gcc编译器，则需要安装如下包：
<pre>
# ls -al
total 51584
drwxrwxr-x  2 root root     4096 May 17 17:11 .
drwx------ 18 root root     4096 Jun 13 16:27 ..
-rw-r--r--  1 root root  6225248 May 16 10:51 cpp-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root 16957504 May 16 10:53 gcc-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root  7519876 May 17 17:07 gcc-c++-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root  3767280 May 16 10:53 glibc-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root 12038164 May 16 10:54 glibc-common-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root  1106292 May 16 10:54 glibc-devel-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root   684244 May 16 10:54 glibc-headers-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root  1599280 May 16 10:52 glibc-static-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root   213324 May 16 10:52 glibc-utils-2.17-157.el7.x86_64.rpm
-rw-r--r--  1 root root    51732 May 16 10:54 libmpc-1.0.1-3.el7.x86_64.rpm
-rw-r--r--  1 root root    32904 May 16 10:55 libmpc-devel-1.0.1-3.el7.x86_64.rpm
-rw-r--r--  1 root root   306804 May 17 17:10 libstdc++-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root  1574820 May 17 17:11 libstdc++-devel-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root   418184 May 17 17:10 libstdc++-static-4.8.5-11.el7.x86_64.rpm
-rw-r--r--  1 root root   208316 May 16 10:55 mpfr-3.1.1-4.el7.x86_64.rpm
-rw-r--r--  1 root root    69904 May 16 10:55 mpfr-devel-3.1.1-4.el7.x86_64.rpm
</pre>







<br />
<br />

**[参看]**

1. [CentOS 7 rpm安装gcc 详解](https://blog.csdn.net/yangjjuan/article/details/70244935)




<br />
<br />
<br />


