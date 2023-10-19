---
layout: post
title: 深入浅出HTTPS工作原理(转)
tags:
- http
categories: http
description: https协议
---

HTTP协议由于是明文传送，所以存在三大风险：

* 被窃听的风险： 第三方可以截获并查看你的内容

* 被篡改的风险： 第三方可以截获并修改你的内容

* 被冒充的风险： 第三方可以伪装成通信方与你通信

HTTP因为存在以上三大安全风险，所以才有了HTTPS的出现。HTTPS涉及到了很多概念，比如SSL(Secure Sockets Layer)/TLS(Transport Layer Security)、数字证书、数字签名、加密、认证、公钥和私钥等，比较容易混淆。我们先从一次简单的安全通信故事讲起吧，其中穿插复习一些密码学的概念。



<!-- more -->


## 1. 关于Bob与他好朋友通信的故事

故事的主人公是**Bob**，他有三个好朋友**Pat**、**Doug**和**Susan**。Bob经常给他们写信，因为他的信是明文传输的，在传递过程中可能被人截获偷窥，也可能被人截获然后篡改了，更有可能别人伪装成Bob本人跟他的好朋友通信，总之是不安全的。他很苦恼，经过一番苦苦探索，诶，他发现计算机安全学里有一种叫非对称加密算法的东东，好像可以帮助他解决这个问题。

<pre>
说明： 非对称加密算法(RSA)是内容加密的一类算法，它有两个秘钥： 公钥与私钥。公钥是公开的钥匙，所有人都可以知道；
私钥是保密的，只有持有者知道。通过公钥加密的内容，只能通过私钥才能解开。非对称加密算法的安全性很高，但是因为计算
量庞大，比较消耗性能。
</pre>

好了，现在我们来看看Bob是怎么应用非对称算法与他的好朋友通信的：

1) 首先Bob弄到了两把钥匙： 公钥和私钥

![https-pubpriv-key](https://ivanzz1001.github.io/records/assets/img/http/https_pubpriv_key.jpg)


2) Bob自己留下私钥，把公钥复制成三份送给了他的三个好朋友Pat、Doug和Susan。

![https-pubkey-distribute](https://ivanzz1001.github.io/records/assets/img/http/https_pubkey_distribute.jpg)


3)  此时，Bob总算可以安心地和他的好朋友愉快地通信了。比如Susan要和他讨论关于去哪吃午饭的事情，Susan就可以先把自己的内容（明文）首先用Bob送给他的公钥做一次加密，然后把加密的内容传送给Bob。Bob收到信后，再用自己的私钥解开信的内容。

![https-info-transfer](https://ivanzz1001.github.io/records/assets/img/http/https_info_transfer.jpg)

说明： 这其实是计算机安全学里的加密的概念，加密的目的是为了不让别人看到传送的内容，加密的手段是通过一定的加密算法及约定的密钥进行的（比如上述用了非对称加密算法以及Bob的公钥），而解密则需要相关的解密算法及约定的密钥（如上述用了非对称加密算法和Bob自己的私钥），可以看出加密是可逆的（可解密的）。


4) Bob看完信后，决定给Susan回一封信。为了防止信的内容被篡改（或者别人伪装成他的身份跟Susan通信），他决定先对信的内容用hash算法做一次处理，得到一个字符串哈希值，Bob又用自己的私钥对哈希值做了一次加密得到一个签名，然后把签名和信（明文的）一起发送给Susan。

![https-signature](https://ivanzz1001.github.io/records/assets/img/http/https_signature.jpg)

说明： Bob的内容实质上是明文传输的，所以这个过程是可以被人截获和窥探的，但是Bob不担心被人窥探，他担心的是内容被人篡改或者有人冒充自己跟Susan通信。这里其实涉及到了计算机安全学中的认证概念，Bob要向Susan证明通信的对方是Bob本人，另外也需要确保自己的内容是完整的。



5) Susan接收到了Bob的信，首先用Bob给的公钥对签名做了解密处理，得到了哈希值A，然后Susan用了同样的Hash算法对信的内容做了一次哈希处理，得到了另外一个哈希值B，对比A和B，如果这两个值是相同的，那么可以确认信就是Bob本人写的，并且内容没有被篡改过。

![https-signature-check](https://ivanzz1001.github.io/records/assets/img/http/https_signature_check.jpg)

说明： 4跟5其实构成了一次完整的通过数字签名进行认证的过程。数字签名的过程简述为：发送方通过不可逆算法对内容```text1```进行处理（哈希），得到的结果值为```hash1```，然后用私钥加密```hash1```得到结果值```encry1```; 对方接收到```text1```和```encry1```，用公钥解密```encry1```得到```hash1```，然后用```text1```进行相同的不可逆处理得到```hash2```，对```hash1```和```hash2```进行对比即可认证发送方。

6) 此时，另外一种比较复杂的情况出现了，Bob是通过网络把公钥寄送给他的三个好朋友的，有一个不怀好意的家伙```Jerry```截获了Bob给Doug的公钥。Jerry开始伪装成Bob跟Doug通信，Doug感觉通信的对象不像是Bob，但是他又无法确认。

![https-cheat](https://ivanzz1001.github.io/records/assets/img/http/https_cheat.jpg)


7) Bob最终发现了自己的公钥被Jerry截获了，他感觉自己的公钥通过网络传输给自己的小伙伴似乎也是不安全的，不怀好意的家伙可以截获这个明文传输的公钥。为此，他想到了去第三方权威机构```证书中心```(Certificate Authority,简称CA）做认证。证书中心用自己的私钥对Bob的公钥和其他信息做了一次加密。这样Bob通过网络将数字证书传递给他的小伙伴后，小伙伴们先用CA给的公钥解密证书，这样就可以安全获取Bob的公钥了。

![https-sign-data](https://ivanzz1001.github.io/records/assets/img/http/https_sign_data.jpg)


## 2. HTTPS通信过程

通过Bob与他的小伙伴的通信，我们已经可以大致了解一个安全通信的过程，也可以了解基本的加密、解密、认证等概念。HTTPS就是基于这样一个逻辑设计的。

首先看看组成HTTPS的协议： HTTP协议和SSL/TLS协议。HTTP协议就不用讲了，而SSL/TLS就是负责加密解密等安全处理的模块，所以HTTPS的核心在SSL/TLS上面。整个通信过程如下：

![https-work-flow](https://ivanzz1001.github.io/records/assets/img/http/https_work_flow.jpg)


1) 浏览器向服务器的443端口发起请求，请求携带了浏览器支持的```加密算法```和```哈希算法```

2) 服务器收到请求，选择浏览器支持的```加密算法```和```哈希算法```

3) 服务器将数字证书返回给浏览器，这里的数字证书可以是向某个可靠机构申请的，也可以是自制的

4） 浏览器进入数字证书认证环节，这一部分是浏览器内置的TLS完成的：
{% highlight string %}
 4.1) 首先，浏览器会从内置的证书列表中索引，找到服务器下发证书对应的机构，如果没有找到，此时就会提示用户该证书不是由权威机构
      颁发，是不可信任的。如果查到了对应的机构，则取出该机构颁发的公钥；

 4.2） 用机构的证书公钥解密得到证书的内容和签名，内容包括网站的网址、网站的公钥、证书的有效期等。浏览器会先验证证书签名的合法
    性（验证过程类似上面Bob和Susan的通信）。签名通过后，浏览器验证证书记录的网址是否和当前网址是一致的，不一致会提示用户。如
    果网址一致会查看证书的有效期，证书过期了也会提示用户。这些都通过认证时，浏览器就可以安全使用证书中的网站公钥了。

 4.3） 浏览器生成一个随机数R， 并使用网站的公钥对R进行加密
{% endhighlight %}
 
5） 浏览器将加密的R传送给服务器

6） 服务器用自己的私钥解密得到R

7） 服务器以R为密钥使用了对称加密算法加密网页内容并传输给浏览器

8） 浏览器以R为密钥使用之前约定好的解密算法获取网页内容



备注：
<pre>
1） 前5步其实就是HTTPS的握手过程，这个过程主要是认证服务器端证书（内置的公钥）的合法性。因为非对称加密计算量较大，整个通信过程
    只会用到一次非对称加密算法（主要是用来保护传输客户端生成的用于对称加密的随机数私钥）。后续内容的加解密都是通过一开始约定好
    的对称加密算法进行的。

2） SSL/TLS是HTTPS安全性的核心模块，TLS的前身是SSL， TLS1.0就是SSL3.1， TLS1.1就是SSL3.2， TLS1.2就是SSL3.3。SSL/TLS
    是建立在TCP协议之上的，因而也是应用层级别的协议。其包括TLS Record Protocol和TLS Handshaking Protocol两个模块，后者
    负责握手过程中的身份认证，前者则保证数据传输过程中的完整性和私密性。 
</pre>


## 3. HTTPS通信过程详解

先来看下整个SSL/TLS的握手过程，之后我们再分步骤详细解读，每一步都干了些什么.

![https-handshake](https://ivanzz1001.github.io/records/assets/img/http/https_handshake.png)

### 3.1 客户端发送ClientHello

当TCP建立连接之后，TLS握手的第一步由客户端发起，发送ClientHello的消息到服务器。ClientHello消息包含：

* 客户端支持的SSL/TLS版本

* 客户端支持的加密套件(Cipher Suites)

* 会话Idsession id（如果有的值的话，服务器端会复用对应的握手信息，避免短时间内重复握手）

* 随机数client-random

*延伸阅读*

加密套件名如```TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA256```，这么长的名字看着有点晕吧，不用怕，其实它的命名非常规范，格式很固定。

>基本的形式是: 密钥交换算法-服务身份验证算法-对称加密算法-握手校验算法

握手过程中，证书签名使用的RSA算法，如果证书验证正确，再使用ECDHE算法进行密钥交换，握手后的通信使用的是AES256的对称算法分组模式是GCM。验证证书签名合法性使用SHA256作哈希算法检验。相关的算法的用处将在后文中详解。


### 3.2 服务端发送ServerHello响应
服务器端在收到这个ClientHello，从中选择服务器支持的版本和套件，发送ServerHello消息：

* 服务器所能支持的最高SSL/TLS版本

* 服务器选择的加密套件

* 随机数server-random

* 会话Idsession id(用于下次复用当前握手的信息，避免短时间内重复握手。)


随后服务器发送服务器的安全证书(含公钥)。

如果需要客户端也提供证书的话，还会发出客户端证书请求(Client Certificate Request)，只有少数金融机构才需要客户端也提供客户端证书。

此后客户端发送Server Hello Done消息表示Hello阶段完成。


### 3.3 校验证书

客户端收到ServerHello后，会对收到的证书进行验证。下面我们来看一下数字证书的生成及校验：

![https-handshake](https://ivanzz1001.github.io/records/assets/img/http/https_certificate_check.webp)

1） CA签发证书的过程

如上图左边部分:

* 首先CA会把持有者的公钥、用途、颁发者、有效时间等信息打成一个包，然后对这些信息进行 Hash 计算，得到一个 Hash 值；

* 然后 CA 会使用自己的私钥将该 Hash 值加密，生成 Certificate Signature，也就是 CA 对证书做了签名；

* 最后将 Certificate Signature 添加在文件证书上，形成数字证书；


2）客户端校验服务端的数字证书的过程

如上图右边部分：

* 首先客户端会使用同样的Hash算法获取该证书的 Hash 值 H1；

* 通常浏览器和操作系统中集成了CA的公钥信息，浏览器收到证书后可以使用CA的公钥解密 Certificate Signature 内容，得到一个Hash值H2 ；

最后比较H1和H2，如果值相同，则为可信赖的证书，否则则认为证书不可信。

>ps: 当然还需校验证书中服务器名称是否合法以及验证证书是否过期

这样就免受中间人攻击了，因为假如有中间人修改了证书的内容（如将证书中的公钥替换成自己的公钥），那么将获得不同的哈希值，从而两个哈希值不匹配导致验证失败。如果要绕过这个机制，中间人必须要也替换签名，使签名也相匹配。而做到这一点就需要破解到了根证书的密钥（而这是不可能的，中间人必然会失败）。浏览器会出现以下画面，告诉你正在遭受中间人攻击，因为证书被篡改了：

![https-modify](https://ivanzz1001.github.io/records/assets/img/http/https_modify.png)

那聪明的你肯定也想到了，如果你开发了一个系统还在测试阶段，还没有正式申请一张证书，那么你可以为服务器自签名一张证书，然后将证书导入客户端的CA信任列表中。



3） 信任链机制

我们来看一下为什么可以通过CA(Certificate Authority，证书颁发机构)签发的证书来确认网站的身份？

当我们安装操作系统或者浏览器的时候，会安装一组可信任的CA（根证书CA包括GlobalSign、GeoTrust、Verisign等）列表。根CA如GlobalSign就在我们的可信任的CA列表里，你的浏览器或者操作系统含有GlobalSign的公钥。

先来看一下Google的证书，当你访问Google的时候，Google会发给你它的证书。证书中包含颁发机构的签名以及服务器的公钥:

![https-handshake](https://ivanzz1001.github.io/records/assets/img/http/https_signature_show.png)

![https-handshake](https://ivanzz1001.github.io/records/assets/img/http/https_ca_chain.png)


可以看到证书路径是:GlobalSign Root CA-R2 -> GTS CA 1O1->*.google.com。

因为我们的浏览器信任GlobalSign Root CA，根据信任链机制，你相信了根CA颁发的证书，也要相信它签名的子CA颁发的证书，也要相信子CA签名的子子CA的证书…而我们通过一级级的校验，如果从根证书到最下层的证书都没有被篡改过，我们就相信最下层的这个服务器证书是合法的。所以在这个机制中，你就需要无条件的相信根证书的颁发机构。

### 3.4 密钥交换过程
如果通过上面证书的验证，就可以进入密钥交换过程: 

* 客户端随机生成pre-master，然后用上面```步骤3```中获取到的服务器公钥对其进行加密，发送给服务端

* 服务端用自己的私钥进行解密，获取到明文的pre-master

注意，客户端和服务端并不是直接用pre-master来进行数据加解密的，而是采用```master secret```(server-random + client-random + pre-master)。为什么不能只用一个pre-master作为之后加密的对称密钥？

虽然只有服务器有私钥，能够解密pre-master呀，但仅用它作为master secret是不够安全的，这是因为要以防客户端的pre-master并不是随机数的情况。加上另外两个随机数client-random以及server-random（而这两个随机数和时间有相关性），这样就能保证最后生成的master secret一定是随机数。


### 3.5 完成握手

参看本节第一张图的步骤7)、步骤8)，进行如下完成握手动作：

* 客户端用master secret加密了一条握手完成的消息发送给服务器。

* 服务器端也回发了一条用master secret加密的握手完成的消息

完成握手后，就可以用master secret对信息进行加密传输了。


<br />
<br />

**[参考]**


1. [深入浅出HTTPS工作原理](https://blog.csdn.net/wangtaomtk/article/details/80917081)

2. [HTTPS 的工作原理](https://blog.csdn.net/kevinxxw/article/details/105931263)

3. [客户端证书、证书链验证](https://www.jianshu.com/p/2227ed9c4afa)

<br />
<br />
<br />

