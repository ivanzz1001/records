---
layout: post
title: patchelf用法
tags:
- LinuxOps
categories: linux
description: patchelf用法
---

在开发过程中，有时候会遇到libc库不兼容的情况，本文记录一下如何更换一个可执行应用程序的libc库。


<!-- more -->

# 1. 编译glibc

本文以编译glibc-2.38为例。


1. **下载源代码**

    下载glibc 的ftp网站为: https://ftp.gnu.org/gnu/glibc/
    ```bash
    # mkdir glibc-compile
    # cd glibc-compile
    # wget https://ftp.gnu.org/gnu/glibc/glibc-2.38.tar.gz
    # tar -zxvf glibc-2.38.tar.gz
    ```

1. **编译**

    ```bash
    # mkdir build
    # cd build
    # ../glibc-2.38/configure --prefix=/opt/glibc-2.38 
    # make -j4
    # make install
    # tree /opt/glibc-2.38
      ├── include/    
      ├── lib/        
      └── lib/ld-linux-x86-64.so.2  
    ```

# 2. 使用patchelf更换一个已经编译好的程序的glibc

在 Linux 系统中，使用 patchelf 工具可以修改 ELF 可执行文件的动态链接器（interpreter）和库依赖路径（RPATH/RUNPATH）。以下是更换程序依赖的 libc 库的详细步骤：


1. **安装patchelf**

    如果系统未安装 patchelf，先通过包管理器安装：

    - Debian/Ubuntu系统
      ```bash
      # apt-get install patchelf
      ```

    - Centos/RHRL
      ```bash
      # yum install patchelf
      ```

1. **准备好glibc**

    上文我们已经安装好自己的glibc，位置在: /opt/glibc-2.38

1. **使用patchelf修改程序**

    假设目标程序为 `your_program`，按以下步骤操作：

    - 修改动态链接器(interpreter)

      ```bash
      # patchelf --set-interpreter /opt/glibc-2.38/lib/ld-linux-x86-64.so.2 your_program
      ```

    - 设置库搜索路径（RPATH/RUNPATH）

      ```bash
      # patchelf --set-rpath /opt/glibc-2.38/lib your_program
      ```

      若程序依赖其他库（如 libstdc++.so），需确保它们在新环境中可用，可以通过 `--add-rpath` 添加路径:

      ```bash
      # patchelf --add-rpath /opt/glibc-2.38/lib your_program
      ```

1. **验证修改结果**

    ```bash
    # ldd your-program
    ```

# 3. 编译程序时使用自定义的glibc

我们假定编译好的glibc根目录为: /opt/glibc-2.38

使用 gcc 的以下参数强制链接到自定义 glibc：

1. **指定头文件路径**

    ```text
    -I/opt/glibc-2.38/include 
    ```

1. **指定库文件路径**

    ```text
    -L/opt/glibc-2.38/lib -Wl,-rpath=/opt/glibc-2.38/lib
    ```

    - `-L`: 链接时搜索库文件的目录

    - `-Wl,-rpath`: 设置运行时库搜索路径（等价于 `--set-rpath`）

1. **指定动态链接器**

    ```text
    -Wl,--dynamic-linker=/opt/glibc-2.38/lib/ld-linux-x86-64.so.2
    ```

1. **完整编译命令示例**

    假设源文件为 hello.c，编译命令如下：

    ```bash
    # gcc hello.c -o hello \
    -I/opt/glibc-2.38/include \
    -L/opt/glibc-2.38/lib \
    -Wl,--dynamic-linker=/opt/glibc-2.38/lib/ld-linux-x86-64.so.2 \
    -Wl,-rpath=/opt/glibc-2.38/lib \
    -nostdinc
    ```

    说明: `-nostdinc`的含义为禁止搜索系统默认头文件

## 3.1 验证编译结果

1. **检查动态链接器和库依赖**

    ```bash
    # readelf -l hello | grep "interpreter"
    [Requesting program interpreter: /opt/glibc-2.38/lib/ld-linux-x86-64.so.2]

    # ldd hello
    libc.so.6 => /opt/glibc-2.38/lib/libc.so.6
    ```

1. **运行程序**

    ```bash
    # ./hello
    ```
    若程序正常运行，说明已成功使用自定义 glibc。


## 3.2 静态链接glibc(不推荐)

若需完全静态链接（避免依赖动态库），可以按如下方法编译程序:

```bash
# gcc hello.c -o hello \
  -static \
  -I/opt/glibc-2.38/include \
  -L/opt/glibc-2.38/lib
```

# 4. 直接使用ld-linux-x86-64.so.2 运行程序

我们也可以不使用上文介绍的`patchelf`来更换程序所链接的glibc，而是直接用ld-linux-x86-64.so.2来启动程序。例如：

```text
# pwd
/package/lib/x86_64-linux-gnu
# tree ./x86_64-linux-gnu/
./x86_64-linux-gnu/
├── ld-linux-x86-64.so.2
├── libc.so.6
├── libdl.so.2
├── libm.so.6
├── libpthread.so.0
└── librt.so.1

0 directories, 6 files
# /package/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 --library-path /package/lib/x86_64-linux-gnu:<other-lib-path> my_binary
```



<br />
<br />
<br />




