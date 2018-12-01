---
layout: post
title: core/ngx_regex.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本章我们主要讲述一下nginx中表达式匹配(ngx_regex)的实现。


<!-- more -->


## 1. ngx_regex_t数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_REGEX_H_INCLUDED_
#define _NGX_REGEX_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

#include <pcre.h>


#define NGX_REGEX_NO_MATCHED  PCRE_ERROR_NOMATCH   /* -1 */

#define NGX_REGEX_CASELESS    PCRE_CASELESS


typedef struct {
    pcre        *code;
    pcre_extra  *extra;
} ngx_regex_t;
{% endhighlight %}
这里```ngx_regex_t```只是对```pcre```的一个再次封装。其中：

* code: 代表一个编译后的正则表达式

* extra: 用作pcre的辅助数据，一般是通过```pcre_study()```学习到的，可以加快后面的后续的匹配过程


## 2. 数据结构
{% highlight string %}
typedef struct {
    ngx_str_t     pattern;
    ngx_pool_t   *pool;
    ngx_int_t     options;

    ngx_regex_t  *regex;
    int           captures;
    int           named_captures;
    int           name_size;
    u_char       *names;
    ngx_str_t     err;
} ngx_regex_compile_t;
{% endhighlight %}

```ngx_regex_compile_t```表示一个编译的正则表达式模式(pattern)。其各字段含义如下：

* pattern: 对应的正则表达式模式

* pool: 所关联的内存池

* options: 执行pcre_compile()时所传递的参数

* regex: 编译完成后的pcre实例

* captures: 用于保存```pattern```中所有子模式的个数，包括命名子模式(named subpattern)和非命名子模式。

* named_captures: 用于保存```pattern```中命名子模式的个数

* name_size: ```name table```中每一项的大小

* names: ```name table```的入口地址

* err: 用于存放相应的错误


## 3. ngx_regex_elt_t数据结构
{% highlight string %}
typedef struct {
    ngx_regex_t  *regex;
    u_char       *name;
} ngx_regex_elt_t;
{% endhighlight %}
存放到```ngx_pcre_studies```链表中的元素， 其中各字段含义如下：

* regex: pattern编译完成后的pcre实例

* name: 保存对应的Pattern

## 4. 相关函数声明
{% highlight string %}

//初始化nginx pcre
void ngx_regex_init(void);

//pcre pattern，封装了pcre_compile()
ngx_int_t ngx_regex_compile(ngx_regex_compile_t *rc);

//pcre_exec()的包装
#define ngx_regex_exec(re, s, captures, size)                                \
    pcre_exec(re->code, re->extra, (const char *) (s)->data, (s)->len, 0, 0, \
              captures, size)
#define ngx_regex_exec_n      "pcre_exec()"


//将字符串s匹配模式数组中的所有模式
ngx_int_t ngx_regex_exec_array(ngx_array_t *a, ngx_str_t *s, ngx_log_t *log);
{% endhighlight %}


## 5. 附录： PCRE库中pcre_fullinfo混合子模式调用结果
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <pcre.h>
 
#define NGX_REGEX_CASELESS    PCRE_CASELESS
 
typedef struct {
    pcre        *code;
    pcre_extra  *extra;
} ngx_regex_t;
 
 
int main(int argc, char *argv[])
{
	pcre  *re;
	const char *errstr;
	int  erroff;

	char  *data = "(eeeee)(?<abb> exception)(?<adfa>xydz)(ddddd)\\k<abb>\\1\\2";

	int captures = 0;
	int named_captures = 0;
	int name_entrysize = 0;
	char *name_entry;
	char *p;

	int i,j,n;


	printf("pattern: %s\n\n", data);

	re = pcre_compile(data, PCRE_CASELESS, &errstr, &erroff, NULL);
	if(NULL == re)
	{
		printf("compile pcre failed\n");

		return 0;
	}


	//1) get all subpatterns count
	n = pcre_fullinfo(re, NULL, PCRE_INFO_CAPTURECOUNT, &captures);
	if(n < 0)
	{
		printf("pcre_fullinfo PCRE_INFO_CAPTURECOUNT failed %d \n", n);

		return 0;

	}         
	printf("captures %d \n", captures);


	//2) get named subpatterns count
	n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMECOUNT, &named_captures);
	if(n < 0)
	{
		printf("pcre_fullinfo PCRE_INFO_NAMECOUNT failed %d \n", n);
		return 0;
	}
	printf("named_captures %d \n", named_captures);

	//3) get name table entry
	n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMETABLE, &name_entry);
	if(n < 0)
	{
		printf("pcre_fullinfo PCRE_INFO_NAMETABLE failed %d \n", n);
		return 0;
	}
	p = name_entry;

	
	//4) get every entry size
	n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMEENTRYSIZE, &name_entrysize);
	if(n < 0)
	{
		printf("pcre_fullinfo PCRE_INFO_NAMEENTRYSIZE failed %d \n", n);
		return 0;
	}
	printf("name_entrysize %d \n", name_entrysize);


	//5) print all named entry
	for(i = 0; i < named_captures; i++)
	{
		for(j = 0; j < 2; j++)
		{
			printf("%x ", p[i]);
		}
		printf("entry: %s\n", &p[2]);

		p += name_entrysize;
	}
	return 1;
}
{% endhighlight %}
编译运行：
{% highlight string %}
# gcc -c -o test.o test.c
# gcc -o test test.o -lpcre
# ./test
pattern: (eeeee)(?<abb> exception)(?<adfa>xydz)(ddddd)\k<abb>\1\2

captures 4 
named_captures 2 
name_entrysize 7 
0 0 entry: abb
3 3 entry: adfa
{% endhighlight %}



<br />
<br />

**[参看]**

1. [Nginx模块开发中使用PCRE正则表达式匹配](https://blog.x-speed.cc/archives/38.html)

2. [nx单独使用pcre的一个小坑](http://dinic.iteye.com/blog/2057150)

3. [深入解析Nginx的pcre库及相关注意事项](https://blog.csdn.net/deltatang/article/details/8754002)

4. [Nginx模块开发中使用PCRE正则表达式匹配](https://blog.x-speed.cc/archives/38.html)

5. [pcre官网](http://www.pcre.org/)

6. [PCRE接口pcre_fullinfo混合子模式调用结果](https://blog.csdn.net/zwleagle/article/details/8563364)

7. [正则表达式30分钟入门教程](https://blog.csdn.net/wushuai1346/article/details/7180920#backreference)

<br />
<br />
<br />

