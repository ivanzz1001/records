---
layout: post
title: ceph源代码编译
tags:
- ceph
categories: ceph
description: ceph源代码编译
---


当前生产环境中我们所使用的ceph版本为jewel版本：
<pre>
# ceph --version
ceph version 10.2.10 (5dc1e4c05cb68dbf62ae6fce3f0700e4654fdbbe)
</pre>
因此，这里我们也以该版本为例来介绍ceph源代码的编译。当前我们的操作系统环境如下(ceph编译时需要耗费极大量的内存，建议内存至少4G以上)：
<pre>
# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# free -h
              total        used        free      shared  buff/cache   available
Mem:           3.7G        1.2G        520M         65M        2.0G        2.0G
Swap:          4.9G          0B        4.9G
</pre>
所使用的编译器版本为：
<pre>
# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-16)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>



<!-- more -->

## 1. 编译安装Jewel ceph

### 1.1 下载ceph源码

这里我们下载ceph-10.2.10版本：
<pre>
# wget https://github.com/ceph/ceph/archive/v10.2.10.tar.gz
# tar -zxvf v10.2.10.tar.gz
# ls
ceph-10.2.10  v10.2.10.tar.gz
</pre>

注： 在ceph官网下载的tar.gz包缺少文件，用git clone下载的ceph包比较全，因此我们采用如下方法
<pre>
# git clone --recursive https://github.com/ceph/ceph.git  
</pre>
下载ceph的其他子模块，否则在编译过程中会出现缺少库:
<pre>
# git submodule update --force --init --recursive
</pre>
进入ceph源码目录，git checkout jewel切换到jewel版本:
<pre>
# git checkout jewel
</pre>

### 1.2 编译ceph
下载完ceph源码，我们先参看一下```README.md```的说明(或查看官网[ceph编译说明](https://docs.ceph.com/docs/master/install/build-ceph/))，以了解大体编译步骤。

1) **安装依赖项**

执行如下命令检测并安装相应的依赖项：
<pre>
# ./install-deps.sh
</pre>

2) **生成相关编译文件**

执行如下命令预先生成编译时需要的相关文件：
<pre>
# ./autogen.sh
</pre>
在执行过程中提示没有libtoolize,及gcc,g++等工具，执行如下命令进行安装：
{% highlight string %}
# yum install libtool
# yum install gcc
# yum install gcc-c++
{% endhighlight %}

3) **生成Makefile文件**

执行如下命令生成编译对应的Makefile文件：
<pre>
# ./configure 
</pre>
在执行configure过程中可能会提示需要安装相应的软件包，执行如下命令安装：
<pre>
# yum install snappy-devel
# yum install leveldb-devel
# yum install libuuid-devel
# yum install libblkid-devel
# yum install libudev-devel
# yum install keyutils-libs-devel
# yum install cryptopp-devel
# yum install fuse-devel
# yum install libatomic_ops-devel
# yum install libaio-devel
# yum install xfsprogs-devel
# yum install boost-devel
# yum install libedit-devel
# yum install expat-devel
</pre>
>如果不想要依赖于google-perftools，请使用: ./configure --without-tcmalloc

4) **编译**

直接执行make命令编译即可：



## 3. 编译安装rocksdb
rocksdb起源于Facebook的实验室项目，实现了一个高性能的快速存储器，是基于C++编写的key value数据库，很多软件都是采用内置rocksdb的方式运行。因此这里我们先介绍一下rocksdb的安装。


1) **下载rocksdb源码**

这里我们安装RocksDB v6.3.6:
<pre>
# wget https://github.com/facebook/rocksdb/archive/v6.3.6.tar.gz
# tar -zxvf v6.3.6.tar.gz 
# cd rocksdb-6.3.6/
</pre>


2） **编译安装rocksdb**

查看```rocksdb-6.3.6```中INSTALL.md文件，发现rocksdb需要依赖gflags、snappy、zlib、bzip2、lz4、ASAN、zstandard，下面我们分别安装：

* 安装gflags
<pre>
# git clone https://github.com/gflags/gflags.git
# cd gflags
# git checkout v2.0
# ./configure && make && sudo make install
</pre>
安装完成后，可以看到在/usr/local/lib目录下有libgflags相关的文件：
<pre>
# ls /usr/local/lib/libgflags*
/usr/local/lib/libgflags.a            /usr/local/lib/libgflags_nothreads.la    /usr/local/lib/libgflags_nothreads.so.2.1.0  /usr/local/lib/libgflags.so.2.1.0
/usr/local/lib/libgflags.la           /usr/local/lib/libgflags_nothreads.so    /usr/local/lib/libgflags.so
/usr/local/lib/libgflags_nothreads.a  /usr/local/lib/libgflags_nothreads.so.2  /usr/local/lib/libgflags.so.2
[root@localhost gflags]# 
</pre>

* 安装snappy
<pre>
# yum install snappy snappy-devel
</pre>

* 安装zlib
<pre>
# yum install zlib zlib-devel
</pre>

* 安装bzip2
<pre>
# yum install bzip2 bzip2-devel
</pre>

* 安装lz4
<pre>
# yum install lz4-devel
</pre>

* 安装ANSN(主要用于调试)
<pre>
# yum install libasan
</pre>

* 安装zstandard
<pre>
# wget https://github.com/facebook/zstd/archive/v1.1.3.tar.gz
# mv v1.1.3.tar.gz zstd-1.1.3.tar.gz
# tar zxvf zstd-1.1.3.tar.gz
# cd zstd-1.1.3
# make && sudo make install            
</pre>
安装完成后就会在/usr/loca/lib目录下生成zstd相关的动态链接库。

在上述依赖都安装完成后，执行如下命令编译rocksdb(根据自己需要选择对应的方式）:

* 编译静态库，获得librocksdb.a
<pre>
# make static_lib
# make install-static
</pre>

* 编译动态库，获得librocksdb.so
<pre>
# make shared_lib
# make install-shared
</pre>

* 如果嫌命令太多，可以进行以下操作
<pre>
# make all
</pre>
但是根据文档说明，不建议采用此种方式，因为```make all```会debug模式来编译rocksdb，不适合在生产环境中使用。

我们推荐使用如下方式来进行编译：
<pre>
# make static_lib
# make shared_lib
# make install           //默认安装static_lib，如果shared_lib存在，则也会被安装
</pre>
默认情况下会安装到/usr/local/lib目录。

### 3.1 RocksDB简单测试程序
如下我们来写一段简单的RocksDB的测试程序(rocksdb_test.cpp):
{% highlight string %}
#include <cstdio>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

using namespace std;
using namespace rocksdb;

const std::string PATH = "kv/rocksdb_tmp";

int main(){
    DB* db;
    Options options;
    options.create_if_missing = true;
    Status status = DB::Open(options, PATH, &db);
    assert(status.ok());
    Slice key("foo");
    Slice value("bar");
    
    std::string get_value;
    status = db->Put(WriteOptions(), key, value);
    if(status.ok()){
        status = db->Get(ReadOptions(), key, &get_value);
        if(status.ok()){
            printf("get %s\n", get_value.c_str());
        }else{
            printf("get failed\n"); 
        }
    }else{
        printf("put failed\n");
    }

    delete db;
}
{% endhighlight %}
编译：
<pre>
# gcc -o rocksdb_test rocksdb_test.cpp -std=c++11 -lstdc++ -lrocksdb -ldl
</pre>
执行如下命令运行：
<pre>
# export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH 
# mkdir kv
# rocksdb_test
get bar
</pre>




<br />
<br />

**参考**:

1. [指定ceph版本下载](https://github.com/ceph/ceph/releases?after=v12.2.4)

2. [RocksDB简单使用](https://www.jianshu.com/p/f233528c8303)

3. [rocksdb 安装全过程 & 一些问题解决方法](https://blog.csdn.net/weixin_38976558/article/details/91616093)

4. [ceph（jewel版）编译](https://blog.csdn.net/weixin_34137799/article/details/92231040)
<br />
<br />
<br />

