---
layout: post
title: go开发环境的搭建
tags:
- go-language
categories: go-language
description: go开发环境的搭建
---

本文主要简单讲述一下go开发环境的搭建，这里做一个简单的记录。

<!-- more -->


## 1. go环境变量设置

下载安装程序```https://golang.org/dl/``` (墙内下载地址```http://www.golangtc.com/download```)。Go开发环境的搭建比较简单，这里主要介绍一下需要配置的几个环境变量：

* ```GOROOT：``` Go的安装目录
* ```GOPATH：``` go build时除了会查找GOROOT目录下的src目录，还会查找这里GOPATH指定的目录。因此这里GOPATH可用作我们的工作目录
* ```GOBIN：``` 一般用作go install安装程序的顶层目录，可以不用指定
* ```GOOS:``` 用于指定Go运行的操作系统
* ```GOARCH：``` 指定系统环境，i386表示x86，amd64表示x64
* ```PATH：``` 一般需要导出GOROOT,GOROOT下的bin两个目录

例如，我们可以在```/etc/profile```文件中定义如下：

![go-env](https://ivanzz1001.github.io/records/assets/img/go/go-env.png)


另外，我们还可以针对自己的工程(project)建立一个局部的配置```env.cfg```：
<pre>
# pwd
/home/ivanzz1001/Go/oss

# ls
env.cfg  src


# cat env.cfg
export GOROOT=/usr/local/go/
export GOPATH=/home/ivanzz1001/Go/oss/
export GOBIN=/usr/local/go/
#export GOTOOLDIR=/usr/local/go1.13.3/pkg/tool/${platform}
export GOTOOLDIR=/usr/local/go1.13.3/pkg/tool/linux_amd64
</pre>

这样在每一次需要进行编译时，先执行```source env.cfg```将相应的全局配置更改称为当前项目所需的配置。

<br />

IDE工具：**JetBrains Gogland**

画图工具: EDraw

## 2. Centos上安装Go开发环境

### 2.1 安装Go开发环境
我们当前的操作系统环境为：
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux bogon 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

这里yum源上的go版本符合我们的要求，因此这里直接采用yum来安装：
<pre>
# yum install golang
# which go
/usr/bin/go 

//最终我们找到go的安装目录为
# ls /usr/lib/golang/
api  bin  favicon.ico  lib  pkg  robots.txt  src  test  VERSION
</pre>
或者我们也可以直接到[go官网](https://golang.org/)去翻墙下载，然后直接解压到对应的安装目录即可：
<pre>
# wget https://dl.google.com/go/go1.12.9.linux-amd64.tar.gz
# tar -zxvf go1.12.9.linux-amd64.tar.gz -C /usr/local/
# ls  /usr/local/go
api  AUTHORS  bin  CONTRIBUTING.md  CONTRIBUTORS  doc  favicon.ico  lib  LICENSE  misc  PATENTS  pkg  README.md  robots.txt  src  test  VERSION
# ln -s /usr/local/go/bin/go /usr/bin/go
# ln -s /usr/local/go/bin/godoc /usr/bin/godoc
# ln -s /usr/local/go/bin/gofmt /usr/bin/gofmt
</pre>


在/etc/profile文件中添加如下：
<pre>
export GOROOT=/usr/lib/golang
export GOBIN=/usr/lib/golang/bin
export GOPATH=/opt/gowork
export GOOS=linux
export GOARCH=amd64
export PATH=$PATH:$GOROOT:$GOBIN
</pre>

上面我们将/opt/gowork/目录作为我们的根工作目录（注意： 通常我们会在根目录下创建src目录用于存放我们的项目源代码）， 然后我们执行如下命令：
<pre>
# source /etc/profile
# go version
# go version
go version go1.8.3 linux/amd64
</pre>

注： 如果我们经常需要切换根工作目录，我们其实也可以在对应的目录下创建一个env.cfg文件，然后在我们要切换到对应的根工作目录下时，先执行该根目录下的'source env.cfg'命令。下面给出env.cfg文件的一个示例：
{% highlight string %}
# cat env.cfg
export GOROOT=/usr/local/go/
export GOPATH=/home/work/another-space
export GOBIN=/usr/local/go/
{% endhighlight %}

### 2.2 测试
我们在```GOPATH```指定的根目录下创建```test```工程:
<pre>
# mkdir -p /opt/gowork/test && cd /opt/gowork/test
</pre>
编写```hello.go```源代码文件：
{% highlight string %}
package main
import "fmt"


func main() {
    fmt.Println("Hello, 世界")
}
{% endhighlight %}

编译运行：
<pre>
# go run hello.go 
Hello, 世界
</pre>

## 3. 64bit Win7上安装Go开发环境

### 3.1 安装Go开发环境

这里我们直接安装```go1.8.windows-amd64.msi```可执行文件(这里假设我们安装在```D:\Go```目录），安装完成后新建如下系统变量：

* GOARCH: 配置当前系统架构，这里配置为amd64

* GOOS: 配置当前操作系统，这里配置为```windows```

* GOPATH: 配置我们当前的工作目录。假设以后我们在```E:\Workspace\go```目录下编写程序，那么我们可以将此目录设置到GOPATH中

* GOROOT: 这里将我们的安装目录（即```D:\Go```)设置为GOROOT。

接着将```%GOROOT%bin;```加入到系统环境变量PATH中。

到此为止，我们就把Go开发环境搭建完毕。
{% highlight string %}
C:\Users\Administrator>go version
go version go1.8 windows/amd64
{% endhighlight %}

### 3.2 安装goland
这里安装```goland-2018.1.exe```并进行破解。安装完成后，我们在上面设置的```GOPATH```目录下新建```oss/src```目录。然后启动goland。然后进入**"File"**->**Settings**，重新设置我们的```工程路径```(project GOPATH)，如下图所示：

![goland-usage](https://ivanzz1001.github.io/records/assets/img/go/goland-usage.png)

上面我们注意到，设置的project GOPATH是```E:\Workspace\go\oss```，并没有包含```src```。而我们实际建立的工程文件夹是放在src目录下的。

>注： 新版go sdk默认采用module的方式来进行编译，可以执行如下命令来关闭
>
> go env -w GO111MODULE=off 或者 go env -w GO111MODULE=auto




<br />
<br />
**[参看]：**

1. [go——搭建Win7下的Go开发环境](http://www.cnblogs.com/caiyezi/p/5641363.html)

2. [GoLand软件的免激活使用](http://blog.csdn.net/benben_2015/article/details/78725467)

3. [goland的激活码](https://www.cnblogs.com/aomi/p/8288137.html)

4. [go官网](https://golang.org/dl/)

5. [go国内官网](https://golang.google.cn/)

6. [centos 7搭建go环境](https://www.cnblogs.com/ylqs/p/7541806.html)

7. [Goland2018破解](https://blog.csdn.net/github_39533414/article/details/81038333)

8. [go官网](https://golang.org/)

<br />
<br />
<br />

