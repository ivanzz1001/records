---
layout: post
title: Centos7.3下部署Java开发环境
tags:
- LinuxOps
categories: linux
description: Centos7.3下部署Java开发环境
---

本文我们主要讲述一下如何在Centos7.3下部署Java开发环境。当前我们的操作系统环境如下：


<!-- more -->
<pre>
[root@localhost nginx-1.10.3]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core
</pre>

这里我们安装JDK1.8版本。


## 1. 卸载JDK(可选）

先查看我们是否已经安装过JDK：
{% highlight string %}
# rpm -qa|grep jdk
{% endhighlight %}

如果已经安装，可以执行如下命令卸载：
{% highlight string %}
# rpm -e  --nodeps java-1.7.0-openjdk-headless-1.7.0.111-2.6.7.2.el7_2.x86_64
{% endhighlight %}
```注意：上面nodeps后面的那串名称是通过上面命令查询出来的，查出几个卸载几个```


## 2. 安装JDK

我们可以通过如下两种方式下载JDK1.8:

* 可以直接到官网下载http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html (这里选择的是rpm 64位的)，下载完然后复制到centos中的自己想要存放的目录中
* 直接在centos中，创建一个目录存储下载的jdk，然后通过命令下载
<pre>
# wget jdk-8u131-linux-x64.rpm http://download.oracle.com/otn-pub/java/jdk/8u131-b11/jdk-8u131-linux-x64.rpm
</pre>

下载完成后执行如下命令进行安装：
{% highlight string %}
# rpm -ivh jdk-8u131-linux-x64.rpm
{% endhighlight %}

执行如下命令检查是否安装成功：
<pre>
[root@localhost nginx-1.10.3]# java -version
openjdk version "1.8.0_141"
OpenJDK Runtime Environment (build 1.8.0_141-b16)
OpenJDK 64-Bit Server VM (build 25.141-b16, mixed mode)
</pre>
 
## 3. 配置环境变量
执行如下命令查看安装目录：
{% highlight string %}
[root@localhost nginx-1.10.3]# which java
/usr/bin/java
[root@localhost nginx-1.10.3]# 
[root@localhost nginx-1.10.3]# ls -lrt /usr/bin/java
lrwxrwxrwx 1 root root 22 Jul 31 23:28 /usr/bin/java -> /etc/alternatives/java
[root@localhost nginx-1.10.3]# 
[root@localhost nginx-1.10.3]# ls -lrt /etc/alternatives/java
lrwxrwxrwx 1 root root 73 Jul 31 23:28 /etc/alternatives/java -> /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre/bin/java
[root@localhost nginx-1.10.3]# 
[root@localhost nginx-1.10.3]# ls -lrt /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64
total 4
drwxr-xr-x 2 root root 4096 Jul 31 23:28 bin
drwxr-xr-x 3 root root  132 Jul 31 23:28 include
drwxr-xr-x 3 root root  144 Jul 31 23:28 lib
drwxr-xr-x 2 root root  204 Jul 31 23:28 tapset
drwxr-xr-x 4 root root   28 Jul 31 23:28 jre
{% endhighlight %}

最后我们得出安装目录为：
<pre>
/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre/bin/java
</pre>

将如下环境变量配置到/etc/profile中：
{% highlight string %}
export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64

export JRE_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre

export CLASSPATH=$JAVA_HOME/lib:$JRE_HOME/lib:$CLASSPATH

export PATH=$JAVA_HOME/bin:$PATH
{% endhighlight %}

执行```source /etc/profile```命令使配置生效。

检查环境变量设置是否成功：
<pre>
[root@localhost nginx-1.10.3]# echo $JAVA_HOME
/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64
[root@localhost nginx-1.10.3]#
[root@localhost nginx-1.10.3]# echo $JRE_HOME 
/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre
[root@localhost nginx-1.10.3]#
[root@localhost nginx-1.10.3]# echo $CLASSPATH
/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/lib:/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre/lib:
</pre>

## 4. 测试
编写如下测试程序(HelloWorld.java)：
{% highlight string %}
package com.TT;

public class HelloWorld{
        public static void main(String[] args)
        {
                System.out.println("Hello,world");
        }
}
{% endhighlight %}

执行如下命令进行编译、执行：
<pre>
[root@localhost java-src]# javac  -d . HelloWorld.java
[root@localhost java-src]# 
[root@localhost java-src]# java com.TT.HelloWorld
Hello,world
</pre>






说明,Linux环境下后台执行命令的两种方式：

* command & ： 后台运行，你关掉终端会停止运行

* nohup command & ： 后台运行，你关掉终端也会继续运行


<br />
<br />

**参看：**

1. [Centos7.2下部署Java开发环境](http://www.cnblogs.com/layezi/p/7049015.html)

<br />
<br />
<br />





