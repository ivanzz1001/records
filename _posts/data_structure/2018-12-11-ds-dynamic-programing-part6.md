---
layout: post
title: 动态规划之最短编辑距离
tags:
- data-structure
categories: data-structure
description: 动态规划之最短编辑距离
---


本文记录一下Leetcode[动态规划: 最短编辑距离](https://leetcode.cn/problems/edit-distance/description/?envType=study-plan-v2&envId=top-interview-150)问题的解法。


<!-- more -->

## 1. 题目描述

给你两个单词 word1 和 word2， 请返回将 word1 转换成 word2 所使用的最少操作数 。

你可以对一个单词进行如下三种操作：

- 插入一个字符
- 删除一个字符
- 替换一个字符

#### 示例1
{% highlight string %}
输入：word1 = "horse", word2 = "ros"
输出：3
解释：
horse -> rorse (将 'h' 替换为 'r')
rorse -> rose (删除 'r')
rose -> ros (删除 'e')
{% endhighlight %}

#### 示例2
{% highlight string %}
输入：word1 = "intention", word2 = "execution"
输出：5
解释：
intention -> inention (删除 't')
inention -> enention (将 'i' 替换为 'e')
enention -> exention (将 'n' 替换为 'x')
exention -> exection (将 'n' 替换为 'c')
exection -> execution (插入 'u')
{% endhighlight %}

## 2. 解题思路
**深度理解编辑的含义：**

- 如果$word1[i] == word2[j]$, 那么我们接下来只需要比较$word1[0 .. i-1]$和$word2[0 .. j-1]$

- 否则，有三个选项：

    - 执行插入操作：那么接下来只需要比较$word1[0..i]$和$word2[0..j-1]$

    - 执行删除操作：那么接下来只需要比较$word1[0..i-1]$和$word2[0..j]$
    
    - 执行替换操作: 那么接下来只需要比较$word1[0..i-1]$和$word2[0..j-1]$
>ps: 选择上述三个选项中最小的那个加1


**模式识别:**

- 一旦涉及子问题，可以用自顶向下的递归，和自底向上的动态规划

## 3. 算法实现

#### 递归解法
{% highlight string %}
class Solution {
public:
    int minDistance(string word1, string word2) {
		if(word1.length() == 0 || word2.length() == 0){
			return max(word1.length(), word2.length());
		}
		
		if(word1.back() == word2.back()){
			return minDistance(word1.substr(0, word1.length()-1), word2.substr(0, word2.length() -1));
		}
		
		//计算执行插入操作
		int a = 1 + minDistance(word1, word2.substr(0, word2.length()-1));
		
		//计算执行删除操作
		int b = 1 + minDistance(word1.substr(0, word1.length() -1), word2);
		
		//计算执行替换操作
		int c = 1 + minDistance(word1.substr(0, word1.length() -1), word2.substr(0, word2.length() -1));
		
		return min(min(a, b), c);
	}
};
{% endhighlight %}

对于上述递归算法的实现，在运行时会超时，原因是存在大量的重复计算：
{% highlight string %}
//计算执行插入操作
int a = 1 + minDistance(word1, word2.substr(0, word2.length()-1));

//计算执行删除操作
int b = 1 + minDistance(word1.substr(0, word1.length() -1), word2);

//计算执行替换操作
int c = 1 + minDistance(word1.substr(0, word1.length() -1), word2.substr(0, word2.length() -1));
{% endhighlight %}

这里我们有两种优化方式：

- Memorization

- 动态规划


#### 动态规划解法
我们从上面的解题思路中抽象出状态转移方程：

- 如果$word1[i]==word2[j]$，那么$op[i][j]=op[i-1][j-1]$

- 否则, $op[i][j]=1+min(op[i][j-1], op[i-1][j], op[i-1][j-1])$

>ps: 这里的op[i][j]的含义为将word1[0..i]变成word2[0..j]所需要的最小操作次数

* 动态规划实现方式1
{% highlight string %}  
class Solution {
public:
    int minDistance(string word1, string word2) {
		int length1 = word1.size();
		int length2 = word2.size();
		
		vector<vector<int>> op;
		
		//定义出口: 即word1为空或者word2为空的情况
		for(int i = 0; i < length1 + 1, i++){
			vector<int> row;
			
			for(int j = 0; j < length2 + 1; j++){
				if (i == 0){
					row.push_back(j);
				}else if(j == 0){
					row.push_back(i);
				}else{
					row.push_back(0);
				}
			}
			
			op.push_back(row);
		}
		
		//根据转移方程实现动态规划
		for(int i = 0; i < length1; i++){
			for(int j = 0; j <length2; j++){
				if(word1[i] == word2[j]){
					op[i+1][j+1] = op[i][j];
				}else{
					op[i+1][j+1] = 1 + min(min(op[i+1][j], op[i][j+1]), op[i][j]);
				}
			}
		}
		
		return op[length1][length2];
	}
};
{% endhighlight %}

* 动态规划实现方式2
{% highlight string %}
class Solution {
public:
    int minDistance(string word1, string word2) {
        int n = word1.length();
        int m = word2.length();

        // 有一个字符串为空串
        if (n * m == 0) return n + m;

        // DP 数组
        vector<vector<int>> D(n + 1, vector<int>(m + 1));

        // 边界状态初始化
        for (int i = 0; i < n + 1; i++) {
            D[i][0] = i;
        }
        for (int j = 0; j < m + 1; j++) {
            D[0][j] = j;
        }

        // 计算所有 DP 值
        for (int i = 1; i < n + 1; i++) {
            for (int j = 1; j < m + 1; j++) {
                int left = D[i - 1][j] + 1;                 //执行删除操作
                int down = D[i][j - 1] + 1;                 //执行插入操作
                int left_down = D[i - 1][j - 1];

                if (word1[i - 1] != word2[j - 1]) left_down += 1;

                D[i][j] = min(left, min(down, left_down));

            }
        }
        return D[n][m];
    }
};

{% endhighlight %}

    
  






<br />
<br />
**[参看]:**




<br />
<br />
<br />


