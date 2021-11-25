---
layout: post
title: Linux中openssl的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux中openssl的使用
---


openssl是目前最流行的SSL密码库工具，其提供了一个通用、健壮、功能完备的工具套件，用以支持SSL/TLS协议的实现。官网：[https://www.openssl.org/source/](https://www.openssl.org/source/)。其主要有以下借个部分构成：

* 密码算法库

* 秘钥和证书封装管理功能

* SSL通信API接口

主要用途有：

* 建立 RSA、DH、DSA key 参数

* 建立X.509证书、证书签名请求（CSR）和CRLs（证书回收列表）

* 计算消息摘要

* 使用各种Cipher加密/解密

* SSL/TLS客户端以及服务器的测试

* Https/MIME或者邮件加密


<!-- more -->

## 1. 密钥、证书请求、证书概要说明
在证书申请签发过程中，客户端涉及到秘钥、证书请求、证书这几个概念，初学者可能会搞不清楚这三者的关系。我们以申请证书的流程说明三者的关系。客户端（相对于CA）在申请证书的时候，大体上有三个步骤：
<pre>
第一步：  生成客户端的额秘钥， 即客户端的公私钥对，切要保证私钥只有客户端自己拥有。

第二步： 以客户端的秘钥和客户端的自身信息（国家、机构、域名、邮箱等）为输入，生成证书请求文件。其中客户端公钥和客户端信息是明文保存在证书请求文件中的；
        而客户端私钥的作用是对客户端公钥及客户端信息做签名，自身是不包含在证书请求中的。然后把证书请求文件发送给CA机构。
（注意： 这里明文的意思是指没必要加密，用户可以明文形式保存在证书请求文件中，具体做法是生成证书请求时添加-pubkey选项）

第三步： CA机构接收到客户端的证书请求文件后，首先校验其签名，然后审核客户端的信息，最后CA机构使用自己的私钥为证书请求文件签名，生成证书文件，下发给客户端。
        此证书就是客户端的身份证，来表明用户的身份。
</pre>
至此客户端申请证书流程结束，其中涉及到证书签发机构CA，CA是被绝对信任的机构。如果把客户端证书比作用户身份证，那么CA就是颁发身份证的机构，我们以```https```为例说明证书的用处：

为了数据传输安全，越来越多的网站启用https。在https握手阶段，服务器首先把自己的证书发送给用户（浏览器），浏览器查看证书中的发证机构，然后在机器内置的证书中（在PC机或者手机上，内置了世界上著名的CA机构的证书）查找对应的CA证书，然后使用内置的证书公钥校验服务器证书的真伪。如果校验失败，浏览器会提示服务器证书有问题，询问用户是否继续。

例如12306网站，它使用自签名的证书，所以浏览器会提示证书有问题，在12306的网站上有提示下载安装根证书，其用户就是把该根证书安装到用户的内置证书中，这样浏览器就不会报证书错误。但是注意，除非特别相信某个机构，否则不要在机器上随便导入证书，很危险。

## 2. openssl安装

这里我们讲述一下Openssl的源代码安装（Centos7.4):

1） **下载源代码**

到openssl官方网站:[https://www.openssl.org/source/](https://www.openssl.org/source/)，下载稳定版本的openssl。当前稳定版本为```1.1.0```:
<pre>
# wget https://www.openssl.org/source/openssl-1.1.0g.tar.gz
# tar -zxvf openssl-1.1.0g.tar.gz
# cd openssl-1.1.0g/
</pre>

2) **编译**
<pre>
# mkdir -p /usr/local/openssl
# ./config -fPIC --prefix=/usr/local/openssl/ enable-shared   

# make
# make install
# ls /usr/local/openssl
bin  include  lib  share  ssl
</pre>
这里：
* ```--prefix```：指定安装目录

* ```-fPIC```:编译openssl的静态库

* ```enable-shared```:编译动态库

3) **查看openssl依赖关系**
<pre>
# /usr/local/openssl/bin/openssl --help
/usr/local/openssl/bin/openssl: error while loading shared libraries: libssl.so.1.1: cannot open shared object file: No such file or directory
# ldd /usr/local/openssl/bin/openssl 
        linux-vdso.so.1 =>  (0x00007ffc0d5f6000)
        libssl.so.1.1 => not found
        libcrypto.so.1.1 => not found
        libdl.so.2 => /lib64/libdl.so.2 (0x00007f819423a000)
        libpthread.so.0 => /lib64/libpthread.so.0 (0x00007f819401e000)
        libc.so.6 => /lib64/libc.so.6 (0x00007f8193c5c000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f8194454000)
</pre>
这里我们看到，有一些库并没有找到。因此这里我们需要设定环境变量:
<pre>
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/openssl/lib
# /usr/local/openssl/bin/openssl --help
Invalid command '--help'; type "help" for a list.
</pre>
因为这里我们系统中有另外一个openssl，这里我们就不真正把该路径配置到配置文件中了：
<pre>
# which openssl
/usr/bin/openssl
</pre>

## 3. 示例

### 3.1 RSA秘钥操作
默认情况下，openssl输出格式为： ```PKCS#1-PEM```

1) **生成RSA私钥(无加密)**
<pre>
# openssl genrsa -out rsa_private.key 2048
Generating RSA private key, 2048 bit long modulus
..............................+++
..........................................+++
e is 65537 (0x10001)

//生成的私钥文件
# ls
rsa_private.key
</pre>

2) **生成RSA公钥**
<pre>
# openssl rsa -in rsa_private.key -pubout -out rsa_public.key
writing RSA key

# ls
rsa_private.key  rsa_public.key
</pre>

3) **生成RSA私钥（使用aes256加密)**

因为有时候我们不想别人看到我们的明文私钥，这时候我们可以对产生的RSA私钥进行加密。这样即使我们的私钥加密文件泄露，私钥还是安全的：
<pre>
# openssl genrsa -aes256 -passout pass:111111 -out rsa_aes_private.key 2048
Generating RSA private key, 2048 bit long modulus
............+++
.........................+++
e is 65537 (0x10001)

# ls
rsa_aes_private.key  
</pre>
其中 passout 代替shell 进行密码输入，否则会提示输入密码。此时如果基于此，要生成**公钥**，需要提供密码：
<pre>
# openssl rsa -in rsa_aes_private.key -passin pass:111111 -pubout -out rsa_public.key
</pre>

### 3.2 openssl转换命令

1) **对加密的私钥进行解密**

首先我们使用```aec256方法```，产生一个加密的私钥，然后再对其进行解密：
<pre>
# ls

# openssl genrsa -aes256 -passout pass:111111 -out rsa_aes_private.key 2048         //产生加密私钥
# ls
rsa_aes_private.key

# openssl rsa -in rsa_aes_private.key -passin pass:111111 -out rsa_private.key     //解密后的私钥
writing RSA key
# ls
rsa_aes_private.key  rsa_private.key
</pre>

2) **对明文私钥进行加密**

这里我们对上文解密后的私钥```rsa_private.key```再进行手动加密，看得出的结果是否与原来的```rsa_aes_private.key```一致：
<pre>
# ls

# openssl genrsa -out rsa_private.key 2048                        //产生明文私
Generating RSA private key, 2048 bit long modulus
............................................+++
................................................................+++
e is 65537 (0x10001)

# openssl rsa -in rsa_private.key -aes256 -passout pass:111111 -out rsa_aes_private.key    //对明文私钥进行加密
writing RSA key
# ls
rsa_aes_private.key  rsa_private.key

#  openssl rsa -in rsa_aes_private.key -passin pass:111111 -out rsa_private_2.key          //在对加密后的私钥进行解密
writing RSA key

# diff rsa_private.key rsa_private_2.key                                //对比，发现一模一样
</pre>

3) **私钥PEM转DER**

默认产生的公、私钥都是```PEM```格式的，现在转换成```DER```格式：
<pre>
#ls

# openssl genrsa -out rsa_private.key 2048                              //产生一个明文私钥
# openssl rsa -in rsa_private.key -outform der -out rsa_private.der     //转换成DER格式
writing RSA key

# ls
rsa_private.der  rsa_private.key
</pre>


4) **查看私钥明细**
<pre>
# openssl genrsa -out rsa_private.key 2048         //产生一个明文私钥
# openssl rsa -in rsa_private.key -noout -text
Private-Key: (2048 bit)
modulus:
    00:bf:3b:ce:56:56:3b:f9:16:8c:31:6e:6d:1d:10:
    b1:3b:57:34:99:e9:f0:10:aa:06:a6:a7:de:59:23:
    cb:09:ab:31:e9:b0:e7:ab:a1:30:9c:32:8d:9f:a0:
    c2:a3:e7:3e:f6:76:26:a5:14:56:56:04:cb:5e:3a:
    c7:84:f2:51:1a:53:09:dd:91:5c:24:e7:53:25:bb:
    ed:51:0e:6d:04:71:4b:bb:58:86:26:d2:bd:c4:bc:
    f7:7e:dc:ef:79:ed:b6:00:e7:d4:25:dc:10:5f:27:
    7d:51:8b:1b:b4:36:80:4d:8a:3c:28:6f:71:f4:92:
    3a:65:de:17:2a:92:d4:03:7a:2d:46:b2:3c:c1:cf:

.....
</pre>

这里将```-in```参数替换成```-pubin```参数，就可以查看公钥明细.

5) **私钥PKCS#1转PKCS#8**
<pre>
# openssl genrsa -out rsa_private.key 2048         //产生一个明文私钥(传统私钥格式PKCS#1,即rsa)
# openssl pkcs8 -topk8 -in rsa_private.key -passout pass:111111 -out pkcs8_private.key  (rsa转PKCS#8)
</pre>
其中-passout指定了密码，输出的pkcs8格式密钥为加密形式，pkcs8默认采用des3 加密算法。使用```-nocrypt```参数可以输出无加密的pkcs8密钥，如下：
<pre>
# openssl pkcs8 -topk8 -in rsa_private.key -nocrypt -out nocrypt_pkcs8_private.key
</pre>


### 3.3 生成自签名证书

1) **生成RSA私钥和自签名证书**
<pre>
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt
Generating a 2048 bit RSA private key
............+++
.........................................................+++
writing new private key to 'rsa_private.key'
-----
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [XX]:CN
State or Province Name (full name) []:Guangdong
Locality Name (eg, city) [Default City]:Shenzhen
Organization Name (eg, company) [Default Company Ltd]:test_company
Organizational Unit Name (eg, section) []:IT
Common Name (eg, your name or your server's hostname) []:test_name
Email Address []:11111111@qq.com

# ls
cert.crt  rsa_private.key
</pre>
上面我们输入了：

* Country Name

* State/Province Name

* Locality Name(Default city)

* Organizational Name(Default Company Ltd)

* Organizational Unit name(section)

* Common Name(your name or your server's hostname)    

* Email Address

如果想要省去这些输入，可以用```-subj```选项：
<pre>
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
</pre>

我们再对上面的命令做一个简单的说明： ```req```是证书请求的子命令，```-newkey rsa:2048 -keyout private_key.pem``` 表示生成私钥(PKCS8格式)，```-nodes``` 表示私钥不加密，若不带参数将提示输入密码；
```-x509```表示输出证书，```-days365``` 为有效期。

<br />

2) **使用 已有RSA 私钥生成自签名证书**
<pre>
# openssl genrsa -out rsa_private.key 2048     //产生一个明文私钥
# openssl req -new -x509 -days 365 -key rsa_private.key -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
# ls
cert.crt  rsa_private.key
</pre>

```-new```指生成证书请求，加上-x509 表示直接输出证书，```-key```指定私钥文件，其余选项与上述命令相同


### 3.4 生成签名求情及CA签名

1) **使用RSA私钥生成CSR签名请求**
<pre>
# openssl genrsa -aes256 -passout pass:111111 -out server.key 2048      //生成aes256加密的rsa私钥
# ls
server.key

# openssl req -new -key server.key  -out server.csr                //生成CSR签名请求
Enter pass phrase for server.key:
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [XX]:CN
State or Province Name (full name) []:Guangdong
Locality Name (eg, city) [Default City]:Shenzhen
Organization Name (eg, company) [Default Company Ltd]:test_company
Organizational Unit Name (eg, section) []:IT
Common Name (eg, your name or your server's hostname) []:test_name
Email Address []:11111111@qq.com

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
An optional company name []:

# ls
server.csr  server.key
</pre>
如上分别输入了解密```server.key```的密码```111111```; 然后是```subject```主题信息；再接着会要求输入一个```challenge```密码（此密码可以为空）。也可以采用如下命令，简化输入：
<pre>
# openssl genrsa -aes256 -passout pass:111111 -out server.key 2048 

# openssl req -new -key server.key -passin pass:111111 -out server.csr -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
</pre>

注意这里可以增加```-pubkey```选项，使客户端公钥和客户信息明文保存在证书请求文件中：
<pre>
# openssl genrsa -aes256 -passout pass:111111 -out server.key 2048 
# openssl req -new -key server.key -passin pass:111111 -out server.csr -pubkey -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
</pre>

***```此时生成的 csr签名请求文件可提交至 CA进行签发```***


2) **查看CSR 的细节**
<pre>
# cat server.csr
-----BEGIN CERTIFICATE REQUEST-----
MIIC0jCCAboCAQAwgYwxCzAJBgNVBAYTAkNOMRIwEAYDVQQIDAlHdWFuZ2Rvbmcx
ETAPBgNVBAcMCFNoZW56aGVuMRUwEwYDVQQKDAx0ZXN0X2NvbXBhbnkxCzAJBgNV
BAsMAklUMRIwEAYDVQQDDAl0ZXN0X25hbWUxHjAcBgkqhkiG9w0BCQEWDzExMTEx
MTExQHFxLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMjMzntn
V8IeDtOq78iVnaUtrtEfho2ugzD3ozV7/YQVwpj8NLK8tNKzL84kqENBFDExWGZv
eo1/eSpxrSOE36QLrjlizhT365B9bRXUyqlg53J6EvecVxCmhTfaYNIu0LVtJSdw
IGhBjjfOL004xPkwOvaGvQs149G6lADH+lIJPJVlOPb3OcLoZUJka96c4699REmI
M9RJIGuiltix7TLLrQhjRh9U/jAO/s9K7VdezAzO2N+7ObY27I9HHhjmZ9wPMPlJ
Ffo20kpGRgdwc8VOPmAxtvBe8e9ih9NcS1SH5KyQH9z1sCU9WnIz2BEwI1OiW4/6
Nr+jW5DyYl0wiT8CAwEAAaAAMA0GCSqGSIb3DQEBCwUAA4IBAQBZj3eFle1V3UQO
wlh7oyez/lNpf1W/bO3FIVb8hJhFmnGTIOdypT2t5wSJpChOKqSn2D6Jyd91ccd6
9FvXISA5s0Jkq8pmgLbt3cjQ2civFhCI0meX7FGGO3ArwWsbbiGJWZqZuxfZWWr6
oKuOtYq7EWie2dcz4rSpiVc3twfU4+vuInOIUGM5jTCsVRalHqcms1WwOBrP6+Vu
i2kMzA51FSfmB71x69MT41bYDwn40D3EDRdF9d9ly8v+TGAY84BIoyW+qwOkFd7r
8o+IwQsoVA4QJzsr9yoPB3m5aF+rDJf4mU1/UwNTOjCKgx/SqT1TI/j7tPsguQ0K
YapUplH4
-----END CERTIFICATE REQUEST-----

# openssl req -noout -text -in server.csr
</pre>


3) **使用 CA 证书及CA密钥 对请求签发证书进行签发，生成 x509证书**
<pre>
//首先我们产生一个自签名的CA证书及CA秘钥
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=ivan1001/emailAddress=1181891136@qq.com"
Generating a 2048 bit RSA private key
......................................+++
...+++
writing new private key to 'rsa_private.key'
-----
# ls
cert.crt  rsa_private.key  server.csr  server.key


//然后采用上述自签名的CA证书及CA秘钥，对请求签发证书进行签发
# openssl x509 -req -days 3650 -in server.csr -CA cert.crt -CAkey rsa_private.key -passin pass:111111 -CAcreateserial -out server.crt
Signature ok
subject=/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com
Getting CA Private Key
# ls
cert.crt  cert.srl  rsa_private.key  server.crt  server.csr  server.key


//用CA根证书对我们签发的x509证书进行校验
# openssl verify -CAfile cert.crt server.crt 
server.crt: OK
</pre>
其中```CAxxx```选项用于指定```CA```参数输入。上面我们对server.csr进行了签发，生成了```server.crt``` x509证书。(注意这里```-CAkey```是证书签发机构的私钥）

### 3.5 证书查看及转换

1） **查看证书细节**
<pre>
# openssl x509 -in cert.crt -noout -text           //查看我们上面产生的自签名证书细节
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number:
            d0:81:5a:0e:1f:64:77:92
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CN, ST=Guangdong, L=Shenzhen, O=test_company, OU=IT, CN=ivan1001/emailAddress=1181891136@qq.com
        Validity
            Not Before: Mar 23 02:17:39 2018 GMT
            Not After : Mar 23 02:17:39 2019 GMT
        Subject: C=CN, ST=Guangdong, L=Shenzhen, O=test_company, OU=IT, CN=ivan1001/emailAddress=1181891136@qq.com
        Subject Public Key Info:

# openssl x509 -in server.crt -noout -text         //查看我们上面产生的CA签名证书细节
Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number:
            d6:81:fa:f4:ad:fd:48:19
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CN, ST=Guangdong, L=Shenzhen, O=test_company, OU=IT, CN=ivan1001/emailAddress=1181891136@qq.com
        Validity
            Not Before: Mar 23 02:19:12 2018 GMT
            Not After : Mar 20 02:19:12 2028 GMT
        Subject: C=CN, ST=Guangdong, L=Shenzhen, O=test_company, OU=IT, CN=test_name/emailAddress=11111111@qq.com
        Subject Public Key Info:
</pre>

2） **转换证书编码格式**
<pre>
# openssl x509 -in cert.crt -inform PEM -outform DER -out cert.der
# ls
cert.crt  cert.der  cert.srl  rsa_private.key  server.crt  server.csr  server.key
</pre>
默认产生的证书格式是```PEM```格式。

3) **合成 pkcs#12 证书(含私钥)**

* 将 pem 证书和私钥转 pkcs#12 证书
<pre>
//将server的pem证书和私钥进行转换
# openssl pkcs12 -export -in server.crt -inkey server.key -passin pass:111111 -password pass:111111 -out server.p12
# ls
cert.crt  cert.der  cert.srl  rsa_private.key  server.crt  server.csr  server.key  server.p12

//将自签名的pem证书和私钥进行转换(因为这里自签名的pem证书并没有密码，因此这里不需要-passin选项）
# openssl pkcs12 -export -in cert.crt -inkey rsa_private.key  -password pass:111111 -out cert.p12
# ls
cert.crt  cert.der  cert.p12  cert.srl  rsa_private.key  server.crt  server.csr  server.key  server.p12
</pre>
其中```-export```指导出pkcs#12 证书，```-inkey``` 指定了私钥文件，```-passin``` 为私钥(文件)密码(nodes为无加密)，```-password``` 指定 p12文件的密码(导入导出)

* 将pem 证书和私钥/CA 证书 合成pkcs#12 证书
<pre>
# openssl pkcs12 -export -in server.crt -inkey server.key -passin pass:111111 -chain -CAfile cert.crt -password pass:111111 -out server-all.p12
# ls
cert.crt  cert.der  cert.p12  cert.srl  rsa_private.key  server-all.p12  server.crt  server.csr  server.key  server.p12
</pre>

其中```-chain```指示同时添加证书链，```-CAfile``` 指定了CA证书，导出的p12文件将包含多个证书。(其他选项：```-name```可用于指定server证书别名；```-caname```用于指定ca证书别名)

* pcks#12 提取PEM文件(含私钥)
<pre>
# mkdir out
# openssl pkcs12 -in server.p12 -password pass:111111 -passout pass:111111 -out out/server.pem
MAC verified OK
# ls out/
server.pem

# cat out/server.pem
</pre>
其中```-password```指定 p12文件的密码(导入导出)，```-passout```指输出私钥的加密密码(nodes为无加密)导出的文件为pem格式，同时包含证书和私钥(pkcs#8)：

4) **从pkcs#12证书中提取私钥**
<pre>
# openssl pkcs12 -in server.p12 -password pass:111111 -passout pass:111111 -nocerts -out out/key.pem
MAC verified OK
</pre>


5) **仅提取证书（所有证书）**
<pre>
# openssl pkcs12 -in server.p12 -password pass:111111 -nokeys -out out/server_all.crt
MAC verified OK
</pre>

6) **仅提取CA证书**
<pre>
# openssl pkcs12 -in server-all.p12 -password pass:111111 -nokeys -cacerts -out out/cacert.pem
</pre>

7) **仅提取server证书**
<pre>
# openssl pkcs12 -in server-all.p12 -password pass:111111 -nokeys -clcerts -out out/cert.pem 
</pre>

## 4. openssl 命令参考
{% highlight string %}
1. openssl list-standard-commands(标准命令)
    1) asn1parse: asn1parse用于解释用ANS.1语法书写的语句(ASN一般用于定义语法的构成) 
    2) ca: ca用于CA的管理 
    openssl ca [options]:
        2.1) -selfsign
        使用对证书请求进行签名的密钥对来签发证书。即"自签名"，这种情况发生在生成证书的客户端、签发证书的CA都是同一台机器(也是我们大多数实验中的情况)，
        我们可以使用同一个密钥对来进行"自签名"
        2.2) -in file
        需要进行处理的PEM格式的证书
        2.3) -out file
        处理结束后输出的证书文件
        2.4) -cert file
        用于签发的根CA证书
        2.5) -days arg 
        指定签发的证书的有效时间
        2.6) -keyfile arg   
        CA的私钥证书文件
        2.7) -keyform arg
        CA的根私钥证书文件格式:
            2.7.1) PEM
            2.7.2) ENGINE 
        2.8) -key arg   
        CA的根私钥证书文件的解密密码(如果加密了的话)
        2.9) -config file    
        配置文件
    example1: 利用CA证书签署请求证书
    openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key  

    3) req: X.509证书签发请求(CSR)管理
    openssl req [options] <infile >outfile
        3.1) -inform arg
        输入文件格式
            3.1.1) DER
            3.1.2) PEM
        3.2) -outform arg   
        输出文件格式
            3.2.1) DER
            3.2.2) PEM
        3.3) -in arg
        待处理文件
        3.4) -out arg
        待输出文件
        3.5) -passin        
        用于签名待生成的请求证书的私钥文件的解密密码
        3.6) -key file
        用于签名待生成的请求证书的私钥文件
        3.7) -keyform arg  
            3.7.1) DER
            3.7.2) NET
            3.7.3) PEM
        3.8) -new
        新的请求
        3.9) -x509          
        输出一个X509格式的证书 
        3.10) -days
        X509证书的有效时间  
        3.11) -newkey rsa:bits 
        生成一个bits长度的RSA私钥文件，用于签发  
        3.12) -[digest]
        HASH算法
            3.12.1) md5
            3.12.2) sha1
            3.12.3) md2
            3.12.4) mdc2
            3.12.5) md4
        3.13) -config file   
        指定openssl配置文件
        3.14) -text: text显示格式
    example1: 利用CA的RSA密钥创建一个自签署的CA证书(X.509结构) 
    openssl req -new -x509 -days 3650 -key server.key -out ca.crt 
    example2: 用server.key生成证书签署请求CSR(这个CSR用于之外发送待CA中心等待签发)
    openssl req -new -key server.key -out server.csr
    example3: 查看CSR的细节
    openssl req -noout -text -in server.csr

    4) genrsa: 生成RSA参数
    openssl genrsa [args] [numbits]
        [args]
        4.1) 对生成的私钥文件是否要使用加密算法进行对称加密:
            4.1.1) -des: CBC模式的DES加密
            4.1.2) -des3: CBC模式的DES加密
            4.1.3) -aes128: CBC模式的AES128加密
            4.1.4) -aes192: CBC模式的AES192加密
            4.1.5) -aes256: CBC模式的AES256加密
        4.2) -passout arg: arg为对称加密(des、des、aes)的密码(使用这个参数就省去了console交互提示输入密码的环节)
        4.3) -out file: 输出证书私钥文件
        [numbits]: 密钥长度
    example: 生成一个1024位的RSA私钥，并用DES加密(密码为1111)，保存为server.key文件
    openssl genrsa -out server.key -passout pass:1111 -des3 1024 

    5) rsa: RSA数据管理
    openssl rsa [options] <infile >outfile
        5.1) -inform arg
        输入密钥文件格式:
            5.1.1) DER(ASN1)
            5.1.2) NET
            5.1.3) PEM(base64编码格式)
         5.2) -outform arg
         输出密钥文件格式
            5.2.1) DER
            5.2.2) NET
            5.2.3) PEM
        5.3) -in arg
        待处理密钥文件 
        5.4) -passin arg
        输入这个加密密钥文件的解密密钥(如果在生成这个密钥文件的时候，选择了加密算法了的话)
        5.5) -out arg
        待输出密钥文件
        5.6) -passout arg  
        如果希望输出的密钥文件继续使用加密算法的话则指定密码 
        5.7) -des: CBC模式的DES加密
        5.8) -des3: CBC模式的DES加密
        5.9) -aes128: CBC模式的AES128加密
        5.10) -aes192: CBC模式的AES192加密
        5.11) -aes256: CBC模式的AES256加密
        5.12) -text: 以text形式打印密钥key数据 
        5.13) -noout: 不打印密钥key数据 
        5.14) -pubin: 检查待处理文件是否为公钥文件
        5.15) -pubout: 输出公钥文件
    example1: 对私钥文件进行解密
    openssl rsa -in server.key -passin pass:111 -out server_nopass.key
    example:2: 利用私钥文件生成对应的公钥文件
    openssl rsa -in server.key -passin pass:111 -pubout -out server_public.key

    6) x509:
    本指令是一个功能很丰富的证书处理工具。可以用来显示证书的内容，转换其格式，给CSR签名等X.509证书的管理工作
    openssl x509 [args]    
        6.1) -inform arg
        待处理X509证书文件格式
            6.1.1) DER
            6.1.2) NET
            6.1.3) PEM
        6.2) -outform arg   
        待输出X509证书文件格式
            6.2.1) DER
            6.2.2) NET
            6.2.3) PEM
        6.3) -in arg 
        待处理X509证书文件
        6.4) -out arg       
        待输出X509证书文件
        6.5) -req            
        表明输入文件是一个"请求签发证书文件(CSR)"，等待进行签发 
        6.6) -days arg       
        表明将要签发的证书的有效时间 
        6.7) -CA arg 
        指定用于签发请求证书的根CA证书 
        6.8) -CAform arg     
        根CA证书格式(默认是PEM) 
        6.9) -CAkey arg      
        指定用于签发请求证书的CA私钥证书文件，如果这个option没有参数输入，那么缺省认为私有密钥在CA证书文件里有
        6.10) -CAkeyform arg  
        指定根CA私钥证书文件格式(默认为PEM格式)
        6.11) -CAserial arg   
        指定序列号文件(serial number file)
        6.12) -CAcreateserial 
        如果序列号文件(serial number file)没有指定，则自动创建它     
    example1: 转换DER证书为PEM格式
    openssl x509 -in cert.cer -inform DER -outform PEM -out cert.pem
    example2: 使用根CA证书对"请求签发证书"进行签发，生成x509格式证书
    openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt
    example3: 打印出证书的内容
    openssl x509 -in server.crt -noout -text 

    7) crl: crl是用于管理CRL列表 
    openssl crl [args]
        7.1) -inform arg
        输入文件的格式
            7.1.1) DER(DER编码的CRL对象)
            7.1.2) PEM(默认的格式)(base64编码的CRL对象)
        7.2) -outform arg
        指定文件的输出格式 
            7.2.1) DER(DER编码的CRL对象)
            7.2.2) PEM(默认的格式)(base64编码的CRL对象)
        7.3) -text: 
        以文本格式来打印CRL信息值。
        7.4) -in filename
        指定的输入文件名。默认为标准输入。
        7.5) -out filename
        指定的输出文件名。默认为标准输出。
        7.6) -hash
        输出颁发者信息值的哈希值。这一项可用于在文件中根据颁发者信息值的哈希值来查询CRL对象。
        7.7) -fingerprint
        打印CRL对象的标识。
        7.8) -issuer
        输出颁发者的信息值。
        7.9) -lastupdate
        输出上一次更新的时间。
        7.10) -nextupdate
        打印出下一次更新的时间。 
        7.11) -CAfile file
        指定CA文件，用来验证该CRL对象是否合法。 
        7.12) -verify
        是否验证证书。        
    example1: 输出CRL文件，包括(颁发者信息HASH值、上一次更新的时间、下一次更新的时间)
    openssl crl -in crl.crl -text -issuer -hash -lastupdate –nextupdate 
    example2: 将PEM格式的CRL文件转换为DER格式
    openssl crl -in crl.pem -outform DER -out crl.der  

    8) crl2pkcs7: 用于CRL和PKCS#7之间的转换 
    openssl crl2pkcs7 [options] <infile >outfile
    转换pem到spc
    openssl crl2pkcs7 -nocrl -certfile venus.pem -outform DER -out venus.spc
    https://www.openssl.org/docs/apps/crl2pkcs7.html

    9) pkcs12: PKCS#12数据的管理
    pkcs12文件工具，能生成和分析pkcs12文件。PKCS#12文件可以被用于多个项目，例如包含Netscape、 MSIE 和 MS Outlook
    openssl pkcs12 [options] 
    http://blog.csdn.net/as3luyuan123/article/details/16105475
    https://www.openssl.org/docs/apps/pkcs12.html

    10) pkcs7: PCKS#7数据的管理 
    用于处理DER或者PEM格式的pkcs#7文件
    openssl pkcs7 [options] <infile >outfile
    http://blog.csdn.net/as3luyuan123/article/details/16105407
    https://www.openssl.org/docs/apps/pkcs7.html
 
 
2. openssl list-message-digest-commands(消息摘要命令)
    1) dgst: dgst用于计算消息摘要 
    openssl dgst [args]
        1.1) -hex           
        以16进制形式输出摘要
        1.2) -binary        
        以二进制形式输出摘要
        1.3) -sign file    
        以私钥文件对生成的摘要进行签名
        1.4) -verify file    
        使用公钥文件对私钥签名过的摘要文件进行验证 
        1.5) -prverify file  
        以私钥文件对公钥签名过的摘要文件进行验证
        verify a signature using private key in file
        1.6) 加密处理
            1.6.1) -md5: MD5 
            1.6.2) -md4: MD4         
            1.6.3) -sha1: SHA1 
            1.6.4) -ripemd160
    example1: 用SHA1算法计算文件file.txt的哈希值，输出到stdout
    openssl dgst -sha1 file.txt
	
    example2: 用dss1算法验证file.txt的数字签名dsasign.bin，验证的private key为DSA算法产生的文件dsakey.pem
    openssl dgst -dss1 -prverify dsakey.pem -signature dsasign.bin file.txt
	
    example3: 用sha256算法计算文件uploadfile的哈希值，输出到stdout
    openssl dgst -sha256 ./uploadfile
	
    example4: hmac-sha256算法
    openssl dgst -sha256 -hmac keystr ./uploadfile

    2) sha1: 用于进行RSA处理
    openssl sha1 [args] 
        2.1) -sign file
        用于RSA算法的私钥文件 
        2.2) -out file
        输出文件爱你
        2.3) -hex   
        以16进制形式输出
        2.4) -binary
        以二进制形式输出  
    example1: 用SHA1算法计算文件file.txt的HASH值,输出到文件digest.txt
    openssl sha1 -out digest.txt file.txt
    example2: 用sha1算法为文件file.txt签名,输出到文件rsasign.bin，签名的private key为RSA算法产生的文件rsaprivate.pem
    openssl sha1 -sign rsaprivate.pem -out rsasign.bin file.txt


3. openssl list-cipher-commands (Cipher命令的列表)
    1) aes-128-cbc
    2) aes-128-ecb
    3) aes-192-cbc
    4) aes-192-ecb
    5) aes-256-cbc
    6) aes-256-ecb
    7) base64
    8) bf
    9) bf-cbc
    10) bf-cfb
    11) bf-ecb
    12) bf-ofb
    13) cast
    14) cast-cbc
    15) cast5-cbc
    16) cast5-cfb
    17) cast5-ecb
    18) cast5-ofb
    19) des
    20) des-cbc
    21) des-cfb
    22) des-ecb
    23) des-ede
    24) des-ede-cbc
    25) des-ede-cfb
    26) des-ede-ofb
    27) des-ede3
    28) des-ede3-cbc
    29) des-ede3-cfb
    30) des-ede3-ofb
    31) des-ofb
    32) des3
    33) desx
    34) rc2
    35) rc2-40-cbc
    36) rc2-64-cbc
    37) rc2-cbc
    38) rc2-cfb
    39) rc2-ecb
    40) rc2-ofb
    41) rc4
    42) rc4-40
	  
# openssl version -v   (查看openssl版本)
OpenSSL 1.0.2k-fips  26 Jan 2017
{% endhighlight %}

<br />
<br />

**[参看]:**

1. [openssl 证书请求和自签名命令req详解](https://www.cnblogs.com/gordon0918/p/5409286.html)

2. [Centos7下的Openssl和CA](http://blog.51cto.com/lidongfeng/2068516)

3. [使用 openssl 生成证书](https://www.cnblogs.com/littleatp/p/5878763.html)

4. [Nginx+Https配置](https://segmentfault.com/a/1190000004976222)

5. [Linux下OpenSSL 源码安装的9个步骤](https://blog.csdn.net/chengqiuming/article/details/70139714)

6. [openssl - 数字证书的编程解析](https://www.cnblogs.com/huhu0013/p/4791430.html)

7. [SSL多域名绑定证书的解决方案](https://blog.csdn.net/kexiuyi/article/details/51837703)

8. [linux下openssl命令详解](https://blog.csdn.net/baidu_36649389/article/details/54379935)

<br />
<br />
<br />


