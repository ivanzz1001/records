---
layout: post
title: nginx几个重要的头文件
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


在分析nginx源代码时，我们看到几乎所有的源文件都会首先包含```ngx_config.h```，```ngx_core.h```这两个头文件。这两个头文件又包含一些其他头文件，其他头文件我们主要会介绍```ngx_linux_config.h```与```ngx_auto_config.h```这两个。按头文件包含**由里到外**的顺序为：


<!-- more -->


* ngx_auto_config.h

* ngx_linux_config.h

* ngx_config.h

* ngx_core.h


如下我们也按这个顺序来进行介绍。




## 1. ngx_auto_config.h头文件











<br />
<br />
<br />

