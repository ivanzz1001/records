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
# ./configure --with-rbd --with-debug --with-rados --with-radosgw --with-cephfs --enable-client --enable-server --enable-xio --enable-pgrefdebugging --enable-valgrind
</pre>
上面我们```enable```一些选项，主要是为了便于我们后边在研究ceph源代码时可以对相关的单元测试脚本进行测试。

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

如果我们想要使用GDB来进行调试的话，我们需要修改Makefile文件，找到所有```O2```行，把它替换成```-O0-Wall -g```:
<pre>
# pwd
/root/ceph-inst/ceph

# grep -nw "O2" ./Makefile
431:CCASFLAGS = -g -O2
433:CFLAGS = -D_LARGEFILE64_SOURCE -g -O2
443:CXXFLAGS = -g -O2 -std=gnu++11
532:PYTHON_CFLAGS = -I/usr/include/python2.7 -I/usr/include/python2.7 -fno-strict-aliasing -O2 -g ...
</pre>


如果需要进行更深层次调试的话，可以执行如下命令安装调试依赖包：
<pre>
# yum install  lttng-tools* lttng-ust* lttng*
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
# make

//单独编译osd模块，可执行(src目录下)
# make ceph-osd
//执行make check会运行src/tests目录下的单元测试

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
	echo "run here 3...";\
	target=`echo $@ | sed s/-recursive//`;\
	echo $$target
{% endhighlight %}
如下我们执行上面这个Makefile：
{% highlight string %}
# make -n
fail=;\
echo "run here 1...";\
echo "run here 2...";\
echo "run here 3...";\
target=`echo all-recursive | sed s/-recursive//`;\
echo $target

# make
run here 1...
run here 2...
run here 3...
all
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
<pre>
-m  指出monitor节点的ip地址和默认端口6789

-n  指出此次部署为全新部署

-d  指出使用debug模式（便于调试代码）

-r  指出启动radosgw进程

--mon_num  指出部署的monitor个数

--osd_num  指出部署的osd个数

--mds_num  指出部署的mds个数

--blustore  指出ceph后端存储使用最新的bluestore
</pre>

现在我们通过如下命令来尝试启动ceph集群：
{% highlight string %}
# cd src
# MON=1 MDS=0 OSD=3  ./vstart.sh -d -n -x -r  
** going verbose **
./init-ceph: ceph conf ./ceph.conf not found; system is not configured.
hostname localhost
ip 172.19.0.1
ip 172.19.0.1
port 
creating /root/ceph-inst/ceph/src/keyring
./monmaptool --create --clobber --add a 172.19.0.1:6789 --print /tmp/ceph_monmap.85091
./monmaptool: monmap file /tmp/ceph_monmap.85091
./monmaptool: generated fsid c4ea0e5a-962d-48a8-b8a5-56d322d25659
epoch 0
fsid c4ea0e5a-962d-48a8-b8a5-56d322d25659
last_changed 2020-04-07 10:55:43.306120
created 2020-04-07 10:55:43.306120
0: 172.19.0.1:6789/0 mon.a
./monmaptool: writing epoch 0 to /tmp/ceph_monmap.85091 (1 monitors)
.....
.....
setting up user testid
2020-04-07 10:55:57.438088 7feb49a459c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:55:57.438264 7feb49a459c0  0 lockdep start
2020-04-07 10:55:57.438596 7feb49a459c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:55:57.466235 7feb49a459c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:10.906012 7feb49a459c0  0 lockdep stop
setting up s3-test users
2020-04-07 10:56:10.978801 7f31069a39c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:10.979357 7f31069a39c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.000158 7f31069a39c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.175270 7f31069a39c0  0 lockdep stop
2020-04-07 10:56:11.234809 7fb2f27879c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.235285 7fb2f27879c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.258143 7fb2f27879c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.410559 7fb2f27879c0  0 lockdep stop
setting up user tester
2020-04-07 10:56:11.469506 7fdb4a6359c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.470049 7fdb4a6359c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:11.490293 7fdb4a6359c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:12.926742 7fdb4a6359c0  0 lockdep stop

S3 User Info:
  access key:  0555b35654ad1656d804
  secret key:  h7GhxuBLTrlhVUyxSPUKUV8r/2EI4ngqJxD7iBdBYLhwluN30JaT3Q==

Swift User Info:
  account   : test
  user      : tester
  password  : testing

start rgw on http://localhost:8000
2020-04-07 10:56:14.562873 7fc55bc759c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 10:56:14.562944 7fc55bc759c0 -1 WARNING: the following dangerous and experimental features are enabled: *
started.  stop.sh to stop.  see out/* (e.g. 'tail -f out/????') for debug output.

export PYTHONPATH=./pybind:./pybind:
export LD_LIBRARY_PATH=.libs

# ps -ef | grep ceph
root      85149      1  0 10:55 pts/0    00:03:00 ./ceph-mon -i a -c /root/ceph-inst/ceph/src/ceph.conf
root      85285      1  0 10:55 ?        00:02:16 ./ceph-osd -i 0 -c /root/ceph-inst/ceph/src/ceph.conf
root      85471      1  0 10:55 ?        00:02:49 ./ceph-osd -i 1 -c /root/ceph-inst/ceph/src/ceph.conf
root      85675      1  0 10:55 ?        00:02:23 ./ceph-osd -i 2 -c /root/ceph-inst/ceph/src/ceph.conf
root      86187      1  0 10:56 ?        00:01:31 /root/ceph-inst/ceph/src/.libs/lt-radosgw -c /root/ceph-inst/ceph/src/ceph.conf --log-file=/root/ceph-inst/ceph/src/out/rgw.log --debug-rgw=20 --debug-ms=1

# netstat -nlp
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name    
tcp        0      0 0.0.0.0:111             0.0.0.0:*               LISTEN      1/systemd           
tcp        0      0 0.0.0.0:6800            0.0.0.0:*               LISTEN      85285/./ceph-osd    
tcp        0      0 0.0.0.0:6801            0.0.0.0:*               LISTEN      85285/./ceph-osd    
tcp        0      0 0.0.0.0:6802            0.0.0.0:*               LISTEN      85285/./ceph-osd    
tcp        0      0 0.0.0.0:6803            0.0.0.0:*               LISTEN      85285/./ceph-osd    
tcp        0      0 0.0.0.0:6804            0.0.0.0:*               LISTEN      85471/./ceph-osd    
tcp        0      0 0.0.0.0:6805            0.0.0.0:*               LISTEN      85471/./ceph-osd    
tcp        0      0 192.168.122.1:53        0.0.0.0:*               LISTEN      2298/dnsmasq        
tcp        0      0 0.0.0.0:6806            0.0.0.0:*               LISTEN      85471/./ceph-osd    
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      1007/sshd           
tcp        0      0 0.0.0.0:6807            0.0.0.0:*               LISTEN      85471/./ceph-osd    
tcp        0      0 127.0.0.1:631           0.0.0.0:*               LISTEN      967/cupsd           
tcp        0      0 0.0.0.0:6808            0.0.0.0:*               LISTEN      85675/./ceph-osd    
tcp        0      0 0.0.0.0:6809            0.0.0.0:*               LISTEN      85675/./ceph-osd    
tcp        0      0 0.0.0.0:6810            0.0.0.0:*               LISTEN      85675/./ceph-osd    
tcp        0      0 0.0.0.0:6811            0.0.0.0:*               LISTEN      85675/./ceph-osd    
tcp        0      0 0.0.0.0:8000            0.0.0.0:*               LISTEN      86187/lt-radosgw    
tcp        0      0 172.19.0.1:6789         0.0.0.0:*               LISTEN      85149/./ceph-mon 

# ./ceph -s
*** DEVELOPER MODE: setting PATH, PYTHONPATH and LD_LIBRARY_PATH ***
2020-04-07 11:00:48.604465 7f9e1306c700 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:00:48.631141 7f9e1306c700 -1 WARNING: the following dangerous and experimental features are enabled: *
    cluster 5f43afcf-46bd-4bbb-b4fc-b6c86a9ed2a4
     health HEALTH_OK
     monmap e1: 1 mons at {a=172.19.0.1:6789/0}
            election epoch 3, quorum 0 a
     osdmap e23: 3 osds: 3 up, 3 in
            flags sortbitwise,require_jewel_osds
      pgmap v180: 80 pgs, 10 pools, 3441 bytes data, 182 objects
            130 GB used, 95847 MB / 224 GB avail
                  80 active+clean
2020-04-07 11:00:48.748806 7f9e1306c700  0 lockdep stop
{% endhighlight %}
所采用的ceph配置文件为： ```./src/ceph.conf```

2) **简单测试开发集群**

我们通过上面的命令建立了一个ceph开发集群，系统在启动时创建了一些pools:
{% highlight string %}
# ./ceph osd pool stats
*** DEVELOPER MODE: setting PATH, PYTHONPATH and LD_LIBRARY_PATH ***
2020-04-07 11:12:26.518390 7f9e1d831700 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:12:26.547846 7f9e1d831700 -1 WARNING: the following dangerous and experimental features are enabled: *
pool rbd id 0
  nothing is going on

pool .rgw.root id 1
  nothing is going on

pool default.rgw.control id 2
  nothing is going on

pool default.rgw.data.root id 3
  nothing is going on

pool default.rgw.gc id 4
  nothing is going on

pool default.rgw.log id 5
  nothing is going on

pool default.rgw.users.uid id 6
  nothing is going on

pool default.rgw.users.email id 7
  nothing is going on

pool default.rgw.users.keys id 8
  nothing is going on

pool default.rgw.users.swift id 9
  nothing is going on


2020-04-07 11:12:26.667305 7f9e1d831700  0 lockdep stop

# ./rados df
2020-04-07 11:14:59.865548 7f2584b38a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:14:59.866512 7f2584b38a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:14:59.899009 7f2584b38a40 -1 WARNING: the following dangerous and experimental features are enabled: *
pool name                 KB      objects       clones     degraded      unfound           rd        rd KB           wr        wr KB
.rgw.root                  2            4            0            0            0            0            0            4            5
default.rgw.control            0            8            0            0            0            0            0            0            0
default.rgw.data.root            0            0            0            0            0            0            0            0            0
default.rgw.gc             0           32            0            0            0           96           64           64            0
default.rgw.log            0          127            0            0            0          762          635          508            0
default.rgw.users.email            1            3            0            0            0            0            0            3            3
default.rgw.users.keys            1            3            0            0            0            0            0            3            3
default.rgw.users.swift            1            1            0            0            0            0            0            1            1
default.rgw.users.uid            2            4            0            0            0            0            0           11            5
rbd                        0            0            0            0            0            0            0            0            0
  total used       137340900          182
  total avail       97939236
  total space      235280136
2020-04-07 11:14:59.931645 7f2584b38a40  0 lockdep stop
{% endhighlight %}
之后，我们创建一个pool并在其上运行一些benchmarks:
{% highlight string %}
# ./rados mkpool mypool
2020-04-07 11:19:45.616841 7fa375b5ba40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:19:45.618207 7fa375b5ba40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:19:45.655335 7fa375b5ba40 -1 WARNING: the following dangerous and experimental features are enabled: *
successfully created pool mypool
2020-04-07 11:19:46.026736 7fa375b5ba40  0 lockdep stop
# ./rados -p mypool bench 10 write -b 123
2020-04-07 11:20:24.984180 7f3fa2f77a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:20:24.984408 7f3fa2f77a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:20:25.005372 7f3fa2f77a40 -1 WARNING: the following dangerous and experimental features are enabled: *
Maintaining 16 concurrent writes of 123 bytes to objects of size 123 for up to 10 seconds or 0 objects
Object prefix: benchmark_data_localhost.localdomain_88020
  sec Cur ops   started  finished  avg MB/s  cur MB/s last lat(s)  avg lat(s)
    0       0         0         0         0         0           -           0
    1      16       346       330  0.038795 0.0387096   0.0807099   0.0460022
    2      16       541       525 0.0308099 0.0228739   0.0318836    0.059042
    3      16       569       553  0.021611 0.00328445    0.135172   0.0750213
    4      16       706       690 0.0202186 0.0160704   0.0357038   0.0839862
    5      16       865       849 0.0199035  0.018651    0.411248   0.0863968
    6      16      1011       995 0.0194387 0.0171261   0.0260452   0.0904323
    7      16      1173      1157  0.019375 0.0190029   0.0368545    0.094143
    8      16      1317      1301 0.0190634 0.0168915   0.0734105   0.0944436
    9      16      1481      1465 0.0190815 0.0192375   0.0222677   0.0981518
   10      16      1619      1603 0.0187919 0.0161877   0.0382804    0.097628
Total time run:         10.323772
Total writes made:      1620
Write size:             123
Object size:            123
Bandwidth (MB/sec):     0.0184069
Stddev Bandwidth:       0.00865831
Max bandwidth (MB/sec): 0.0387096
Min bandwidth (MB/sec): 0.00328445
Average IOPS:           156
Stddev IOPS:            73
Max IOPS:               330
Min IOPS:               28
Average Latency(s):     0.101956
Stddev Latency(s):      0.128364
Max latency(s):         0.727147
Min latency(s):         0.00689636
Cleaning up (deleting benchmark objects)
Clean up completed and total clean up time :2.174140
2020-04-07 11:20:38.252923 7f3fa2f77a40  0 lockdep stop
{% endhighlight %}
之后我们通过如下的命令上传一些文件到pool中：
{% highlight string %}
# echo "hello,world, this is the first file" >> /opt/helloworld.txt
# echo "this file just for test" >> /opt/ossobject.txt
# ./rados -p mypool put objectone /opt/helloworld.txt 
2020-04-07 11:27:05.979105 7fad259ffa40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:05.979418 7fad259ffa40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:06.001004 7fad259ffa40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:06.035992 7fad259ffa40  0 lockdep stop
# ./rados -p mypool put objecttwo /opt/ossobject.txt 
2020-04-07 11:27:19.146952 7fb41d43ca40  0 lockdep start
2020-04-07 11:27:19.147590 7fb41d43ca40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:19.148017 7fb41d43ca40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:19.175291 7fb41d43ca40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:27:19.209158 7fb41d43ca40  0 lockdep stop
{% endhighlight %}
上传完成后，我们通过如下命令列出pool中的object:
{% highlight string %}
# ./rados -p mypool ls
2020-04-07 11:32:19.140324 7f97cea6ea40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:32:19.140542 7f97cea6ea40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:32:19.161769 7f97cea6ea40 -1 WARNING: the following dangerous and experimental features are enabled: *
objectone
objecttwo
2020-04-07 11:32:19.203814 7f97cea6ea40  0 lockdep stop
{% endhighlight %}

然后我们在把刚上传的那两个object下载下来看看：
{% highlight string %}
# ./rados -p mypool get objectone /opt/download_helloworld.txt
2020-04-07 11:34:20.877566 7fca88094a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:20.878229 7fca88094a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:20.906457 7fca88094a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:20.948758 7fca88094a40  0 lockdep stop
# ./rados -p mypool get objecttwo /opt/download_ossobject.txt
2020-04-07 11:34:40.642173 7f5dee1f1a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:40.642473 7f5dee1f1a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:40.665007 7f5dee1f1a40 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 11:34:40.693866 7f5dee1f1a40  0 lockdep stop


# cat /opt/download_helloworld.txt 
hello,world, this is the first file
# cat /opt/download_ossobject.txt 
this file just for test
{% endhighlight %}
可以看到当前我们的ceph集群可以正常的进行上传下载操作。


3) **通过RGW测试上传下载**

这里我们首先通过如下命令创建一个admin用户，并为其分配权限：
<pre>
# ./radosgw-admin user create --uid=admin --display-name="admin"
# ./radosgw-admin caps add --uid=admin --caps="data=*"
# ./radosgw-admin caps add --uid=admin --caps="metadata=*"
# ./radosgw-admin caps add --uid=admin --caps="usage=*"
# ./radosgw-admin caps add --uid=admin --caps="users=*"
# ./radosgw-admin caps add --uid=admin --caps="zone=*"
</pre>
创建完成后，通过如下命令查看admin用户的相关信息：
{% highlight string %}
# ./radosgw-admin user info --uid=admin
2020-04-07 12:13:12.072043 7fac5b3419c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:13:12.072393 7fac5b3419c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:13:12.094619 7fac5b3419c0 -1 WARNING: the following dangerous and experimental features are enabled: *
{
    "user_id": "admin",
    "display_name": "admin",
    "email": "",
    "suspended": 0,
    "max_buckets": 1000,
    "auid": 0,
    "subusers": [],
    "keys": [
        {
            "user": "admin",
            "access_key": "9C2DKZUZQ8APDKHW3NFQ",
            "secret_key": "qcDJoqadbaUek6xZafb6UPHIXMjJkYfvpNbpoHob"
        }
    ],
    "swift_keys": [],
    "caps": [
        {
            "type": "buckets",
            "perm": "*"
        },
        {
            "type": "metadata",
            "perm": "*"
        },
        {
            "type": "usage",
            "perm": "*"
        },
        {
            "type": "users",
            "perm": "*"
        },
        {
            "type": "zone",
            "perm": "*"
        }
    ],
    "op_mask": "read, write, delete",
    "default_placement": "",
    "placement_tags": [],
    "bucket_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "user_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "temp_url_keys": []
}

2020-04-07 12:13:12.241983 7fac5b3419c0  0 lockdep stop
{% endhighlight %}

之后我们就可以使用S3相关接口创建```S3用户```、```bucket```，并使用S3来上传下载文件了。

如下是我们通过S3接口创建的用户：
{% highlight string %}
# ./radosgw-admin user info --uid=oss_dev_test
2020-04-07 12:39:44.884378 7f6b0f86d9c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:39:44.885079 7f6b0f86d9c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:39:44.911123 7f6b0f86d9c0 -1 WARNING: the following dangerous and experimental features are enabled: *
{
    "user_id": "oss_dev_test",
    "display_name": "oss_dev_test@1586234316",
    "email": "",
    "suspended": 0,
    "max_buckets": 1,
    "auid": 0,
    "subusers": [],
    "keys": [
        {
            "user": "oss_dev_test",
            "access_key": "AGE9S1IKZILW0948EMWB",
            "secret_key": "haTvRTiBTN2mfT0SGCWv77G5SPRJenjVIu8XON4i"
        }
    ],
    "swift_keys": [],
    "caps": [
        {
            "type": "buckets",
            "perm": "read"
        },
        {
            "type": "metadata",
            "perm": "*"
        }
    ],
    "op_mask": "read, write, delete",
    "default_placement": "",
    "placement_tags": [],
    "bucket_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "user_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "temp_url_keys": []
}
{% endhighlight %}
如下是我们通过S3接口创建的bucket:
{% highlight string %}
# ./radosgw-admin bucket list
2020-04-07 12:41:57.550448 7f52ca0e19c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:41:57.551200 7f52ca0e19c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:41:57.573576 7f52ca0e19c0 -1 WARNING: the following dangerous and experimental features are enabled: *
[
    "oss_dev_test_bucket"
]
2020-04-07 12:41:57.709383 7f52ca0e19c0  0 lockdep stop
{% endhighlight %}
如下是我们通过S3接口分片上传的一个文件：
{% highlight string %}
# ./radosgw-admin bucket list --bucket=oss_dev_test_bucket
2020-04-07 12:46:03.125303 7f800d4d89c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:46:03.125862 7f800d4d89c0 -1 WARNING: the following dangerous and experimental features are enabled: *
2020-04-07 12:46:03.148132 7f800d4d89c0 -1 WARNING: the following dangerous and experimental features are enabled: *
[
    {
        "name": "helloworld_1",
        "instance": "",
        "namespace": "",
        "owner": "oss_dev_test",
        "owner_display_name": "oss_dev_test@1586234316",
        "size": 11,
        "mtime": "2020-04-07 04:45:24.720447Z",
        "etag": "469e01d115cb913ad709c749df1c5666-1",
        "content_type": "binary\/octet-stream",
        "tag": "c6f931df-dc3f-4a06-a92d-138246d413b3.4113.7",
        "flags": 0,
        "user_data": ""
    }
]
2020-04-07 12:46:03.288231 7f800d4d89c0  0 lockdep stop
{% endhighlight %}
并且通过S3接口也能够正常的下载。



4) **停止开发集群**

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

