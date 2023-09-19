---
layout: post
title: 动态规划背包问题(转)
tags:
- data-structure
categories: data-structure
description: 动态规划背包问题
---



本文介绍一下动态规划之背包问题。


<!-- more -->



## 1. 前言
本文主要介绍常见的四种背包问题，思维导图如下：

![bag0](https://ivanzz1001.github.io/records/assets/img/data_structure/dp/dp_bag0.png)

## 2 01背包
> 现有N件物品和一个最多能承重M的背包，第i件物品的重量是$w_i$，价值是$v_i$。在背包能承受的范围内，试问将哪些物品装入背包后可使总价值最大，求这个最大价值。


因为每件物品只有选与不选两种状态，所以该问题又称01背包问题。

设dp[i][j]的含义是: 在背包承重为j的前提下，从前i个物品中选能够得到的最大价值。不难发现dp[N][M]就是本题的答案。


如何计算dp[i][j]呢？我们可以将它划分为以下两部分：

* 选第i个物品: 由于第i个物品一定会被选择，那么相当于从前```i-1```个物品中选且总重量不超过(j-w[i])，对应dp[i-1][j-w[i]] + v[i]

* 不选第i个物品: 意味着从前```i-1```个物品中选且总重量不超过j, 对应dp[i-1][j]

结合以上两点可得递推公式：
$
dp[i][j] = max(dp[i−1][j],dp[i−1][j−w[i]]+v[i])
$

由于下标不能是负数，所以上述递推公式要求$j \ge w[i]$。当$j \lt w[i]$时，意味着第i个物品无法装进背包, 此时dp[i][j]=dp[i-1][j]。综合以上可以得出：
$
dp[i][j] = \begin{cases} dp[i-1][j], \qquad \qquad \qquad \qquad  \qquad \qquad \qquad \qquad j<w[i] \\ max\{dp[i-1][j], dp[i-1][j-w[i]] + v[i]\}, \qquad \quad j>=w[i] \end{cases}
$






<br />
<br />
**[参看]:**

1. [动态规划专题——背包问题](https://blog.csdn.net/raelum/article/details/128996521)



<br />
<br />
<br />


