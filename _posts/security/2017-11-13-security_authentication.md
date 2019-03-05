---
layout: post
title: 安全认证之kerberos协议
tags:
- security
categories: security
description: 安全认证之kerberos协议
---


本文主要介绍一下在安全认证方面经常用到的kerberos协议。


<!-- more -->


## 1. Kerberos协议
客户端通过向认证服务器(Authentication Server, AS)上的```KDC```(key distribution center)发送用户身份信息以进行认证。KDC会返回一个```TGT```(ticket-granting ticket)给客户端，该```TGT```中包含时间戳(timestamp)信息，并且经过了```TGS(ticket-granting service)秘钥```的加密。此一过程通常发生并不频繁，一般发生在用户登录过程；TGT通常会在未来的某个时间段内过期，但是可以在过期时间段内由客户端的session manager进行更新。

当客户端需要和另外一个节点上的某个服务(service)进行通信的时候，客户端会发送TGT到TGS(注： TGS通常和KDC处于同一台主机上）。以SPN命名规则(Service Principal Name)命名的service必须在TGT中进行了登记。客户端使用SPN来请求访问这个服务。在成功校验TGT的有效性之后，用户被允许访问所请求的服务，TGS会向客户端传递```ticket```和```session keys```。然后客户端拿着这个ticket向```SS```(Service Server)请求相应的服务。

![security-kerberos](https://ivanzz1001.github.io/records/assets/img/security/security_kerberos.jpg)


如下详细描述该协议：

### **1.1 User Client-based Logon**

1) 在客户机上用户输入```用户名```,```密码```。(其他一些认证机制可能允许使用一个public key来代替密码）

2) 客户端使用对称加密算法将```密码```(password)转换成```key```值。(可以使用内置的key产生算法，或者单向hash来产生，这取决于所使用的密码套件）

<br />

### **1.2 Client Authentication**

1) 客户端以明文```user ID```的方式发送消息到AS(Authentication Server)以请求```services```（注意：注意这里既不发送```secret key```，也不发送密码到AS)


2) AS检查该客户是否在数据库中。假如客户在数据库中，则AS对从数据库中找到的password进行hash以产生```secret key```，然后发送回客户端如下两个消息：

* Message A: ```Client/TGS Session Key```，该session key用client/user的```secret key```进行了加密

* Message B: ```Ticket-Granting-Ticket```(TGT,里面通常含有client ID，client network address，ticket validity period, and client/TGS session key), 其用TGS的secret key进行了加密。


3) 一旦客户端收到Message A 和 Message B，就会使用客户端输入的password产生的secret key来对Message A进行解密。假如用户输入的密码并不匹配AS数据库中保存的密码，则产生的客户端secret key将会不同，因此就不能对Message A进行解密； 假如用户输入了正确的password，则可以产生正确的secret key，然后用该secret key可以成功的对Message A进行解密以获取到```Client/TGS Session Key```。该session key被用于后续与TGS进行通信。(注意：客户端并不能够对Message B进行解密，因为它是使用TGS的secret key进行过加密）到此为止，客户端已经有足够的信息来向TGS来认证自己。

<br />

### **1.3 Client Service Authorization**

1) 当要请求```services```的时候，客户端发送如下消息到TGS

* Message C: 将Message B返回过来的加密过的TGT以及所请求service的ID(service-id)打包在一起

* Message D: 使用```Client/TGS Session Key```加密过的Authenticator(包含了client ID以及timestamp信息)


2) 当TGS收到Message C和Message D之后，TGS首先从Message C中分离出Message B，然后使用TGS的```secret key```对Message B进行解密，这样就可以获得其中的```client/TGS session key```。然后TGS使用该key值来对Message D(Authenticator)进行解密，并且对比来自Message C和Message D中的client ID，假如匹配的话则发送如下两个消息到客户端：

* Message E: ```Client-to-server ticket```(其中包括client ID，client network address, validity period, and client/Server Session Key)，该ticket被service的```secret key```加密过

* Message F: 被```Client/TGS Session Key```加密过的```Client/Server Session Key```

<br />

### **1.4 Client Service Request**

1) 当客户端从TGS接收到Message E和Message F之后，client已经有足够的信息向Service Server(SS)完成自我认证。 客户端连接上SS然后发送如下两条消息：

* Message E: 上一个步骤接收到的Message E(被service加密过的```client-to-server ticket```)

* Message G: 一个```新的Authenticator```， 其中包含client ID,timestamp。并使用```Client/Server Session Key```对其进行加密



2) SS使用其自己的secret key对ticket(Message E)进行解密，以获得```client/Server Session Key```。SS使用该session key对Authenticator进行解密，然后对比Message E和Message G中的client ID。假如匹配的话， SS发送如下信息给客户端以确认成功进行身份验证并且准备为客户端提供服务：

* Message H: client的Authenticator中的timestamp（在Kerberos v4版本中需要对timestamp进行加1，而kerberos v5版本不需要），使用```Client/Server Session Key```进行加密


3) 客户端使用```Client/Server Session Key```对来自SS的确认消息Message H进行解密，并对比其中的timestamp是否正确。假如正确的话，则客户端信任该Server，然后就可以向该Server发送服务请求


4) Server向client提供所请求的服务


## 1.5. Kerberos协议相关交互总结

**1） 客户端相关概念**

* client_name: 客户端的用户名

* client_id: 用户登录时服务器根据用户名返回过来的用户ID

* client_passwd: 用户输入的密码

* client_secret: 根据用户输入的密码产生的秘钥,例如```client_secret = hash(client_passwd)```

**2) TGS相关概念**

```TGS```是ticket-granting service的缩写，提供TGT以及ticket相关的授予及校验服务： ```tgs_secret_key``` 以及相应service的```service_secret_key```
  

**3) 相关消息**

Message A: TGS返回给Client的经过加密的```Client/TGS Session Key```
<pre>
Message A = Encrypt(Client/TGS Session key, client_secret)
</pre>


Message B: TGS返回给Client的经过加密的TGT
<pre>
TGT={client ID，client network address，ticket validity period, client/TGS session key}

Message B = Encrypt(TGT,tgs_secret_key)
</pre>

Message C: 客户端发送给TGS的Message B以及相应服务的service ID
<pre>
Message C = {Message B, service ID}
</pre>

<br />
<br />

**【参看】：**

1. [Kerberos (protocol)](https://en.wikipedia.org/wiki/Kerberos_(protocol))

2. [Kerberos原理和使用](http://blog.csdn.net/kkdelta/article/details/46633557)


<br />
<br />
<br />


