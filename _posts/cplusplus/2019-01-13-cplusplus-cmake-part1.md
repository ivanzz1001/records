---
layout: post
title: cmake的使用
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文先概要性的讲解cmake的使用。参看：

- [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

- [cmake官网](https://cmake.org/)

- [cmake install](https://cmake.org/install/)

- [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

- [官方cmake命令](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

- [cmake modules](https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html)




<!-- more -->


## 1. cmake使用概览

CMake涉及到的命令行工具主要包括三个：

- cmake
- ctest
- cpack

对上述命令行工具的使用这里并不打算介绍。其实我们可以将CMakefile看成是一门编程语言，从我们过往学习一门编程语言的经验来看，我们通常需要先大致掌握：程序的结构、变量的使用、循环及条件控制、函数库等。这里我们按照这一思路先讲解下列内容，以便尽快的入门CMakeFile的编写：

- cmake-buildsystem: 构建系统
- cmake-commands: 相关命令
- cmake-env-variables: 环境变量


## 2. cmake-buildsystem
 
一个基于CMake的buildsystem是由一系列顶层的逻辑target组成，这些target可以是：

- 可执行文件

- library

- 自定义target

在buildsystem中target之间的依赖也是可以被表述的，并以此来决定各个target的构建顺序。


### 2.1 Binary Targets

对于生成可执行文件这种类型的target，我们可以使用add_executable()命令来指定；对于生成library这种类型的target，我们可以使用add_library()命令来指定； executable target与library target之间的依赖可以使用target_link_libraries()命令来指定。例如：

```
add_library(archive archive.cpp zip.cpp lzma.cpp)
add_executable(zipapp zipapp.cpp)
target_link_libraries(zipapp archive)
```
上面archive被指定为了一个静态链接库，其通过编译archive.cpp、zip.cpp、lzma.cpp来生成；zipapp被指定为了一个可执行文件，其通过编译并链接zipapp.cpp；当链接zipapp这样一个可执行程序时，会将archive静态库链接进去。


- ### Binary Executables

    使用add_executable()命令来指定一个```二进制可执行程序```target，例如：

    ```
      add_executable(mytool mytool.cpp)
    ```

- ### Binary Library Types
    上面我们讲到可以使用add_library()来指定library target，而library我们比较熟悉的有动态链接库、静态链接库，当然还有其他， 我们作如下划分：

    - Normal Libraries
    
    - Apple Frameworks

    - Object Libraries

对于Apple Frameworks以及Object Libraries这两种library target，我们不进行介绍。对于Normal Libraries可以细分为静态链接库和动态链接库，分别使用如下命令来指定:

- 动态链接库target
   ```
    add_library(archive SHARED archive.cpp zip.cpp lzma.cpp)
   ```

- 静态链接库target
    >ps: 默认情况下为静态链接库
    ```
    add_library(archive STATIC archive.cpp zip.cpp lzma.cpp)
    ```


## 3. cmake命令
cmake命令整体上可分为:

- Scripting commands

- Project commands

- CTest Commands

- Deprecated Commands


1) Scriptings commands

脚本命令任何地方可以使用，主要含有如下：

- block
- break
- cmake_host_system_information
- cmake_language
- cmake_minimum_required
- cmake_parse_arguments
- cmake_path
- cmake_policy
- configure_file
- continue
- else
- elseif
- endblock
- endforeach
- endfunction
- endif
- endmacro
- endwhile
- execute_process
- file
- find_file
- find_library
- find_package
- find_path
- find_program
- foreach
- function
- get_cmake_property
- get_directory_property
- get_filename_component
- get_property
- if
- include
- include_guard
- list
- macro
- mark_as_advanced
- math
- message
- option
- return
- separate_arguments
- set
- set_directory_properties
- set_property
- site_name
- string
- unset
- variable_watch
- while

可以看到，其包含了条件循环控制、变量定义等命令。


2） Project commands

这些命令只能用于cmake projects中：

- add_compile_definitions
- add_compile_options
- add_custom_command
- add_custom_target
- add_definitions
- add_dependencies
- add_executable
- add_library
- add_link_options
- add_subdirectory
- add_test
- aux_source_directory
- build_command
- cmake_file_api
- create_test_sourcelist
- define_property
- enable_language
- enable_testing
- export
- fltk_wrap_ui
- get_source_file_property
- get_target_property
- get_test_property
- include_directories
- include_external_msproject
- include_regular_expression
- install
- link_directories
- link_libraries
- load_cache
- project
- remove_definitions
- set_source_files_properties
- set_target_properties
- set_tests_properties
- source_group
- target_compile_definitions
- target_compile_features
- target_compile_options
- target_include_directories
- target_link_directories
- target_link_libraries
- target_link_options
- target_precompile_headers
- target_sources
- try_compile
- try_run

## 4. CMakefile的一个简单示例
我们编写一个简单的程序，然后看如何使用CMake来编译：

1） 编写程序`chapter1/tutorial.cxx`

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


2) 编写CMakeLists.txt文件

```
cmake_minimum_required(VERSION 3.10)

# set the project name
project(Tutorial)

# add the executable 
add_executable(Tutorial tutorial.cxx)

install(TARGETS Tutorial
        RUNTIME
           DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
       
```

>ps: add_executable()必须工作在cmake project下，因此我们定义了一个Tutorial project

3）编写构建脚本prebuild.sh

我们编写一个`prebuild.sh`脚本来进行构建:

```
#!/bin/bash

CURRENT_DIR=$(pwd)
PREBUILT_DIR=${CURRENT_DIR}/prebuilt_dir
export PATH=${PREBUILT_DIR}:$PATH
nproc=2


function build_tutorial()
{
  cd ${CURRENT_DIR}/chapter1 && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=${PREBUILT_DIR} -B build && cmake --build build -j ${nproc} && cmake --build build --target install

  if [ $? -ne 0 ]; then
    echo "build tutorial failed"
    exit
  fi

  echo "BUILD tutorial COMPLETED"
}

function do_build()
{
  build_tutorial
}

function do_clean()
{
   echo "current do nothing"
}

if [[ $1 == "clean" ]]; then
  do_clean
else
  do_build
fi
```

4) 使用脚本构建程序

```
# ./prebuild.sh 
-- The C compiler identification is GNU 11.4.0
-- The CXX compiler identification is GNU 11.4.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /root/learn_cmake/chapter1/build
[ 50%] Building CXX object CMakeFiles/Tutorial.dir/tutorial.cxx.o
[100%] Linking CXX executable Tutorial
[100%] Built target Tutorial
Consolidate compiler generated dependencies of target Tutorial
[100%] Built target Tutorial
Install the project...
-- Install configuration: "RelWithDebInfo"
-- Installing: /root/learn_cmake/prebuilt_dir/bin/Tutorial
BUILD tutorial COMPLETED

# tree
.
├── chapter1
│   ├── build
│   │   ├── CMakeCache.txt
│   │   ├── CMakeFiles
│   │   │   ├── 3.22.1
│   │   │   │   ├── CMakeCCompiler.cmake
│   │   │   │   ├── CMakeCXXCompiler.cmake
│   │   │   │   ├── CMakeDetermineCompilerABI_C.bin
│   │   │   │   ├── CMakeDetermineCompilerABI_CXX.bin
│   │   │   │   ├── CMakeSystem.cmake
│   │   │   │   ├── CompilerIdC
│   │   │   │   │   ├── a.out
│   │   │   │   │   ├── CMakeCCompilerId.c
│   │   │   │   │   └── tmp
│   │   │   │   └── CompilerIdCXX
│   │   │   │       ├── a.out
│   │   │   │       ├── CMakeCXXCompilerId.cpp
│   │   │   │       └── tmp
│   │   │   ├── cmake.check_cache
│   │   │   ├── CMakeDirectoryInformation.cmake
│   │   │   ├── CMakeOutput.log
│   │   │   ├── CMakeTmp
│   │   │   ├── Makefile2
│   │   │   ├── Makefile.cmake
│   │   │   ├── progress.marks
│   │   │   ├── TargetDirectories.txt
│   │   │   └── Tutorial.dir
│   │   │       ├── build.make
│   │   │       ├── cmake_clean.cmake
│   │   │       ├── compiler_depend.internal
│   │   │       ├── compiler_depend.make
│   │   │       ├── compiler_depend.ts
│   │   │       ├── DependInfo.cmake
│   │   │       ├── depend.make
│   │   │       ├── flags.make
│   │   │       ├── link.txt
│   │   │       ├── progress.make
│   │   │       ├── tutorial.cxx.o
│   │   │       └── tutorial.cxx.o.d
│   │   ├── cmake_install.cmake
│   │   ├── install_manifest.txt
│   │   ├── Makefile
│   │   └── Tutorial
│   ├── CMakeLists.txt
│   └── tutorial.cxx
├── prebuild.sh
└── prebuilt_dir
    └── bin
        └── Tutorial

12 directories, 37 files
```








<br />
<br />
<br />


