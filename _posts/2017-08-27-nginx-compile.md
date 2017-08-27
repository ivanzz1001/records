---
layout: post
title: nginx源代码编译安装
tags:
- nginx
categories: nginx
description: nginx源代码编译安装
---

本文主要讲述从nginx源代码编译安装nginx的过程。我们这里使用的操作系统环境是32位Ubuntu16.04，所用的nginx版本为nginx1.10.3。 文章主要包括两个部分：

* Configure编译选项的介绍
* Ubuntu16.04(32bit)下nginx1.10.3的安装 

<!-- more -->


## 1. Building nginx from sources
这里主要参看[nginx官方网站](http://nginx.org/en/docs/configure.html)。在从源代码编译nginx时，需要通过```configure```命令来进行编译选项的配置。它定义了系统的许多方面，包括nginx连接所采用的方法。配置完成之后，会生成一个Makefile文件。```configure```命令支持如下的一些参数：

* ```--prefix=path:``` 它定义了存放服务器文件的路径。configure及nginx.conf配置文件中的所有路径都是以该目录作为一个相对路径（库源文件路径除外）。默认情况下，其会被设置为/usr/local/nginx目录.
* --sbin-path=path: 设置nginx可执行文件的名字。该名字只在安装过程中会被用到。默认情况下该文件会被命名为prefix/sbin/nginx
* 









<br />
<br />
<br />

