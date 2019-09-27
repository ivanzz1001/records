---
layout: post
title: win-console控制台编辑模式
tags:
- windows
categories: windows
description: win-console控制台编辑模式
---


本文简要讲述一下windows-console程序的控制台编辑模式，在程序调试时容易出现的问题。



<!-- more -->

## 1. 问题描述及原因
问题： win10下编写Windows下控制台程序时发现程序经常被阻塞。

原因： windows Powershell分快速编辑模式和标准模式。当处于快速编辑模式时，鼠标点击控制台会导致程序被阻塞。且系统默认情况下是```快速编辑模式```。

![win10-console](https://ivanzz1001.github.io/records/assets/img/windows/win10-console.png)

解决： 通过代码重置控制台的属性
{% highlight string %}
HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);  
DWORD mode;  
GetConsoleMode(hStdin, &mode);  
mode &= ~ENABLE_QUICK_EDIT_MODE;         //移除快速编辑模式
mode &= ~ENABLE_INSERT_MODE;             //移除插入模式
mode &= ~ENABLE_MOUSE_INPUT;
SetConsoleMode(hStdin, mode); 
{% endhighlight %}

重置之后如下所示：

![win10-console-reset](https://ivanzz1001.github.io/records/assets/img/windows/win10-console-reset.png)


<br />
<br />
**[参看]:**

1. [c++ windows console 快速编辑模式 关闭](https://blog.csdn.net/o_longzhong/article/details/80276645)

2. [Win10中控制台程序输出阻塞问题](https://blog.csdn.net/qingyang8513/article/details/88865955)
<br />
<br />
<br />





