---
layout: post
title: cmake-20 find_library用法
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文参考:

- [cmake find_library()命令](https://cmake.org/cmake/help/latest/command/find_library.html)




<!-- more -->


## 1. find_library()用法

find_library() 命令的简化形式如下：

```
find_library (<VAR> name1 [path1 path2 ...])
```

下面我们给出其通用形式：
```
find_library (
          <VAR>
          name | NAMES name1 [name2 ...] [NAMES_PER_DIR]
          [HINTS [path | ENV var]... ]
          [PATHS [path | ENV var]... ]
          [REGISTRY_VIEW (64|32|64_32|32_64|HOST|TARGET|BOTH)]
          [PATH_SUFFIXES suffix1 [suffix2 ...]]
          [VALIDATOR function]
          [DOC "cache documentation string"]
          [NO_CACHE]
          [REQUIRED]
          [NO_DEFAULT_PATH]
          [NO_PACKAGE_ROOT_PATH]
          [NO_CMAKE_PATH]
          [NO_CMAKE_ENVIRONMENT_PATH]
          [NO_SYSTEM_ENVIRONMENT_PATH]
          [NO_CMAKE_SYSTEM_PATH]
          [NO_CMAKE_INSTALL_PREFIX]
          [CMAKE_FIND_ROOT_PATH_BOTH |
           ONLY_CMAKE_FIND_ROOT_PATH |
           NO_CMAKE_FIND_ROOT_PATH]
         )
```

该命令用于查找库，查找结果会存放在`<VAR>`变量中（ps：该变量可以是一个缓存变量，也可以是一个普通变量(指定了NO_CACHE情况下)）。假如对应的library已经找到了，那么将不会进行重复的搜索；假如没有找到，那么结果为`<VAR>-NOTFOUND`。

下面介绍一下其中的一些可选项：

- NAMES

可以指定一个或多个库的可能的名称。

当使用NAMES选项指定带版本号与不带版本号的library名称时，推荐将不带版本号的library名称放在首位，这样可以使得本地所构建的package可以优先被找到。

- HINTS, PATHS

指定处默认查找目录外的其他查找目录

- PATH_SUFFIXES

额外指定每一个目录下的子目录。

- DOC

为缓存变量`<VAR>`指定documentation信息

### 1.1 find_library()命令的查找过程

假如指定了`NO_DEFAULT_PATH`的话，那么find_library()将不会去查找一些额外的路径。否则，按如下方式进行查找：

1） 假如是在find module中调用本命令， 或者是在脚本中通过调用`find_package(<PackageName>)`，那么会在如下一系列的前缀目录中执行查找

- `<PackageName>_ROOT`: CMake变量所指定的目录，其中<PackageName>为要搜索的package的名称

- `<PACKAGENAME>_ROOT`: CMake变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写

- `<PackageName>_ROOT` 环境变量所指定的目录，其中<PackageName>为要搜索的package的名称

- `<PACKAGENAME>_ROOT` 环境变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写


2) 搜索cmake特定的缓存变量所指定的路径。这些缓冲变量通常倾向于通过命令行来传递(ps: 命令行传递方式为-DVAR=VALUE)。这些所传递的值以分号作为分隔。

- 假如指定了CMAKE_LIBRARY_ARCHITECTURE的话，那么会查找`CMAKE_PREFIX_PATH`所指定的所有前缀下的`<prefix>/lib/<arch>`目录； 否则查找`CMAKE_PREFIX_PATH`所指定所有前缀下的`<prefix>/lib`

- CMAKE_LIBRARY_PATH

- CMAKE_FRAMEWORK_PATH

3) 搜索cmake特定的环境变量所指定的路径。这些变量倾向于通过用户shell来进行设置(比如Linux中export某个变量)

- 假如指定了CMAKE_LIBRARY_ARCHITECTURE的话，那么会查找`CMAKE_PREFIX_PATH`所指定的所有前缀下的`<prefix>/lib/<arch>`目录； 否则查找`CMAKE_PREFIX_PATH`所指定所有前缀下的`<prefix>/lib`

- CMAKE_LIBRARY_PATH

- CMAKE_FRAMEWORK_PATH


4) 搜索由HINTS选项所指定的路径。HINTS选项所指定的路径一般是通过系统自省方式产生的，例如通过另一个已经被找到的library来提供hint路径。如果通过硬编码指定路径的话，建议使用`PATHS`选项来指定


5） 查找标准系统环境变量所指定的路径(ps: 可以通过NO_SYSTEM_ENVIRONMENT_PATH来跳过)

- LIB或PATH环境变量所指定的目录

>ps: 还有一些其他不太常用的搜索路径，这里不再细述.


## 2. find_library()与find_package()


find_library 和 find_package 是 CMake 中用于查找第三方库和包的命令。

1） find_library 命令用于查找库文件。它会在指定的路径中查找指定的库文件，并将库的路径存储在一个变量中。

例如，如果你想找到名为 jsoncpp 的库，你可以这样做：

```
find_library(JSONCPP_LIBRARY
             NAMES jsoncpp
             PATHS /usr/lib
                   /usr/local/lib
                   /opt/local/lib
             PATH_SUFFIXES jsoncpp)
```

在这个例子中，JSONCPP_LIBRARY 是一个 CMake 变量，它将包含找到的库的路径。


2) find_package 命令用于查找包，它会在指定的路径中查找包的配置文件，并将包的路径、include路径、库路径、以及其他相关信息存储在多个变量中。

例如，如果你想找到 jsoncpp 包，你可以这样做：

```
find_package(JSONCPP REQUIRED)
```

在这个例子中，`JSONCPP_INCLUDE_DIRS` 和 `JSONCPP_LIBRARIES`是CMake变量，它们将包含 jsoncpp 包的 include 路径和库路径。

注意，find_package 会查找一个名为`JSONCPPConfig.cmake`或`jsoncpp-config.cmake`的配置文件，这个文件通常由第三方库提供，并在其安装目录中的 lib/cmake/jsoncpp 或 lib64/cmake/jsoncpp 下。

总结：find_library 用于查找库文件，而 find_package 用于查找包，包通常会提供一个配置文件，其中包含了库文件的信息。


<br />
<br />
<br />


