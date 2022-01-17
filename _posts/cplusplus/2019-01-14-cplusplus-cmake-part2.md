---
layout: post
title: cmake的安装及使用(2)
tags:
- cplusplus
categories: cplusplus
description: cmake的安装及使用
---

我们接上文继续讲解cmake的使用。

<!-- more -->

## 1. cmake tutorial 

### 1.1 Adding System Introspection 
下面我们考虑向工程中添加一些依赖于特定目标平台(target platform)的代码实现。比如在下面的例子中，我们会添加```log```和```exp```两个函数，而这些函数可能在某些目标平台上不存在。

假如当前的目标平台支持```log```和```exp```，那么在mysqrt()函数中我们会使用这些函数来计算平方根。首先我们在MathFunctions/CMakeLists.txt中使用```CheckSymbolExists```模块来校验这些函数是否可用。另外在某一些目标平台上，我们可能还需要链接```m```库，然后再次检查是否含有这些函数。

参看如下步骤：

1） 创建相应目录

这里创建Step5目录，然后将上一节中Step4目录下的文件拷贝到Step5:
<pre>
# mkdir Step5
# cp -ar Step4/* Step5/
# cd Step5
</pre>

2）修改Step5/MathFunctions/CMakeLists.txt文件 

修改相应CMakeLists.txt，添加如下：
{% highlight string %}
include(CheckSymbolExists)
check_symbol_exists(log "math.h" HAVE_LOG)
check_symbol_exists(exp "math.h" HAVE_EXP)
if(NOT (HAVE_LOG AND HAVE_EXP))
  unset(HAVE_LOG CACHE)
  unset(HAVE_EXP CACHE)
  set(CMAKE_REQUIRED_LIBRARIES "m")
  check_symbol_exists(log "math.h" HAVE_LOG)
  check_symbol_exists(exp "math.h" HAVE_EXP)
  if(HAVE_LOG AND HAVE_EXP)
    target_link_libraries(MathFunctions PRIVATE m)
  endif()
endif()
{% endhighlight %}

如果对应的log()及exp()函数可用，则使用target_compile_definitions()命令指定```HAVE_LOG```和```HAVE_EXP```作为```PRIVATE```编译选项：
{% highlight string %}
if(HAVE_LOG AND HAVE_EXP)
  target_compile_definitions(MathFunctions
                             PRIVATE "HAVE_LOG" "HAVE_EXP")
endif()
{% endhighlight %}

最后修改后的Step5/MathFunctions/CMakeLists.txt如下所示：
{% highlight string %}
add_library(MathFunctions mysqrt.cxx)


include(CheckSymbolExists)
check_symbol_exists(log "math.h" HAVE_LOG)
check_symbol_exists(exp "math.h" HAVE_EXP)
if(NOT (HAVE_LOG AND HAVE_EXP))
  unset(HAVE_LOG CACHE)
  unset(HAVE_EXP CACHE)
  set(CMAKE_REQUIRED_LIBRARIES "m")
  check_symbol_exists(log "math.h" HAVE_LOG)
  check_symbol_exists(exp "math.h" HAVE_EXP)
  if(HAVE_LOG AND HAVE_EXP)
    target_link_libraries(MathFunctions PRIVATE m)
  endif()
endif()

if(HAVE_LOG AND HAVE_EXP)
  target_compile_definitions(MathFunctions
                             PRIVATE "HAVE_LOG" "HAVE_EXP")
endif()

target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )


install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

3) 修改MathFunctions/mysqrt.cxx 

假如在我们的目标系统上函数log()与exp()可用，则在mysqrt()函数中我们调用其来计算平方根。修改MathFunctions/mysqrt.cxx如下所示：
{% highlight string %}
#include <iostream>
#include <cmath>

// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

#if defined(HAVE_LOG) && defined(HAVE_EXP)
  double result = exp(log(x)*0.5);
  std::cout << "Computing sqrt of " << x << " to be " << result
            << " using log and exp" << std::endl;
#else
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
#endif

  return result;
}
{% endhighlight %}
>注：上面记得添加cmath头文件

4）测试验证 

首先执行如下命令进行编译、构建：
<pre>
# mkdir Step5_build
# cd Step5_build 
# cmake ../Step5
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
-- Looking for log
-- Looking for log - not found
-- Looking for exp
-- Looking for exp - not found
-- Looking for log
-- Looking for log - found
-- Looking for exp
-- Looking for exp - found
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step5_build

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
Computing sqrt of 2.25 to be 1.5 using log and exp
The square root of 2.25 is 1.5
</pre>

### 1.2 Adding a Custom Command and Generated File
这里假设我们并不想要使用目标平台的```log()```和```exp()```函数，取而代之的是我们想要生成一个预计算好(precomputed)的映射表，然后在mysqrt()函数中使用该映射表来计算平方根。在本节中，我们会创建创建一个映射表，并使其作为构建程序的一部分，然后将该映射表直接编译进我们的应用程序。

1） 创建相应目录

这里创建Step6目录，然后将上一节中Step5目录下的文件拷贝到Step6:
<pre>
# mkdir Step6
# cp -ar Step5/* Step6/
# cd Step6
</pre>

2）还原Step6/MathFunctions 

在上一节我们为了支持特定目标平台的```log()```与```exp()```函数，添加了很多额外的东西，这里我们还原回来：

* 还原后MathFunctions/CMakeLists.txt 
{% highlight string %}
add_library(MathFunctions mysqrt.cxx)
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )

install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

* 还原后MathFunctions/mysqrt.cxx 
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

2）添加新文件MakeTable.cxx 

在Step6/MathFunctions目录下添加一个新的源文件MakeTable.cxx，用于产生映射表：
{% highlight string %}
// A simple program that builds a sqrt table
#include <cmath>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
  // make sure we have enough arguments
  if (argc < 2) {
    return 1;
  }

  std::ofstream fout(argv[1], std::ios_base::out);
  const bool fileOpen = fout.is_open();
  if (fileOpen) {
    fout << "double sqrtTable[] = {" << std::endl;
    for (int i = 0; i < 10; ++i) {
      fout << sqrt(static_cast<double>(i)) << "," << std::endl;
    }
    // close the table with a zero
    fout << "0};" << std::endl;
    fout.close();
  }
  return fileOpen ? 0 : 1; // return 0 if wrote the file
}
{% endhighlight %}

上面我们看到，这是用C++编写的程序，通过参数传递输出文件的名称。

3）修改Step6/MathFunctions/CMakeLists.txt 

在这里我们需要向MathFunctions/CMakeLists.txt中添加适当的命令以构建```MakeTable```可执行文件，然后运行该可执行文件使其作为构建Step6整个工程的一部分。我们只需要添加少数几个命令即可实现。

首先在MathFunctions/CMakeLists.txt的开头，类似于编译其他可执行文件一样添加构建```MakeTable```可执行文件：
{% highlight string %}
add_executable(MakeTable MakeTable.cxx)
{% endhighlight %}

然后添加一个自定义命令，用于指定如何通过运行MakeTable来产生```Table.h```头文件：
{% highlight string %}
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  DEPENDS MakeTable
  )
{% endhighlight %}

之后，我们需要让CMake知道mysqrt.cxx依赖于生成的```Table.h```头文件。这可以通过将生成的Table.h头文件添加到MathFunctions库源代码列表中来完成：
{% highlight string %}
add_library(MathFunctions
            mysqrt.cxx
            ${CMAKE_CURRENT_BINARY_DIR}/Table.h
            )
{% endhighlight %}

之后，我们还需要将当前的binary directory添加到include directories列表，这样mysqrt.cxx就能找到并包含该头文件：
{% highlight string %}
target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
          )
{% endhighlight %}

最后，我们的Step6/MathFunctions/CMakeLists.txt内容如下：
{% highlight string %}
add_executable(MakeTable MakeTable.cxx)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  DEPENDS MakeTable
  )

add_library(MathFunctions
            mysqrt.cxx
            ${CMAKE_CURRENT_BINARY_DIR}/Table.h
            )


target_include_directories(MathFunctions
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
          )

install(TARGETS MathFunctions DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

4）修改mysqrt.cxx

这里我们修改mysqrt.cxx文件，让其包含所生成的Table.h头文件，然后重写mysqrt()函数：
{% highlight string %}
#include <iostream>
#include "Table.h"

// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

  // use the table to help find an initial value
  double result = x;
  if (x >= 1 && x < 10) {
    std::cout << "Use the table to help find an initial value " << std::endl;
    result = sqrtTable[static_cast<int>(x)];
  }else{
    // do ten iterations
    for (int i = 0; i < 10; ++i) {
      if (result <= 0) {
        result = 0.1;
      }
      double delta = x - (result * result);
      result = result + 0.5 * delta / result;
      std::cout << "Computing sqrt of " << x << " to be " << result << std::endl;
	}
  }

  return result;
}
{% endhighlight %}

5) 验证测试 

执行如下命令进行验证测试：
<pre>
# mkdir Step6_build
# cd Step6_build 
# cmake ../Step6
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
-- Build files have been written to: /home/ivanzz1001/workspace/test/Step6_build

# cmake --build .
Scanning dependencies of target MakeTable
[ 14%] Building CXX object MathFunctions/CMakeFiles/MakeTable.dir/MakeTable.cxx.o
[ 28%] Linking CXX executable MakeTable
[ 28%] Built target MakeTable
[ 42%] Generating Table.h
Scanning dependencies of target MathFunctions
[ 57%] Building CXX object MathFunctions/CMakeFiles/MathFunctions.dir/mysqrt.cxx.o
[ 71%] Linking CXX static library libMathFunctions.a
[ 71%] Built target MathFunctions
Scanning dependencies of target Tutorial
[ 85%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial

# ./Tutorial 2.25
Use the table to help find an initial value 
The square root of 2.25 is 1.41421
</pre>

在这个工程中，其首先会构建```MakeTable```可执行文件，然后运行```MakeTable```产生```Table.h```头文件。之后编译mysqrt.cxx产生MathFunctions库。

### 1.3 Package an Installer 

假设我们想要发布工程(project)给其他人使用，需要提供针对不同目标平台的二进制包(binary package)以及源代码包(source package)。在前面的```Install and Testing```章节所介绍的是通过编译源代码生成二进制包(binary package)来实现安装，这与本文
所介绍的Package an Installer还是有些区别。在本例子中，我们构建出来的安装包支持二进制安装和包管理特征(feature)。这里我们会使用```CPack```来创建指定目标平台的```installer```。

1） 创建相应目录

这里创建Step7目录，然后将上一节中Step6目录下的文件拷贝到Step7:
<pre>
# mkdir Step7
# cp -ar Step6/* Step7/
# cd Step7
</pre>

2）修改Step7/CMakeLists.txt 

在CMakeLists.txt文件的末尾添加如下：
{% highlight string %}
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include(CPack)
{% endhighlight %}

修改完成后的CMakeLists.txt如下所示：
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

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include(CPack)
{% endhighlight %}
上面就是实现```installer```功能所需的所有代码了。首先通过include命令包含了```InstallRequiredSystemLibraries```，该模块会包含工程在当前平台运行所需要的所有runtime libraries。接下来就是设置一些CPack变量用于指定
License的安装位置和工程的版本信息等。其中版本信息我们在前面的章节中已经介绍过了如何设置，而license.txt则直接在Step7根目录下：
{% highlight string %}
This is the open source License.txt file introduced in
CMake/Tutorial/Step7...
{% endhighlight %}

最后一行就是通过include命令包含```CPack```模块，该模块会使用上面设置的这些变量和当前系统的一些其他属性来构建安装包(installer)。

3）构建工程，并运行cpack可执行文件

接下来我们执行如下命令构建工程：
<pre>
# mkdir Step7_build
# cd Step7_build
# cmake ../Step7 
# cmake --build .
# cpack
CPack: Create package using STGZ
CPack: Install projects
CPack: - Run preinstall target for: Tutorial
CPack: - Install project: Tutorial []
CPack: Create package
CPack: - package: /home/ivanzz1001/workspace/test/Step7_build/Tutorial-1.0-Linux.sh generated.
CPack: Create package using TGZ
CPack: Install projects
CPack: - Run preinstall target for: Tutorial
CPack: - Install project: Tutorial []
CPack: Create package
CPack: - package: /home/ivanzz1001/workspace/test/Step7_build/Tutorial-1.0-Linux.tar.gz generated.
CPack: Create package using TZ
CPack: Install projects
CPack: - Run preinstall target for: Tutorial
CPack: - Install project: Tutorial []
CPack: Create package
CPack: - package: /home/ivanzz1001/workspace/test/Step7_build/Tutorial-1.0-Linux.tar.Z generated.

# ls
CMakeCache.txt       CPackConfig.cmake        CTestTestfile.cmake   MathFunctions          Tutorial-1.0-Linux.tar.gz
CMakeFiles           _CPack_Packages          install_manifest.txt  Tutorial               Tutorial-1.0-Linux.tar.Z
cmake_install.cmake  CPackSourceConfig.cmake  Makefile              Tutorial-1.0-Linux.sh  TutorialConfig.h
</pre>
上面我们看到生成了Tutorial-1.0-Linux.sh、Tutorial-1.0-Linux.tar.gz、Tutorial-1.0-Linux.tar.Z等文件。

我们也可以通过```-G```选项指定generator，另外如果有多配置(multi-config)构建，可以通过使用```-C```选项。例如：
<pre>
# cpack -G ZIP -C Debug
</pre>

如果要生成源代码发布包，我们可以使用如下命令：
<pre>
cpack --config CPackSourceConfig.cmake
</pre>


### 1.4 Adding Support for a Testing Dashboard
在前面的章节Testing Support中我们介绍了cmake对测试的支持，现在我们介绍如何将这些测试结果提交到darshboard。

1） 创建相应目录 

这里创建Step8目录，然后将上一节中Step7目录下的文件拷贝到Step8:
<pre>
# mkdir Step8
# cp -ar Step7/* Step8/
# cd Step8
</pre>

2） 修改Step8/CMakeLists.txt 

我们只需要将```enable_testing()```更换为```include(CTest)```:
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


# enable dashboard scripting
include(CTest)

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

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include(CPack)
{% endhighlight %}

经过上面修改之后，CTest就会自动调用enable_testing()，这样我们就可以将其从CMake文件中移除。

3）创建CTestConfig.cmake文件 

我们需要在Step8/目录下创建CTestConfig.cmake文件，然后在里面指定工程名以及将测试结果提交到何处：
{% highlight string %}
set(CTEST_PROJECT_NAME "CMakeTutorial")
set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=CMakeTutorial")
set(CTEST_DROP_SITE_CDASH TRUE)
{% endhighlight %}

在执行```ctest```命令时就会读取该文件。

4） 创建dashboard 

要创建一个dashboard的话，我们可以先运行cmake来配置工程，然后**不要**构建(build)，而是执行如下命令：
<pre>
ctest [-VV] -D Experimental
</pre>

下面我们创建Step8_build目录，然后尝试执行上面的命令：
<pre>
# mkdir Step8_build 
# cd Step8_build 
# cmake ../Step8 
# ctest -VV -D Experimental
</pre>

通过执行上面```ctest```命令，我们可以构建(build)并测试(test)整个工程，然后将测试结果提交到[Kitware的开放dashboard](https://my.cdash.org/index.php?project=CMakeTutorial)中

### 1.5 Selecting Static or Shared Libraries 
在本章我们会介绍如何通过```BUILD_SHARED_LIBS```变量控制add_library()的行为，以及在不显式指定类型(STATIC、SHARED、MODULE、OBJECT)时如何影响库的构建。

1） 创建相应目录 
<pre>
# mkdir Step9
# cp -ar Step8/* Step9/
# cd Step9
</pre>

2）修改Step9/CMakeLists.txt文件

我们需要在Step9/CMakeLists.txt中添加```BUILD_SHARED_LIBS```。这里我们使用option()命令来设置，这样可以使得用户可以自行选择相应选项的开关:
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# configure a header file to pass the version number only
configure_file(TutorialConfig.h.in TutorialConfig.h)

# add the MathFunctions library
add_subdirectory(MathFunctions)

# add the executable
add_executable(Tutorial tutorial.cxx)
target_link_libraries(Tutorial PUBLIC MathFunctions)
{% endhighlight %}

最终修改后，Step9/CMakeLists.txt如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

#add the MathFunctions library
add_subdirectory(MathFunctions)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC MathFunctions)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )


# enable dashboard scripting
include(CTest)

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

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include(CPack)
{% endhighlight %}
上面我们将```MathFunctions```库改为必须使用到的库，这样我们就必须更新该lib库的一些逻辑。

3）修改Step9/MathFunctions/CMakeLists.txt 

修改MathFunctions/CMakeLists.txt文件，使得当```USE_MYMATH```启用时能够自动的构建和安装该```SqrtLibrary```。这里作为例子，我们将SqrtLibrary构建为静态的。
{% highlight string %}
# add the library that runs
add_library(MathFunctions MathFunctions.cxx)

# state that anybody linking to us needs to include the current source dir
# to find MathFunctions.h, while we don't.
target_include_directories(MathFunctions
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                           )

# should we use our own math functions
option(USE_MYMATH "Use tutorial provided math implementation" ON)
if(USE_MYMATH)

  target_compile_definitions(MathFunctions PRIVATE "USE_MYMATH")

  # first we add the executable that generates the table
  add_executable(MakeTable MakeTable.cxx)

  # add the command to generate the source code
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    DEPENDS MakeTable
    )

  # library that just does sqrt
  add_library(SqrtLibrary STATIC
              mysqrt.cxx
              ${CMAKE_CURRENT_BINARY_DIR}/Table.h
              )

  # state that we depend on our binary dir to find Table.h
  target_include_directories(SqrtLibrary PRIVATE
                             ${CMAKE_CURRENT_BINARY_DIR}
                             )

  target_link_libraries(MathFunctions PRIVATE SqrtLibrary)
endif()

# define the symbol stating we are using the declspec(dllexport) when
# building on windows
target_compile_definitions(MathFunctions PRIVATE "EXPORTING_MYMATH")

# install rules
set(installable_libs MathFunctions)
if(TARGET SqrtLibrary)
  list(APPEND installable_libs SqrtLibrary)
endif()
install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

4) 增加Step9/MathFunctions/mysqrt.h头文件 
{% highlight string %}
namespace mathfunctions {
namespace detail {
double mysqrt(double x);
}
}
{% endhighlight %}

5）修改MathFunctions/mysqrt.cxx 

下面我们修改MathFunctions/mysqrt.cxx为使用mathfunctions和detail命名空间：
{% highlight string %}
#include <iostream>

#include "mysqrt.h"

// include the generated table
#include "Table.h"

namespace mathfunctions {
namespace detail {
// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

  // use the table to help find an initial value
  double result = x;
  if (x >= 1 && x < 10) {
    std::cout << "Use the table to help find an initial value " << std::endl;
    result = sqrtTable[static_cast<int>(x)];
  }

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
}
}
{% endhighlight %}

6）修改Step9/tutorial.cxx 

这里我们需要对Step9/tutorial.cxx也做一些修改，使得tutorial.cxx不再使用```USE_MYMATH```:

* Always include MathFunctions.h

* Always use mathfunctions::sqrt

* Don't include cmath

修改后如下所示：
{% highlight string %}
// A simple program that computes the square root of a number
#include <iostream>
#include <string>
#include "TutorialConfig.h"
#include "MathFunctions.h"

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

  const double outputValue = mathfunctions::sqrt(inputValue);

  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
{% endhighlight %}

7）修改Step9/MathFunctions/MathFunctions.h

我们修改MathFunctions.h头文件，将其放入mathfunctions名称空间，并使用dll导出形式来定义：
{% highlight string %}
#if defined(_WIN32)
#  if defined(EXPORTING_MYMATH)
#    define DECLSPEC __declspec(dllexport)
#  else
#    define DECLSPEC __declspec(dllimport)
#  endif
#else // non windows
#  define DECLSPEC
#endif

namespace mathfunctions {
double DECLSPEC sqrt(double x);
}
{% endhighlight %}

8）增加Step9/MathFunctions/MathFunctions.cxx 

内容如下：
{% highlight string %}
#include "MathFunctions.h"

#include <cmath>

#ifdef USE_MYMATH
#  include "mysqrt.h"
#endif

namespace mathfunctions {
double sqrt(double x)
{
#ifdef USE_MYMATH
  return detail::mysqrt(x);
#else
  return std::sqrt(x);
#endif
}
}
{% endhighlight %}

9) 测试验证 

<pre>
# mkdir Step9_build 
# cd Step9_build 
# cmake ../Step9 
# cmake --build .
[ 11%] Building CXX object MathFunctions/CMakeFiles/MakeTable.dir/MakeTable.cxx.o
[ 22%] Linking CXX executable ../MakeTable
[ 22%] Built target MakeTable
[ 33%] Generating Table.h
[ 44%] Building CXX object MathFunctions/CMakeFiles/SqrtLibrary.dir/mysqrt.cxx.o
[ 55%] Linking CXX static library ../libSqrtLibrary.a
[ 55%] Built target SqrtLibrary
[ 66%] Building CXX object MathFunctions/CMakeFiles/MathFunctions.dir/MathFunctions.cxx.o
[ 77%] Linking CXX shared library ../libMathFunctions.so
/usr/bin/ld: ../libSqrtLibrary.a(mysqrt.cxx.o): relocation R_X86_64_32 against `.rodata' can not be used when making a shared object; recompile with -fPIC
../libSqrtLibrary.a: error adding symbols: 错误的值
collect2: 错误：ld 返回 1
gmake[2]: *** [libMathFunctions.so] 错误 1
gmake[1]: *** [MathFunctions/CMakeFiles/MathFunctions.dir/all] 错误 2
gmake: *** [all] 错误 2
</pre>

在执行```cmake --build```时，我们注意到链接失败，提示的原因是：在链接时将```非PIC```的静态库(libSqrtLibrary.a)与```PIC```的动态库(libMathFunctions.so)合并失败。
要解决这个问题，我们需要显式的指明无论何种构建类型(static、dynamic、module、object)下，都将```SqrtLibrary```这个target的``` POSITION_INDEPENDENT_CODE```属性为True。修改MathFunctions/CMakeLists.txt，增加如下：
{% highlight string %}
# state that SqrtLibrary need PIC when the default is shared libraries
set_target_properties(SqrtLibrary PROPERTIES
					POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
					)

target_link_libraries(MathFunctions PRIVATE SqrtLibrary)
{% endhighlight %}

最终Step9/MathFunctions/CMakeLists.txt如下：
{% highlight string %}
# add the library that runs
add_library(MathFunctions MathFunctions.cxx)

# state that anybody linking to us needs to include the current source dir
# to find MathFunctions.h, while we don't.
target_include_directories(MathFunctions
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                           )

# should we use our own math functions
option(USE_MYMATH "Use tutorial provided math implementation" ON)
if(USE_MYMATH)

  target_compile_definitions(MathFunctions PRIVATE "USE_MYMATH")

  # first we add the executable that generates the table
  add_executable(MakeTable MakeTable.cxx)

  # add the command to generate the source code
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    DEPENDS MakeTable
    )

  # library that just does sqrt
  add_library(SqrtLibrary STATIC
              mysqrt.cxx
              ${CMAKE_CURRENT_BINARY_DIR}/Table.h
              )

  # state that we depend on our binary dir to find Table.h
  target_include_directories(SqrtLibrary PRIVATE
                             ${CMAKE_CURRENT_BINARY_DIR}
                             )
  # state that SqrtLibrary need PIC when the default is shared libraries
  set_target_properties(SqrtLibrary PROPERTIES
                        POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
                        )

  target_link_libraries(MathFunctions PRIVATE SqrtLibrary)
endif()

# define the symbol stating we are using the declspec(dllexport) when
# building on windows
target_compile_definitions(MathFunctions PRIVATE "EXPORTING_MYMATH")

# install rules
set(installable_libs MathFunctions)
if(TARGET SqrtLibrary)
  list(APPEND installable_libs SqrtLibrary)
endif()
install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}

然后再进行测试：
<pre>
# cd Step9_build 
# rm -rf *
# cmake ../Step9 
# cmake --build .
[ 11%] Building CXX object MathFunctions/CMakeFiles/MakeTable.dir/MakeTable.cxx.o
[ 22%] Linking CXX executable ../MakeTable
[ 22%] Built target MakeTable
[ 33%] Generating Table.h
[ 44%] Building CXX object MathFunctions/CMakeFiles/SqrtLibrary.dir/mysqrt.cxx.o
[ 55%] Linking CXX static library ../libSqrtLibrary.a
[ 55%] Built target SqrtLibrary
[ 66%] Building CXX object MathFunctions/CMakeFiles/MathFunctions.dir/MathFunctions.cxx.o
[ 77%] Linking CXX shared library ../libMathFunctions.so
[ 77%] Built target MathFunctions
[ 88%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial

# ./Tutorial 2.25
Use the table to help find an initial value 
Computing sqrt of 2.25 to be 1.5026
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
The square root of 2.25 is 1.5
</pre>

### 1.6 Adding Generator Expressions

生成表达式(Generator expressions)会在构建系统生成构建配置(build configuration)时产生作用。在许多target properties上下文中都允许使用```generator expressions```。此外，
也可以在一些cmake的命令中使用生成表达式，如 target_link_libraries(), target_include_directories(), target_compile_definitions()等。

generator expressions可用于conditional linking、编译时的conditional definitions、conditional include directories等。

Generator Expressions有多种类型，包括：

* Logical expressions 

* Informational expressions 

* Output expressions

其中，Logical expressions被用于创建条件输出，基本有两种：```0表达式```和```1表达式```。```$<0:...>```产生一个空字符串；```$<1:...>```产生```...```的内容。

生成表达式(generator expressions)的一个常见用途是为编译器(compiler)添加相关选项，比如language levels、warnings等。一个经典的做法是为```INTERFACE``` target指明一些特性，比如指定其要使用C++11标准。

1） 创建相应目录
<pre>
# mkdir Step10
# cp -ar Step9/* Step10/
# cd Step10
</pre>

2）修改Step10/CMakeLists.txt以设置C++标准

这里我们不使用``` CMAKE_CXX_STANDARD```来进行设置，因此如下代码：
<pre>
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
</pre>

会被替换为：
<pre>
add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)
</pre>

3）修改Step10/CMakeLists.txt以设置编译器警告标识 

这里我们为工程设置编译器的警告flags。由于warning flags在不同的编译器之间差别很大，因此我们使用```COMPILE_LANG_AND_ID```生成表达式来控制不同的编译器使用不同的编译参数：
{% highlight string %}
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)
{% endhighlight %}
上面我们看到，warning flags都被封装在```BUILD_INTERFACE```条件中。通过这样，其他使用我们工程的用户将不会继承这些warning flags。

最后修改后的Step10/CMakeLists.txt如下：
{% highlight string %}
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Tutorial VERSION 1.0)

add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

configure_file(TutorialConfig.h.in TutorialConfig.h)

#add the MathFunctions library
add_subdirectory(MathFunctions)

# add the executable 
add_executable(Tutorial tutorial.cxx)

target_link_libraries(Tutorial PUBLIC MathFunctions)

target_include_directories(Tutorial PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

install(TARGETS Tutorial DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )


# enable dashboard scripting
include(CTest)

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

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include(CPack)
{% endhighlight %}

4）修改Step10/MathFunctions/CMakeLists.txt 
{% highlight string %}
# add the library that runs
add_library(MathFunctions MathFunctions.cxx)

# state that anybody linking to us needs to include the current source dir
# to find MathFunctions.h, while we don't.
target_include_directories(MathFunctions
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
                           )

# should we use our own math functions
option(USE_MYMATH "Use tutorial provided math implementation" ON)
if(USE_MYMATH)

  target_compile_definitions(MathFunctions PRIVATE "USE_MYMATH")

  # first we add the executable that generates the table
  add_executable(MakeTable MakeTable.cxx)

  # add the command to generate the source code
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
    DEPENDS MakeTable
    )

  # library that just does sqrt
  add_library(SqrtLibrary STATIC
              mysqrt.cxx
              ${CMAKE_CURRENT_BINARY_DIR}/Table.h
              )

  # state that we depend on our binary dir to find Table.h
  target_include_directories(SqrtLibrary PRIVATE
                             ${CMAKE_CURRENT_BINARY_DIR}
                             )
  # state that SqrtLibrary need PIC when the default is shared libraries
  set_target_properties(SqrtLibrary PROPERTIES
                        POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
                        )

  target_link_libraries(MathFunctions PRIVATE tutorial_compiler_flags SqrtLibrary)
endif()

# define the symbol stating we are using the declspec(dllexport) when
# building on windows
target_compile_definitions(MathFunctions PRIVATE "EXPORTING_MYMATH")

# install rules
set(installable_libs MathFunctions)
if(TARGET SqrtLibrary)
  list(APPEND installable_libs SqrtLibrary)
endif()
install(TARGETS ${installable_libs} DESTINATION lib)
install(FILES MathFunctions.h DESTINATION include)
{% endhighlight %}
上面在target_link_libraries中调用target_link_libraries。

5）测试验证 
<pre>
# mkdir Step10_build
# cd Step10_build 
# cmake ../Step10 
# cmake --build .
# ./Tutorial 2.25
Use the table to help find an initial value 
Computing sqrt of 2.25 to be 1.5026
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
Computing sqrt of 2.25 to be 1.5
The square root of 2.25 is 1.5
</pre>



<br />
<br />

**[参看]**

1. [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

2. [cmake官网](https://cmake.org/)

3. [cmake install](https://cmake.org/install/)

4. [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

5. [官方cmake命令](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

6. [cmake modules](https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html)

<br />
<br />
<br />


