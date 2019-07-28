---
layout: post
title: phxpaxos的安装及使用
tags:
- paxos
categories: paxos
description: phxpaxos的安装及使用
---


phxpaxos是PhxPaxos是腾讯公司微信后台团队自主研发的一套基于Paxos协议的多机状态拷贝类库。如下我们简要的来说明一下phxpaxos的安装及使用。当前我们的操作系统环境为：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core) 
# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-28)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

<!-- more -->

## 1. phxpaxos的安装

这里我们安装[phxpaxos-1.1.3版本](https://github.com/Tencent/phxpaxos/releases)。在安装之前，我们首先来看一下各目录的依赖关系：

|      目录          |       编译对象          |     内部依赖                       |           第三方库依赖    |
|:------------------|:-----------------------|:-----------------------------------|:------------------------|
|根目录              |  libphxpaxos.a         |      无                            | protobuf,leveldb        |           
|plugin             |  libphxpaxos_plugin.a  | libphxpaxos.a	                  | glog                    |
|sample/phxelection |  可执行程序	             | libphxpaxos.a,libphxpaxos_plugin.a | 无                      |
|sample/phxecho	    |  可执行程序	             | libphxpaxos.a,libphxpaxos_plugin.a | 无                      |
|sample/phxkv	    |  可执行程序	             | libphxpaxos.a,libphxpaxos_plugin.a | grpc                    |
|src/ut	            |  单元测试	             | 无	                              | gtest,gmock             |

这里我们首先新建一个安装目录```paxos-inst```，后面所有的安装均在该目录下操作：
<pre>
# mkdir -p /root/paxos-inst
# cd /root/paxos-inst//root/paxos-inst/
# ls
</pre>

### 1.1 安装前的准备

phxpaxos依赖于protobuf、leveldb、glog、grpc、gtest、gmock，因此这里我们首先需要安装这些组件。经过观察```phxpaxos-1.1.3```的安装脚本，protobuf以及leveldb自动安装，因此这里我们跳过这两个包的安装。

1） **安装grpc**

关于grpc的安装，我们前面介绍过，这里我们不再细数。

>说明： 这里我们可以完全按照grpc的安装步骤来进行安装，包括其所依赖的protobuf。上面提到phxpaxos-1.1.3安装脚本也会自动安装一个protobuf，这样导致安装两个protobuf，但通常没有关系，因为它们会被安装到不同的目录。一般我们只需要保证两个protobuf的大版本相同就不会出现问题。

我们将grpc安装在*/usr/local/grpc*目录；将protobuf安装在*/usr/local/protobuf*目录。然后执行如下命令导出protoc以及grpc_cpp_plugin:
<pre>
# export PATH=$PATH:/usr/local/protobuf/bin/:/usr/local/grpc/bin/
</pre>

2） **安装googletest**

这里我们需要安装的版本是```release-1.6.0```，执行如下命令进行安装：
<pre>
# pwd
/root/paxos-inst
# git clone --branch release-1.6.0 https://github.com/google/googletest.git
# cd googletest
# cd make
# make
# make gtest.a
# ls
gtest.a  gtest-all.o  gtest_main.a  gtest_main.o  Makefile  sample1.o  sample1_unittest  sample1_unittest.o
</pre>
安装完成后，我们需要将上面生成的```gtest_main.a```以及```gtest.a```拷贝到指定的地方，以方便后续phxpaxos的查找：
<pre>
# pwd 
/root/paxos-inst/googletest
# mkdir lib
# cp make/gtest.a lib/libgtest.a
# cp make/gtest_main.a lib/libgtest_main.a
# ls lib/
libgtest.a  libgtest_main.a
</pre>

3) **安装googlemock**

这里我们需要安装的版本是```release-1.6.0```，执行如下命令进行安装：
<pre>
# pwd
/root/paxos-inst
# git clone --branch release-1.6.0 https://github.com/google/googlemock.git
# cd googlemock
</pre>
由于```googlemock```在编译安装时需要依赖```googletest```，因此执行如下命令让其能够找到googletest:
{% highlight string %}
# pwd
/root/paxos-inst/googlemock
# ln -sf /root/paxos-inst/googletest/ gtest
# ls -al gtest
lrwxrwxrwx 1 root root 28 Jul 24 03:32 gtest -> /root/paxos-inst/googletest/
{% endhighlight %}
之后再执行如下命令进行安装：
<pre>
# pwd
/root/paxos-inst/googlemock
# cd make
# make
# make gmock.a
# ls
# ls
gmock.a  gmock-all.o  gmock_main.a  gmock_main.o  gmock_test  gmock_test.o  gtest-all.o  Makefile
</pre>
安装完成后，我们需要将上面生成的```gmock_main.a```以及```gmock.a```拷贝到指定的地方，以方便后续phxpaxos的查找：
<pre>
# pwd
/root/paxos-inst/googlemock
# mkdir lib
# cp make/gmock.a lib/libgmock.a
# cp make/gmock_main.a lib/libgmock_main.a
# ls lib/
libgmock.a  libgmock_main.a
</pre>

4) **安装glog**

这里我们需要安装的版本是```v0.3.3```，执行如下命令进行安装：
<pre>
# pwd
/root/paxos-inst
# git clone -b v0.3.3 https://github.com/google/glog.git
# cd glog
# ./configure --prefix=/usr/local/glog
# make
# make install
# ls /usr/local/glog
include  lib  share
</pre>


### 1.2 安装phxpaxos

1) **下载phxpaxos**

这里我们安装```1.1.3```版本的phxpaxos:
<pre>
# pwd
/root/paxos-inst
# wget https://github.com/Tencent/phxpaxos/archive/v1.1.3.tar.gz
# tar -zxvf v1.1.3.tar.gz
# cd phxpaxos-1.1.3/
</pre>

2) **设置第三方库路径**

我们在phxpaxos-1.1.3源代码目录下的third_party目录按如下方式指定第三方库：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3/
# cd third_party
# rm -rf *
# ln -sf /usr/local/glog/ glog 
# ln -sf /root/paxos-inst/googlemock/ gmock
# ln -sf /root/paxos-inst/googletest/ gtest
# ln -sf /usr/local/grpc/ grpc

# ls -al
total 4212
drwxrwxr-x  5 root  root     182 Jul 24 03:47 .
drwxrwxr-x 12 root  root    4096 Jul 24 04:37 ..
lrwxrwxrwx  1 root  root      16 Jul 24 03:43 glog -> /usr/local/glog/
lrwxrwxrwx  1 root  root      28 Jul 24 03:42 gmock -> /root/paxos-inst/googlemock/
lrwxrwxrwx  1 root  root      16 Jul 24 03:42 grpc -> /usr/local/grpc/
lrwxrwxrwx  1 root  root      28 Jul 24 03:42 gtest -> /root/paxos-inst/googletest/
</pre>

3) **修改编译脚本makefile.mk**

我们修改phxpaxos-1.1.3源代码根目录下的```makefile.mk```编译脚本。将*GLOG_INCLUDE_PATH*变量改为：
<pre>
#GLOG_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/glog/src/
GLOG_INCLUDE_PATH=$(SRC_BASE_PATH)/third_party/glog/include/
</pre>
同时添加*CARES_LIB_PATH*变量到```makefile.mk```文件中：
<pre>
CARES_LIB_PATH=/usr/local/cares/lib
</pre>
说明： libcares.a是grpc所依赖的一个包

4） **编译根目录**

在phxpaxos-1.1.3根目录下编译生成libphxpaxos.a，执行如下命令：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3
# ./build.sh
# make install
</pre> 
成功编译完成过后就会在```lib```目录下生成```libphxpaxos.a```。

5) **编译plugin目录**

在*phxpaxos-1.1.3/plugin*目录下编译生成libphxpaxos_plugin.a：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3
# cd plugin
# make
# make install
</pre>
编译完成后就会在*phxpaxos-1.1.3/lib*目录下生成*libphxpaxos_plugin.a*。


6) **编译sample目录**

这里我们首先要修改*sample/phxkv*目录下的```Makefile.define```文件，在*PHXKV_CLIENT_SYS_LIB*与*PHXKV_GRPCSERVER_SYS_LIB*变量末尾均加上:
<pre>
$(CARES_LIB_PATH)/libcares.a
</pre>
修改完成后大体如下：
{% highlight string %}
...

PHXKV_CLIENT_SYS_LIB=$(GRPC_LIBE_PATH)/libgrpc++_unsecure.a $(CARES_LIB_PATH)/libcares.a

....

PHXKV_GRPCSERVER_SYS_LIB=$(PHXPAXOS_LIB_PATH)/libphxpaxos_plugin.a $(CARES_LIB_PATH)/libcares.a 
{% endhighlight %}

这里修改完以后，一定要记得到*phxpaxos-1.1.3*根目录重新执行*autoinstall.sh*脚本以更新Makefile文件(直接修改Makefile文件貌似不行，可能是有些地方没修改到，这里不细究)：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3
# ./autoinstall.sh
</pre>
执行完成之后，再进入```phxpaxos-1.1.3/sample```文件夹编译：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3/sample
# make
</pre>
这样就可以成功编译sample。

7） **编译src/ut**

进入*src/ut*目录编译单元测试程序：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3/src/ut
# make
</pre>

到此为止，所有相关目录都已经编译完成。


## 2. 测试phxpaxos
如下我们先简单的测试一下phxpaxos给出的三个示例程序。

### 2.1 sample/phxecho的测试
phxecho并不是一个真实的服务器，其只是展示了如何使用phxpaxos。

1） **创建日志目录**

在运行phxecho时，我们需要先创建一个名称为```log```的子目录，执行如下命令：
<pre>
# pwd
/root/paxos-inst/phxpaxos-1.1.3/sample/phxecho
# mkdir log
# cat run_echo.sh
</pre>


2) **启动phxecho**

接着我们开启3个命令行窗口来分别运行下面三条命令：
{% highlight string %}
# ./phxecho 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113     
# ./phxecho 127.0.0.1:11112 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 
# ./phxecho 127.0.0.1:11113 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
{% endhighlight %}

3） **简单测试phxecho**

在其中一个命令行窗口输入消息：
{% highlight string %}
# ./phxecho 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
run paxos ok
echo server start, ip 127.0.0.1 port 11111

please input: <echo req value>
hello,world
[SM Execute] ok, smid 1 instanceid 0 value hello,world
echo resp value hello,world

please input: <echo req value>
how are you?
[SM Execute] ok, smid 1 instanceid 1 value how are you?
echo resp value how are you?

please input: <echo req value>
{% endhighlight %}
上面可以看到phxecho采用Multi-Paxos来使三个节点达成一致性。

现在我们先关掉```11113```这一个节点所运行的phxecho，可以看到集群仍能够正常的工作。现在我们将```11112```节点也关掉，继续进行测试：
{% highlight string %}
please input: <echo req value>
good afternoon
{% endhighlight %}
此时发现，程序被卡住。而如果此时我们重启```11112```节点，则程序又可以正常的运行，达成一致性。

### 2.2 sample/phxelection的测试
本示例演示了如何使用paxos来进行master的选举。我们分别在三个命令行窗口执行如下命令：
<pre>
# ./phxelection 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
# ./phxelection 127.0.0.1:11112 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
# ./phxelection 127.0.0.1:11113 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
</pre>
运行后可以看到能够正常的进行master的选举，并通过version不断的续期。


### 2.3 sample/phxkv的测试
本示例演示了paxos是如何保存kv值的。请参看如下步骤：

1） **创建相关目录**

执行```prepare_dir.sh```脚本创建程序运行所需的相关目录：
<pre>
# chmod 777 prepare_dir.sh 
# ./prepare_dir.sh 
</pre>

2) **运行服务器程序**

分别在三个命令行窗口运行如下命令：
<pre>
#./phxkv_grpcserver 127.0.0.1:21111 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 ./storage/kvdb_0 ./storage/paxoslog_0
#./phxkv_grpcserver 127.0.0.1:21112 127.0.0.1:11112 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 ./storage/kvdb_1 ./storage/paxoslog_1
#./phxkv_grpcserver 127.0.0.1:21113 127.0.0.1:11113 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 ./storage/kvdb_2 ./storage/paxoslog_2
</pre>

3) **运行客户端程序**

在另一个命令行窗口执行如下命令：
<pre>
#./phxkv_client_tools 127.0.0.1:21112 put key_hello value_paxos 0
#./phxkv_client_tools 127.0.0.1:21112 getlocal key_hello
#./phxkv_client_tools 127.0.0.1:21112 getglobal key_hello
#./phxkv_client_tools 127.0.0.1:21112 delete key_hello 0

#./phxkv_client_tools 127.0.0.1:21111 put key_hello value_paxos 0
#./phxkv_client_tools 127.0.0.1:21111 getlocal key_hello
#./phxkv_client_tools 127.0.0.1:21111 getglobal key_hello
#./phxkv_client_tools 127.0.0.1:21111 delete key_hello 0

#./phxkv_client_tools 127.0.0.1:21113 put key_hello value_paxos 0
#./phxkv_client_tools 127.0.0.1:21113 getlocal key_hello
#./phxkv_client_tools 127.0.0.1:21113 getglobal key_hello
#./phxkv_client_tools 127.0.0.1:21113 delete key_hello 0
</pre>
可以看到每次调用```put```来设置或修改值的时候，对应kv的version就会增加1，后续再需要对value进行修改，所对应的的version必须要正确，否则将不能修改成功。其实就是演示了一个分布式乐观锁的概念。


### 2.4 src/ut的测试
本目录是对phxpaxos的一个单元性测试。参看如下：
<pre>
# ./phxpaxos_ut 
[==========] Running 27 tests from 6 test cases.
[----------] Global test environment set-up.
[----------] 7 tests from MultiDatabase
[ RUN      ] MultiDatabase.ClearAllLog
[       OK ] MultiDatabase.ClearAllLog (43 ms)
[ RUN      ] MultiDatabase.PUT_GET
[       OK ] MultiDatabase.PUT_GET (15 ms)
[ RUN      ] MultiDatabase.Del
[       OK ] MultiDatabase.Del (18 ms)
[ RUN      ] MultiDatabase.GetMaxInstanceID
[       OK ] MultiDatabase.GetMaxInstanceID (16 ms)
[ RUN      ] MultiDatabase.Set_Get_MinChosenInstanceID
[       OK ] MultiDatabase.Set_Get_MinChosenInstanceID (14 ms)
[ RUN      ] MultiDatabase.Set_Get_SystemVariables
[       OK ] MultiDatabase.Set_Get_SystemVariables (18 ms)
[ RUN      ] MultiDatabase.Set_Get_MasterVariables
[       OK ] MultiDatabase.Set_Get_MasterVariables (14 ms)
[----------] 7 tests from MultiDatabase (139 ms total)

[----------] 2 tests from NodeID
[ RUN      ] NodeID.IPPort2NodeID
[       OK ] NodeID.IPPort2NodeID (0 ms)
[ RUN      ] NodeID.NodeID2IPPort
[       OK ] NodeID.NodeID2IPPort (0 ms)
[----------] 2 tests from NodeID (0 ms total)

[----------] 2 tests from Timer
[ RUN      ] Timer.AddTimer
[       OK ] Timer.AddTimer (36 ms)
[ RUN      ] Timer.PopTimer
[       OK ] Timer.PopTimer (492 ms)
[----------] 2 tests from Timer (529 ms total)

[----------] 3 tests from WaitLock
[ RUN      ] WaitLock.Lock
[       OK ] WaitLock.Lock (62 ms)
[ RUN      ] WaitLock.LockTimeout
[       OK ] WaitLock.LockTimeout (31 ms)
[ RUN      ] WaitLock.TooMuchLockWating
[       OK ] WaitLock.TooMuchLockWating (55 ms)
[----------] 3 tests from WaitLock (148 ms total)

[----------] 6 tests from Acceptor
[ RUN      ] Acceptor.OnPrepare_Promise
[       OK ] Acceptor.OnPrepare_Promise (0 ms)
[ RUN      ] Acceptor.OnPrepare_Reject
[       OK ] Acceptor.OnPrepare_Reject (1 ms)
[ RUN      ] Acceptor.OnPrepare_PersistFail
[       OK ] Acceptor.OnPrepare_PersistFail (0 ms)
[ RUN      ] Acceptor.OnAccept_Pass
[       OK ] Acceptor.OnAccept_Pass (0 ms)
[ RUN      ] Acceptor.OnAccept_Reject
[       OK ] Acceptor.OnAccept_Reject (1 ms)
[ RUN      ] Acceptor.OnAccept_PersistFail
[       OK ] Acceptor.OnAccept_PersistFail (0 ms)
[----------] 6 tests from Acceptor (2 ms total)

[----------] 7 tests from Proposer
[ RUN      ] Proposer.NewValue
[       OK ] Proposer.NewValue (0 ms)
[ RUN      ] Proposer.OnPrepareReply_Skip
[       OK ] Proposer.OnPrepareReply_Skip (0 ms)
[ RUN      ] Proposer.OnPrepareReply_Pass
[       OK ] Proposer.OnPrepareReply_Pass (0 ms)
[ RUN      ] Proposer.OnPrepareReply_Reject
[       OK ] Proposer.OnPrepareReply_Reject (0 ms)
[ RUN      ] Proposer.OnAcceptReply_Skip
[       OK ] Proposer.OnAcceptReply_Skip (0 ms)
[ RUN      ] Proposer.OnAcceptReply_Reject
[       OK ] Proposer.OnAcceptReply_Reject (0 ms)
[ RUN      ] Proposer.OnAcceptReply_Pass
[       OK ] Proposer.OnAcceptReply_Pass (0 ms)
[----------] 7 tests from Proposer (0 ms total)

[----------] Global test environment tear-down
[==========] 27 tests from 6 test cases ran. (818 ms total)
[  PASSED  ] 27 tests.
</pre>




<br />
<br />
**参看：**

1. [phxpaxos](https://github.com/Tencent/phxpaxos/blob/master/README.zh_CN.md)

2. [Paxos从理论到实践](http://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)

<br />
<br />
<br />


