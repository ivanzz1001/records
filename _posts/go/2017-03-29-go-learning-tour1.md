---
layout: post
title: golang学习之旅
tags:
- go-language
categories: go-language
description: golang学习之旅
---


本文简单记录一下golang的学习过程，以备后续查阅。当前go开发环境的基本情况如下：
<pre>
C:\Users\Administrator>echo %GOPATH%
E:\Workspace\go

C:\Users\Administrator>go version
go version go1.8 windows/amd64

C:\Users\Administrator>echo %GOARCH%
amd64
</pre>
在```%GOPATH%```目录下新建```src```目录用于存放源文件。


<!-- more -->

## 1. Golang之helloworld
在```src```目录下新建**helloworld**工程（文件夹），在里面创建**helloworld.go**:
{% highlight string %}
package main

import "fmt"

func main(){
	fmt.Println("hello,world")
}
{% endhighlight %}
进入```%GOPATH%```路径，执行如下命令编译并同时运行程序：
{% highlight string %}
E:\Workspace\go>go run src/helloworld/helloworld.go
hello,world
{% endhighlight %}
我们也可以把**编译**和**运行**两个步骤分开：
{% highlight string %}
E:\Workspace\go>go build src/helloworld/helloworld.go

E:\Workspace\go>.\helloworld
hello,world
{% endhighlight %}

上述代码中，**package main**比较特殊，其用于定义一个可单独执行的程序，而不是一个lib库。在package main中的**main()**函数用于定义可执行程序的入口点。



### 1.1 ```go get```命令获取远程代码
我们可以使用```go get```命令来从远程下载golang源代码到```%GOPATH%/src```目录下。例如：
<pre>
# go get gopl.io/ch1/helloworld
</pre>

### 1.2 golang语法说明

Golang在每一条**语句**(statement)或**声明**(declaration)的末尾并不需要添加一个```;```，除非在同一行上有多条语句或声明。例如：
{% highlight string %}
func main(){
	fmt.Printf("First Line\n"); fmt.Printf("Second Line\n")
}
{% endhighlight %}
事实上，处于**语句**或**声明**末尾的```newlines```都会被转换成semicolons，因此```newlines```所在的位置会影响到Golang代码的解析。例如，一个函数的左括号```{```必须与该函数处于同一行； 而对于表达式```x+y```，```newlines```只能出现在```+```号后面。

### 1.3 ```go doc```命令
我们可以使用**go doc**命令来查看go标准库中某个函数或全局变量的帮助信息。例如：
{% highlight string %}
E:\Workspace\go>go doc os.Args
var Args []string
    Args hold the command-line arguments, starting with the program name.


E:\Workspace\go>go doc fmt.Printf
func Printf(format string, a ...interface{}) (n int, err error)
    Printf formats according to a format specifier and writes to standard
    output. It returns the number of bytes written and any write error
    encountered.
{% endhighlight %}


## 2. Program Structure

本节我们会简单介绍一下Golang中的基本数据类型，赋值，类型声明等。

### 2.1 Names
golang的命名规范与C语言类似，这里不进行细说。golang中有25个关键字：
<pre>
break     default       func    interface    select
case      defer         go      map          struct
chan      else          goto    package      switch
const     fallthrough   if      range        type
continue  for           import  return       var
</pre>
另外，还有如下三种系统预定义的```names```:
<pre>
Constants:  true  false  iota  nil

Types:      int  int8  int16  int32  int64
            uint  uint8  uint16  uint32  uint64  uintptr
            float32  float64  complex128  complex64
            bool  byte  rune  string  error

Functions:  make  len  cap  new  append  copy  close  delete
            complex  real  imag
            panic  recover
</pre>




<br />
<br />
**[参看]：**



<br />
<br />
<br />

