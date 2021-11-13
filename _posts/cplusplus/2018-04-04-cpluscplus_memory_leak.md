---
layout: post
title: C/C++内存泄露排查(转)
tags:
- cplusplus
categories: cplusplus
description: C/C++内存泄露排查
---

本文主要介绍一下如何排查C/C++中内存泄露(OOM: out of memory)

“该死系统存在内存泄漏问题”，项目中由于各方面因素，总是有人抱怨存在内存泄漏，系统长时间运行之后，可用内存越来越少，甚至导致了某些服务失败。内存泄漏是最难发现的常见错误之一，因为除非用完内存或调用```malloc```失败，否则都不会导致任何问题。实际上，使用C/C++这类没有垃圾回收机制的语言时，你很多时间都花在处理如何正确释放内存上。如果程序运行时间足够长，如后台进程运行在服务器上，只要服务器不宕机就一直运行，一个小小的失误也会对程序造成重大的影响，如造成某些关键服务失败。


<!-- more -->

对于内存泄漏，本人深有体会！实习的时候，公司一个项目中就存在内存泄漏问题，项目的代码两非常大，后台进程也比较多，造成内存泄漏的地方比较难找。这次机会是我对如何查找内存泄漏问题，有了一定的经验，后面自己的做了相关实验，在此我分享一下内存泄漏如何调试查找，主要内容如下：

* 内存泄漏简介
* Windows平台下的内存泄漏检测
  * 检测是否存在内存泄漏问题
  * 定位具体的内存泄漏地方
* Linux平台下的内存泄漏检测 
* 总结

其实Windows、Linux下面的内存检测都可以单独开篇详细介绍，方法和工具也远远不止文中介绍到的，我的方法也不是最优的，如果您有更好的方法，也请您告诉我和大家。


## 1. 内存泄漏简介及后果
wikipedia中这样定义内存泄漏：在计算机科学中，内存泄漏指由于疏忽或错误造成程序未能释放已经不再使用的内存的情况。内存泄漏并非指内存在物理上的消失，而是应用程序分配某段内存后，由于设计错误，导致在释放该段内存之前就失去了对该段内存的控制，从而造成了内存的浪费。

最难捉摸也最难检测到的错误之一是内存泄漏，即未能正确释放以前分配的内存的 bug。 只发生一次的小的内存泄漏可能不会被注意，但泄漏大量内存的程序或泄漏日益增多的程序可能会表现出各种征兆：从性能不良（并且逐渐降低）到内存完全用尽。 更糟的是，泄漏的程序可能会用掉太多内存，以致另一个程序失败，而使用户无从查找问题的真正根源。 此外，即使无害的内存泄漏也可能是其他问题的征兆。

内存泄漏会因为减少可用内存的数量从而降低计算机的性能。最终，在最糟糕的情况下，过多的可用内存被分配掉导致全部或部分设备停止正常工作，或者应用程序崩溃。内存泄漏可能不严重，甚至能够被常规的手段检测出来。在现代操作系统中，一个应用程序使用的常规内存在程序终止时被释放。这表示一个短暂运行的应用程序中的内存泄漏不会导致严重后果。


在以下情況，内存泄漏导致较严重的后果：

* 程序运行后置之不理，并且随着时间的流失消耗越来越多的内存（比如服务器上的后台任务，尤其是嵌入式系统中的后台任务，这些任务可能被运行后很多年内都置之不理）；

* 新的内存被频繁地分配，比如当显示电脑游戏或动画视频画面时；

* 程序能够请求未被释放的内存（比如共享内存），甚至是在程序终止的时候；

* 泄漏在操作系统内部发生；

* 泄漏在系统关键驱动中发生；

* 内存非常有限，比如在嵌入式系统或便携设备中；

* 当运行于一个终止时内存并不自动释放的操作系统（比如AmigaOS）之上，而且一旦丢失只能通过重启来恢复。

下面我们通过以下例子来介绍如何检测内存泄漏问题：
{% highlight string %}
#include <stdlib.h>
#include <iostream>
using namespace std;
 
void GetMemory(char *p, int num)
{
    p = (char*)malloc(sizeof(char) * num);//使用new也能够检测出来
}
 
int main(int argc,char** argv)
{
    char *str = NULL;
    GetMemory(str, 100);
    cout<<"Memory leak test!"<<endl;

    //如果main中存在while循环调用GetMemory(), 那么问题将变得很严重
    //while(1){GetMemory(...);}

    return 0;
}
{% endhighlight %}

实际中不可能这么简单，如果这么简单也用不着别的方法，程序员一眼就可以看出问题，此程序只用于测试。

## 2. Windows平台下的内存泄漏检测

### 2.1 检测是否存在内存泄漏问题
Windows平台下面Visual Studio 调试器和 C 运行时 (CRT) 库为我们提供了检测和识别内存泄漏的有效方法，原理大致如下：内存分配要通过CRT在运行时实现，只要在分配内存和释放内存时分别做好记录，程序结束时对比分配内存和释放内存的记录就可以确定是不是有内存泄漏。在vs中启用内存检测的方法如下：

1) **步骤一： 添加相应头文件和宏定义**

在程序中添加以下语句：
{% highlight string %}
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
{% endhighlight %}

>注：```#include```语句必须采用上文所示顺序。如果更改了顺序，所使用的函数可能无法正常工作。

通过包括```crtdbg.h```，将```malloc()```和```free()```函数映射到它们的调试版本，即```_malloc_dbg()```和 ```_free_dbg()```，这两个函数将跟踪内存分配和释放。 此映射只在调试版本（在其中定义了```_DEBUG```）中发生。 发布版本使用普通的 malloc() 和free()函数。


```#define```语句将CRT堆函数的基版本映射到对应的```“Debug”```版本。 并非绝对需要该语句；但如果没有该语句，内存泄漏转储包含的有用信息将较少。


2) **步骤二： 设置转储内存泄露信息**

在上面步骤1中添加了相应的头文件和宏定义之后，可以通过在程序中包括以下语句（通常应恰好放在程序退出位置之前）来转储内存泄漏信息：
{% highlight string %}
_CrtDumpMemoryLeaks();
{% endhighlight %}

此时，完整的代码如下：
{% highlight string %}
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
 
#include <iostream>
using namespace std;
 
void GetMemory(char *p, int num)
{
    p = (char*)malloc(sizeof(char) * num);      //note: line 10
}
 
int main(int argc,char** argv)
{
    char *str = NULL;
    GetMemory(str, 100);
    cout<<"Memory leak test!"<<endl;
    _CrtDumpMemoryLeaks();
    return 0;
}
{% endhighlight %}

当在调试器下运行程序时，[_CrtDumpMemoryLeaks](http://msdn.microsoft.com/zh-cn/library/d41t22sb.aspx) 将在[“输出”窗口](http://msdn.microsoft.com/zh-cn/library/3hk6fby3.aspx)中显示内存泄漏信息。 内存泄漏信息如下所示：

![memory-leak](https://ivanzz1001.github.io/records/assets/img/cplusplus/memory-leak-figure1.png)

如果没有使用**#define _CRTDBG_MAP_ALLOC**语句，内存泄漏转储将如下所示：

![memory-leak](https://ivanzz1001.github.io/records/assets/img/cplusplus/memory-leak-figure2.png)

未定义**_CRTDBG_MAP_ALLOC**时，所显示的会是：

* 内存分配编号（在```大括号``内）

* 块类型（普通、客户端或 CRT）

    * ```“普通块”```是由程序分配的普通内存。

    * ```“客户端块”```是由 MFC 程序用于需要析构函数的对象的特殊类型内存块。 MFC new 操作根据正在创建的对象的需要创建普通块或客户端块。

    * ```“CRT 块”```是由 CRT 库为自己使用而分配的内存块。 CRT 库处理这些块的释放，因此您不大可能在内存泄漏报告中看到这些块，除非出现严重错误（例如 CRT 库损坏）。

从不会在内存泄漏信息中看到下面两种块类型：

>注：从不会在内存泄漏信息中看到下面两种块类型：
>
> * “可用块”是已释放的内存块。
> 
> * “忽略块”是您已特别标记的块，因而不出现在内存泄漏报告中。

* 十六进制形式的内存位置。

* 以字节为单位的块大小。

* 前16字节的内容（亦为十六进制）。


当定义了```_CRTDBG_MAP_ALLOC```时，还会显示在其中分配泄漏的内存的文件。 文件名后括号中的数字（本示例中为 10）是该文件中的行号。


----------

注意,如果程序总是在同一位置退出，调用[_CrtDumpMemoryLeaks](http://msdn.microsoft.com/zh-cn/library/d41t22sb.aspx)将非常容易。 如果程序从多个位置退出，则无需在每个可能退出的位置放置对```_CrtDumpMemoryLeaks```的调用，而可以在程序开始处包含以下调用：
{% highlight string %}
_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
{% endhighlight %}

该语句在程序退出时自动调用_CrtDumpMemoryLeaks()。 必须同时设置```_CRTDBG_ALLOC_MEM_DF```和 ```_CRTDBG_LEAK_CHECK_DF```两个位域，如前面所示。

### 2.2 定位具体的内存泄漏地方
通过上面的方法，我们几乎可以定位到是哪个地方调用内存分配函数malloc()和new()等，如上例中的GetMemory()函数中，即第```10行```！但是不能定位到，在哪个地方调用GetMemory()导致的内存泄漏，而且在大型项目中可能有很多处调用GetMemory()。如何要定位到在哪个地方调用GetMemory()导致的内存泄漏？

定位内存泄漏的另一种技术涉及在关键点对应用程序的内存状态拍快照。 CRT库提供一种结构类型```_CrtMemState```，您可用它存储内存状态的快照：
{% highlight string %}
_CrtMemState s1, s2, s3;
{% endhighlight %}

若要在给定点对内存状态拍快照，请向[_CrtMemCheckpoint](http://msdn.microsoft.com/zh-cn/library/h3z85t43.aspx)函数传递```_CrtMemState```结构。 该函数用当前内存状态的快照填充此结构:
{% highlight string %}
_CrtMemCheckpoint( &s1 );
{% endhighlight %}

通过向[_CrtMemDumpStatistics](http://msdn.microsoft.com/zh-cn/library/swh3417y.aspx)函数传递 ```_CrtMemState```结构，可以在任意点转储该结构的内容：
{% highlight string %}
_CrtMemDumpStatistics( &s1 );
{% endhighlight %}

若要确定代码中某一部分是否发生了内存泄漏，可以在该部分之前和之后对内存状态拍快照，然后使用```_CrtMemDifference```比较这两个状态:
{% highlight string %}
_CrtMemCheckpoint( &s1 );
// memory allocations take place here
_CrtMemCheckpoint( &s2 );
 
if ( _CrtMemDifference( &s3, &s1, &s2) )
   _CrtMemDumpStatistics( &s3 );
{% endhighlight %}

顾名思义，**_CrtMemDifference()**比较两个内存状态（```s1```和```s2```），生成这两个状态之间差异的结果（s3）。 在程序的开始和结尾放置**_CrtMemCheckpoint()**调用，并使用**_CrtMemDifference()**比较结果，是检查内存泄漏的另一种方法。 如果检测到泄漏，则可以使用 _CrtMemCheckpoint()调用通过二进制搜索技术来划分程序和定位泄漏。

如上面的例子程序我们可以这样来定位确切的调用GetMemory的地方：
{% highlight string %}
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
 
#include <iostream>
using namespace std;
 
_CrtMemState s1, s2, s3;
 
void GetMemory(char *p, int num)
{
    p = (char*)malloc(sizeof(char) * num);
}
 
int main(int argc,char** argv)
{
    _CrtMemCheckpoint( &s1 );
    char *str = NULL;
    GetMemory(str, 100);
    _CrtMemCheckpoint( &s2 );
    if ( _CrtMemDifference( &s3, &s1, &s2) )
        _CrtMemDumpStatistics( &s3 );
    cout<<"Memory leak test!"<<endl;
    _CrtDumpMemoryLeaks();
    return 0;
}
{% endhighlight %}
调试时，程序输出如下结果：

![memory-leak](https://ivanzz1001.github.io/records/assets/img/cplusplus/memory-leak-figure3.png)

这说明在s1和s2之间存在内存泄漏！！！如果GetMemory不是在s1和s2之间调用，那么就不会有信息输出.

## 3. Linux平台下的内存泄漏检测

在上面我们介绍了，vs中在代码中包含```crtdbg.h```，将[malloc()](http://msdn.microsoft.com/zh-cn/library/6ewkz86d.aspx)和[free()](http://msdn.microsoft.com/zh-cn/library/we1whae7.aspx)函数映射到它们的调试版本，即[_malloc_dbg()](http://msdn.microsoft.com/zh-cn/library/faz3a37z.aspx)和[_free_dbg()](http://msdn.microsoft.com/zh-cn/library/16swbsbc.aspx)，这两个函数将跟踪内存分配和释放。此映射只在调试版本（在其中定义了```_DEBUG```）中发生。发布版本使用普通的 malloc()和free()函数，即为malloc和free做了钩子，用于记录内存分配信息。

Linux下面也有原理相同的方法——[mtrace](http://en.wikipedia.org/wiki/Mtrace)。方法类似，我这就不具体描述，参加给出的链接。这节我主要介绍一个非常强大的工具valgrind。如下图所示：

![memory-leak](https://ivanzz1001.github.io/records/assets/img/cplusplus/memory-leak-figure4.png)

如上图所示知道：
<pre>
==6118== 100 bytes in 1 blocks are definitely lost in loss record 1 of 1
==6118==    at 0x4024F20: malloc (vg_replace_malloc.c:236)
==6118==    by 0x8048724: GetMemory(char*, int) (in /home/netsky/workspace/a.out)
==6118==    by 0x804874E: main (in /home/netsky/workspace/a.out)
</pre>

是在main中调用了GetMemory()导致的内存泄漏，GetMemory()中是调用了malloc导致泄漏了100字节的内存。

>Things to notice:
>
>• There is a lot of information in each error message; read it carefully.
>
>• The 6118 is the process ID; it’s usually unimportant.
>
>• The ﬁrst line ("Heap Summary") tells you what kind of error it is.
>
>• Below the ﬁrst line is a stack trace telling you where the problem occurred. Stack traces can get quite large, and be
confusing, especially if you are using the C++ STL. Reading them from the bottom up can help.
>
>• The code addresses (eg. 0x4024F20) are usually unimportant, but occasionally crucial for tracking down weirder bugs.
>
>The stack trace tells you where the leaked memory was allocated. Memcheck cannot tell you why the memory leaked,unfortunately. (Ignore the "vg_replace_malloc.c", that’s an implementation detail.)
>
>There are several kinds of leaks; the two most important categories are:
>
>• "deﬁnitely lost": your program is leaking memory -- ﬁx it!
>
>• "probably lost": your program is leaking memory, unless you’re doing funny things with pointers (such as moving them to point to the middle of a heap block)

Valgrind的使用请见手册[http://valgrind.org/docs/manual/manual.html](http://valgrind.org/docs/manual/manual.html)。


## 4. 总结
其实内存泄漏的原因可以概括为：调用了malloc/new等内存申请的操作，但缺少了对应的free/delete，总之就是，malloc/new比free/delete的数量多。我们在编程时需要注意这点，保证每个malloc都有对应的free，每个new都有对应的deleted！！！平时要养成这样一个好的习惯。

要避免内存泄漏可以总结为以下几点：

* 程序员要养成良好习惯，保证malloc/new和free/delete匹配；

* 检测内存泄漏的关键原理就是，检查malloc/new和free/delete是否匹配，一些工具也就是这个原理。要做到这点，就是利用宏或者钩子，在用户程序与运行库之间加了一层，用于记录内存分配情况。




<br />
<br />

**[参考]**

1. [C/C++内存泄漏及检测](https://www.cnblogs.com/skynet/archive/2011/02/20/1959162.html)



<br />
<br />
<br />





