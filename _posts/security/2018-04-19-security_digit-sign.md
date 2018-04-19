---
layout: post
title: 数字签名技术
tags:
- security
categories: security
description: 数字签名技术
---


本文我们主要介绍一下数字签名技术的实现原理，然后在最后给出一个实现示例。


<!-- more -->





## 1. 示例

### 1.1 产生证书
{% highlight string %}
//产生根证书
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout ca.key \
    -x509 -days 365 -out ca.crt \
    -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test/emailAddress=11111111@qq.com"

# ls
ca.crt  ca.key


//产生证书签名请求
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout harbor-registry.key \
    -out harbor-registry.csr \
    -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=192.168.69.128/emailAddress=11111111@qq.com"

# ls
ca.crt  ca.key  harbor-registry.csr  harbor-registry.key


//产生证书
# echo subjectAltName = IP:192.168.69.128 > extfile.cnf

# openssl x509 -req -days 365 -in harbor-registry.csr -CA ca.crt -CAkey ca.key -CAcreateserial -extfile extfile.cnf -out harbor-registry.crt

# ls
ca.crt  ca.key  ca.srl  extfile.cnf  harbor-registry.crt  harbor-registry.csr  harbor-registry.key
{% endhighlight %}


### 1.2 测试程序
编写digital_sign.c文件如下：
{% highlight string %}
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>


#define DIGITAL_CRT    			"harbor-registry.crt"
#define DIGITAL_PRIVATE_KEY     "harbor-registry.key"

#define BUFFSIZE 8192


RSA* getPrivateKey(char* in_szKeyPath)
{
	FILE	*fp = NULL;
	char	szKeyPath[1024];
	RSA		*priRsa = NULL, *pOut = NULL;

	memset(szKeyPath, 0 ,sizeof(szKeyPath));

	if(256 < strlen(in_szKeyPath))
		strncpy(szKeyPath, in_szKeyPath, 256);
	else
		strncpy(szKeyPath, in_szKeyPath, strlen(in_szKeyPath));

	printf("private key file [%s]\n", szKeyPath);

    /*open private key file*/
	if(NULL == (fp = fopen(szKeyPath, "rb")))
	{
		printf( "open private key file [%s] failure\n", szKeyPath);
		return NULL;
	}

	/*get private key*/
	if(NULL == (priRsa = PEM_read_RSAPrivateKey(fp, &priRsa, NULL,NULL)))
	{
		printf( "read private key failure\n");
		fclose(fp);
		return NULL;
	}
	fclose(fp);

	printf("read private key success\n");
	pOut = priRsa;
	return pOut;
}

#if 0
RSA* getPublicKey(char* in_szKeyPath)
{
	FILE	*fp = NULL;
	char	szKeyPath[1024];
	RSA		*pubRsa = NULL, *pOut = NULL;


	memset(szKeyPath, 0 ,sizeof(szKeyPath));
	if(256 < strlen(in_szKeyPath))
		strncpy(szKeyPath, in_szKeyPath, 256);
	else
		strncpy(szKeyPath, in_szKeyPath, strlen(in_szKeyPath));

	printf("public key file [%s]\n", szKeyPath);

	/*open crt file(public key file)*/
	if(NULL == (fp = fopen(szKeyPath, "rb")))
	{
		printf( "open public key [%s] failure\n", szKeyPath);
		return NULL;
	}

	/*get public key*/
	if(NULL == (pubRsa = PEM_read_RSA_PUBKEY(fp,NULL, NULL,NULL)))
	{
		printf("read public key failure\n");
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	printf("read public key success\n");
	pOut = pubRsa;
	return pOut;
}
#endif

RSA* getPublicKey(char* in_szKeyPath)
{
	char	szKeyPath[1024];
	BIO *bio;
	X509 *x509;
	EVP_PKEY *pubk;
	RSA *rsa;

	memset(szKeyPath, 0 ,sizeof(szKeyPath));

	if(256 < strlen(in_szKeyPath))
		strncpy(szKeyPath, in_szKeyPath, 256);
	else
		strncpy(szKeyPath, in_szKeyPath, strlen(in_szKeyPath));

	printf("public key file [%s]\n", szKeyPath);


    if(NULL == (bio = BIO_new_file(DIGITAL_CRT,"rb")))
    {
    	return NULL;
    }

    if(NULL == (x509 = PEM_read_bio_X509(bio,NULL,NULL,NULL)))
    {
        BIO_free(bio);
		return NULL;
    }

    if(NULL == (pubk = X509_get_pubkey(x509)))
    {
        BIO_free(bio);
		X509_free(x509);
		return NULL;
    }

    rsa = EVP_PKEY_get1_RSA(pubk);

	BIO_free(bio);
	X509_free(x509);

	return rsa;
}




int main(void)
{
	char	szEncData[]="orderId=01010500201502000004reqTime=20150205012727ext=20151120ext2=1";
	int strlen, iRet;
	
	RSA		*pRsaPriv = NULL, *pRSAPub = NULL;
	char sign_digest_buf[BUFFSIZE], sign_buf[BUFFSIZE];
	char verify_digest_buf[BUFFSIZE], verify_buf[BUFFSIZE];
	int siglen;


	if(NULL == (pRsaPriv = getPrivateKey(DIGITAL_PRIVATE_KEY)))
	{
    	RSA_free(pRsaPriv);
		printf("get private key failure\n");
		return -1;
	}

	//RSA_print_fp(stdout, prsa, 11);

	strlen = sizeof(szEncData) -1;
	printf("szEnData:[%s]\n", szEncData);


	//1) Get the message digest
	memset(sign_digest_buf, 0, BUFFSIZE);
	SHA1(szEncData, strlen, sign_digest_buf);

	//2) use private-key to sign the message digest
	memset(sign_buf, 0, BUFFSIZE);
	iRet = RSA_sign(NID_sha1, (unsigned char *)sign_digest_buf, 20, (unsigned char*)sign_buf, &siglen, pRsaPriv);
	if(iRet != 1 )
	{
		printf("sign failure\n");
		RSA_free(pRsaPriv);
		return -1;
	}
	RSA_free(pRsaPriv);
	printf("sign success\n\n");




	//签名串szTmp1二进制数据需要转成base64编码传输至对方








	// 3) validate the digital sign

	if(NULL == (pRSAPub = getPublicKey(DIGITAL_CRT)))
	{
		RSA_free(pRSAPub);
		printf("get public key failure\n");
		return -1;
	}

	//签名数据 和 SHA1摘要 因该是由通信报文中获得，这里演示直接用使用同一变量
	strlen = sizeof(szEncData) -1;
	printf("szEnData:[%s]\n", szEncData);


	//4) Get the message digest
	memset(verify_digest_buf, 0, BUFFSIZE);
	SHA1(szEncData, strlen, verify_digest_buf);

	//5) verify
	iRet = RSA_verify(NID_sha1, (unsigned char *)verify_digest_buf, 20, (unsigned char*)sign_buf, siglen, pRSAPub);
	if(iRet != 1 )
	{
    	printf("verify failure\n");
    	RSA_free(pRSAPub);
        return -1;
	}
	else
    	printf("verify success\n");

	RSA_free(pRSAPub);
	return 0;
}
{% endhighlight %}

编译运行：
<pre>
# ./digital_sign 
private key file [harbor-registry.key]
read private key success
szEnData:[orderId=01010500201502000004reqTime=20150205012727ext=20151120ext2=1]
sign success

public key file [harbor-registry.crt]
szEnData:[orderId=01010500201502000004reqTime=20150205012727ext=20151120ext2=1]
verify success
</pre>


<br />
<br />

**[参考]**

1. [数字签名](https://baike.baidu.com/item/%E6%95%B0%E5%AD%97%E7%AD%BE%E5%90%8D/212550?fr=aladdin)

2. [linux c 使用openssl实现SHA1WithRSA实现，签名，验签](https://download.csdn.net/download/vr7jj/9846871)

3. [OpenSSL编程-RSA编程详解](http://www.qmailer.net/archives/216.html)

4. [openssl 学习之从证书中提取RSA公钥N 和 E](https://www.cnblogs.com/huhu0013/p/4794613.html)

5. [openssl之RSA编程（3）（签名和认证）](http://yuanshuilee.blog.163.com/blog/static/21769727520140139251684/)
<br />
<br />
<br />


