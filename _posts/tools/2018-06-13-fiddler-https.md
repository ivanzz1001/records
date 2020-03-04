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

因此，要想使用fiddler，我们就要启用WinINet的HTTP代理功能。参看文章：[fiddler抓包代理设置问题](https://blog.csdn.net/tsj11514oo/article/details/51794330)

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

![fiddler-mgr](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_certmgr.png)

3) 清除浏览器中与fiddler相关的根证书

清除浏览器上的证书文件，此处需要仔细查找带有```FiddlerRoot```的字样，并删除。以Google浏览器为例说明，在浏览器上输入： chrome://settings/

![fiddler-certclr](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_certclr.png)


4) 重置所有的certificates

打开fiddler，点击工具栏中的**Tools->Options**，点击**Actions**，选择最后一项**Reset All certifications**，然后关闭。如下图所示：

![fiddler-certrst](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_certrst.png)

### 2.2 fiddler抓取https包

1) 下载最新版fiddler，强烈建议在官网下载（官网地址: https://www.telerik.com/download/fiddler)

2) 正常傻瓜式安装，下一步，下一步，安装完毕后，先不用急于打开软件。

3) 下载并安装Fiddler证书生成器： http://www.telerik.com/docs/default-source/fiddler/addons/fiddlercertmaker.exe?sfvrsn=2

>注： fiddler安装后默认的根证书FiddlerRoot通常有些问题(其“证书预期目的”经常不是“所有”），导致可能在抓取https包时出现故障.

4） 打开Fiddler，点击工具栏中的Tools->Options

![fiddler-tools](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_tools.png)

5) 点击https设置选项，勾选选择项

![fiddler-https-set](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_https_set.png)

6) 设置FiddlerRoot证书

点击```Actions```按钮，如下所示

![fiddler-actions](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_actions.jpg)

可以看到这里有多个选项。通过这我们有两种方法来在系统中设置FiddlerRoot证书，下面我们分别介绍

* 自动设置

我们可以选择*Trust Root Certificate*，然后让Fiddler自动的帮我们设置FiddlerRoot根证书到系统，如下所示：

![fiddler-trust](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_trust_cert.jpg)

通过此方法，fiddler通常会把FiddlerRoot证书帮我们导入到“受信任的根证书”目录。但有时，可能也会遇到问题，此时我们可以采用下面介绍的```手动设置```方法来进行操作。

* 手动设置

我们可以选择*Export Root Certificate to Desktop*将FiddlerRoot证书导出到桌面。之后，将该根证书导入到浏览器中。以Google浏览器为例，在浏览器输入：chrome://settings/，然后进入高级设置，点击管理证书，将证书导入到“受信任的根证书颁发机构”，如下图所示：

![fiddler-root](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_root.jpg)


7) 关闭fiddler，再重新打开，抓取https数据包

如下图所示，这里我们抓取*https://www.baidu.com*:

![fiddler-baidu](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_baidu.jpg)

## 3. 火狐浏览器抓包
经过上面的设置之后我们可以通过Fiddler抓到IE和chrome等浏览器的http和https请求了，但如果我们使用的是火狐浏览器，则有可能会抓取不到任何请求数据，我们需要手动设置一些信息才行。

1) 查看当前fiddler监听端口

通过Options->Connections查看Fiddler的监听端口，如下所示：

![fiddler-proxy](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_proxy.jpg)

2) 设置火狐浏览器代理

打开火狐浏览器，选择```选项```，如下图所示：

![fiddler-firefox](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_firefox.png)

打开之后，搜索代理：

![fiddler-firefox-proxy](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_firefox_proxy.png)

之后，再手动设置代理(这里将代理接口设置为8888)：

![fiddler-firefox-set](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_firefox_set.png)

3） 添加证书

这里我们将证书导入到火狐浏览器。打开火狐浏览器，进入设置选项搜索证书：

![fiddler-firefox-cert](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_firefox_cert.png)

之后将FiddlerRoot.cer导入火狐浏览器即可：

![fiddler-firefox-import](https://ivanzz1001.github.io/records/assets/img/tools/fiddler_firefox_import.png)

导入成功之后，重启火狐浏览器就可以进行抓包了。

<br />
<br />

**[参看]**

1. [fiddler抓web请求](https://www.cnblogs.com/mxqh2016/p/9212941.html)

2. [fiddler抓取https设置详解](https://www.cnblogs.com/joshua317/p/8670923.html)

3. [fiddler抓HTTPS及APP请求的配置教程](https://www.cnblogs.com/nmb-musen/p/10621430.html)

<br />
<br />
<br />

