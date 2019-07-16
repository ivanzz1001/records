---
layout: post
title: protobuf通信协议
tags:
- cplusplus
categories: cplusplus
description: protobuf通信协议
---

本章简单介绍一下protobuf通信协议。


<!-- more -->

## 1. protobuf的安装
1）**准备必要安装环境**

在Centos下通过源代码安装protobuf，通常要求系统已经安装有如下一些工具：

* autoconf

* automake

* libtool

* make

* g++

* unzip

我们可以执行如下命令来检查相关的工具是否安装：
<pre>
# yum list installed | grep autoconf
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
autoconf.noarch                        2.69-11.el7                     @base 
</pre>
或者直接运行相应的命令，看是否可以执行：
<pre>
# autoconf -v
autoconf: error: no input file
</pre>

1) **下载安装包并解压**
<pre>
# mkdir protobuf-inst
# cd protobuf-inst
# wget https://github.com/protocolbuffers/protobuf/releases/download/v3.8.0/protobuf-all-3.8.0.tar.gz
# ls
protobuf-all-3.8.0.tar.gz
# tar -zxvf protobuf-all-3.8.0.tar.gz 
# cd protobuf-3.8.0
</pre>

2) **编译并安装protobuf**
<pre>
# ./configure --prefix=/usr/local/protobuf
# make
# make check
PASS: protobuf-test
PASS: protobuf-lazy-descriptor-test
PASS: protobuf-lite-test
PASS: google/protobuf/compiler/zip_output_unittest.sh
PASS: google/protobuf/io/gzip_stream_unittest.sh
PASS: protobuf-lite-arena-test
PASS: no-warning-test
============================================================================
Testsuite summary for Protocol Buffers 3.8.0
============================================================================
# TOTAL: 7
# PASS:  7
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
============================================================================

# make install
# ls /usr/local/protobuf/
bin  include  lib
</pre>
至此，安装已经完成。

如果我们想以后使用方便，可以导出如下环境变量到```/etc/profile```中(记得执行*source /etc/profile*)：
<pre>
export PATH=$PATH:/usr/local/protobuf/bin/
export PKG_CONFIG_PATH=/usr/local/protobuf/lib/pkgconfig/
</pre>
另外如果想要我们以后的程序在运行时可以直接找到protobuf相关的动态库，也可以在protobuf库路径添加到*/etc/ld.so.conf*中。这里我们可以如下操作：
{% highlight string %}
# echo "/usr/local/protobuf/lib" >> /etc/ld.so.conf.d/protobuf-x86_64.conf
# ls /etc/ld.so.conf.d
dyninst-x86_64.conf  kernel-3.10.0-514.el7.x86_64.conf  libiscsi-x86_64.conf  mysql-x86_64.conf  protobuf-x86_64.conf
{% endhighlight %}

```注：``` 这里我们为了较为详细的演示protobuf的相关编译、运行步骤，暂时都未设置这些环境变量

## 2. protobuf程序示例(C++版）
该程序的大致功能是： 定义一个```Person```结构体，以及存放Person信息的```AddressBook```，然后一个写程序向一个文件写入该结构体信息，另一个程序从文件中读出该信息并打印到输出中.

1) **编写address.proto文件**

这里我们采用```proto2```语法格式：
{% highlight string %}
syntax = "proto2";
package tutorial;

message Person {
    required string name = 1;
    required int32 age = 2;
}

message AddressBook {
    repeated Person person = 1;
}
{% endhighlight %}
执行如下命令对```address.proto```文件进行编译：
<pre>
# /usr/local/protobuf/bin/protoc --cpp_out=./ ./address.proto
# ls
address.pb.cc  address.pb.h  address.proto
</pre>
之后我们可以大体看一下生成的这两个文件，其结构大体如下：
{% highlight string %}
//address.pb.h
namespace tutorial{
class Person{
};

class AddressBook{
};

}
{% endhighlight %}
其会为Person以及AddressBook生成很多方法。

2） **编写addressbook_write.cpp，向文件中写入AddressBook信息**

这里我们编写addressbook_write.cpp，然后以二进制方式向文件中写入AddressBook信息：
{% highlight string %}
#include <iostream>
#include <fstream>
#include <string>
#include "address.pb.h"


void PromptForAddress(tutorial::Person *person){

        std::string name;
        int age;

        std::cout<<"Enter person name: ";
        std::cin >> name;
        person->set_name(name);

        std::cout<<"Enter person age: ";
        std::cin >> age;
        person->set_age(age);
}

int main(int arc, char *argv[])
{
        if (arc != 2){
                std::cerr << "Usage: addressbook_write <bookfile>" <<  std::endl;
                return -1;
        }

        tutorial::AddressBook address_book;

        PromptForAddress(address_book.add_person());

        //write to file
        {
                std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
                if (!address_book.SerializeToOstream(&output)){
                        std::cerr << "Failed to write address book." << std::endl;
                        return -1;
                }
        }

        // Optional: Delete all global objects allocated by libprotobuf.
        //google::protobuf::ShutdownProtobufLibrary();

        return 0x0;
}
{% endhighlight %}
执行如下命令进行编译(注： 这里需要加上```-std=c++11```)：
<pre>
# gcc -o addressbook_write address.pb.cc addressbook_write.cpp -std=c++11 -I/usr/local/protobuf/include \
-L/usr/local/protobuf/lib -lprotobuf -lpthread -lz -lstdc++

或者
# export PKG_CONFIG_PATH=/usr/local/protobuf/lib/pkgconfig/
# g++ -o addressbook_write address.pb.cc addressbook_write.cpp -std=c++11 `pkg-config --cflags --libs protobuf` 
# ls
addressbook_write  addressbook_write.cpp  address.pb.cc  address.pb.h  address.proto
</pre>
这里建议采用上面第2种方法编译，比较不容易出错，否则后续可能遇到各种奇怪的问题。编译成功之后运行：
<pre>
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/protobuf/lib
# ./addressbook_write addressbook.bin
Enter person name: ivanzz1001
Enter person age: 18
# ls
addressbook.bin  addressbook_write  addressbook_write.cpp  address.pb.cc  address.pb.h  address.proto
</pre>

3） **编写addressbook_read.cpp，从文件中读入AddressBook信息**

这里我们编写addressbook_read.cpp，然后从二进制方式向文件中读入AddressBook信息：
{% highlight string %}
#include <iostream>
#include <fstream>
#include <string>
#include "address.pb.h"


void ListPerson(const tutorial::AddressBook& address_book){
	for(int i = 0; i < address_book.person_size(); i++){
		const tutorial::Person &person = address_book.person(i);
		
		std::cout<<person.name() << "  " << person.age() << std::endl;
	}
}


int main(int argc, char *argv[])
{
        //GOOGLE_PROTOBUF_VERIFY_VERSION;

        if (argc != 2){
                std::cerr << "Usage: addressbook_write <bookfile>" << std::endl;
                return -1;
        }

        tutorial::AddressBook address_book;

        //read from file
        {
                std::fstream input(argv[1], std::ios::in | std::ios::binary);
                if (!address_book.ParseFromIstream(&input)){
                        std::cerr << "Failed to parse addressbook." << std::endl;
                        return -1;
                }
        }

        ListPerson(address_book);


    // Optional: Delete all global objects allocated by libprotobuf.
    //google::protobuf::ShutdownProtobufLibrary();

        return 0;
}
{% endhighlight %}
执行如下命令进行编译(注： 这里需要加上```-std=c++11```)：
<pre>
# gcc -o addressbook_read address.pb.cc addressbook_read.cpp -std=c++11 -I/usr/local/protobuf/include \
-L/usr/local/protobuf/lib -lprotobuf -lpthread -lz -lstdc++

或者
# export PKG_CONFIG_PATH=/usr/local/protobuf/lib/pkgconfig/
# g++ -o addressbook_read address.pb.cc addressbook_read.cpp -std=c++11 `pkg-config --cflags --libs protobuf` 
# ls
addressbook.bin  addressbook_read  addressbook_read.cpp  addressbook_write  addressbook_write.cpp  address.pb.cc  address.pb.h  address.proto
</pre>
这里建议采用上面第2种方法编译，比较不容易出错，否则后续可能遇到各种奇怪的问题。编译成功之后运行：
<pre>
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/protobuf/lib
# ./addressbook_read addressbook.bin
ivanzz1001  18
</pre>

这里我们可以看到已经成功的读取出了前面写入的信息。

<br />
<br />

[参看]:

1. [Protobuf通信协议详解： 代码演示、详细原理介绍等](http://www.360doc.com/content/16/0907/15/478627_589080443.shtml)

2. [全方位评测：Protobuf性能到底有没有比JSON快5倍？](http://www.52im.net/forum.php?mod=viewthread&tid=772#lastpost)

3. [Protobuf github](https://github.com/google/protobuf/tree/master/src/google/protobuf)

<br />
<br />
<br />





