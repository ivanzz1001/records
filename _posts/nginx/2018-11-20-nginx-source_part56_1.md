---
layout: post
title: core/ngx_regex.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


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


## 3. 


<br />
<br />

**[参看]**

1. [Nginx模块开发中使用PCRE正则表达式匹配](https://blog.x-speed.cc/archives/38.html)

2. [nx单独使用pcre的一个小坑](http://dinic.iteye.com/blog/2057150)

3. [深入解析Nginx的pcre库及相关注意事项](https://blog.csdn.net/deltatang/article/details/8754002)

4. [Nginx模块开发中使用PCRE正则表达式匹配](https://blog.x-speed.cc/archives/38.html)

5. [pcre官网](http://www.pcre.org/)

6. [pcre使用例子](http://blog.chinaunix.net/uid-26575352-id-3517146.html)

7. [PCRE的安装及使用](https://www.cnblogs.com/LiuYanYGZ/p/5903954.html)

8. [正则表达式引擎pcre使用JIT...](https://segmentfault.com/q/1010000000366720)

9. [c语言正则表达式库pcre使用例子](https://blog.csdn.net/earbao/article/details/52152625)

10. [PCRE-正则库及用法](https://www.cnblogs.com/LiuYanYGZ/p/5903946.html)

<br />
<br />
<br />

