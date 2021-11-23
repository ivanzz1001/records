---
layout: post
title: Linux pmap命令的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux pmap命令的使用
---


本章我们介绍一下pmap命令的使用，在此做一个记录，以便后续查阅。


<!-- more -->

## 1. pmap命令
Linux pmap命令可用于查看一个或多个进程的内存映射。pmap会显式进程的地址空间信息(address space)或内存映射信息(memory usage map)。

pmap实际上是Sun OS上的一个命令，在Linux系统上实现了其部分feature。pmap在查看一个进程的```完整地址空间```(complete address of a process)时具有十分重要的作用。要查看进程的内存使用情况([memory usage of process](https://www.linoxide.com/linux-shell-script/linux-memory-usage-program/))，我们首先需要获得该进程的pid，这可以通过```ps```命令或者```/proc```文件系统来获得。

在前面的章节中，我们曾介绍过使用```ps```命令或```top```名来查看进程的内存状况：

* [process memory check using ps](https://linoxide.com/ps-command-memory-use/)

* [process memory check using top command](https://linoxide.com/top-command-memory/)

如下我们介绍使用```pmap```命令来查看进程的内存使用状况。

### 1.1 Usage syntax

pmap命令的使用语法如下：
<pre>
# pmap PID

or 

# pmap [options] PID
</pre>

在输出中，其会显式total address, kbytes, mode以及mapping。

其中所支持的选项(options)有：

* -x extended Show the extended format.

* -d device Show the device format.

* -q quiet Do not display some header/footer lines.

* -V show version Displays version of program.


### 1.2 使用示例
1）**Memory usage map of single process**

我们可以通过pmap后接pid的方式来查看单个进程的内存映射，如下：
{% highlight string %}
# pmap 1345
1345:   /usr/sbin/sshd -D
000055a8a137b000    800K r-x-- sshd
000055a8a1642000     16K r---- sshd
000055a8a1646000      4K rw--- sshd
000055a8a1647000     36K rw---   [ anon ]
000055a8a2bbf000    132K rw---   [ anon ]
00007fe59d566000     48K r-x-- libnss_files-2.17.so
00007fe59d572000   2044K ----- libnss_files-2.17.so
00007fe59d771000      4K r---- libnss_files-2.17.so
00007fe59d772000      4K rw--- libnss_files-2.17.so
00007fe59d773000     24K rw---   [ anon ]
00007fe59d779000     60K r-x-- libbz2.so.1.0.6
00007fe59d788000   2044K ----- libbz2.so.1.0.6
00007fe59d987000      4K r---- libbz2.so.1.0.6
00007fe59d988000      4K rw--- libbz2.so.1.0.6
00007fe59d989000     92K r-x-- libelf-0.176.so
00007fe59d9a0000   2044K ----- libelf-0.176.so
00007fe59db9f000      4K r---- libelf-0.176.so
00007fe59dba0000      4K rw--- libelf-0.176.so
00007fe59dba1000     16K r-x-- libattr.so.1.1.0
00007fe59dba5000   2044K ----- libattr.so.1.1.0
00007fe59dda4000      4K r---- libattr.so.1.1.0
00007fe59dda5000      4K rw--- libattr.so.1.1.0
00007fe59dda6000     12K r-x-- libkeyutils.so.1.5
00007fe59dda9000   2044K ----- libkeyutils.so.1.5
00007fe59dfa8000      4K r---- libkeyutils.so.1.5
00007fe59dfa9000      4K rw--- libkeyutils.so.1.5
00007fe59dfaa000     56K r-x-- libkrb5support.so.0.1
00007fe59dfb8000   2048K ----- libkrb5support.so.0.1
00007fe59e1b8000      4K r---- libkrb5support.so.0.1
00007fe59e1b9000      4K rw--- libkrb5support.so.0.1
00007fe59e1ba000      8K r-x-- libfreebl3.so
00007fe59e1bc000   2044K ----- libfreebl3.so
00007fe59e3bb000      4K r---- libfreebl3.so
00007fe59e3bc000      4K rw--- libfreebl3.so
00007fe59e3bd000    232K r-x-- libnspr4.so
00007fe59e3f7000   2044K ----- libnspr4.so
00007fe59e5f6000      4K r---- libnspr4.so
00007fe59e5f7000      8K rw--- libnspr4.so
00007fe59e5f9000      8K rw---   [ anon ]
00007fe59e5fb000     16K r-x-- libplc4.so
00007fe59e5ff000   2044K ----- libplc4.so
00007fe59e7fe000      4K r---- libplc4.so
00007fe59e7ff000      4K rw--- libplc4.so
00007fe59e800000     12K r-x-- libplds4.so
00007fe59e803000   2044K ----- libplds4.so
00007fe59ea02000      4K r---- libplds4.so
00007fe59ea03000      4K rw--- libplds4.so
00007fe59ea04000    164K r-x-- libnssutil3.so
00007fe59ea2d000   2044K ----- libnssutil3.so
00007fe59ec2c000     28K r---- libnssutil3.so
00007fe59ec33000      4K rw--- libnssutil3.so
00007fe59ec34000   1176K r-x-- libnss3.so
00007fe59ed5a000   2048K ----- libnss3.so
00007fe59ef5a000     20K r---- libnss3.so
00007fe59ef5f000      8K rw--- libnss3.so
00007fe59ef61000      8K rw---   [ anon ]
00007fe59ef63000    148K r-x-- libsmime3.so
00007fe59ef88000   2044K ----- libsmime3.so
00007fe59f187000     12K r---- libsmime3.so
00007fe59f18a000      4K rw--- libsmime3.so
00007fe59f18b000    332K r-x-- libssl3.so
00007fe59f1de000   2048K ----- libssl3.so
00007fe59f3de000     16K r---- libssl3.so
00007fe59f3e2000      4K rw--- libssl3.so
00007fe59f3e3000      4K rw---   [ anon ]
00007fe59f3e4000    412K r-x-- libssl.so.1.0.2k
00007fe59f44b000   2048K ----- libssl.so.1.0.2k
00007fe59f64b000     16K r---- libssl.so.1.0.2k
00007fe59f64f000     28K rw--- libssl.so.1.0.2k
00007fe59f656000    112K r-x-- libsasl2.so.3.0.0
00007fe59f672000   2044K ----- libsasl2.so.3.0.0
00007fe59f871000      4K r---- libsasl2.so.3.0.0
00007fe59f872000      4K rw--- libsasl2.so.3.0.0
00007fe59f873000     92K r-x-- libpthread-2.17.so
00007fe59f88a000   2044K ----- libpthread-2.17.so
00007fe59fa89000      4K r---- libpthread-2.17.so
00007fe59fa8a000      4K rw--- libpthread-2.17.so
00007fe59fa8b000     16K rw---   [ anon ]
00007fe59fa8f000     84K r-x-- libgcc_s-4.8.5-20150702.so.1
00007fe59faa4000   2044K ----- libgcc_s-4.8.5-20150702.so.1
00007fe59fca3000      4K r---- libgcc_s-4.8.5-20150702.so.1
00007fe59fca4000      4K rw--- libgcc_s-4.8.5-20150702.so.1
00007fe59fca5000    312K r-x-- libdw-0.176.so
00007fe59fcf3000   2048K ----- libdw-0.176.so
00007fe59fef3000      8K r---- libdw-0.176.so
00007fe59fef5000      4K rw--- libdw-0.176.so
00007fe59fef6000     16K r-x-- libgpg-error.so.0.10.0
00007fe59fefa000   2044K ----- libgpg-error.so.0.10.0
00007fe5a00f9000      4K r---- libgpg-error.so.0.10.0
00007fe5a00fa000      4K rw--- libgpg-error.so.0.10.0
00007fe5a00fb000    500K r-x-- libgcrypt.so.11.8.2
00007fe5a0178000   2044K ----- libgcrypt.so.11.8.2
00007fe5a0377000      4K r---- libgcrypt.so.11.8.2
00007fe5a0378000     12K rw--- libgcrypt.so.11.8.2
00007fe5a037b000      4K rw---   [ anon ]
00007fe5a037c000     80K r-x-- liblz4.so.1.7.5
00007fe5a0390000   2044K ----- liblz4.so.1.7.5
00007fe5a058f000      4K r---- liblz4.so.1.7.5
00007fe5a0590000      4K rw--- liblz4.so.1.7.5
00007fe5a0591000    148K r-x-- liblzma.so.5.2.2
00007fe5a05b6000   2044K ----- liblzma.so.5.2.2
00007fe5a07b5000      4K r---- liblzma.so.5.2.2
00007fe5a07b6000      4K rw--- liblzma.so.5.2.2
00007fe5a07b7000     28K r-x-- librt-2.17.so
00007fe5a07be000   2044K ----- librt-2.17.so
00007fe5a09bd000      4K r---- librt-2.17.so
00007fe5a09be000      4K rw--- librt-2.17.so
00007fe5a09bf000   1028K r-x-- libm-2.17.so
00007fe5a0ac0000   2044K ----- libm-2.17.so
00007fe5a0cbf000      4K r---- libm-2.17.so
00007fe5a0cc0000      4K rw--- libm-2.17.so
00007fe5a0cc1000     16K r-x-- libcap.so.2.22
00007fe5a0cc5000   2044K ----- libcap.so.2.22
00007fe5a0ec4000      4K r---- libcap.so.2.22
00007fe5a0ec5000      4K rw--- libcap.so.2.22
00007fe5a0ec6000    384K r-x-- libpcre.so.1.2.0
00007fe5a0f26000   2048K ----- libpcre.so.1.2.0
00007fe5a1126000      4K r---- libpcre.so.1.2.0
00007fe5a1127000      4K rw--- libpcre.so.1.2.0
00007fe5a1128000     16K r-x-- libcap-ng.so.0.0.0
00007fe5a112c000   2048K ----- libcap-ng.so.0.0.0
00007fe5a132c000      4K r---- libcap-ng.so.0.0.0
00007fe5a132d000      4K rw--- libcap-ng.so.0.0.0
00007fe5a132e000     92K r-x-- libnsl-2.17.so
00007fe5a1345000   2044K ----- libnsl-2.17.so
00007fe5a1544000      4K r---- libnsl-2.17.so
00007fe5a1545000      4K rw--- libnsl-2.17.so
00007fe5a1546000      8K rw---   [ anon ]
00007fe5a1548000   1804K r-x-- libc-2.17.so
00007fe5a170b000   2048K ----- libc-2.17.so
00007fe5a190b000     16K r---- libc-2.17.so
00007fe5a190f000      8K rw--- libc-2.17.so
00007fe5a1911000     20K rw---   [ anon ]
00007fe5a1916000     12K r-x-- libcom_err.so.2.1
00007fe5a1919000   2044K ----- libcom_err.so.2.1
00007fe5a1b18000      4K r---- libcom_err.so.2.1
00007fe5a1b19000      4K rw--- libcom_err.so.2.1
00007fe5a1b1a000    196K r-x-- libk5crypto.so.3.1
00007fe5a1b4b000   2044K ----- libk5crypto.so.3.1
00007fe5a1d4a000      8K r---- libk5crypto.so.3.1
00007fe5a1d4c000      4K rw--- libk5crypto.so.3.1
00007fe5a1d4d000    868K r-x-- libkrb5.so.3.3
00007fe5a1e26000   2044K ----- libkrb5.so.3.3
00007fe5a2025000     56K r---- libkrb5.so.3.3
00007fe5a2033000     12K rw--- libkrb5.so.3.3
00007fe5a2036000    296K r-x-- libgssapi_krb5.so.2.2
00007fe5a2080000   2048K ----- libgssapi_krb5.so.2.2
00007fe5a2280000      4K r---- libgssapi_krb5.so.2.2
00007fe5a2281000      8K rw--- libgssapi_krb5.so.2.2
00007fe5a2283000     88K r-x-- libresolv-2.17.so
00007fe5a2299000   2048K ----- libresolv-2.17.so
00007fe5a2499000      4K r---- libresolv-2.17.so
00007fe5a249a000      4K rw--- libresolv-2.17.so
00007fe5a249b000      8K rw---   [ anon ]
00007fe5a249d000     32K r-x-- libcrypt-2.17.so
00007fe5a24a5000   2044K ----- libcrypt-2.17.so
00007fe5a26a4000      4K r---- libcrypt-2.17.so
00007fe5a26a5000      4K rw--- libcrypt-2.17.so
00007fe5a26a6000    184K rw---   [ anon ]
00007fe5a26d4000     84K r-x-- libz.so.1.2.7
00007fe5a26e9000   2044K ----- libz.so.1.2.7
00007fe5a28e8000      4K r---- libz.so.1.2.7
00007fe5a28e9000      4K rw--- libz.so.1.2.7
00007fe5a28ea000      8K r-x-- libutil-2.17.so
00007fe5a28ec000   2044K ----- libutil-2.17.so
00007fe5a2aeb000      4K r---- libutil-2.17.so
00007fe5a2aec000      4K rw--- libutil-2.17.so
00007fe5a2aed000     56K r-x-- liblber-2.4.so.2.10.7
00007fe5a2afb000   2044K ----- liblber-2.4.so.2.10.7
00007fe5a2cfa000      4K r---- liblber-2.4.so.2.10.7
00007fe5a2cfb000      4K rw--- liblber-2.4.so.2.10.7
00007fe5a2cfc000    328K r-x-- libldap-2.4.so.2.10.7
00007fe5a2d4e000   2048K ----- libldap-2.4.so.2.10.7
00007fe5a2f4e000      8K r---- libldap-2.4.so.2.10.7
00007fe5a2f50000      4K rw--- libldap-2.4.so.2.10.7
00007fe5a2f51000      8K r-x-- libdl-2.17.so
00007fe5a2f53000   2048K ----- libdl-2.17.so
00007fe5a3153000      4K r---- libdl-2.17.so
00007fe5a3154000      4K rw--- libdl-2.17.so
00007fe5a3155000   2264K r-x-- libcrypto.so.1.0.2k
00007fe5a338b000   2048K ----- libcrypto.so.1.0.2k
00007fe5a358b000    112K r---- libcrypto.so.1.0.2k
00007fe5a35a7000     52K rw--- libcrypto.so.1.0.2k
00007fe5a35b4000     16K rw---   [ anon ]
00007fe5a35b8000    188K r-x-- libsystemd.so.0.6.0
00007fe5a35e7000   2048K ----- libsystemd.so.0.6.0
00007fe5a37e7000      4K r---- libsystemd.so.0.6.0
00007fe5a37e8000      4K rw--- libsystemd.so.0.6.0
00007fe5a37e9000    144K r-x-- libselinux.so.1
00007fe5a380d000   2044K ----- libselinux.so.1
00007fe5a3a0c000      4K r---- libselinux.so.1
00007fe5a3a0d000      4K rw--- libselinux.so.1
00007fe5a3a0e000      8K rw---   [ anon ]
00007fe5a3a10000     52K r-x-- libpam.so.0.83.1
00007fe5a3a1d000   2048K ----- libpam.so.0.83.1
00007fe5a3c1d000      4K r---- libpam.so.0.83.1
00007fe5a3c1e000      4K rw--- libpam.so.0.83.1
00007fe5a3c1f000    120K r-x-- libaudit.so.1.0.0
00007fe5a3c3d000   2044K ----- libaudit.so.1.0.0
00007fe5a3e3c000      4K r---- libaudit.so.1.0.0
00007fe5a3e3d000      4K rw--- libaudit.so.1.0.0
00007fe5a3e3e000     40K rw---   [ anon ]
00007fe5a3e48000     36K r-x-- libwrap.so.0.7.6
00007fe5a3e51000   2044K ----- libwrap.so.0.7.6
00007fe5a4050000      4K r---- libwrap.so.0.7.6
00007fe5a4051000      4K rw--- libwrap.so.0.7.6
00007fe5a4052000      4K rw---   [ anon ]
00007fe5a4053000      8K r-x-- libfipscheck.so.1.2.1
00007fe5a4055000   2044K ----- libfipscheck.so.1.2.1
00007fe5a4254000      4K r---- libfipscheck.so.1.2.1
00007fe5a4255000      4K rw--- libfipscheck.so.1.2.1
00007fe5a4256000    136K r-x-- ld-2.17.so
00007fe5a4457000     92K rw---   [ anon ]
00007fe5a4476000      4K rw---   [ anon ]
00007fe5a4477000      4K r---- ld-2.17.so
00007fe5a4478000      4K rw--- ld-2.17.so
00007fe5a4479000      4K rw---   [ anon ]
00007ffe7c7d7000    132K rw---   [ stack ]
00007ffe7c7f9000      8K r-x--   [ anon ]
ffffffffff600000      4K r-x--   [ anon ]
 total           112928K
{% endhighlight %}

2) **Memory usage map of multiple processes**

在pmap命令后接多个pid就可以查看多个进程的内存映射：
{% highlight string %}
# pmap 1013 1217 1118
{% endhighlight %}

3) **Extended memory map about a process**

如果要查看一个进程的扩张内存信息，我们可以使用```-x```选项。加```-x```选项后会打印出Address、Kbyte、Dirty、RSS、mode、mapping等信息。

下面我们介绍一下扩展格式相关字段(Extended and Device Format Fields)的含义：
<pre>
Address: start address of map

Kbytes: size of map in kilobytes

RSS: resident set size in kilobytes

Dirty: dirty pages (both shared and private) in kilobytes

Mode: permissions on map: read, write, execute, shared, private (copy on write)

Mapping: file backing the map, or '[ anon ]' for allocated memory, or '[ stack ]' for the program stack

Offset: offset into the file

Device: device name (major:minor)
</pre>

参看如下示例：
{% highlight string %}
# pmap -x 1345
1345:   /usr/sbin/sshd -D
Address           Kbytes     RSS   Dirty Mode  Mapping
000055a8a137b000     800      76       0 r-x-- sshd
000055a8a1642000      16      16      16 r---- sshd
000055a8a1646000       4       4       4 rw--- sshd
000055a8a1647000      36      36      36 rw---   [ anon ]
000055a8a2bbf000     132      56      56 rw---   [ anon ]
00007fe59d566000      48       0       0 r-x-- libnss_files-2.17.so
00007fe59d572000    2044       0       0 ----- libnss_files-2.17.so
00007fe59d771000       4       4       4 r---- libnss_files-2.17.so
00007fe59d772000       4       4       4 rw--- libnss_files-2.17.so
00007fe59d773000      24       0       0 rw---   [ anon ]
00007fe59d779000      60       0       0 r-x-- libbz2.so.1.0.6
00007fe59d788000    2044       0       0 ----- libbz2.so.1.0.6
00007fe59d987000       4       4       4 r---- libbz2.so.1.0.6
00007fe59d988000       4       4       4 rw--- libbz2.so.1.0.6
00007fe59d989000      92       0       0 r-x-- libelf-0.176.so
00007fe59d9a0000    2044       0       0 ----- libelf-0.176.so
00007fe59db9f000       4       4       4 r---- libelf-0.176.so
00007fe59dba0000       4       4       4 rw--- libelf-0.176.so
00007fe59dba1000      16       0       0 r-x-- libattr.so.1.1.0
00007fe59dba5000    2044       0       0 ----- libattr.so.1.1.0
00007fe59dda4000       4       4       4 r---- libattr.so.1.1.0
00007fe59dda5000       4       4       4 rw--- libattr.so.1.1.0
00007fe59dda6000      12       0       0 r-x-- libkeyutils.so.1.5
00007fe59dda9000    2044       0       0 ----- libkeyutils.so.1.5
00007fe59dfa8000       4       4       4 r---- libkeyutils.so.1.5
00007fe59dfa9000       4       4       4 rw--- libkeyutils.so.1.5
00007fe59dfaa000      56       0       0 r-x-- libkrb5support.so.0.1
00007fe59dfb8000    2048       0       0 ----- libkrb5support.so.0.1
00007fe59e1b8000       4       4       4 r---- libkrb5support.so.0.1
00007fe59e1b9000       4       4       4 rw--- libkrb5support.so.0.1
00007fe59e1ba000       8       0       0 r-x-- libfreebl3.so
00007fe59e1bc000    2044       0       0 ----- libfreebl3.so
00007fe59e3bb000       4       4       4 r---- libfreebl3.so
00007fe59e3bc000       4       4       4 rw--- libfreebl3.so
00007fe59e3bd000     232       0       0 r-x-- libnspr4.so
00007fe59e3f7000    2044       0       0 ----- libnspr4.so
00007fe59e5f6000       4       4       4 r---- libnspr4.so
00007fe59e5f7000       8       8       8 rw--- libnspr4.so
00007fe59e5f9000       8       0       0 rw---   [ anon ]
00007fe59e5fb000      16       0       0 r-x-- libplc4.so
00007fe59e5ff000    2044       0       0 ----- libplc4.so
00007fe59e7fe000       4       4       4 r---- libplc4.so
00007fe59e7ff000       4       4       4 rw--- libplc4.so
00007fe59e800000      12       0       0 r-x-- libplds4.so
00007fe59e803000    2044       0       0 ----- libplds4.so
00007fe59ea02000       4       4       4 r---- libplds4.so
00007fe59ea03000       4       4       4 rw--- libplds4.so
00007fe59ea04000     164       0       0 r-x-- libnssutil3.so
00007fe59ea2d000    2044       0       0 ----- libnssutil3.so
00007fe59ec2c000      28      28      28 r---- libnssutil3.so
00007fe59ec33000       4       4       4 rw--- libnssutil3.so
00007fe59ec34000    1176       0       0 r-x-- libnss3.so
00007fe59ed5a000    2048       0       0 ----- libnss3.so
00007fe59ef5a000      20      20      20 r---- libnss3.so
00007fe59ef5f000       8       8       8 rw--- libnss3.so
00007fe59ef61000       8       0       0 rw---   [ anon ]
00007fe59ef63000     148       0       0 r-x-- libsmime3.so
00007fe59ef88000    2044       0       0 ----- libsmime3.so
00007fe59f187000      12      12      12 r---- libsmime3.so
00007fe59f18a000       4       4       4 rw--- libsmime3.so
00007fe59f18b000     332       0       0 r-x-- libssl3.so
00007fe59f1de000    2048       0       0 ----- libssl3.so
00007fe59f3de000      16      16      16 r---- libssl3.so
00007fe59f3e2000       4       4       4 rw--- libssl3.so
00007fe59f3e3000       4       0       0 rw---   [ anon ]
00007fe59f3e4000     412       0       0 r-x-- libssl.so.1.0.2k
00007fe59f44b000    2048       0       0 ----- libssl.so.1.0.2k
00007fe59f64b000      16      16      16 r---- libssl.so.1.0.2k
00007fe59f64f000      28      28      28 rw--- libssl.so.1.0.2k
00007fe59f656000     112       0       0 r-x-- libsasl2.so.3.0.0
00007fe59f672000    2044       0       0 ----- libsasl2.so.3.0.0
00007fe59f871000       4       4       4 r---- libsasl2.so.3.0.0
00007fe59f872000       4       4       4 rw--- libsasl2.so.3.0.0
00007fe59f873000      92       8       0 r-x-- libpthread-2.17.so
00007fe59f88a000    2044       0       0 ----- libpthread-2.17.so
00007fe59fa89000       4       4       4 r---- libpthread-2.17.so
00007fe59fa8a000       4       4       4 rw--- libpthread-2.17.so
00007fe59fa8b000      16       4       4 rw---   [ anon ]
00007fe59fa8f000      84       0       0 r-x-- libgcc_s-4.8.5-20150702.so.1
00007fe59faa4000    2044       0       0 ----- libgcc_s-4.8.5-20150702.so.1
00007fe59fca3000       4       4       4 r---- libgcc_s-4.8.5-20150702.so.1
00007fe59fca4000       4       4       4 rw--- libgcc_s-4.8.5-20150702.so.1
00007fe59fca5000     312       0       0 r-x-- libdw-0.176.so
00007fe59fcf3000    2048       0       0 ----- libdw-0.176.so
00007fe59fef3000       8       8       8 r---- libdw-0.176.so
00007fe59fef5000       4       4       4 rw--- libdw-0.176.so
00007fe59fef6000      16       0       0 r-x-- libgpg-error.so.0.10.0
00007fe59fefa000    2044       0       0 ----- libgpg-error.so.0.10.0
00007fe5a00f9000       4       4       4 r---- libgpg-error.so.0.10.0
00007fe5a00fa000       4       4       4 rw--- libgpg-error.so.0.10.0
00007fe5a00fb000     500       0       0 r-x-- libgcrypt.so.11.8.2
00007fe5a0178000    2044       0       0 ----- libgcrypt.so.11.8.2
00007fe5a0377000       4       4       4 r---- libgcrypt.so.11.8.2
00007fe5a0378000      12      12      12 rw--- libgcrypt.so.11.8.2
00007fe5a037b000       4       0       0 rw---   [ anon ]
00007fe5a037c000      80       0       0 r-x-- liblz4.so.1.7.5
00007fe5a0390000    2044       0       0 ----- liblz4.so.1.7.5
00007fe5a058f000       4       4       4 r---- liblz4.so.1.7.5
00007fe5a0590000       4       4       4 rw--- liblz4.so.1.7.5
00007fe5a0591000     148       0       0 r-x-- liblzma.so.5.2.2
00007fe5a05b6000    2044       0       0 ----- liblzma.so.5.2.2
00007fe5a07b5000       4       4       4 r---- liblzma.so.5.2.2
00007fe5a07b6000       4       4       4 rw--- liblzma.so.5.2.2
00007fe5a07b7000      28       0       0 r-x-- librt-2.17.so
00007fe5a07be000    2044       0       0 ----- librt-2.17.so
00007fe5a09bd000       4       4       4 r---- librt-2.17.so
00007fe5a09be000       4       4       4 rw--- librt-2.17.so
00007fe5a09bf000    1028       0       0 r-x-- libm-2.17.so
00007fe5a0ac0000    2044       0       0 ----- libm-2.17.so
00007fe5a0cbf000       4       4       4 r---- libm-2.17.so
00007fe5a0cc0000       4       4       4 rw--- libm-2.17.so
00007fe5a0cc1000      16       0       0 r-x-- libcap.so.2.22
00007fe5a0cc5000    2044       0       0 ----- libcap.so.2.22
00007fe5a0ec4000       4       4       4 r---- libcap.so.2.22
00007fe5a0ec5000       4       4       4 rw--- libcap.so.2.22
00007fe5a0ec6000     384       0       0 r-x-- libpcre.so.1.2.0
00007fe5a0f26000    2048       0       0 ----- libpcre.so.1.2.0
00007fe5a1126000       4       4       4 r---- libpcre.so.1.2.0
00007fe5a1127000       4       4       4 rw--- libpcre.so.1.2.0
00007fe5a1128000      16       0       0 r-x-- libcap-ng.so.0.0.0
00007fe5a112c000    2048       0       0 ----- libcap-ng.so.0.0.0
00007fe5a132c000       4       4       4 r---- libcap-ng.so.0.0.0
00007fe5a132d000       4       4       4 rw--- libcap-ng.so.0.0.0
00007fe5a132e000      92       0       0 r-x-- libnsl-2.17.so
00007fe5a1345000    2044       0       0 ----- libnsl-2.17.so
00007fe5a1544000       4       4       4 r---- libnsl-2.17.so
00007fe5a1545000       4       4       4 rw--- libnsl-2.17.so
00007fe5a1546000       8       0       0 rw---   [ anon ]
00007fe5a1548000    1804     116       0 r-x-- libc-2.17.so
00007fe5a170b000    2048       0       0 ----- libc-2.17.so
00007fe5a190b000      16      16      16 r---- libc-2.17.so
00007fe5a190f000       8       8       8 rw--- libc-2.17.so
00007fe5a1911000      20      20      20 rw---   [ anon ]
00007fe5a1916000      12       0       0 r-x-- libcom_err.so.2.1
00007fe5a1919000    2044       0       0 ----- libcom_err.so.2.1
00007fe5a1b18000       4       4       4 r---- libcom_err.so.2.1
00007fe5a1b19000       4       4       4 rw--- libcom_err.so.2.1
00007fe5a1b1a000     196       0       0 r-x-- libk5crypto.so.3.1
00007fe5a1b4b000    2044       0       0 ----- libk5crypto.so.3.1
00007fe5a1d4a000       8       8       8 r---- libk5crypto.so.3.1
00007fe5a1d4c000       4       4       4 rw--- libk5crypto.so.3.1
00007fe5a1d4d000     868       0       0 r-x-- libkrb5.so.3.3
00007fe5a1e26000    2044       0       0 ----- libkrb5.so.3.3
00007fe5a2025000      56      56      56 r---- libkrb5.so.3.3
00007fe5a2033000      12      12      12 rw--- libkrb5.so.3.3
00007fe5a2036000     296       0       0 r-x-- libgssapi_krb5.so.2.2
00007fe5a2080000    2048       0       0 ----- libgssapi_krb5.so.2.2
00007fe5a2280000       4       4       4 r---- libgssapi_krb5.so.2.2
00007fe5a2281000       8       8       8 rw--- libgssapi_krb5.so.2.2
00007fe5a2283000      88       0       0 r-x-- libresolv-2.17.so
00007fe5a2299000    2048       0       0 ----- libresolv-2.17.so
00007fe5a2499000       4       4       4 r---- libresolv-2.17.so
00007fe5a249a000       4       4       4 rw--- libresolv-2.17.so
00007fe5a249b000       8       0       0 rw---   [ anon ]
00007fe5a249d000      32       0       0 r-x-- libcrypt-2.17.so
00007fe5a24a5000    2044       0       0 ----- libcrypt-2.17.so
00007fe5a26a4000       4       4       4 r---- libcrypt-2.17.so
00007fe5a26a5000       4       4       4 rw--- libcrypt-2.17.so
00007fe5a26a6000     184       0       0 rw---   [ anon ]
00007fe5a26d4000      84       0       0 r-x-- libz.so.1.2.7
00007fe5a26e9000    2044       0       0 ----- libz.so.1.2.7
00007fe5a28e8000       4       4       4 r---- libz.so.1.2.7
00007fe5a28e9000       4       4       4 rw--- libz.so.1.2.7
00007fe5a28ea000       8       0       0 r-x-- libutil-2.17.so
00007fe5a28ec000    2044       0       0 ----- libutil-2.17.so
00007fe5a2aeb000       4       4       4 r---- libutil-2.17.so
00007fe5a2aec000       4       4       4 rw--- libutil-2.17.so
00007fe5a2aed000      56       0       0 r-x-- liblber-2.4.so.2.10.7
00007fe5a2afb000    2044       0       0 ----- liblber-2.4.so.2.10.7
00007fe5a2cfa000       4       4       4 r---- liblber-2.4.so.2.10.7
00007fe5a2cfb000       4       4       4 rw--- liblber-2.4.so.2.10.7
00007fe5a2cfc000     328       0       0 r-x-- libldap-2.4.so.2.10.7
00007fe5a2d4e000    2048       0       0 ----- libldap-2.4.so.2.10.7
00007fe5a2f4e000       8       8       8 r---- libldap-2.4.so.2.10.7
00007fe5a2f50000       4       4       4 rw--- libldap-2.4.so.2.10.7
00007fe5a2f51000       8       0       0 r-x-- libdl-2.17.so
00007fe5a2f53000    2048       0       0 ----- libdl-2.17.so
00007fe5a3153000       4       4       4 r---- libdl-2.17.so
00007fe5a3154000       4       4       4 rw--- libdl-2.17.so
00007fe5a3155000    2264      68       0 r-x-- libcrypto.so.1.0.2k
00007fe5a338b000    2048       0       0 ----- libcrypto.so.1.0.2k
00007fe5a358b000     112     112     112 r---- libcrypto.so.1.0.2k
00007fe5a35a7000      52      52      52 rw--- libcrypto.so.1.0.2k
00007fe5a35b4000      16       8       8 rw---   [ anon ]
00007fe5a35b8000     188       0       0 r-x-- libsystemd.so.0.6.0
00007fe5a35e7000    2048       0       0 ----- libsystemd.so.0.6.0
00007fe5a37e7000       4       4       4 r---- libsystemd.so.0.6.0
00007fe5a37e8000       4       4       4 rw--- libsystemd.so.0.6.0
00007fe5a37e9000     144       0       0 r-x-- libselinux.so.1
00007fe5a380d000    2044       0       0 ----- libselinux.so.1
00007fe5a3a0c000       4       4       4 r---- libselinux.so.1
00007fe5a3a0d000       4       4       4 rw--- libselinux.so.1
00007fe5a3a0e000       8       4       4 rw---   [ anon ]
00007fe5a3a10000      52       0       0 r-x-- libpam.so.0.83.1
00007fe5a3a1d000    2048       0       0 ----- libpam.so.0.83.1
00007fe5a3c1d000       4       4       4 r---- libpam.so.0.83.1
00007fe5a3c1e000       4       4       4 rw--- libpam.so.0.83.1
00007fe5a3c1f000     120       0       0 r-x-- libaudit.so.1.0.0
00007fe5a3c3d000    2044       0       0 ----- libaudit.so.1.0.0
00007fe5a3e3c000       4       4       4 r---- libaudit.so.1.0.0
00007fe5a3e3d000       4       4       4 rw--- libaudit.so.1.0.0
00007fe5a3e3e000      40       0       0 rw---   [ anon ]
00007fe5a3e48000      36       0       0 r-x-- libwrap.so.0.7.6
00007fe5a3e51000    2044       0       0 ----- libwrap.so.0.7.6
00007fe5a4050000       4       4       4 r---- libwrap.so.0.7.6
00007fe5a4051000       4       4       4 rw--- libwrap.so.0.7.6
00007fe5a4052000       4       0       0 rw---   [ anon ]
00007fe5a4053000       8       0       0 r-x-- libfipscheck.so.1.2.1
00007fe5a4055000    2044       0       0 ----- libfipscheck.so.1.2.1
00007fe5a4254000       4       4       4 r---- libfipscheck.so.1.2.1
00007fe5a4255000       4       4       4 rw--- libfipscheck.so.1.2.1
00007fe5a4256000     136       8       0 r-x-- ld-2.17.so
00007fe5a4457000      92      92      92 rw---   [ anon ]
00007fe5a4476000       4       4       4 rw---   [ anon ]
00007fe5a4477000       4       4       4 r---- ld-2.17.so
00007fe5a4478000       4       4       4 rw--- ld-2.17.so
00007fe5a4479000       4       4       4 rw---   [ anon ]
00007ffe7c7d7000     132      20      20 rw---   [ stack ]
00007ffe7c7f9000       8       4       0 r-x--   [ anon ]
ffffffffff600000       4       0       0 r-x--   [ anon ]
---------------- ------- ------- ------- 
total kB          112928    1300    1020
{% endhighlight %}






<br />
<br />

**[参看]**

1. [How to Check Memory Usage of Process with Linux pmap Command](https://linoxide.com/pmap-command/)

2. [Linux commands](https://linoxide.com/category/commands/)

3. [pmap源码](https://gitlab.com/procps-ng/procps/blob/master/pmap.c)

4. [How to Check Memory Usage Per Process on Linux](https://linuxhint.com/check_memory_usage_process_linux/)

5. [pmap man page](https://www.man7.org/linux/man-pages/man1/pmap.1.html)

6. [Linux pmap命令：查看进程用了多少内存](https://os.51cto.com/art/201311/415915.htm)



<br />
<br />
<br />


