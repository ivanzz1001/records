---
layout: post
title: go开发环境的搭建
tags:
- go-language
categories: go语言
description: go开发环境的搭建
---

本文主要简单讲述一下go开发环境的搭建，这里做一个简单的记录。

<!-- more -->


## 1. go环境变量设置

下载安装程序https://golang.org/dl/ (墙内下载地址http://www.golangtc.com/download)。Go开发环境的搭建比较简单，这里主要介绍一下需要配置的几个环境变量：

* ```GOROOT：``` Go的安装目录
* ```GOPATH：``` go build时除了会查找GOROOT目录下的src目录，还会查找这里GOPATH指定的目录。因此这里GOPATH可用作我们的工作目录
* ```GOBIN：``` 一般用作go install安装程序的顶层目录，可以不用指定
* ```GOOS:``` 用于指定Go运行的操作系统
* ```GOARCH：``` 指定系统环境，i386表示x86，amd64表示x64
* ```PATH：``` 一般需要导出GOROOT,GOROOT下的bin两个目录

例如：

![go-env](https://ivanzz1001.github.io/records/assets/img/go/go-env.png)


IDE工具：**JetBrains Gogland**

画图工具: EDraw



<br />
<br />
**[参看]：**

http://www.cnblogs.com/caiyezi/p/5641363.html

2. [GoLand软件的免激活使用](http://blog.csdn.net/benben_2015/article/details/78725467)

<br />
<br />
<br />

