---
layout: post
title: Linux中openssl的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux中openssl的使用
---


本文主要记录一下Linux操作系统中openssl的使用。

<!-- more -->




## 1. 示例

### 1.1 RSA秘钥操作
默认情况下，openssl输出格式为： ```PKCS#1-PEM```

1) **生成RSA私钥(无加密)**
{% highlight string %}
# openssl genrsa -out rsa_private.key 2048
Generating RSA private key, 2048 bit long modulus
..............................+++
..........................................+++
e is 65537 (0x10001)

//生成的私钥文件
# ls
rsa_private.key
{% endhighlight %}

2) **生成RSA公钥**
{% highlight string %}
# openssl rsa -in rsa_private.key -pubout -out rsa_public.key
writing RSA key

# ls
rsa_private.key  rsa_public.key
{% endhighlight %}

3) **生成RSA私钥（使用aes256加密)**

因为有时候我们不想别人看到我们的明文私钥，这时候我们可以对产生的RSA私钥进行加密。这样即使我们的私钥加密文件泄露，私钥还是安全的：
{% highlight string %}
# openssl genrsa -aes256 -passout pass:111111 -out rsa_aes_private.key 2048
Generating RSA private key, 2048 bit long modulus
............+++
.........................+++
e is 65537 (0x10001)

# ls
rsa_aes_private.key  
{% endhighlight %}
其中 passout 代替shell 进行密码输入，否则会提示输入密码。此时如果基于此，要生成**公钥**，需要提供密码：
{% highlight string %}
# openssl rsa -in rsa_aes_private.key -passin pass:111111 -pubout -out rsa_public.key
{% endhighlight %}

### 1.2 openssl转换命令

1) **对加密的私钥进行解密**

首先我们使用```aec256方法```，产生一个加密的私钥，然后再对其进行解密：
{% highlight string %}
# ls

# openssl genrsa -aes256 -passout pass:111111 -out rsa_aes_private.key 2048         //产生加密私钥
# ls
rsa_aes_private.key

# openssl rsa -in rsa_aes_private.key -passin pass:111111 -out rsa_private.key     //解密后的私钥
writing RSA key
# ls
rsa_aes_private.key  rsa_private.key
{% endhighlight %}

2) **对明文私钥进行加密**

这里我们对上文解密后的私钥```rsa_private.key```再进行手动加密，看得出的结果是否与原来的```rsa_aes_private.key```一致：
{% highlight string %}
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
{% endhighlight %}

3) **私钥PEM转DER**

默认产生的公、私钥都是```PEM```格式的，现在转换成```DER```格式：
{% highlight string %}
#ls

# openssl genrsa -out rsa_private.key 2048                              //产生一个明文私钥
# openssl rsa -in rsa_private.key -outform der -out rsa_private.der     //转换成DER格式
writing RSA key

# ls
rsa_private.der  rsa_private.key
{% endhighlight %}


4) **查看私钥明细**
{% highlight string %}
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
{% endhighlight %}

这里将```-in```参数替换成```-pubin```参数，就可以查看公钥明细.

5) **私钥PKCS#1转PKCS#8**
{% highlight string %}
# openssl genrsa -out rsa_private.key 2048         //产生一个明文私钥(传统私钥格式PKCS#1,即rsa)
# openssl pkcs8 -topk8 -in rsa_private.key -passout pass:111111 -out pkcs8_private.key  (rsa转PKCS#8)
{% endhighlight %}
其中-passout指定了密码，输出的pkcs8格式密钥为加密形式，pkcs8默认采用des3 加密算法。使用```-nocrypt```参数可以输出无加密的pkcs8密钥，如下：
{% highlight string %}
# openssl pkcs8 -topk8 -in rsa_private.key -nocrypt -out nocrypt_pkcs8_private.key
{% endhighlight %}


### 1.3 生成自签名证书

1) **生成RSA私钥和自签名证书**
{% highlight string %}
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
{% endhighlight %}
上面我们输入了：

* Country Name

* State/Province Name

* Locality Name(Default city)

* Organizational Name(Default Company Ltd)

* Organizational Unit name(section)

* Common Name(your name or your server's hostname)    

* Email Address

如果想要省去这些输入，可以用```-subj```选项：
{% highlight string %}
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
{% endhighlight %}

我们再对上面的命令做一个简单的说明： ```req```是证书请求的子命令，```-newkey rsa:2048 -keyout private_key.pem``` 表示生成私钥(PKCS8格式)，```-nodes``` 表示私钥不加密，若不带参数将提示输入密码；
```-x509```表示输出证书，```-days365``` 为有效期。

<br />

2) **使用 已有RSA 私钥生成自签名证书**
{% highlight string %}
# openssl genrsa -out rsa_private.key 2048     //产生一个明文私钥
# openssl req -new -x509 -days 365 -key rsa_private.key -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
# ls
cert.crt  rsa_private.key
{% endhighlight %}

```-new```指生成证书请求，加上-x509 表示直接输出证书，```-key```指定私钥文件，其余选项与上述命令相同


### 1.4 生成签名求情及CA签名

1) **使用RSA私钥生成CSR签名请求**
{% highlight string %}
# openssl genrsa -aes256 -passout pass:111111 -out server.key 2048      //生成aes256加密的rsa私钥
# ls
server.key

# openssl req -new -key server.key -out server.csr                  //申城CSR签名请求
Enter pass phrase for server.key:
unable to load Private Key
139621627549600:error:06065064:digital envelope routines:EVP_DecryptFinal_ex:bad decrypt:evp_enc.c:592:
139621627549600:error:0906A065:PEM routines:PEM_do_header:bad decrypt:pem_lib.c:488:
[root@localhost test-src]# openssl req -new -key server.key  -out server.csr
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
[root@localhost test-src]# ls
server.csr  server.key
{% endhighlight %}
如上分别输入了解密```server.key```的密码```111111```; 然后是```subject```主题信息；再接着会要求输入一个```challenge```密码（此密码可以为空）。也可以采用如下命令，简化输入：
{% highlight string %}
# openssl req -new -key server.key -passin pass:111111 -out server.csr -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"
{% endhighlight %}

***```此时生成的 csr签名请求文件可提交至 CA进行签发```***

<br />

2) **查看CSR 的细节**
{% highlight string %}
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
{% endhighlight %}


3) **使用 CA 证书及CA密钥 对请求签发证书进行签发，生成 x509证书**
{% highlight string %}
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
{% endhighlight %}
其中```CAxxx```选项用于指定```CA```参数输入。上面我们对server.csr进行了签发，生成了```server.crt``` x509证书。(注意这里```-CAkey```是证书签发机构的私钥）





<br />
<br />

**[参看]:**

1. [openssl 证书请求和自签名命令req详解](https://www.cnblogs.com/gordon0918/p/5409286.html)

2. [Centos7下的Openssl和CA](http://blog.51cto.com/lidongfeng/2068516)

3. [使用 openssl 生成证书](https://www.cnblogs.com/littleatp/p/5878763.html)

4. [Nginx+Https配置](https://segmentfault.com/a/1190000004976222)

<br />
<br />
<br />


