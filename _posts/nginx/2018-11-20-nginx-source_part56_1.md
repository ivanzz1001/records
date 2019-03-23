---
layout: post
title: pcre库的使用
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

在分析nginx中```ngx_regex_module```之前，我们先讲述一下pcre库的使用。

本章我们主要讲述一下nginx中表达式匹配(ngx_regex)的实现。我们先讲解pcre库的基本使用，接着在讲解nginx中对pcre的封装。


<!-- more -->


## 1. PCRE库的介绍

PCRE是 **'Perl Compatible Regular Expressions'**(Perl兼容的正则表达式）的缩写。PCRE库由一系列函数组成，实现了与Perl5相同的语法、语义的正则表达式匹配功能。PCRE有其自己的native API，同时也包含了一系列的wrapper函数以兼容POSIX正则表达式API。

PCRE最初是为```Exim MTA```项目而开发的，但是到目前位置很多其他的高级开源软件也采用了PCRE，比如Apache、PHP、KDE、Nmap等。同时PCRE在一些商业化的软件中也有使用，比如Apple Safari。

### 1.1 PCRE版本
目前PCRE库有两个主版本(major version)。当前的版本,```PCRE2```的初次发布是在2015年，现在已经到了```10.32```版了。

更老一点的PCRE版本初次发布时间是1997年，当前仍被广泛的使用，现在已经到了```8.42```版了。其相应的API及特性当前均已稳定（后续发布也只会进行bug修正）。对于一些新的特性，将会被加入到```PCRE2```中，而并不会加入到```8.x```系列中。

## 2. PCRE库的安装
这里我们安装PCRE ```8.x```系列：
<pre>
# wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.40.tar.gz
# tar -zxvf pcre-8.40.tar.gz
# cd pcre-8.40
# ./configure
# make
# make install
</pre>
执行```configure```后我们看到如下的基本配置信息：
{% highlight string %}
pcre-8.40 configuration summary:

    Install prefix .................. : /usr/local
    C preprocessor .................. : gcc -E
    C compiler ...................... : gcc
    C++ preprocessor ................ : g++ -E
    C++ compiler .................... : g++
    Linker .......................... : /usr/bin/ld -m elf_x86_64
    C preprocessor flags ............ : 
    C compiler flags ................ : -g -O2 -fvisibility=hidden
    C++ compiler flags .............. : -O2 -fvisibility=hidden -fvisibility-inlines-hidden
    Linker flags .................... : 
    Extra libraries ................. : 

    Build 8 bit pcre library ........ : yes
    Build 16 bit pcre library ....... : no
    Build 32 bit pcre library ....... : no
    Build C++ library ............... : yes
    Enable JIT compiling support .... : no
    Enable UTF-8/16/32 support ...... : no
    Unicode properties .............. : no
    Newline char/sequence ........... : lf
    \R matches only ANYCRLF ......... : no
    EBCDIC coding ................... : no
    EBCDIC code for NL .............. : n/a
    Rebuild char tables ............. : no
    Use stack recursion ............. : yes
    POSIX mem threshold ............. : 10
    Internal link size .............. : 2
    Nested parentheses limit ........ : 250
    Match limit ..................... : 10000000
    Match limit recursion ........... : MATCH_LIMIT
    Build shared libs ............... : yes
    Build static libs ............... : yes
    Use JIT in pcregrep ............. : no
    Buffer size for pcregrep ........ : 20480
    Link pcregrep with libz ......... : no
    Link pcregrep with libbz2 ....... : no
    Link pcretest with libedit ...... : no
    Link pcretest with libreadline .. : no
    Valgrind support ................ : no
    Code coverage ................... : no
{% endhighlight %}

安装完成后，我们看到：
<pre>
# ls /usr/local/lib/
libpcre.a              libpcrecpp.so          libpcre.la             libpcreposix.so        libpcre.so             pkgconfig/             
libpcrecpp.a           libpcrecpp.so.0        libpcreposix.a         libpcreposix.so.0      libpcre.so.1           valgrind/              
libpcrecpp.la          libpcrecpp.so.0.0.1    libpcreposix.la        libpcreposix.so.0.0.4  libpcre.so.1.2.8      

# ls /usr/local/include/
pcrecpparg.h  pcrecpp.h  pcre.h  pcreposix.h  pcre_scanner.h  pcre_stringpiece.h

# ls -al /usr/local/share/man/man3/pcre32_*
</pre>
运行```RunTest```测试情况如下：
{% highlight string %}
# ./RunTest 

PCRE C library tests using test data from ./testdata
PCRE version 8.40 2017-01-11

---- Testing 8-bit library ----

Test 1: Main functionality (Compatible with Perl >= 5.10)
  OK
  OK with study
Test 2: API, errors, internals, and non-Perl stuff (not UTF-8)
  OK
  OK with study
Test 3: Locale-specific features (using 'fr_FR' locale)
  OK
  OK with study
Test 4: UTF-8 support (Compatible with Perl >= 5.10)
  Skipped because UTF-8 support is not available
Test 5: API, internals, and non-Perl stuff for UTF-8 support
  Skipped because UTF-8 support is not available
Test 6: Unicode property support (Compatible with Perl >= 5.10)
  Skipped because Unicode property support is not available
Test 7: API, internals, and non-Perl stuff for Unicode property support
  Skipped because Unicode property support is not available
Test 8: DFA matching main functionality
  OK
  OK with study
Test 9: DFA matching with UTF-8
  Skipped because UTF-8 support is not available
Test 10: DFA matching with Unicode properties
  Skipped because Unicode property support is not available
Test 11: Internal offsets and code size tests
  Skipped because Unicode property support is not available
Test 12: JIT-specific features (when JIT is available)
  Skipped because JIT is not available or not usable
Test 13: JIT-specific features (when JIT is not available)
  OK
Test 14: Specials for the basic 8-bit library
  OK
  OK with study
Test 15: Specials for the 8-bit library with UTF-8 support
  Skipped because UTF-8 support is not available
Test 16: Specials for the 8-bit library with Unicode propery support
  Skipped because Unicode property support is not available
Test 17: Specials for the basic 16/32-bit library
  Skipped when running 8-bit tests
Test 18: Specials for the 16/32-bit library with UTF-16/32 support
  Skipped when running 8-bit tests
Test 19: Specials for the 16/32-bit library with Unicode property support
  Skipped when running 8-bit tests
Test 20: DFA specials for the basic 16/32-bit library
  Skipped when running 8-bit tests
Test 21: Reloads for the basic 16/32-bit library
  Skipped when running 8-bit tests
Test 22: Reloads for the 16/32-bit library with UTF-16/32 support
  Skipped when running 8-bit tests
Test 23: Specials for the 16-bit library
  Skipped when running 8/32-bit tests
Test 24: Specials for the 16-bit library with UTF-16 support
  Skipped when running 8/32-bit tests
Test 25: Specials for the 32-bit library
  Skipped when running 8/16-bit tests
Test 26: Specials for the 32-bit library with UTF-32 support
  Skipped when running 8/16-bit tests
{% endhighlight %}


## 3. PCRE正则表达式的定义
用于描述字符排列和匹配模式的一种语法规则。它主要用于字符串的模式分割、匹配、查找及替换操作。正则表达式中重要的几个概念有： 元字符、转义、模式单元（重复）、反义、引用和断言。

1) **常用的元字符(Meta-character)**

* ```\A```: 匹配字符串串首的原子；

* ```\Z```: 匹配字符串串尾的原子； 例如```pattern=job\\Z```，输入input="hello,world, I am looking a job"

* ```\b```: 匹配单词的边界。例如， ```\bis```用于匹配头为is的字符串， ```is\b```用于匹配尾是is的字符串, ```\bis\b```用于定界.

* ```\B```: 匹配除单词边界之外的任意字符。例如```\\Bis```匹配单词```This```中的```is```

* ```\d```: 匹配一个数字，等价于[0-9];

* ```\D```: 匹配除数字以外的任何一个字符，等价于 [^0-9]

* ```\w```: 匹配一个英文字母、数字或下划线，等价于 [0-9a-zA-Z_]

* ```\W```: 匹配除英文字母、数字和下划线以外任何一个字符，等价于 [^0-9a-zA-Z_]

* ```\s```: 匹配一个空白字符， 等价于[\f\t\v]

* ```\S```: 匹配除空白字符以外任何一个字符，等价于[^\f\t\v]

* ```\f```: 匹配一个换页符，等价于```\x0c```或者```\cL```

* ```\n```: 匹配一个换行符，等价于```\x0a```或者```\cJ```

* ```\r```: 匹配一个回车符，等价于```\x0d```或者```\cM```

* ```\t```: 匹配一个制表符，等价于```\x09```或者```\cl```

* ```\v```: 匹配一个垂直制表符，等价于```\x0b```或者```\ck```

* ```\oNN```: 匹配一个八进制数字

* ```\xNN```: 匹配一个十六进制数字

* ```\cx```: 匹配由```x```指明的控制字符。例如```\cM``` 匹配一个 Control-M 或回车符。x 的值必须为 A-Z 或 a-z 之一。否则，将 c 视为一个原义的 'c' 字符。

2) **模式修正符(Pattern Modifiers)**

模式修正符在忽略大小写、匹配多行中使用特别多，掌握了这一个修正符，往往能解决我们遇到的很多问题。

* ```(?i)```: 可同时匹配大小写字母

* ```(?m)```: 将字符串视为多行

* ```(?s)```: 将字符串视为单行，换行符做普通字符看待，使```.```匹配任何字符

* ```(?X)```: 忽略无效的转义

* ```(?U)```: 匹配到最近的字符串

* ```(?e)```: 将替换的字符串作为表达式使用


例如下面可以匹配以```Hel```开头的单词（忽略大小写):
{% highlight string %}
char src[] = "hello,world, I am looking a job";
char pattern[] = "(?i)\\AHel";
{% endhighlight %}

## 4. PCRE的函数简介

### 4.1 PCRE函数的基本介绍

PCRE是一个```NFA```正则引擎，不然不能提供完全与Perl一致的正则语法功能。但PCRE同时也实现了```DFA```， 只是满足数学意义上的正则。PCRE提供了19个接口函数，这里只介绍几个主要和常用的接口函数，另外的可以通过PCRE源码文档进行了解。注意，使用PCRE主要是使用下面介绍的前4个函数，对这4个函数有了了解，使用PCRE库的时候就会简单很多。

下面所介绍的函数，均在PCRE头文件中进行了声明。

1) **函数pcre_compile()**

函数原型为：
<pre>
pcre *pcre_compile(const char *pattern, int options, const char **errptr, int *erroffset, const unsigned char *tableptr)
</pre>

功能： 将一个正则表达式编译成一个内部的```pcre```结构，在匹配多个字符串时，可以加速匹配。其中```pcre_compile2()```功能一样，只是缺少一个参数```errorcodeptr```。

参数说明：

* pattern: 正则表达式

* options: 为0， 或者其他参数选项

* errptr: 返回出错信息

* erroffset: 返回出错位置

* tableptr: 指向一个字符数组的指针，可以设置为NULL


2) **函数pcre_compile2()**

函数原型：
<pre>
pcre *pcre_compile2(const char *pattern, int options, int *errorcodeptr, const char **errptr, int *erroffset, const unsigned char *tableptr)
</pre>

功能： 将一个正则表达式编译成一个内部的```pcre```结构，在匹配多个字符串时，可以加速匹配。其同```pcre_compile()```功能一样，只是多一个参数```errorcodeptr```。

参数说明：

* pattern: 正则表达式

* options: 为0， 或者其他参数选项

* errorcodeptr: 存放出错码

* errptr: 出错消息

* erroffset: 出错位置

* tableptr: 指向一个字符数组的指针，可以设置为NULL


**3) 函数pcre_exec()**

函数原型：
<pre>
int pcre_exec(const pcre *code, const pcre_extra *extra, const char *subject, int length, int startoffset, int options, int *ovector, int ovecsize)
</pre>
功能： 使用编译好的模式进行匹配，采用与```Perl```相似的算法。返回值大于0，表示匹配到的个数； 否则表示出错信息

参数说明：

* code: 编译好的模式

* extra: 指向一个pcre_extra结构体，可以为NULL

* subject: 需要匹配的字符串

* length: 匹配的字符串长度(Byte)

* startoffset: 匹配的开始位置

* options: 选项位

* ovector: 指向一个结果的整形数组

* ovecsize: 数组大小(数组大小一般应为3的整数倍)

**4) 函数pcre_study()**

函数原型：
<pre>
pcre_extra *pcre_study(const pcre *code, int options, const char **errptr)
</pre>
功能： 对编译的模式进行学习，提取可以加速匹配过程的信息

参数说明：

* code: 已编译的模式

* options: 选项

* errptr: 出错消息


**5) 函数pcre_version()**

函数原型：
<pre>
char *pcre_version(void)
</pre>

功能： 返回PCRE的版本信息

参数说明： 无


**6) 函数pcre_config()**

函数原型：
<pre>
int pcre_config(int what, void *where)
</pre>

功能： 查询当前PCRE版本中使用的选项信息

参数说明： 

* what: 选项名

* where: 存储结果的位置


**7) 函数pcre_maketables()**

函数原型：
<pre>
const unsigned char *pcre_maketables(void)
</pre>
功能： 生成一个字符表，表中每一个元素的值不大于256，可以用它传给pcre_compile()替换掉内建的字符表

参数： 无


### 4.2 使用PCRE在C语言中实现正则表达式的解析
上面讲述了这么多```PCRE```相关函数的介绍，目的还是为了能够运用上，所以这里就先讲解下使用PCRE的过程。主要过程分三步： 首先编译正则表达式； 接着匹配正则表达式； 最后释放正则表达式。

**1) 编译正则表达式**

为了提高效率，在将一个字符串与正则表达式进行比较之前，首先要用pcre_compile()/pcre_compile2()函数对它进行编译，转化成```PCRE```引擎能够识别的结构(struct real_pcre)。

这里还可以调用pcre_study()函数，对编译后的正则表达式结构(struct real_pcre)进行分析和学习，学习的结果是一个数据结构(struct pcre_extra)，这个数据结构连同编译后的规则(struct real_pcre)可以一起送给pcre_exec()单元进行匹配。


**2) 匹配正则表达式**

一旦用函数pcre_compile()/pcre_compile2()成功地编译了正则表达式，接下来就可以调用pcre_exec()函数完成模式匹配，根据正则表达式到指定的字符串中进行查找和匹配，并输出匹配结果。


**3） 释放正则表达式**

无论什么时候，当不再需要已编译过的正则表达式时，都应该调用函数free()将其释放，以免产生内存泄露。


## 5. PCRE使用示例
如下我们通过几个pcre的基本使用示例来简单介绍一些pcre库的使用。

### 5.1 匹配手机号码
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pcre.h>
#include <string.h>



#define OVECCOUNT 30 /* should be a multiple of 3 */
#define EBUFLEN 128
#define BUFLEN 1024



int main(int argc, char *argv[])
{
	/*
	 * China mobile(中国移动) 
	 * China unicom(中国联通)
	 * China Telecom(中国电信)
	 * CDMA
	 */
	pcre *reCM, *reUN, *reTC, *reCDMA;
	
	const char *error_cm, *error_un, *error_tc, *error_cdma;
	int erroffset_cm, erroffset_un, erroffset_tc, erroffset_cdma;
	int ovecter[OVECCOUNT];

	int rcCM, rcUN, rcTC, rcCDMA, i;
	

	/*
	 * telephones
	 *
	 * CM: 134, 135, 136, 137, 138, 139, 150, 151, 152, 157, 158, 159, 187, 188, 147
	 * UN: 130, 131, 132, 155, 156, 185, 186
	 * TC: 180, 189
	 * CDMA: 133, 153
	 */
	char src[22]; 
	char pattern_CM[] = "^1(3[4-9]|5[012789]|8[78])\\d{8}$";  
	char pattern_UN[] = "^1(3[0-2]|5[56]|8[56])\\d{8}$";  
	char pattern_TC[] = "^18[09]\\d{8}$";  
	char pattern_CDMA[] = "^1[35]3\\d{8}$";

	printf("please input your telephone number: "); 
	scanf("%s", src);    
	printf("String : %s\n", src);   
	printf("Pattern_CM: \"%s\"\n", pattern_CM);  
	printf("Pattern_UN: \"%s\"\n", pattern_UN);
	printf("Pattern_TC: \"%s\"\n", pattern_TC);  
	printf("Pattern_CDMA: \"%s\"\n", pattern_CDMA);
	printf("\n");

	reCM = pcre_compile(pattern_CM, 0, &error_cm, &erroffset_cm, NULL);
	reUN = pcre_compile(pattern_UN, 0, &error_un, &erroffset_un, NULL); 
	reTC = pcre_compile(pattern_TC, 0, &error_tc, &erroffset_tc, NULL);  
	reCDMA = pcre_compile(pattern_CDMA, 0, &error_cdma, &erroffset_cdma, NULL);

	if(!reCM)
	{
		printf("PCRE compilation (China mobile) telephone pattern failed at offset %d: %s\n", erroffset_cm, error_cm);
	}else{
		rcCM = pcre_exec(reCM, NULL, src, strlen(src), 0, 0, ovecter, OVECCOUNT);
		if (rcCM < 0)
		{
			if(rcCM == PCRE_ERROR_NOMATCH){
				printf("sorry, China mobile telephone no match...\n");
			}else{
				printf("China mobile telephone match error: %d\n", rcCM);
			}
		}else{
			printf("OK, has matched China mobile telephone\n");
		}
		free(reCM);
	}

	if(!reUN)
	{
		printf("PCRE compilation (China unicom) telephone pattern failed at offset %d: %s\n", erroffset_un, error_un);
	}else{
		rcUN = pcre_exec(reUN, NULL, src, strlen(src), 0, 0, ovecter, OVECCOUNT);
		if (rcUN < 0)
		{
			if(rcUN == PCRE_ERROR_NOMATCH){
				printf("sorry, China unicom telephone no match...\n");
			}else{
				printf("China unicom telephone match error: %d\n", rcUN);
			}
		}else{
			printf("OK, has matched China unicom telephone\n");
		}
		free(reUN);
	}

	if(!reTC)
	{
		printf("PCRE compilation (China Telecom) telephone pattern failed at offset %d: %s\n", erroffset_tc, error_tc);
	}else{
		rcTC = pcre_exec(reTC, NULL, src, strlen(src), 0, 0, ovecter, OVECCOUNT);
		if (rcTC < 0)
		{
			if(rcTC == PCRE_ERROR_NOMATCH){
				printf("sorry, China Telecom telephone no match...\n");
			}else{
				printf("China Telecom telephone match error: %d\n", rcTC);
			}
		}else{
			printf("OK, has matched China Telecom telephone\n");
		}
		free(reTC);
	}

	if(!reCDMA)
	{
		printf("PCRE compilation (CDMA) telephone pattern failed at offset %d: %s\n", erroffset_cdma, error_cdma);
	}else{
		rcCDMA = pcre_exec(reCDMA, NULL, src, strlen(src), 0, 0, ovecter, OVECCOUNT);
		if (rcCDMA < 0)
		{
			if(rcCDMA == PCRE_ERROR_NOMATCH){
				printf("sorry, CDMA telephone no match...\n");
			}else{
				printf("CDMA telephone match error: %d\n", rcCDMA);
			}
		}else{
			printf("OK, has matched CDMA telephone\n");
		}
		free(reCDMA);
	}

	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -c -o test.o test.c
# gcc -o test test.o -lpcre
# ./test
please input your telephone number: 13838382438
String : 13838382438
Pattern_CM: "^1(3[4-9]|5[012789]|8[78])\d{8}$"
Pattern_UN: "^1(3[0-2]|5[56]|8[56])\d{8}$"
Pattern_TC: "^18[09]\d{8}$"
Pattern_CDMA: "^1[35]3\d{8}$"

OK, has matched China mobile telephone
sorry, China unicom telephone no match...
sorry, China Telecom telephone no match...
sorry, CDMA telephone no match...
</pre>

从上面我们可以看到PCRE库的使用相对简单，首先执行```pcre_compile()```函数将模式编译为```pcre```数据结构，然后再采用该数据结构来完成后续的匹配。

### 5.2 pcre_exec()函数用法
{% highlight string %}
#include <stdio.h>
#include <string.h>
#include <pcre.h>


#define OVECCOUNT 30 /* should be a multiple of 3 */
#define EBUFLEN 128
#define BUFLEN 1024
 
int main(int argc, char **argv)
{
    pcre *re;
    const char *error;
    int  erroffset;
    int  ovector[OVECCOUNT];
    int  rc, i;
 
    char src[] = "123.123.123.123:80|1.1.1.1:88";
    char pattern[] = "(\\d*.\\d*.\\d*.\\d*):(\\d*)";
 
    printf("String : %s\n", src);
    printf("Pattern: \"%s\"\n", pattern);
 
 
    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }
 
    char *p = src;
    while ( ( rc = pcre_exec(re, NULL, p, strlen(p), 0, 0, ovector, OVECCOUNT)) != PCRE_ERROR_NOMATCH )
    {
        printf("\nOK, has matched ...\n\n");
 
        for (i = 0; i < rc; i++)
        {
            char *substring_start = p + ovector[2*i];
            int substring_length = ovector[2*i+1] - ovector[2*i];
            char matched[1024];
            memset( matched, 0, 1024 );
            strncpy( matched, substring_start, substring_length );
 
            printf( "match:%s\n", matched );
        }
 
        p += ovector[1];
        if ( !p )
        {
            break;
        }
    }
    pcre_free(re);
 
    return 0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -c -o test.o test.c
# gcc -o test test.o -lpcre
# ./test
String : 123.123.123.123:80|1.1.1.1:88
Pattern: "(\d*.\d*.\d*.\d*):(\d*)"

OK, has matched ...

match:123.123.123.123:80
match:123.123.123.123
match:80

OK, has matched ...

match:1.1.1.1:88
match:1.1.1.1
match:88
</pre>
从这里我们可以看出，```pcre_exec()```函数返回的是总匹配的条目数（请参看匹配模式中```(```的用法）。



<br />
<br />

**[参看]**

1. [pcre官网](http://www.pcre.org/)

2. [pcre使用例子](http://blog.chinaunix.net/uid-26575352-id-3517146.html)

3. [PCRE的安装及使用](https://www.cnblogs.com/LiuYanYGZ/p/5903954.html)

4. [正则表达式引擎pcre使用JIT...](https://segmentfault.com/q/1010000000366720)

5. [c语言正则表达式库pcre使用例子](https://blog.csdn.net/earbao/article/details/52152625)

6. [PCRE-正则库及用法](https://www.cnblogs.com/LiuYanYGZ/p/5903946.html)

7. [正则表达式边界符](https://blog.csdn.net/justheretobe/article/details/53152267)

8. [正则表达式 - 语法](http://www.runoob.com/regexp/regexp-syntax.html)

9. [正则表达式30分钟入门教程](https://blog.csdn.net/wushuai1346/article/details/7180920)

10. [Specifying Modes Inside The Regular Expression](https://www.regular-expressions.info/modifiers.html)
<br />
<br />
<br />

