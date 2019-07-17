---
layout: post
title: protobuf通信协议(proto2)
tags:
- cplusplus
categories: cplusplus
description: protobuf通信协议
---


本文主要讲述如何使用protobuf语言来结构化数据，这包括```.proto```文件语法，以及如何通过```.proto```文件产生相应的访问类(class)。本文内容是以proto2为基础来进行讲解的。

<pre>
通常我们建议使用proto3，因为其更容易使用，也支持更多的编程语言。
</pre>

<!-- more -->


## 1. 定义消息类型
首先我们来看一个很简单的示例，我们需要定义一个搜索请求的消息格式，给搜素请求具有一个query string，查询的起始页，以及每页显示的条目数。如下所示我们定义一个```.proto```文件：
{% highlight string %}
message SearchRequest {
  required string query = 1;
  optional int32 page_number = 2;
  optional int32 result_per_page = 3;
}
{% endhighlight %}
```SearchRequest```消息定义了三个字段。

1） **指定字段类型**

在上面的例子中，所有的字段都是```标量类型```(scalar types)： 两个数值类型字段(page_number、result_per_page)，一个字符串类型字段(query)。然而你也可以指定```复合类型```，这包括枚举类型(enumerations)以及其他message类型.

<pre>
标量类型(scalar types)是相对复合类型（Compound type）来说的：标量类型只能有一个值，而复合类型可以包含多个值。复合类型是由标量类型构成的
</pre>


2) **指定字段编号**

正如你所看到的，在消息定义中每一个字段都有一个唯一的编号。在protobuf二进制消息格式中这些```编号```被用于标识相应的字段，在消息类型被使用之后这些编号值是不能再进行修改的。在1~15范围内的字段采用1个字节来编码，这包括字段编号以及字段类型； 在16~2047范围内的字段采用2个字节来编码。因此为了尽量降低整个消息的长度，对那些频繁出现的消息元素，尽量使用1~15范围内的编号。另外，记得为消息的扩展保留一些1~15范围的编号，这样可使得后面扩展一些频繁使用的消息元素时不会占用太多的空间。

最小的字段编号是1，最大的编号字段是2^29-1(即536870911）。另外，我们禁止使用编号19000至19999范围内的编号（FieldDescriptor::kFirstReservedNumber 至 FieldDescriptor::kLastReservedNumber），这个范围内的编号被protobuf协议所保留。假如你在```.proto```文件中采用该范围内的字段，在protoc编译时会产生错误。同样也不能使用消息中明确被指定为保留的字段编号(通过```reserved```关键字）。


3） **指定字段时的规则**

你可以在指定消息字段时添加如下修饰符：

* required: 表明该字段在消息中必须存在该字段

* optional: 表明该字段在消息中可以出现0次或1次

* repeated: 表明该字段可以在消息中出现任意多次（包括0次，即不出现）。这些重复出现的值的顺序在协议中并未有强制约定

由于历史原因，对于```标量数值类型```(scalar numeric types)的repeated字段，其在编码时并不是那么高效。在新代码里我们添加一个额外的选项以获得更高效的编码，例如：
<pre>
repeated int32 samples = 4 [packed=true];
</pre>

4) **添加更多消息类型**

我们可以在一个```.proto```文件中定义多个消息类型。通常我们会把一些有```关联```的消息类型都放到同一个```.proto```文件中。例如：
{% highlight string %}
message SearchRequest {
  required string query = 1;
  optional int32 page_number = 2;
  optional int32 result_per_page = 3;
}

message SearchResponse {
 ...
}
{% endhighlight %}

5) **添加注释**

我们可以使用C/C++风格(//或者/* ... */)来为```.proto```文件添加注释：
{% highlight string %}
/* SearchRequest represents a search query, with pagination options to
 * indicate which results to include in the response. */

message SearchRequest {
  required string query = 1;
  optional int32 page_number = 2;  // Which page number do we want?
  optional int32 result_per_page = 3;  // Number of results to return per page.
}
{% endhighlight %}

6) **保留字段**

在有些情况下我们需要保留某些字段以及字段编号，我们可以通过如下方式：
{% highlight string %}
message Foo {
  reserved 2, 15, 9 to 11;
  reserved "foo", "bar";
}
{% endhighlight %}
注： 不能将保留的```字段名称```与```字段编号```写在同一行

7） **通过.proto会产生什么文件？**

当你对一个```.proto```文件运行protobuf编译器的时候，编译器会根据你所选定的语言产生特定代码，生成的代码里面通常会包含相应字段的get/set方法，将消息序列化到输出流的方法，以及从输入流中解析消息的方法：

* 对于C++语言，每一个```.proto```文件会产生一个```.h```与```.cc```文件，```.proto```文件中的每一条消息对应一个类

* 对于Java语言，```.proto```文件中的每一条消息都会产生一个```.java```文件，文件中包含该消息对应的class，以及一个```Builder```类用于创建消息实例

* 对于Python语言，python编译器会产生一个模块，在模块中针对```.proto```文件中每一个消息类型有一个static descriptor。

* 对于Go语言，编译器会产生一个```.pb.go```文件，文件中有相应的类型对应于```.proto```文件的每条消息

## 2. 标量值类型
标量消息类型字段可以是如下类型之一（下表展示了```.proto```文件中的类型与该类型在对应语言中的转换）：
<pre>
.proto类型               说明                          C++类型     Java类型   Python类型[2]       Go类型
------------------------------------------------------------------------------------------------------
double                                                double      double    float              float64
float                                                 float       float     float              float32
int32       采用变长编码。对于负数，编码效率较低。因此假如  int32       int       int                int32
            相应的字段会出现负值的话，建议采用sint32       

int64       采用变长编码。对于负数，编码效率较低。因此假如  int64       long      int/long[3]        int64
            相应的字段会出现负值的话，建议采用sint64 

uint32      采用变长编码                                uint32      int[1]    int/long[3]        uint32
uint64      采用变长编码                                uint64      long[1]   int/long[3]        uint64
sint32      采用变长编码                                int32       int       int                int32
sint64      采用变长编码                                int64       long      int/long[3]        int64

fixed32     总是采用4个字节长度来编码，假如值经常大于2^28   uint32      int[1]    int/long[3]        uint32
            情况下，比uint32高效
fixed64     总是采用8个字节长度来编码，假如值经常大于2^56   uint64      long[1]    int/long[3]        uint64
            情况下，比uint64高效

sfixed32    总是采用4个字节长度来编码                     int32       int       int                int32
sfixed64    总是采用8个字节长度来编码                     int64       long      int/long[3]        int64

bool                                                   bool        boolean   bool               bool
string      字符串必须是UTF8编码或7位的ASCII文本          string      String    unicode(python2)   string
                                                                             str(python3)
bytes       可以包含任何序列的字节                        string   ByteString   bytes              []byte
</pre>

对于上面类型后面中括号[]的说明如下：

**[1]**: 在Java中，usigned 32-bit与unsigned 64-bit整数是根据最高位是```0```还是```1```来进行区分的

**[2]**: 在所有情况下，当对某一个字段进行设置值的时候都会检查其是否有效

**[3]**: 64-bit或者unsigned 32-bit整数类型在解码时都会被解析成long类型，但是如果在设置值时传递的是int类型，那么其也可以是一个int

## 3. 可选字段以及默认值
正如上面所提到的，对于消息中的元素可以添加```optional```修饰符。对于一个```良好格式```(well-formed)的消息而言，假如在消息解析时并不包含该字段则会将该字段的值设为默认值。默认值可以在字段的后面进行说明，例如：
<pre>
optional int32 result_per_page = 3 [default = 10];
</pre>
假如可选字段并未明确指定默认值，则会采用类型特定的默认值。例如，对于字符串类型，该类型的默认值为空字符串。

## 4. 枚举
你可以使用```enum```关键字类定义枚举类型。如下所示：
{% highlight string %}
message SearchRequest {
  required string query = 1;
  optional int32 page_number = 2;
  optional int32 result_per_page = 3 [default = 10];
  enum Corpus {
    UNIVERSAL = 0;
    WEB = 1;
    IMAGES = 2;
    LOCAL = 3;
    NEWS = 4;
    PRODUCTS = 5;
    VIDEO = 6;
  }
  optional Corpus corpus = 4 [default = UNIVERSAL];
}
{% endhighlight %}
假如在一个枚举中，有两个不同的枚举常量其值相等（例如下面的```STARTED```与```RUNNING```)，那么需要设置```allow_alias```选项位true，否则proto编译器会报错：
{% highlight string %}
enum EnumAllowingAlias {
  option allow_alias = true;
  UNKNOWN = 0;
  STARTED = 1;
  RUNNING = 1;
}
enum EnumNotAllowingAlias {
  UNKNOWN = 0;
  STARTED = 1;
  // RUNNING = 1;  // Uncommenting this line will cause a compile error inside Google and a warning message outside.
}
{% endhighlight %}
另外枚举类型的值必须是一个32-bit整数所能表示的范围。因为enum类型时变长编码，负数通常编码效率较低，因此不建议采用负值。

你既可以在一个message定义中内嵌定义一个enum类型，也可以在message外部定义enum类型。如果我们在一个message内部定义enum类型，在另一个message中要引用该枚举类型的话，可用MessageType.EnumType方式。

### 4.1 枚举保留值
同样，我们为枚举保留某些name与值。例如：
{% highlight string %}
enum Foo {
  reserved 2, 15, 9 to 11, 40 to max;
  reserved "FOO", "BAR";
}
{% endhighlight %}

## 5. 使用其他消息类型
你可以使用其他消息类型来作为field的类型。例如：
{% highlight string %}
message SearchResponse {
  repeated Result result = 1;
}

message Result {
  required string url = 1;
  optional string title = 2;
  repeated string snippets = 3;
}
{% endhighlight %}

1) **导入定义**

在上面的例子中，Result消息类型与SearchResponse消息类型定义在同一个```.proto```文件中。但是如果定义在不同的```.proto```文件中，我们该如何处理呢？此时我们可以导入另一个```.proto```文件，语法如下：
<pre>
import "myproject/other_protos.proto";
</pre>
通常情况下，我们只能够使用直接导入的```.proto```文件中定义的消息类型，而不能递归。如果想要递归，需要使用```import public```。例如：
{% highlight string %}
// new.proto
// All definitions are moved here


// old.proto
// This is the proto that all clients are importing.
import public "new.proto";
import "other.proto";


// client.proto
import "old.proto";
// You use definitions from old.proto and new.proto, but not other.proto
{% endhighlight %}
在```client.proto```中我们只能使用```old.proto```与```new.proto```中定义的消息类型，而并不能使用```other.proto```中定义的消息类型。

protobuf编译器在编译```.proto```文件时会搜索```-I```或```--proto_path```选项指定的目录中的```.proto```文件。假如并未指定相关选项的话，则只会搜索protoc的当前运行目录。通常你应该将```--proto_path```设置为你当前项目的根目录。

2) **使用proto3消息类型**

我们可以在```proto2```消息中导入```proto3```消息类型，或者相反。但proto2中的枚举不能被用在proto3语法中。通常建议不要混合使用。

## 6. 内嵌类型



<br />
<br />

[参看]:

1. [Protobuf通信协议详解： 代码演示、详细原理介绍等](http://www.360doc.com/content/16/0907/15/478627_589080443.shtml)

2. [全方位评测：Protobuf性能到底有没有比JSON快5倍？](http://www.52im.net/forum.php?mod=viewthread&tid=772#lastpost)

3. [Protobuf github](https://github.com/google/protobuf/tree/master/src/google/protobuf)

<br />
<br />
<br />





