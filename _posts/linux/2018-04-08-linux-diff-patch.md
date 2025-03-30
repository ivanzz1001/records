---
layout: post
title: Linux中使用diff及patch命令打补丁
tags:
- LinuxOps
categories: linux
description: 使用diff及patch命令打补丁
---

如下来自于Deepseek关于“Linux diff patch打补丁用法”的回答。

<!-- more -->


# 1. 生成补丁文件


## 1.1 使用diff命令生成补丁文件

1. **基本命令格式**

    ```bash
    # diff [选项] 旧文件/目录 新文件/目录 > 补丁文件.patch
    ```


1. **核心选项说明**

    ```text
    选项        说明
    ----------------------------------------------------------------------------------------
    -u         生成统一差异格式（Unified Diff），适合代码补丁
    -N         将新增文件视为空文件对比，确保补丁包含新增文件
    -a         强制将文件视为文本，即使看似二进制文件
    -r         递归对比子目录（用于目录级补丁）
    -p         显示差异所在的函数名（适用于代码）
    ```

1. **生成补丁示例**

    (1) 对比两个文件

    ```bash
    # diff -u old_code.c new_code.c > code_fix.patch
    ```   

    (2) 对比两个目录

    ```bash
    # diff -Naur old_project/ new_project/ > feature_update.patch
    ```

      - 补丁示例内容：

        ```text
        --- old_project/src/main.c  2023-10-01 00:00:00 +0800
        +++ new_project/src/main.c  2023-10-02 00:00:00 +0800
        @@ -5,6 +5,6 @@
         int main() {
             printf("Hello World!\n");
        -    old_function();
        +    new_function();
             return 0;
         }
        Only in new_project/docs/: README.md  # 新增文件
        ```


## 1.2 使用git diff命令生成补丁文件

在Git中为两个文件的修改生成补丁(patch)并附加上你的Git账号信息，可以按如下步骤进行操作。

### 1.2.1 配置Git账号信息(如果未配置)

确保你的Git全局配置中已包含用户名和邮箱，这些信息会自动附加到补丁中：

```bash
git config --global user.name "你的名字"
git config --global user.email "你的邮箱@example.com"
```

### 1.2.2 生成补丁文件

**方法1：基于提交生成补丁（推荐）**

此方法会保留完整的 Git 提交信息（包括作者信息），适合已提交的更改。

1. 针对原始文件创建一个git工程并进行初始提交

    ```bash
    # mkdir a
    # cd a
    # git init .
    # touch file1.txt
    # echo "hello, world" > file1.txt
    # git add file1.txt
    # git commit -m "initial file1.txt"
    ```

1. 修改file1.txt并提交更改

    ```bash
    # echo "hello, world2" > file1.txt
    # git add file1.txt
    # git commit -m "modified file1"
    ```

1. 生成补丁

    ```bash
    git format-patch -1 HEAD  # 生成最近一次提交的补丁方法1
    git format-patch HEAD^    # 生成最近一次提交的补丁方法2   
    ```
    其中：

    - 生成的补丁文件名为`0001-modified-file1.patch`

    - 补丁内容自动包含作者信息（如下示例)
 
       ```text
       From 699167f6608f4a45755faa345927bbef0f39311b Mon Sep 17 00:00:00 2001
       From: ivan1001 <782740456@qq.com>
       Date: Mon, 31 Mar 2025 00:16:40 +0800
       Subject: [PATCH] modified a.txt
       
       ---
       a.txt | 1 +
       1 file changed, 1 insertion(+)
       
       diff --git a/a.txt b/a.txt
       index e69de29..4b5fa63 100644
       --- a/a.txt
       +++ b/a.txt
       @@ -0,0 +1 @@
       +hello, world
       -- 
       2.34.
       ```

**方法2：直接生成差异补丁（未提交的更改）**

如果尚未提交更改，可以强制附加作者信息到补丁中：

```bash
git diff --patch --author="你的名字 <你的邮箱@example.com>" file1.txt file2.txt > my_changes.patch
```

补丁内容示例:

```text
Author: 你的名字 <你的邮箱@example.com>
diff --git a/file1.txt b/file1.txt
index 1111111..2222222 100644
--- a/file1.txt
+++ b/file1.txt
@@ -1,3 +1,4 @@
 Hello World!
-Old line.
+New line.
+Another line.
```

# 2. 应用补丁文件

1. **基本命令格式**

    ```bash
    # patch [选项] < 补丁文件.patch
    ```

1. **核心选项说明**

    ```text
    选项        说明
    -------------------------------------------------------------------------------
    -pNUM       剥离路径前缀层数（解决路径不匹配）
    -i          指定补丁文件（如 -i fix.patch）
    -R          撤销已应用的补丁（回滚更改）
    -b          备份原始文件（生成 .orig 文件）
    --dry-run   模拟应用补丁（不修改文件）
    -d DIR      指定补丁应用的目标目录         
    ```

1. **关键操作示例**

    (1) 标准应用流程

    ```bash
    # 进入目标目录
    cd old_project

    # 应用补丁（剥离路径前缀 1 层）
    patch -p1 < ../feature_update.patch

    # 输出示例：
    patching file src/main.c
    creating new file docs/README.md
    ```
     
    (2) 路径剥离(-pNum)详解

      - 补丁路径示例

        ```text
        --- a/src/main.c
        +++ b/src/main.c
        ```

      - 应用场景

        - `-p0`: 保留完整路径 a/src/main.c（需目录结构完全匹配）

        - `-p1`: 剥离`a/`，路径变为 src/main.c（最常用）

        - `-p2`: 剥离 `a/src/`，路径变为 main.c（适用于进入 `src/` 目录后操作）

    (3) 撤销补丁

    ```bash
    patch -R -p1 < ../feature_update.patch
    ```

    (4) 备份原始文件

    ```bash
    patch -p1 -b < ../feature_update.patch
    # 原文件备份为 main.c.orig
    ```

    (5) 测试补丁

    ```bash
    patch --dry-run -p1 < ../feature_update.patch
    # 输出模拟结果，无实际修改
    ```

# 3. 路径问题与解决方案

## 3.1 错误示例

```text
can't find file to patch at input line 5
```
原因：补丁中的路径与当前目录结构不匹配。

## 3.2 解决步骤

1. **查看补丁中的路径**

```bash
# head -n 5 feature_update.patch
--- a/src/main.c
+++ b/src/main.c
```

1. **调整`-p`值**

    - 若当前在项目根目录(含`src/`子目录)

      ```bash
      patch -p1 < feature_update.patch  # 剥离 a/ → src/main.c
      ```

    - 若当前在`src/`目录

    ```bash
    patch -p2 < feature_update.patch  # 剥离 a/src/ → main.c
    ```















<br />
<br />
<br />





