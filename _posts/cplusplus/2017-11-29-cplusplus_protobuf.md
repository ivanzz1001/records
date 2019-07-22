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
## 1. protobuf简介
Protocol Buffers(也被称为protobuf)是Google开发的一种跨语言、跨平台、可扩展的用于序列化数据的机制，相比较XML和json格式而言，google protobuf更小、更快捷也更简单。你只需对想要的数据结构化定义一次，然后就可以用编译器(protoc)生成特定编程语言的代码来简单的实现对结构化数据的读写。当前protobuf支持多种语言：C++、Java、Python、C#、Go、Objective-C、JavaScript、Ruby、PHP、Dart。

### 1.1 protobuf是如何工作的？
在protocol buffer消息类型文件```.proto```中，我们可以定义需要实现序列化的结构化数据信息。每一条protocol buffer消息都是一个小的逻辑记录，包含了一系列的```name-value```键值对。如下是一个基础的```.proto```文件示例，定义了一个person对象包含的一些信息：
<pre>
message Person {
  required string name = 1;
  required int32 id = 2;
  optional string email = 3;

  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }

  message PhoneNumber {
    required string number = 1;
    optional PhoneType type = 2 [default = HOME];
  }

  repeated PhoneNumber phone = 4;
}
</pre>
如上所示，每一条消息类型都有一个或多个编号唯一的field，并且每一个field都具有名称和类型。其中类型可以是：

* numbers(integer or floating-point)

* booleans

* strings

* raw bytes

* protobuf message类型（这使得结构化数据可以具有层次性）

另外，你可以用```optional```、```required```、```repeated```等关键字来修饰某个field。更多关于```.proto```文件的信息我们后面会详细介绍。

一旦定义好了上述消息类型，就可以使用protobuf编译器(protoc)生成对应语言的数据访问类(class)，这些类会提供简单的方法来访问这些数据成员（如name()、set_name())，并且还会提供方法来将这种消息序列化成二进制数据，或者将二进制数据解析为消息类型。因此，假如你选择使用C++语言的话，对上述示例运行protoc编译器将会生成一个```Person```类，之后你就可以在应用程序中使用该类来分发、序列化以及获取```Person```消息。你可以编写类似如下的代码：
{% highlight string %}
Person person;
person.set_name("John Doe");
person.set_id(1234);
person.set_email("jdoe@example.com");
fstream output("myfile", ios::out | ios::binary);
person.SerializeToOstream(&output);
{% endhighlight %}
之后，你也可以将消息重新从文件中读回：
{% highlight string %}
fstream input("myfile", ios::in | ios::binary);
Person person;
person.ParseFromIstream(&input);
cout << "Name: " << person.name() << endl;
cout << "E-mail: " << person.email() << endl;
{% endhighlight %}
你可以向上述消息格式中添加新的field，但仍可保持消息的向下兼容性： 当解析到原先老的二进制数据时，会将后面新添加的field直接忽略。因此，假如你采用protobuf作为数据通信协议的话，你就可以很好的扩展协议，而不必担心会破坏原有的代码。

### 1.2 为什么不采用XML?

在序列化结构性数据时，protobuf对比XML具有很多优势：

* 更简单

* 比XML所产生的数据量小3至10倍

* 比XML快20到100倍

* 较少产生歧义性

* 所产生的数据访问类对象在编程中更容易使用

例如，假如我们想要表示一个具有name与email属性的Person对象，用XML的话需要如下：
{% highlight string %}
<person>
    <name>John Doe</name>
    <email>jdoe@example.com</email>
  </person>
{% endhighlight %}
而当用protobuf消息来传递时格式如下（注： 这里是protobuf消息的文本格式表示）：
{% highlight string %}
# Textual representation of a protocol buffer.
# This is *not* the binary format used on the wire.
person {
  name: "John Doe"
  email: "jdoe@example.com"
}
{% endhighlight %}
当这样一条消息被编码成protobuf二进制格式后，它也许只需要占用28字节并且只需花费100~200纳秒来进行解析。如果采用XML格式至少需要69个字节（假如把空格之类的全移除），并且需要花费5000~10000纳秒来进行解析。

另外，当我们要操纵protobuf时，也更为简便：
{% highlight string %}
 cout << "Name: " << person.name() << endl;
  cout << "E-mail: " << person.email() << endl;
{% endhighlight %}
而如果用XML的话，我们需要通过如下方式操作：
{% highlight string %}
 cout << "Name: "
       << person.getElementsByTagName("name")->item(0)->innerText()
       << endl;
  cout << "E-mail: "
       << person.getElementsByTagName("email")->item(0)->innerText()
       << endl;
{% endhighlight %}
值得指出的是，protobuf并不是在所有方面都比XML好，比如protobuf并不适合用来处理一些文本标记语言（HTML等），因为如果用protobuf的话你并不能很好的将文本与相应的结构进行分离。另外，XML具有更好的（人类）可读性与编辑性，而二进制protobuf通常不适合人类阅读。

### 1.3 proto3简介
当前protobuf的最新版本是版本3（也被称为proto3)，其提供了一些新的特性。proto3简化了protobuf语言，使其更容易使用，也使得其可被用于更多的编程语言：当前的proto3已经支持Java, C++, Python, Java Lite, Ruby, JavaScript, Objective-C, and C#。另外，你也可以使用go protoc编译器为Go语言生成proto3代码，请参看[golang/protobuf](https://github.com/golang/protobuf)

值得注意的是，protobuf的这两个版本(proto2、proto3)的API并不完全兼容。为了避免对现有用户造成不便，在新发布的protobuf版本中仍加持续支持proto2.

后续相关章节我们会更为详细的介绍proto3.

### 1.4 protobuf的一点历史
Google最先开发protobuf的时候主要是为了处理索引服务器(index server)请求与响应。在开发protobuf之前，我们可能通常需要编写类似如下的代码来处理对应的请求与响应：
{% highlight string %}
if (version == 3) {
   ...
 } else if (version > 4) {
   if (version == 5) {
     ...
   }
   ...
 }
{% endhighlight %}
同样如上面这种明确格式化的协议也使得要发布新的协议版本变得十分复杂，因为开发者必须要确定发出请求(request)的服务器与实际处理请求的服务器必须要双方都能理解新的协议版本。

protobuf就是被设计用来解决很多有关这方面问题的：

* 可以很容易的引入新的field，并且中间的服务器并不需要被另行通知即可解析所有的字段；

* protobuf数据格式具有很好的自我描述性，并且可以被更多的编程语言所处理

然而，假如仅仅只是这样的话，用户仍必须手写代码来解析相应的数据。因此，protobuf还提供了如下一些特性：

* 可以自动化产生序列化与反序列化代码，以避免手写代码来解析

* 为了支持短生命周期的RPC请求，在数据持久化存储方面人们也开始使用protobuf来作为一种方便的数据自我描述格式（如bigtable)

* 服务器RPC接口也作为protobuf文件的一部分被声明，然后通过protoc编译器产生相应的stub类，之后用户可以重写相应的方法来实现服务器接口


## 2. protobuf的安装
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

2) **下载安装包并解压**
<pre>
# mkdir protobuf-inst
# cd protobuf-inst
# wget https://github.com/protocolbuffers/protobuf/releases/download/v3.8.0/protobuf-all-3.8.0.tar.gz
# ls
protobuf-all-3.8.0.tar.gz
# tar -zxvf protobuf-all-3.8.0.tar.gz 
# cd protobuf-3.8.0
</pre>

3) **编译并安装protobuf**
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

# pkg-config --cflags protobuf
-pthread -I/usr/local/protobuf/include  
# pkg-config --libs protobuf
-L/usr/local/protobuf/lib -lprotobuf 
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

## 3. protobuf程序示例(C++版）
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
-L/usr/local/protobuf/lib -lprotobuf -lpthread -lstdc++

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
-L/usr/local/protobuf/lib -lprotobuf -lpthread -lstdc++

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

4. [protobuf developers](https://developers.google.com/protocol-buffers/)

<br />
<br />
<br />





