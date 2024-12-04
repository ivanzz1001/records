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




## 2. 使用Bazel来构建一个Golang工程

## 3. Bazel基本概念
