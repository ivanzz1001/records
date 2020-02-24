---
layout: post
title: HTML5 websocket(转) 
tags:
- http
categories: http
description: http协议
---

本章我们介绍一下HTML5 websocket相关的一些内容。



<!-- more -->

## 1. HTML5 websocket
websocket是HTML5开始提供的一种在单个TCP连接上进行全双工通讯的协议。WebSocket通信协议于2011年被IETF定为标准RFC 6455，并由RFC7936补充规范。WebSocket API也被W3C定为标准。

websocket使得客户端和服务器之间的数据交换变得更加简单，允许服务端主动向客户端推送数据。在websocket api中，浏览器和服务器只需要完成一次握手，两者之间就直接可以创建持久性的连接，进行双向数据传输。

在websocket api中，浏览器和服务器只需要做一个握手的动作，然后，浏览器和服务器之间就形成了一条快速通道。两者之间就可以直接进行数据互传。

现在，很多网站为了实现推送技术，都是通过ajax轮询来实现的。轮询是在特定的时间间隔（如每1秒），由浏览器对服务器发出HTTP请求，然后由服务器返回最新的数据给客户端的浏览器。这种传统的模式带来很明显的缺点，即浏览器需要不断的向服务器发出请求，然而HTTP请求可能包含较长的头部，其中真正有效的数据可能只是很小的一部分，显然这样会浪费很多的带框等资源。

HTML5定义的websocket协议，能更好的节省服务器资源和带宽，并且能够更实时的进行通讯。

![ws](https://ivanzz1001.github.io/records/assets/img/http/ws_arch.png)

浏览器通过Javascript向服务器发出建立Websocket连接的请求，连接建立以后，客户端和服务端就可以通过TCP连接直接交换数据。

当你获取Websocket连接后，你可以通过```send()```方法来向服务器发送数据，并通过```onmessage```事件来接收服务器返回的数据。以下API用于创建Websocket对象：
<pre>
var Socket = new WebSocket(url, [protocol] );
</pre>
上面代码中的第一个参数```url```指定连接的URL地址，第二个参数protocol是可选的，用于指定可接受的子协议。

### 1.1 WebSocket属性
以下是websocket对象的属性。假定我们使用了以上代码创建了socket对象：
<pre>
       属性                             描述
-----------------------------------------------------------------------------------
Socket.readyState           只读属性 readyState 表示连接状态，可以是以下值：
                            0 --- 表示连接尚未建立
                            1 --- 表示连接已建立，可以进行通讯
                            2 --- 表示连接正在进行关闭
                            3 --- 表示连接已经关闭，或者连接不能打开


Socket.bufferedAmount       只读属性, bufferedAmount表示已被send()放入队列中等待传输，
                            但还没有发出的UTF-8文本字节数
----------------------------------------------------------------------------------- 
</pre>

### 1.2 WebSocket事件
以下是websocket对象的相关事件。假定我们使用了以上代码创建了Socket对象：
<pre>
事件               事件处理程序                   描述
-----------------------------------------------------------------------------------
open              Socket.onopen             连接建立时触发

message           Socket.onmessage          客户端接收服务端数据时触发

error             Socket.onerror            通信发生错误时触发

close             Socket.onclose            连接关闭时触发
</pre>

### 1.3 WebSocket方法
以下是 WebSocket 对象的相关方法。假定我们使用了以上代码创建了 Socket 对象：
<pre>
方法                        描述
-----------------------------------------------------------
Socket.send()          使用连接发送数据

Socket.close()         关闭连接
</pre>

### 1.4 Websocket实例
WebSocket协议本质上是一个基于TCP的协议。为了建立一个WebSocket连接，客户端浏览器首先要向服务器发起一个HTTP请求，这个请求和通常的HTTP请求不同，包含了一些附加头信息，其中附加头信息```Upgrade: WebSocket```表明这是一个申请协议升级的HTTP请求，服务端解析这些附加的头信息，然后产生应答信息返回给客户端，客户端和服务端的WebSocket连接就建立起来了。双方就可以通过这个连接通道自由的传递信息，并且这个连接会持续存在，直到客户端或者服务器端的某一方主动的关闭连接。

###### 客户端的HTML和Javascript
目前大部分浏览器支持 WebSocket() 接口，你可以在以下浏览器中尝试实例： Chrome, Mozilla, Opera 和 Safari。

```ws_client.html```文件内容:
{% highlight string %}
<!DOCTYPE HTML>
<html>
   <head>
   <meta charset="gb2312">
   <title>菜鸟教程(runoob.com)</title>
    
      <script type="text/javascript">
         function WebSocketTest()
         {
            if ("WebSocket" in window)
            {
               alert("您的浏览器支持 WebSocket!");
               
               // 打开一个 web socket
               var ws = new WebSocket("ws://192.168.10.130:9998/echo");
                
               ws.onopen = function()
               {
                  // Web Socket 已连接上，使用 send() 方法发送数据
                  ws.send("hello,world");
                  alert("数据发送中...");
               };
                
               ws.onmessage = function (evt) 
               { 
                  var received_msg = evt.data;
                  alert("数据已接收:" + received_msg);
               };
                
               ws.onclose = function()
               { 
                  // 关闭 websocket
                  alert("连接已关闭..."); 
               };
            }
            
            else
            {
               // 浏览器不支持 WebSocket
               alert("您的浏览器不支持 WebSocket!");
            }
         }
      </script>
        
   </head>
   <body>
   
      <div id="sse">
         <a href="javascript:WebSocketTest()">运行 WebSocket</a>
      </div>
      
   </body>
</html>
{% endhighlight %} 

###### 安装pywebsocket
在执行以上程序前，我们需要创建一个支持WebSocket的服务。从[pywebsocket](https://github.com/google/pywebsocket)下载mod_pywebsocket,或者使用 git 命令下载：
<pre>
# git clone https://github.com/google/pywebsocket.git
</pre>

```mod_pywebsocket```需要python环境支持，它是一个Apache HTTP的Web Socket扩展，安装步骤如下：

* 解压下载的文件

* 进入pywebsocket目录

* 执行命令
<pre>
# python setup.py build
# sudo python setup.py install
</pre>
注： 这里我们使用的是python版本是```2.7.12```

* 查看文档说明
<pre>
# pydoc mod_pywebsocket
</pre>

###### 开启服务

在*pywebsocket/mod_pywebsocket*目录下执行以下命令：
<pre>
sudo python standalone.py -p 9998 -w ../example/
</pre>
以上命令会开启一个端口号为 9998 的服务，使用```-w```来设置处理程序 echo_wsh.py 所在的目录。

现在我们可以在 Chrome 浏览器打开前面创建的```ws_client.html```文件。如果你的浏览器支持WebSocket(), 点击"运行 WebSocket"，你就可以看到整个流程各个步骤弹出的窗口。

## 2. WebSocket协议深入
Websocket 使用```ws```或```wss```的统一资源标志符，类似于HTTPS，其中```wss```表示建立在```TLS```之上的 Websocket。如：
<pre>
ws://example.com/wsapi
wss://secure.example.com/
</pre>

Websocket使用和HTTP相同的TCP端口，可以绕过大多数防火墙的限制。默认情况下，WebSocket协议使用```80```端口；运行在TLS之上时，默认使用443端口。

### 2.1 典型的WebSocket握手请求
一个典型的Websocket握手请求如下：

* 客户端请求
<pre>
GET / HTTP/1.1
Upgrade: websocket
Connection: Upgrade
Host: example.com
Origin: http://example.com
Sec-WebSocket-Key: sN9cRrP/n9NdMgdcy2VJFQ==
Sec-WebSocket-Version: 13
</pre>

* 服务端回应
<pre>
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: fFBooB7FAkLlXgRSz0BT3v4hq5s=
Sec-WebSocket-Location: ws://example.com/
</pre>

1) **相关字段的解释**

* ```Connection``` 必须设置为Upgrade，表示客户端希望连接升级

* ```Upgrade``` 必须设置为websocket，表示希望升级到websocket协议

* ```Sec-WebSocket-Key``` 是随机的字符串，服务端会用这些数据来构造出一个```SHA-1```的信息摘要。把'Sec-WebSocket-Key' 加上一个特殊字符串 '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'，然后计算```SHA-1```摘要，之后进行```BASE-64```编码，将结果做为```Sec-WebSocket-Accept```头的值，返回给客户端。如此操作，可以尽量避免普通 HTTP 请求被误认为 Websocket 协议。

* ```Sec-WebSocket-Version```表示支持的 Websocket 版本。RFC6455 要求使用的版本是```13```，之前草案的版本均应当弃用。

* ```Origin``` 字段是可选的，通常用来表示在浏览器中发起此 Websocket 连接所在的页面，类似于 Referer。但是，与 Referer 不同的是，Origin 只包含了协议和主机名称

* 其他一些定义在 HTTP 协议中的字段，如 Cookie 等，也可以在 Websocket 中使用。

2) **服务器支持**

在服务器方面，网上都有不同对websocket支持的服务器：

* php - http://code.google.com/p/phpwebsocket/

* jetty - http://jetty.codehaus.org/jetty/（版本7开始支持websocket）

* netty - http://www.jboss.org/netty

* ruby - http://github.com/gimite/web-socket-ruby

* Kaazing - https://web.archive.org/web/20100923224709/http://www.kaazing.org/confluence/display/KAAZING/Home

* Tomcat - http://tomcat.apache.org/（7.0.27支持websocket，建议用tomcat8，7.0.27中的接口已经过时）

* WebLogic - http://www.oracle.com/us/products/middleware/cloud-app-foundation/weblogic/overview/index.html（12.1.2開始支持）

* node.js - https://github.com/Worlize/WebSocket-Node

* node.js - http://socket.io

* nginx - http://nginx.com/

* mojolicious - http://mojolicio.us/

* python - https://github.com/abourget/gevent-socketio

* Django - https://github.com/stephenmcd/django-socketio

* erlang - https://github.com/ninenines/cowboy.git


### 2.2 WebSocket与普通socket的区别
说到websocket，我觉得有必要说下跟socket的区别：

软件通信有七层结构，下三层结构偏向与数据通信，上三层更偏向于数据处理，中间的传输层则是连接上三层与下三层之间的桥梁。每一层都做不同的工作，上层协议依赖于下层协议。基于这个通信结构的概念。

Socket其实并不是一个协议，是应用层与TCP/IP协议族通信的中间软件抽象层，它是一组接口。当两台主机通信时，让Socket去组织数据，以符合指定的协议。TCP连接则更依靠于底层的IP协议，IP协议的连接则依赖于链路层等更低层次协议。

WebSocket则是一个典型的应用层协议。

总的来说： Socket是传输控制层协议，WebSocket是应用层协议。


<br />
<br />

**[参考]**


1. [HTML5 websocket](https://www.runoob.com/html/html5-websocket.html)


<br />
<br />
<br />

