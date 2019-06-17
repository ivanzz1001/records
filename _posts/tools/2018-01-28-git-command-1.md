---
layout: post
title: git常用命令(上)
tags:
- tools
categories: tools
description: git常用命令(上)
---

本章我们介绍一下Git中的一些常用命令。

<!-- more -->


## 1. git config命令

```git config```命令用于获取并设置存储库或全局选项。这些变量可以控制Git的外观和操作的各个方面。

1） **语法简介**
{% highlight string %}
git config [<file-option>] [type] [-z|--null] name [value [value_regex]]
git config [<file-option>] [type] --add name value
git config [<file-option>] [type] --replace-all name value [value_regex]
git config [<file-option>] [type] [-z|--null] --get name [value_regex]
git config [<file-option>] [type] [-z|--null] --get-all name [value_regex]
git config [<file-option>] [type] [-z|--null] --get-regexp name_regex [value_regex]
git config [<file-option>] --unset name [value_regex]
git config [<file-option>] --unset-all name [value_regex]
git config [<file-option>] --rename-section old_name new_name
git config [<file-option>] --remove-section name
git config [<file-option>] [-z|--null] -l | --list
git config [<file-option>] --get-color name [default]
git config [<file-option>] --get-colorbool name [stdout-is-tty]
git config [<file-option>] -e | --edit
{% endhighlight %}

2) **描述**

可以使用此命令查询、设置、替换、取消相应的选项。```git config```中的name实际上是由section与key所组成的(形式为```section.name```)，而实际的值会被转义。

3） **配置文件的存储位置**






<br />
<br />

**[参看]**

1. [git教程](https://www.yiibai.com/git/getting-started-git-basics.html)

2. [git创建远程仓库并上传代码到远程仓库中](https://blog.csdn.net/liuweixiao520/article/details/78971221)

3. [git book](https://git-scm.com/book/zh/v2)

4. [Git 基础再学习之：git checkout -- file](https://www.cnblogs.com/Calvino/p/5930656.html)

5. [git使用ssh密钥](https://www.cnblogs.com/superGG1990/p/6844952.html)

6. [git创建远程分支](https://blog.csdn.net/u012701023/article/details/79222731)

7. [git clone 单个分支项目或者所有项目分支](https://blog.csdn.net/she_lock/article/details/79453484)

8. [Git HEAD detached from XXX (git HEAD 游离) 解决办法](https://blog.csdn.net/u011240877/article/details/76273335)

<br />
<br />
<br />

