---
layout: post
title: Git Head游离
tags:
- tools
categories: tools
description: Git Head游离
---

本章介绍一下Git HEAD detached及其对应的解决办法。


<!-- more -->


## 1. 什么是HEAD
Git中的HEAD可以理解为一个指针，我们可以在命令行中输入```cat .git/HEAD```查看当前HEAD指向哪，一般它指向当前工作目录所在分支的最新提交：

![git-head-show](https://ivanzz1001.github.io/records/assets/img/tools/git-head-show.jpg)

当使用```git checkout <branch_name>```切换分支时，HEAD会移动到指定分支：

![git-head-checkout](https://ivanzz1001.github.io/records/assets/img/tools/git-head-checkout.jpg)

但是如果使用的是```git checkout <commit-id>```，即切换到指定的某一次提交，HEAD就会处于```detached```(游离）状态。

![git-head-detached](https://ivanzz1001.github.io/records/assets/img/tools/git-head-detached.jpg)


## 2. HEAD游离状态的利弊
HEAD处于游离状态时，我们可以很方便地在历史版本之间互相切换。比如需要回到某次提交，直接```checkout```对应的```commit-id```或者```tag```名即可。

它的弊端就是： 它在这个基础上的提交会新开一个匿名分支

![git-head-commit](https://ivanzz1001.github.io/records/assets/img/tools/git-head-commit.jpg)

也就是说我们的提交是无法可见保存的，一旦切到别的分支，游离状态以后的提交就不可追溯了。

![git-head-track](https://ivanzz1001.github.io/records/assets/img/tools/git-head-track.jpg)

解决办法就是新建一个分支保存游离状态后的提交：

![git-head-branch](https://ivanzz1001.github.io/records/assets/img/tools/git-head-branch.jpg)




<br />
<br />

**[参看]**

1. [Git HEAD detached](https://blog.csdn.net/u011240877/article/details/76273335)




<br />
<br />
<br />

