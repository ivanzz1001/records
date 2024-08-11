---
layout: post
title: CMakeLists脚本的编写
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文并不打算十分详细的介绍如何编写CMakeLists.txt脚本文件，而是先大致介绍一下其包含的几个重要方面：

- 变量定义

- 条件控制

- 循环控制

- 函数

- macro

- 文件包含

- target(前文已介绍）

- install

- CPack

知道了上面这些，就基本可以编写大部分CMake编译脚本了。

这里建议在参考[cmake官网文档](https://cmake.org/)的同时，最好从GitHub上下载一个其他人编写的CMake编译工程示例，以便我们交叉对照的学习。这里我们从GitHub上克隆`gflags`:

```
# git clone https://github.com/protocolbuffers/protobuf.git 

```

参看：

- [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

- [cmake官网](https://cmake.org/)

- [cmake install](https://cmake.org/install/)

- [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

- [官方cmake命令](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

- [cmake modules](https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html)




<!-- more -->


## 1. 设置变量

变量是CMake语言的基本存储单元， 其值类型均为字符串类型(ps: 尽管某些命令可能会将其解释为其他类型）。我们一般使用`set`或者`unset`命令来显式的设置、取消变量，但也有一些其他的命令具有修改变量值的语义。


一个变量有对应的作用范围：

- Block scope
    
    block()命令中创建的变量具有Block scope

- Function scope
    
    function()命令中创建的变量具有Function scope

- Directory scope

    源代码目录树中的每一个目录均有其自己的变量作用域绑定。在处理一个目录中的CMakeLists.txt文件之前，CMake都会拷贝其父目录中的所有变量绑定。

- Persistent Cache

    CMake保存着一份单独的"cache"变量集合，被称为"cache entries"，这些值可横跨CMake构建工程的多次运行。cache entries有一个单独的绑定域名，除非通过set、unset命令的'CACHE'选项加以修改。


下面我们介绍set命令的使用，我们可以使用该命令设置三种类型变量的值：

- 普通变量(Normal variable)

- 缓存变量(Cache variable)

- 环境变量(Env variable)

>ps: 这里的环境变量是[CMake环境变量](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)), 而不是一般意义上的Linux环境变量

### 1.1 设置普通变量(Normal variable)

基本语法规则：
```
set(<variable> <value>... [PARENT_SCOPE])
```

用于set或unset当前函数或目录作用域中的变量：

- 假如指定了至少一个`<value>...`的话，那么变量将会被设置为指定的值

- 假如并不包含`<value>`，则表示unset一个变量。这等价于unset(<variable>)命令

## 2. 条件控制
控制条件执行一组命令，其基本的语法规则如下：
```
if(<condition>)
  <commands>
elseif(<condition>) # optional block, can be repeated
  <commands>
else()              # optional block
  <commands>
endif()
```

### 2.1 Condition语法
如下的语法规则适用于if、elseif、while命令的condition参数。

1）基本表达式condition

- if(<constant>)

    假如constant的值为1、ON、YES、TRUE、Y、或者非零数字， 则为True；假如constant的值为0、OFF、NO、FALSE、N、IGNORE、空字符串，则为Fasle。

- if(<variable>)

    这里值得注意的是Enviroment variable不能使用此方式来测试。

- if(<string>)
    
    除非字符串的值为true常量，否则其他任何字符串都为false。

2） 逻辑操作符

- if(NOT <condition>)

- if(<cond1> AND <cond2>)¶

- if(<cond1> OR <cond2>

3） 存在性检查

- if(COMMAND <command-name>)

     用于指定的名称是否是command、macro、function，如果是则返回true，否则返回false

- if(POLICY <policy-id>)

- if(TARGET <target-name>)

4) 文件操作

- if(EXISTS <path-to-file-or-directory>)

    假如指定的文件或目录存在且可读的话， 返回true

5) 比较操作

- if(<variable|string> LESS <variable|string>)

- if(<variable|string> GREATER <variable|string>)


6）版本比较

7）路径比较


## 3. 循环控制

循环有两种形式：

- foreach

- while

1) foreach循环

基本语法格式：

```
foreach(<loop_var> <items>)
  <commands>
endforeach()
```

2) while循环

基本语法格式：
```
while(<condition>)
  <commands>
endwhile()
```

## 4. 函数

function的基本语法格式：
```
function(<name> [<arg1> ...])
  <commands>
endfunction()
```


## 5. macro
基本语法格式：

```
macro(<name> [<arg1> ...])
  <commands>
endmacro()
```

## 6. 文件包含
基本语法格式：
```
include(<file|module> [OPTIONAL] [RESULT_VARIABLE <var>]
                      [NO_POLICY_SCOPE])
```

用于加载一个指定的文件或目录。


## 7. install
基本语法格式：
```
install(TARGETS <target>... [...])
install(IMPORTED_RUNTIME_ARTIFACTS <target>... [...])
install({FILES | PROGRAMS} <file>... [...])
install(DIRECTORY <dir>... [...])
install(SCRIPT <file> [...])
install(CODE <code> [...])
install(EXPORT <export-name> [...])
install(RUNTIME_DEPENDENCY_SET <set-name> [...])
```

## 8. 几个常用的命令介绍

### 8.1 add_custom_command与add_custom_target
在CMake构建系统中，add_custom_command和add_custom_target是两个强大的指令，它们允许我们添加自定义的构建规则。这两个指令在复杂的项目中特别有用，因为它们允许我们执行一些标准的构建步骤之外的操作。

参看:

- [CMake中的add_custom_command和add_custom_target指令详解](https://cloud.baidu.com/article/3282122)

1) **add_custom_command**

`add_custom_command`指令用于为生成的目标文件添加自定义构建规则。它通常用于在构建过程中生成源代码、头文件或其他文件。这个指令的基本语法如下：
```
add_custom_command(OUTPUT output1 [output2 ...]
                   COMMAND command1 [ARGS] [args1...]
                   [COMMAND command2 [ARGS] [args2...] ...]
                   [MAIN_DEPENDENCY depend]
                   [DEPENDS [depends...]]
                   [BYPRODUCTS [files...]]
                   [IMPLICIT_DEPENDS <lang1> depend1
                                    [<lang2> depend2] ...]
                   [WORKING_DIRECTORY dir]
                   [COMMENT comment]
                   [DEPFILE depfile]
                   [JOB_POOL job_pool]
                   [JOB_SERVER_AWARE <bool>]
                   [VERBATIM] [APPEND] [USES_TERMINAL]
                   [COMMAND_EXPAND_LISTS]
                   [DEPENDS_EXPLICIT_ONLY])
```

>ps: 上面这是add_custom_command指令的一种较为常见的用法

- OUTPUT：指定由命令生成的文件。这些文件将成为后续构建步骤的依赖项。

- COMMAND：要执行的命令。这可以是任何可以在命令行中运行的命令。

- MAIN_DEPENDENCY：可选参数，指定主要依赖项。这通常是一个源文件，当该文件更改时，将重新运行命令。

- DEPENDS：其他依赖项列表。当这些文件更改时，也将重新运行命令。

- IMPLICIT_DEPENDS：隐式依赖项。这允许你指定命令对哪些文件有隐式依赖。

- VERBATIM：如果设置，命令将不会通过CMake的命令行解释器，而是直接传递给构建系统。

- WORKING_DIRECTORY：指定命令的工作目录。

- COMMENT：为构建系统提供的注释，通常用于描述命令的目的。

- PREBUILDS或POSTBUILDS：指定命令是在目标构建之前还是之后运行。

- BYPRODUCTS：指定命令生成的副产品文件。这些文件不会触发重新构建，但如果它们不存在，构建将被视为失败。

2) **add_custom_target**

`add_custom_target`指令用于添加不生成输出文件的自定义目标。这通常用于执行一些不需要生成文件的任务，如运行测试、清理工作区等。它的基本语法如下：
```
add_custom_target(Name [ALL] [command1 [args1...]]
                  [COMMAND command2 [args2...] ...]
                  [DEPENDS depend depend depend ... ]
                  [BYPRODUCTS [files...]]
                  [WORKING_DIRECTORY dir]
                  [COMMENT comment]
                  [JOB_POOL job_pool]
                  [JOB_SERVER_AWARE <bool>]
                  [VERBATIM] [USES_TERMINAL]
                  [COMMAND_EXPAND_LISTS]
                  [SOURCES src1 [src2...]])
```
- target_name：自定义目标的名称。

- ALL：可选参数，如果设置，该目标将被添加到默认构建目标中，即执行make或cmake --build时会自动构建。

- DEPENDS：其他依赖项列表。当这些目标或文件更改时，该目标将被重新构建。

- WORKING_DIRECTORY、COMMAND、VERBATIM、IMPLICIT_DEPENDS和BYPRODUCTS的参数与add_custom_command中的相同。

3) **实际应用**

在实际项目中，add_custom_command和add_custom_target可以非常有用。例如，你可能需要：

- 使用add_custom_command生成由源代码生成的头文件，如使用protobuf工具生成C++头文件。

- 使用add_custom_target运行测试套件，确保代码质量。

- 使用add_custom_target清理构建过程中生成的文件，以保持工作区的整洁。

通过合理使用这两个指令，你可以极大地扩展CMake的构建能力，使其适应各种复杂的构建需求。


4) **execute_process**

`execute_process`指令用于执行一个或多个子进程。及基本语法规则如下：
```
execute_process(COMMAND <cmd1> [<arguments>]
                [COMMAND <cmd2> [<arguments>]]...
                [WORKING_DIRECTORY <directory>]
                [TIMEOUT <seconds>]
                [RESULT_VARIABLE <variable>]
                [RESULTS_VARIABLE <variable>]
                [OUTPUT_VARIABLE <variable>]
                [ERROR_VARIABLE <variable>]
                [INPUT_FILE <file>]
                [OUTPUT_FILE <file>]
                [ERROR_FILE <file>]
                [OUTPUT_QUIET]
                [ERROR_QUIET]
                [COMMAND_ECHO <where>]
                [OUTPUT_STRIP_TRAILING_WHITESPACE]
                [ERROR_STRIP_TRAILING_WHITESPACE]
                [ENCODING <name>]
                [ECHO_OUTPUT_VARIABLE]
                [ECHO_ERROR_VARIABLE]
                [COMMAND_ERROR_IS_FATAL <ANY|LAST>])
```

cmake在```配置阶段```阶段使用该指令来执行命令；而add_custom_command与add_custom_target是在构建阶段执行相应命令。

### 8.2 文件操作指令

`file`指令用于操作文件或路径。基本语法规则如下：
```
Reading
  file(READ <filename> <out-var> [...])
  file(STRINGS <filename> <out-var> [...])
  file(<HASH> <filename> <out-var>)
  file(TIMESTAMP <filename> <out-var> [...])
  file(GET_RUNTIME_DEPENDENCIES [...])

Writing
  file({WRITE | APPEND} <filename> <content>...)
  file({TOUCH | TOUCH_NOCREATE} <file>...)
  file(GENERATE OUTPUT <output-file> [...])
  file(CONFIGURE OUTPUT <output-file> CONTENT <content> [...])

Filesystem
  file({GLOB | GLOB_RECURSE} <out-var> [...] <globbing-expr>...)
  file(MAKE_DIRECTORY <directories>...)
  file({REMOVE | REMOVE_RECURSE } <files>...)
  file(RENAME <oldname> <newname> [...])
  file(COPY_FILE <oldname> <newname> [...])
  file({COPY | INSTALL} <file>... DESTINATION <dir> [...])
  file(SIZE <filename> <out-var>)
  file(READ_SYMLINK <linkname> <out-var>)
  file(CREATE_LINK <original> <linkname> [...])
  file(CHMOD <files>... <directories>... PERMISSIONS <permissions>... [...])
  file(CHMOD_RECURSE <files>... <directories>... PERMISSIONS <permissions>... [...])

Path Conversion
  file(REAL_PATH <path> <out-var> [BASE_DIRECTORY <dir>] [EXPAND_TILDE])
  file(RELATIVE_PATH <out-var> <directory> <file>)
  file({TO_CMAKE_PATH | TO_NATIVE_PATH} <path> <out-var>)

Transfer
  file(DOWNLOAD <url> [<file>] [...])
  file(UPLOAD <file> <url> [...])

Locking
  file(LOCK <path> [...])

Archiving
  file(ARCHIVE_CREATE OUTPUT <archive> PATHS <paths>... [...])
  file(ARCHIVE_EXTRACT INPUT <archive> [...])
```
比如我们使用如下的命令来匹配出src目录下的所有cpp文件：

```
file(GLOB SOURCES "src/*.cpp")
```


如下给出一个示例，用于遍历目录下的所有.proto文件，使用protoc生成.cpp：
```
cmake_minimum_required(VERSION 3.10)
project(protobuf_generation LANGUAGES CXX)
 
find_package(Protobuf REQUIRED)
 
include_directories(${CMAKE_CURRENT_BINARY_DIR})
 
file(GLOB_RECURSE PROTO_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.proto")
 
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
    set(PROTO_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_DIR}")
 
    execute_process(
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        --cpp_out=${PROTO_SRC_DIR}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE})
endforeach()
 
add_executable(protobuf_example main.cpp)
target_link_libraries(protobuf_example protobuf)
```


<br />
<br />
<br />


