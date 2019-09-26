---
layout: post
title: 字节对齐与结构体大小
tags:
- cplusplus
categories: cplusplus
description: 字节对齐与结构体大小
---

结构体的sizeof值,并不是简单的将其中各元素所占字节相加，而是要考虑到存储空间的字节对齐问题。本文讲述一下C++中结构体大小的求法。 实验操作系统环境为64bit Centos7.3：

<!-- more -->
<pre>
# uname -a
Linux compile 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
</pre>


## 1. 解释
现代计算机中内存空间都是按照byte划分的，从理论上讲似乎对任何类型的变量的访问可以从任何地址开始， 但实际情况是在访问特定类型变量的时候经常在特定的内存地址访问， 这就需要各种类型数据按照一定的规则在空间上排列， 而不是顺序的一个接一个的排放，这就是对齐。

各个硬件平台对存储空间的处理上有很大的不同。一些平台对某些特定类型的数据只能从某些特定的地址开始存取。比如有些架构的CPU在访问一个没有进行对齐的变量的时候会发生错误， 那么在这种架构下编程必须保证字节对齐。其他平台可能没有这种情况，但是最常见的是如果不按照适合其平台要求对数据存放进行对齐的话， 会在存取效率上带来损失。 比如有些平台每次读都是从偶地址开始， 如果一个int型数据存放在```4字节对齐```的位置， 那么一个读周期就可以读出这32bit； 而如果存放的位置不是```4字节对齐```，则很有可能需要多个读周期才能将这32bit数据读取出来（可能需要进行高低字节拼凑)。

## 2. 准则
其实字节对齐的细节和具体编译器的实现相关，但一般而言，满足三个准则：

* 结构体变量的首地址能够被其```最宽基本类型```成员的大小所整除

* 结构体每个成员相对于结构体首地址的偏移量都是成员大小的整数倍，如有需要编译器会在成员之间加上填充字节

* 结构体的总大小为结构体```最宽基本类型```成员大小的整数倍，如有需要编译器会在最末一个成员之后加上填充字节

## 3. 基本概念

```字节对齐```： 计算机存储系统中以byte为单位存储数据，不同数据类型所占的空间不同。例如，整形(int)数据占用4个字节，字符型(char)数据占用一个字节， 短整形(short)数据占用两个字节，等等。计算机为了快速的读写数据，默认情况下将数据存放在某个地址的起始位置，如： 整形数据(int)默认存储在地址能被4整除的起始位置，字符型数据(char)可以存放在任何地址位置（被1整除），短整形(short)数据存储在地址能被2整除的起始位置。这就是默认字节对齐方式。

## 4. 结构体长度求法

**1) 成员都相同时（或含数组，且数组数据类型与结构体其他成员数据类型相同）**

结构体长度 = 成员数据类型长度 x 成员个数;

结构体中数组长度 = 数组数据类型长度 x 数组元素个数;


**2) 成员不同且不含其他结构体时**

* 分析各个成员长度；

* 找出最大长度的成员， 其长度M， 则结构体的长度一定是```M```的整数倍；

* 并按最大成员长度出现的位置将结构体分为若干部分；

* 各个部分长度一次相加，求出和```S```，然后使```S```进行```M```字节上对齐，即为该部分的长度；

* 将各个部分长度相加之和即为结构体长度；


**3) 含有其他结构体时**

* 分析各个成员长度;

* 如果该成员是另一个```结构体```，则其长度按上面的方法计算;

* 分析各个成员的长度（成员为另一个```结构体```的，则继续分析其子成员），求出最大值```M```;

* 若长度最大成员在```子结构体```中，则按结构体成员为分界点分界； 若其他成员中有最大成员，则该成员为分界点。然后求出各段长度S，然后使S进行```M```字节上对齐

* 将各个部分长度相加之和即为结构体长度


## 5. 空结构体
<pre>
struct S5{};

int sz = sizeof(struct S5) = 0;   //结果为1
</pre>

```空结构体```(不含数据成员)的大小不为0，而是1。试想一个```不占空间```的变量如何被取地址，两个不同的```空结构体```变量又如何得以区分呢？ ```空结构体```变量也得被存储，这样编译器也就只能为其分配一个字节的空间用于占位了。


## 6. 有static的结构体
<pre>
struct S4{
    char a;
    long b;
    static long c;
};
</pre>

静态变量存放在全局数据区内，而sizeof计算栈中分配的空间大小，故不计算在内。S4的大小为 ```(8 + 8) = 16```。


## 7. 举例

### 7.1 示例1

{% highlight string %}
struct student{
    char name[5];
    int num;
    short score;
};
{% endhighlight %}

上面本来只用了```11bytes```(5+4+2)空间，但是由于int型默认4字节对齐，存放在能被4整除的起始位置。即，如果name[0]从0地址开始存放，它占用5bytes，而num则从第```8```（偏移量）个字节开始存放，因此sizeof(struct student) = 16。于是中间空出几个字节闲置着， 但这样便于计算机快速读写数据，是一种以空间换时间的方式。其数据对齐如下：
<pre>
|char|char|char|char| 
|char|----|----|----| 
|--------int--------| 
|--short--|----|----| 
</pre>

如果我们将结构体中变量的顺序改变为：
{% highlight string %}
struct student 
{ 
    int num; 
    char name[5]; 
    short score; 
};
{% endhighlight %}
此时sizeof(struct student) = 12， 其数据对齐如下图：
<pre>
|--------int--------| 
|char|char|char|char| 
|char|----|--short--| 
</pre>


### 7.2 示例2

**1） 示例2.1**
{% highlight string %}
struct test1 
{ 
	int a; 
	int b[4]; 
};
{% endhighlight %}
这里sizeof(struct test1) = 20;

**2) 示例2.2**

{% highlight string %}
struct test2 
{ 
	char a; 
	int b; 
	double c; 
	bool d; 
};
{% endhighlight %} 

```分析:``` 该结构体最大长度double类型，长度是8，因此结构体长度分为两部分：

* 第一部分是a、b、c的长度和，长度分别是1、4、8，则该部分长度和为13，进行8字节上对齐之后长度为16

* 第二部分是d，长度为1，进行8字节上对齐之后长度为8

因此最后的长度为sizeof(struct test2) = 24;


**3） 示例2.3**
{% highlight string %}
struct test3{
    char a;
    struct test2 b;    //见上
    int cc;
};
{% endhighlight %}
```分析:``` 该结构体有三个成员，其中第二个bb是类型为test2的结构体，长度为24，且该结构体最大长度成员类型为double类型， 以后成员中没有double型，所以按```b```分解为两部分：

* 第一部分有a、b两部分， a长度为1， b长度为24， 总长度为25， 取8字节上对齐为32

* 第二部分有cc，长度为4， 取8字节上对齐为8

因此，最后的长度为sizeof(struct test3) = 40

**4) 示例2.4**
{% highlight string %}
struct test4{
   char a;
   int b;
};
struct test5{
    char c;
    struct test4 d;
    double e;
    bool f;
};
{% endhighlight %}
```分析：``` 这里struct test5中含有结构体struct test4， 按上面```示例2.2```容易知道sizeof(struct test4) = 8，且其成员最大长度为4；则结构体test5的最大成员长度为8(double型），因此这里以```e```作为分界点，将test5分成两部分：

* 第一部分由c、d、e组成，长度分别为1、8、8，因此总长度为17，进行8字节上对齐为24；

* 第二部分由f组成，长度为1，取8字节上对齐为8

因此,最后的长度sizeof(struct test5) = 32


## 8. union长度

union的长度取决于其中的长度最大的那个成员变量的长度。即union中成员变量是重叠摆放的，其开始地址相同。

其实union的各个成员是以同一个地址开始存放的，每一个时刻只可以存储一个成员，这样要求它在分配内存单元的时候要满足两点：

* 一般而言，union类型实际占用存储空间为其最长的成员所占的存储空间；

* 若是该最长的存储空间对其他成员的元类型（如果是数组，取其类型的数据长度。例如int a[5]，其类型长度为4）不满足整除关系，则该最大空间自动延伸；

下面我们来参看这段代码：
{% highlight string %}
union mm{
     char a;
     int b[5];
     double c;
     int d[3];
};
{% endhighlight %}
本来```union mm```的空间应该是```sizeof(int)*5 = 20```；但是如果只是20个单元的话，可以存放几个double类型呢？ 两个半？ 当然不可以，所以```union mm```的空间延伸为既要大于20，又要满足其他成员所需空间的整数倍， 即24。

所以union的存储空间先看它的成员中哪个占用的空间最大，拿它与其他成员的元长度比较， 如果可以整除就行。



## 9. 运行示例

### 9.1 运行示例1

下面给出上面所讲各例子的运行示例```test.cpp```：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


struct student{
   char name[5];
   int num;
   short score;
};

struct test1 
{ 
    int a; 
    int b[4]; 
};

struct test2 
{ 
    char a; 
    int b; 
    double c; 
    bool d; 
};


struct test3{
    char a;
    struct test2 b;    
    int cc;
};

 struct test4{
    char a;
    int b;
 };

 struct test5{
      char c;
      struct test4 d;
      double e;
      bool f;
 };

 union mm{
   char a;
   int b[5];
   double c;
   int d[3];
 };

int main(int argc,char *argv[])
{
    printf("size student: %d\n", sizeof(struct student));
    printf("size test1: %d\n", sizeof(struct test1));
    printf("size test2: %d\n", sizeof(struct test2));
    printf("size test3: %d\n", sizeof(struct test3));
    printf("size test4: %d\n", sizeof(struct test4));
    printf("size test5: %d\n", sizeof(struct test5));
    printf("size mm: %d\n", sizeof(union mm));
    return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
size student: 16
size test1: 20
size test2: 24
size test3: 40
size test4: 8
size test5: 32
size mm: 24
</pre>


### 9.2 运行示例2

参看如下测试示例```test.c```:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

typedef union {
	long a;
	int b[5];
	char c;
} Date;

typedef struct {
	int a;
	char b;
	Date d;
	short e;
} Some;


int main(int argc, char *argv[])
{
	printf("sizeof(Date): %d\n", sizeof(Date));
	printf("sizeof(Some): %d\n", sizeof(Some));
	printf("%d\n", sizeof(Date) + sizeof(Some));

	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c
# ./test
sizeof(Date): 24
sizeof(Some): 40
64
</pre>



<br />
<br />

**[参考]**

1. [内存对齐与补齐 字节对齐与结构体大小](https://blog.csdn.net/u010479322/article/details/51137907)

2. [相关工具](https://tool.lu/)

<br />
<br />
<br />





