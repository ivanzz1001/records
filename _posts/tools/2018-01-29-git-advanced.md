---
layout: post
title: Git进阶知识
tags:
- tools
categories: tools
description: git进阶知识
---

本章介绍一下Git的一些进阶知识：

* Git HEAD介绍

* Git存储位置与方式

* Git暂存区深入介绍

<!-- more -->


## 1. Git HEAD
很多Git使用者对```HEAD```都有所了解，但仅限于它指向当前所在的分支。下面我们将较为详细深入的介绍一下```HEAD```到底是什么。

首先看一下当前分支的提交记录：
{% highlight string %}
$ git log --oneline
937dd8e (HEAD -> master) c3
c485217 c2
6c89271 c1
{% endhighlight %}
从上面我们可以看到```HEAD```指向master分支，master分支指向```sha-1```值为937dd8e(缩写）的commit提交。下面看一个更为形象的图示：


![git-head](https://ivanzz1001.github.io/records/assets/img/tools/git-head.jpg)

1) **HEAD是什么**

```HEAD```是一个指针，通常情况下可以将它与当前分支等同（其实它是指向当前分支）。```.git```目录中有一个```HEAD```文件，它记录这```HEAD```的内容，查看其中保存的信息：
<pre>
$ cat .git/HEAD
ref: refs/heads/master
</pre>
可以看到HEAD指向master分支，下面再来看一下refs/heads/master中的内容：
<pre>
$ cat .git/refs/heads/master
937dd8e299b04933bf6010b58f2145720b948aed
</pre>
master分支也只是一个存放40位```sha-1```值的文件而已，正是当前分支所指向commit的sha-1值。

2) **切换分支**

当前位于```master```分支，现在切换到```Develop```分支，命令如下：
<pre>
$ git checkout Develop
Switched to branch Develop
</pre>
可以看到现在已经切换到```Develop```分支了。再来看```HEAD```文件的内容：
<pre>
$ cat .git/HEAD
ref: refs/heads/Develop
</pre>
可以看到现在HEAD已经指向了Develop分支。再来看refs/heads/Develop中的内容：
<pre>
$ cat .git/refs/heads/Develop
bad523df42ac99f7382d9d49eb4824cc68beeb5b
</pre>
也就是说```HEAD```通常会指向当前所在的分支。

3) **detached HEAD**

当```HEAD```没有指向某一个分支，而是指向一个commit，则会形成detached HEAD。我们会在下面的```Git detached HEAD```中进行介绍。

4) **HEAD缩写形式**

在```Git1.8.5```版本之后，HEAD有一个缩写形式```@```，确实可以省略几个字符：
<pre>
$ git reset HEAD^
</pre>
上面的代码可以缩写为以下形式：
<pre>
$ git reset @^
</pre>
虽然少写了几个字符，但是总感觉不能够见词达意，好像失去了一点什么。

## 2. Git ORIG_HEAD用法介绍
从外观来看，```ORIG_HEAD```与```HEAD```有些相似。```ORIG```中文是*‘最初的’*或者*‘原本的’*意思，ORIG_HEAD也就有原本HEAD的含义，作用也印证这点。

在```.git```目录中，与HEAD文件类似，还有```ORIG_HEAD```文件。当进行一些有风险的操作的时候，如```git reset```、```git merge```或者```git rebase```，Git会将原来所指向commit对象的```sha-1```值存放于```ORIG_HEAD```文件中。也就是说ORIG_HEAD可以让我们找到最近一次危险操作之前的HEAD位置。

首先看一下当前分支的提交历史：
{% highlight string %}
$ git log --oneline
903d5af (HEAD -> master) c4
4f66476 c3
e577355 c2
b0aa963 c1
{% endhighlight %}
记住当前HEAD所在位置的commit的```sha-1```值```903d5af```。下面执行回滚操作：
{% highlight string %}
$ git reset HEAD^ --hard
HEAD is now at 4f66476 c3
{% endhighlight %}
现在看```ORIG_HEAD```文件中的内容：
{% highlight string %}
$ cat .git/ORIG_HEAD
903d5af6a38aed2d4533b3199f981e3f2d371da8
{% endhighlight %}
内容恰好是在执行回滚操作前HEAD所在的commit对象的```sha-1```值。

## 3. detached HEAD详解
HEAD是一个指针，通常情况下，它指向当前所在分支，而分支又指向一个commit提交。HEAD并不总指向一个分支，某些时候仅指向某个commit提交，这就形成```detached HEAD```。

### 3.1 产生原因
1) 使用```git checkout```指令切换到指定commit提交

2) 使用```git checkout```指令切换到远程分支

3） ```git rebase```操作也会产生detached HEAD状态

### 3.2 状态分析
1） **git checkout切换到指定commit**

不要把```detached HEAD```想的很特别，其实就是HEAD指向某个commit提交，而这个提交恰巧没有被分支指向。首先看一下当前master分支的提交历史：
{% highlight string %}
$ git log --oneline
66050f0 (HEAD -> master) c4
8f3244b c3
2bc214d c2
296f5f0 c1
{% endhighlight %}

共四个commit提交，下面通过```git checkout```命令切换到c2提交：
{% highlight string %}
$ git checkout 2bc214d
Note: checking out '2bc214d'.

You are in 'detached HEAD' state. You can look around, make experimental
changes and commit them, and you can discard any commits you make in this
state without impacting any branches by performing another checkout.

If you want to create a new branch to retain commits you create, you may
do so (now or later) by using -b with the checkout command again. Example:

  git checkout -b <new-branch-name>

HEAD is now at 2bc214d... this is my commit info note.

$ git status
HEAD detached at 2bc214d
nothing to commit, working directory clean

{% endhighlight %}
可见当前```HEAD```已经指向```2bc214d```提交，但是此commit提交没有分支指向它，处于```detached HEAD```状态。

下面我们修改一下文件，然后进行一次新的提交，这时它的表现与普通分支无异，HEAD指针继续向前推进。但是HEAD并没有指向一个分支，也就是新的提交并没有被分支指向：
<pre>
$ git commit -m "detached HEAD"
</pre>
进行上述提交后，查看当前状态：
{% highlight string %}
$ git status
HEAD detached at 426ad27
nothing to commit, working directory clean
{% endhighlight %}
可以看到```HEAD```依然处于detached HEAD状态。在```detached HEAD```状态提交的commit与普通分支提交commit相比，当切换到其他分支后不容易被找到：
<pre>
a) 要么记住提交的sha-1值

b） 或者通过git reflog查找

c) 如果提交长期没有被再次使用，就会被资源回收机制收回。
</pre>
如果想要保留这个提交，可以在此提交的基础上创建一个分支：
<pre>
$ git branch newBr
</pre>
因为HEAD当前指向```426ad27```提交，上面代码与如下等同：
<pre>
$ git branch newBr 426ad27
</pre>

2) **git checkout切换到远程分支**

首先下载远程分支到本地，代码如下：
{% highlight string %}
$ git clone https://github.com/githubantzone/myGit.git remoteN
{% endhighlight %}
下面来查看一下远程分支：
{% highlight string %}
$ git branch --remote
origin/HEAD -> origin/master
origin/master
origin/newBr
{% endhighlight %}
下面切换到```origin/newBr```分支，命令如下：
{% highlight string %}
$ git checkout origin/newBr
Note: checking out 'origin/newBr'.

You are in 'detached HEAD' state. You can look around, make experimental
changes and commit them, and you can discard any commits you make in this
state without impacting any branches by performing another checkout.

If you want to create a new branch to retain commits you create, you may
do so (now or later) by using -b with the checkout command again. Example:

  git checkout -b <new-branch-name>

HEAD is now at 6326ed3... this is my commit info note.
{% endhighlight %}

由此可以看到，更准确的说，```detached HEAD```是因为HEAD没有指向```本地```的分支导致。当然在本地建立与远程分支对应的本地分支，切换到本地分支自然就不会产生```detached HEAD```状态。

克隆远程分支的时候，Git会自动在本地创建一个master分支来跟踪```origin/master```分支，但是不会在本地自动创建其他分支来跟踪远程服务器上除master分支外的其他分支，代码如下：
{% highlight string %}
$ git branch
* master
{% endhighlight %}
只在本地创建了master分支，下面创建一个本地分支跟踪```origin/newBr```，代码如下：
{% highlight string %}
$ git checkout -t origin/newBr
Switched to a new branch 'newBr'
Branch 'newBr' set up to track remote branch 'newBr' from 'origin'.
{% endhighlight %}


## 4. Git存储位置与方式



<br />
<br />

**[参看]**

1. [Git ORIG_HEAD用法介绍](http://www.softwhy.com/article-8502-1.html)




<br />
<br />
<br />

