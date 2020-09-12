---
layout: post
title: OSD从down到out过程中osdmap的变化(2)
tags:
- ceph
categories: ceph
description: ceph pg
---


接着上一章，我们这里结合```osd3_watch.txt```日志文件，以及pgmap_active_clean.txt、pgmap_in_down.txt、pgmap_out_down.txt，从中选出```4```个具有代表性的PG，来分析一下osd0从```in+up```到```in+down```再到```out+down```这一整个过程中PG所执行的动作。

选取的4个PG如下：
<pre>
    in + up                                                        in + down                                                       out + down
---------------------------------------------------------------------------------------------------------------------------------------------------------------------
pg_stat up      up_primary	acting	acting_primary      pg_stat  up     up_primary	acting	acting_primary       pg_stat    up      up_primary	acting	acting_primary

11.4    [0,3]   0           [0,3]	0                   11.4	 [3]    3	        [3]	    3                    11.4	    [3,2]	3	        [3,2]	3

22.2c   [0,3]   0	        [0,3]	0                   22.2c	 [3]    3	        [3]	    3                    22.2c	    [5,7]	5	        [5,7]	5

22.2a   [3,0]   3	        [3,0]	3                   22.2a	 [3]    3	        [3]	    3                    22.2a	    [3,6]	3	        [3,6]	3

22.16   [3,7]   3	        [3,7]	3                   22.16	 [3,7]	3       	[3,7]	3                    22.16	    [3,7]	3	        [3,7]	3
</pre>

上面4个PG代表4种典型的场景：

* **PG 11.4** : PG的主osd关闭，在osd out之后，PG的其中一个副本进行remap

* **PG 22.2c**: PG的主OSD关闭，在osd out之后，PG的两个副本进行remap

* **PG 22.2a**: PG的副本OSD关闭，在osd out之后，PG的其中一个副本进行remap

* **PG 22.16**: 关闭的OSD并不是PG的任何副本

使用如下命令从osd3_watch.txt中分别导出该PG相关的日志信息：
{% highlight string %}
# grep -rnw "11.4" ./osd3_watch.txt > ./osd3_watch_pg_11.4.txt
# grep -rnw "22.2c" ./osd3_watch.txt > ./osd3_watch_pg_22.2c.txt
# grep -rnw "22.2a" ./osd3_watch.txt > ./osd3_watch_pg_22.2a.txt
# grep -rnw "22.16" ./osd3_watch.txt > ./osd3_watch_pg_22.16.txt
{% endhighlight %}

下面我们就针对这4种场景分别来分析。
<!-- more --> 

## 1. PG 11.4分析
这里我们过滤osd3_watch.txt日志文件，找出所有与PG 11.4相关的日志（如下日志经过了适当的修改）：
{% highlight string %}

{% endhighlight %}










<br />
<br />
<br />
