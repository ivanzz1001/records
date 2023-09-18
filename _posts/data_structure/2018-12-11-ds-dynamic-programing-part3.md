---
layout: post
title: 动态规划:最长公共子串(转)
tags:
- data-structure
categories: data-structure
description: 动态规划:最长公共子串
---



本文介绍一下笔试面试算法经典--最长公共子串（Longest Common SubString)问题。


<!-- more -->

## 1. 最长公共子串

最长公共子串(Longest Common Substring): 是指两个字符串中最长连续相同的子串长度。

例如: str1="1AB2345CD",str2="12345EF",则str1，str2的最长公共子串为2345。


### 1.1 解法1
如果str1的长度为N，str2的长度为M，生成大小为```M*N```的数组dp, dp[i][j]表示```str2[0…i]```与```str1[0…j]```的最长公共子串的长度。

计算dp[i][j] 的方法如下：

1）矩阵dp的第一列dp[0…m-1][0]: 对于 某个位置(i，0), 如果str1[0]==str2[i],则dp[i][0]=1,否则dp[i][0]=0

2）矩阵dp的第一行dp[0][0…n-1]: 对于 某个位置(0，j), 如果str1[j]==str2[0],则dp[0][j]=1,否则dp[0][j]=0

3） 其他位置从左到右从上到下计算，dp[i][j]的值只有两种情况：
* str1[i]==str2[j], 则dp[i][j]=dp[i-1][j-1]+1;

* str1[i]!=str2[j], 则dp[i][j]=0;

str1="abc",str2="caba"的 dp 矩阵如下:
<pre>
     a     b     c
------------------------
c    0     0     1

a    1     0     0

b    0     2     0

a    1     0     0
</pre>

代码如下：
{% highlight string %}
public static void Lcss(char str1[],char str2[])
{       
    int dp[][]=new int[str1.length][str2.length];
    //对dp矩阵的第一列赋值
    for(int i=0;i<str1.length;i++)
    {
        if(str2[0]==str1[i])
            dp[i][0]=1;
        else {
            dp[i][0]=0;
        }
    }

    //对dp矩阵的第一行赋值
    for(int j=0;j<str2.length;j++)
    {
        if(str1[0]==str2[j])
            dp[0][j]=1;
        else {
            dp[0][j]=0;
        }
    }

    for(int i=1;i<str1.length;i++)
    {
        for(int j=1;j<str2.length;j++)
        {
            if(str1[i]==str2[j])
            {
                dp[i][j]=dp[i-1][j-1]+1;
            }
            else {
                dp[i][j]=0;
            }
        }
    }

    int max=dp[0][0];
    for(int i=0;i<str1.length;i++)
    {
        for(int j=0;j<str2.length;j++)
        {
            max=Math.max(max,dp[i][j]);
        }
    }
    
    System.out.println(max);
}
{% endhighlight %}

### 1.2 解法2
经典动态规划的方法需要大小为M*N的dp矩阵，但实际上是可以减少至O(1)的，因为计算每一个dp[i][j]的时候只需要计算dp[i-1][j-1],所以按照斜线方向计算所有的值，只需要一个变量就可以计算:
{% highlight string %}
public static void Lcss1(char str1[],char str2[])
{
    int len=0,max=0;
    int row=0;
    int col=str2.length-1;

    //计算矩阵中的每一条斜对角线上值。
    while(row<str1.length)
    {
        int i=row;
        int j=col;

        while(i<str1.length&&j<str2.length)
        {
            if(str1[i]==str2[j])
            {
                len++;
                max=Math.max(max, len);
            }
            else {
                len=0;
            }

            i++;
            j++;
        }

        if(col>0)
        {
            col--;
        }
        else {
            row++;
        }
    }

    System.out.println(max);
}
{% endhighlight %}



<br />
<br />
**[参看]:**

1. [最长公共子串](https://blog.csdn.net/u013309870/article/details/69479488)



<br />
<br />
<br />


