---
layout: post
title: Linux C语言实现urlencode和urldecode
tags:
- cplusplus
categories: cplusplus
description: Linux C语言实现urlencode和urldecode
---

本文主要记录一下urlencode和urldecode的C语言实现，作为一个简易工具使用。


<!-- more -->


## 1. urlencode编码的基本规则

URL编码做了如下操作：



* 字符```"a"-"z"，"A"-"Z"，"0"-"9"，"."，"-"，"*"，和"_"``` 都不被编码，维持原值；

* 空格```" "```被转换为加号```"+"```。

* 其他每个字节都被表示成```"%XY"```格式的由3个字符组成的字符串，编码为UTF-8(特别需要注意： 这里是大写形式的hexchar)。


## 2. urlencode编码
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>





static unsigned char hexchars[] = "0123456789ABCDEF";



/**
 * @brief URLEncode : encode the base64 string "str"
 *
 * @param str:  the base64 encoded string
 * @param strsz:  the str length (exclude the last \0)
 * @param result:  the result buffer
 * @param resultsz: the result buffer size(exclude the last \0)
 *
 * @return: >=0 represent the encoded result length
 *              <0 encode failure
 *
 * Note:
 * 1) to ensure the result buffer has enough space to contain the encoded string, we'd better
 *     to set resultsz to 3*strsz
 *
 * 2) we don't check whether str has really been base64 encoded
 */
int URLEncode(const char *str, const int strsz, char *result, const int resultsz)
{
    int i,j;
	char ch;

    if(strsz < 0 || resultsz < 0)
		return -1;

	for(i = 0,j = 0;i<strsz && j < resultsz;i++)
	{
		ch = *(str + i);
		if((ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= '0' && ch <= '9') ||
			ch == '.' || ch == '-' || ch == '*' || ch == '_')
			result[j++] = ch;
		else if(ch == ' ')
			result[j++] = '+';
		else{
			if(j + 3 <= resultsz)
			{
			    result[j++] = '%';
				result[j++] = hexchars[(unsigned char)ch >> 4];
				result[j++] = hexchars[(unsigned char)ch & 0xF];
			}
			else{
				return -2;
			}
		}
	}

    if(i == 0)
		return 0;
	else if(i == strsz)
		return j;
	return -2;
}



// return < 0: represent failure
int main(int argc,char *argv[])
{
    int fd = -1;
	char buf[1024],result[1024*3];
	int ret;
	int i = 0;

	if(argc != 2)
	{
	   printf("please input the encoding filename\n");
	   return -1;
	}

	if((fd = open(argv[1],O_RDONLY)) == -1)
	{
	    printf("open file %s failure\n",argv[1]);
		return -2;
	}

    while((ret = read(fd,buf,1024)) >= 0)
    {
        if(ret == 0)
			break;

		ret = URLEncode(buf,ret,result,1024*3);
		if(ret < 0)
		    break;

		for(i = 0;i<ret;i++)
			printf("%c",result[i]);

    }

	if(ret < 0)
	{
	   printf("encode data failure\n");
	}

	close(fd);
	return ret;
}
{% endhighlight %}

## 3. urldecode解码
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>





static unsigned char hexchars[] = "0123456789ABCDEF";



/**
 * @brief URLDecode : decode the urlencoded str to base64 encoded string
 *
 * @param str:  the urlencoded string
 * @param strsz:  the str length (exclude the last \0)
 * @param result:  the result buffer
 * @param resultsz: the result buffer size(exclude the last \0)
 *
 * @return: >=0 represent the decoded result length
 *              <0 encode failure
 *
 * Note:
 * 1) to ensure the result buffer has enough space to contain the decoded string, we'd better
 *     to set resultsz to strsz
 *
 */
int URLDecode(const char *str, const int strsz, char *result, const int resultsz, const char **last_pos)
{
    int i,j;
	char ch;
	char a;

    *last_pos = str;
    if(strsz < 0 || resultsz < 0)
		return -1;

    for(i = 0,j = 0;i<strsz && j<resultsz;j++)
    {
        ch = *(str + i);

		if(ch == '+')
		{
			result[j] = ' ';
			i += 1;
		}
		else if(ch == '%')
		{
		    if(i+3 <= strsz)
		    {
		        ch = *(str + i + 1);

				if(ch >= 'A' && ch <= 'F')
				{
				    a = (ch - 'A')+10;
				}
				else if(ch >= '0' && ch <= '9')
				{
				    a = ch - '0';
				}
				else if(ch >= 'a' && ch <= 'f')
				{
				   a = (ch - 'a') + 10;
				}
				else{
					return -2;
				}

			    a <<= 4;

				ch = *(str + i + 2);
				if(ch >= 'A' && ch <= 'F')
				{
				    a |= (ch - 'A') + 10;
				}
				else if(ch >= '0' && ch <= '9')
				{
				    a |= (ch - '0');
				}
				else if(ch >= 'a' && ch <= 'f')
				{
				     a |= (ch - 'a') + 10;
				}
				else{
					return -2;
				}

			    result[j] = a;

                i += 3;
		    }
			else
				break;
		}
		else if((ch >= 'A' && ch <= 'Z') ||
			(ch >= 'a' && ch <= 'z') ||
			(ch >= '0' && ch <= '9') ||
			ch == '.' || ch == '-' || ch == '*' || ch == '_'){

			result[j] = ch;
			i+=1;
		}
		else{
			return -2;
		}

    }

    *last_pos =  str + i;
    return j;

}



// return < 0: represent failure
int main(int argc,char *argv[])
{
    int fd = -1;
	char buf[4096],result[4096];
	char *start_pos;
	const char * last_pos;
	int ret,sz;
	int i = 0;

	if(argc != 2)
	{
	   printf("please input the encoding filename\n");
	   return -1;
	}

	if((fd = open(argv[1],O_RDONLY)) == -1)
	{
	    printf("open file %s failure\n",argv[1]);
		return -2;
	}

    start_pos = buf;
	last_pos = NULL;

	while((ret = read(fd,start_pos,buf + 4096 - start_pos)) >= 0)
	{
	    if(ret == 0)
	    {
	        if(start_pos == buf)
				break;
			else
			{
			    ret = -3;
				break;
			}
	    }
		sz = URLDecode(buf,start_pos - buf + ret,result,4096,&last_pos);
		if(sz < 0)
		{
			ret = -4;
			break;
		}

		if(last_pos != start_pos + ret)
		{
		    memcpy(buf,last_pos,start_pos + ret - last_pos);
			start_pos = buf + (start_pos + ret - last_pos);
		}
		else{
			start_pos = buf;
		}

		for(i = 0;i<sz;i++)
			printf("%c",result[i]);

	}

    if(ret < 0)
    {
        printf("decode data failure\n");
    }
	close(fd);
	return ret;
}
{% endhighlight %}

<br />
<br />

**[参看]:**

1. [URL encode 与 URL decode 的C语言实现](http://blog.csdn.net/langeldep/article/details/6264058)

2. [c语言实现urlencode和decode](http://blog.csdn.net/lifan5/article/details/8671697)




<br />
<br />
<br />





