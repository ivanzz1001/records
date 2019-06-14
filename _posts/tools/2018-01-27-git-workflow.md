---
layout: post
title: git工作流程
tags:
- tools
categories: tools
description: git工作流程
---

在本章，我们先介绍一下Git的工作流程，之后是整个工作流程所对应的一些操作。Git的一般工作流程如下：

* 将Git的一个存储库克隆为工作副本

* 可以通过添加/编辑文件修改工作副本

* 如有必要，还可以通过让其他开发人员一起来更改/更新工作副本

* 在提交之前查看更改

* 提交更改：如果一切正常，那么将您的更改推送到存储库

* 提交后，如果意识到某些错误并修改错误后，则将最后一个正确的修改提交并推送到存储库。


<!-- more -->

下面显示的是工作流程的图示：

![git-workflow](https://ivanzz1001.github.io/records/assets/img/tools/git_workflow.png)


## 1. 创建存储库
通常我们只需要在第三方托管平台[GitHub](https://github.com/)注册好相应的账号，简单几步就可以创建好。

我们新建一个sample库，地址为：
<pre>
https://github.com/ivanzz1001/sample.git
</pre>

## 2. Git克隆操作

通常我们只需要在对应的目录执行```git clone <url>```即可:
<pre>
$ git clone https://github.com/ivanzz1001/sample.git
</pre>

## 3. 执行变更操作

现在克隆存储库之后，我们开始学习Git基本的文件修改和版本管理操作。假设要使用```sample```这个存储库来协同管理一个Python的项目。首先创建一个Python的代码文件: ```main.py```，编写了一些代码完成并保存文件后， main.py现在的内容如下所示：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")
{% endhighlight %}
假设上面代码编译并通过了测试，一切正常。现在，我们可以安全地将这些更改添加到存储库。

查看当前工作区状态：
{% highlight string %}
$ git status -s
?? main.py
{% endhighlight %}
Git在文件名之前显示两个```问号```。因为到目前操作为止，这些文件还不是Git的一部分(Git还不能控制这些文件），Git不知道该怎么处理这些文件。这就是为什么Git在文件名之前显示问号。





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

<br />
<br />
<br />

