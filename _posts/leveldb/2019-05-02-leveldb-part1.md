---
layout: post
title: LevelDB基础
tags:
- ceph
categories: ceph
description: leveldb源代码分析
---


LevelDB是一个可持久化的快速(fast) KV存储引擎，实现了string keys到string values的有序映射(ordered mapping)，是由Google传奇工程师```Jeff Dean```和```Sanjay Ghemawat```开发并开源。



<!-- more -->

## 1. LevelDB简介
leveldb库实现了持久化的key/value存储功能。其中keys与values可以是任意的字节数组(byte arrays)。在LevelDB的KV存储系统中，keys是按照用户指定的比较函数(user-specified comparator function)顺序排列的。

1) **Features**

LevelDB具有如下特征：

* keys与values可以是任意的字节数组;

* 数据是按照key来有序存储的;

* 用户可以提供自定义比较函数(comparison function)来覆盖默认的排序规则;

* LevelDB的基本操作有：Put(key, value)、Get(key)、Delete(key);

* 可以在一个atomic batch中实现多数据的修改

* 用户可通过创建临时快照的方式获得数据的一致性视图

* 可对数据实现双向遍历(Forward and backward iteration)

* 使用[Snappy compression library](https://google.github.io/snappy/)来实现数据的自动压缩；

* 通过虚拟接口的方式实现与操作系统的交互

2) **Limitations**

* LevelDB并不是一个SQL数据库，其并不支持关系数据模型(relational data model)、也不支持SQL查询(SQL queries)、更不支持索引。

* 在同一时间内，只允许单进程(possibly multi-threaded)访问数据库

* LevelDB库不含内置的client-server支持。如果应用程序需要相应支持的话，需要通过包装(wrap)levelDB库以实现自己的server来实现

## 2. LevelDB的编译和使用

### 2.1 LevelDB的编译

1） **克隆LevelDB源代码**

首先执行如下命令克隆LevelDB源代码：
<pre>
# mkdir leveldb-inst && cd leveldb-inst
# git clone --recurse-submodules https://github.com/google/leveldb.git
# cd leveldb
# git submodule update --recursive

# ls
AUTHORS  benchmarks  cmake  CMakeLists.txt  CONTRIBUTING.md  db  doc  helpers  include  issues  LICENSE  NEWS  port  README.md  table  third_party  TODO  util
# ls third_party/
benchmark  googletest
</pre>


2) **编译levelDB**


查看```README.md```文件，在与leveldb同级目录创建leveldb-build目录：
<pre>
# mkdir leveldb-build 
# ls
leveldb  leveldb-build
# cd leveldb-build

# cmake -DCMAKE_BUILD_TYPE=Release ../leveldb 
CMake Error at CMakeLists.txt:5 (cmake_minimum_required):
  CMake 3.9 or higher is required.  You are running version 2.8.12.2


-- Configuring incomplete, errors occurred!
</pre>

上面我们看到编译LevelDB需要v3.9及以上版本的cmake，而我们当前的cmake版本为2.8.12.2，因此这里需要先写在cmake，然后重新安装指定版本：
{% highlight string %}
# cmake --version
cmake version 2.8.12.2

# yum remove cmake -y
Loaded plugins: fastestmirror, langpacks
Resolving Dependencies
--> Running transaction check
---> Package cmake.x86_64 0:2.8.12.2-2.el7 will be erased
--> Finished Dependency Resolution

Dependencies Resolved

===================================================================================================================================================================================================================
 Package                                         Arch                                             Version                                                    Repository                                       Size
===================================================================================================================================================================================================================
Removing:
 cmake                                           x86_64                                           2.8.12.2-2.el7                                             @base                                            27 M

Transaction Summary
===================================================================================================================================================================================================================
Remove  1 Package

Installed size: 27 M
Downloading packages:
Running transaction check
Running transaction test
Transaction test succeeded
Running transaction
  Erasing    : cmake-2.8.12.2-2.el7.x86_64                                                                                                                                                                     1/1 
  Verifying  : cmake-2.8.12.2-2.el7.x86_64                                                                                                                                                                     1/1 

Removed:
  cmake.x86_64 0:2.8.12.2-2.el7                                                                                                                                                                                    

Complete!
{% endhighlight %}

具体新版本cmake的安装请参看其他文档，安装后的新版本CMake如下:
<pre>
# cmake --version
cmake version 3.20.5

CMake suite maintained and supported by Kitware (kitware.com/cmake).
</pre>

然后再执行如下命令进行编译：
<pre>
# pwd
/root/leveldb-inst/leveldb-build

# cmake -DCMAKE_BUILD_TYPE=Release ../leveldb
-- The C compiler identification is GNU 4.8.5
-- The CXX compiler identification is unknown
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
CMake Error at CMakeLists.txt:7 (project):
  No CMAKE_CXX_COMPILER could be found.

  Tell CMake where to find the compiler by setting either the environment
  variable "CXX" or the CMake cache entry CMAKE_CXX_COMPILER to the full path
  to the compiler, or to the compiler name if it is in the PATH.


-- Configuring incomplete, errors occurred!
See also "/root/leveldb-inst/leveldb-build/CMakeFiles/CMakeOutput.log".
See also "/root/leveldb-inst/leveldb-build/CMakeFiles/CMakeError.log".
</pre>

上面提示需要```CMAKE_CXX_COMPILER```，这里我们只安装了```gcc 4.8.5```，需要执行如下命令安装```gcc-c++```:
<pre>
# yum install gcc-c++
# g++ --version
g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-44)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

另外可能还需要将```gcc 4.8.5```版本升级为7.3版本，执行如下命令：
<pre>
# yum install centos-release-scl
# yum install devtoolset-7-gcc*
# scl enable devtoolset-7 bash
# which gcc
/opt/rh/devtoolset-7/root/usr/bin/gcc
# gcc --version
gcc (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
Copyright (C) 2017 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

之后再执行：
<pre>
# cmake -DCMAKE_BUILD_TYPE=Release ../leveldb
# cmake --build .
# ls
arena_test        cache_test      cmake_install.cmake  c_test               db_test               filename_test      issue178_test  lib           Makefile            skiplist_test  version_edit_test
autocompact_test  cmake           coding_test          CTestTestfile.cmake  env_posix_test        filter_block_test  issue200_test  libleveldb.a  memenv_test         status_test    version_set_test
bin               CMakeCache.txt  corruption_test      db_bench             env_test              hash_test          issue320_test  logging_test  no_destructor_test  table_test     write_batch_test
bloom_test        CMakeFiles      crc32c_test          dbformat_test        fault_injection_test  include            leveldbutil    log_test      recovery_test       third_party
</pre>
上面我们看到生成了```libleveldb.a```静态链接库，

>注：如果要对代码进行gdb调试的话，需要修改编译类型
> # cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .


3) **levelDB安装**

上面我们编译完成了LevelDB，现在我们将其安装到'/opt/leveldb'目录下。在leveldb-build目录下执行如下：
<pre>
# mkdir -p /opt/leveldb
# cmake --install . --prefix="/opt/leveldb"
# ls /opt/leveldb/
include  lib64
# tree /opt/leveldb
/opt/leveldb
├── include
│   ├── benchmark
│   │   └── benchmark.h
│   ├── gmock
│   │   ├── gmock-actions.h
│   │   ├── gmock-cardinalities.h
│   │   ├── gmock-function-mocker.h
│   │   ├── gmock.h
│   │   ├── gmock-matchers.h
│   │   ├── gmock-more-actions.h
│   │   ├── gmock-more-matchers.h
│   │   ├── gmock-nice-strict.h
│   │   ├── gmock-spec-builders.h
│   │   └── internal
│   │       ├── custom
│   │       │   ├── gmock-generated-actions.h
│   │       │   ├── gmock-matchers.h
│   │       │   ├── gmock-port.h
│   │       │   └── README.md
│   │       ├── gmock-internal-utils.h
│   │       ├── gmock-port.h
│   │       └── gmock-pp.h
│   ├── gtest
│   │   ├── gtest-death-test.h
│   │   ├── gtest.h
│   │   ├── gtest-matchers.h
│   │   ├── gtest-message.h
│   │   ├── gtest-param-test.h
│   │   ├── gtest_pred_impl.h
│   │   ├── gtest-printers.h
│   │   ├── gtest_prod.h
│   │   ├── gtest-spi.h
│   │   ├── gtest-test-part.h
│   │   ├── gtest-typed-test.h
│   │   └── internal
│   │       ├── custom
│   │       │   ├── gtest.h
│   │       │   ├── gtest-port.h
│   │       │   ├── gtest-printers.h
│   │       │   └── README.md
│   │       ├── gtest-death-test-internal.h
│   │       ├── gtest-filepath.h
│   │       ├── gtest-internal.h
│   │       ├── gtest-param-util.h
│   │       ├── gtest-port-arch.h
│   │       ├── gtest-port.h
│   │       ├── gtest-string.h
│   │       └── gtest-type-util.h
│   └── leveldb
│       ├── cache.h
│       ├── c.h
│       ├── comparator.h
│       ├── db.h
│       ├── dumpfile.h
│       ├── env.h
│       ├── export.h
│       ├── filter_policy.h
│       ├── iterator.h
│       ├── options.h
│       ├── slice.h
│       ├── status.h
│       ├── table_builder.h
│       ├── table.h
│       └── write_batch.h
└── lib64
    ├── cmake
    │   ├── benchmark
    │   │   ├── benchmarkConfig.cmake
    │   │   ├── benchmarkConfigVersion.cmake
    │   │   ├── benchmarkTargets.cmake
    │   │   └── benchmarkTargets-release.cmake
    │   ├── GTest
    │   │   ├── GTestConfig.cmake
    │   │   ├── GTestConfigVersion.cmake
    │   │   ├── GTestTargets.cmake
    │   │   └── GTestTargets-release.cmake
    │   └── leveldb
    │       ├── leveldbConfig.cmake
    │       ├── leveldbConfigVersion.cmake
    │       ├── leveldbTargets.cmake
    │       └── leveldbTargets-release.cmake
    ├── libbenchmark.a
    ├── libbenchmark_main.a
    ├── libgmock.a
    ├── libgmock_main.a
    ├── libgtest.a
    ├── libgtest_main.a
    ├── libleveldb.a
    └── pkgconfig
        ├── benchmark.pc
        ├── gmock_main.pc
        ├── gmock.pc
        ├── gtest_main.pc
        └── gtest.pc

15 directories, 79 files
</pre>

### 2.2 LevelDB使用示例
在完成上面的编译安装步骤之后，我们生成了一个levelDB静态库和相应头文件，这里我们写一个测试代码进行测试。新建leveldb-test文件夹：
<pre>
# mkdir leveldb-test 
# ls
leveldb  leveldb-build  leveldb-test
# cd leveldb-test
# pwd
/root/leveldb-inst/leveldb-test
</pre>

编写如下测试代码(test_leveldb.cpp):
{% highlight string %}
#include <assert.h>
#include <iostream>
#include <sstream>
#include "leveldb/db.h"

using namespace std;

int main(int argc, char *argv[]){
	leveldb::DB* db; 
	leveldb::Options options;
	options.create_if_missing = true;

	//open a database
	leveldb::Status status = leveldb::DB::Open(options,"./testdb1",&db);
	int count = 0;


	while (count < 1000) {
		std::stringstream keys ;
		std::string key;
		std::string value = "shuningzhang@itworld123.com";

		keys << "itworld123-" << count;
		key = keys.str();
		status = db->Put(leveldb::WriteOptions(), key, value);    //add value
		assert(status.ok());

		status = db->Get(leveldb::ReadOptions(), key, &value);    //get value
		assert(status.ok());
		std::cout<<key<<" ==> "<<value<<std::endl;

		count ++; 
	}   

	//close the database 
	delete db;

	return 0;  
}
{% endhighlight %}
编译运行：
{% highlight string %}
# gcc -c test_leveldb.cpp -I /opt/leveldb/include
# gcc -o test_leveldb test_leveldb.o -L/opt/leveldb/lib64 -lleveldb -lstdc++ -lpthread
# ls
test_leveldb  test_leveldb.cpp  test_leveldb.o
# ./test_leveldb 
itworld123-0 ==> shuningzhang@itworld123.com
itworld123-1 ==> shuningzhang@itworld123.com
itworld123-2 ==> shuningzhang@itworld123.com
itworld123-3 ==> shuningzhang@itworld123.com
itworld123-4 ==> shuningzhang@itworld123.com
itworld123-5 ==> shuningzhang@itworld123.com
itworld123-6 ==> shuningzhang@itworld123.com
itworld123-7 ==> shuningzhang@itworld123.com
itworld123-8 ==> shuningzhang@itworld123.com
itworld123-9 ==> shuningzhang@itworld123.com
itworld123-10 ==> shuningzhang@itworld123.com
itworld123-11 ==> shuningzhang@itworld123.com
itworld123-12 ==> shuningzhang@itworld123.com
itworld123-13 ==> shuningzhang@itworld123.com
itworld123-14 ==> shuningzhang@itworld123.com

# ls
testdb1  test_leveldb  test_leveldb.cpp  test_leveldb.o
{% endhighlight %}
上面可以看到在当前目录下生成了```testdb1```这样一个数据库。





<br />
<br />

**[参看]**

1. [Leveldb高效存储实现](https://stor.51cto.com/art/201903/593197.htm)

2. [LevelDB深入浅出之整体架构](https://zhuanlan.zhihu.com/p/67833030)

3. [leveldb源码阅读系列](https://zhuanlan.zhihu.com/p/80684560)

4. [LevelDB入门教程十篇](https://zhuanlan.zhihu.com/p/25349591)

5. [LevelDB github官网](https://github.com/google/leveldb)

6. [LevelDB官方文档](https://github.com/google/leveldb/blob/master/doc/index.md)

7. [gcc编译、静态库与动态库](https://blog.csdn.net/daidaihema/article/details/80902012)

<br />
<br />
<br />

