---
layout: post
title: 如何关闭go vscode缓存
tags:
- tools
categories: tools
description: 如何关闭go vscode缓存
---

使用go test进行测试，没有修改测试代码的情况下， 重复多次测试，发现测试结果没有变化。如在外部修改了数据库，但是在go test查询mysql却显示之前的查询结果。参看：


- [go:vscode 关闭go test 缓存 / go test 测试结果没有更新](https://blog.csdn.net/qq_35066345/article/details/89262974)


- [VSCODE 关闭 go 的test缓存](https://blog.csdn.net/pphboy/article/details/133061867)

<!-- more -->

## 1. 相关问题描述
使用go test进行测试，没有修改测试代码的情况下， 重复多次测试，发现测试结果没有变化。

这是因为测试并没有被实际运行，显示的是之前缓存的测试结果。从Go1.10开始，测试结果将被缓存，golang缓存测试结果这点在官方文档也能看到说明。见:https://golang.org/cmd/go/#hdr-Testing_flags说明。当go test以包列表模式运行时，go test会缓存成功的包的测试结果以避免不必要的重复测试。当你的测试代码没有变化的时候，它会显示缓存下来的测试结果。

附：根据测试结果来看，显示哪一次测试结果，是看你当前测试代码的签名，类似于map[代码签名]测试结果。如果你只是添加了几个字符，测试后又删掉，那么代码的签名和添加字符前一样，再次测试的时候还是显示的是之前的缓存。

## 2. scode 禁用 go test缓存

要禁用go test的缓存，需要添加参数:-count=1。那么go test的命令是这样子的：
```
go test -v -count=1 gofile_test.go
```

在vscode中添加步骤：

- 左下角打开设置

- 配置项搜索```go testFlags```

- 选择工作区设置，如果需要全局设置禁用缓存的话，用户设置也需要重复同样的操作

- 添加参数```-count=1```(-v 的参数设置是测试时显示打印信息)



![go-testFlags](https://ivanzz1001.github.io/records/assets/img/tools/go-testFlags.png)

点击 Edit in settings.json, 在里面添加```-count=1```:

![go-testFlags](https://ivanzz1001.github.io/records/assets/img/tools/9c5074f05e3472dd45217783aff479fe.png)



<br />
<br />
<br />