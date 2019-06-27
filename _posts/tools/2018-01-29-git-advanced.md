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

Git是一个工具，只要熟练掌握它的操作就能完成工作中的绝大多数任务。我们很少会考虑Git历史提交数据存储在哪，又以何种方式存储，下面对此进行一下介绍。首先看一段代码实例：
{% highlight string %}
let obj = {
  webName: "蚂蚁部落",
  address: "青岛市南区"
}
obj.age = 4;
console.log(obj.webName);
console.log(obj.address);
console.log(obj.age);
{% endhighlight %}
上面是一个```对象直接量```的简单演示，```对象直接量```以键/值对方式存储数据，通过键可以获取对应的值。

本质上Git存储数据的方式也是如此，键就是数据内容```sha-1```，而值自然就是对象的内容。现在我们初始化一个全新的仓库```git-store```来进行测试：
<pre>
$ mkdir git-store
$ cd git-store
$ git init
Initialized empty Git repository in F:/worksp/git-store/.git/
$ ls -al
total 8
drwxr-xr-x 1 Administrator None 0 六月 26 10:03 ./
drwxr-xr-x 1 Administrator None 0 六月 26 10:03 ../
drwxr-xr-x 1 Administrator None 0 六月 26 10:03 .git/
</pre>
初始化一个全新的仓库之后，在目录中只有一个隐藏的```.git```文件夹。我们进入```.git/objects```目录，可以看到有如下两个目录：
<pre>
$ ls .git/objects/
info/  pack/
</pre>
这两个子目录是系统自动创建的，可以看到当前并没有存储任何数据。

下面开始向里面存储数据，代码如下：
{% highlight string %}
$ echo "蚂蚁部落" | git hash-object -w --stdin
210a3e5558a2c25c0a577a3f2555c2f82e5529c6
{% endhighlight %}
上面```git hash-object```命令用来计算所要存储对象的```sha-1```值；```-w```选项用于指定对数据进行存储；```--stdin```选项表示内容是通过标准输入设备获取的。

现在来看一下```.git/objects```目录中的内容：
<pre>
$ ls .git/objects/
21/  info/  pack/
$ ls .git/objects/21
0a3e5558a2c25c0a577a3f2555c2f82e5529c6
</pre>
现在多出了一个子目录，名字是```21```，并且该文件夹下有一个名为 *0a3e5558a2c25c0a577a3f2555c2f82e5529c6*的文件，通过对比我们得出如下结论：

* git的对象存储于```.git/objects```目录中

* 以对象的```sha-1```值前两位作为子目录名称，具体存储对象内容的文件名为```sha-1```值的后38位

现在我们查看*0a3e5558a2c25c0a577a3f2555c2f82e5529c6*文件的内容：
{% highlight string %}
$ cat .git/objects/21/0a3e5558a2c25c0a577a3f2555c2f82e5529c6
xK▒▒OR04fx1▒▒ŬƗ▒+^L▒▒f▒
~
{% endhighlight %}
可以看到是乱码。这是因为Git存储的并不是原始数据```蚂蚁部落```，而是通过zlib压缩的内容。

下面根据```sha-1```值这个键来查看对应的值（也就是存储的数据）：
<pre>
$ git cat-file -p 210a3e5558a2c25c0a577a3f2555c2f82e5529c6
蚂蚁部落
</pre>

## 5. Git暂存区深入理解
我们知道可以通过```git add```命令将工作区中的内容加入暂存区，代码实例如下：
<pre>
$ git add readme.txt
</pre>
上述命令将工作区中的```readme.txt```文件加入到暂存区。从```暂存区```名字来理解，此区域好像是一个仓库，把将要提交的内容暂时存放于此。上述理解从感性上来说没什么问题， 并且有助于接受此概念。然而，暂存区的实质是什么呢？ 仅仅是一个文件罢了，截图如下：

![git-index-zone](https://ivanzz1001.github.io/records/assets/img/tools/git-index-zone.png)

所谓的暂存区仅仅是```.git```目录下的index文件罢了，这也是为什么被称为index了。

下面先查看一下当前项目的状态：
{% highlight string %}
$ git status
On branch master

Initial commit

Changes to be committed:
  (use "git rm --cached <file>..." to unstage)

        new file:   readme.txt

{% endhighlight %}

## 6. Git版本的历史变迁记录
我们执行```git log```命令可以看到Git提交的历史记录。那么这个commit链是如何组织起来的呢？ 我们首先看一下当前我们的提交的历史记录：
{% highlight string %}
$ git log
commit 8223325b008ba973afe1e0df42aaa8c31716e840
Author: ivanzz1001 <1181891136@qq.com>
Date:   Thu Jun 27 10:40:01 2019 +0800

    add helloworld

commit 182339e9b659ea72202f1378ffd517d553893706
Author: ivanzz1001 <1181891136@qq.com>
Date:   Wed Jun 26 19:18:21 2019 +0800

    test index zone

{% endhighlight %}
可以看到有两个提交，最新的提交是*8223325b008ba973afe1e0df42aaa8c31716e840*，我们也可以通过查看```.git/HEAD```来查看当前分支的最新提交：
<pre>
$ cat .git/HEAD
ref: refs/heads/master

$ cat .git/refs/heads/master
8223325b008ba973afe1e0df42aaa8c31716e840
</pre>
从上面我们知道当前分支的最新提交是```822332```，那么其是如何找到其```父提交```(即Parent提交）的呢？ 我们执行如下命令：
<pre>
$ git cat-file -p 8223325b008ba973afe1e0df42aaa8c31716e840
tree 6a79d37d5372e76f690ea2b66ddb7b410615e454
parent 182339e9b659ea72202f1378ffd517d553893706
author ivanzz1001 <1181891136@qq.com> 1561603201 +0800
committer ivanzz1001 <1181891136@qq.com> 1561603201 +0800

add helloworld

$ git cat-file -p 6a79d37d5372e76f690ea2b66ddb7b410615e454
100644 blob 210a3e5558a2c25c0a577a3f2555c2f82e5529c6    antzone.txt
100644 blob 2d832d9044c698081e59c322d5a2a459da546469    hello.txt
100644 blob d9b401251bb36c51ca5c56c2ffc8a24a78ff20ae    readme.txt
</pre>
从这里我们可以看到其父提交是*182339e9b659ea72202f1378ffd517d553893706*。由此可见，Git就是通过这样的方式一级一级的形成提交链表。

注意，我们执行```git cat-file -p <commit-id>```，实际上查询的是```.git/objects```目录下的文件。其实不管是我们提交的普通代码文件，还是Git本身的提交记录信息都是存放于```.git/objects```目录中的。



<br />
<br />

**[参看]**

1. [Git ORIG_HEAD用法介绍](http://www.softwhy.com/article-8502-1.html)




<br />
<br />
<br />

