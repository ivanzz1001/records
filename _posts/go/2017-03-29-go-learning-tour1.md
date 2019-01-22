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




<br />
<br />
**[参看]：**



<br />
<br />
<br />

