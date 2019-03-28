---
layout: post
title: http协议
tags:
- http
categories: http
description: http协议
---


本章我们对http协议相关内容做一个记录。


<!-- more -->


## 1. HTTP协议8种请求类型介绍
HTTP协议中共定义了8种方法或者叫```动作```(action)来表明对Request-URI指定的资源的不同操作方式。具体介绍如下：

* OPTIONS: 返回服务器针对特定资源所支持的HTTP请求方法。也可以利用向Web服务器发送```*```的请求来测试服务器的功能性。

* HEAD： 向服务器索要与GET请求相一致的响应，只不过响应体将不会被返回。这一方法可以在不必传输整个响应内容的情况下，就可以获取包含在响应消息头中的元信息。

* GET: 向特定的资源发出请求

* POST: 向指定资源提交数据进行处理请求（例如提交表单或者上传文件）。数据被包含在请求体中。POST请求可能会导致新的资源的创建和(或）已有资源的修改。

* PUT: 向指定资源位置上传其最新内容

* DELETE: 请求服务器删除Request-URI所标识的资源

* TRACE： 回显服务器收到的请求，主要用于测试或诊断

* CONNECT: Http/1.1协议中预留给能够将连接改为管道方式的代理服务器。

虽然HTTP的请求方式有8种，但是我们在实际应用中常用的也就是GET和POST，其他的请求方式也都可以通过这两种方式间接的来实现。


## 2. HTTP常用状态码
HTTP状态码(HTTP status code)是用以表示网页服务器超文本传输协议响应状态的3位数字代码。它由 RFC 2616 规范定义的，并得到 RFC 2518、RFC 2817、RFC 2295、RFC 2774 与 RFC 4918 等规范扩展。所有状态码的第一个数字代码了响应的5种状态之一:

* 1xx: 消息。 

* 2xx: 成功

* 3xx: 重定向

* 4xx: 请求错误

* 5xx: 服务器错误 

所示的消息短语是典型的，但是可以提供任何可读取的替代方案。除非另有说明，状态码是HTTP/1.1标准(RFC 7231)的一部分。下面我们对各种类型的一些常见的错误码做一个简单介绍。

### 2.1 消息

这种类型的状态码，代表请求已被接受，需要继续处理。这类响应是临时响应，只包含状态行和某些可选的响应头信息，并以空行结束。由于HTTP/1.0协议中并没有定义任何```1xx```状态码，所以除非在某些试验条件下，服务器禁止向此类客户端发送1xx响应。

1) **100 Continue**

 客户端应当继续发送请求。这个临时响应是用来通知客户端它的部分请求已经被服务器接收，且仍未被拒绝。客户端应当继续发送请求的剩余部分，或者如果请求已经完成，忽略这个响应。服务器必须在请求完成后向客户端发送一个最终响应。

2) **101 Switching Protocols**

服务器已经理解了客户端的请求，并将通过Upgrade消息头通知客户端采用不同的协议来完成这个请求。在发送完这个响应最后的空行后，服务器将会切换到在Upgrade消息头中定义的那些协议。

只有在切换新的协议更有好处的时候才应该采取类似措施。例如，切换到新的HTTP版本比旧的版本更有优势，或者切换到一个实时且同步的协议以传送利用此类特性的资源。

3) **102 Processing**

由WebDAV（RFC 2518）扩展的状态码，代表处理将被继续执行


### 2.2 成功
这一类型的状态码，代表请求已成功被服务器接收、理解、并接受。

1) **200 OK**

200 OK: 请求已成功，请求所希望的响应头或数据体将随此响应返回。出现此状态码是表示正常状态。


2) **206 Partial Content**

服务器已经成功处理了部分GET请求。类似于```FlashGet```或者```迅雷```这类的HTTP下载工具都是使用此类响应实现断点续传或者将一个大文档分解为多个下载段同时下载。

该请求必须包含```Range```头信息来指示客户端希望得到的内容范围，并且可能包含```If-Range```来作为请求条件。

响应必须包含如下的头部域：

* Content-Range: 用以指示本次响应中返回的内容范围。如果是Content-Type为```multipart/byteranges```的多段下载，则每一multipart段中都应包含Content-Range域用以指示本段的内容范围。假如响应中包含Content-Length,那么它的数值必须匹配它返回的内容范围的真实字节数。

* Date

* ETag(和/或 Content-Location): 假如同样的请求本应该返回200响应

* Expires,Cache-Control 和/或 Vary: 假如其值可能与之前相同变量的其他响应对应的值不同的话




### 2.3 重定向
这类状态码代表需要客户端采取进一步的操作才能完成请求。通常，这些状态码用来重定向，后续的请求地址（重定向目标）在本次响应的Location域中指明。

当且仅当后续的请求所使用的方法是GET或者HEAD时，用户浏览器才可以在没有用户介入的情况下自动提交所需要的后续请求。客户端应当自动监测无限循环重定向（例如：A->A，或者A->B->C->A)，因为这会导致服务器和客户端大量不必要的资源消耗。按照HTTP/1.0版规范的建议，浏览器不应自动访问超过5次的重定向。

1） **302 Move temporarily**

请求的资源临时从不同的URI响应请求。由于这样的重定向是临时的，客户端应当继续向原有地址发送以后的请求。只有在```Cache-Control```或者```Expires```中进行了指定的情况下，这个响应才是可缓存的。

如果这不是一个GET或HEAD请求，那么浏览器禁止自动进行重定向，除非得到用户的确认，因为请求的条件可能因此发生变化。

注意：虽然RFC 1945和RFC 2068规范不允许客户端在重定向时改变请求的方法，但是很多现存的浏览器将302响应试作303响应，并且使用GET方式访问在Location中规定的URI，而无视原先请求的方法。状态码303和307被添加了进来，用以明确服务器期待客户端进行何种反应。

下面我们来看一下通过Chrome浏览器向```http://www.baidu.com```发起请求的场景，然后通过```Fiddler```进行抓包。如下是HTTP请求：
{% highlight string %}
GET http://www.baidu.com/ HTTP/1.1
Host: www.baidu.com
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
{% endhighlight %}

如下是返回的响应：
{% highlight string %}
HTTP/1.1 302 Found
Connection: Keep-Alive
Content-Length: 225
Content-Type: text/html
Date: Fri, 08 Mar 2019 06:34:47 GMT
Location: https://www.baidu.com/
P3p: CP=" OTI DSP COR IVA OUR IND COM "
Server: BWS/1.1
Set-Cookie: BAIDUID=D389251314805B518EDC730B18F6CEF1:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BIDUPSID=D389251314805B518EDC730B18F6CEF1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: PSTM=1552026887; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BD_LAST_QID=15083339198343949115; path=/; Max-Age=1
X-Ua-Compatible: IE=Edge,chrome=1

<html>
<head><title>302 Found</title></head>
<body bgcolor="white">
<center><h1>302 Found</h1></center>
<hr><center>5291656f5c4a71bf0ac4c2afdd3e6ada7f9111f6
Time : Mon Mar  4 15:58:43 CST 2019</center>
</body>
</html>
{% endhighlight %}





2) **304 Not Modified**

如果客户端发送了一个带条件的 GET 请求且该请求已被允许，而文档的内容（自上次访问以来或者根据请求的条件）并没有改变，则服务器应当返回这个状态码。304响应禁止包含消息体，因此始终以消息头后的第一个空行结尾。

304状态码或许不应该认为是一种错误，而是对客户端有缓存情况下服务端的一种响应。



### 2.4 请求错误

这类的状态码代表了客户端看起来可能发生了错误，妨碍了服务器的处理。除非响应的是一个```HEAD```请求，否则服务器就应该返回一个解释当前错误状态的实体，以及这是临时的还是永久性的状况。这些状态码适用于任何请求方法。浏览器应当向用户显示任何包含在此类错误响应中的实体内容。

如果错误发生时客户端正在传送数据，那么使用TCP的服务器实现应当仔细确保在关闭客户端与服务器之间的连接之前，客户端已经收到了包含错误信息的数据包。如果客户端在收到错误信息后继续向服务器发送数据，服务器的TCP栈将向客户端发送一个```重置```(rst)数据包，以清除该客户端所有还未识别的输入缓冲，以免这些数据被服务器上的应用程序读取并干扰后者。






### 2.5 服务器错误
该类状态码代表了服务器在处理请求的过程中有错误或者异常状态发生，也有可能是服务器意识到以当前的软硬件资源无法完成对请求的处理。除非这是一个```HEAD```请求，否则服务器应当包含一个解释当前错误状态以及这个状况是临时的还是永久的解释信息实体。浏览器应当向用户展示任何在当前响应中被包含的实体。

这些状态码适用于任何响应方法。





<br />
<br />

**[参考]**

1. [HTTP协议的8种请求类型介绍](https://www.cnblogs.com/xuyuQAQ/p/8371559.html)

2. [HTTP状态码](https://baike.baidu.com/item/HTTP%E7%8A%B6%E6%80%81%E7%A0%81/5053660?fr=aladdin)


<br />
<br />
<br />

