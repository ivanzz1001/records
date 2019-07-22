---
layout: post
title: gRPC的安装及使用
tags:
- cplusplus
categories: cplusplus
description: gRPC的安装及使用
---

本章我们简单介绍一下gRPC的安装及使用


<!-- more -->

## 1. gRPC的安装

1) **安装protobuf**

关于protobuf的安装，我们前面介绍过，这里不再赘述。

2） **安装依赖库libcares**

在安装gRPC时我们发现需要依赖```libcares```库，但是gRPC其本身的编译脚本不能很好的安装该库，这里我们手动安装：
<pre>
# wget https://c-ares.haxx.se/download/c-ares-1.15.0.tar.gz
# tar -zxvf c-ares-1.15.0.tar.gz 
# cd c-ares-1.15.0/
# ./configure --prefix=/usr/local/cares
# make
# make install
# ls /usr/local/cares
include  lib  share
</pre>

3) **安装perftools**

```gperftools```依赖于```libunwind```，首先执行如下命令安装：
<pre>
# wget http://download.savannah.nongnu.org/releases/libunwind/libunwind-1.3.1.tar.gz
# tar -zxvf libunwind-1.3.1.tar.gz 
# cd libunwind-1.3.1/
./configure --prefix=/usr/local/unwind
# make
# make install
# ls /usr/local/unwind/
include  lib
</pre>

接着再安装```google-perfile```:
<pre>
# wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.7/gperftools-2.7.tar.gz
# tar -zxvf gperftools-2.7.tar.gz 
# cd gperftools-2.7/
# ./configure --prefix=/usr/local/gperftools
# make
# make install
# ls /usr/local/gperftools/
bin  include  lib  share
</pre>

3） **安装gRPC**

这里我们先下载gRPC安装软件包并解压：
<pre>
# mkdir gRPC-inst
# cd gRPC-inst
# wget https://github.com/grpc/grpc/archive/v1.22.0.tar.gz
# tar -zxvf v1.22.0.tar.gz 
# cd grpc-1.22.0/
</pre>

由于gRPC编译安装脚本写的不是很好，在执行```make run_dep_checks```时会通过```pkg-config```来识别编译时所依赖的库是否存在，而真正在执行```make```时却又会识别不了，因此我们导出如下几个环境变量：
<pre>
# export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/root/bin:/usr/local/protobuf/bin/
# export PKG_CONFIG_PATH=/usr/local/protobuf/lib/pkgconfig/:/usr/local/cares/lib/pkgconfig/:/usr/local/gperftools/lib/pkgconfig/      //编译时
# export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/protobuf/lib/:/usr/local/cares/lib/:/usr/local/gperftools/lib/               //编译时
</pre>
修改Makefile文件，在*PERFTOOLS_CHECK_CMD*命令最后加上```pkg-config```，最后如下：
{% highlight string %}
PERFTOOLS_CHECK_CMD = $(CC) $(CPPFLAGS) $(CFLAGS) -o $(TMPOUT) test/build/perftools.c -lprofiler $(LDFLAGS) `pkg-config --cflags libprofiler`
{% endhighlight %}
然后执行如下命令检查编译环境是否有问题：
<pre>
# make run_dep_checks
</pre>
其实，这里我们可以不用安装```perftools```，检查时发现有```perftools```方面的错误，但都是test里面的，因此可以忽略跳过。最后接着执行如下命令进行安装：
<pre>
# make 
# make prefix=/usr/local/grpc
# make install prefix=/usr/local/grpc
</pre>

## 2. gRPC示例

如下我们使用gRPC+protobuf实现一个回显功能。

1) **编写proto文件**

如下我们实现一个```回显```服务器，编写```echoserver.proto```:
{% highlight string %}
syntax = "proto3";

message EchoRequest {

        string request = 1;
}

message EchoResponse {
        string response = 1;
}

service EchoService {
        rpc echo(EchoRequest) returns (EchoResponse);
}
{% endhighlight %}

2) **自动生成protobuf及grpc代码**

如下我们使用```echoserver.proto```自动生成grpc及protobuf代码：
<pre>
# /usr/local/protobuf/bin/protoc --cpp_out=./ echoserver.proto
# /usr/local/protobuf/bin/protoc --grpc_out=./ --plugin=protoc-gen-grpc=/usr/local/grpc/bin/grpc_cpp_plugin echoserver.proto
# ls
echoserver.grpc.pb.cc  echoserver.grpc.pb.h  echoserver.pb.cc  echoserver.pb.h  echoserver.proto
</pre>
这两个命令将会生成如下4个文件：echoserver.pb.c(h)、echoserver.grpc.pb.c(h)

3) **编写服务器代码**

如下我们编写服务器代码```grpc_echo_server.cpp```:
{% highlight string %}
#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
 
#include "echoserver.grpc.pb.h"
 
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class EchoServiceImpl final : public EchoService::Service{
        //override echo 
        ::grpc::Status echo(::grpc::ServerContext* context, const ::EchoRequest* request, ::EchoResponse* response){
                std::string prefix("Hello ");
                response->set_response(prefix + request->request());
                return Status::OK;
        }
};
 
void RunServer(){
        std::string server_address("0.0.0.0:50051");
        EchoServiceImpl echoservice;

        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&echoservice);
        std::unique_ptr<Server> server(builder.BuildAndStart());

        std::cout << "Server listening on " << server_address << std::endl;

        server->Wait();
}

 
int main(int argc, char *argv[]){
        RunServer();

        return 0x0;
}
{% endhighlight %}
执行如下命令进行编译：
<pre>
# export PKG_CONFIG_PATH=/usr/local/cares/lib/pkgconfig/:/usr/local/protobuf/lib/pkgconfig/:/usr/local/grpc/lib/pkgconfig/
# gcc -o grpc_echo_server ./echoserver.pb.cc ./echoserver.grpc.pb.cc ./grpc_echo_server.cpp \
-std=c++11 `pkg-config --cflags --libs grpc grpc++ protobuf libcares gpr` -lstdc++
# ls
echoserver.grpc.pb.cc  echoserver.grpc.pb.h  echoserver.pb.cc  echoserver.pb.h  echoserver.proto  grpc_echo_server  grpc_echo_server.cpp
</pre>

4) **编写客户端代码**

如下我们编写服务器代码```grpc_echo_client.cpp```:
{% highlight string %}
#include <iostream>
#include <memory>
#include <string>
 
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
 
#include "echoserver.grpc.pb.h"
 
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
 
 
class EchoClient {
 public:
  explicit EchoClient(std::shared_ptr<Channel> channel)
      : stub_(EchoService::NewStub(channel)) {}
 
  std::string echo(const std::string& msg) {
 
    EchoRequest request;
    request.set_request(msg);
 
    EchoResponse reply;
   
    ClientContext context;
 
    CompletionQueue cq;
 
    Status status;
 
    std::unique_ptr<ClientAsyncResponseReader<EchoResponse> > rpc(
        stub_->Asyncecho(&context, request, &cq));
 
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;
  
    GPR_ASSERT(cq.Next(&got_tag, &ok));
 
   
    GPR_ASSERT(got_tag == (void*)1);
  
    GPR_ASSERT(ok);
 
    if (status.ok()) {
      return reply.response();
    } else {
      return "RPC failed";
    }
  }
 
 private:
 
  std::unique_ptr<EchoService::Stub> stub_;
};
 
int main(int argc, char *argv[]) {
  EchoClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string msg("world");
  std::string reply = client.echo(msg);  // The actual RPC call!
  std::cout << "client received: " << reply << std::endl;
 
  return 0;
} 
{% endhighlight %}
执行如下命令进行编译：
<pre>
# export PKG_CONFIG_PATH=/usr/local/cares/lib/pkgconfig/:/usr/local/protobuf/lib/pkgconfig/:/usr/local/grpc/lib/pkgconfig/
# gcc -o grpc_echo_client ./echoserver.pb.cc ./echoserver.grpc.pb.cc ./grpc_echo_client.cpp \
-std=c++11 `pkg-config --cflags --libs grpc grpc++ protobuf libcares gpr` -lstdc++
# ls
echoserver.grpc.pb.cc  echoserver.pb.cc  echoserver.proto  grpc_echo_client.cpp  grpc_echo_server.cpp
echoserver.grpc.pb.h   echoserver.pb.h   grpc_echo_client  grpc_echo_server
</pre>


5) **运行测试**

服务端运行：
<pre>
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/protobuf/lib/:/usr/local/grpc/lib/:/usr/local/cares/lib/
# ./grpc_echo_server 
Server listening on 0.0.0.0:50051
</pre>

客户端运行：
<pre>
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/protobuf/lib/:/usr/local/grpc/lib/:/usr/local/cares/lib/
# ./grpc_echo_client 
client received: Hello world
</pre>

<br />
<br />

[参看]:

1. [gRPC GitHub](https://github.com/grpc/grpc)

2. [gRPC官网](https://www.grpc.io/docs/quickstart/cpp/)

3. [grpc+protobuf 的C++ service 实例解析](https://blog.csdn.net/u012023606/article/details/54583526)

4. [Protobuf github](https://github.com/google/protobuf/tree/master/src/google/protobuf)

5. [protobuf developers](https://developers.google.com/protocol-buffers/)

6. [libcares库](https://c-ares.haxx.se/)

7. [libunwind安装](http://www.nongnu.org/libunwind/download.html)

8. [gperftools GitHUB](https://github.com/gperftools/gperftools)

<br />
<br />
<br />





