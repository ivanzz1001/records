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


## 3. 基本数据类型
Golang中数据类型基本可以分为如下四大类：

* basic types： 如numbers、strings、booleans等类型

* aggregate types: 如arrays、structs、slices、maps等类型

* reference types: 如pointers、slices、maps、functions、channels

* interface types

说明：与C语言相比，在golang中有一个新的运算符```&^```(bit clear)，用于清除某一个bit位。另外，对于```^```运算符，若作为**双目**运算符时，其表示```按位异或```； 若作为**单目**运算符时，其表示按位取反。

## 4. Composite Types
本节我们会介绍arrays、structs、slices、maps这4种复合类型。在Golang中，array类型并不是一种```reference```类型, slice也不纯粹是一个```reference```类型，因为当其长度发生改变时，可能会指向一个新的内存空间。

### 4.1 arrays

**1) 数组的定义**
{% highlight string %}

1. 直接定义并默认初始化为0
var a [3]int

2. 直接定义，并赋值
var a [20]int = [20]int{1,2,3}
b := [...]int{1,2,3}
{% endhighlight %} 


### 4.2 slice类型
slice与array类型紧密关联。slice类型的写法为**[]Type**，其由3个组件所组成： pointer、length、capacity。 我们不能够像```array```那样通过```==```来比较两个slice是否相同，而是需要我们自己遍历来进行比较，例如：
{% highlight string %}
func equal(x,y []string) bool{

	if len(x) != len(y){
		return false;
	}

	for i, _ := range x{

		if x[i] != y[i]{
			return false;
		}
	}

	return true;
}
{% endhighlight %}
对于```slice```，我们只能将其与```nil```进行比较，以判断是否为**空**。例如：
<pre>
if summer == nil{ /* ... */ }
</pre>
这里需要指出的是，对于一个```slice```，若其为nil，那么len(slice)为0，cap(slice)也为0； 但是len(slice)为0且cap(slice)也为0的slice并不一定为nil， 比如**[]int{}**或**make([]int,3)[3:]**。对于为nil的slice，其实质是没有指向一个有效的地址空间。例如：
{% highlight string %}
var s []int         // len(s) == 0, s == nil

s = nil             // len(s) == 0, s == nil

s = []int(nil)      // len(s) == 0, s == nil

s = []int{}         // len(s) == 0, s != nil
{% endhighlight %}
因此，假如你需要判断一个slice，其是否有元素，那么使用len(slice)来进行判断。


**1) slice的创建**
<pre>
1. 直接定义
var a []int

2. 直接定义并初始化
var a []int = {1,2,3}

3. 通过make来创建
var a []int = make([]int,3)       //len(a) = cap(a) = 3
var b []int = make([]int,3, 9)    //len(b) = 3, cap(b) = 9

4. 基于数组来创建
var a [5]int
var slice_b []int = a[:]

5. 通过强制装换来创建
var s []int = []int(nil)
var runes []rune = []rune("Hello,世界")
</pre>



## 3. 示例
### 3.1 一个任务队列的实现
{% highlight string %}
package queue

import (
	"container/list"
	"sync"
)

type TaskQueue struct{
	buffer *list.List
	lock    sync.Mutex
	popable *sync.Cond
}


func CreateTaskQueue() *TaskQueue{
	taskQueue := &TaskQueue{
		buffer: list.New(),
	}
	taskQueue.popable = sync.NewCond(&taskQueue.lock)

	return taskQueue
}

func (queue *TaskQueue)Pop()(* interface{}, int){
	length := 0

	queue.lock.Lock()
	for queue.buffer.Len() == 0{
		queue.popable.Wait()
	}
	element := queue.buffer.Front()

	task := element.Value
	queue.buffer.Remove(element)
	length = queue.buffer.Len()

	queue.lock.Unlock()

	return &task, length
}

func (queue *TaskQueue)TryPop()(interface{}, bool){
	queue.lock.Lock()

	if queue.buffer.Len() > 0{
		element := queue.buffer.Front()
		task := element.Value
		queue.buffer.Remove(element)

		queue.lock.Unlock()
		return task, true
	}

	queue.lock.Unlock()
	return nil, false
}

/*
 * returns current message count
 */
func (queue *TaskQueue)Push(task interface{})int{
	length := 0

	queue.lock.Lock()
	if queue.buffer.Len() == 0{
		queue.buffer.PushBack(task)
		queue.popable.Signal()
	}else{
		queue.buffer.PushBack(task)
	}
	length = queue.buffer.Len()
	queue.lock.Unlock()

	return length
}

func (queue *TaskQueue)Length()int{
	length := 0

	queue.lock.Lock()
	length = queue.buffer.Len()
	queue.lock.Unlock()

	return length
}

{% endhighlight %}

测试示例：
{% highlight string %}
package queue

import(
	"testing"
	"fmt"
)

func TestTaskQueuev1(t *testing.T){

	type Task_Work struct{
		name string
		content string
	}

	taskQueue := CreateTaskQueue()
	work1 := Task_Work{
		"work1",
		"work1_content",
	}
	work2 := Task_Work{
		"work2",
		"work2_content",
	}
	taskQueue.Push(work1)
	taskQueue.Push(work2)


	outwork1, _:= taskQueue.Pop()
	if outwork1 != nil{
		outRealWork := (*outwork1).(Task_Work)
		fmt.Printf("name: %s content: %s\n", outRealWork.name, outRealWork.content)

	}

}
{% endhighlight %}


<br />
<br />
**[参看]：**



<br />
<br />
<br />

