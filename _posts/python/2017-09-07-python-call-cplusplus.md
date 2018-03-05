---
layout: post
title: Python3与C/C++的相互调用
tags:
- python
categories: python
description: Python3与C/C++的相互调用
---

Python与C/C++可以相互调用。在这一章节里，我们会分别从Linux与Windows平台通过示例的方式介绍它们之间是如何调用的。



<!-- more -->

<br />
<br />

## 1. Linux下Python3与C/C++的相互调用

这里我们分成两个部分：Python3调用C/C++； C/C++调用Python3.

### 1.1 Linux下Python3调用C/C++

**(1) Python3调用C动态链接库**

Python3调用C库比较简单，不需要经过任何封装打包成so，再使用python的ctypes调用即可。我们分成如下步骤：

* 编写C语言源文件并打包生成动态链接库
* Python3调用动态链接库

```头文件:test.h```
{% highlight string %}
#ifndef __TEST_H_
#define __TEST_H_


int add(int a,int b);


#endif
{% endhighlight %}

```源文件:test.c```
{% highlight string %}
#include "test.h"

int add(int a,int b)
{
        return a + b;
}
{% endhighlight %}

编译成动态链接库：
<pre>
[root@localhost python_c]# gcc -o libtest.so -shared -fPIC test.c
[root@localhost python_c]# ls
libtest.so  test.c  test.h
</pre>

<br />
Python3调用动态链接库：
{% highlight string %}
import ctypes

lib = ctypes.cdll.LoadLibrary("./libtest.so")

result = lib.add(2,4)

print(result)
{% endhighlight %}

执行程序：
<pre>
[root@localhost python_c]# python3 ./python_c.py 
6
</pre>

<br />

**(2) Python3调用C++动态链接库**

Python不能直接调用C++函数，需要通过对应的包装，通过C函数的方式导出才能够被使用。

```头文件:test.h```
{% highlight string %}
#ifndef __TEST_H_
#define __TEST_H_

class Algorithm{

public:
        int add(int a,int b);

};


extern "C" int add(int a,int b);



#endif
{% endhighlight %}

```源文件：test.cpp```
{% highlight string %}
#include <stdlib.h>
#include <stdio.h>
#include "test.h"

int Algorithm::add(int a,int b)
{
        printf("Algorithm::add-------\n");
        return a + b;
}

int add(int a,int b)
{
        Algorithm alg;
        return alg.add(a,b);
}
{% endhighlight %}
编译成动态链接库：
<pre>
[root@localhost python_cplusplus]# gcc -o libtest.so -shared -fPIC  test.cpp
[root@localhost python_cplusplus]# ls
libtest.so  python_cplusplus.py  test.cpp  test.h
</pre>

<br />

Python3调用动态链接库：
{% highlight string %}
import ctypes

lib = ctypes.cdll.LoadLibrary("./libtest.so")

result = lib.add(100,4)

print(result)
{% endhighlight %}
执行程序：
<pre>
[root@localhost python_cplusplus]# python3 ./python_cplusplus.py 
Algorithm::add-------
104
</pre>
  







<br />
<br />
<br />

