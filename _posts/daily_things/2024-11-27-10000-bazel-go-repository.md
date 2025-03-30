---
layout: post
title: bazel go_repository为某个library生成的BUILD文件在哪？
tags:
- bazel
categories: bazel
description: bazel go_repository为某个library生成的BUILD文件在哪
---

在Bazel中，`go_repository`依赖的BUILD文件是自动生成的，但它不会直接存在于bazel-out/external目录。Bazel会在内部解析Go模块，并为其创建`BUILD.bazel`文件。要找到这些BUILD文件，可以使用以下方法。


<!-- more -->


# 1. bazel 查找go_repository生成的BUILD文件

## 1.1 定位生成的BUILD文件

**步骤1：获取Bazel输出基目录路径**

  运行以下命令，查看Bazel的输出基目录(output_base):

  ```bash
  bazel info output_base
  ```

  输出示例:

  ```text
  /home/username/.cache/bazel/_bazel_username/abcdef1234567890
  ```

**步骤2: 导航到 external 目录**

  生成的 BUILD 文件位于 output_base/external/[repository_name] 目录下:

  ```bash
  cd $(bazel info output_base)/external
  ls
  ```

**步骤3：找到目标仓库的 BUILD 文件**

  假设你的 go_repository 规则定义如下：

  ```python
  go_repository(
    name = "com_github_example_library",
    importpath = "github.com/example/library",
    commit = "abc123...",
  )
  ```
  
  则对应的 BUILD 文件路径为：

  ```text
  output_base/external/com_github_example_library/BUILD
  ```

## 2. 查看 BUILD 文件内容

## 2.1 直接通过路径访问

```bash
cat $(bazel info output_base)/external/com_github_example_library/BUILD
```

## 2.2 使用 bazel query 快速定位

```bash
bazel query @com_github_example_library//... --output=location
```

其中：

- `@com_github_example_library//...`表示查询名为 com_github_example_library 的外部仓库（通过 go_repository 定义）下的 所有构建目标

- `--output=location`要求输出结果包含目标的 物理位置信息（文件路径和行号）

1. **输出示例**

    ```text
    /home/user/.cache/bazel/_bazel_user/abcd1234/external/com_github_example_library/BUILD.bazel:10:1: go_library rule @com_github_example_library//:library
    ```

    各字段含义:

    - `/home/user/.cache/bazel/.../BUILD.bazel`: 目标所在的 BUILD 文件路径

    - `:10:1`：规则在文件中的位置（第 10 行，第 1 列）

    - `go_library rule`：目标类型（这里是 Go 库规则）。

    - `@com_github_example_library//:library`: 目标的完整标签


## 3. 关键说明

- 生成机制：go_repository 会基于 Go 模块的依赖关系自动生成 BUILD 文件，无需手动维护。

- 不可修改性：这些 BUILD 文件由 Bazel 自动管理，直接修改它们不会持久化（下次同步可能被覆盖）。

- 自定义构建规则：如需覆盖生成的规则，可在项目根目录的 BUILD.bazel 中使用 http_archive + patch 或自定义宏。

## 4. 调试技巧

- 查看依赖树：使用 `bazel query 'deps(@com_github_example_library//...)'` 分析依赖关系。

- 手动触发生成：运行 `bazel sync --only=com_github_example_library` 强制重新生成。
