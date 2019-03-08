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

1. 首先Bob弄到了两把钥匙： 公钥和私钥

![https-pubpriv-key](https://ivanzz1001.github.io/records/assets/img/http/https_pubpriv_key.jpg)


2. Bob自己留下私钥，把公钥复制成三份送给了他的三个好朋友Pat、Doug和Susan。

![https-pubkey-distribute](https://ivanzz1001.github.io/records/assets/img/http/https_pubkey_distribute.jpg)


3. 此时，Bob总算可以安心地和他的好朋友愉快地通信了。比如Susan要和他讨论关于去哪吃午饭的事情，Susan就可以先把自己的内容（明文）首先用Bob送给他的公钥做一次加密，然后把加密的内容传送给Bob。Bob收到信后，再用自己的私钥解开信的内容。

![https-info-transfer](https://ivanzz1001.github.io/records/assets/img/http/https_info_transfer.jpg)

说明： 这其实是计算机安全学里的加密的概念，加密的目的是为了不让别人看到传送的内容，加密的手段是通过一定的加密算法及约定的密钥进行的（比如上述用了非对称加密算法以及Bob的公钥），而解密则需要相关的解密算法及约定的密钥（如上述用了非对称加密算法和Bob自己的私钥），可以看出加密是可逆的（可解密的）。


4. Bob看完信后，决定给Susan回一封信。为了防止信的内容被篡改（或者别人伪装成他的身份跟Susan通信），他决定先对信的内容用hash算法做一次处理，得到一个字符串哈希值，Bob又用自己的私钥对哈希值做了一次加密得到一个签名，然后把签名和信（明文的）一起发送给Susan。

![https-signature](https://ivanzz1001.github.io/records/assets/img/http/https_signature.jpg)




<br />
<br />

**[参考]**


1. [深入浅出HTTPS工作原理](https://blog.csdn.net/wangtaomtk/article/details/80917081)



<br />
<br />
<br />

