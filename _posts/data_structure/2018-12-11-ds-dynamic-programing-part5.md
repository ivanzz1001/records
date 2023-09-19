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

设dp[i][j]的含义是: 在背包承重为```j```的前提下，从前```i```个物品中选能够得到的最大价值。不难发现dp[N][M]就是本题的答案。


如何计算dp[i][j]呢？我们可以将它划分为以下两部分：

* 选第i个物品: 由于第i个物品一定会被选择，那么相当于从前```i-1```个物品中选且总重量不超过```j-w[i]```，对应dp[i-1][j-w[i]] + v[i]

* 不选第i个物品: 意味着从前```i-1```个物品中选且总重量不超过j, 对应dp[i-1][j]

结合以上两点可得递推公式：

$
dp[i][j] = max(dp[i−1][j],dp[i−1][j−w[i]]+v[i])
$


由于下标不能是负数，所以上述递推公式要求$j \ge w[i]$。当$j \lt w[i]$时，意味着第i个物品无法装进背包, 此时dp[i][j]=dp[i-1][j]。综合以上可以得出：

$
dp[i][j] = \begin{cases} dp[i-1][j], \qquad \qquad \qquad \qquad \qquad \qquad \qquad \qquad j<w[i] \\\ max(dp[i-1][j], dp[i-1][j-w[i]] + v[i]), \qquad j \ge w[i] \end{cases}
$

dp数组应当如何初始化呢？当背包承重为0时，显然装不下任何物品，所以$dp[i][0]=0 \;(1 \leq i \leq N)$; 若一个物品也不选（即从前0个物品中选），此时最大价值也是0，所以$dp[0][j]=0 \; (0 \leq j\leq M)$。由此可知， dp数组应当全0初始化，即声明为全局变量。

题目链接: [AcWing 2. 01背包问题](https://www.acwing.com/problem/content/2/)

{% highlight string %}
#include <bits/stdc++.h>

using namespace std;

const int N = 1010;

int w[N], v[N];
int dp[N][N];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    for (int i = 1; i <= n; i++) cin >> w[i] >> v[i];

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            if (j < w[i]) dp[i][j] = dp[i - 1][j];
            else dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - w[i]] + v[i]);
        }
    }

    cout << dp[n][m] << "\n";

    return 0;
}
{% endhighlight %}

时间复杂度为$O(mn)$。


### 1.1 使用滚动数组优化
之前我们用到的dp数组是二维数组，它可以进一步优化成一维数组。

观察递推公式不难发现，dp数组中第i行的元素仅由第```i-1```行的元素得来，即第0行元素的更新值放到第1行，第1行元素的更新值放到第2行，以此类推。与其把一行的更新值放到新的一行，不如直接就地更新，因此我们的dp数组只需要一行来存储，即一维数组。

去掉dp数组的第一维后，递推公式变成:

$
dp[j] = \begin{cases} dp[j], \qquad \quad \qquad \qquad \qquad \qquad \qquad j<w[i] \\\ max(dp[j], dp[j-w[i]] + v[i]), \qquad j \ge w[i] \end{cases}
$

原先j是从1遍历至m的，现在只需从w[i] 遍历至m。但，这个遍历顺序真的对吗？

请看下图：

![bag1](https://ivanzz1001.github.io/records/assets/img/data_structure/dp/dp_bag1.png)


红色箭头表示，在二维数组中，$dp[i][j]$由$dp[i-1][j-w[i]]和dp[i-1][j]$得来，$dp[i][j+w[i]]$由$dp[i−1][j]$和$dp[i−1][j+w[i]]$得来。用一维数组的话来讲就是，第i行的$dp[j]$由第```i-1```行的$dp[j−w[i]]$和$dp[j]$得来，第```i```行的$dp[j+w[i]]$由第```i-1```行的$dp[j]$和$dp[j+w[i]]$得来。

如果```j```从小到大遍历，那么会先更新$dp[j]$, 再更新$dp[j+w[i]]$，这就导致在更新$dp[j+w[i]]$时使用的是第i行的$dp[j]$而非第```i-1```行的dp[j]，即当j从小到大遍历时，二维数组的递推式变成了：

$
dp[i][j] = \begin{cases} dp[i-1][j], \qquad \qquad \qquad \qquad \qquad \qquad \qquad j<w[i] \\\ max(dp[i-1][j], dp[i][j-w[i]] + v[i]), \qquad j \ge w[i] \end{cases}
$

>PS: 请牢记该式，后续讲解完全背包时会提到它。

这显然是错误的。事实上，让j从大到小遍历就不会出现这个问题。

{% highlight string %}
#include <bits/stdc++.h>

using namespace std;

const int N = 1010;

int w[N], v[N];
int dp[N];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    for (int i = 1; i <= n; i++) cin >> w[i] >> v[i];

    for (int i = 1; i <= n; i++)
        for (int j = m; j >= w[i]; j--)
            dp[j] = max(dp[j], dp[j - w[i]] + v[i]);

    cout << dp[m] << "\n";

    return 0;
}
{% endhighlight %}
当然，w数组和v数组也是不必要的，我们可以边输入边处理，因此可以得到01背包问题的最终版代码：
{% highlight string %}
#include <bits/stdc++.h>

using namespace std;

const int N = 1010;

int dp[N];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    for (int i = 1; i <= n; i++) {
        int w, v;
        cin >> w >> v;
        for (int j = m; j >= w; j--)
            dp[j] = max(dp[j], dp[j - w] + v);
    }

    cout << dp[m] << "\n";

    return 0;
}
{% endhighlight %}

到此为止，可以总结出，当dp数组是二维数组时，j既可以从小到大遍历也可以从大到小遍历，但当dp数组是一维数组时，j只能从大到小遍历。


## 2. 完全背包问题
> 现有N种物品和一个最多能承重M的背包，每种物品都有无限个，第i种物品的重量是$w_i$, 价值是$v_i$。在背包能承受的范围内，试问将哪些物品装入背包后可使总价值最大，求这个最大价值

设dp[i][j]的含义是：在背包承重为j的前提下，从前i种物品中选能够得到的最大价值。

如何计算dp[i][j] 呢？我们可以将它划分为以下若干部分：

* 选0个第i种物品：相当于不选第i种物品，对应dp[i−1][j]

* 选1个第i种物品：对应dp[i−1][j−w[i]]+v[i]。

* 选2个第i种物品：对应$dp[i-1][j-2\cdot w[i]]+2\cdot v[i]$ 

* ...

上述过程并不会无限进行下去，因为背包承重是有限的。设第i种物品最多能选t个，于是可知$t=\lfloor \frac{j}{w[i]}\rfloor$ 从而得到递推式：

$
dp[i][j]=\max_{0 \le k \le t} dp[i-1][j - k\cdot w[i]] + k \cdot v[i]
$


题目链接：[AcWing 3. 完全背包问题](https://www.acwing.com/problem/content/3/)
{% highlight string %}
#include <bits/stdc++.h>

using namespace std;

const int N = 1010;

int w[N], v[N];
int dp[N][N];

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    for (int i = 1; i <= n; i++) cin >> w[i] >> v[i];

    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++) {
            int t = j / w[i];
            for (int k = 0; k <= t; k++)
                dp[i][j] = max(dp[i][j], dp[i - 1][j - k * w[i]] + k * v[i]);
        }

    cout << dp[n][m] << "\n";

    return 0;
}
{% endhighlight %}
若将t的值改为$\min(1,\,j/w[i])$，则完全背包将退化为01背包。

上述代码的时间复杂度为$O(m^2\sum_iw_i^{-1})\approx O(m^2n)$, TLE是必然的。

### 2.1 使用滚动数组优化
考虑$dp[i][j]$，此时第```i```种





<br />
<br />
**[参看]:**

1. [动态规划专题——背包问题](https://blog.csdn.net/raelum/article/details/128996521)



<br />
<br />
<br />


