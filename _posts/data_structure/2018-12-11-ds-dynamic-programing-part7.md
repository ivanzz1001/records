---
layout: post
title: 买卖股票最佳时机
tags:
- data-structure
categories: data-structure
description: 买卖股票最佳时机
---


本文记录一下Leetcode[动态规划: 买卖股票最佳时机](https://leetcode.cn/problems/best-time-to-buy-and-sell-stock-iv/description/?envType=study-plan-v2&envId=top-interview-150)问题的解法。


<!-- more -->

## 1. 题目描述

给你一个整数数组 prices 和一个整数 k ，其中 prices[i] 是某支给定的股票在第 i 天的价格。

设计一个算法来计算你所能获取的最大利润。你最多可以完成 k 笔交易。也就是说，你最多可以买 k 次，卖 k 次。

注意：你不能同时参与多笔交易（你必须在再次购买前出售掉之前的股票）。


#### 示例1
{% highlight string %}
输入：k = 2, prices = [2,4,1]
输出：2
解释：在第 1 天 (股票价格 = 2) 的时候买入，在第 2 天 (股票价格 = 4) 的时候卖出，
      这笔交易所能获得利润 = 4-2 = 2 。
{% endhighlight %}

#### 示例2
{% highlight string %}
输入：k = 2, prices = [3,2,6,5,0,3]
输出：7

解释：在第 2 天 (股票价格 = 2) 的时候买入，在第 3 天 (股票价格 = 6) 的时候卖出, 
     这笔交易所能获得利润 = 6-2 = 4 。

     随后，在第 5 天 (股票价格 = 0) 的时候买入，在第 6 天 (股票价格 = 3) 的时候卖出,
     这笔交易所能获得利润 = 3-0 = 3 。
{% endhighlight %}

## 2. 思路与算法

与其余的股票问题类似，我们使用一系列变量存储「买入」的状态，再用一系列变量存储「卖出」的状态，通过动态规划的方法即可解决本题。

我们用$buy[i][j]$表示对于数组$prices[0..i]$中的价格而言，进行恰好$j$笔交易，并且当前手上持有一支股票，这种情况下的最大利润；用$sell[i][j]$表示恰好进行$j$笔交易，并且当前手上不持有股票，这种情况下的最大利润。


那么我们可以对状态转移方程进行推导:

1） **buy[i][j]状态转移方程的推导**

对于$buy[i][j]$，我们考虑当前手上持有的股票是否是在第$i$天买入的：

- 如果是在第$i$天买入的，那么在第$i-1$天时，我们手上不持有股票，对应的状态$sell[i-1][j]$，并且需要扣除$prices[i]$的买入花费；

- 如果不是在第$i$天买入的，那么在第$i-1$天时，我们手上持有股票，对应状态$buy[i-1][j]$

  通过上面两种情况我们可以得到状态转移方程：

$$
buy[i][j]=max \{ buy[i−1][j],sell[i−1][j]−price[i] \}
$$


2）**sell[i][j]状态转移方程的推导**

同理对于$sell[i][j]$:

- 如果是第$i$天卖出的，那么在第$i-1$天时，我们手上持有股票，对应状态$buy[i−1][j−1]$，并且需要增加$prices[i]$的卖出收益；
 
- 如果不是第$i$天卖出的，那么在第$i-1$天时，我们手上不持有股票，对应状态$sell[i-1][j]$;
 
那么我们可以得到状态转移方程：

$$
sell[i][j]=max \{ sell[i−1][j],buy[i−1][j−1]+price[i] \}
$$


----------

由于在所有的$n$天结束后，手上不持有股票对应的最大利润一定是严格由于手上持有股票对应的最大利润的，然而完成的交易数并不是越多越好（例如数组$prices[i]$单调递减，我们不进行任何交易才是最优的），因此最终的答案即为 $sell[n-1][0..k]$中的最大值。


### 2.1 细节

在上述的状态转移方程中，确定边界条件是非常重要的步骤。我们可以考虑将所有的 $buy[0][0..k]$ 以及 $sell[0][0..k]$设置为边界。

- 对于$buy[0][0..k]$，由于只有$prices[0]$唯一的股价，因此我们不可能进行过任何交易，那么我们可以将所有的$buy[0][1..k]$设置为一个非常小的值，表示不合法的状态。而对于$buy[0][0]$，它的值为$-prices[0]$，即「我们在第0天以prices[0]的价格买入股票」是唯一满足手上持有股票的方法。

- 对于$sell[0][0..k]$，同理我们可以将所有的$sell[0][1..k]$设置为一个非常小的值，表示不合法的状态。而对于$sell[0][0]$，它的值为0，即「我们在第 000 天不做任何事」是唯一满足手上不持有股票的方法。


在设置完边界之后，我们就可以使用二重循环，在$i\in[1,n), j\in[0,k]$的范围内进行状态转移。需要注意的是，$sell[i][j]$的状态转移方程中包含$buy[i-1][j-1]$，在$j=0$时其表示不合法的状态，因此在$j=0$时，我们无需对$sell[i][j]$进行转移，让其保持值为0即可。

最后需要注意的是，本题中k的最大值可以达到$10^9$，然而这是毫无意义的，因为n天最多只能进行 $\lfloor \frac{n}{2} \rfloor$笔交易，其中$lfloor x \rfloor$表示对$x$向下取整。因此我们可以将 kkk 对$\lfloor \frac{n}{2} \rfloor$ 取较小值之后再进行动态规划。


### 2.2 代码参考
{% highlight string %}
class Solution {
public:
    int maxProfit(int k, vector<int>& prices) {
        if (prices.empty()) {
            return 0;
        }

        int n = prices.size();
        k = min(k, n / 2);
        vector<vector<int>> buy(n, vector<int>(k + 1));
        vector<vector<int>> sell(n, vector<int>(k + 1));

        buy[0][0] = -prices[0];
        sell[0][0] = 0;
        for (int i = 1; i <= k; ++i) {
            buy[0][i] = sell[0][i] = INT_MIN / 2;
        }

        for (int i = 1; i < n; ++i) {
            buy[i][0] = max(buy[i - 1][0], sell[i - 1][0] - prices[i]);
            for (int j = 1; j <= k; ++j) {
                buy[i][j] = max(buy[i - 1][j], sell[i - 1][j] - prices[i]);
                sell[i][j] = max(sell[i - 1][j], buy[i - 1][j - 1] + prices[i]);   
            }
        }

        return *max_element(sell[n - 1].begin(), sell[n - 1].end());
    }
};
{% endhighlight %}
  






<br />
<br />
**[参看]:**




<br />
<br />
<br />


