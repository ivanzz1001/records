---
layout: post
title: ceph的数据读写
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph的服务端OSD（书中简称OSD模块或者OSD）的实现。其对应的源代码在src/osd目录下。OSD模块是Ceph服务进程的核心实现，它实现了服务端的核心功能。本章先介绍OSD模块静态类图相关数据结构，再着重介绍服务端数据的写入和读取流程。

OSD进程启动的主函数位于： src/ceph_osd.cc

<!-- more -->


## 1. OSD模块静态类图
OSD模块的静态类图如下图6-1所示：

![ceph-chapter5-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter6_1.jpg)

OSD模块的核心类及其之间的静态类图说明如下：

* 类OSD和OSDService是核心类，处理一个osd节点层面的工作。在早期的版本中，OSD和OSDService是一个类。由于OSD的类承载了太多的功能，后面的版本中引入OSDService类，分担一部分原OSD类的功能；

* 类PG处理PG相关的状态维护以及实现PG层面的基本功能。其核心功能是用boost库的statechart状态机来实现的PG状态转换；

* 类ReplicatedPG继承了类PG，在其基础上实现了PG内的数据读写以及数据恢复相关的操作；

* 类PGBackend的主要功能是把数据以事务的形式同步到一个PG其他从OSD节点上

&emsp; - PGBackend的内部类PGTransaction就是同步的事务接口，其两个类型的实现分别对应RPGTransaction和ECTransaction两个子类；

&emsp; - PGBackend两个子类ReplicatedBackend和ECBackend分别对应PG的两种类型的实现


* 类SnapMapper额外保存对象和对象的快照信息，在对象的属性里保存了相关的快照信息。这里保存的快照信息为冗余信息，用于数据校验。

下面我们再来看一下OSD运行起来之后有哪些工作线程：
<pre>
# ps -ef | grep osd
root       12025       1 27  2019 ?        255-07:47:08 /usr/bin/ceph-osd -f --cluster ceph --id 12 --setuser root --setgroup root
root       12035       1 32  2019 ?        304-21:34:58 /usr/bin/ceph-osd -f --cluster ceph --id 9 --setuser root --setgroup root
root       12085       1 32  2019 ?        304-15:44:09 /usr/bin/ceph-osd -f --cluster ceph --id 16 --setuser root --setgroup root
root       12095       1 16  2019 ?        156-18:47:42 /usr/bin/ceph-osd -f --cluster ceph --id 10 --setuser root --setgroup root
root       12115       1 34  2019 ?        319-13:58:10 /usr/bin/ceph-osd -f --cluster ceph --id 14 --setuser root --setgroup root
root       12144       1 28  2019 ?        267-08:09:04 /usr/bin/ceph-osd -f --cluster ceph --id 15 --setuser root --setgroup root
root       12155       1 30  2019 ?        278-05:29:00 /usr/bin/ceph-osd -f --cluster ceph --id 17 --setuser root --setgroup root
root       12165       1 28  2019 ?        259-21:16:13 /usr/bin/ceph-osd -f --cluster ceph --id 13 --setuser root --setgroup root
root       37770       1 41 Mar22 ?        103-17:30:20 /usr/bin/ceph-osd -f --cluster ceph --id 11 --setuser root --setgroup root
root     3475542 3475501  0 10:58 pts/0    00:00:00 grep --color=auto osd
# ps -T -p 12025
    PID    SPID TTY          TIME CMD
  12025   12025 ?        00:00:02 ceph-osd
  12025   12174 ?        12:33:10 log
  12025   12215 ?        00:04:04 service
  12025   12225 ?        00:00:00 admin_socket
  12025   12244 ?        00:06:17 ceph-osd
  12025   12245 ?        00:07:36 ms_reaper
  12025   12264 ?        00:00:01 ms_reaper
  12025   12274 ?        00:00:00 ms_reaper
  12025   12284 ?        00:00:00 ms_reaper
  12025   12294 ?        00:00:09 ms_reaper
  12025   12304 ?        00:00:00 ms_reaper
  12025   12314 ?        00:42:13 safe_timer
  12025   12315 ?        00:59:27 safe_timer
  12025   12325 ?        00:00:00 safe_timer
  12025   13345 ?        03:01:45 wb_throttle
  12025   13354 ?        19:41:26 filestore_sync
  12025   13365 ?        17:49:42 journal_write
  12025   13374 ?        10:43:32 journal_wrt_fin
  12025   13375 ?        05:48:26 fn_jrn_objstore
  12025   13384 ?        3-09:00:53 tp_fstore_op
  12025   13385 ?        3-08:57:35 tp_fstore_op
  12025   13394 ?        3-08:55:45 tp_fstore_op
  12025   13395 ?        3-08:54:53 tp_fstore_op
  12025   13404 ?        3-08:57:47 tp_fstore_op
  12025   13405 ?        06:36:03 fn_odsk_fstore
  12025   13414 ?        11:07:32 fn_appl_fstore
  12025   13415 ?        00:00:03 safe_timer
  12025   14224 ?        00:13:53 ms_dispatch
  12025   14225 ?        00:00:00 ms_local
  12025   14234 ?        00:07:16 ms_accepter
  12025   14235 ?        01:53:03 ms_dispatch
  12025   14244 ?        00:00:00 ms_local
  12025   14254 ?        00:00:01 ms_dispatch
  12025   14255 ?        00:00:00 ms_local
  12025   14264 ?        00:00:00 ms_dispatch
  12025   14265 ?        00:00:00 ms_local
  12025   14275 ?        00:00:00 ms_dispatch
  12025   14284 ?        00:00:00 ms_local
  12025   14294 ?        00:00:00 ms_dispatch
  12025   14295 ?        00:00:00 ms_local
  12025   14304 ?        00:04:38 safe_timer
  12025   14305 ?        00:00:00 fn_anonymous
  12025   14314 ?        04:35:33 tp_osd
  12025   14315 ?        04:35:27 tp_osd
  12025   14334 ?        23-09:11:24 tp_osd_tp
  12025   14335 ?        21-04:24:33 tp_osd_tp
  12025   14344 ?        23-13:28:19 tp_osd_tp
  12025   14345 ?        20-14:27:07 tp_osd_tp
  12025   14354 ?        20-09:15:53 tp_osd_tp
  12025   14355 ?        23-08:43:44 tp_osd_tp
  12025   14364 ?        21-04:21:41 tp_osd_tp
  12025   14365 ?        23-13:24:38 tp_osd_tp
  12025   14374 ?        20-15:07:18 tp_osd_tp
  12025   14375 ?        20-09:12:04 tp_osd_tp
  12025   14384 ?        3-21:58:35 tp_osd_recov
  12025   14385 ?        00:22:46 tp_osd_disk
  12025   14394 ?        00:07:39 tp_osd_cmd
  12025   14395 ?        04:26:42 osd_srv_heartbt
  12025   14404 ?        00:00:00 fn_anonymous
  12025   14405 ?        00:00:00 fn_anonymous
  12025   14414 ?        00:00:00 safe_timer
  12025   14415 ?        00:00:00 safe_timer
  12025   14424 ?        00:00:00 safe_timer
  12025   14425 ?        00:00:00 safe_timer
  12025   14434 ?        00:00:01 osd_srv_agent
  12025   14445 ?        00:00:00 sginal_handler
  12025    4936 ?        1-00:04:18 tp_fstore_op
  12025 2457356 ?        00:00:14 ms_pipe_write
  12025 2457359 ?        00:00:10 ms_pipe_read
  12025 2570326 ?        00:00:00 ms_accepter
  12025 2570327 ?        00:00:00 ms_accepter
  12025 2570328 ?        00:00:00 ms_accepter
  12025 2570366 ?        00:00:22 ms_pipe_write
  12025 2570367 ?        00:00:22 ms_pipe_write
  12025 2570371 ?        00:00:18 ms_pipe_read
  12025 2570372 ?        00:00:18 ms_pipe_read
  12025 2570374 ?        00:00:22 ms_pipe_write
  12025 2570375 ?        00:00:22 ms_pipe_write
  12025 2570390 ?        00:00:16 ms_pipe_write
  12025 2570392 ?        00:00:17 ms_pipe_read
  12025 2570394 ?        00:00:16 ms_pipe_write
  12025 2570395 ?        00:00:16 ms_pipe_read
  12025 2570396 ?        00:00:16 ms_pipe_write
  12025 2570398 ?        00:00:16 ms_pipe_write
  12025 2570400 ?        00:00:16 ms_pipe_write
  12025 2570401 ?        00:00:16 ms_pipe_write
  12025 2570402 ?        00:00:16 ms_pipe_write
  12025 2570405 ?        00:00:16 ms_pipe_write
  12025 2570412 ?        00:00:18 ms_pipe_read
  12025 2570415 ?        00:00:17 ms_pipe_read
  12025 2570416 ?        00:00:17 ms_pipe_read
  12025 2570417 ?        00:00:16 ms_pipe_read
  12025 2570419 ?        00:00:16 ms_pipe_read
  12025 2570423 ?        00:00:16 ms_pipe_read
  12025 2570424 ?        00:00:16 ms_pipe_read
  12025 2570425 ?        00:00:16 ms_pipe_read
  12025 2570454 ?        00:00:31 ms_pipe_read
  12025 2570455 ?        00:00:30 ms_pipe_read
  12025 2570456 ?        00:00:23 ms_pipe_read
  12025 2570457 ?        00:00:16 ms_pipe_write
  12025 2570458 ?        00:00:16 ms_pipe_write
  12025 2570459 ?        00:00:16 ms_pipe_write
  12025 2570460 ?        00:03:24 ms_pipe_write
  12025 2570461 ?        00:05:24 ms_pipe_write
  12025 2570462 ?        00:03:45 ms_pipe_write
  12025 2570463 ?        00:03:34 ms_pipe_read
  12025 2570464 ?        00:03:52 ms_pipe_read
  12025 2570465 ?        00:06:45 ms_pipe_read
  12025 2570466 ?        00:00:30 ms_pipe_read
  12025 2570467 ?        00:00:31 ms_pipe_read
  12025 2570468 ?        00:04:03 ms_pipe_read
  12025 2570469 ?        00:00:16 ms_pipe_write
  12025 2570470 ?        00:00:17 ms_pipe_write
  12025 2570471 ?        00:02:05 ms_pipe_write
  12025 2570472 ?        00:00:30 ms_pipe_read
  12025 2570473 ?        00:00:31 ms_pipe_read
  12025 2570474 ?        00:00:31 ms_pipe_read
  12025 2570475 ?        00:00:30 ms_pipe_read
  12025 2570476 ?        00:00:16 ms_pipe_write
  12025 2570477 ?        00:00:17 ms_pipe_write
  12025 2570479 ?        00:00:17 ms_pipe_write
  12025 2570480 ?        00:00:16 ms_pipe_write
  12025 2570481 ?        00:04:04 ms_pipe_read
  12025 2570483 ?        00:02:12 ms_pipe_write
  12025 2570484 ?        00:00:31 ms_pipe_read
  12025 2570485 ?        00:00:30 ms_pipe_read
  12025 2570486 ?        00:00:17 ms_pipe_write
  12025 2570487 ?        00:00:17 ms_pipe_write
  12025 2570491 ?        00:05:22 ms_pipe_write
  12025 2570493 ?        00:00:10 ms_pipe_write
  12025 2570494 ?        00:02:16 ms_pipe_write
  12025 2570495 ?        00:03:15 ms_pipe_write
  12025 2570496 ?        00:02:55 ms_pipe_write
  12025 2570497 ?        00:05:12 ms_pipe_read
  12025 2570499 ?        00:03:29 ms_pipe_read
  12025 2570500 ?        00:04:14 ms_pipe_read
  12025 2570501 ?        00:03:05 ms_pipe_read
  12025 2570503 ?        00:00:14 ms_pipe_read
  12025 2570504 ?        00:00:32 ms_pipe_read
  12025 2570505 ?        00:00:30 ms_pipe_read
  12025 2570507 ?        00:00:16 ms_pipe_write
  12025 2570508 ?        00:00:16 ms_pipe_write
  12025 2570514 ?        00:00:31 ms_pipe_read
  12025 2570515 ?        00:00:30 ms_pipe_read
  12025 2570516 ?        00:00:16 ms_pipe_write
  12025 2570517 ?        00:00:16 ms_pipe_write
  12025 2570520 ?        00:00:31 ms_pipe_read
  12025 2570521 ?        00:00:30 ms_pipe_read
  12025 2570522 ?        00:00:17 ms_pipe_write
  12025 2570523 ?        00:00:16 ms_pipe_write
  12025 2570524 ?        00:00:31 ms_pipe_read
  12025 2570525 ?        00:00:30 ms_pipe_read
  12025 2570526 ?        00:00:17 ms_pipe_write
  12025 2570527 ?        00:00:16 ms_pipe_write
  12025 2570535 ?        00:00:31 ms_pipe_read
  12025 2570536 ?        00:00:30 ms_pipe_read
  12025 2570537 ?        00:00:17 ms_pipe_write
  12025 2570538 ?        00:00:16 ms_pipe_write
  12025 2570539 ?        00:00:30 ms_pipe_read
  12025 2570540 ?        00:00:31 ms_pipe_read
  12025 2570541 ?        00:00:16 ms_pipe_write
  12025 2570542 ?        00:00:16 ms_pipe_write
  12025 2570543 ?        00:00:31 ms_pipe_read
  12025 2570544 ?        00:00:31 ms_pipe_read
  12025 2570545 ?        00:00:17 ms_pipe_write
  12025 2570546 ?        00:00:16 ms_pipe_write
  12025 2570547 ?        00:00:30 ms_pipe_read
  12025 2570548 ?        00:00:31 ms_pipe_read
  12025 2570549 ?        00:00:16 ms_pipe_write
  12025 2570550 ?        00:00:17 ms_pipe_write
  12025 2570551 ?        00:00:30 ms_pipe_read
  12025 2570552 ?        00:00:31 ms_pipe_read
  12025 2570553 ?        00:00:17 ms_pipe_write
  12025 2570554 ?        00:00:16 ms_pipe_write
  12025 2570569 ?        00:00:00 ms_pipe_write
  12025 2570570 ?        00:00:00 ms_pipe_read
  12025 2570574 ?        00:00:00 ms_pipe_read
  12025 2570577 ?        00:00:00 ms_pipe_write
  12025 2570585 ?        00:00:00 ms_pipe_write
  12025 2570589 ?        00:00:00 ms_pipe_read
  12025 2570591 ?        00:00:00 ms_pipe_read
  12025 2570592 ?        00:00:00 ms_pipe_write
  12025 2570603 ?        00:00:31 ms_pipe_read
  12025 2570604 ?        00:00:30 ms_pipe_read
  12025 2570605 ?        00:00:17 ms_pipe_write
  12025 2570606 ?        00:00:17 ms_pipe_write
  12025 2570609 ?        00:05:17 ms_pipe_read
  12025 2570610 ?        00:03:40 ms_pipe_write
  12025 2570616 ?        00:05:06 ms_pipe_read
  12025 2570617 ?        00:04:35 ms_pipe_write
  12025 2570738 ?        00:00:00 ms_pipe_read
  12025 2570739 ?        00:00:00 ms_pipe_write
  12025 2570872 ?        00:00:00 ms_pipe_read
  12025 2570873 ?        00:00:00 ms_pipe_write
  12025 3145848 ?        00:00:13 ms_pipe_read
  12025 3145849 ?        00:00:05 ms_pipe_write
  12025 3245901 ?        00:00:06 ms_pipe_write
  12025 3252563 ?        00:00:10 ms_pipe_read
  12025 3254659 ?        00:00:15 ms_pipe_read
  12025 3254660 ?        00:00:10 ms_pipe_write
  12025 3313166 ?        00:01:23 ms_pipe_read
  12025 3313167 ?        00:00:56 ms_pipe_write
  12025 3313612 ?        00:00:00 ms_pipe_read
  12025 3313613 ?        00:00:00 ms_pipe_write
  12025 3313728 ?        00:00:00 ms_pipe_read
  12025 3313729 ?        00:00:00 ms_pipe_write
  12025 3313797 ?        00:00:00 ms_pipe_read
  12025 3313798 ?        00:00:00 ms_pipe_write
  12025 3313799 ?        00:00:00 ms_pipe_read
  12025 3313800 ?        00:00:00 ms_pipe_write
  12025 3313911 ?        00:00:00 ms_pipe_read
  12025 3313912 ?        00:00:00 ms_pipe_write
  12025 3314037 ?        00:00:00 ms_pipe_read
  12025 3314038 ?        00:00:00 ms_pipe_write
  12025 3314075 ?        00:00:00 ms_pipe_read
  12025 3314076 ?        00:00:00 ms_pipe_write
  12025 3377265 ?        00:00:01 ms_pipe_read
  12025 3377266 ?        00:00:01 ms_pipe_write
  12025 3377624 ?        00:00:00 ms_pipe_read
  12025 3377625 ?        00:00:00 ms_pipe_write
  12025 3377933 ?        00:00:00 ms_pipe_read
  12025 3377934 ?        00:00:00 ms_pipe_write
  12025 3378058 ?        00:00:00 ms_pipe_read
  12025 3378059 ?        00:00:00 ms_pipe_write
  12025 3378498 ?        00:00:00 ms_pipe_read
  12025 3378499 ?        00:00:00 ms_pipe_write
  12025 3378629 ?        00:00:00 ms_pipe_read
  12025 3378630 ?        00:00:00 ms_pipe_write
  12025 3389562 ?        00:00:03 ms_pipe_read
  12025 3389563 ?        00:00:03 ms_pipe_read
  12025 3389564 ?        00:00:03 ms_pipe_write
  12025 3389566 ?        00:00:03 ms_pipe_write
  12025 3389568 ?        00:00:03 ms_pipe_read
  12025 3389569 ?        00:00:03 ms_pipe_read
  12025 3389570 ?        00:00:02 ms_pipe_write
  12025 3389571 ?        00:00:02 ms_pipe_write
  12025 3389575 ?        00:00:03 ms_pipe_read
  12025 3389576 ?        00:00:03 ms_pipe_read
  12025 3389578 ?        00:00:03 ms_pipe_write
  12025 3389580 ?        00:00:03 ms_pipe_write
  12025 3389584 ?        00:00:02 ms_pipe_write
  12025 3389585 ?        00:00:02 ms_pipe_write
  12025 3389586 ?        00:00:02 ms_pipe_write
  12025 3389587 ?        00:00:02 ms_pipe_write
  12025 3389588 ?        00:00:02 ms_pipe_write
  12025 3389589 ?        00:00:02 ms_pipe_write
  12025 3389590 ?        00:00:02 ms_pipe_write
  12025 3389591 ?        00:00:02 ms_pipe_write
  12025 3389592 ?        00:00:02 ms_pipe_write
  12025 3389593 ?        00:00:02 ms_pipe_write
  12025 3389594 ?        00:00:03 ms_pipe_read
  12025 3389595 ?        00:00:03 ms_pipe_read
  12025 3389596 ?        00:00:02 ms_pipe_write
  12025 3389597 ?        00:00:02 ms_pipe_read
  12025 3389598 ?        00:00:02 ms_pipe_read
  12025 3389599 ?        00:00:02 ms_pipe_write
  12025 3389600 ?        00:00:03 ms_pipe_read
  12025 3389601 ?        00:00:02 ms_pipe_write
  12025 3389602 ?        00:00:03 ms_pipe_read
  12025 3389603 ?        00:00:03 ms_pipe_read
  12025 3389604 ?        00:00:03 ms_pipe_read
  12025 3389605 ?        00:00:02 ms_pipe_write
  12025 3389606 ?        00:00:02 ms_pipe_read
  12025 3389607 ?        00:00:02 ms_pipe_write
  12025 3389608 ?        00:00:01 ms_pipe_read
  12025 3389609 ?        00:00:03 ms_pipe_read
  12025 3389610 ?        00:00:02 ms_pipe_write
  12025 3389611 ?        00:00:02 ms_pipe_write
  12025 3389612 ?        00:00:02 ms_pipe_write
  12025 3389613 ?        00:00:02 ms_pipe_read
  12025 3389614 ?        00:00:02 ms_pipe_read
  12025 3389615 ?        00:00:02 ms_pipe_read
  12025 3389616 ?        00:00:02 ms_pipe_read
  12025 3389617 ?        00:00:02 ms_pipe_read
  12025 3389618 ?        00:00:02 ms_pipe_write
  12025 3389619 ?        00:00:02 ms_pipe_write
  12025 3389620 ?        00:00:02 ms_pipe_write
  12025 3389621 ?        00:00:02 ms_pipe_write
  12025 3389622 ?        00:00:02 ms_pipe_write
  12025 3389623 ?        00:00:02 ms_pipe_write
  12025 3389624 ?        00:00:02 ms_pipe_read
  12025 3389625 ?        00:00:02 ms_pipe_read
  12025 3389626 ?        00:00:03 ms_pipe_read
  12025 3389627 ?        00:00:02 ms_pipe_read
  12025 3389628 ?        00:00:02 ms_pipe_read
  12025 3389629 ?        00:00:03 ms_pipe_read
  12025 3389630 ?        00:00:03 ms_pipe_read
  12025 3389631 ?        00:00:03 ms_pipe_read
  12025 3391295 ?        00:00:00 ms_pipe_write
  12025 3393933 ?        00:00:00 ms_pipe_read
  12025 3393934 ?        00:00:00 ms_pipe_write
  12025 3404012 ?        00:00:00 ms_pipe_read
  12025 3404018 ?        00:00:00 ms_pipe_read
  12025 3404019 ?        00:00:00 ms_pipe_write
  12025 3405549 ?        00:00:00 ms_pipe_read
  12025 3405550 ?        00:00:00 ms_pipe_write
  12025 3406486 ?        00:00:19 ms_pipe_read
  12025 3406488 ?        00:00:17 ms_pipe_write
  12025 3406506 ?        00:00:02 ms_pipe_write
  12025 3406507 ?        00:00:02 ms_pipe_write
  12025 3406508 ?        00:00:02 ms_pipe_read
  12025 3406509 ?        00:00:02 ms_pipe_read
  12025 3425139 ?        00:00:00 ms_pipe_read
  12025 3425140 ?        00:00:00 ms_pipe_write
  12025 3444471 ?        00:00:00 ms_pipe_read
  12025 3444472 ?        00:00:00 ms_pipe_write
  12025 3451139 ?        00:00:00 ms_pipe_read
  12025 3451140 ?        00:00:00 ms_pipe_write
  12025 3458441 ?        00:00:00 ms_pipe_read
  12025 3458442 ?        00:00:00 ms_pipe_write
  12025 3462283 ?        00:00:00 ms_pipe_read
  12025 3462284 ?        00:00:00 ms_pipe_write
  12025 3463676 ?        00:00:00 ms_pipe_read
  12025 3463677 ?        00:00:00 ms_pipe_write
  12025 3464147 ?        00:00:00 ms_pipe_read
  12025 3464148 ?        00:00:00 ms_pipe_write
  12025 3464195 ?        00:00:00 ms_pipe_read
  12025 3464196 ?        00:00:00 ms_pipe_write
  12025 3464805 ?        00:00:00 ms_pipe_read
  12025 3464806 ?        00:00:00 ms_pipe_write
  12025 3465173 ?        00:00:00 ms_pipe_read
  12025 3465174 ?        00:00:00 ms_pipe_write
  12025 3465338 ?        00:00:00 ms_pipe_read
  12025 3465339 ?        00:00:00 ms_pipe_write
  12025 3465699 ?        00:00:00 ms_pipe_read
  12025 3465700 ?        00:00:00 ms_pipe_write
  12025 3466303 ?        00:00:00 ms_pipe_read
  12025 3466304 ?        00:00:00 ms_pipe_write
  12025 3466658 ?        00:00:00 ms_pipe_read
  12025 3466659 ?        00:00:00 ms_pipe_write
  12025 3466900 ?        00:00:00 ms_pipe_read
  12025 3466901 ?        00:00:00 ms_pipe_write
  12025 3467135 ?        00:00:00 ms_pipe_read
  12025 3467136 ?        00:00:00 ms_pipe_write
  12025 3467349 ?        00:00:00 ms_pipe_read
  12025 3467350 ?        00:00:00 ms_pipe_write
  12025 3467741 ?        00:00:00 ms_pipe_read
  12025 3467742 ?        00:00:00 ms_pipe_write
  12025 3467910 ?        00:00:00 ms_pipe_read
  12025 3467911 ?        00:00:00 ms_pipe_write
  12025 3468411 ?        00:00:00 ms_pipe_read
  12025 3468412 ?        00:00:00 ms_pipe_write
  12025 3468989 ?        00:00:00 ms_pipe_read
  12025 3468990 ?        00:00:00 ms_pipe_write
  12025 3469082 ?        00:00:00 ms_pipe_read
  12025 3469083 ?        00:00:00 ms_pipe_write
  12025 3469340 ?        00:00:00 ms_pipe_read
  12025 3469341 ?        00:00:00 ms_pipe_write
  12025 3469535 ?        00:00:00 ms_pipe_read
  12025 3469536 ?        00:00:00 ms_pipe_write
  12025 3469609 ?        00:00:00 ms_pipe_read
  12025 3469610 ?        00:00:00 ms_pipe_write
  12025 3469636 ?        00:00:00 ms_pipe_read
  12025 3469637 ?        00:00:00 ms_pipe_write
  12025 3469664 ?        00:00:00 ms_pipe_read
  12025 3469665 ?        00:00:00 ms_pipe_write
  12025 3469893 ?        00:00:00 ms_pipe_read
  12025 3469894 ?        00:00:00 ms_pipe_write
  12025 3469895 ?        00:00:00 ms_pipe_read
  12025 3469896 ?        00:00:00 ms_pipe_write
  12025 3470564 ?        00:00:00 ms_pipe_read
  12025 3470565 ?        00:00:00 ms_pipe_write
  12025 3470848 ?        00:00:00 ms_pipe_read
  12025 3470849 ?        00:00:00 ms_pipe_write
  12025 3470865 ?        00:00:00 ms_pipe_read
  12025 3470866 ?        00:00:00 ms_pipe_write
  12025 3470884 ?        00:00:00 ms_pipe_read
  12025 3470885 ?        00:00:00 ms_pipe_write
  12025 3471331 ?        00:00:00 ms_pipe_read
  12025 3471332 ?        00:00:00 ms_pipe_write
  12025 3471456 ?        00:00:00 ms_pipe_read
  12025 3471457 ?        00:00:00 ms_pipe_write
  12025 3472409 ?        00:00:00 ms_pipe_read
  12025 3472410 ?        00:00:00 ms_pipe_write
  12025 3472415 ?        00:00:00 ms_pipe_read
  12025 3472416 ?        00:00:00 ms_pipe_write
  12025 3472743 ?        00:00:00 ms_pipe_read
  12025 3472744 ?        00:00:00 ms_pipe_write
  12025 3472908 ?        00:00:00 ms_pipe_read
  12025 3472909 ?        00:00:00 ms_pipe_write
  12025 3473128 ?        00:00:00 ms_pipe_read
  12025 3473129 ?        00:00:00 ms_pipe_write
  12025 3473252 ?        00:00:00 ms_pipe_read
  12025 3473253 ?        00:00:00 ms_pipe_write
  12025 3473800 ?        00:00:00 ms_pipe_read
  12025 3473801 ?        00:00:00 ms_pipe_write
  12025 3473945 ?        00:00:00 ms_pipe_read
  12025 3473946 ?        00:00:00 ms_pipe_write
  12025 3474118 ?        00:00:00 ms_pipe_read
  12025 3474119 ?        00:00:00 ms_pipe_write
  12025 3474267 ?        00:00:00 ms_pipe_read
  12025 3474268 ?        00:00:00 ms_pipe_write
  12025 3474418 ?        00:00:00 ms_pipe_read
  12025 3474419 ?        00:00:00 ms_pipe_write
  12025 3474429 ?        00:00:00 ms_pipe_read
  12025 3474430 ?        00:00:00 ms_pipe_write
  12025 3474448 ?        00:00:00 ms_pipe_read
  12025 3474449 ?        00:00:00 ms_pipe_write
  12025 3474722 ?        00:00:00 ms_pipe_read
  12025 3474723 ?        00:00:00 ms_pipe_write
  12025 3474797 ?        00:00:00 ms_pipe_read
  12025 3474798 ?        00:00:00 ms_pipe_write
  12025 3474934 ?        00:00:00 ms_pipe_read
  12025 3474938 ?        00:00:00 ms_pipe_write
  12025 3474975 ?        00:00:00 ms_pipe_read
  12025 3474976 ?        00:00:00 ms_pipe_write
  12025 3475220 ?        00:00:00 ms_pipe_read
  12025 3475221 ?        00:00:00 ms_pipe_write
  12025 3475261 ?        00:00:00 ms_pipe_read
  12025 3475262 ?        00:00:00 ms_pipe_write
  12025 3475306 ?        00:00:00 ms_pipe_read
  12025 3475307 ?        00:00:00 ms_pipe_write
  12025 3475310 ?        00:00:00 ms_pipe_read
  12025 3475311 ?        00:00:00 ms_pipe_write
  12025 3475434 ?        00:00:00 ms_pipe_read
  12025 3475435 ?        00:00:00 ms_pipe_write
  12025 3475470 ?        00:00:00 ms_pipe_read
  12025 3475471 ?        00:00:00 ms_pipe_write
</pre>
上面总共有418个线程，我们可以使用```strace```跟踪其中一个线程的运行，例如,我们可以跟踪```14234```这个ms_accepter线程接收客户端链接：
<pre>
# strace -p 32726 -e trace=accept
</pre>

## 2. 相关数据结构
下面将介绍OSD模块相关的一些核心的数据结构。从最高的逻辑层次为pool的概念，然后是PG的概念。其次是OSDMap记录了集群的所有的配置信息。数据结构OSDOp是一个操作上下文的封装。结构object_info_t保存了一个对象的元数据信息和访问信息。对象ObjectState是在object_info_t的基础上添加了一些内存的状态信息。SnapSetContext和ObjectContext分别保存了快照和对象的上下文相关的信息。Session保存了一个端到端的链接相关的上下文信息。

### 2.1 Pool
Pool是整个集群层面定义的一个逻辑的存储池。对一个Pool可以设置相应的数据冗余类型，目前有副本和纠删码两种实现。数据结构pg_pool_t用于保存Pool的相关信息。Pool的数据结构如下(src/osd/osd_types.h)：
{% highlight string %}
struct pg_pool_t {
	enum {
		TYPE_REPLICATED = 1,     //副本
		//TYPE_RAID4 = 2,        //从来没有实现的raid4
		TYPE_ERASURE = 3,        //纠删码
	};
	
	uint64_t flags;              //pool的相关的标志，见FLAG_*
	__u8 type;                   //类型
	__u8 size, min_size;         //pool的size和min_size，也就是副本数和至少保证的副本数（一个PG中的OSD个数)。
	__u8 crush_ruleset;          //rule set的编号
	__u8 object_hash;            //将对象名映射为ps的hash函数
	
private:
  __u32 pg_num, pgp_num;         //PG、PGP的数量
  
public:
  map<string,string> properties;  //过时，当前已经不再使用
  string erasure_code_profile;    //EC的配置信息
  
  ...
  uint64_t quota_max_bytes;      //pool最大的存储字节数
  uint64_t quota_max_objects;    //pool最大的object个数
  ....
};
{% endhighlight %}

* type: 定义了Pool的类型，目前有replication和ErasureCode两种类型；

* size和min_size定义了Pool的冗余模式

&emsp; - 如果是replication模式，size定义了副本数目，min_size为副本的最小数目。例如：如果size设置为3，副本数为3，min_size设置为1就只允许两副本损坏。

&emsp; - 如果是Erasure Code(M+N)，size是总的分片数M+N； min_size是实际数据的分片数M。

* crush_ruleset: Pool对应的crush规则号

* erasure_code_profile: EC的配置方式

* object_hash: 通过对象名映射到PG的hash函数

* pg_num: Pool里的PG的数量

通过上面的介绍可以了解到，Pool根据类型不同，定义了两种模式，分别保存了两种模式相关的参数。此外，在结构pg_pool_t里还定义了Pool级别的快照相关的数据结构、Cache Tier相关的数据结构，以及其他一些统计信息。在介绍快照(参见第9章）和Cache Tier（参见第13章）时再详细介绍相关的字段。

### 2.2 PG
PG可以认为是一组对象的集合，该集合里的对象有共同的特征： 副本都分布在相同的OSD列表中。PG的数据结构如下（src/osd/osd_types.h)：
{% highlight string %}
struct pg_t {
  uint64_t m_pool;              //pg所在的pool
  uint32_t m_seed;              //pg的序号
  int32_t m_preferred;          //pg优先选择的主OSD
};
{% endhighlight %}

结构体pg_t只是一个PG的静态描述信息。类PG及其子类ReplicatedPG都是和PG相关的处理。
{% highlight string %}
struct spg_t {
  pg_t pgid;
  shard_id_t shard;
};
{% endhighlight %}
数据结构spt_t在pg_t的基础上，增加了一个shard_id字段，代表了该PG所在的OSD在对应的OSD列表中的序号。例如，现在有一个PG 2.f，其映射到的OSD为(2,5,8)，那么在OSD5上该PG对应的shard值就为1。

在Erasure Code模式下，该字段保存了每个分片的序号。该序号在EC的数据encode和decode过程中很关键；对于副本模式，该字段没有意义，都设置为shard_id_t::NO_SHARD值。

###### PG的分裂

当一个pool里的PG数量不够时，系统允许通过增加PG的数量，就会产生PG的分裂，使得一个PG分裂为2的幂次方个PG。PG分裂后，新的PG和其父PG的OSD列表是一致的，其数据的移动也是本地数据的移动，开销比较小。


### 2.3 OSDMap
类OSDMap定义了Ceph整个集群的全局信息。它由Monitor实现管理，并以全量或者增量的方式向整个集群扩散。每一个epoch对应的OSDMap都需要持久化保存在meta下对应对象的omap属性中。

下面介绍OSDMap核心成员，内部类Incremental以增量的形式保存了OSDMap新增的信息，其内部成员和OSDMap类似，这里就不介绍了。代码位于src/osd/OSDMap.h:
{% highlight string %}
class OSDMap{
public:
	class Incremental{
	};

private:
  //系统相关信息
  uuid_d fsid;                   //当前集群的fsid
  epoch_t epoch;                 //当前集群的epoch值
  utime_t created, modified;     //创建、修改的时间戳
  int32_t pool_max;              //最大的pool数量

  uint32_t flags;                //一些标志信息


  //OSD相关的信息
  int num_osd;                  //OSD总数量（并不会持久化保存，请参看calc_num_osds)
  int num_up_osd;               //处于up状态的OSD数量（并不会持久化保存，请参看calc_num_osds)
  int num_in_osd;               //处于in状态的OSD数量（并不会持久化保存，请参看calc_num_osds)

  int32_t max_osd;              //OSD的最大数目
  vector<uint8_t> osd_state;    //OSD的状态
  struct addrs_s {
    vector<ceph::shared_ptr<entity_addr_t> > client_addr;
    vector<ceph::shared_ptr<entity_addr_t> > cluster_addr;
    vector<ceph::shared_ptr<entity_addr_t> > hb_back_addr;
    vector<ceph::shared_ptr<entity_addr_t> > hb_front_addr;
    entity_addr_t blank;
  };
  ceph::shared_ptr<addrs_s> osd_addrs;   //OSD的地址（从上面我们看到，一个OSD可以有多个不同类型的地址
  vector<__u32>   osd_weight;           //OSD权重（16.16固定浮点数，即16bit整数部分，16bit小数部分）
  vector<osd_info_t> osd_info;          //OSD的基本信息
  ceph::shared_ptr< vector<uuid_d> > osd_uuid;   //OSD对应的UUID
  vector<osd_xinfo_t> osd_xinfo;                 //OSD的一些扩展信息


  //PG相关的信息
  ceph::shared_ptr< map<pg_t,vector<int32_t> > > pg_temp  // temp pg mapping (e.g. while we rebuild)
  ceph::shared_ptr< map<pg_t,int32_t > > primary_temp;    // temp primary mapping (e.g. while we rebuild)
  ceph::shared_ptr< vector<__u32> > osd_primary_affinity; // < 16.16 fixed point, 0x10000 = baseline


 //pool相关的信息map<int64_t,pg_pool_t> pools;             //pool的id到类pg_pool_t的映射
  map<int64_t,string> pool_name;                          //pool的id到pool的名字的映射
  map<string,map<string,string> > erasure_code_profiles;  //pool的EC相关的信息
  map<string,int64_t> name_pool;                          //pool的名字到pool的id的映射


  //Crush相关的信息
  ceph::shared_ptr<CrushWrapper> crush;                   //CRUSH算法
 
  ....
};
{% endhighlight %}
通过OSDMap数据成员的了解，可以看到，OSDMap包含了四类信息：首先是集群的信息；其次是pool相关的信息；然后是临时PG相关的信息；最后就是所有OSD的状态信息。


### 2.4 OSDOp
类MOSDOp封装了一些基本操作先关的数据(src/messages/MOSDOp.h)：
{% highlight string %}
class MOSDOp : public Message {

  static const int HEAD_VERSION = 7;
  static const int COMPAT_VERSION = 3;

private:
  uint32_t client_inc;
  __u32 osdmap_epoch;             //OSDMap的epoch值
  __u32 flags;
  utime_t mtime;
  eversion_t reassert_version;
  int32_t retry_attempt;          // 0 is first attempt.  -1 if we don't know.

  object_t oid;                   //操作的对象
  object_locator_t oloc;          //对象的位置信息
  pg_t pgid;                      //对象所在的PG的id
  bufferlist::iterator p;
 

  atomic<bool> partial_decode_needed;
  atomic<bool> final_decode_needed;
  //
public:
  vector<OSDOp> ops;              //针对oid的多个操作集合


private:
  //快照相关
  snapid_t snapid;                //snapid，如果是CEPH_NOSNAP，就是head对象；否则就是等于snap_seq
  snapid_t snap_seq;              //如果是head对象，就是最新的快照序号；如果是snap对象，就是snap对应的seq
  vector<snapid_t> snaps;         //所有的snap列表

  uint64_t features;              //一些feature的标志

  osd_reqid_t reqid;             // reqid explicitly set by sender

public:
  friend class MOSDOpReply;
};
{% endhighlight %}
MOSDOp在其成员ops向量里封装了多个类型为OSDOp操作数据。MOSDOp封装的操作都是关于对象oid相关的操作，一个MOSDOp只封装针对同一个对象oid的操作。但是对于rados_clone_range这样的操作，需要有一个目标对象oid，还有一个源对象oid，那么源对象的oid就保存在结构OSDOp里。


数据结构OSDOp封装了一个OSD操作需要的数据和元数据(src/osd/osd_types.h)：
{% highlight string %}
struct OSDOp {
  ceph_osd_op op;                 //具体操作数据的封装
  sobject_t soid;                 //src oid，并不是op操作的对象，而是源操作对象。例如rados_clone_range需要目标obj和源obj

  bufferlist indata, outdata;     //操作的输入输出的data
  int32_t rval;                   //操作返回值

  OSDOp() : rval(0) {
    memset(&op, 0, sizeof(ceph_osd_op));
  }
};
{% endhighlight %}

### 2.5 object_info_t
结构object_info_t保存了一个对象的元数据信息和访问信息(src/osd/osd_types.h)。其作为对象的一个属性，持久化保存在对象xattr中，对应的key为OI_ATTR(" _")，value就是object_info_t的encode后的数据。
{% highlight string %}
struct object_info_t {
  hobject_t soid;                      //对应的对象
  eversion_t version, prior_version;   //对象的当前版本，前一个版本
  version_t user_version;              //用户操作的版本
  osd_reqid_t last_reqid;              //最后请求的reqid

  uint64_t size;                       //对象的大小
  utime_t mtime;                       //修改时间
  utime_t local_mtime;                 //修改的本地时间

  // note: these are currently encoded into a total 16 bits; see
  // encode()/decode() for the weirdness.
  typedef enum {
    FLAG_LOST     = 1<<0,
    FLAG_WHITEOUT = 1<<1,       // object logically does not exist
    FLAG_DIRTY    = 1<<2,       // object has been modified since last flushed or undirtied
    FLAG_OMAP     = 1 << 3,     // has (or may have) some/any omap data
    FLAG_DATA_DIGEST = 1 << 4,  // has data crc
    FLAG_OMAP_DIGEST = 1 << 5,  // has omap crc
    FLAG_CACHE_PIN = 1 << 6,    // pin the object in cache tier
    // ...
    FLAG_USES_TMAP = 1<<8,       // deprecated; no longer used.
  } flag_t;

  flag_t flags;                  //对象的一些标记

  .... 
  
  vector<snapid_t> snaps;                //clone对象的快照信息

  uint64_t truncate_seq, truncate_size;  //truncate操作的序号和size

  //watchers记录了客户端监控信息，一旦对象的状态发生变化，需要通知客户端
  map<pair<uint64_t, entity_name_t>, watch_info_t> watchers;   

  // opportunistic checksums; may or may not be present
  __u32 data_digest;                    //data crc32c
  __u32 omap_digest;                    //omap crc32c
};
{% endhighlight %}


### 2.6 ObjectState
对象ObjectState是在object_info_t的基础上添加了一个字段exists，用来标记对象是否存在(src/osd/osd_types.h)：
{% highlight string %}
struct ObjectState {
  object_info_t oi;      //对象元数据信息
  bool exists;           // the stored object exists (i.e., we will remember the object_info_t)

  ObjectState() : exists(false) {}

  ObjectState(const object_info_t &oi_, bool exists_)
    : oi(oi_), exists(exists_) {}
};
{% endhighlight %}
为什么要加一个额外的bool变量来标记呢？因为object_info_t可能是从缓存的attrs[OI_ATTR]中获取的，并不能确定对象是否存在。

### 2.7 SnapSetContext
SnapSetContext保存了快照的相关信息，即SnapSet的上下文信息。关于SnapSet的内容，可以参考快照相关的介绍()：
{% highlight string %}
struct SnapSetContext {
  hobject_t oid;               //对象
  SnapSet snapset;             //SnapSet, 对象快照相关的记录
  int ref;                     //本结构的引用计数
  bool registered : 1;         //是否在SnapSet Cache中记录
  bool exists : 1;             //snapset是否存在

  explicit SnapSetContext(const hobject_t& o) :
    oid(o), ref(0), registered(false), exists(true) { }
};
{% endhighlight %}

### 2.8 ObjectContext
ObjectContext可以说是对象在内存中的一个管理类，保存了一个对象的上下文信息：
{% highlight string %}
struct ObjectContext {
  ObjectState obs;           //主要是object_info_t，描述了对象的状态信息

  SnapSetContext *ssc;       //快照上下文信息，如果没有快照就为NULL

  Context *destructor_callback;   //析构时的回调

private:
  Mutex lock;
public:
  Cond cond;

  //正在写操作的数目，正在读操作的数目
  //等待写操作的数目，等待读操作的数目
  int unstable_writes, readers, writers_waiting, readers_waiting;


  //如果该对象的写操作被阻塞去恢复另一个对象，设置这个属性
  ObjectContextRef blocked_by;      //本对象被某一个对象阻塞
  set<ObjectContextRef> blocking;   //本对象阻塞的对象集合


  //任何在obs.io.watchers的watchers，其要么在watchers队列中，要么在unconnected_watchers队列中
  map<pair<uint64_t, entity_name_t>, WatchRef> watchers;

  // attr cache
  map<string, bufferlist> attr_cache;      //属性的缓存


  struct RWState {
    enum State {
      RWNONE,
      RWREAD,
      RWWRITE,
      RWEXCL,
    };

    list<OpRequestRef> waiters;           //等待状态变化的waiters
    int count;                            //读或写的数目

    State state:4;                        //读写的状态
    
    bool recovery_read_marker:1;          //如果设置，获得锁后，重新执行backfill操作

    bool snaptrimmer_write_marker:1;      //如果设置，获得锁后重新加入snaptrim队列中
  }

  .....
};
{% endhighlight %}
下面两个字段比较难理解，进行一些补充说明：

* blocked_by记录了当前对象被其他对象阻塞，blocking记录了本对象阻塞其他对象的情况。当一个对象的写操作依赖其他对象时，就会出现这些情况。这一般对应一个操作涉及多个对象，比如copy操作。吧对象obj1上的部分数据拷贝到对象obj2，如果源对象obj1处于missing状态，需要恢复，那么obj2对象就block了obj1对象。

* 内部类RWState通过定义了4种状态，实现了对对象的读写加锁。

### 2.9 Session
类Session是和Connection相关的一个类，用于保存Connection上下文相关的信息(src/osd/osd.h)：
{% highlight string %}
struct Session : public RefCountedObject {
	EntityName entity_name;              //peer实例的名称
	OSDCap caps;                         //
	int64_t auid;
	ConnectionRef con;                   //相关的Connection
	WatchConState wstate;

	Mutex session_dispatch_lock;
	list<OpRequestRef> waiting_on_map;   //所有的OpRequest请求都先添加到这个队列里

	OSDMapRef osdmap;                    //waiting_for_pg当前所对应的OSDMap
	map<spg_t, list<OpRequestRef> > waiting_for_pg;   //当前需要更新OSDMap的pg和对应的请求

	Spinlock sent_epoch_lock;
	epoch_t last_sent_epoch;
	Spinlock received_map_lock;
	epoch_t received_map_epoch;          //largest epoch seen in MOSDMap from here

explicit Session(CephContext *cct) :
  RefCountedObject(cct),
  auid(-1), con(0),
  session_dispatch_lock("Session::session_dispatch_lock"), 
  last_sent_epoch(0), received_map_epoch(0)
{}
}；
{% endhighlight %}
函数update_waiting_for_pg()用于检查是否有最新的osdmap:

1) 如果该PG有分裂的PG，就把分裂出的新的PG以及对应的OpRequest加入到Session的waiting_for_pg队列里；

2） 如果该PG不分裂，就不把PG和OpRequest加入到waiting_for_pg队列里。


### 2.10 ShardedOpWQ
OSD在进行数据读写等操作时，通常要求一个PG内的事务具有严格的顺序性，而不同PG间事务是可以并发执行的，ShardedOpWQ用于实现此一功能(src/osd/OSD.h):
{% highlight string %}
class ShardedOpWQ: public ShardedThreadPool::ShardedWQ < pair <PGRef, PGQueueable> > {
	struct ShardData {
		Mutex sdata_lock;
		Cond sdata_cond;
		Mutex sdata_op_ordering_lock;
		map<PG*, list<PGQueueable> > pg_for_processing;
		std::unique_ptr<OpQueue< pair<PGRef, PGQueueable>, entity_inst_t>> pqueue;      //PG对应的队列
	};

	vector<ShardData*> shard_list;                //PGs要shard到的队列向量
	OSD *osd;
	uint32_t num_shards;                          //shard_list的长度

};
{% endhighlight %}

这样我们一个shardData一个线程，确保了单个PG的顺序性(当然，一个shardData多个线程的话，我们可以通过ShardData::lock来保证顺序性）。




<br />
<br />

**[参看]**





<br />
<br />
<br />

