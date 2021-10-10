---
layout: post
title: LevelDB基础
tags:
- leveldb
categories: leveldb
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
<pre>
# cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .
</pre>

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
# tree
.
├── testdb1
│   ├── 000005.ldb
│   ├── 000006.log
│   ├── CURRENT
│   ├── LOCK
│   ├── LOG
│   ├── LOG.old
│   └── MANIFEST-000004
├── test_leveldb
├── test_leveldb.cpp
├── test_leveldb.i
├── test_leveldb.o
└── test_leveldb.s

1 directory, 12 files

{% endhighlight %}
上面可以看到在当前目录下生成了```testdb1```这样一个数据库，该数据库其实是一个目录结构。

## 3. LevelDB常用API操作

本节我们参考LevelDB官方文档，介绍一下LevelDB的常用API操作。

1） **Opening A Database**

一个leveldb数据库对应于文件系统上的一个目录。该数据库的所有内容都存储于对应目录中。下面的例子演示了如何打开一个数据库(假如数据库不存在，则会创建):
{% highlight string %}
#include <cassert>
#include "leveldb/db.h"

leveldb::DB* db;
leveldb::Options options;
options.create_if_missing = true;
leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
assert(status.ok());
...
{% endhighlight %}

假如你想要在数据库存在时产生相应错误，在执行```leveldb::DB::Open```之前设置如下选项：
<pre>
options.error_if_exists = true;
</pre>


2) **Status**

你可能已经注意到了上面的```leveldb::Status```数据类型。在LevelDB中，针对有可能报错的大多数函数都会返回该类型的值。我们可以通过检查对应的结果是否ok，从而判断函数的执行是否出错，如果出错则打印相关的错误信息：
{% highlight string %}
leveldb::Status s = ...;
if (!s.ok()) cerr << s.ToString() << endl;
{% endhighlight %}


3) **Closing A Database**

当我们在打开的数据库上完成了所有的操作，删除对应的db对象即完成了数据库的关闭。例如：
{% highlight string %}
... open the db as described above ...
... do something with db ...
delete db;
{% endhighlight %}


4) **Reads And Writes**

LevelDB数据库提供了Put、Delete、Get方法以实现数据库的修改和查询功能。例如，下面的代码实现将key1的值move到key2：
{% highlight string %}
std::string value;
leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
if (s.ok()) s = db->Put(leveldb::WriteOptions(), key2, value);
if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);
{% endhighlight %}

5) **Atomic Updates**

在上面```Reads And Writes```例子中，假如对应进程在执行完Put操作之后，但还没来得及执行Delete操作就崩溃了，则key1的value可能会存在于多个keys中(即key1和key2的value都相同）。如果要避免这种情况的发生，我们可以使用```WriteBatch```类来实现多个操作的原子更新：
{% highlight string %}
#include "leveldb/write_batch.h"
...
std::string value;
leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
if (s.ok()) {
  leveldb::WriteBatch batch;
  batch.Delete(key1);
  batch.Put(key2, value);
  s = db->Write(leveldb::WriteOptions(), &batch);
}
{% endhighlight %}
```WriteBatch```中按顺序维持了数据库的修改操作，在同一个batch中的修改操作会按顺序被应用到数据库。值得注意的是，在上面的代码中我们先Delete操作再调用Put操作，这样假如key1和key2相同的话，就不会错误的删除相应记录。

上面除了实现了原子操作，WriteBatch还可以用于将多个更新操作放入同一个batch中，从而达到加速批量更新的功能。

6) **Synchronous Writes**

默认情况下，LevelDB的每一个写操作都是异步写入的：将写入的数据由Leveldb进程推送到操作系统后就直接返回。数据从操作系统内存到底层的持久化存储之间是异步的。我们可以通过设置sync flag来执行同步写（在Posix系统上，在函数返回前是通过调用fsync(...)、或者fdatasync(...)或者msync(...,MS_SYNC)来实现的）：
{% highlight string %}
leveldb::WriteOptions write_options;
write_options.sync = true;
db->Put(write_options, ...);
{% endhighlight %}
```异步```写操作的效率通常是```同步```写操作的1000倍以上，异步写操作的一个缺点就是当主机崩溃时可能导致最后的少量更新数据丢失。需要指出的是，假如只是写进程崩溃的话，即使是异步写也不会丢失数据。

```WriteBatch```提供了另外一种方式的写操作。多个更新操作可以放入同一个WriteBatch，然后使用synchronous write将这些数据一起写入到数据库。这样，synchronous write操作的额外耗时将会被平摊到batch中的每一个更新上。

7） **Concurrency**

一个数据库在同一时间只能由一个进程打开。leveldb底层是通过从操作系统获取锁来防止多个进程同时打开的。在一个进程内，同一个```leveldb::DB```对象可以安全地被多个线程共享，例如不同的线程可以在不借助任何外部同步代码块地基础上，同时对同一个db对象执行write into、fetch iterators、Get操作。然而，其他对象(如Iterator、WriteBatch)则需要获得external synchronization。假如两个线程共享这样一个对象的话，则必须使用锁机制来对相应对象进行保护。

8） **Iteration**

下面的例子演示了如何打印数据库中所有的key，value对：
{% highlight string %}
leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
for (it->SeekToFirst(); it->Valid(); it->Next()) {
  cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
}
assert(it->status().ok());  // Check for any errors found during the scan
delete it;
{% endhighlight %}

下面的变体实现演示了如何处理指定范围[start, limit)内的keys：
{% highlight string %}
for (it->Seek(start);
   it->Valid() && it->key().ToString() < limit;
   it->Next()) {
  ...
}
{% endhighlight %}

此外，我们也可以反向遍历LevelDB的entries。（注：反向遍历会比正向遍历在某种程度上稍慢一些）
{% highlight string %}
for (it->SeekToLast(); it->Valid(); it->Prev()) {
  ...
}
{% endhighlight %}


9) **Snapshots**

Snapshots可以为key-value存储的整个状态提供一致的只读视图。ReadOptions::snapshot可以为非空(none-NULL)值，以表明在某一个特定版本的DB状态上执行读操作。假如ReadOptions::snapshot为NULL的话，则隐含是在当前状态上执行只读操作。

可以通过DB::GetSnapshot()方法来创建一个只读快照：
{% highlight string %}
leveldb::ReadOptions options;
options.snapshot = db->GetSnapshot();
... apply some updates to db ...
leveldb::Iterator* iter = db->NewIterator(options);
... read using iter to view the state when the snapshot was created ...
delete iter;
db->ReleaseSnapshot(options.snapshot);
{% endhighlight %}
需要指出的是，当snapshot不再需要的时候，应该使用DB::ReleaseSnapshot接口来对其进行释放。这样就可以释放维持该快照的一些状态信息。

10） **Slice**

在上面it->key()和it->value()的调用中，其返回值均为```leveldb::Slice```类型的实例。Slice是一个简单的数据结构，其包含一个长度字段，以及一个指向外部字节数组的指针。通过返回一个Slice比返回一个std::string会更高效，因为std::string可能需要拷贝潜在的large keys和values。另外，leveldb中的函数并不需要返回一个以```null```结尾的C风格的字符串，因为leveldb中的key/values本身是可以含有```\0```字节的。

C++类型的字符串和以```null```结尾的C风格字符串都可以很容易转换为Slice:
{% highlight string %}
leveldb::Slice s1 = "hello";

std::string str("world");
leveldb::Slice s2 = str;
{% endhighlight %}

此外，Slice也可以很容易的转换回C++字符串：
{% highlight string %}
std::string str = s1.ToString();
assert(str == std::string("hello"));
{% endhighlight %}

当使用Slice时需要特别小心，因为这需要调用者来确保在Slice使用期间指向的外部字节数组是一直有效的。例如，下面的代码就十分容易出问题：
{% highlight string %}
leveldb::Slice slice;
if (...) {
  std::string str = ...;
  slice = str;
}
Use(slice);
{% endhighlight %}
当不在if代码块作用范围时，str将会被销毁，此时slice所引用的存储空间就被释放了。

11） **Comparators**

在前面的例子中，使用的是默认的排序函数来对key进行排序，默认是按照字节词典顺序(bytes lexicographically)排列的。当打开数据库时，我们可以提供一个自定义的比较器(comparator)。比如，假设数据库中的keys包含有两个```整数```，首先应该按照第一个整数排序，如果相同再按第二个整数排序。首先我们定义一个子类来表达这些规则，该子类需要继承自```leveldb::Comparator```:
{% highlight string %}
class TwoPartComparator : public leveldb::Comparator {
 public:
  // Three-way comparison function:
  //   if a < b: negative result
  //   if a > b: positive result
  //   else: zero result
  int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
    int a1, a2, b1, b2;
    ParseKey(a, &a1, &a2);
    ParseKey(b, &b1, &b2);
    if (a1 < b1) return -1;
    if (a1 > b1) return +1;
    if (a2 < b2) return -1;
    if (a2 > b2) return +1;
    return 0;
  }

  // Ignore the following methods for now:
  const char* Name() const { return "TwoPartComparator"; }
  void FindShortestSeparator(std::string*, const leveldb::Slice&) const {}
  void FindShortSuccessor(std::string*) const {}
};
{% endhighlight %}

现在我们可以创建一个数据库，然后使用上述自定义comparator:
{% highlight string %}
TwoPartComparator cmp;
leveldb::DB* db;
leveldb::Options options;
options.create_if_missing = true;
options.comparator = &cmp;
leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
...
{% endhighlight %}

12) **Backwards compatibility**

当数据库被创建时，Comparator的Name()方法就与数据库绑定了，并且会在后续数据库每一次进行打开时进行检查。假如名称发生了改变，```leveldb::DB::Open```将会打开失败。因此，当且仅当新的key格式与比较函数与已存在的数据库不兼容，并且可以丢弃已存在数据库时才会修改该名称。



不过，您仍然可以通过一点预先规划，随着时间的推移逐渐改进key格式。比如，我们可以在key的末尾存储一个版本号(通常一个字节对大部分用户来说已经足够）。当我们想要切换到一个新的key格式的时候（TwoPartComparator中可以添加第三个可选部分来对key进行处理），a) 仍保持一致的comparator name; b) 怎加新的keys的版本号； c) 修改comparator函数，通过解析keys中的版本号来决定如何进行比较

## 4. Performance

可以通过调整定义在```include/options.h```中的默认值来对性能进行调优。

### 4.1 Block size

LevelDB将邻近的keys聚合在一起放入同一个block，并以block为传输单元来```存取```（包括读和写）数据。默认的block大小大概为4096未压缩字节。当应用程序在对数据库数据进行bulk扫描时，通常会希望增加block大小；而当应用程序需要读取大量位置的small values时，则可能倾向于将block size设置的更小，以提高效率。通常来说，block的大小小于1KB或者大于若干MB时，对性能其实是没什么帮助的。

另外有一点需要注意，当block size较大时，对数据的压缩其实是有益的。

### 4.2 Compression
在将数据写到持久化存储之前，每一个block的数据是单独被压缩的。默认情况下，压缩是开启的，因为压缩本身效率十分高，而对于不可压缩数据则会自动地关闭。在极少数情况下，应用程序可能想要完全禁止压缩，但在禁止之前最好做相应的性能测试，只有性能有明显地提高才将压缩关闭：
{% highlight string %}
leveldb::Options options;
options.compression = leveldb::kNoCompression;
... leveldb::DB::Open(options, name, ...) ....
{% endhighlight %}


### 4.3 Cache

LevelDB数据库中的数据是存放于文件系统上一系列文件中的，每一个文件按顺序存放着一些压缩blocks。假如options.block_cache为non-NULL的话，它会被用于缓存频繁使用的未压缩的块内容：
{% highlight string %}
#include "leveldb/cache.h"

leveldb::Options options;
options.block_cache = leveldb::NewLRUCache(100 * 1048576);  // 100MB cache
leveldb::DB* db;
leveldb::DB::Open(options, name, &db);
... use the db ...
delete db
delete options.block_cache;
{% endhighlight %}

需要注意的是，cache中保存的是```未压缩```的数据，因此需要根据应用程序级别的数据大小来设置block_cache的大小（对压缩的block的缓存是直接采用操作系统的buffer cache，或者是由客户端提供的其他自定义实现）。

当执行bulk read时，应用程序可能期望禁止cache，以便大容量读取处理的数据不会最终替换大部分缓存内容。我们可以使用一个per-iterator选项来实现：
{% highlight string %}
leveldb::ReadOptions options;
options.fill_cache = false;
leveldb::Iterator* it = db->NewIterator(options);
for (it->SeekToFirst(); it->Valid(); it->Next()) {
  ...
}
{% endhighlight %}

### 4.4 Key Layout

这里需要注意的是，disk transfer以及cache的单元均是block。相邻的key(根据数据库的排序规则）通常会放到同一个block中。因此应用程序可以通过将需要一起访问的keys放在相邻位置，从而提高访问性能，而将不一起访问的keys放到不同的区域。

例如，假设我们需要在leveldb上实现一个简易的文件系统，我们想要存储的条目的类型为：
{% highlight string %}
filename -> permission-bits, length, list of file_block_ids
file_block_id -> data
{% endhighlight %}
此时我们可能会期望将filename keys的前缀都设置为'/'，而将file_block_id keys的前缀设置为'0'，这样当我们仅仅只想扫描元数据的时候，就不会抓取并缓存到实际的文件内容。

### 4.5 Filters

由于LevelDB的数据是存放于磁盘上的，因此一个单独的Get()调用可能会涉及到多次磁盘读取操作。我们可以使用FilterPolicy选项机制来较大的减少磁盘的读取次数：
{% highlight string %}
leveldb::Options options;
options.filter_policy = NewBloomFilterPolicy(10);
leveldb::DB* db;
leveldb::DB::Open(options, "/tmp/testdb", &db);
... use the database ...
delete db;
delete options.filter_policy;
{% endhighlight %}

在上面的代码中，在打开LevelDB数据库时关联了一个基于Bloom filter的过滤策略。基于Bloom filter的过滤依赖于在内存中为每个key保留一定字节的数据（在上面的例子中，我们为每一个key保留了10bit，因为我们传递给布隆过滤器的参数为10）。通过该过滤器，在执行Get()操作时，可以大概减少100倍非必要的磁盘读取操作。通过增加每一个key的bit位数，可以更多的减少非必要的磁盘读取操作，但是会耗费更大的内存。我们建议如果应用程序需要进行大量的随机读，并且数据量与内存不匹配时，设置一个filter可以获得较好的性能。

假如你使用了一个自定义comparator，需要确保所使用的过滤策略与comparator兼容。例如，假设一个自定义的comparator在比较keys时会忽略末尾的空格，则在这种Comparator中不能使用NewBloomFilterPolicy。这种情况下，应用程序需要提供一个自定义的过滤策略以忽略末尾的空格。比如：
{% highlight string %}
class CustomFilterPolicy : public leveldb::FilterPolicy {
 private:
  FilterPolicy* builtin_policy_;

 public:
  CustomFilterPolicy() : builtin_policy_(NewBloomFilterPolicy(10)) {}
  ~CustomFilterPolicy() { delete builtin_policy_; }

  const char* Name() const { return "IgnoreTrailingSpacesFilter"; }

  void CreateFilter(const Slice* keys, int n, std::string* dst) const {
    // Use builtin bloom filter code after removing trailing spaces
    std::vector<Slice> trimmed(n);
    for (int i = 0; i < n; i++) {
      trimmed[i] = RemoveTrailingSpaces(keys[i]);
    }
    return builtin_policy_->CreateFilter(trimmed.data(), n, dst);
  }
};
{% endhighlight %}
更高级的应用程序可能会提供一个不使用布隆过滤器来实现的过滤策略，而是使用一些其他的机制来summarizing 一系列keys。请参考leveldb/filter_policy.h。

## 5. Checksums
LevelDB对存放于文件系统上的所有数据都会关联checksums。对这些校验和的验证力度有两种不同的控制：

* ReadOptions::verify_checksums

当设置为true时(可以在读取数据的过程中进行设置)，会对从文件系统上读取的数据强制进行checksum校验。。默认情况下，不会进行此种类型的校验。


* Options::paranoid_checks

当设置为true时(需要在打开数据库之前进行设置），LevelDB在检测到内部数据被损坏时就会报告相应的错误。取决于于数据库的那一部分被损坏，LevelDB可能会在数据库打开时就报错，或者在之后的数据库操作中报错。默认情况下，paranoid checking是被关闭的，这样LevelDB在部分持久化数据被损坏时仍让可以使用。


假如一个数据库被损坏(在paranoid checking打开时，可能数据库会打开失败）时，可以使用leveldb::RepairDB函数来尽可能的修复数据。

## 6. Approximate Sizes

可以使用GetApproximateSizes()函数来近似的获取一个或多个key区间所占用的磁盘存储空间。
{% highlight string %}
leveldb::Range ranges[2];
ranges[0] = leveldb::Range("a", "c");
ranges[1] = leveldb::Range("x", "z");
uint64_t sizes[2];
db->GetApproximateSizes(ranges, 2, sizes);
{% endhighlight %}

在上面的调用示例中，sizes[0]会保存range[a, c)范围内的keys数据所占用的磁盘存储空间；sizes[1]会保存range[x, z)范围内的keys数据所占用的磁盘存储空间。

## 7. Environment
LevelDB中所有的文件操作（也包括其他的操作系统调用）的实现都是通过一个leveldb::Env对象来传递的。复杂的客户端可能期望使用自己的Env实现来获得更好的控制。例如，一个应用程序可能期望在执行文件IO时引入人为的延迟，以限制leveldb对系统中其他活动的影响。
{% highlight string %}
class SlowEnv : public leveldb::Env {
  ... implementation of the Env interface ...
};

SlowEnv env;
leveldb::Options options;
options.env = &env;
Status s = leveldb::DB::Open(options, ...);
{% endhighlight %}

## 8. Porting
LevelDB可以被移植到其他新的平台，此时只需要实现```leveldb/port/port.h```文件中与平台相关的types/methods/functions即可。请参看```leveldb/port/port_example.h```以了解更多详细信息。

另外，新的platform也需要提供一个默认的新的leveldb::Env实现。请参看```leveldb/util/env_posix.h```示例。






<br />
<br />

**[参看]**

1. [Leveldb高效存储实现](https://stor.51cto.com/art/201903/593197.htm)

2. [LevelDB深入浅出之整体架构](https://zhuanlan.zhihu.com/p/67833030)

3. [leveldb源码阅读系列](https://zhuanlan.zhihu.com/p/80684560)

4. [LevelDB入门教程十篇](https://zhuanlan.zhihu.com/p/25349591)

5. [LevelDB github官网](https://github.com/google/leveldb)

6. [LevelDB官方文档](https://github.com/google/leveldb/blob/master/doc/index.md)

7. [LevelDb实现原理](https://blog.csdn.net/gdutliuyun827/article/details/70911342)

<br />
<br />
<br />

