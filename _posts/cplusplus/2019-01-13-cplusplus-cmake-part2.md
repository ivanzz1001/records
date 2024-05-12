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
# git clone https://github.com/gflags/gflags.git

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










<br />
<br />
<br />


