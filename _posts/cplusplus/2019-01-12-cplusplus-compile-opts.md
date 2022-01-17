---
layout: post
title: 编译选项含义——CFLAGS和LDFLAGS
tags:
- cplusplus
categories: cplusplus
description: 编译选项含义
---


在编译一些开源包的时候，我们经常会遇到```CFLAGS```、```DFLAGS```等一些变量，这里我们简单介绍一下，在此做个记录，以便于后续查阅。

<!-- more -->


## 1. CFLAGS 和CXXFLAGS区别

* CFLAGS通常表示用于C编译器的选项。比如指定头文件(.h文件）的路径
<pre>
CFLAGS=-I/usr/include -I/path/include
</pre>
通过上面就把相应目录追加到了头文件的查找路径中。

* CXXFLAGS表示用于C++编译器的选项

## 2. LDFLAGS和LIBS区别
LDFLAGS是选项，LIBS是要链接的库。
<pre>
LDFLAGS = -L/var/xxx/lib -L/opt/mysql/lib
LIBS = -lmysqlclient -liconv
</pre>

## 3. 命令行
建议在gcc命令行中显式的指定这些参数，而不是```export```的方式。
<pre>
# gcc ${CFLAGS} main.c
</pre>

再比如：
<pre>
# ./configure --prefix=install_dir CFLAGS="-I/home/ivanzz1001/3rd/openssl/include" LDFLAGS="-L/home/ivanzz1001/3rd/openssl/lib"
</pre>






<br />
<br />

**[参看]**


1. [编译选项含义:CFLAGS和LDFLAGS](https://www.jianshu.com/p/08537d6cc0a3)

<br />
<br />
<br />


