---
layout: post
title: Bazel Getting Started
tags:
- bazel
categories: bazel
description: bazel Getting Started
---

本文主要包含两个方面的内容： 1） 演示Bazel的基本使用流程 2) 介绍Bazel使用过程中的一些基本概念

参看:

- [bazel release](https://github.com/bazelbuild/bazel/releases)

- [Installing / Updating Bazel using Bazelisk](https://bazel.build/install/bazelisk)

- [bazelist GitHub](https://github.com/bazelbuild/bazelisk)




<!-- more -->

## 1. 使用Bazel来构建一个C++工程

执行如下命令从Bazel GitHub仓库克隆出一个sample:
<pre>
# mkdir bazel-workspace
# cd bazel-workspace
# git clone https://github.com/bazelbuild/examples
</pre>
我们来看看本节所需要用到的`examples/cpp-tutorial`目录：
<pre>
# tree examples/cpp-tutorial
examples
└── cpp-tutorial
    ├──stage1
    │  ├── main
    │  │   ├── BUILD
    │  │   └── hello-world.cc
    │  └── MODULE.bazel
    ├──stage2
    │  ├── main
    │  │   ├── BUILD
    │  │   ├── hello-world.cc
    │  │   ├── hello-greet.cc
    │  │   └── hello-greet.h
    │  └── MODULE.bazel
    └──stage3
       ├── main
       │   ├── BUILD
       │   ├── hello-world.cc
       │   ├── hello-greet.cc
       │   └── hello-greet.h
       ├── lib
       │   ├── BUILD
       │   ├── hello-time.cc
       │   └── hello-time.h
       └── MODULE.bazel
</pre>

从上面我们看到`cpp-turorial`包含3个stage文件夹，本文也分3个stage来进行讲解。在stage1中，我们会从单个package中构建单个target； 在stage2中，我们会从单个package中构建两个target: binary和library； 在stage3中，我们从多个package中构建多个targets。

### 1.1 预备知识
在我们开始build一个project之前，首先需要创建一个workspace。一个worksapce含有工程源文件和bazel输出的目录，此外目录中通常还会包含如下一些典型的文件：

- `MODULE.bazel`文件：用于标识该目录是一个bazel workspace，放置于工程的根目录中， 可以在该文件中指定所需的外部依赖；

- 一个或多个`BUILD`文件：用于告诉Bazel如何构建工程的不同部分。工程中包含`BUILD`文件的目录称为`package`。

1） **Understand BUILD file**

一个BUILD文件中可以包含许多不同类型的Bazel指令。每个BUILD文件都至少需要设置一个rule，用于告诉bazel如何构建构建你想要的输出。BUILD文件中的每一个rule实例称为`target`，其指定了构建相应的输出所需要的源文件和依赖项。此外，一个target也可以指向多个其他的targets。

下面我们来看看`cpp-tutorial/stage1/main`目录下的BUILD文件：

```
cc_binary(
    name = "hello-world",
    srcs = ["hello-world.cc"],
)
```
在我们的例子中，`hello-world`这个target实例化了bazel的内置[cc_binary rule](https://bazel.google.cn/reference/be/c-cpp#cc_binary)。通过该规则，bazel就知道了需要通过源文件`hello-world.cc`来构建一个自包含的可执行文件，并且构建时没有其他依赖。

### 1.2 Stage 1: single target, single package

接下来我们使用bazel来构建stage1这一工程，先来看看该工程的目录结构：

```
examples
└── cpp-tutorial
    └──stage1
       ├── main
       │   ├── BUILD
       │   └── hello-world.cc
       └── MODULE.bazel
```
进入到该目录执行如下命令:
<pre>
# cd cpp-tutorial/stage1
# bazel build //main:hello-world                                                                       
2024/12/04 14:19:23 Warning: used fallback version "8.0.0rc6"
2024/12/04 14:19:23 Downloading https://releases.bazel.build/8.0.0/rc6/bazel-8.0.0rc6-linux-x86_64...
2024/12/04 14:19:23 Skipping basic authentication for releases.bazel.build because no credentials found in /root/.netrc
Downloading: 61 MB out of 61 MB (100%) 
Extracting Bazel installation...
Starting local Bazel server and connecting to it...
WARNING: Couldn't auto load rules or symbols, because no dependency on module/repository 'rules_android' found. This will result in a failure if there's a reference to those rules or symbols.
INFO: Analyzed target //main:hello-world (68 packages loaded, 468 targets configured).
INFO: Found 1 target...
Target //main:hello-world up-to-date:
  bazel-bin/main/hello-world
INFO: Elapsed time: 29.465s, Critical Path: 0.27s
INFO: 6 processes: 4 internal, 2 linux-sandbox.
INFO: Build completed successfully, 6 total actions
</pre>

命令执行后我们来看看所产生的output:
```
# tree
.
├── bazel-bin -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin
├── bazel-out -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out
├── bazel-stage1 -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main
├── bazel-testlogs -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/testlogs
├── main
│   ├── BUILD
│   └── hello-world.cc
├── MODULE.bazel
├── MODULE.bazel.lock
└── README.md

5 directories, 5 files

# tree bazel-bin
bazel-bin
└── main
    ├── hello-world
    ├── hello-world-0.params
    ├── hello-world.repo_mapping
    ├── hello-world.runfiles
    │   ├── _main
    │   │   └── main
    │   │       └── hello-world -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world
    │   ├── MANIFEST -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.runfiles_manifest
    │   └── _repo_mapping -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.repo_mapping
    ├── hello-world.runfiles_manifest
    └── _objs
        └── hello-world
            ├── hello-world.pic.d
            └── hello-world.pic.o

6 directories, 9 files

# tree bazel-out
bazel-out
├── k8-fastbuild
│   ├── bin
│   │   └── main
│   │       ├── hello-world
│   │       ├── hello-world-0.params
│   │       ├── hello-world.repo_mapping
│   │       ├── hello-world.runfiles
│   │       │   ├── _main
│   │       │   │   └── main
│   │       │   │       └── hello-world -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world
│   │       │   ├── MANIFEST -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.runfiles_manifest
│   │       │   └── _repo_mapping -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.repo_mapping
│   │       ├── hello-world.runfiles_manifest
│   │       └── _objs
│   │           └── hello-world
│   │               ├── hello-world.pic.d
│   │               └── hello-world.pic.o
│   └── testlogs
├── stable-status.txt
├── _tmp
│   └── actions
│       ├── stderr-5
│       ├── stderr-7
│       ├── stdout-5
│       └── stdout-7
└── volatile-status.txt

11 directories, 15 files

# tree bazel-stage1
bazel-stage1
├── bazel-out
│   ├── k8-fastbuild
│   │   ├── bin
│   │   │   └── main
│   │   │       ├── hello-world
│   │   │       ├── hello-world-0.params
│   │   │       ├── hello-world.repo_mapping
│   │   │       ├── hello-world.runfiles
│   │   │       │   ├── _main
│   │   │       │   │   └── main
│   │   │       │   │       └── hello-world -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world
│   │   │       │   ├── MANIFEST -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.runfiles_manifest
│   │   │       │   └── _repo_mapping -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/execroot/_main/bazel-out/k8-fastbuild/bin/main/hello-world.repo_mapping
│   │   │       ├── hello-world.runfiles_manifest
│   │   │       └── _objs
│   │   │           └── hello-world
│   │   │               ├── hello-world.pic.d
│   │   │               └── hello-world.pic.o
│   │   └── testlogs
│   ├── stable-status.txt
│   ├── _tmp
│   │   └── actions
│   │       ├── stderr-5
│   │       ├── stderr-7
│   │       ├── stdout-5
│   │       └── stdout-7
│   └── volatile-status.txt
├── external
│   ├── bazel_tools -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/external/bazel_tools
│   ├── bazel_tools+xcode_configure_extension+local_config_xcode -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/external/bazel_tools+xcode_configure_extension+local_config_xcode
│   ├── platforms -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/external/platforms
│   ├── rules_cc+ -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/external/rules_cc+
│   └── rules_cc++cc_configure_extension+local_config_cc -> /root/.cache/bazel/_bazel_root/63b5237301a1c80ee5b3af8a4d0a90e9/external/rules_cc++cc_configure_extension+local_config_cc
├── main -> /workspace/cpp_proj/bazel-workspace/examples/cpp-tutorial/stage1/main
├── MODULE.bazel -> /workspace/cpp_proj/bazel-workspace/examples/cpp-tutorial/stage1/MODULE.bazel
├── MODULE.bazel.lock -> /workspace/cpp_proj/bazel-workspace/examples/cpp-tutorial/stage1/MODULE.bazel.lock
└── README.md -> /workspace/cpp_proj/bazel-workspace/examples/cpp-tutorial/stage1/README.md

19 directories, 18 files

# tree bazel-testlogs
bazel-testlogs

0 directories, 0 files
```




## 2. 使用Bazel来构建一个Golang工程

## 3. Bazel基本概念
