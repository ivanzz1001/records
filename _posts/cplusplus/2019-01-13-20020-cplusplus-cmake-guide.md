---
layout: post
title: cmake-20 find_path用法
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文参考:

- [cmake find_path()命令](https://cmake.org/cmake/help/latest/command/find_path.html)




<!-- more -->


## 1. find_path()用法

find_path() 命令的简化形式如下：

```
find_path (<VAR> name1 [path1 path2 ...])
```

下面我们给出其通用形式：
```
find_path (
          <VAR>
          name | NAMES name1 [name2 ...]
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

该命令用于查找包含指定文件的目录。查找结果会存放在`<VAR>`变量中（ps：该变量可以是一个缓存变量，也可以是一个普通变量(指定了NO_CACHE情况下)。假如文件查找成功，则对应的路径会存放到`<VAR>`变量中，且只要该变量未被清除，那么后续将不会进行重复的查找；如查找失败，则结果为`<VAR>-NOTFOUND`

下面介绍一下其中的一些可选项：

- NAMES

为要查找的文件指定一个或多个名称。

当使用此选项来指定要查找文件的名称时，可以携带或不携带版本号，但我们通常建议把不携带版本号的name放在前面，这样可以使得本地所构建的包可以优先被找到。

- HINTS, PATHS

指定除默认查找目录外的其他查找目录

- PATH_SUFFIXES

额外指定每一个目录下的子目录。

- DOC

为缓存变量`<VAR>`指定documentation信息

### 1.1 find_path()命令的查找过程

假如指定了`NO_DEFAULT_PATH`的话，那么find_path()将不会去查找一些额外的路径。否则，按如下方式进行查找：

1） 假如是在find module中调用本命令， 或者是在脚本中通过调用`find_package(<PackageName>)`，那么会在如下一系列的前缀目录中执行查找

- `<PackageName>_ROOT`: CMake变量所指定的目录，其中<PackageName>为要搜索的package的名称

- `<PACKAGENAME>_ROOT`: CMake变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写， 参看CMP0144 policy

- `<PackageName>_ROOT` 环境变量所指定的目录，其中<PackageName>为要搜索的package的名称

- `<PACKAGENAME>_ROOT` 环境变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写

由于package root变量都是以栈(stack)的形式保存的，这就意味着如果是从一个内层find-module或config-packages调用此命令的话，那么也将会查找parent find-module或parent config-packages中上述变量指定的位置。



2) 搜索cmake特定的缓存变量所指定的路径。这些缓冲变量通常倾向于通过命令行来传递(ps: 命令行传递方式为-DVAR=VALUE)。这些所传递的值以分号作为分隔。

- 假如指定了CMAKE_LIBRARY_ARCHITECTURE的话，那么会查找`CMAKE_PREFIX_PATH`所指定的所有前缀下的`<prefix>/include/<arch>`目录； 否则查找`CMAKE_PREFIX_PATH`所指定所有前缀下的`<prefix>/include`

- CMAKE_INCLUDE_PATH

- CMAKE_FRAMEWORK_PATH

3) 搜索cmake特定的环境变量所指定的路径。这些变量倾向于通过用户shell来进行设置(比如Linux中export某个变量)

- 假如指定了CMAKE_LIBRARY_ARCHITECTURE的话，那么会查找`CMAKE_PREFIX_PATH`所指定的所有前缀下的`<prefix>/include/<arch>`目录； 否则查找`CMAKE_PREFIX_PATH`所指定所有前缀下的`<prefix>/include`

- CMAKE_INCLUDE_PATH

- CMAKE_FRAMEWORK_PATH


4) 搜索由HINTS选项所指定的路径。HINTS选项所指定的路径一般是通过系统自省方式产生的，例如通过另一个已经被找到的library来提供hint路径。如果通过硬编码指定路径的话，建议使用`PATHS`选项来指定


5） 查找标准系统环境变量所指定的路径(ps: 可以通过NO_SYSTEM_ENVIRONMENT_PATH来跳过)

- INCLUDE或PATH环境变量所指定的目录


6) 查找当前系统platform文件中所定义的cmake变量所指定的路径

- 假如指定了CMAKE_LIBRARY_ARCHITECTURE的话，那么会查找`CMAKE_SYSTEM_PREFIX_PATH`所指定的所有前缀下的`<prefix>/include/<arch>`目录； 否则查找`CMAKE_SYSTEM_PREFIX_PATH`所指定所有前缀下的`<prefix>/include`

- CMAKE_SYSTEM_INCLUDE_PATH

- CMAKE_SYSTEM_FRAMEWORK_PATH

>ps: 还有一些其他不太常用的搜索路径，这里不再细述.



<br />
<br />
<br />


