---
layout: post
title: LevelDB频繁压缩导致的生产事故
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

ceph中OSD频繁压缩，可能导致OSD无法进行读写操作，在此做一个记录。


<!-- more -->


## 1. 事故现象
ceph中OSD节点因LevelDB频繁出现：
<pre>
2019-05-02 17:27:01.355656 7ff1c5075700  1 leveldb: Current memtable full; waiting...
2019-05-02 17:27:01.422993 7ff2465dd700  1 leveldb: Level-0 table #470367: 8198532 bytes OK
2019-05-02 17:27:01.496324 7ff2465dd700  1 leveldb: Delete type=0 #470364
2019-05-02 17:27:01.511052 7ff2465dd700  1 leveldb: Moved #470320 to level-2 2127798 bytes OK: files[ 10 47 170 1746 8407 4565 0 ]
2019-05-02 17:27:01.511209 7ff2465dd700  1 leveldb: Level-0 table #470369: started
2019-05-02 17:26:54.145137 7ff1c4874700  1 leveldb: Too many L0 files; waiting...
2019-05-02 17:26:54.145295 7ff2465dd700  1 leveldb: Moved #469945 to level-4 2127420 bytes OK: files[ 12 14 155 1746 8407 4565 0 ]
2019-05-02 17:26:54.145480 7ff2465dd700  1 leveldb: Compacting 12@0 + 14@1 files
2019-05-02 17:26:54.145491 7ff1c4874700  1 leveldb: Too many L0 files; waiting...
2019-05-02 17:26:54.198582 7ff2465dd700  1 leveldb: Generated table #470286: 16677 keys, 2126282 bytes
2019-05-02 17:26:54.257033 7ff2465dd700  1 leveldb: Generated table #470287: 16334 keys, 2128515 bytes
2019-05-02 17:26:54.315654 7ff2465dd700  1 leveldb: Generated table #470288: 15972 keys, 2128662 bytes
</pre>
导致数据读写请求受阻。







<br />
<br />

**[参看]**

1. [Ceph亚太峰会RGW议题分享](https://cloud.tencent.com/developer/article/1146413)

2. [leveldb 产生大量ldb文件，导致IO error](https://www.oschina.net/question/2848189_2187722?p=1)

3. [Ceph蹚坑笔记](https://blog.csdn.net/jeegnchen/article/details/50827154)

<br />
<br />
<br />

