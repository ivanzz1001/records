---
layout: post
title: cmake使用教程 - 变量与缓存
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文转载自：

- [变量与缓存](https://modern-cmake-cn.github.io/Modern-CMake-zh_CN/chapters/basics/variables.html)

- [CMake 属性之目标属性 ](https://www.cnblogs.com/mengps/p/18452409)

- [CMake Cache](https://cmake.org/cmake/help/book/mastering-cmake/chapter/CMake%20Cache.html)

<!-- more -->


## 1. 本地变量
我们首先讨论变量，你可以这样声明一个本地(local)变量：

```
set(MY_VARIABLE "value")
```

变量名通常全部用大写，变量值跟在其后。你可以通过`${}`来引用一个变量，例如`${MY_VARIABLE}`。

CMake有作用域的概念，在声明一个变量后，你只可以在它的作用域范围内访问这个变量。如果你将一个函数或 一个文件放到一个子目录中，这个变量将不再被定义。你可以通过在变量声明末尾添加`PARENT_SCOPE`来将它的作用域定为当前的上一级作用域。


列表就是简单的包含一系列变量：
```
set(MY_LIST "one" "two")
```

你也可以通过`;`分隔变量，这和空格的作用是一样的：
```
set(MY_LIST "one;two")
```

有一些和list()进行协同的命令，separate_arguments()可以把一个以空格分隔的字符串分割成一个列表。需要注意的是，在CMake中如果一个值没有空格，那么加和不加引号的效果是一样的。这使你可以在处理知道不可能含有空格的值时不加引号。

当一个变量用 `${}` 括起来的时候，空格的解析规则和上述相同。对于路径来说要特别小心，路径很有可能会包含空格，因此你应该总是将解析变量得到的值用引号括起来，也就是，应该这样 "${MY_PATH}" 。


## 2. 缓存变量

CMake 提供了一个缓存变量来允许你从命令行中设置变量。CMake 中已经有一些预置的变量，像 CMAKE_BUILD_TYPE 。如果一个变量还没有被定义，你可以这样声明并设置它。

```
set(MY_CACHE_VARIABLE "VALUE" CACHE STRING "Description")
```

这么写不会覆盖已定义的值。这是为了让你只能在命令行中设置这些变量，而不会在 CMake 文件执行的时候被重新覆盖。如果你想把这些变量作为一个临时的全局变量，你可以这样做：

```
set(MY_CACHE_VARIABLE "VALUE" CACHE STRING "" FORCE)
mark_as_advanced(MY_CACHE_VARIABLE)
```

第一行将会强制设置该变量的值，第二行将使得用户运行`cmake -L ..`或使用 GUI 界面的时候不会列出该变量。此外，你也可以通过 INTERNAL 这个类型来达到同样的目的（尽管在技术上他会强制使用 STRING 类型，这不会产生任何的影响）：

```
set(MY_CACHE_VARIABLE "VALUE" CACHE INTERNAL "")
```
因为 BOOL 类型非常常见，你可以这样非常容易的设置它:

```
option(MY_OPTION "This is settable from the command line" OFF)
```

对于 BOOL 这种数据类型，对于它的 ON 和 OFF 有几种不同的说辞 (wordings) 。

你可以查看[cmake-variables](https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html)来查看 CMake 中已知变量的清单。


## 3. 环境变量
你也可以通过`set(ENV{variable_name} value)`和`$ENV{variable_name}`来设置和获取环境变量，不过一般来说，我们最好避免这么用。

## 4. 缓存

缓存实际上就是个文本文件，CMakeCache.txt ，当你运行 CMake 构建目录时会创建它。 CMake 可以通过它来记住你设置的所有东西，因此你可以不必在重新运行 CMake 的时候再次列出所有的选项。

## 5. 属性

CMake 可以通过属性来存储信息。它就像是一个变量，但它被附加到一些其他的实体上，像是一个目录或者是一个目标。例如一个全局的属性可以是一个有用的非缓存的全局变量。

在 CMake 的众多属性中，目标属性 ( Target Properties ) 扮演着尤为重要的角色，它们直接关联到最终生成的可执行文件、库文件等构建产物。

更直观一点，如果把目标类比为 类 ( Class )，那么目标属性则类似 类成员 ( Class  Member )。

```
class Target {
    string target_property;
};
```

1) **定义目标**

生成目标的方式有三种：
```
#生成可执行文件目标test
add_executable(test test.cpp)

#生成共享库目标test_lib
add_library(test_lib SHARED test.cpp)

#生成静态库目标test_lib
add_library(test_lib STATIC test.cpp)
```
>ps: If no <type> is given the default is STATIC or SHARED based on the value of the BUILD_SHARED_LIBS variable.


2) **定义目标属性**

给目标定义属性的命令为:

```
define_property(<GLOBAL | DIRECTORY | TARGET | SOURCE |
                 TEST | VARIABLE | CACHED_VARIABLE>
                 PROPERTY <name> [INHERITED]
                 [BRIEF_DOCS <brief-doc> [docs...]]
                 [FULL_DOCS <full-doc> [docs...]]
                 [INITIALIZE_FROM_VARIABLE <variable>])
```

在范围内定义一个属性，用于 set_property() 和 get_property() 命令。它主要用于定义属性的初始化或继承方式。从历史上看，该命令还将文档与属性相关联，但这不再被视为主要用例。

示例：
```
# 定义一个目标属性 TEST_TARGET，带有简短和详细描述 
define_property(TARGET PROPERTY TEST_TARGET 
    BRIEF_DOCS "A test property"
    FULL_DOCS "A long description of this test property"
)
```

3) **设置目标属性**

给目标设置属性的命令为：
```
set_property(<GLOBAL                      |
              DIRECTORY [<dir>]           |
              TARGET    [<target1> ...]   |
              SOURCE    [<src1> ...]
                        [DIRECTORY <dirs> ...]
                        [TARGET_DIRECTORY <targets> ...] |
              INSTALL   [<file1> ...]     |
              TEST      [<test1> ...]     |
              CACHE     [<entry1> ...]    >
             [APPEND] [APPEND_STRING]
             PROPERTY <name> [<value1> ...])
```

其中，有一个专用于设置目标属性命令：

```
set_target_properties(target1 target2 ...
                      PROPERTIES prop1 value1
                      prop2 value2 ...)
```

设置目标的属性。该命令的语法是列出您要更改的所有目标，然后提供您接下来要设置的值。您可以使用任何您想要的 prop 值对，稍后使用 get_property() 或 get_target_property() 命令提取它。

示例：
```
# 设置目标 TEST_TARGET 的属性 P_1 和 P_2 的值
set_target_properties(TEST_TARGET PROPERTIES
    P_1 "这是属性P_1的值"
    P_2 "这是属性P_2的值" 
)
```

4) **获取目标属性**

获取目标的属性的命令为：

```
get_property(<variable>
             <GLOBAL             |
              DIRECTORY [<dir>]  |
              TARGET    <target> |
              SOURCE    <source>
                        [DIRECTORY <dir> | TARGET_DIRECTORY <target>] |
              INSTALL   <file>   |
              TEST      <test>   |
              CACHE     <entry>  |
              VARIABLE           >
             PROPERTY <name>
             [SET | DEFINED | BRIEF_DOCS | FULL_DOCS])
```

其中，有一个专用于获取目标属性命令：
```
get_target_property(<VAR> target property)
```

从目标获取属性。属性的值存储在变量“”中。如果未找到目标属性，则行为取决于它是否已被定义为 INHERITED 属性（请参阅:command:define_property）。非继承属性会将<VAR>设置为<VAR>-NOTFOUND，而继承属性将搜索相关的父范围，如 define_property() 命令所述，如果仍然找不到属性 <VAR> 将被设置为空字符串。

使用 set_target_properties() 设置目标属性值。属性通常用于控制目标的构建方式，但有些属性会查询目标。此命令可以获得迄今为止创建的任何目标的属性。目标不需要位于当前的 CMakeLists.txt 文件中。

示例：
```
# 获取目标 TEST_TARGET 的属性 P_1 的值，并将其存储在变量 TEST_TARGET_P1 中
get_target_property(TEST_TARGET_P1 TEST_TARGET P_1) 
```
5) **完整示例**

```
# 要求 CMake 最低版本为 3.16
cmake_minimum_required(VERSION 3.16)

# 定义项目名称为 PROPERTY_TEST，版本号为 1.0，使用 C++ 语言
project(PROPERTY_TEST VERSION 1.0 LANGUAGES CXX)

# 添加一个名为 TEST_TARGET 的可执行文件，其源文件为 test.cpp
add_executable(TEST_TARGET test.cpp)

# 定义一个目标属性 TEST_TARGET，带有简短和详细描述 
define_property(TARGET PROPERTY TEST_TARGET 
    BRIEF_DOCS "A test property"
    FULL_DOCS "A long description of this test property"
)

# 设置目标 TEST_TARGET 的属性 P_1 和 P_2 的值
set_target_properties(TEST_TARGET PROPERTIES
    P_1 "这是属性P_1的值"
    P_2 "这是属性P_2的值" 
)

# 获取目标 TEST_TARGET 的属性 P_1 的值，并将其存储在变量 TEST_TARGET_P1 中
get_target_property(TEST_TARGET_P1 TEST_TARGET P_1) 

# 打印变量 TEST_TARGET_P1 的值
message("TEST_TARGET_P1: ${TEST_TARGET_P1}") 
```



<br />
<br />
<br />


