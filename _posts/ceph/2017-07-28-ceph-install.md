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

Ceph的源码可以去Github之上clone下来，或者去Ceph官网下载。这里重点提一下Ceph的版本问题，Ceph在Hammer版本之后，采取了新的版本命名规则：

* x.0.z - 开发版

* x.1.z - 候选版

* x.2.z - 稳定、修正版


<!-- more -->

## 1. 编译安装Jewel ceph

### 1.1 下载ceph源码

这里我们下载ceph-10.2.10版本。我们有如下几种方法：

1） **直接克隆指定版本**
{% highlight string %}
# git clone https://github.com/ceph/ceph.git -b v10.2.10 --depth 1
# cd ceph
# ls -al .
./                  .git/               .gitmodule_mirrors  .mailmap            .peoplemap          
../                 .gitignore          .gitmodules         .organizationmap    
# git submodule update --force --init --recursive
# git log -s
commit 5dc1e4c05cb68dbf62ae6fce3f0700e4654fdbbe
Author: Jenkins Build Slave User <ceph-release-team@redhat.com>
Date:   Wed Oct 4 14:17:25 2017 +0000

    10.2.10
{% endhighlight %}


2) **克隆整个ceph工程**

在ceph官网下载的tar.gz包缺少文件，用git clone下载的ceph包比较全，因此我们采用如下方法
<pre>
# git clone --recursive https://github.com/ceph/ceph.git  
</pre>
下载ceph的其他子模块，否则在编译过程中会出现缺少库:
<pre>
# git submodule update --force --init --recursive
</pre>

克隆完成后，我们使用```git branch -a```命令来查看所有的分支：
<pre>
# git branch -a
* master
  remotes/origin/BZ-1650306
  remotes/origin/HEAD -> origin/master
  remotes/origin/argonaut
  remotes/origin/bobtail
  remotes/origin/ceph-facts-role
  remotes/origin/cuttlefish
  remotes/origin/dumpling
  remotes/origin/emperor
  remotes/origin/firefly
  remotes/origin/giant
  remotes/origin/guits-add_dep
  remotes/origin/guits-fix_simple_activate_fs
  remotes/origin/hammer
  remotes/origin/hammer-next
  remotes/origin/infernalis
  remotes/origin/jewel
  remotes/origin/jewel-next
  remotes/origin/joscollin-patch-1
  remotes/origin/joscollin-patch-2
  remotes/origin/kraken
  remotes/origin/luminous
  remotes/origin/luminous-no-scripts
  remotes/origin/master
  remotes/origin/mimic
  remotes/origin/mimic-new
  remotes/origin/nautilus
  remotes/origin/revert-26048-luminous-37977
  remotes/origin/rh-luminous
  remotes/origin/rh-luminous_old
  remotes/origin/v
  remotes/origin/v10.2.0
  remotes/origin/wip-36686-luminous
  remotes/origin/wip-41038-nautilus
  remotes/origin/wip-41238-nautilus
  remotes/origin/wip-add-diag-suite
  remotes/origin/wip-cd-vol-size
  remotes/origin/wip-ceph-volume-tests-no-dashboard
  remotes/origin/wip-daemonwatchdog-testing6
  remotes/origin/wip-ldapauth-wusui
  remotes/origin/wip-luminous-conf-error-message
  remotes/origin/wip-pcuzner-testing
  remotes/origin/wip-perf-keys
  remotes/origin/wip-qa-rgw-swift-server
  remotes/origin/wip-qa-rgw-swift-server-nautilus
  remotes/origin/wip-rm37865
  remotes/origin/wip-smoke-use-ca
  remotes/origin/wip-zafman-26971-diag
</pre>
另外也可以通过```git tag```命令来查询所有标签：
<pre>
# git tag -l
mark-v0.70-wip
v0.1
v0.10
v0.11
v0.12
v0.13
v0.14
v0.15
v0.16
v0.16.1
v0.17
v0.18
v0.19
v0.19.1
v0.2
v0.20
v0.20.1
v0.20.2
</pre>
然后使用如下命令切换到```10.2.10```版本：
<pre>
# git checkout v10.2.10
</pre>

3) **克隆指定tag或commitID**

对于一个大工程，我们很适合采用此方法。

* 首先新建一个文件夹```ceph-10.2.10```
<pre>
# mkdir ceph-10.2.10
# cd ceph-10.2.10
</pre>

* 接着在当前目录```ceph-10.2.10```创建一个空repository:
<pre>
# git init
</pre>

* 之后将其添加到一个远程仓库：
<pre>
# git remote add my-ceph https://github.com/ceph/ceph.git  
</pre>

* 之后fetch一个commit(或branch 或tag)
<pre>
# git fetch my-ceph v10.2.10     //注意此处v10.2.10为远程ceph仓库对应的一个tag
</pre>

* 将本地仓库的master分支reset为指定的commit
<pre>
# git reset --hard FETCH_HEAD
</pre>

* 最后再更新submodules
<pre>
# git submodule update --force --init --recursive
# git log -s
</pre>

### 1.2 编译ceph
下载完ceph源码，我们先参看一下```README.md```的说明(或查看官网[ceph编译说明](https://docs.ceph.com/docs/master/install/build-ceph/))，以了解大体编译步骤（请保持环境干净，否则可能引起不必要的麻烦。笔者编译时就因为事先安装过rocksdb而出现问题)。

1) **安装依赖项**

执行如下命令检测并安装相应的依赖项：
<pre>
# ./install-deps.sh
</pre>

2) **编译ceph**

其实ceph编译有两种方式，一种是运行autogen.sh后接着运行configure，接着运行make编译，编译完成后用make install安装。还有另外一种是直接编译成deb包。下面我们就分别介绍一下这两种方式。

###### 方式1

* 1.1 生成相关编译文件

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

* 1.2 生成Makefile文件

执行如下命令生成编译对应的Makefile文件：
<pre>
# ./configure --with-rbd --with-debug --with-rados --with-radosgw
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
>如果不想要依赖于google-perftools，请使用: ./configure --without-tcmalloc；如果需要调试以及编译测试程序可以加上```--with-debug```选项

此外还可以设置```--prefix=<dir>```来控制编译安装目录，例如：
<pre>
--prefix=/usr/local/ceph --sysconfdir=/etc/ceph
</pre>

* 1.3 编译

在编译时我们最好将```src```目录设置到PATH中，以防在编译过程中对```ceph-authtool```等工具找不到：
<pre>
# pwd
/root/ceph-inst/ceph
# export PATH=$PATH:/root/ceph-inst/ceph/src
</pre>
然后直接执行make命令即可：
<pre>
# make check
============================================================================
Testsuite summary for ceph 10.2.10
============================================================================
# TOTAL: 141
# PASS:  138
# SKIP:  0
# XFAIL: 0
# FAIL:  3
# XPASS: 0
# ERROR: 0
============================================================================

# make
</pre>

>注：最后make编译的过程中，如果遇到编译器错误，可以添加-j参数指定处理器数量，make -j2

此外编译过程中可能出现如下错误：
<pre>
You are using pip version 9.0.1, however version 20.0.2 is available.
You should consider upgrading via the 'pip install --upgrade pip' command.
src/test/cli/ceph-authtool/add-key-segv.t: failed
</pre>
这里这里需要把pip版本进行一下升级，比如本文编译时升级到了```pip-20.0.2```。执行如下命令：
<pre>
# ./src/test/virtualenv/bin/pip -V
pip 9.0.1 from /root/ceph-inst/ceph/src/test/virtualenv/lib/python2.7/site-packages (python 2.7)
# ./src/test/virtualenv/bin/pip install --upgrade pip
# ./src/test/virtualenv/bin/pip -V
pip 20.0.2 from /root/ceph-inst/ceph/src/test/virtualenv/lib/python2.7/site-packages/pip (python 2.7)
</pre>


在升级了pip之后，继续进行编译又遇到如下问题：
{% highlight string %}
Usage:   
  pip install [options] <requirement specifier> [package-index-options] ...
  pip install [options] -r <requirements file> [package-index-options] ...
  pip install [options] [-e] <vcs project url> ...
  pip install [options] [-e] <local project path> ...
  pip install [options] <archive url/path> ...

no such option: --use-wheel
{% endhighlight %}

这是因为新版本(9.0.1之后)的pip并不支持```--use-wheel```选项，遇到此种情况，我们找到对应的Makefile，将```--use-wheel```选项去掉即可。例如替换*./src/tools/setup-virtualenv.sh*中的```--use-wheel```选项，可以执行如下命令：
{% highlight string %}
# grep -rn "use-wheel" ./
# sed -i 's/--use-wheel//g' ./src/ceph-detect-init/CMakeLists.txt
# sed -i 's/--use-wheel//g' ./src/ceph-detect-init/Makefile.am
# sed -i 's/--use-wheel//g' ./src/ceph-detect-init/tox.ini
# sed -i 's/--use-wheel//g' ./src/ceph-disk/CMakeLists.txt
# sed -i 's/--use-wheel//g' ./src/ceph-disk/Makefile.am
# sed -i 's/--use-wheel//g' ./src/ceph-disk/tox.ini
# sed -i 's/--use-wheel//g' ./src/tools/setup-virtualenv.sh
# sed -i 's/--use-wheel//g' ./src/Makefile.in
{% endhighlight %}

>注：Individual tests run by make check may bind fixed ports or use identical files or subdirectories to store temporary data. They should be cleaned up so that make -j4 check can be used. make check currently needs about 25 minutes on a spinner and 16 minutes on a SSD.


* 1.4 安装
<pre>
# make install
</pre>
执行make install命令安装到本地，这一步也可以通过手动移动二进制文件和配置文件到相应目录。其中，二进制文件放到/usr/bin，库文件放到/usr/lib,配置文件存入/etc/ceph。

* 1.5 关于ceph的makefile文件

这里我们模拟ceph的Makefile文件，给一个简单的示例：
{% highlight string %}
am__recursive_targets = all-recursive


all: all-recursive



$(am__recursive_targets):
        @fail=;\
        echo "run here 1...";\
        echo "run here 2...";\
        echo "run here 3..."
{% endhighlight %}
如下我们执行上面这个Makefile：
{% highlight string %}
# make -n
fail=;\
echo "run here 1...";\
echo "run here 2...";\
echo "run here 3..."

# make 
run here 1...
run here 2...
run here 3...
{% endhighlight %}


###### 方式2

略，暂时不做介绍。


### 1.3 开发环境编译
对于开发人员，可以按如下方式进行编译：
<pre>
# ./install-deps.sh
# ./run-make-check.sh
</pre>

注： 我们可以运行```make check```来进行单元测试。


### 1.4 测试环境部署
通常我们在使用*```开发环境编译```*方式编译完成ceph后(正式环境按默认方式编译后，也可以使用此方法来进行测试)，我们可以通过如下的方式来快速搭建一个测试环境。

1) **启动开发集群**

在ceph的src目录下有一个名称为```vstart.sh```的脚本文件，开发人员可以使用该部署脚本快速的部署一个调试环境。一旦编译完成，使用如下的命令来启动ceph部署：
<pre>
# cd src
# ./vstart.sh -d -n -x
</pre>
我们也可以指定mon、mds、osd的个数：
<pre>
# MON=1 MDS=0 OSD=3  ./vstart.sh -d -n -x
</pre>
这里我们针对相关的参数做一个简单的说明：
{% highlight string %}
-m  指出monitor节点的ip地址和默认端口6789

-n  指出此次部署为全新部署

-d  指出使用debug模式（便于调试代码）

-r  指出启动radosgw进程

--mon_num  指出部署的monitor个数

--osd_num  指出部署的osd个数

--mds_num  指出部署的mds个数

--blustore  指出ceph后端存储使用最新的bluestore
{% endhighlight %}


3) **停止开发集群**

执行如下命令停止开发集群：
<pre>
# ./src/stop.sh 
</pre>



## 3. 编译安装rocksdb
rocksdb起源于Facebook的实验室项目，实现了一个高性能的快速存储器，是基于C++编写的key value数据库，很多软件都是采用内置rocksdb的方式运行。因此这里我们先介绍一下rocksdb的安装。（ceph中会自动下载rocksdb模块，不需要像此处进行额外安装，否则可能会引起很多不必要的麻烦）


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

int main(int argc, char *argv[]){
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

5. [源码编译ceph](https://blog.csdn.net/yuanfang_way/article/details/77076219)

6. [手动编译配置ceph](https://my.oschina.net/linuxhunter/blog/682013)

7. [ceph源码包官方下载](http://download.ceph.com/tarballs/)

8. [ceph 阿里云镜像](http://mirrors.aliyun.com/ceph/tarballs/)

9. [ceph编译安装教程](https://www.jianshu.com/p/2618036d7ec7)

10. [Developer Guide](https://github.com/ceph/ceph/blob/jewel-next/doc/dev/quick_guide.rst)

11. [Ceph调试开发环境搭建](https://www.jianshu.com/p/844c48d5d45e)

12. [ceph官方调试环境搭建](https://github.com/ceph/ceph/blob/jewel/doc/dev/quick_guide.rst)

13. [Ceph源码编译与打包](http://blog.itpub.net/30088583/viewspace-2136827/)
<br />
<br />
<br />

