---
layout: post
title: phxpaxos的安装及使用
tags:
- paxos
categories: paxos
description: phxpaxos的安装及使用
---


phxpaxos是PhxPaxos是腾讯公司微信后台团队自主研发的一套基于Paxos协议的多机状态拷贝类库。如下我们简要的来说明一下phxpaxos的安装及使用。


<!-- more -->



## 1. phxpaxos的安装

## 1.1 编译环境
Linux。

GCC-4.8及以上版本。

## 1.2 编译前的第三方库准备
首先我们看一下各目录的依赖关系。如下：
|      目录         |       编译对象         |     内部依赖                       |           第三方库依赖    |
|:-----------------|:----------------------|:-----------------------------------|:------------------------|
|根目录             |  libphxpaxos.a        |      无                            | protobuf,leveldb        |           
|plugin            |  libphxpaxos_plugin.a | libphxpaxos.a	                    | glog                    |
|sample/phxelection|  可执行程序	           | libphxpaxos.a,libphxpaxos_plugin.a	| 无                      |
|sample/phxecho	   |  可执行程序	           | libphxpaxos.a,libphxpaxos_plugin.a	| 无                      |
|sample/phxkv	   |  可执行程序	           | libphxpaxos.a,libphxpaxos_plugin.a | grpc                    |
|src/ut	           |  单元测试	           | 无	                                | gtest,gmock             |


**1) 安装protobuf**

<pre>
# apt-cache search protobuf
# apt-get install libprotobuf-dev
</pre>

**2) 安装leveldb**
<pre>
# apt-cache search leveldb
# apt-get install libleveldb-dev
</pre>

**3) 安装glog**
<pre>
# apt-cache search glog
# apt-get install libgoogle-glog-dev
</pre>

**4) 安装grpc**
<pre>
# apt-cache search grpc
# apt-get install libgrpc-dev
</pre>

**5) 安装gtest,gmock**
<pre>
# git clone https://github.com/google/googletest.git
# cd googletest/
# ./travis.sh
# apt-get install cmake
# cmake ./CMakeLists.txt
# make
# sudo make install 
</pre>

参看：[ubuntu下 google gmock使用](http://blog.csdn.net/qq_26437925/article/details/68947498)

**6) 安装grpc**
<pre>
# git clone https://github.com/grpc/grpc.git grpc; cd grpc;

//注意此处需要修改boringssl-with-bazel的URL: https://github.com/1330018801/boringssl-with-bazel
# git submodule update --init
# make
# sudo make install prefix=/usr/local/ 
</pre>
参看：[ linux grpc+protobuff 安装](http://blog.csdn.net/u012023606/article/details/54584282)



**6) 安装phxpaxos**

到这里发现，其实不用通过上面的方式安装依赖库。
<pre>
#  git clone --recursive https://github.com/Tencent/phxpaxos.git
# cd phxpaxos/
# cat INSTALL

//third_party/leveldb目录
# make
# mkdir lib;cd lib;ln -s ../libleveldb.a libleveldb.a

//third_party目录
//download official v3.0.0 tar ball
# rm -rf protobuf
# wget https://github.com/google/protobuf/releases/download/v3.0.0/protobuf-cpp-3.0.0.tar.gz
# tar -zxvf protobuf-cpp-3.0.0.tar.gz; mv protobuf-3.0.0 protobuf
# cd protobuf
# ./autogen.sh
# ./configure CXXFLAGS=-fPIC --prefix=`pwd`
# make && make install



//third_party/glog目录
# ./configure CXXFLAGS=-fPIC --prefix=`pwd`
# make && make install




//编译libphxpaxos.a
//在PhxPaxos根目录下
# ./autoinstall.sh
# make
# make install

//编译libphxpaxos_plugin.a
//在plugin目录下
# make
# make install



//编译sample可执行程序(此处需要安装grpc,并把grpc库文件放置到third_party/grpc/lib目录)
# make sub_dir



//编译src/ut	测试程序
//这里首先需要参照上面“安装gtest,gmock”，然后再将对应lib库文件拷贝到third_party的相应目录中
//src/ut目录下
# make 
</pre>

## 2. 测试phxpaxos
执行如下命令简单测试一下phxpaxos:
<pre>
// sample/phxecho目录
# mkdir -p log
# cat run_echo.sh
# ./phxecho 127.0.0.1:11111 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
# ./phxecho 127.0.0.1:11112 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113
# ./phxecho 127.0.0.1:11113 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113


//测试性能,src/ut目录
# ./phxpaxos_ut 
[==========] Running 27 tests from 6 test cases.
[----------] Global test environment set-up.
[----------] 7 tests from MultiDatabase
[ RUN      ] MultiDatabase.ClearAllLog
[       OK ] MultiDatabase.ClearAllLog (255 ms)
[ RUN      ] MultiDatabase.PUT_GET
[       OK ] MultiDatabase.PUT_GET (162 ms)
[ RUN      ] MultiDatabase.Del
[       OK ] MultiDatabase.Del (206 ms)
[ RUN      ] MultiDatabase.GetMaxInstanceID
[       OK ] MultiDatabase.GetMaxInstanceID (192 ms)
[ RUN      ] MultiDatabase.Set_Get_MinChosenInstanceID
[       OK ] MultiDatabase.Set_Get_MinChosenInstanceID (163 ms)
[ RUN      ] MultiDatabase.Set_Get_SystemVariables
[       OK ] MultiDatabase.Set_Get_SystemVariables (182 ms)
[ RUN      ] MultiDatabase.Set_Get_MasterVariables
[       OK ] MultiDatabase.Set_Get_MasterVariables (170 ms)
[----------] 7 tests from MultiDatabase (1337 ms total)

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
[       OK ] Timer.PopTimer (479 ms)
[----------] 2 tests from Timer (517 ms total)

[----------] 3 tests from WaitLock
[ RUN      ] WaitLock.Lock
[       OK ] WaitLock.Lock (62 ms)
[ RUN      ] WaitLock.LockTimeout
[       OK ] WaitLock.LockTimeout (32 ms)
[ RUN      ] WaitLock.TooMuchLockWating
[       OK ] WaitLock.TooMuchLockWating (28 ms)
[----------] 3 tests from WaitLock (125 ms total)

[----------] 6 tests from Acceptor
[ RUN      ] Acceptor.OnPrepare_Promise
[       OK ] Acceptor.OnPrepare_Promise (1 ms)
[ RUN      ] Acceptor.OnPrepare_Reject
[       OK ] Acceptor.OnPrepare_Reject (1 ms)
[ RUN      ] Acceptor.OnPrepare_PersistFail
[       OK ] Acceptor.OnPrepare_PersistFail (0 ms)
[ RUN      ] Acceptor.OnAccept_Pass
[       OK ] Acceptor.OnAccept_Pass (0 ms)
[ RUN      ] Acceptor.OnAccept_Reject
[       OK ] Acceptor.OnAccept_Reject (0 ms)
[ RUN      ] Acceptor.OnAccept_PersistFail
[       OK ] Acceptor.OnAccept_PersistFail (1 ms)
[----------] 6 tests from Acceptor (4 ms total)

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
[       OK ] Proposer.OnAcceptReply_Reject (3 ms)
[ RUN      ] Proposer.OnAcceptReply_Pass
[       OK ] Proposer.OnAcceptReply_Pass (0 ms)
[----------] 7 tests from Proposer (6 ms total)

[----------] Global test environment tear-down
[==========] 27 tests from 6 test cases ran. (1994 ms total)
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


