---
layout: post
title: 大整数乘法
tags:
- data-structure
categories: data-structure
description: 大整数乘法
---

如下给出一个大整数乘法的实现。


<!-- more -->

## 1. 大整数乘法

**题目描述**

求两个不超过200位的非负整数的积。

**输入**

第一行为一个整数t，表示有t组数据。接下来有2t行，每两行表示一组数据。 对于一组数据，每行是一个不超过200位的非负整数，没有多余的前导0。

**输出**

对于每组数据，输出一行，即相乘后的结果。结果里不能有多余的前导0，即如果结果是342，那么就不能输出为0342。

**样例输入**

1234567890098765432100

**样例输出**

1219326311126352690000 


{% highlight string %}
#include <iostream>
#include <cstring>
using namespace std;

int main(int argc, char *argv[])
{
	char a[210],b[210]; //开个较大数组
	int t,i,j,k,c[500],pa,pb,jinwei,flag=0;
	cin >> t;

    while (t--)
    {
		jinwei=0;
		flag=0;
		memset(a,0,sizeof(a));//数组全部数清0
		memset(b,0,sizeof(b));//数组全部数清0
		
		memset(c,0,sizeof(c));//数组全部数清0
		
		cin >> a >> b;
		pa=strlen(a);//读取数组长度
		pb=strlen(b);
		for (i=0;i<pa/2;i++)//数组前后的数倒置
		{
			k=a[i];
			a[i]=a[pa-i-1];
			a[pa-i-1]=k;
		}

		for (i=0;i<pb/2;i++)//数组前后的数倒置
		{
			k=b[i];
			b[i]=b[pb-i-1];
			b[pb-i-1]=k;
		}

		for (i=0;i<pa;i++)//双循环，每层循环代表一个乘数
		{
			for (j=0;j<pb;j++)
			{
				c[i+j]+=(a[i]-'0')*(b[j]-'0');//i+j是核心，i+j表示c数组接收个位，十位等数。如：123*13，以123不变乘1，3；乘1时自动进位。
				jinwei=c[i+j]/10;//进位
				c[i+j]=c[i+j]%10;//只要个位取余
				c[i+j+1]+=jinwei;//进位给前一个数
			}
		}

		if (jinwei>0)//最后jinwei>0,说明前面还有一位数
		{
			c[pa+pb-1]=jinwei;
			k=pa+pb-1;//不管什么数相乘，都不大于两个乘数的各位数相加再加1的；如：123*34，<5或6的
		}
		else{
			k=pa+pb-2;
		}

        j=0;//判断是否乘0的情况
        for (i=k;i>=0;i--)
        {
            if (c[i]==0&&flag==0)//首位不为0
            {
				continue;
			}
            else{
				flag=1;
			}
			cout << c[i];
			j++;
		}

		if (j==0)
		{
			cout << 0 ;
		}
		cout << endl;
	}
}
{% endhighlight %}





<br />
<br />
**[参看]:**



<br />
<br />
<br />


