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

9. [自然对数(LN)](https://upimg.baike.so.com/doc/315890-334445.html) ps: 本链接对于理解此算法很有效

10. [kernel_awsome_feature](https://github.com/0voice/kernel_awsome_feature/blob/main/「核心」Ceph学习三部曲之一:A%20First%20Glance%20At%20CRUSH.md)

11. [understanding bucket_straw2_choose](https://www.spinics.net/lists/ceph-devel/msg36979.html)

12. [通俗易懂理解指数分布](https://www.cnblogs.com/Renyi-Fan/p/13948742.html)

13. [泊松分布和指数分布：10分钟教程](https://mp.weixin.qq.com/s?__biz=MzU0MDQ1NjAzNg==&mid=2247534580&idx=3&sn=96b21a6d77194db5dd94b01b0c7ff214&chksm=fb3ae2ffcc4d6be90e38d7069dcf055ebfb2b4f14cad2164fa2383401e0d4f86fc3faa9b3278&scene=27)

<br />
<br />
<br />


