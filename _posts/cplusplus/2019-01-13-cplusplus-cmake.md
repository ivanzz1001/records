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


## 3. cmake tutorial 
下面我们分成12个小节，来介绍cmake的使用。

### 3.1 入门案例

通常，我们需要将工程(project)中的的源代码编译为一个可执行文件。对于最简单的项目来说，只需要一个3行的```CMakeList.txt```文件即可完成。

1） 创建Step1文件夹
<pre>
# mkdir -p Step1 
# cd Step1
</pre>

2) 编写tutorial.cxx源文件

我们在上面的```Step1```文件中编写turorial.cxx源代码如下：
{% highlight string %}
// A simple program that computes the square root of a number
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " number" << std::endl;
    return 1;
  }

  // convert input to double
  const double inputValue = atof(argv[1]);

  // calculate square root
  const double outputValue = sqrt(inputValue);
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
{% endhighlight %}
上面的代码用于计算一个数的平方根。

3) 创建CMakeLists.txt文件

在```Step1```目录下创建CMakeLists.txt文件，内容如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name
project(Tutorial)

# add the executable 
add_executable(Tutorial tutorial.cxx)
{% endhighlight %}
上面的CMakeLists.txt例子中，里面的cmake命令都使用的是小写格式。事实上，使用大写、小写、或者大小写混合，cmake都是支持的。

###### 3.1.1 增加版本号和一个用于配置的头文件
在这里，我们为可执行文件和工程添加```版本号```(version number)支持。我们可以不用修改```turorial.cxx```源代码，而是使用```CMakeLists.txt```以提供更好的灵活性：

1） 使用project()命令设置工程名和版本号

修改```CMakeLists.txt```文件，使用project()命令来设置工程名和版本号：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)
{% endhighlight %}

2) 配置一个头文件来将版本号传递给源文件

修改```CMakeLists.txt```如下：
{% highlight string %}
configure_file(TutorialConfig.h.in TutorialConfig.h)
{% endhighlight %}
由于配置文件将会被写入到[binary tree](https://cmake.org/cmake/help/v3.0/variable/PROJECT_BINARY_DIR.html)，因此我们必须将该目录添加到源文件的搜索路径中。可以在```CMakeLists.txt```文件中添加如下：
{% highlight string %}
target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
{% endhighlight %}
>注：
> SOURCE_DIR: The top-level source directory on which CMake was run.
>BINARY_DIR: The top-level build directory on which CMake was run.


这样修改完成之后，到目前为止，我们的CMakeLists.txt看起来如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

configure_file(TutorialConfig.h.in TutorialConfig.h)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
{% endhighlight %}

3) 创建TutorialConfig.h.in配置文件

我们在```源代码目录```创建TutorialConfig.h.in配置文件，内容如下:
{% highlight string %}
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@
{% endhighlight %}
当CMake配置该头文件时，```@Tutorial_VERSION_MAJOR@```和```@Tutorial_VERSION_MINOR@```就会被替换。

4） 修改tutorial.cxx源文件

下面我们修改tutorial.cxx源文件，让其包含TutorialConfig.h头文件。并添加打印版本号的语句：
{% highlight string %}
// A simple program that computes the square root of a number
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include "TutorialConfig.h"



int main(int argc, char* argv[])
{
  if (argc < 2) {
    // report version
    std::cout << argv[0] << " Version " << Tutorial_VERSION_MAJOR << "."
              << Tutorial_VERSION_MINOR << std::endl;
    std::cout << "Usage: " << argv[0] << " number" << std::endl;
    return 1;
  }

  // convert input to double
  const double inputValue = atof(argv[1]);

  // calculate square root
  const double outputValue = sqrt(inputValue);
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
{% endhighlight %}

###### 3.1.2 指定C++标准
下面我们增加一些c++ 11的特性到工程中： 将tutorial.cxx源文件中的atof()替换为std::stod()
{% highlight string %}
// A simple program that computes the square root of a number
#include <cmath>
#include <iostream>
#include <string>
#include "TutorialConfig.h"



int main(int argc, char* argv[])
{
  if (argc < 2) {
    // report version
    std::cout << argv[0] << " Version " << Tutorial_VERSION_MAJOR << "."
              << Tutorial_VERSION_MINOR << std::endl;
    std::cout << "Usage: " << argv[0] << " number" << std::endl;
    return 1;
  }

  // convert input to double
  const double inputValue = std::stod(argv[1]);

  // calculate square root
  const double outputValue = sqrt(inputValue);
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
{% endhighlight %}
>注：可以不需要cstdlib头文件了

这样修改之后，我们需要显式的在CMake代码中指定编译所需要的flag。最容易的方法就是通过使用```CMAKE_CXX_STANDARD```变量来指定相应的C++标准。在本例子中，我们可以在CMakeLists.txt文件中设置```CMAKE_CXX_STANDARD```变量的值为11，并将```CMAKE_CXX_STANDARD_REQUIRED```设置为True。注意，请确保将```CMAKE_CXX_STANDARD```声明在add_execute()命令之前。

如下是修改后的CMakeList.txt:
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


configure_file(TutorialConfig.h.in TutorialConfig.h)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
{% endhighlight %}

###### 3.1.3 Build and Test 
执行```cmake```命令或者```cmake-gui```来配置工程，之后再选择指定的工具完成编译。

1）创建编译目录 

我们在与```Step1```平级的目录中创建构建目录```Step1_build```:
<pre>
# mkdir Step1_build
# ls 
Step1  Step1_build
</pre>

2) 产生本地构建系统

进入build目录，然后执行cmake来配置工程，并产生本地构建系统：
{% highlight string %}
# cd Step1_build 
# cmake ../Step1 
-- The C compiler identification is GNU 6.5.0
-- The CXX compiler identification is GNU 6.5.0
-- Check for working C compiler: /usr/local/bin/gcc
-- Check for working C compiler: /usr/local/bin/gcc - works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/local/bin/c++
-- Check for working CXX compiler: /usr/local/bin/c++ - works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step1_build
# ls
CMakeCache.txt  CMakeFiles  cmake_install.cmake  Makefile  TutorialConfig.h
# tree
.
├── CMakeCache.txt
├── CMakeFiles
│   ├── 3.17.2
│   │   ├── CMakeCCompiler.cmake
│   │   ├── CMakeCXXCompiler.cmake
│   │   ├── CMakeDetermineCompilerABI_C.bin
│   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   ├── CMakeSystem.cmake
│   │   ├── CompilerIdC
│   │   │   ├── a.out
│   │   │   ├── CMakeCCompilerId.c
│   │   │   └── tmp
│   │   └── CompilerIdCXX
│   │       ├── a.out
│   │       ├── CMakeCXXCompilerId.cpp
│   │       └── tmp
│   ├── cmake.check_cache
│   ├── CMakeDirectoryInformation.cmake
│   ├── CMakeOutput.log
│   ├── CMakeTmp
│   ├── Makefile2
│   ├── Makefile.cmake
│   ├── progress.marks
│   ├── TargetDirectories.txt
│   └── Tutorial.dir
│       ├── build.make
│       ├── cmake_clean.cmake
│       ├── DependInfo.cmake
│       ├── depend.make
│       ├── flags.make
│       ├── link.txt
│       └── progress.make
├── cmake_install.cmake
├── Makefile
└── TutorialConfig.h

8 directories, 27 files
{% endhighlight %}

3) 编译工程 

执行如下命令来完成工程的编译和链接：
{% highlight string %}
# cmake --build .
Scanning dependencies of target Tutorial
[ 50%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial
# ls
CMakeCache.txt  CMakeFiles  cmake_install.cmake  Makefile  Tutorial  TutorialConfig.h
# tree
.
├── CMakeCache.txt
├── CMakeFiles
│   ├── 3.17.2
│   │   ├── CMakeCCompiler.cmake
│   │   ├── CMakeCXXCompiler.cmake
│   │   ├── CMakeDetermineCompilerABI_C.bin
│   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   ├── CMakeSystem.cmake
│   │   ├── CompilerIdC
│   │   │   ├── a.out
│   │   │   ├── CMakeCCompilerId.c
│   │   │   └── tmp
│   │   └── CompilerIdCXX
│   │       ├── a.out
│   │       ├── CMakeCXXCompilerId.cpp
│   │       └── tmp
│   ├── cmake.check_cache
│   ├── CMakeDirectoryInformation.cmake
│   ├── CMakeOutput.log
│   ├── CMakeTmp
│   ├── Makefile2
│   ├── Makefile.cmake
│   ├── progress.marks
│   ├── TargetDirectories.txt
│   └── Tutorial.dir
│       ├── build.make
│       ├── cmake_clean.cmake
│       ├── CXX.includecache
│       ├── DependInfo.cmake
│       ├── depend.internal
│       ├── depend.make
│       ├── flags.make
│       ├── link.txt
│       ├── progress.make
│       └── tutorial.cxx.o
├── cmake_install.cmake
├── Makefile
├── Tutorial
└── TutorialConfig.h

8 directories, 31 files
{% endhighlight %}
上面我们看到生成了```Tutorial```可执行文件。

对比前面，我们看到多出了如下4个文件：

* Step1_build/Tutorial文件

* Step1_build/Tutorial.dir/CXX.includecache文件

* Step1_build/Tutorial.dir/depend.internal文件

* Step1_build/Tutorial.dir/tutorial.cxx.o文件



4) 测试验证

在上面编译完成之后生成了可执行文件```Tutotial```，执行如下命令来验证：
<pre>
# ./Tutorial 4294967296
The square root of 4.29497e+09 is 65536
# ./Tutorial 10
The square root of 10 is 3.16228
# ./Tutorial 
./Tutorial Version 1.0
Usage: ./Tutorial number
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


