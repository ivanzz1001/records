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
你可以在一个message类型中定义和使用其他message类型。如下示例，我们在SearchResponse中定义了Result消息类型：
{% highlight string %}
message SearchResponse {
  message Result {
    required string url = 1;
    optional string title = 2;
    repeated string snippets = 3;
  }
  repeated Result result = 1;
}
{% endhighlight %}
假如你想要在```SearchResponse```外边复用```Result```消息类型的话，你可以使用```Parent.Type```来引用。例如：
{% highlight string %}
message SomeOtherMessage {
  optional SearchResponse.Result result = 1;
}
{% endhighlight %}
目前来说，内嵌类型的深度暂时没有限制（但不建议有太深的内嵌）：
{% highlight string %}
message Outer {                  // Level 0
  message MiddleAA {  // Level 1
    message Inner {   // Level 2
      required int64 ival = 1;
      optional bool  booly = 2;
    }
  }
  message MiddleBB {  // Level 1
    message Inner {   // Level 2
      required int32 ival = 1;
      optional bool  booly = 2;
    }
  }
}
{% endhighlight %}

### 6.1 Groups
Groups是在消息中定义内嵌类型的另一种方式，例如：
{% highlight string %}
message SearchResponse {
  repeated group Result = 1 {
    required string url = 2;
    optional string title = 3;
    repeated string snippets = 4;
  }
}
{% endhighlight %}
上面SearchResponse包含了一个Result列表。

```说明```： 不要使用Groups这种方式来定义内嵌类型，该方式已经过时。使用前面介绍的第一种方式。

## 7. 更新消息类型
 
假如我们当前的消息类型已经不能很好的满足需求的时候（例如我们想要在message中添加一个新的field），在不破坏向下兼容的情况下，我们可以很简单的更新Message类型。但是请注意，需要遵循如下规则：

* 不要修改现存字段(field)的编号

* 添加一个新的字段(field)，并将字段设为optional或者repeated，同时为该新字段设置合适的默认值。这意味着我们的新代码仍可识别该消息的```旧格式```，同时老代码也仍能识别消息的```新格式```（其会简单的忽略该新添加的未知字段）。

* 可以将```Non-required```字段简单的移除。但是移除时，可能需要将该字段所占用的```字段编号```保留，以免后续新添加的字段用到该字段编号（使用```reserved```来保留字段编号）

* 将```none-required```字段设置为extensions

* int32, uint32, int64, uint64,bool 之间是兼容的

* sint32 与 sint64 之间是兼容的

* string 与 bytes之间是兼容的

* fixed32 与 sfixed32之间是兼容的，fixed64 与 sfixed64之间是兼容的

* optional与repeated之间也是兼容的

* 通常来说修改默认值是允许的

* enum与int32, uint32, int64,uint64是兼容的

## 8. Extensions
```Extensions```允许你在消息中声明某一个范围内字段编号是用作第三方扩展的。其实```扩展```就是为一个字段(filed)预留的恶一个占位符，后续就可以在其他的```.proto```文件中添加新的字段到该占位符中。例如：
{% highlight string %}
message Foo {
  // ...
  extensions 100 to 199;
}
{% endhighlight %}
上面显示[100,199]范围内的字段编号被保留用于以后的扩展。因此其他用户可以在自己的```.proto```文件中先导入上述```.proto```文件，然后再使用该保留的字段编号范围来对```Foo```消息进行扩展。例如：
{% highlight string %}
extend Foo {
  optional int32 bar = 126;
}
{% endhighlight %}
我们添加了一个```bar```字段到原来的```Foo```消息中，其中字段编号为126.

当Foo消息被序列化时，扩展的bar字段会被正常的序列化进去。然而我们```访问扩展字段```与```访问普通字段```有些不同，我们有专门的方法来访问扩展字段。如下是C++语言中访问扩展字段的方法：
{% highlight string %}
Foo foo;
foo.SetExtension(bar, 15);
{% endhighlight %}

相似的，```Foo```类还还提供了如下的一些模板方法来访问扩展字段：
<pre>
HasExtension()
ClearExtension()
GetExtension()
MutableExtension()
AddExtension()
</pre>
值得注意的是，扩展字段可以是任何类型，包括message类型，但是不能是```oneof```或者```map```类型


### 8.1 内嵌扩展
我们可以在另一个messageB中对messageA进行扩展，例如：
{% highlight string %}
message Baz {
  extend Foo {
    optional int32 bar = 126;
  }
  ...
}
{% endhighlight %}
在这种情况下，如果我们要通过C++来访问扩展的话，可以通过如下方式：
{% highlight string %}
Foo foo;
foo.SetExtension(Baz::bar, 15);
{% endhighlight %}

注： 我们通常不建议使用此种方式，这里也不对此种使用方式做更多说明。

### 8.2 选择扩展字段编号
在扩展字段时，很重要的一点是两个不同的用户不会使用相同的```字段编号```来对消息进行扩展（这会导致数据被损坏）。假如你向保留的扩展字段编号的范围很大的话，可以采用如下这种方式：
{% highlight string %}
message Foo {
  extensions 1000 to max;
}
{% endhighlight %}
这里max为2^29-1，即 536,870,911。

同时我们应该注意不要选在[19000,19999]范围内的字段编号，该范围内字段编号通常为系统保留。其中19000可以用FieldDescriptor::kFirstReservedNumber来引用；19999可以用FieldDescriptor::kLastReservedNumber来引用。

## 9. Oneof
假如在一个message中有很多字段(field)是```optional```的，并且这些字段在同一时间内至多只有一个会被设置，我们可以使用```Oneof```特征来节省内存空间。

oneof中的字段类似于```optional```字段，只不过oneof中的所有字段都共享相同的内存空间（类似于C语言中的union)，并且oneof在同一时间内最多只有一个字段会被设置。设置oneof中的任何一个字段都会清除其他字段的值。我们可以根据自己所选定的编程语言通过case()或者WhichOneof()方法来检查到底哪一个字段被设置了。

### 9.1 Oneof的使用
如果要在```.proto```文件中定义一个```oneof```，那么可以使用```oneof```关键字后面跟随对应的名称即可。例如：
{% highlight string %}
message SampleMessage {
  oneof test_oneof {
     string name = 4;
     SubMessage sub_message = 9;
  }
}
{% endhighlight %}
我们可以在```oneof```中添加任何字段，但是不能使用```required```、```optional```、```repeated```关键词来修饰。假如我们要添加一个```repeated```字段到oneof中，这是禁止的，此时我们可以直接在message中添加该repeated字段。

在通过```protoc```编译生成的代码中，```oneof```中的每一个字段同样会生成对应的getter、settter方法。

### 9.2 Oneof特征
* 设置oneof中的某一个字段会自动的清空oneof中的其他字段的值（因为它们共享存储空间），因此假如你你设定多个oneof字段的值，那么之后最后设置的那个字段值有效
{% highlight string %}
SampleMessage message;
message.set_name("name");
CHECK(message.has_name());
message.mutable_sub_message();   // Will clear name field.
CHECK(!message.has_name());
{% endhighlight %}

* 假如解析器遇到多个oneof中的字段，那么只有遇到的最后一个oneof字段有效

* 扩展字段并不允许oneof

* oneof中的字段并不能是repeated的

* 假如你将某个oneof字段设置有默认值，那么该字段是会被序列化的，并且针对该字段调用```case```,会返回```字段被设置```

* 假如你采用C++的话，请确保代码不会造成内存被破坏。如下的示例代码会导致程序崩溃，因为调用set_name()时会将sub_message分配的内存给删除
{% highlight string %}
SampleMessage message;
SubMessage* sub_message = message.mutable_sub_message();
message.set_name("name");      // Will delete sub_message
sub_message->set_...            // Crashes here
{% endhighlight %}

* 假如你采用C++，并且使用Swap()来交换两个带oneof的message，则交换之后每个message都带有对方的一个oneof字段
{% highlight string %}
SampleMessage msg1;
msg1.set_name("name");
SampleMessage msg2;
msg2.mutable_sub_message();
msg1.swap(&msg2);
CHECK(msg1.has_sub_message());
CHECK(msg2.has_name());
{% endhighlight %}

### 9.3 Oneof兼容性问题
在添加或移除oneof中的字段时要特别小心。假如检查某个oneof的值返回None或NOT_SET的话，可能对应两种情况：

* 该oneof并未被设置

* 设置了另一个不同版本的oneof(比如我们在oneof的第一个版本中有2个字段，在第二个版本中新添加了一个字段后变成了3个字段）

## 10. Maps
假如你想要定义一个map的话，可以通过如下方式：
{% highlight string %}
map<key_type, value_type> map_field = N;
{% endhighlight %}
这里```key_type```可以是```整数类型```或```字符串类型```(任何scalar类型都可以，除浮点类型与bytes类型外）。另外，enum类型不能作为key_type。value_type可以是除map类型外的任何类型。

例如：
{% highlight string %}
map<string, Project> projects = 3;
{% endhighlight %}

1) **Map的特征**

* map并不支持Extensions

* map并不能使用repeated、optional、required等关键词修饰

* 序列化或遍历map中的元素时并没有特定的顺序


2） **向下兼容性**

map语法定义的数据在网络上传输时等价于如下格式：
{% highlight string %}
message MapFieldEntry {
  optional key_type key = 1;
  optional value_type value = 2;
}

repeated MapFieldEntry map_field = N;
{% endhighlight %}
因此即使protobuf的实现并不支持map的话，其仍然可以处理对应的数据。

## 11. Packages
我们可以为某个```.proto```文件添加一个```package```(可选）以防止消息类型的冲突（类似于namespace的概念）：
{% highlight string %}
package foo.bar;
message Open { ... }
{% endhighlight %}
当我们在定义自己的message类型时可通过如下的方式来引用别的package中的类型：
{% highlight string %}
message Foo {
  ...
  required foo.bar.Open open = 1;
  ...
}
{% endhighlight %}

再用protoc编译器产生对应的代码时，会根据我们选定的语言对```package```做不同的处理：

* C++语言中会产生对应的c++ namespace。比如上面```Open```将会在foo::bar名称空间中

* Java语言会产生对应的Java package，

* Python语言会忽略该```package```指令

* Go语言会忽略该```package```指令

通常情况下我们建议使用```package```以防止名称冲突。

## 12. 定义Service
假如你想要在一个RPC系统中使用message类型，你可以在```.proto```文件中定义RPC服务接口(interface)，之后protobuf编译器就会产生对应的服务接口代码和stubs。比如，我们想定义一个RPC服务，其有一个方法Search，参数为```SearchRequest```，返回结果为```SearchResponse```，那么可以如下定义```.proto```文件：
{% highlight string %}
service SearchService {
  rpc Search (SearchRequest) returns (SearchResponse);
}
{% endhighlight %}
默认情况下，protobuf编译器会产生一个抽象接口```SearchService```和一个具体的```stub```实现（类似于EJB的skeleton与stub概念）。stub会把所有的请求发送到```RpcChannel```，之后由我们对RpcChannel的具体实现来真正发送出去。比如，我们有一个RpcChannel其实现了序列化消息，然后把该序列化后的消息通过HTTP发送到服务器。因此，对于C++来说其可能产生类似如下的代码：
{% highlight string %}
using google::protobuf;

protobuf::RpcChannel* channel;
protobuf::RpcController* controller;
SearchService* service;
SearchRequest request;
SearchResponse response;

void DoSearch() {
  // You provide classes MyRpcChannel and MyRpcController, which implement
  // the abstract interfaces protobuf::RpcChannel and protobuf::RpcController.
  channel = new MyRpcChannel("somehost.example.com:1234");
  controller = new MyRpcController;

  // The protocol compiler generates the SearchService class based on the
  // definition given above.
  service = new SearchService::Stub(channel);

  // Set up the request.
  request.set_query("protocol buffers");

  // Execute the RPC.
  service->Search(controller, request, response, protobuf::NewCallback(&Done));
}

void Done() {
  delete service;
  delete channel;
  delete controller;
}
{% endhighlight %}
如上所示，所有的service类都实现了```Service```接口。


另外，在服务器一端，这可以用来实现一个RPC Server，并通过其来注册相关的服务，如下所示：
{% highlight string %}
using google::protobuf;

class ExampleSearchService : public SearchService {
 public:
  void Search(protobuf::RpcController* controller,
              const SearchRequest* request,
              SearchResponse* response,
              protobuf::Closure* done) {
    if (request->query() == "google") {
      response->add_result()->set_url("http://www.google.com");
    } else if (request->query() == "protocol buffers") {
      response->add_result()->set_url("http://protobuf.googlecode.com");
    }
    done->Run();
  }
};

int main() {
  // You provide class MyRpcServer.  It does not have to implement any
  // particular interface; this is just an example.
  MyRpcServer server;

  protobuf::Service* service = new ExampleSearchService;
  server.ExportOnPort(1234, service);
  server.Run();

  delete service;
  return 0;
}
{% endhighlight %}
假如你并不想要将protobuf嵌入到你自己的RPC系统中，那么你可以使用```gRPC```： 它是Google所开发的一个跨语言跨平台的开源RPC系统。gRPC可以很好的协同protobuf工作，并且通过特定的protobuf编译器插件可以直接从```.proto```文件生成相应的RPC代码。然而由于使用proto2与proto3产生的客户端、服务器服务器代码存在一些潜在的兼容性问题，因此我们建议当使用```gPRC```服务时使用proto3。

说明： 要想生成上述代码可能需要在```.proto```文件中加上如下选项
<pre>
option cc_generic_services = true;
</pre>

## 13. Options
我们可以在```.proto```文件中添加一系列的```options```。通过在```.proto```文件中添加options，虽然其并不会改变文件中相关声明（message、service等）的含义，但是在一个特定上下文下却可能影响对相关声明的处理。完整的```option```列表定义在*google/protobuf/descriptor.proto*。

其中有一些```选项```是文件级别(file-level)的选项，这意味着它们应该写在最外层，而不应该放在message、enum、或service内；有一些```选项```是属于消息级别(message-level)的选项，这意味着它们应该放在message定义中；有一些选项是属于字段级别(field-level)的选项，这意味着它们只能用在字段定义中。当前并没有针对enum types、enum values、service types、service methods的选项。

如下我们列出一些常用的选项：

* java_package(file option): 指定你要生成的Java类所需要使用的包名。假如并未在```.proto```文件汇中指定```java_package```选项的话， 则默认会使用```package```关键字所指定的包名。假如我们并不生成Java代码的话，则本选项并不会起任何作用
<pre>
option java_package = "com.example.foo";
</pre>

* java_outer_classname(file option): 用于指定最外层Java类的名称。假如并在```.proto```文件中显式指定本选项的话，则会将```.proto```文件名按驼峰格式转换为类名（例如foo_bar.proto会转换为FooBar.java)。假如我们并不生成Java代码的话，则本选项不会起任何作用
<pre>
option java_outer_classname = "Ponycopter";
</pre>

* optimize_for (file option): 可以被设置的值有```SPEED```、```CODE_SIZE```或```LITE_RUNTIME```，这会以如下方式影响C++和Java生成器
{% highlight string %}
SPEED(default): protobuf编译器会产生序列化、反序列化以及对相关message操作的代码，这些代码都被高度的优化，

CODE_SIZE: protobuf编译器将会产生最小的代码，会依赖反射等机制来实现序列化、反序列化等操作。产生的代码量会比较少，
           但是运行效率会较低。这通常适用于有大量.proto文件，但对效率要求较低的场合

LITE_RUNTIME: protobuf编译器将会产生依赖于libprotobuf-lite库的类(而不是默认的libprotobuf库）。产生的代码仍会
              像SPEED那样被高度优化，但是有些特性会被阉割。
{% endhighlight %}

如下是使用本选项的格式：
<pre>
option optimize_for = CODE_SIZE;
</pre>

* cc_generic_services, java_generic_services, py_generic_services (file options): 用于告诉protobuf编译器在产生C++、Java、Python等代码时是否针对```service```定义产生对应的抽象代码。通常情况下，默认值为为```true```。然而自从2.3.0版本以来，考虑到在具体的RPC实现时，由所对应的[code generator plugins](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.compiler.plugin.pb)能够生成更符合系统要求的代码，因此将默认值就改为了```false```，表示依赖plugins来生成service代码：
<pre>
// This file relies on plugins to generate service code.
option cc_generic_services = false;
option java_generic_services = false;
option py_generic_services = false;
</pre>

* cc_enable_arenas (file option)： 表示针对生成的C++代码启用```arena allocation```

* message_set_wire_format (message option): 假如设置为```true```，则生成的二进制格式(binary format)会兼容Google内部的一种称为```MessageSet``` 老格式。非Google内部员工的话，通常不会用到此选项
<pre>
message Foo {
  option message_set_wire_format = true;
  extensions 4 to max;
}
</pre>

* packed (field option): 假如在一个由```repeated```修饰的数值类型字段上将此选项设置为```true```的话，则会使用一种更为紧凑的编码方式。通常使用此选项并不会产生任何缺陷，但是假如在```2.3.0```版本之前，当解析器收到```packed```的数据时则会将忽略相应的字段，因此这可能会造成不兼容性。在```2.3.0```版本（包括该版本）之后，则通产不会产生任何问题。
<pre>
repeated int32 samples = 4 [packed=true];
</pre>

* deprecated (field option): 假如将此选项设置为true，则表明该该字段已经过时，在新的代码中不应该被使用。在大多数语言中其并不会产生任何实质性影响。在Java语言中，其会被标记上```@Deprecated```注释。假如对应的字段(field)并不会被任何人使用，可以考虑使用```reserved```来阻止新用户继续使用它。本选项的使用格式如下：
<pre>
optional int32 old_field = 6 [deprecated=true];
</pre>

### 13.1 自定义选项
protobuf甚至允许你定义和使用自己的选项(option)。值得指出的是这是一个```高级特性```，大部分人并不需要用到此特性。由于```options```是定义在*google/protobuf/descriptor.proto*文件的messages中的，因此在定义我们自己的选项时就相当于```扩展```(extend)这些message。例如：
<pre>
import "google/protobuf/descriptor.proto";

extend google.protobuf.MessageOptions {
  optional string my_option = 51234;
}

message MyMessage {
  option (my_option) = "Hello world!";
}
</pre>
上面我们通过继承MessageOptions定义了一个新的```消息级别```(message-level）的选项。当我们使用此选项的时候，选项名称必须用一对小括号```()```括起来，用于指示这是我们的扩充的自定义选项。在C++中，我们可以采用类似于如下的代码来读取```my_option```选项的值：
{% highlight string %}
string value = MyMessage::descriptor()->options().GetExtension(my_option);
{% endhighlight %}
上面```MyMessage::descriptor()->options()```会为```MyMessage```返回一个```MessageOptions```对象，然后就可以从它来读取自定义选项。

类似的，在Java中我们可以用如下代码：
{% highlight string %}
String value = MyProtoFile.MyMessage.getDescriptor().getOptions()
  .getExtension(MyProtoFile.myOption);
{% endhighlight %}
在Python中，我们可以用如下代码：
{% highlight string %}
value = my_proto_file_pb2.MyMessage.DESCRIPTOR.GetOptions()
  .Extensions[my_proto_file_pb2.my_option]
{% endhighlight %}
在protobuf语言中，我们可以为每一种类型都扩展自定义选项，参看如下示例：
{% highlight string %}
import "google/protobuf/descriptor.proto";

extend google.protobuf.FileOptions {
  optional string my_file_option = 50000;
}
extend google.protobuf.MessageOptions {
  optional int32 my_message_option = 50001;
}
extend google.protobuf.FieldOptions {
  optional float my_field_option = 50002;
}
extend google.protobuf.EnumOptions {
  optional bool my_enum_option = 50003;
}
extend google.protobuf.EnumValueOptions {
  optional uint32 my_enum_value_option = 50004;
}
extend google.protobuf.ServiceOptions {
  optional MyEnum my_service_option = 50005;
}
extend google.protobuf.MethodOptions {
  optional MyMessage my_method_option = 50006;
}

option (my_file_option) = "Hello world!";

message MyMessage {
  option (my_message_option) = 1234;

  optional int32 foo = 1 [(my_field_option) = 4.5];
  optional string bar = 2;
}

enum MyEnum {
  option (my_enum_option) = true;

  FOO = 1 [(my_enum_value_option) = 321];
  BAR = 2;
}

message RequestType {}
message ResponseType {}

service MyService {
  option (my_service_option) = FOO;

  rpc MyMethod(RequestType) returns(ResponseType) {
    // Note:  my_method_option has type MyMessage.  We can set each field
    //   within it using a separate "option" line.
    option (my_method_option).foo = 567;
    option (my_method_option).bar = "Some string";
  }
}
{% endhighlight %}

值得指出的是，假如你想要在一个package中使用另一个package定义的自定义选项，那么你必须加上对应的```package-name```前缀，参看如下示例：
{% highlight string %}
// foo.proto
import "google/protobuf/descriptor.proto";
package foo;
extend google.protobuf.MessageOptions {
  optional string my_option = 51234;
}
// bar.proto
import "foo.proto";
package bar;
message MyMessage {
  option (foo.my_option) = "Hello world!";
}
{% endhighlight %}
上面在```bar.proto```中定义了```bar```这个package，现在要使用```foo.proto```文件中```foo```这个package定义的选项，因此需要加上前缀```foo```。

最后一点就是，由于自定义选项是扩展(extensions)，因此因此你必须为该字段(field)指定一个编号。在上面的例子中，我们使用的编号范围是50000-99999，该范围内的编号被保留用作```各组织```内部使用，因此我们可以在自己的应用程序中自由的使用。假如你想要在一些公开的应用程序(public applications)中使用自定义选项，你最好确保所对应的字段编号是全球唯一的。为了获取全局唯一的字段编号，你可以发送邮件到[protobuf global extension registry](https://github.com/protocolbuffers/protobuf/blob/master/docs/options.md)进行登记。

我们可以只使用一个```字段编号```来声明多个自定义选项，那就是将这些选项放到一个子消息(sub-message)中。参看如下：
{% highlight string %}
message FooOptions {
  optional int32 opt1 = 1;
  optional string opt2 = 2;
}

extend google.protobuf.FieldOptions {
  optional FooOptions foo_options = 1234;
}

// usage:
message Bar {
  optional int32 a = 1 [(foo_options).opt1 = 123, (foo_options).opt2 = "baz"];
  // alternative aggregate syntax (uses TextFormat):
  optional int32 b = 2 [(foo_options) = { opt1: 123 opt2: "baz" }];
}
{% endhighlight %}
上面我们定义了一个自定义选项```foo_options```，但是其拥有两个属性。我们通过```FooOptions```封装了```opt1```和```opt2```。

同样，每一个选项类型(file-level, message-level, field-level等）都有其自己的```字段编号空间```(number space)，这意味着我们可以在FieldOptions与MessageOptions中有相同的字段编号的自定义选项。

## 14. 将.proto文件生成class
为了将```.proto```文件中的message类型生成对应的Java、Python、C++代码，你需要运行protobuf编译器```protoc```。

```protoc```的调用方式类似于如下：
{% highlight string %}
# protoc --proto_path=IMPORT_PATH --cpp_out=DST_DIR --java_out=DST_DIR --python_out=DST_DIR path/to/file.proto
{% endhighlight %}

* ```--proto_path```选项的主要作用是当解析到```.proto```文件的import指令时，应该从哪个目录导入。假如省略此选项的话，则默认导入目录为当前目录。如果需要导入多个目录的话，可以多次使用本选项。本选项的缩写形式为```-I```




<br />
<br />

[参看]:

1. [Protobuf通信协议详解： 代码演示、详细原理介绍等](http://www.360doc.com/content/16/0907/15/478627_589080443.shtml)

2. [全方位评测：Protobuf性能到底有没有比JSON快5倍？](http://www.52im.net/forum.php?mod=viewthread&tid=772#lastpost)

3. [Protobuf github](https://github.com/google/protobuf/tree/master/src/google/protobuf)

4. [protobuf developers](https://developers.google.com/protocol-buffers/)

5. [grpc官网](https://www.grpc.io/docs/)

5. [codedump](https://www.codedump.info/?p=169)

<br />
<br />
<br />





