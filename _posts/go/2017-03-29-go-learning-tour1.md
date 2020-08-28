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



## 5. 反射（Reflection)

### 5.1 Reflect.Type and Reflect.Value
```Reflection```是由reflect包所提供，在其中定义了两个重要的类型。其中```Type```代表着Go type，其是一个interface，拥有很多method用于辨识```Type```:
{% highlight string %}
// Type is the representation of a Go type.
//
// Not all methods apply to all kinds of types. Restrictions,
// if any, are noted in the documentation for each method.
// Use the Kind method to find out the kind of type before
// calling kind-specific methods. Calling a method
// inappropriate to the kind of type causes a run-time panic.
//
// Type values are comparable, such as with the == operator.
// Two Type values are equal if they represent identical types.
type Type interface {
	// Methods applicable to all types.

	// Align returns the alignment in bytes of a value of
	// this type when allocated in memory.
	Align() int

	// FieldAlign returns the alignment in bytes of a value of
	// this type when used as a field in a struct.
	FieldAlign() int

	// Method returns the i'th method in the type's method set.
	// It panics if i is not in the range [0, NumMethod()).
	//
	// For a non-interface type T or *T, the returned Method's Type and Func
	// fields describe a function whose first argument is the receiver.
	//
	// For an interface type, the returned Method's Type field gives the
	// method signature, without a receiver, and the Func field is nil.
	Method(int) Method

	// MethodByName returns the method with that name in the type's
	// method set and a boolean indicating if the method was found.
	//
	// For a non-interface type T or *T, the returned Method's Type and Func
	// fields describe a function whose first argument is the receiver.
	//
	// For an interface type, the returned Method's Type field gives the
	// method signature, without a receiver, and the Func field is nil.
	MethodByName(string) (Method, bool)

	// NumMethod returns the number of exported methods in the type's method set.
	NumMethod() int

	// Name returns the type's name within its package.
	// It returns an empty string for unnamed types.
	Name() string

	// PkgPath returns a named type's package path, that is, the import path
	// that uniquely identifies the package, such as "encoding/base64".
	// If the type was predeclared (string, error) or unnamed (*T, struct{}, []int),
	// the package path will be the empty string.
	PkgPath() string

	// Size returns the number of bytes needed to store
	// a value of the given type; it is analogous to unsafe.Sizeof.
	Size() uintptr

	// String returns a string representation of the type.
	// The string representation may use shortened package names
	// (e.g., base64 instead of "encoding/base64") and is not
	// guaranteed to be unique among types. To test for type identity,
	// compare the Types directly.
	String() string

	// Kind returns the specific kind of this type.
	Kind() Kind

	// Implements reports whether the type implements the interface type u.
	Implements(u Type) bool

	// AssignableTo reports whether a value of the type is assignable to type u.
	AssignableTo(u Type) bool

	// ConvertibleTo reports whether a value of the type is convertible to type u.
	ConvertibleTo(u Type) bool

	// Comparable reports whether values of this type are comparable.
	Comparable() bool

	// Methods applicable only to some types, depending on Kind.
	// The methods allowed for each kind are:
	//
	//	Int*, Uint*, Float*, Complex*: Bits
	//	Array: Elem, Len
	//	Chan: ChanDir, Elem
	//	Func: In, NumIn, Out, NumOut, IsVariadic.
	//	Map: Key, Elem
	//	Ptr: Elem
	//	Slice: Elem
	//	Struct: Field, FieldByIndex, FieldByName, FieldByNameFunc, NumField

	// Bits returns the size of the type in bits.
	// It panics if the type's Kind is not one of the
	// sized or unsized Int, Uint, Float, or Complex kinds.
	Bits() int

	// ChanDir returns a channel type's direction.
	// It panics if the type's Kind is not Chan.
	ChanDir() ChanDir

	// IsVariadic reports whether a function type's final input parameter
	// is a "..." parameter. If so, t.In(t.NumIn() - 1) returns the parameter's
	// implicit actual type []T.
	//
	// For concreteness, if t represents func(x int, y ... float64), then
	//
	//	t.NumIn() == 2
	//	t.In(0) is the reflect.Type for "int"
	//	t.In(1) is the reflect.Type for "[]float64"
	//	t.IsVariadic() == true
	//
	// IsVariadic panics if the type's Kind is not Func.
	IsVariadic() bool

	// Elem returns a type's element type.
	// It panics if the type's Kind is not Array, Chan, Map, Ptr, or Slice.
	Elem() Type

	// Field returns a struct type's i'th field.
	// It panics if the type's Kind is not Struct.
	// It panics if i is not in the range [0, NumField()).
	Field(i int) StructField

	// FieldByIndex returns the nested field corresponding
	// to the index sequence. It is equivalent to calling Field
	// successively for each index i.
	// It panics if the type's Kind is not Struct.
	FieldByIndex(index []int) StructField

	// FieldByName returns the struct field with the given name
	// and a boolean indicating if the field was found.
	FieldByName(name string) (StructField, bool)

	// FieldByNameFunc returns the struct field with a name
	// that satisfies the match function and a boolean indicating if
	// the field was found.
	//
	// FieldByNameFunc considers the fields in the struct itself
	// and then the fields in any anonymous structs, in breadth first order,
	// stopping at the shallowest nesting depth containing one or more
	// fields satisfying the match function. If multiple fields at that depth
	// satisfy the match function, they cancel each other
	// and FieldByNameFunc returns no match.
	// This behavior mirrors Go's handling of name lookup in
	// structs containing anonymous fields.
	FieldByNameFunc(match func(string) bool) (StructField, bool)

	// In returns the type of a function type's i'th input parameter.
	// It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumIn()).
	In(i int) Type

	// Key returns a map type's key type.
	// It panics if the type's Kind is not Map.
	Key() Type

	// Len returns an array type's length.
	// It panics if the type's Kind is not Array.
	Len() int

	// NumField returns a struct type's field count.
	// It panics if the type's Kind is not Struct.
	NumField() int

	// NumIn returns a function type's input parameter count.
	// It panics if the type's Kind is not Func.
	NumIn() int

	// NumOut returns a function type's output parameter count.
	// It panics if the type's Kind is not Func.
	NumOut() int

	// Out returns the type of a function type's i'th output parameter.
	// It panics if the type's Kind is not Func.
	// It panics if i is not in the range [0, NumOut()).
	Out(i int) Type

	common() *rtype
	uncommon() *uncommonType
}
{% endhighlight %}

reflect包中另一个很有用的类型就是```Value```。reflect.Value可以容纳任何类型的值：
{% highlight string %}
// Value is the reflection interface to a Go value.
//
// Not all methods apply to all kinds of values. Restrictions,
// if any, are noted in the documentation for each method.
// Use the Kind method to find out the kind of value before
// calling kind-specific methods. Calling a method
// inappropriate to the kind of type causes a run time panic.
//
// The zero Value represents no value.
// Its IsValid method returns false, its Kind method returns Invalid,
// its String method returns "<invalid Value>", and all other methods panic.
// Most functions and methods never return an invalid value.
// If one does, its documentation states the conditions explicitly.
//
// A Value can be used concurrently by multiple goroutines provided that
// the underlying Go value can be used concurrently for the equivalent
// direct operations.
//
// Using == on two Values does not compare the underlying values
// they represent, but rather the contents of the Value structs.
// To compare two Values, compare the results of the Interface method.
type Value struct {
	// typ holds the type of the value represented by a Value.
	typ *rtype

	// Pointer-valued data or, if flagIndir is set, pointer to data.
	// Valid when either flagIndir is set or typ.pointers() is true.
	ptr unsafe.Pointer

	// flag holds metadata about the value.
	// The lowest bits are flag bits:
	//	- flagStickyRO: obtained via unexported not embedded field, so read-only
	//	- flagEmbedRO: obtained via unexported embedded field, so read-only
	//	- flagIndir: val holds a pointer to the data
	//	- flagAddr: v.CanAddr is true (implies flagIndir)
	//	- flagMethod: v is a method value.
	// The next five bits give the Kind of the value.
	// This repeats typ.Kind() except for method values.
	// The remaining 23+ bits give a method number for method values.
	// If flag.kind() != Func, code can assume that flagMethod is unset.
	// If ifaceIndir(typ), code can assume that flagIndir is set.
	flag

	// A method value represents a curried method invocation
	// like r.Read for some receiver r. The typ+val+flag bits describe
	// the receiver r, but the flag's Kind bits say Func (methods are
	// functions), and the top bits of the flag give the method number
	// in r's type's method table.
}

{% endhighlight %}

说明: reflect.ValueOf()与reflect.Value.Interface()刚好是一个相反操作， reflect.Value.Interface()返回interface{}类型，其容纳了reflect.Value的具体值。例如，
<pre>
v := reflect.ValueOf(3)      // a reflect.Value
x := v.Interface()          // an interface{}
i := x.(int)               // an int
fmt.Printf("%d\n", i)     // "3"
</pre>

reflect.Value与interface{}都可以容纳任何随机值。它们之间的不同在于interface{}隐藏了一个```值```(value)的```表示```(representation)与固有操作，并且没有暴露任何的方法，因此除非你知道其本来的具体类型，然后使用一个强制类型转换来操作，否则我们对该interface{}可能会束手无策。相反，对于reflect.Value则暴露了很多方法，我们可以通过这些方法了解到很多具体信息。


下面给出一个```Reflection```内存模型的基本示意图：

![go-reflection](https://ivanzz1001.github.io/records/assets/img/go/go_reflection.jpg)


## 6. 示例
### 6.1 一个任务队列的实现
{% highlight string %}
package queue

import (
	"container/list"
	"sync"
)

type MessageQueue struct{
	buffer *list.List
	lock    sync.Mutex
	popable *sync.Cond
	running bool
}


func CreateMsgQueue() *MessageQueue{
	msgQueue := &MessageQueue{
		buffer: list.New(),
		running: true,
	}
	msgQueue.popable = sync.NewCond(&msgQueue.lock)
	return msgQueue
}

func (queue *MessageQueue)Close(){
	queue.lock.Lock()
	queue.running = false
	queue.popable.Broadcast()
	queue.lock.Unlock()
}

func (queue *MessageQueue)IsRunning() bool{
	return queue.running
}

func (queue *MessageQueue)Pop()(*interface{}, int){
	length := 0

	queue.lock.Lock()
	for queue.buffer.Len() == 0{
		if queue.running{
			queue.popable.Wait()
		}else{
			queue.lock.Unlock()
			return nil, 0
		}

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
	"strconv"
	"time"
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
		outRealWork, ok := (*outwork1).(Task_Work)
		if !ok{
			fmt.Printf("not expected type\n")
			return
		}
		fmt.Printf("name: %s content: %s\n", outRealWork.name, outRealWork.content)

	}

	outwork2, ok := taskQueue.TryPop()
	if ok{
		outRealWork2, ok := outwork2.(Task_Work)
		if !ok{
			fmt.Printf("not expected type\n")
			return
		}
		fmt.Printf("name: %s content: %s\n", outRealWork2.name, outRealWork2.content)
	}
}

func TestMessageQueue(t *testing.T){
	type internalMessage struct {
		msgID int64
		msgContent string
	}

	//wg := sync.WaitGroup{}
	msgQueue := CreateMsgQueue()

	//wg.Add(1)
	go func(){
		for i:=0;i<5000;i++{
			internalMsg := internalMessage{
				msgID: int64(i+1),
				msgContent: "this is message #" + strconv.Itoa(i+1),
			}

			msgQueue.Push(internalMsg)
			if (i + 1) % 100 == 0{
				time.Sleep(time.Millisecond * 300)
			}
		}

		msgQueue.Close()
		//wg.Done()
	}()

	for{
		msg, _:= msgQueue.Pop()
		if msg != nil{
			internalMsg, ok:= (*msg).(internalMessage)
			if !ok{
				fmt.Printf("[TestMessageQueue] unexpected message type\n")
				continue
			}
			fmt.Printf("msgID: %d msgContent: %s\n", internalMsg.msgID, internalMsg.msgContent)
		}else if msgQueue.IsRunning() == false{
			fmt.Printf("[TestMessageQueue] message queue is not running now, exit the loop\n")
			break
		}
	}

	fmt.Printf("[TestMessageQueue] exit message queue test\n")
}
{% endhighlight %}

### 6.2 带限速功能的队列
{% highlight string %}
package service

import (
	"container/list"
	"sync"
	"time"
	"common/logging"
)

/*
 * Description: Here we realize a throttle queue, which will limit the pop rate in a period(4s)
 */
const(
	MAX_THROTTLE_CNT = 15
)

type ThrottleLimit func()int64

type ThrottleQueue struct{
	buffer *list.List
	lock    sync.Mutex
	popable *sync.Cond
	running bool

	tick int64
	throttle [MAX_THROTTLE_CNT]int64
	exitC chan struct{}
	wg sync.WaitGroup

	limitCallback ThrottleLimit
}


func CreateThrottleQueue(tlimit ThrottleLimit) *ThrottleQueue{
	queue := &ThrottleQueue{
		buffer: list.New(),
		running: true,
		tick: 0x0,
		exitC: make(chan struct{}),
		limitCallback: tlimit,
	}
	queue.popable = sync.NewCond(&queue.lock)

	queue.throttle[queue.tick % MAX_THROTTLE_CNT] = queue.limitCallback()
	go queue.throttlePump()

	return queue
}

func (queue *ThrottleQueue)Close(){
	queue.lock.Lock()
	queue.running = false
	queue.popable.Broadcast()
	queue.lock.Unlock()
	queue.exitC <- struct{}{}

	queue.wg.Wait()
}

func (queue *ThrottleQueue)throttlePump(){
	queue.wg.Add(1)

	ticker := time.NewTicker(time.Second * 4)
	defer func(){
		ticker.Stop()
		queue.wg.Done()
	}()

	Loop:
		for{
			select{
			case <-ticker.C:
				limit := queue.limitCallback()
				if limit <= 0{
					logging.Warning("[throttlePump] current limit throttle is %d", limit)
				}
				queue.lock.Lock()
				lastTick := queue.tick
				queue.tick++
				queue.throttle[queue.tick % MAX_THROTTLE_CNT] = limit
				if queue.buffer.Len() > 0 && queue.throttle[lastTick % MAX_THROTTLE_CNT] <= 0{
					//queue.popable.Signal()
					queue.popable.Broadcast()     //most probably we should wakeup all
				}
				queue.lock.Unlock()
			case <-queue.exitC:
				break Loop
			}
		}
}
func (queue *ThrottleQueue)IsRunning() bool{
	return queue.running
}

func (queue *ThrottleQueue)Pop()(*interface{}, int){
	length := 0

	queue.lock.Lock()
	for queue.buffer.Len() == 0 || queue.throttle[queue.tick % MAX_THROTTLE_CNT] <= 0{
		if queue.running{
			queue.popable.Wait()
		}else{
			queue.lock.Unlock()
			return nil, 0
		}

	}
	queue.throttle[queue.tick % MAX_THROTTLE_CNT]--

	element := queue.buffer.Front()
	task := element.Value
	queue.buffer.Remove(element)
	length = queue.buffer.Len()

	queue.lock.Unlock()

	return &task, length
}


func (queue *ThrottleQueue)TryPop()(* interface{}, bool){
	queue.lock.Lock()

	if queue.buffer.Len() > 0 && queue.throttle[queue.tick % MAX_THROTTLE_CNT] > 0{
		queue.throttle[queue.tick % MAX_THROTTLE_CNT]--

		element := queue.buffer.Front()
		task := element.Value
		queue.buffer.Remove(element)

		queue.lock.Unlock()
		return &task, true
	}

	queue.lock.Unlock()
	return nil, false
}

/*
 * returns current message count
 */
func (queue *ThrottleQueue)Push(task interface{})int{
	length := 0

	queue.lock.Lock()
	if queue.buffer.Len() == 0 && queue.throttle[queue.tick % MAX_THROTTLE_CNT] > 0{
		queue.buffer.PushBack(task)
		queue.popable.Signal()
	}else{
		queue.buffer.PushBack(task)
	}
	length = queue.buffer.Len()
	queue.lock.Unlock()

	return length
}



func (queue *ThrottleQueue)PeakAll()([] *interface{}){
	result := []*interface{}{}

	queue.lock.Lock()

	for e := queue.buffer.Front(); e != nil; e = e.Next(){
		task := &e.Value
		result = append(result, task)
	}
	queue.lock.Unlock()
	return result
}

func (queue *ThrottleQueue)Length()int{
	length := 0

	queue.lock.Lock()
	length = queue.buffer.Len()
	queue.lock.Unlock()

	return length
}

func (queue *MessageQueue)Clear()bool{
	queue.lock.Lock()

	for e := queue.buffer.Front(); e != nil; {
		next := e.Next()
		queue.buffer.Remove(e)
		e = next
	}

	queue.lock.Unlock()

	return true
}

type MQClearCallback func(interface{}) bool
func (queue *MessageQueue)WithCallbackClear(callback MQClearCallback) bool{
	queue.lock.Lock()

	for e := queue.buffer.Front(); e != nil; {
		callback(e.Value)
		next := e.Next()
		queue.buffer.Remove(e)
		e = next
	}

	queue.lock.Unlock()

	return true
}
{% endhighlight %}

测试示例：
{% highlight string %}
package service

import (
	"testing"
	"strconv"
	"time"
	"fmt"
	"sync"
)

type testInternalMessage struct{
	msgID int64
	msgContent string
}

func TestThrottleQueue(t *testing.T){
	throttleQueue := CreateThrottleQueue(func()int64{
		h := time.Now().Hour()
		return 0x0
		if h > 20 || h < 8{
			return 8
		}else{
			return 4
		}
	})


	for i:=0;i<5000;i++{
		internalMsg := testInternalMessage{
			msgID: int64(i+1),
			msgContent: "this is message #" + strconv.Itoa(i+1),
		}

		throttleQueue.Push(internalMsg)
		if (i + 1) % 100 == 0{
			time.Sleep(time.Millisecond * 100)
		}
	}

	consumeMsgCnt := 0
	for{
		msg, _:= throttleQueue.Pop()
		consumeMsgCnt++
		if msg != nil{
			internalMsg, ok:= (*msg).(testInternalMessage)
			if !ok{
				fmt.Printf("[TestMessageQueue] unexpected message type\n")
			}else{
				fmt.Printf("msgID: %d msgContent: %s\n", internalMsg.msgID, internalMsg.msgContent)
			}
		}else if throttleQueue.IsRunning() == false{
			fmt.Printf("[TestMessageQueue] message queue is not running now, exit the loop\n")
			break
		}

		if consumeMsgCnt == 5000{
			fmt.Printf("popped out all the message, exit\n")
			break
		}
	}

	throttleQueue.Close()
}
func produceMsg(throttleQueue *ThrottleQueue, exitC chan <- struct{}){
	for i:=0;i<5000;i++{
		internalMsg := testInternalMessage{
			msgID: int64(i+1),
			msgContent: "this is message #" + strconv.Itoa(i+1),
		}

		throttleQueue.Push(internalMsg)
		if (i + 1) % 100 == 0{
			time.Sleep(time.Millisecond * 800)
		}
	}

	exitC <- struct{}{}

}
func consumeMsg(throttleQueue *ThrottleQueue, wg *sync.WaitGroup){
	wg.Add(1)
	defer wg.Done()

	for{
		msg, _:= throttleQueue.Pop()
		if msg != nil{
			internalMsg, ok:= (*msg).(testInternalMessage)
			if !ok{
				fmt.Printf("[consumeMsg] unexpected message type\n")
			}else{
				fmt.Printf("msgID: %d msgContent: %s\n", internalMsg.msgID, internalMsg.msgContent)
			}
		}else if throttleQueue.IsRunning() == false{
			fmt.Printf("[consumeMsg] message queue is not running now, exit the loop\n")
			break
		}
	}
}
func TestThrottleQueue2(t *testing.T){
	throttleQueue := CreateThrottleQueue(func()int64{
		h := time.Now().Hour()
		if h > 20 || h < 8{
			return 8
		}else{
			return 4
		}
	})

	wg := sync.WaitGroup{}
	produceFinC := make(chan struct{})

	go produceMsg(throttleQueue, produceFinC)
	for i:=0; i<5;i++{
		go consumeMsg(throttleQueue, &wg)
	}

	select{
		case <-produceFinC:
			fmt.Printf("produce message finished\n")
	}

	throttleQueue.Close()
	wg.Wait()
	fmt.Printf("exit the test function\n")
}

func TestThrottleQueue_Exit(t *testing.T){
	throttleQueue := CreateThrottleQueue(func()int64{
		h := time.Now().Hour()
		if h > 20 || h < 8{
			return 8
		}else{
			return 4
		}
	})

	wg := sync.WaitGroup{}
	go func(){
		wg.Add(1)
		defer wg.Done()
		for{
			msg, _:= throttleQueue.Pop()
			if msg != nil{
				internalMsg, ok:= (*msg).(testInternalMessage)
				if !ok{
					fmt.Printf("[consumeMsg] unexpected message type\n")
				}else{
					fmt.Printf("msgID: %d msgContent: %s\n", internalMsg.msgID, internalMsg.msgContent)
				}
			}else if throttleQueue.IsRunning() == false{
				fmt.Printf("[consumeMsg] message queue is not running now, exit the loop\n")
				break
			}
		}
	}()

	time.Sleep(time.Second * 10)
	throttleQueue.Close()

	wg.Wait()
	fmt.Printf("success exit the throttle queue\n")
}
{% endhighlight %}

<br />
<br />
**[参看]：**



<br />
<br />
<br />

