---
layout: post
title: go https用法
tags:
- go-language
categories: go-language
description: go https用法
---

本文讲述一下go https的用法。

<!-- more -->

## 1. 相关概念

### 1.1 http
http是一个客户端与服务端请求及应答的一个基于tcp传输的标准协议。浏览器通过http协议向一个服务器发送请求，服务器接收到请求之后经过一系列的处理将响应结果返回给浏览器，此时浏览器网页上便获得我们所需要的内容。

但是基于http协议浏览器与服务器传输数据的过程中，数据是明文的。在21世纪网络发达的今天，明文数据极易被截获修改，对安全性造成了很大的隐患。因安全要求有必要对传输数据进行加密，因而https协议便由此诞生。


### 1.2 https

https协议是基于http协议基础上的一种安全（基于ssl或者tls安全加密）传输协议。数字证书主要由两部分组成：

* ```C```： 证书相关信息（自身公钥信息、过期时间、ca名称、证书签名算法...)

* ```S```: 证书的数字签名(其实就是对证书```C部分```的内容进行哈希运算)

https工作的主要流程如下：

1) 浏览器（客户端）请求一个安全页面（通过https://开头）

2) WEB服务器返回它的数字证书（里面包含公钥信息）

3) 浏览器校验证书是由可信的机构颁发的（通常是可信的根CA），证书仍然有效并且证书与被访问的网站相关

4） 浏览器从得到的证书中提取公钥，然后执行如下步骤：

* 通过公钥来加密（一般是rsa非对称加密）一个随机的对称秘钥（每次https连接都会产生一个随机对称秘钥，对称加密算法是客户端与服务端协商确定的）
<pre>
注： 上面客户端与服务端协商确定的对称加密算法可能每一次都会改变
</pre>

* 使用客户端、服务器端都知道的对称加密算法， 来加密url及要发送的数据

* 客户端将经过公钥非对称加密过的秘钥、以及上面对称加密过的url、数据发送给服务端

5） 服务端用私钥解密对称秘钥， 然后再用该对称秘钥解密浏览器发送过来的url及http数据

6) 服务端使用```步骤5)```获得的对称秘钥来加密响应内容，然后返回给客户端

7） 客户端使用对称秘钥解密响应内容，并展示给用户


## 2. 示例
当前操作环境为：
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
# uname -a
Linux compile 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# ifconfig
ifconfig
eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 10.133.146.250  netmask 255.255.255.0  broadcast 10.133.146.255
        inet6 fe80::5265:f3ff:fe2e:c45e  prefixlen 64  scopeid 0x20<link>
        ether 50:65:f3:2e:c4:5e  txqueuelen 1000  (Ethernet)
        RX packets 137684569  bytes 22559628818 (21.0 GiB)
        RX errors 0  dropped 1  overruns 0  frame 0
        TX packets 77030309  bytes 21092148463 (19.6 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 20  memory 0xef100000-ef120000  
</pre>

### 2.1 示例1

本示例客户端并不会对服务器端证书进行校验。

1) 产生自签名证书

这里首先生成服务端自签名证书：
<pre>
openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=10.133.146.250/emailAddress=11111111@qq.com"
</pre>

上面注意```CN```必须是我们上面的服务器IP地址。下面是服务器端及客户端相应实现代码：

2) 服务端代码server.go
{% highlight string %}
package main

import(
	"net/http"
	"fmt"
	"io"
)

func handler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Hi, This is an example of https service in golang!")
	io.WriteString(w, "Hi, This is an example of https service in golang!")
}

func main() {
	http.HandleFunc("/", handler)
	http.ListenAndServeTLS("10.133.146.250:18443",
		"cert.crt",   //服务端证书， 包含服务端公钥信息
		"rsa_private.key",   //服务端私钥
			nil)
}
{% endhighlight %}

3) 客户端代码client.go
{% highlight string %}
package main


import (
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/http"
)
func main() {

	/*
		client与server进行通信时 client也要对server返回数字证书进行校验
		因为server自签证书是无效的 为了client与server正常通信
		通过设置客户端跳过证书校验
		TLSClientConfig:{&tls.Config{InsecureSkipVerify: true}
		true:跳过证书校验
	*/
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}

	client := &http.Client{Transport: tr}
	resp, err := client.Get("https://10.133.146.250:18443")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	fmt.Println(string(body))
}
{% endhighlight %}
 
4） 验证

* 服务端运行程序：
<pre>
# go run server.go
</pre>

* 浏览器访问```https://10.133.146.250:18443```，在证书检查之后可以看到能够访问

* 客户端执行如下命令
<pre>
# go run client.go 
Hi, This is an example of https service in golang!
</pre>


### 2.2 示例2
本示例，我们会对服务端证书进行校验。这里首先讲述一下校验的基本原理：
<pre>
1. 浏览器本身内置了一些权威的CA（证书授权机构其实也是一个数字证书）

2. CA证书自身也包含自己的公钥信息，及证书本身的数字签名

3. 客户端对来自服务端证书的校验就是使用CA证书来进行的， 查看来自服务端的证书是否是CA签发的。具体步骤如下：
   1） 客户端利用自身CA证书中的签名算法对服务端证书内容部分（C部分）进行相应的哈希运算得到哈希值（也就是对内容利用
       自身的哈希算法进行签名）
   2） 客户端利用得到的哈希值与服务端数字证书的签名进行比较，若相同则说明服务端证书是由该CA颁发的； 否则不是该CA颁发的
</pre>

接下来我们给出操作步骤：

1） 产生自签名根证书
<pre>
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=ivan1001/emailAddress=1181891136@qq.com"

# ls
cert.crt  rsa_private.key
</pre>
上面```CN```作为测试可以随便指定一个值。这里我们生成了根证书```cert.crt```及私钥```rsa_private.key```。

2） 生成服务端证书

首先我们使用RSA私钥生成CSR签名请求：
<pre>
# openssl genrsa -out server.key 2048 
# openssl req -new -key server.key -out server.csr -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=10.133.146.250/emailAddress=11111111@qq.com"

# ls
cert.crt  rsa_private.key  server.csr  server.key
</pre>
上面我们生成了```server.csr```证书签名请求，和私钥```server.key```。然后将生成的 csr签名请求文件可提交至 CA进行签发:
<pre>
# echo subjectAltName = IP:10.133.146.250 > extfile.cnf
# openssl x509 -req -days 3650 -in server.csr -CA cert.crt -CAkey rsa_private.key -CAcreateserial -extfile extfile.cnf -out server.crt
# ls
cert.crt  cert.srl  rsa_private.key  server.crt  server.csr  server.key
</pre>

上面我们生成了由CA签发的服务端数字证书```server.crt```(上面注意： 对于CN为ip地址的，请加上```-extfile```选项)。

3) 服务端代码server.go
{% highlight string %}
package main

import (
	"fmt"
	"net/http"
	"io"
)
func handler(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Hi, This is an example of https service in golang!")
	io.WriteString(w, "Hi, This is an example of https service in golang!")
}

func main() {
	http.HandleFunc("/", handler)
	http.ListenAndServeTLS("10.133.146.250:18443",
		"server.crt", "server.key", nil)
}
{% endhighlight %}

4) 客户端代码
{% highlight string %}
package main

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net/http"
)

/*
 客户端若要对服务端的数字证书进行校验 需发送请求之前 加载CA证书
*/
func main() {
	pool := x509.NewCertPool()
	caCertPath := "cert.crt"    //根证书

	caCrt, err := ioutil.ReadFile(caCertPath)
	if err != nil {
		fmt.Println("ReadFile err:", err)
		return
	}
	pool.AppendCertsFromPEM(caCrt) //客户端添加ca证书

	tr := &http.Transport{
		TLSClientConfig: &tls.Config{RootCAs: pool}, //客户端加载ca证书
		DisableCompression: true,
	}
	client := &http.Client{Transport: tr}
	resp, err := client.Get("https://10.133.146.250:18443")
	if err != nil {
		fmt.Println("Get error:", err)
		return
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	fmt.Println(string(body))
}
{% endhighlight %}

5) 校验

* 服务端运行程序：
<pre>
# go run server.go
</pre>

* 通过浏览器访问： 这里首先需要将根证书```cert.crt```导入到浏览器，成为受信任的根证书，然后重启浏览器，访问https://10.133.146.250:18443， 可以看到可以正常访问.
<pre>
注意： 对于chrome浏览器，需要采用 sha256+rsa进行自签名

# openssl req -newkey -sha256 rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=ivan1001/emailAddress=1181891136@qq.com"
</pre>


* 运行客户端程序：
<pre>
# go run client.go 
Hi, This is an example of https service in golang!
</pre>








<br />
<br />
**[参看]：**

1. [https原理通俗理解及golang实现](https://www.cnblogs.com/gccxl/p/7127853.html)

2. [HTTPS那些事 用java实现HTTPS工作原理](http://kingj.iteye.com/blog/2103662#)

<br />
<br />
<br />

