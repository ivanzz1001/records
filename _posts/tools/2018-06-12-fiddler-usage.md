---
layout: post
title: fiddler工具的使用
tags:
- tools
categories: tools
description: fiddler工具的使用
---

本文主要介绍一下fiddler工具的使用

<!-- more -->


## 1. Fiddler基础知识

* Fiddler是强大的抓包工具，它的原理是以web代理服务器的形式进行工作的，使用的代理地址是： 127.0.0.1，默认端口8888， 我们也可以通过设置进行修改。

* 代理就是在客户端和服务器之间设置一道关卡，客户端先将请求数据发送从出去，代理服务器会将数据包进行拦截，代理服务器再冒充客户端发送数据到服务器； 同理，服务器将响应数据返回，代理服务器也会将数据拦截，再返回给客户端

* Fiddler可以抓取支持http代理的任意程序的数据包，如果要抓取https的话，要先安装证书

如下是fiddle发送请求：

![fiddler-json-req](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_json_req.jpg)


## 2. Fiddler HTTP代理的设置
Fiddler4.0在启动的时候，默认会监听127.0.0.1:8888端口。网上讲了很多方法，比如手动设置IE/Firefox浏览器代理为```127.0.0.1:8888```， 此种方法虽然可行，但是每个不同浏览器都得设置一下，比较麻烦。我们启动Fiddler之后， 可以发现Fiddler能够自动代理差不多所有的浏览器， 然后再跑到各浏览器代理设置的地方去查看， Fiddler并未在那里帮我们设置。那么Fiddler是怎么帮我们自动做到代理所有浏览器的呢？

其实Fiddler是通过修改注册表的方式来设置代理服务器的。下面我们详细介绍一下操作步骤：

1） 按快捷键```WIN + R```调出运行窗口，输入```regedit```后进入注册表

2) 找到如下：
<pre>
HKEY_CURRENT_USER/Software/Microsoft/Windows/CurrentVersion/Internet Settings/
</pre>
点击， 我们可以看到：

![fiddler-regedit](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_regedit.jpg)

上面Fiddler4.0自动帮我们设置了```ProxyEnable```以及```ProxyServer```。

### 3. Fiddler的使用

1) 要使用Fiddler进行抓包，首先要确保Capture Traffic是开启的（安装后默认是开启的），勾选```FILE->Capture Traffic```，也可以直接点击Fiddler界面左下角的图标开启和关闭抓包

![fiddler-usage-1](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_usage_1.jpg)


2) 所以基本上不需要做什么配置，安装后就可以进行抓包了。点击每一个抓到的包， 然后在右侧点击```Insepector```查看详细内容，主要分为请求（即客户端发出的数据）和响应（服务器返回的数据）两部分

3） 我们也可以到如下页面向web服务器发起请求

![fiddler-usage-2](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_usage_2.jpg)


### 4.  Fiddler添加时间戳信息
有时候，我们通过Fiddler抓包，想看到请求响应的时间戳信息。默认情况下， Fiddler是没有打开时间戳信息的，我们可以在菜单栏"Rules"->"Customize Rules"中添加如下脚本：
{% highlight string %}
// 显示每行请求的发起时间:时分秒毫秒
public static BindUIColumn("BeginTime", 80)
function BeginTimingCol(oS: Session){   

	return oS.Timers.ClientDoneRequest.ToString("HH:mm:ss.fff");

}
// 显示每行请求的响应时间:时分秒毫秒
public static BindUIColumn("EndTime", 80)
function EndTimingCol(oS: Session){     

	return oS.Timers.ServerDoneResponse.ToString("HH:mm:ss.fff");

}
// 显示每行请求的服务端耗时时间
public static BindUIColumn("Time Taken", 80)
function CalcTimingCol(oS: Session){  
   
	var sResult = "0";                  

	if ((oS.Timers.ServerDoneResponse > oS.Timers.ClientDoneRequest)) {  

		sResult = (oS.Timers.ServerDoneResponse - oS.Timers.ClientDoneRequest).TotalMilliseconds.ToString("N0");

	}  

	return sResult + "ms";  
}
{% endhighlight %}
之后继续抓包，则可以看到请求、响应的时间戳信息了。

<br />
<br />

**[参看]**

1. [Fiddler工具使用介绍一](https://www.cnblogs.com/miantest/p/7289694.html)

2. [Win7系统如何使用代理服务器上网？Win7系统设置代理服务器的方法](http://www.xitongzhijia.net/xtjc/20160814/80323.html)

<br />
<br />
<br />

