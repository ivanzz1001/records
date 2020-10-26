---
layout: post
title: golang调试工具delve使用
tags:
- go-language
categories: go-language
description: delve工具的介绍及使用
---


本文我们介绍一下golang中的调试工具delve的安装及使用.


<!-- more -->

## 1. delve工具的安装
Delve是一个Go语言调试器。如下我们介绍一下delve在Linux上的安装。

###### 1.1 安装方式1

采用如下命令下载delve源代码：
<pre>
# go get -u github.com/derekparker/delve/cmd/dlv
# cd src/github.com/
# ls
derekparker  go-delve
</pre>

>注： 如果我们是采用go modules模式来执行该命令的话，应该在module目录外执行

然后进入```go-delve```目录，执行如下命令安装：
<pre>
# cd go-delve/delve/cmd/dlv/
# ls
cmds  dlv_test.go  main.go  tools.go
# go build
# ls
cmds  dlv  dlv_test.go  main.go  tools.go 
</pre>
上述```dlv```即是我们编译出来的可执行程序：
<pre>
#  ./dlv --help
Delve is a source level debugger for Go programs.

Delve enables you to interact with your program by controlling the execution of the process,
evaluating variables, and providing information of thread / goroutine state, CPU register state and more.

The goal of this tool is to provide a simple yet powerful interface for debugging Go programs.

Pass flags to the program you are debugging using `--`, for example:

`dlv exec ./hello -- server --config conf/config.toml`

...
</pre>

###### 1.2 安装方式2
执行如下命令下载delve源代码：
<pre>
# git clone https://github.com/go-delve/delve.git $GOPATH/src/github.com/go-delve/delve
</pre>
>注： 这里注意需要先设置好```GOPATH```。

然后执行如下命令编译安装：
<pre>
# cd $GOPATH/src/github.com/go-delve/delve
# ls
assets  CHANGELOG.md  cmd  CONTRIBUTING.md  Documentation  _fixtures  go.mod  go.sum  ISSUE_TEMPLATE.md  LICENSE  Makefile  pkg  README.md  _scripts  service  vendor
# make
# make install
# ls
assets  CHANGELOG.md  cmd  CONTRIBUTING.md  dlv  Documentation  _fixtures  go.mod  go.sum  ISSUE_TEMPLATE.md  LICENSE  Makefile  pkg  README.md  _scripts  service  vendor
</pre>
可以看到编译完成后，在当前目录下生成了一个```dlv```文件。

## 2. delve常用命令

1） **attach到进程id**
{% highlight string %}
# dlv attach <pid>
Type 'help' for list of commands.
(dlv) help
{% endhighlight %}



2) **打印所有的goroutines列表**
{% highlight string %}
(dlv) goroutines
[10 goroutines]
  Goroutine 1 - User: /usr/local/go/src/syscall/asm_linux_amd64.s:27 syscall.Syscall (0x466e70) (thread 14023)
  Goroutine 2 - User: /usr/local/go/src/runtime/proc.go:292 runtime.gopark (0x42800a)
  Goroutine 3 - User: /usr/local/go/src/runtime/proc.go:292 runtime.gopark (0x42800a)
  Goroutine 17 - User: /usr/local/go/src/runtime/proc.go:292 runtime.gopark (0x42800a)
  Goroutine 18 - User: /usr/local/go/src/runtime/sigqueue.go:139 os/signal.signal_recv (0x43b5b6)
  Goroutine 19 - User: /usr/local/go/src/runtime/proc.go:292 runtime.gopark (0x42800a)
  Goroutine 20 - User: ./main.go:43 main.setupSignalHandler.func1 (0x49318c)
  Goroutine 21 - User: /usr/local/go/src/runtime/time.go:102 time.Sleep (0x442ac6)
  Goroutine 22 - User: /usr/local/go/src/runtime/time.go:102 time.Sleep (0x442ac6)
  Goroutine 23 - User: /usr/local/go/src/runtime/lock_futex.go:227 runtime.notetsleepg (0x40c9a2)
{% endhighlight %}

3) **切换当前goroutine**
{% highlight string %}
(dlv) goroutine 6
Switched from 1 to 6 (thread 9022)
{% endhighlight %}

4) **查看当前goroutine的调用栈**
{% highlight string %}
(dlv) bt
 0  0x0000000000454730 in runtime.systemstack_switch
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/asm_amd64.s:245
 1  0x000000000040f700 in runtime.mallocgc
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/malloc.go:643
 2  0x000000000040fc43 in runtime.rawmem
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/malloc.go:809
 3  0x000000000043c2a5 in runtime.growslice
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/slice.go:95
 4  0x000000000043c015 in runtime.growslice_n
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/slice.go:44
 5  0x0000000000459545 in fmt.(*fmt).padString
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/format.go:130
 6  0x000000000045a13f in fmt.(*fmt).fmt_s
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/format.go:322
 7  0x000000000045e905 in fmt.(*pp).fmtString
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:518
 8  0x000000000046200f in fmt.(*pp).printArg
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:797
 9  0x0000000000468a8d in fmt.(*pp).doPrintf
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:1238
10  0x000000000045c654 in fmt.Fprintf
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:188
{% endhighlight %}
此时输出了10层调用栈，但似乎最原始的我自身程序dbgTest.go的调用栈没有输出， 可以通过bt加depth参数，设定bt的输出深度，进而找到我们自己的调用栈，例如bt 13:
{% highlight string %}
(dlv) bt 13
...
10  0x000000000045c654 in fmt.Fprintf
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:188
11  0x000000000045c74b in fmt.Printf
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/fmt/print.go:197
12  0x000000000045846f in GoWorks/GoDbg/mylib.RunFunc2
    at ./mylib/dbgTest.go:50
13  0x0000000000456df0 in runtime.goexit
    at /home/lday/Tools/Dev_Tools/Go_Tools/go_1_6_2/src/runtime/asm_amd64.s:1998
{% endhighlight %}
我们看到，我们自己dbgTest.go的调用栈在第12层。当前goroutine已经不再我们自己的调用栈上，而是进入到系统函数的调用中，在这种情况下，使用gdb进行调试时，我们发现，此时我们没有很好的方法能够输出我们需要的调用栈变量信息。dlv可以！ 此时只需简单的通过```frame x cmd```就可以输出我们想要的调用栈信息了
{% highlight string %}
(dlv) frame 12 ls
    45:        time.Sleep(10 * time.Second)
    46:        waiter.Done()
    47:    }
    48:    
    49:    func RunFunc2(variable string, waiter *sync.WaitGroup) {
=>  50:        fmt.Printf("var2:%v\n", variable)
    51:        time.Sleep(10 * time.Second)
    52:        waiter.Done()
    53:    }
    54:    
    55:    func RunFunc3(pVariable *[]int, waiter *sync.WaitGroup) {
(dlv) frame 12 print variable 
"golang dbg test"
(dlv) frame 12 print waiter
*sync.WaitGroup {
    state1: [12]uint8 [0,0,0,0,2,0,0,0,0,0,0,0],
    sema: 0,}
{% endhighlight %}


5) **打印单个goroutine的stack**
{% highlight string %}
(dlv) goroutine <goroutine_id> stack
 0  0x0000000000466e70 in syscall.Syscall
    at /usr/local/go/src/syscall/asm_linux_amd64.s:27
 1  0x000000000046692f in syscall.read
    at /usr/local/go/src/syscall/zsyscall_linux_amd64.go:749
 2  0x0000000000466449 in syscall.Read
    at /usr/local/go/src/syscall/syscall_unix.go:162
 3  0x0000000000468798 in internal/poll.(*FD).Read
    at /usr/local/go/src/internal/poll/fd_unix.go:153
 4  0x0000000000469b8e in os.(*File).read
    at /usr/local/go/src/os/file_unix.go:226
 5  0x0000000000468f7a in os.(*File).Read
    at /usr/local/go/src/os/file.go:107
 6  0x00000000004658a6 in io.ReadAtLeast
    at /usr/local/go/src/io/io.go:309
 7  0x0000000000465a18 in io.ReadFull
    at /usr/local/go/src/io/io.go:327
...
{% endhighlight %}

6) **打印所有goroutine的stack**
{% highlight string %}
(dlv) goroutines -t
[10 goroutines]
  Goroutine 1 - User: /usr/local/go/src/syscall/asm_linux_amd64.s:27 syscall.Syscall (0x466e70) (thread 14023)
         0  0x0000000000466e70 in syscall.Syscall
             at /usr/local/go/src/syscall/asm_linux_amd64.s:27
         1  0x000000000046692f in syscall.read
             at /usr/local/go/src/syscall/zsyscall_linux_amd64.go:749
         2  0x0000000000466449 in syscall.Read
             at /usr/local/go/src/syscall/syscall_unix.go:162
          ...
        (truncated)
  Goroutine 2 - User: /usr/local/go/src/runtime/proc.go:292 runtime.gopark (0x42800a)
        0  0x000000000042800a in runtime.gopark
            at /usr/local/go/src/runtime/proc.go:292
        1  0x00000000004280be in runtime.goparkunlock
            at /usr/local/go/src/runtime/proc.go:297
        2  0x0000000000427e4c in runtime.forcegchelper
            at /usr/local/go/src/runtime/proc.go:248
        3  0x0000000000451671 in runtime.goexit
            at /usr/local/go/src/runtime/asm_amd64.s:2361
...
{% endhighlight %}

<br />
<br />
**[参看]：**

1. [delve官网](https://github.com/derekparker/delve)

2. [delve：Golang的最佳调试工具](https://studygolang.com/articles/19149?fr=sidebar)

3. [golang调试工具delve](https://studygolang.com/articles/16244)

4. [golang调试工具Delve](https://blog.csdn.net/mi_duo/article/details/100352597)

<br />
<br />
<br />

