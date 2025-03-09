---
layout: post
title: LD_DEBUG用法
tags:
- LinuxOps
categories: linux
description: LD_PRELOAD基础用法
---

本文参看Deepseek关于`linux LD_DEBUG的使用`回答，记录一下LD_DEBUG的用法。


<!-- more -->

# 1. LD_DEBUG使用

LD_DEBUG是Linux中用于调试动态链接器(ld.so)的强大工具，通过设置环境变量可以输出动态链接过程的详细信息。以下是具体的使用方法和实际场景示例。

## 1.1 基本用法

在运行程序前设置 LD_DEBUG 环境变量，语法如下：

```bash
# LD_DEBUG=<options> program
```

例如:

```bash
# LD_DEBUG=libs ls             # 查看 `ls` 命令加载的动态库
# LD_DEBUG=symbols ./myapp     # 跟踪 `myapp` 的符号解析
```

## 1.2 常用调试选项

```text
选项	     作用
-----------------------------------------------------------
libs	    显示库搜索路径和加载的共享库（最常用）。
symbols	    追踪符号查找（如未定义的符号）。
files	    显示文件处理过程（如打开库文件）。
bindings	显示符号绑定细节（如哪个库解析了符号）。
versions	检查库的版本依赖关系。
help	    列出所有可用选项（需 root 权限）
```

## 1.3 实际使用场景

**场景 1：诊断“库未找到”错误**

```bash
# LD_DEBUG=libs ./myapp
```

  - 输出示例:

    ```text
    查找库 `libfoo.so` -> 搜索路径：/usr/lib:/lib
    尝试打开 `/usr/lib/libfoo.so` -> 失败
    尝试打开 `/lib/libfoo.so` -> 失败
    ```
  - 结论: 动态链接器未找到 libfoo.so，需检查路径或安装库

**场景 2：追踪符号冲突**

```bash
# LD_DEBUG=symbols,bindings ./myapp
```

  - 输出示例

    ```text
	查找符号 `bar` -> 在 `libA.so` 中找到
    符号 `bar` 绑定到 `libA.so`
    符号 `bar` 被 `libB.so` 覆盖
	```

  - 结论: libA.so 和 libB.so 定义了同名符号，需调整库加载顺序

**场景 3：验证 LD_LIBRARY_PATH 是否生效**

```bash
# LD_DEBUG=libs LD_LIBRARY_PATH=/custom/libs ./myapp
```

  - 输出示例

    ```text
	搜索路径：/custom/libs:/usr/lib:/lib
    加载 `/custom/libs/libfoo.so`
	```

  - 结论: 确认自定义库路径生效。

## 1.4 高级用法

1. **将调试输出保存到文件**

    ```bash
	# LD_DEBUG=all LD_DEBUG_OUTPUT=debug.log ./myapp
	```

    输出会写入 debug.log 文件（默认追加到程序名后缀，如 debug.log.1234）

1. **显示所有可用选项（需 root**

    ```bash
	# sudo LD_DEBUG=help /bin/true
	```

	输出所有支持的 LD_DEBUG 选项。

## 1.5 注意事项

1. **安全限制**

    - LD_DEBUG 对 SUID/SGID 程序无效（如 sudo、passwd），避免权限泄露

	- 生产环境慎用，输出可能包含敏感信息

1. **输出过滤**

    - 若输出过多，可用 grep 过滤关键信息

	  ```bash
	  # LD_DEBUG=libs ./myapp 2>&1 | grep "library search path"
	  ```

1. **与 ldd 的区别**

    ldd 仅显示库依赖，而 LD_DEBUG 提供动态链接的详细过程（如符号解析）。


## 1.6 典型错误分析

```text
错误：symbol lookup error: undefined symbol: foo
```
1. 使用 LD_DEBUG=symbols 运行程序

    ```bash
    # LD_DEBUG=symbols ./myapp
    ```

1. 查看输出中 foo 的查找路径

    ```bash
	查找符号 `foo` -> 未找到
	```

1. 解决：检查是否遗漏链接库，或库版本不兼容。

## 1.7 总结

通过 LD_DEBUG 可以精准定位以下问题：

- 动态库路径配置错误（如 LD_LIBRARY_PATH 或 rpath）。

- 符号冲突或未定义。

- 库版本不兼容。

灵活使用 libs、symbols、files 等选项，结合输出日志，可以高效解决动态链接问题。



<br />
<br />
<br />





