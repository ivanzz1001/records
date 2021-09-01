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

>注：所有参考示例都可以在cmake源代码目录cmake-3.20.5/Help/guide/tutorial找到

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


### 3.3 Adding a Library
本章我们会介绍添加一个library到工程中。首先实现一个计算```平方根```的动态链接库，然后将该动态库链接到可执行程序中。

在本例子中，我们会将动态链接库及相关的文件放入一个```MathFunctions```目录中。该目录包含头文件```MathFunctions.h```以及源文件```mysqrt.cxx```。在源文件中实现函数```mysqrt()```用于求一个数的平方根。

创建```Step2```工程目录，然后在工程目录中创建MathFunctions目录，如下：
<pre>
# mkdir Step2 
# mkdir Step2/MathFunctions 
# cd Step2 
</pre>

接上文，此时我们在Step2目录下有如下3个文件：

* Step2/turorial.cxx源文件
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

* Step2/TutorialConfig.h.in配置文件 
{% highlight string %}
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@
{% endhighlight %}

* Step2/CMakeLists.txt 
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


###### 3.2.1 编写库文件
在Step2/MathFunctions目录下创建MathFunctions.h(c)。

1) MathFunctions.h头文件 
{% highlight string %}
double mysqrt(double x);
{% endhighlight %}


2) MathFunctions.c源文件
{% highlight string %}
#include <iostream>

// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

  double result = x;

  // do ten iterations
  for (int i = 0; i < 10; ++i) {
    if (result <= 0) {
      result = 0.1;
    }
    double delta = x - (result * result);
    result = result + 0.5 * delta / result;
    std::cout << "Computing sqrt of " << x << " to be " << result << std::endl;
  }
  return result;
}
{% endhighlight %}


3) 编写库文件对应的CMakeLists.txt 

在Step2/MathFunctions目录下增加CMakeLists.txt文件如下:
{% highlight string %}
add_library(MathFunctions mysqrt.cxx)
{% endhighlight %}

###### 3.2.2 调用MathFunctions库
为了使用上面我们编写的新库(MathFunctions)，在Step2/CMakeLists.txt文件中使用add_subdirectory()命令把相应的路径包含进来，这样就可以完成对该路径下文件的构建。
然后调用target_link_libraries()命令将```MathFunctions```库链接到可执行文件中，最后再调用target_include_directories()将MathFunctions路径包含进binary tree搜索路径。此时Step2/CMakeLists.txt文件如下所示：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(TutorialConfig.h.in TutorialConfig.h)

add_subdirectory(MathFunctions)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC MathFunctions)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           "${PROJECT_SOURCE_DIR}/MathFunctions"
                           )
{% endhighlight %}



下面我们将```MathFunctions```库配置为可选。

1） 设置选项开关

我们设置一个选项开关```USE_MYMATH```，修改Step2/CMakeLists.txt文件如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

add_subdirectory(MathFunctions)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC MathFunctions)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           "${PROJECT_SOURCE_DIR}/MathFunctions"
                           )
{% endhighlight %}
我们可以使用```cmake-gui```或者```ccmake```命令来对该选项进行配置。相应的配置会被保存到```cache```中，这样用户在执行```cmake```命令构建时就不必每一次都进行单独配置。

2）用选项控制编译和链接

在上面```步骤1)```我们设置了一个控制开关，下面我们就要用该开关来控制```MathFunctions```的编译与链接。下面我们修改Step2/CMakeLists.txt文件如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

if(USE_MYMATH)
	add_subdirectory(MathFunctions)
	list(APPEND EXTRA_LIBS MathFunctions)
	list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/MathFunctions")
endif()


# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           ${EXTRA_INCLUDES}
                           )
{% endhighlight %}
上面我们使用```EXTRA_LIBS```变量来控制所要链接的库；使用```EXTRA_INCLUDES```变量来控制所使用的头文件。当有许多可选组件的时候，这是经典的方法来控制到底使用哪一个组件。


3） 修改tutorial.cxx源代码文件

接下来，我们修改turotial.cxx源代码文件如下：
{% highlight string %}
// A simple program that computes the square root of a number
#include <cmath>
#include <iostream>
#include <string>

#include "TutorialConfig.h"

#ifdef USE_MYMATH
#include "MathFunctions.h"
#endif 

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
  #ifdef USE_MYMATH
	const double outputValue = mysqrt(inputValue);
  #else 
    const double outputValue = sqrt(inputValue);
  #endif 
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
{% endhighlight %}

4) 在TutorialConfig.h.in中配置USE_MYMATH

由于我们在源代码中需要使用```USE_MYMATH```，我们可以将其加入到TutorialConfig.h.in文件中：
{% highlight string %}
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@

#cmakedefine USE_MYMATH 
{% endhighlight %}

>注：为什么我们在配置USE_MYMATH选项之后，又需要在TutorialConfig.h.in中配置？

###### 3.2.3 测试验证

下面我们对程序进行编译。

1) 创建编译目录 

首先在与Step2平级的目录创建Step2_build目录：
<pre>
# mkdir Step2_build
# cd Step2_build 
</pre>
 
2）默认编译及链接

* 生成编译文件 

这里我们不进行任何配置，直接采用如下命令生成编译文件：
<pre>
# cmake ../Step2
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
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step2_build
</pre>
此时，我们查看生成的TutorialConfig.h头文件：
{% highlight string %}
# // the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR 1
#define Tutorial_VERSION_MINOR 0

#define USE_MYMATH 
{% endhighlight %}

* 编译链接

执行如下命令进行编译链接：
<pre>
cmake --build .
Scanning dependencies of target MathFunctions
[ 25%] Building CXX object MathFunctions/CMakeFiles/MathFunctions.dir/mysqrt.cxx.o
[ 50%] Linking CXX static library libMathFunctions.a
[ 50%] Built target MathFunctions
Scanning dependencies of target Tutorial
[ 75%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial
</pre>

* 测试 
{% highlight string %}
# ./Tutorial 2.25
Computing sqrt of 2.25 to be 1.625
Computing sqrt of 2.25 to be 1.50481
Computing sqrt of 2.25 to be 1.50001
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
The square root of 2.25 is 1.5
{% endhighlight %}


3) 使用ccmake配置，然后编译

* 使用ccmake配置相应选项
<pre>
# ccmake ../Step2 
</pre>
这里我们配置```USE_MYMATH```，将对应的选项关闭

>注：当配置进行了修改，需要重新按'c'键，然后就会出现'g'选项

* 编译链接 
<pre>
# cmake --build ./
Scanning dependencies of target Tutorial
[ 50%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial
</pre>

* 测试
<pre>
# ./Tutorial 2.25
The square root of 2.25 is 1.5
</pre>

4） 直接使用cmake指定编译选项

* 使用cmake产生编译脚本

我们直接使用```cmake -D```来配置相应的选项：
<pre>
# cmake ../Step2 -DUSE_MYMATH=OFF 
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
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step2_build
</pre>

* 编译链接 
<pre>
# cmake --build ./
Scanning dependencies of target Tutorial
[ 50%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial
</pre>

* 测试 
<pre>
# ./Tutorial 2.25
The square root of 2.25 is 1.5
</pre>

### 3.3 为库添加使用要求(usage requirements)
Usage requirements允许更好地控制库或可执行文件的链接和include行，同时也允许更多地控制CMake中目标的可传递属性。利用usage requirements的主要命令有：

* target_compile_definitions()

* target_compile_options()

* target_include_directories()

* target_link_libraries()

这里我们修改上一节(Add a Library)中的CMakeLists.txt代码，以使用cmake的```usage requirements```功能。这里有一点需要指出的是：如果我们需要链接```MathFunctions```库的话，则需要将当前源代码目录包含进来，但是```MathFunctions```库本身不需要包含。因此，这可以看成是```INTERFACE``` usage requirement。

我们可以将```INTERFACE```理解为：things that consumers require but the producer doesn't。

1) 创建相应目录

这里创建Step3目录，然后将上一节中Step2目录下的文件拷贝到Step3:
<pre>
# mkdir Step3 
# cp -ar Step2/* Step3/
# cd Step3 
</pre>

2）为MathFunctions库添加usage requirements 

修改MathFunctions/CMakeLists.txt如下：
{% highlight string %}
add_library(MathFunctions mysqrt.cxx)
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
{% endhighlight %}

3）修改Step3/CMakeList.txt 

在上面我们为```MathFunctions```库添加了usage requirements，这里我们就可以删除Step3/CMakeLists.txt中的```EXTRA_INCLUDES```，此时Step3/CMakeLists.txt内容如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

if(USE_MYMATH)
        add_subdirectory(MathFunctions)
        list(APPEND EXTRA_LIBS MathFunctions)
endif()


# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
{% endhighlight %}

4）测试验证
<pre>
# mkdir Step3_build
# cd Step3_build 
# cmake ../Step3
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
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step3_build

# cmake --build .
Scanning dependencies of target MathFunctions
[ 25%] Building CXX object MathFunctions/CMakeFiles/MathFunctions.dir/mysqrt.cxx.o
[ 50%] Linking CXX static library libMathFunctions.a
[ 50%] Built target MathFunctions
Scanning dependencies of target Tutorial
[ 75%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial

# ./Tutorial 2.25
Computing sqrt of 2.25 to be 1.625
Computing sqrt of 2.25 to be 1.50481
Computing sqrt of 2.25 to be 1.50001
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
The square root of 2.25 is 1.5
</pre>

### 3.4 Installing and Testing 

本节我们介绍一下如何向工程添加```install```规则和```testing```支持。

###### 3.4.1 Install Rules
install规则非常简单： 对于上面的```MathFunctions```，我们希望安装对应的lib库和头文件；而对于应用来说，我们希望安装对应的可执行文件和生成的配置头文件。参看下面步骤：

1） 创建相应目录

这里创建Step4目录，然后将上一节中Step3目录下的文件拷贝到Step4:
<pre>
# mkdir Step4 
# cp -ar Step3/* Step4/
# cd Step4 
</pre>

2） 修改Step4/MathFunctions/CMakeLists.txt文件
{% highlight string %}
add_library(MathFunctions mysqrt.cxx)
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

3) 修改Step4/CMakeLists.txt文件 

{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

if(USE_MYMATH)
        add_subdirectory(MathFunctions)
        list(APPEND EXTRA_LIBS MathFunctions)
endif()


# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )
{% endhighlight %}

通过上面步骤2)、步骤3)，我们就完成了一个基础的```local install```功能。

4）测试验证 

首先执行如下命令进行编译、构建：
<pre>
# mkdir Step4_build
# cd Step4_build 
# cmake ../Step4
# cmake --build . 
</pre>

然后执行```cmake --install```来进行安装(注：对于老版本的cmake，需要执行make install来安装):
<pre>
# cmake --install . 
</pre>

CMake变量```CMAKE_INSTALL_PREFIX```控制安装的根路径，这里我们看到默认的值为```/usr/local```。我们可以使用```--prefix```参数来覆盖该变量的值：
<pre>
# cmake --install . --prefix "/home/myuser/installdir"               //示例

# cmake --install . --prefix "/home/ivanzz1001/workspace/install"
# cd /home/ivanzz1001/workspace/install 
# tree
.
├── bin
│   └── Tutorial
├── include
│   ├── MathFunctions.h
│   └── TutorialConfig.h
├── lib
│   └── libMathFunctions.a
</pre>

###### 3.4.2 Testing Support 
下面我们对应用程序进行测试。修改Step4/CMakeLists.txt文件，启用```testing```功能，并添加一些基本的测试用例：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(USE_MYMATH "Use tutorial provided math implementation" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

if(USE_MYMATH)
        add_subdirectory(MathFunctions)
        list(APPEND EXTRA_LIBS MathFunctions)
endif()


# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC ${EXTRA_LIBS})

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )


enable_testing()

# does the application run
add_test(NAME Runs COMMAND Tutorial 25)

# does the usage message work?
add_test(NAME Usage COMMAND Tutorial)
set_tests_properties(Usage
  PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*number"
  )

# define a function to simplify adding tests
function(do_test target arg result)
  add_test(NAME Comp${arg} COMMAND ${target} ${arg})
  set_tests_properties(Comp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endfunction(do_test)

# do a bunch of result based tests
do_test(Tutorial 4 "4 is 2")
do_test(Tutorial 9 "9 is 3")
do_test(Tutorial 5 "5 is 2.236")
do_test(Tutorial 7 "7 is 2.645")
do_test(Tutorial 25 "25 is 5")
do_test(Tutorial -25 "-25 is [-nan|nan|0]")
do_test(Tutorial 0.0001 "0.0001 is 0.01")
{% endhighlight %}

上面第一个测试只是简单的校验程序可运行，不会出现段错误或崩溃，并且返回值为0。这是CTest的基本形式。

第二个test是利用```PASS_REGULAR_EXPRESSION```属性来校验输出包含指定的字符串。在这种情况下，当输入一个错误的参数导致校验失败，就会打印出相应的usage message。

最后，我们定义了一个do_test()函数来运行```Tutorial```应用程序，并校验计算出来的平方根是否正确。

我们执行如下的命令来看一下测试结果：
<pre>
# make test
Running tests...
Test project /home/ivanzz1001/workspace/test/Step4_build
    Start 1: Runs
1/9 Test #1: Runs .............................   Passed    0.00 sec
    Start 2: Usage
2/9 Test #2: Usage ............................   Passed    0.00 sec
    Start 3: Comp4
3/9 Test #3: Comp4 ............................   Passed    0.00 sec
    Start 4: Comp9
4/9 Test #4: Comp9 ............................   Passed    0.00 sec
    Start 5: Comp5
5/9 Test #5: Comp5 ............................   Passed    0.00 sec
    Start 6: Comp7
6/9 Test #6: Comp7 ............................   Passed    0.00 sec
    Start 7: Comp25
7/9 Test #7: Comp25 ...........................   Passed    0.00 sec
    Start 8: Comp-25
8/9 Test #8: Comp-25 ..........................   Passed    0.00 sec
    Start 9: Comp0.0001
9/9 Test #9: Comp0.0001 .......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 9

Total Test time (real) =   0.02 sec
</pre>





<br />
<br />

**[参看]**

1. [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

2. [cmake官网](https://cmake.org/)

3. [cmake install](https://cmake.org/install/)

4. [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

5. [官方cmake命令](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

<br />
<br />
<br />


