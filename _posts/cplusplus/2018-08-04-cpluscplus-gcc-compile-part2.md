---
layout: post
title: gcc静态编译
tags:
- cplusplus
categories: cplusplus
description: gcc程序的编译过程和链接原理
---


本文主要包含两部分的内容：

* 静态编译

* 在无C++运行环境下，运行大部分C++代码

当前的操作系统环境以及GCC版本如下：
<pre>
# lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 16.04.3 LTS
Release:        16.04
Codename:       xenial

# gcc --version
gcc (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

</pre>

<!-- more -->

## 1. 程序静态编译
参看如下代码(test.cpp):
{% highlight string %}
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
        std::vector<int> *v = new std::vector<int>();
        
        v->push_back(1);
        v->push_back(2);

        for(std::vector<int>::iterator it = v->begin(); it != v->end(); it++)
                std::cout<<*it<<std::endl;

        std::cout<<"hello, world"<<std::endl;

        delete v;
        return 0x0;
}
{% endhighlight %}

1） **动态编译**

首先我们来执行动态编译：
<pre>
# gcc -o test  test.cpp -lstdc++
# ./test
1
2
hello, world

# readelf -a ./test | grep NEEDED
 0x0000000000000001 (NEEDED)             共享库：[libstdc++.so.6]
 0x0000000000000001 (NEEDED)             共享库：[libgcc_s.so.1]
 0x0000000000000001 (NEEDED)             共享库：[libc.so.6]
</pre>
程序编译后运行正常，使用```readelf```命令查看有3个依赖库。

2) **静态编译**
<pre>
# gcc -o test  test.cpp -static -lstdc++
# ./test
1
2
hello, world
# readelf -a ./test | grep NEEDED
</pre>
通过此种方式，我们可以看到没有其他的依赖库。






<br />
<br />

**[参看]**

1. [如何在无C++运行环境下，运行大部分的C++代码](https://blog.csdn.net/u011057800/article/details/105856817)

<br />
<br />
<br />





