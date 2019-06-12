---
layout: post
title: git快速入门
tags:
- tools
categories: tools
description: git快速入门
---

假如只能阅读一章来学习Git，那么本教程绝对是一个不二的选择。本章内容涵盖你在使用Git完成各种工作中将要使用的各种基本命令。在学习完本章之后，你应该能够配置并初始化一个仓库（```repository```)、开始或停止跟踪(```track```)文件、暂存(```stage```)或提交(commit)更改。本章也将演示如何配置Git来忽略指定的文件和文件模式、如何迅速而简单的撤销错误操作、如何浏览项目的历史版本以及不同提交间的差异、如何向远程仓库推送(push)以及如何从远程仓库拉取(pull)文件。


<!-- more -->

Git的目标是管理一个工程，或者说是一些文件的集合，以跟踪它们的变化。Git使用Repository来存储这些信息。一个仓库主要包含以下内容（也包括其他内容）：

* 许多commit objects

* 到commit objects的指针，叫做heads

* Git的仓库和工程存储在同一个目录下，在一个叫做.git的子目录中




## 1. 创建Repository
在使用Repository(仓库)之前，我们首先需要创建仓库。创建仓库有很多种，这里常见的有如下几种：

* 自己搭个Git服务器，安装如GitLab的Git版本管理系统

* 使用第三方托管平台，如国内的码云(https://gitee.com/login)和国外的GitHub(http://github.com/)

这里我们使用第三个托管平台GitHub，我们只需要注册一个GitHub账号，然后登录简单几步就可以创建:

![git-create-repository](https://ivanzz1001.github.io/records/assets/img/tools/git_create_repository.jpg)

这样一个公开的仓库就创建好了，要记住上面创建好的仓库地址：
<pre>
https://github.com/ivanzz1001/git-start.git
</pre>


## 2. 获取Git仓库
有两种取得Git项目仓库的方法。第一种是从一个服务器clone一个现有的Git仓库； 第二种是在现有项目或目录下导入所有文件到Git中。

### 2.1 clone现有仓库
如果你想获得一份已经存在了的Git仓库的拷贝，比如说，想为某个开源项目贡献自己的一份力，这时就要用到```git clone```命令。如果你对其他VCS系统（比如说Subversion)很熟悉，请留心一下这里使用的命令是```clone```而不是```checkout```。这是Git区别于其他版本控制系统的一个重要特性， Git克隆的是该Git仓库服务器上的几乎所有数据，而不是仅仅复制完成你的工作所需要文件。当你执行```git clone```命令的时候，默认配置下远程Git仓库中的每一个文件的每一个版本都将被拉取下来。如果服务器的磁盘坏掉了，通常可以使用任何一个克隆下来的用户端来重建服务器上的仓库。

在安装了Git的Windows系统上，在一个目录（本示例是： ```F:\worksp```)中，单击右键，在弹出的菜单中选择```Git Bash```，如下图所示：

![git-bash](https://ivanzz1001.github.io/records/assets/img/tools/git_bash.jpg)
克隆仓库的命令格式是```git clone [url]```。比如要克隆上面创建的仓库```git-start```，可以使用下面的命令：
<pre>
git clone https://github.com/ivanzz1001/git-start.git
</pre>
这会在当前目录下创建一个名为```git-start```的目录，并在这个目录下初始化一个```.git```文件夹。从远程仓库拉取下所有数据放入```.git```文件夹，然后从中读取最新版本的文件的拷贝。上面命令执行后输出结果如下所示：
{% highlight string %}
Administrator@ZHANGYW6668 MINGW64 /f/worksp
$ git clone https://github.com/ivanzz1001/git-start.git
Cloning into 'git-start'...
remote: Enumerating objects: 3, done.
remote: Counting objects: 100% (3/3), done.
remote: Total 3 (delta 0), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (3/3), done.
Checking connectivity... done.

Administrator@ZHANGYW6668 MINGW64 /f/worksp

{% endhighlight %}
如果想在克隆远程仓库的时候，自定义本地仓库的名字，可以使用如下命令：
<pre>
git clone https://github.com/ivanzz1001/git-start.git mygit-start
</pre>
这将执行与上一个命令相同的操作，不过在本地创建的仓库名字变为```mygit-start```。

Git支持多种传输协议。上面的例子使用的是https协议，此外还支持git协议。这里不再详述。

### 2.2 在现有目录中初始化仓库
如果不克隆现有的仓库，而是打算使用Git来对现有的项目进行管理。假设有一个项目的目录是```F:\worksp\git-sample```，只需进入该项目的目录并输入：
<pre>
git init
</pre>
执行上面的命令，输出结果如下：
{% highlight string %}
$ cd git-sample

Administrator@ZHANGYW6668 MINGW64 /f/worksp/git-sample
$ git init
Initialized empty Git repository in F:/worksp/git-sample/.git/

Administrator@ZHANGYW6668 MINGW64 /f/worksp/git-sample (master)

{% endhighlight %}
该命令将创建一个名为```.git```的子目录，这个子目录含有初始化的Git仓库中所有的必须文件，这些文件是Git仓库的骨干。但是，在这个时候，我们仅仅是做了一个初始化的操作，项目里的文件还没有被跟踪。

如果是在一个已经存在文件的文件夹（而不是空文件夹）中初始化Git仓库来进行版本控制的话，应该开始跟踪这些文件并提交。可通过```git add```命令来实现对指定文件的跟踪，然后执行```git commit```提交。假设在目录```F:\worksp\git-start.git```中有一些代码需要跟踪（版本控制），比如一个Python代码叫做```hello.py```，内容如下：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("This is my first Python Programming.")
{% endhighlight %}

可通过```git add```命令来实现对```hello.py```文件的跟踪：
<pre>
$ git add hello.py
$ git commit -m 'initial project version'
</pre>

在之后的章节中，我们再逐一解释每一条指令的意思。现在，你已经得到了一个实际维护（或者说是跟踪）着若干个文件的Git仓库。

## 3. 更新提交到仓库

### 3.1 记录每次更新到仓库
现在我们手上有了一个真实项目的Git仓库（如上面clone下来的```git-start.git```)，并从这个仓库中取出了所有文件的工作拷贝。接下来，对这些文件做些修改，在完成了一个阶段的目标之后，提交本次更新到仓库。

工作目录下的每一个文件不外乎这两种状态： 已跟踪或未跟踪。已跟踪的文件是指那些被纳入了版本控制的文件， 在上一次快照中有它们的记录，在工作一段时间后，它们的状态可能处于```未修改```、```已修改```或```已放入暂存区```。工作目录中除已跟踪文件以外的所有其他文件都属于```未跟踪```文件，它们既不存在于上次快照的记录中，也没有放入暂存区。初次克隆某个仓库的时候，工作目录中的所有文件都属于已跟踪文件，并处于未修改状态。

编辑过某些文件之后，由于自上次提交后你对它们做了修改，Git将它们标记为已修改文件。我们逐步将这些修改过的文件放入暂存区，然后提交所有暂存了的修改，如此反复。所以使用Git时文件的生命周期如下：

![git-track](https://ivanzz1001.github.io/records/assets/img/tools/git-track.png)


### 3.2 检查当前文件状态



<br />
<br />

**[参看]**

1. [git教程](https://www.yiibai.com/git/getting-started-git-basics.html)

2. [git创建远程仓库并上传代码到远程仓库中](https://blog.csdn.net/liuweixiao520/article/details/78971221)

3. [git book](https://git-scm.com/book/zh/v2)

4. [Git 基础再学习之：git checkout -- file](https://www.cnblogs.com/Calvino/p/5930656.html)

5. [git使用ssh密钥](https://www.cnblogs.com/superGG1990/p/6844952.html)


<br />
<br />
<br />

