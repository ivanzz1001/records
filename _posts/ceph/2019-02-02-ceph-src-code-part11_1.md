---
layout: post
title: ceph数据修复
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

当PG完成了Peering过程后，处于Active状态的PG就可以对外提供服务了。如果该PG的各个副本上有不一致的对象，就需要进行修复。Ceph的修复过程有两种：Recovery和Backfill。

Recovery是仅依据PG日志中的缺失记录来修复不一致的对象。Backfill是PG通过重新扫描所有的对象，对比发现缺失的对象，通过整体拷贝来修复。当一个OSD失效时间过长导致无法根据PG日志来修复，或者新加入的OSD导致数据迁移时，就会启动Backfill过程。



<!-- more -->




<br />
<br />

**[参看]**



<br />
<br />
<br />

