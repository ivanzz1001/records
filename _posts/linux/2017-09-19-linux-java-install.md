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
# wget http://download.oracle.com/otn-pub/java/jdk/8u131-b11/jdk-8u131-linux-x64.rpm
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

注： Java历史版本可在此下载[javase archive](http://www.oracle.com/technetwork/java/javase/archive-139210.html)
 
## 3. 配置环境变量
执行如下命令查看安装目录：
{% highlight string %}
# which java
/usr/bin/java
# ls -lrt /usr/bin/java
lrwxrwxrwx 1 root root 22 Jul 31 23:28 /usr/bin/java -> /etc/alternatives/java
# ls -lrt /etc/alternatives/java
lrwxrwxrwx 1 root root 73 Jul 31 23:28 /etc/alternatives/java -> /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/jre/bin/java
# ls -lrt /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64
total 4
drwxr-xr-x 2 root root 4096 Jul 31 23:28 bin
drwxr-xr-x 3 root root  132 Jul 31 23:28 include
drwxr-xr-x 3 root root  144 Jul 31 23:28 lib
drwxr-xr-x 2 root root  204 Jul 31 23:28 tapset
drwxr-xr-x 4 root root   28 Jul 31 23:28 jre


# which javac
/usr/bin/javac
# ls -al /usr/bin/javac
lrwxrwxrwx 1 root root 23 May 12 09:55 /usr/bin/javac -> /etc/alternatives/javac
# ls -al /etc/alternatives/javac 
lrwxrwxrwx 1 root root 70 May 12 09:55 /etc/alternatives/javac -> /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/bin/javac
# ls -lrt /usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64/bin/
total 420
-rwxr-xr-x 1 root root   2806 Jan 18 00:38 java-rmi.cgi
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 xjc
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 wsimport
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 wsgen
-rwxr-xr-x 1 root root 103392 Jan 18 01:04 unpack200
-rwxr-xr-x 1 root root   7432 Jan 18 01:04 tnameserv
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 servertool
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 serialver
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 schemagen
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 rmiregistry
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 rmid
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 rmic
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 policytool
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 pack200
-rwxr-xr-x 1 root root   7424 Jan 18 01:04 orbd
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 native2ascii
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 keytool
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jstatd
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jstat
-rwxr-xr-x 1 root root   7424 Jan 18 01:04 jstack
-rwxr-xr-x 1 root root   7368 Jan 18 01:04 jsadebugd
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 jrunscript
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jps
-rwxr-xr-x 1 root root   7416 Jan 18 01:04 jmap
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jjs
-rwxr-xr-x 1 root root   7416 Jan 18 01:04 jinfo
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jhat
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jdeps
-rwxr-xr-x 1 root root   7368 Jan 18 01:04 jdb
-rwxr-xr-x 1 root root   7376 Jan 18 01:04 jconsole
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jcmd
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 javap
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 javah
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 javadoc
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 javac
-rwxr-xr-x 1 root root   7304 Jan 18 01:04 java
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jarsigner
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 jar
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 idlj
-rwxr-xr-x 1 root root   7352 Jan 18 01:04 extcheck
-rwxr-xr-x 1 root root   7360 Jan 18 01:04 appletviewer
{% endhighlight %}

上面我们看到```java```与```javac```的安装目录有稍许不一样。javac主要是用于在编译时使用，而jre主要是用于在运行时使用。最后我们得出java安装主目录为：
<pre>
/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.141-1.b16.el7_3.x86_64
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

说明：可以通过```java -cp```来指定classpath，例如：
{% highlight string %}
# javac -cp ./my-oss-sdk-2.2.2.jar ObjectClientTest.java
# java -cp .:./logback-classic-1.1.7.jar:./logback-core-1.1.7.jar:./slf4j-api-1.7.20.jar:./jcl-over-slf4j-1.7.25.jar \
 :./commons-codec-1.9.jar:./gson-2.8.0.jar:./hamcrest-core-1.3.jar:./httpclient-4.5.3.jar:./httpcore-4.4.6.jar \
 :./httpmime-4.5.3.jar:./my-oss-sdk-2.2.2.jar ObjectClientTest
{% endhighlight %}


## 5. open-jdk依赖
下面给出一个手动安装```java-1.8.0-openjdk-1.8.0.161-0.b14```时大概的依赖包：
<pre>
# ls -al
total 46996
drwxr-xr-x 2 root root     4096 May 12 09:54 .
drwxrwxrwx 7 root root     4096 May 11 21:40 ..
-rw-r--r-- 1 root root   185720 May 11 20:23 chkconfig-1.7.4-1.el7.x86_64.rpm
-rw-r--r-- 1 root root    18900 May 11 20:24 copy-jdk-configs-2.2-3.el7.noarch.rpm
-rw-r--r-- 1 root root   234724 May 11 20:56 fontconfig-2.10.95-11.el7.x86_64.rpm
-rw-r--r-- 1 root root    10116 May 11 20:57 fontpackages-filesystem-1.44-8.el7.noarch.rpm
-rw-r--r-- 1 root root    40964 May 11 20:20 giflib-4.1.6-9.el7.x86_64.rpm
-rw-r--r-- 1 root root   248404 May 11 20:10 java-1.8.0-openjdk-1.8.0.161-0.b14.el7_4.x86_64.rpm
-rw-r--r-- 1 root root 10251448 May 12 09:54 java-1.8.0-openjdk-devel-1.8.0.161-0.b14.el7_4.x86_64.rpm
-rw-r--r-- 1 root root 33169084 May 11 20:12 java-1.8.0-openjdk-headless-1.8.0.161-0.b14.el7_4.x86_64.rpm
-rw-r--r-- 1 root root    61200 May 11 20:29 jpackage-utils-1.7.5-3.16.el6.noarch.rpm
-rw-r--r-- 1 root root    31564 May 11 21:20 libfontenc-1.1.3-3.el7.x86_64.rpm
-rw-r--r-- 1 root root    67720 May 11 20:21 libICE-1.0.9-9.el7.x86_64.rpm
-rw-r--r-- 1 root root    40160 May 11 20:22 libSM-1.2.2-2.el7.x86_64.rpm
-rw-r--r-- 1 root root    22792 May 11 20:16 libXcomposite-0.4.4-4.1.el7.x86_64.rpm
-rw-r--r-- 1 root root    39572 May 11 20:17 libXext-1.3.3-3.el7.x86_64.rpm
-rw-r--r-- 1 root root   155992 May 11 21:02 libXfont-1.5.2-1.el7.x86_64.rpm
-rw-r--r-- 1 root root    41088 May 11 20:18 libXi-1.7.9-1.el7.x86_64.rpm
-rw-r--r-- 1 root root    26312 May 11 20:19 libXrender-0.9.10-1.el7.x86_64.rpm
-rw-r--r-- 1 root root    20812 May 11 20:20 libXtst-1.2.3-1.el7.x86_64.rpm
-rw-r--r-- 1 root root    89660 May 11 20:30 lksctp-tools-1.0.17-2.el7.x86_64.rpm
-rw-r--r-- 1 root root   163172 May 29  2017 lyx-fonts-2.2.3-1.el7.noarch.rpm
-rw-r--r-- 1 root root   129348 Apr 25 19:29 nspr-4.17.0-1.el7.x86_64.rpm
-rw-r--r-- 1 root root   861612 Apr 25 19:29 nss-3.34.0-4.el7.x86_64.rpm
-rw-r--r-- 1 root root    74644 Aug 11  2017 nss-pem-1.0.3-4.el7.x86_64.rpm
-rw-r--r-- 1 root root   318152 Apr 25 19:30 nss-softokn-3.34.0-2.el7.x86_64.rpm
-rw-r--r-- 1 root root   224936 Apr 25 19:30 nss-softokn-freebl-3.34.0-2.el7.x86_64.rpm
-rw-r--r-- 1 root root    62696 Apr 25 19:30 nss-sysinit-3.34.0-4.el7.x86_64.rpm
-rw-r--r-- 1 root root   525004 Apr 25 19:30 nss-tools-3.34.0-4.el7.x86_64.rpm
-rw-r--r-- 1 root root    79484 Apr 25 19:30 nss-util-3.34.0-2.el7.x86_64.rpm
-rw-r--r-- 1 root root    48676 May 11 21:35 ttmkfdir-3.0.9-42.el7.x86_64.rpm
-rw-r--r-- 1 root root   187852 May 11 20:28 tzdata-java-2017c-1.el7.noarch.rpm
-rw-r--r-- 1 root root   533720 May 11 20:15 xorg-x11-fonts-Type1-7.5-9.el7.noarch.rpm
-rw-r--r-- 1 root root    89400 May 11 21:34 xorg-x11-font-utils-7.5-20.el7.x86_64.rpm
</pre>


<br />
<br/>

**说明**,Linux环境下后台执行命令的两种方式：

* command & ： 后台运行，你关掉终端会停止运行

* nohup command & ： 后台运行，你关掉终端也会继续运行


<br />
<br />

**参看：**

1. [Centos7.2下部署Java开发环境](http://www.cnblogs.com/layezi/p/7049015.html)

<br />
<br />
<br />





