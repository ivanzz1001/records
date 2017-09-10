---
layout: post
title: nginx工具型脚本-part5
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


我们在分析configure脚本的过程中，发现里面定义了很多工具型的脚本。这里我们统一的来讲述一下nginx工具型脚本。

参看：http://blog.csdn.net/poechant/article/details/7347046


<!-- more -->


## 1. auto/have脚本

脚本内容如下：
{% highlight string %}
# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $have
#define $have  1
#endif

END
{% endhighlight %}

(1) 主要功能

向自动配置头文件中标示有指定的参数的宏定义。

(2) 作用对象

```$NGX_AUTO_CONFIG_H```变量所表示的自动生成头文件，默认为objs/ngx_auto_config.h

(3) 示例

如果have值为SOME_FLAG，则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
{% highlight string %}
#ifndef SOME_FLAG
#define SOME_FLAG 1
#endif
{% endhighlight %}







<br />
<br />
<br />

