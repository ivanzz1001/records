---
layout: post
title: crush之straw2算法
tags:
- ceph
- crushmap
categories: ceph
description: crushmap算法详解
---

本文介绍以下straw2算法，它具有在增加、删除OSD时减少数据移动方面具有重要的作用。



<!-- more -->







<br />
<br />

**[参看]**

1. [Bucket随机选择算法](https://developer.aliyun.com/article/1237755)

2. [指数分布Wiki](https://encyclopedia.thefreedictionary.com/Inverse+transform+sampling)

3. [探究分布式系统数据分布策略](https://zhuanlan.zhihu.com/p/446177554)

4. [ceph crush算法源码分析](https://blog.csdn.net/XingKong_678/article/details/51590459)

5. [object至PG映射源码分析](https://www.dovefi.com/post/%E6%B7%B1%E5%85%A5%E7%90%86%E8%A7%A3crush3object%E8%87%B3pg%E6%98%A0%E5%B0%84%E6%BA%90%E7%A0%81%E5%88%86%E6%9E%90/)

6. [ceph中相关实现源代码](https://github.com/ceph/ceph/blob/main/src/crush/mapper.c#L504)

7. [crush算法](https://blog.csdn.net/laiyuhua120/article/details/133606685)

8. [机器学习中的统计学基础](https://zhuanlan.zhihu.com/p/99910209)

<br />
<br />
<br />


