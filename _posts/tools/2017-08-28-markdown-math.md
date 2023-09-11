---
layout: post
title: markdown编写数学公式(转)
tags:
- tools
categories: tools
description: markdown编写数学公式
---

在计算机这一块，我们肯定会接触到数学，数学中又包含很多公式，但是到现在，手写这些公式应该不陌生，但是如果让你电脑敲出来，你绝对很懵逼，这也造成了我们有时候写笔记时一些公式没办法在电脑上像我们手写一样灵活，今天在这里分享给大家使用markdown描述公式的语法。

<!-- more -->

## 1. markdown数学公式
markdown数学公式：使用```$```，将数学公式写在两个```$```之间。写在两个```$$```之间是把公式居中。

1) **上下标**

```^```表示上标， ```_```表示下标，如果上标或下标内容多于一个字符，则使用```{}```括起来。

例如：
<pre>
$(x^2 + x^2 )^{x^y}+ x_1^2= y_1 - y_2^{x_1^2-y_1^2}$
</pre>
最后显示结果就是: $(x^2 + x^2 )^{x^y}+ x_1^2= y_1 - y_2^{x_1^2-y_1^2}$

这个等式在数学上并不成立哦，单纯只是为了演示。

>ps: 这里说一点，在平时中我们完全有两个^表示上标，两个~表示下标，个人感觉这种在不涉及复杂的数学公式单纯表示某个变量或未知数时更方便，相信大家也知道。

2) **分数**

公式 ```\frac{分子}{分母}```，或 ```分子 \over 分母```

例如：
<pre>
$\frac{1+x}{y-1}$ 或 $x \over x+y$
</pre>

结果：$\frac{1+x}{y-1}$ 或 $x \over x+y$

这里有一个小细节需要注意， ```$\frac```的$和\之间不能有空格哦，不然会报错；而```$x \over x+y$```的\over前后要有空格哦，用来区分分子分母，没有的话也会报错。


3) **开方**

公式```\sqrt[n]{a}```，其中n是系数，a是自变量，如果省略{n}从数学上来讲它是默认开二次跟

例如：
<pre>
 $\sqrt[3]{4}$ 或 $\sqrt{9}$
</pre>

结果：$\sqrt[3]{4}$ 或 $\sqrt{9}$

4) **括号**

* ```()```、```[]```直接写就行，而 {} 则需要转义（转义：需要左括号前加\和右括号前加\）

例如：
<pre>
$f(x, y) = x^2 + y^2, x \epsilon [0, 100], y \epsilon \{1,2,3\}$
</pre>
结果：$f(x, y) = x^2 + y^2, x \epsilon [0, 100], y \epsilon \{1,2,3\}$

* 长括号，需要左括号前加```\left```和右括号前加```\right```，（此大括号非彼大括号）

例如：
<pre>
$(\sqrt{1 \over 2})^2$加大括号后 $\left(\sqrt{1 \over 2}\right)^2$
</pre>
结果:  $(\sqrt{1 \over 2})^2$ 变成 $\left(\sqrt{1 \over 2}\right)^2$

* ```\left``` 和 ```\right```必须成对出现，对于不显示的一边可以使用```.```代替。
例如：
<pre>
$\frac{du}{dx} | _{x=0}$加大后 $\left. \frac{du}{dx} \right| _{x=0}$
</pre>

$\frac{du}{dx} | _{x=0}$ 变成了 $\left. \frac{du}{dx} \right| _{x=0}$


* 大括号用\begin{cases}表示开始，用\end{cases}表示结束，中间\\来换行
例如：
<pre>
$f(x,y):\begin{cases} x^2+y^2=1\\ x-y = 0 \end{cases}$
</pre>

结果：$f(x,y):\begin{cases} x^2+y^2=1\\ x-y = 0 \end{cases}$


5） **向量**
公式```\vec{a}```

例如：
<pre>
$\vec d \cdot \vec b = 1$
</pre>

结果：$\vec d \cdot \vec b = 1$

注意像这种没有{}来区分的，采用的都是空格制，需要注意格式。

6) **定积分**

公式```\int```，```_```表示下限```^```表示上限。
例如：
<pre>
符号：$\int$，示例公式：$\int_0^1x^2dx$
</pre>
符号：$\int$，示例公式：$\int_0^1x^2dx$

7) **正负无穷**
<pre>
正无穷：其表达式为$+\infty$
负无穷：其表达式为$-\infty$
</pre>

结果: $+\infty$, $-\infty$

8) **极限**

公式```\lim_{n\rightarrow+\infty}```，其中```\rightarrow```表示右箭头

例如：
<pre>
$\lim_{n\rightarrow+\infty}\frac{1}{n}$
</pre>
结果: $\lim_{n\rightarrow+\infty}\frac{1}{n}$

>ps: 毕竟电脑不能完美替代手写，虽然手写一直放在lim下面

9) **累加累乘**

公式累加```\sum_1^n```，累乘```\prod_{i=0}^n```

例如：
<pre>
累加$\sum_1^n$
累乘$\prod_{i=0}^n$
</pre>
结果：累加$\sum_1^n$ 和累乘 $\prod_{i=0}^n$

10) **省略号**

公式```\ldots`````` 表示底线对其的省略号，```\cdots`````` 表示中线对其的省略号，```\cdot```点乘号。

例如：
<pre>
$f(x_1,x_2,\ldots,x_n) = \left({1 \over x_1}\right)^2+\left({1 \over x_2}\right)^2+\cdots+\left({1 \over x_n}\right)^2$
</pre>

结果：$f(x_1,x_2,\ldots,x_n) = \left({1 \over x_1}\right)^2+\left({1 \over x_2}\right)^2+\cdots+\left({1 \over x_n}\right)^2$


11） **数学符号**

![markdown-math](https://ivanzz1001.github.io/records/assets/img/tools/markdown-math.png)

12) **三角函数**

![markdown-triangle](https://ivanzz1001.github.io/records/assets/img/tools/markdown-triangle.png)

其他的三角函数都是取我们数学中平时用的简写。


13) **对数符号**

* ```$\log$```的结果是 log ⁡ 

* ```$\lg$```的结果是 lg 

* ```$\ln$```的结果是 ln 

14) **积分**

![markdown-jifen](https://ivanzz1001.github.io/records/assets/img/tools/markdown-jifen.png)

15) **希腊字母**

![markdown-xila](https://ivanzz1001.github.io/records/assets/img/tools/markdown-xila.png)


16) **矩阵**

* 起始标记：```$\begin{matrix}```

* 结束标记: ```\end{matrix}$```

* 每一行末尾标记```\\```，行间元素之间用 ```&``` 分隔

例如：
<pre>
$$\begin{matrix}
0&1&1\\
1&1&0\\
1&0&1\\
\end{matrix}$$
</pre>

结果为：
$$\begin{matrix}
0&1&1\\
1&1&0\\
1&0&1\\
\end{matrix}$$

* 如果想要添加矩阵边框，遵循在起始、结束标记用下列词替换 matrix

![markdown-matrix](https://ivanzz1001.github.io/records/assets/img/tools/markdown-matrix.png)



<br />
<br />

**[参看]**

1. [markdown编写数学公式](https://blog.csdn.net/weixin_51496226/article/details/131742283)



<br />
<br />
<br />

