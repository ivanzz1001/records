---
layout: post
title: 使用fiddler抓取https数据包
tags:
- tools
categories: tools
description: 使用fiddler抓取https数据包
---


很多人使用Fiddler抓包，对于http来说不需要太多纠结，随便设置下就能用，但是抓取https就死活抓不了，出现诸如以下问题：
<pre>
creation of the root certificate was not successful;

Failed to find the root certificate in User Root List;

The Root certificate could not be found;

The root certificate could not be located;

Fiddler 抓取https 显示 Tunnel to ... 443;

Fiddler’s HTTPS Decryption feature is enabled, but this specific tunnel was configured not to be decrypted.

...
</pre>

本章我们会介绍一下如何使用fiddler抓取https请求。

<!-- more -->


## 1. fiddler抓包原理

fiddler调试器注册到操作系统Internet服务（WinINet)中，系统所有的网络请求都会走Fiddler的代理，所以fiddler才能抓包。请参看如下描述：
<pre>
Debug traffic from any client and browser 

Fiddler helps you debug traffic from any browser: Internet Explorer, Chrome, Firefox, Safari, Opera, and more. Once you start
Fiddler, the web debugger registers itself as the system proxy for Microsoft Windows Internet Services (WinINet), the HTTP 
layer used by Internet Explorer, Microsoft Office, and many other products. As the system proxy, all HTTP requests from WinINet
flow through Fiddler before reaching the target web servers. Similarly, all HTTP responses flow through Fiddler before being
 returned to the client application. 

Additionally, most devices that support Wi-Fi or Ethernet can be configured to send their traffic to Fiddler; this includes iOS,
 Android, Windows Phone and Windows RT devices.
</pre>

### 1.1 fiddler解密https原理
其实fiddler就是中间人攻击，依次经过如下过程：

* fiddler接到客户端的https请求，fiddler将请求转发给服务器

* 服务器生成公钥证书，返回给fiddler。fiddler拦截下真的公钥证书，并生成伪造的公钥证书给客户端；

* 客户端使用伪造的公钥证书加密共享密钥发送给fiddler，fiddler使用伪造的私钥解密获取共享密钥；

* fiddler将解密后的共享密钥，使用真正的公钥加密发送给服务端，服务器使用共享密钥与fiddler通信

* fiddler使用共享密钥与客户端通信

以上是fiddler抓包解密的原理，这个原理是建立在https建立连接的基础上的，请参考[https建立连接过程](http://blog.csdn.net/wangjun5159/article/details/51510594)

## 2. fiddler抓取https包步骤

### 2.1 重置fiddler
如果当前电脑上已经安装或曾经安装过fiddler，最好采用如下方法先重置环境，否则后续在抓取https包时，可能会遇到各种怪异的问题，十分难以解决。执行如下步骤进行重置(首次安装fiddler可以忽略如下步骤)：

1） 清除**C:\Users\Administrator\AppData\Roaming\Microsoft\Crypto\RSA**目录下的所有文件


2） 清除与Fiddler相关的根证书

这里我们清除电脑上的与Fiddler相关的根证书： ```WIN+R```快捷键，输入```certmgr.msc```，然后回车进入证书管理界面。参看如下图示，查找所有fiddler证书，然后删除。





<br />
<br />

**[参看]**

1. [fiddler抓web请求](https://www.cnblogs.com/mxqh2016/p/9212941.html)

2. [fiddler抓取https设置详解](https://www.cnblogs.com/joshua317/p/8670923.html)

3. [fiddler抓HTTPS及APP请求的配置教程](https://www.cnblogs.com/nmb-musen/p/10621430.html)

<br />
<br />
<br />

