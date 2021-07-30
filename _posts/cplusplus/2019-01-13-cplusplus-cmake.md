---
layout: post
title: cmake的安装及使用
tags:
- cplusplus
categories: cplusplus
description: cmake的安装及使用
---


本文介绍一下cmake的安装及使用方法。

<!-- more -->

## 1. cmake介绍

你或许听过好几种Make工具，例如GNU Make、QT的qmake、微软的MS nmake、BSD make(pmake)等等。这些Make工具遵循不同的规范和标准，所执行的Makefile格式也千差万别。这样就带来了一个严峻的问题：如果软件想跨平台，必须要保证能够在不同平台编译。而如果使用上面的Make工具，就得为每一种标准写一次Makefile，这将是一件让人抓狂的工作。

CMake就是针对上面问题所设计的工具：它首先允许开发者编写一种平台无关的```CMakeLists.txt```文件来定制整个编译流程，然后再根据目标用户的平台进一步生成所需的本地化Makefile和工程文件，如Unix的Makefile或Windows的Visual Studio工程。从而做到"Write once, run everywhere"。显然，CMake是一个比上述几种make更高级的编译配置工具。一些使用CMake作为项目架构系统的知名开源项目有VTK、ITK、KDE、OpenCV、OSG等。

CMake是一个开源、跨平台的工具集，主要用来构建(build)、测试(test)、以及打包(package)软件。在Linux平台下，使用CMake生成Makefile并编译的流程如下：

   1) 编写CMake配置文件CMakeLists.txt 
   
   2) 执行命令```cmake PATH```或者```ccmake PATH```生成Makefile。(其中```PATH```是CMakeLists.txt所在的目录）
   
   >注： ccmake和cmake的区别在于前者提供了一个交互式的界面。
   
   3) 使用make命令进行编译
   
### 1.1 cmake的主要特点
cmake和autotools是不同的项目工具，有各自的特点和用户群。存在即为合理，因此我们不会对两者进行优劣比较，这里只给出cmake的一些主要特点：

1） 开放源代码，使用类BSD许可发布；

2） 跨平台，并可生成native编译配置文件。在Linux/Unix平台，生成makefile；在macos平台，可以生成xcode；在windows平台可以生成MSVC的工程文件；

3） 能够管理大型项目，KDE4就是最好的证明。

4） 简化构建过程和编译过程。cmake的工具链非常简单：cmake + make 

5） 高效率，按照KDE官方说法，CMake构建KDE4的kdelibs要比使用autotools来构建KDE3.5.6的kdelibs快40%，主要是因为cmake在工具链中没有libtool。

6） 可扩展，可以为cmake编写特定功能的模块，扩充cmake功能。

## 2. cmake的安装

cmake的安装基本上有如下两种方式：

* 二进制包安装 

* 源代码编译安装 


### 2.1 二进制包安装 

我们可以使用```yum```包管理工具直接安装：
<pre>
# yum list cmake 
cmake.x86_64                                                         2.8.12.2-2.el7                                                         base

# yum install cmake 
</pre>

如下我们介绍到[cmake官网](https://cmake.org/install/) 下载对应的安装包来安装（这里我们以安装cmake 3.20.5为例）：

1） 下载cmake-3.20.5
<pre>
# mkdir cmake-inst
# cd cmake-inst 
# wget https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5-linux-x86_64.tar.gz
# ls 
cmake-3.20.5-linux-x86_64.tar.gz
</pre>

2) 解压到指定目录

创建安装目录```/usr/local/cmake```，并将上面的压缩包解压到该目录：
<pre>
# mkdir -p /usr/local/cmake
# tar -zxvf cmake-3.20.5-linux-x86_64.tar.gz -C /usr/local/cmake
# ls /usr/local/cmake
cmake-3.20.5-linux-x86_64

# ls /usr/local/cmake/cmake-3.20.5-linux-x86_64/
bin  doc  man  share
</pre>

3) 设置环境变量并检查cmake版本
<pre>
# export PATH=$PATH:/usr/local/cmake/cmake-3.20.5-linux-x86_64/bin
# cmake --version
cmake version 3.17.2
</pre>

### 2.2 使用源代码安装

如下我们以安装cmake-3.20.5为例，介绍通过源代码来安装的步骤：

1） 下载源代码包

我们可以到[cmake官网](https://cmake.org/download/) 下载cmake源代码包：
<pre>
# wget https://github.com/Kitware/CMake/releases/download/v3.20.5/cmake-3.20.5.tar.gz
# ls
cmake-3.20.5.tar.gz

# tar -zxvf cmake-3.20.5.tar.gz 
# ls
cmake-3.20.5  cmake-3.20.5.tar.gz
# cd cmake-3.20.5
</pre>

2) 创建编译目录

我们在源代码平级目录创建一个编译目录```cmake-build```:
<pre>
# mkdir cmake-build && cd cmake-build 
</pre>

3) 编译 

创建安装目录```/usr/local/cmake```：
<pre>
# mkdir -p /usr/local/cmake
</pre>

然后执行如下命令进行编译：
<pre>
#  ../cmake-3.20.5/bootstrap --prefix=/usr/local/cmake
# make 
# make install 
# ls /usr/local/cmake
bin  doc  share
</pre>

4) 设置环境变量并检查cmake版本
<pre>
# export PATH=$PATH:/usr/local/cmake/bin
# cmake --version
cmake version 3.17.2

CMake suite maintained and supported by Kitware (kitware.com/cmake).
</pre>



<br />
<br />

**[参看]**

1. [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

2. [cmake官网](https://cmake.org/)

3. [cmake install](https://cmake.org/install/)

4. [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

<br />
<br />
<br />


