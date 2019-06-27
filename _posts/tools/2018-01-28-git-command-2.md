---
layout: post
title: git常用命令(下)
tags:
- tools
categories: tools
description: git常用命令(上)
---

本章我们介绍一下Git中的一些常用命令。

<!-- more -->


## 1. git merge命令
```git merge```命令用于将两个或两个以上的开发历史加入(合并）到一起。其基本语法格式如下：
{% highlight string %}
git merge [-n] [--stat] [--no-commit] [--squash] [--[no-]edit]
    [-s <strategy>] [-X <strategy-option>] [-S[<keyid>]]
    [--[no-]allow-unrelated-histories]
    [--[no-]rerere-autoupdate] [-m <msg>] [<commit>…​]
git merge --abort
git merge --continue
{% endhighlight %}

1) **描述**

将来自命令提交的更改合并到当前分支。该命令由```git pull```用于合并来自另一个存储库的更改，可以手动使用将更改从一个分支合并到另一个分支。

假设我们存在如下的历史版本记录，并且当前我们处于```master```分支:
<pre>
     A---B---C topic
    /
D---E---F---G master
</pre>t
然后在master分支上执行```git merge topic```命令，这将会重放自```topic```分支创建以来所做的所有修改，直到```topic```分支的当前提交```C```，并且在合并完成之后创建一个新的提交：
<pre>
     A---B---C topic
    /         \
D---E---F---G---H master
</pre>


2) **示例**

以下是一些示例：

* 示例1

合并分支```fixes```和```enhancement```，在当前分支的顶部使它们合并：
<pre>
$ git merge fixes enhancement
</pre>

* 示例2

合并```obsolete```分支到当前分支，使用```ours```合并策略：
<pre>
$ git merge -s ours obsolete
</pre>

* 示例3

将分支```maint```合并到当前分支，但不要自动进行新的提交：
<pre>
$ git merge --no-commit maint
</pre>
当你想要对合并进行进一步更改时，可以使用此选项，或者想要自己编写合并提交消息。应该不要滥用这个选项来潜入到合并提交中。小修补程序，如版本名称是可以接受的。


* 示例4

将分支```dev```合并到当前分支中，自动进行新的提交：
<pre>
$ git merge dev
</pre>

## 2. git mergetool命令
```git mergetool```命令用于运行合并冲突解决工具来解决合并冲突。具体语法格式如下：
{% highlight string %}
git mergetool [--tool=<tool>] [-y | --[no-]prompt] [<file>…​]
{% endhighlight %}

1) **描述**

```git mergtool```命令用于运行合并冲突解决工具来解决合并冲突。使用```git mergetool```运行合并实用程序来解决合并冲突。它通常在Git合并后运行。

如果给出一个或多个```<file>```参数，则将运行合并工具程序来解决每个文件的差异（跳过那些没有冲突的文件）。指定目录将包括该路径中的所有未解析文件。如果没有指定```<file>```名称，```git mergetool```将在具有合并冲突的每一个文件上运行合并工具程序。

2） **示例**

以下是一些示例：

Git设置```mergetool```可视化工具。可以设置```BeyondCompare```、```DiffMerge```等作为Git的比较和合并的可视化工具，方便操作。设置如下：

先下载并安装```BeyondCompare```、```DiffMerge```等，这里以```BeyondCompare```为例。设置Git配置，设置BeyondCompare的Git命令如下：
{% highlight string %}
#difftool 配置  
git config --global diff.tool bc4  
git config --global difftool.bc4.cmd "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\""


#mergeftool 配置  
git config --global merge.tool bc4
git config --global mergetool.bc4.cmd  "\"c:/program files (x86)/beyond compare 4/bcomp.exe\" \"$LOCAL\" \"$REMOTE\" \"$BASE\" \"$MERGED\""  
git config --global mergetool.bc4.trustExitCode true

#让git mergetool不再生成备份文件(*.orig)  
git config --global mergetool.keepBackup false
{% endhighlight %}
使用方法如下：

* diff使用方法

<pre>
git difftool HEAD    //比较当前修改情况
</pre>

* merge使用方法
<pre>
git mergetool
</pre>


## 3. git log命令
```git log```命令用于显示提交日志信息。其基本使用语法如下：
{% highlight string %}
git log [<options>] [<revision range>] [[\--] <path>…]
{% endhighlight %}

1) **描述**

```git log```命令用于显示提交日志信息。该命令采用适用于```git rev-list```命令的选项来控制显示的内容以及如何适应于```git diff -*```命令的选项，以及控制如何更改每个提交引入的内容。

2) **示例**

以下是一些示例：

* 显示整个提交历史记录，但跳过合并
{% highlight string %}
$ git log --no-merges
commit c5f8a258babf5eec54edc794ff980d8340396592
Author: maxsu <your_email@mail.com>
Date:   Wed Jul 12 22:07:59 2017 +0800

    commit a new file: newfile.txt
... ...
{% endhighlight %}

* 显示自```v2.6.12```版本以来所有提交更改```include/scsi```或```drivers/scsi```子目录中的任何文件的所有提交
<pre>
$ git log master include/scsi drivers/scsi
</pre>

* 显示最近两周的更改文件```gitk```。```--```是必要的，以避免与名为```gitk```的分支混淆
<pre>
$ git log --since="2 weeks ago" --- gitk
</pre>

* 显示```test```分支中尚未在```release```分支中的提交，以及每个提交修改的路径列表
<pre>
$ git log --name-status release..test
</pre>

* 显示更改```builtin/rev-list.c```的提交，包括在文件被赋予其现有名称之前发生的提交
<pre>
$ git log --follow builtin/rev-list.c
</pre>

* 显示在任何本地分支中的所有提交，但不包括任何远程跟踪分支的起始点(```origin```不具有）
<pre>
$ git log --branches --not --remote=orgin
</pre>

* 显示本地主服务器中的所有提交，但不显示任何远程存储库主分支
<pre>
$ git log master --not --remote=*/master
</pre>

* 显示历史，包括变化差异、但仅从```主分支```的角度来看，忽略来自合并分支的提交，并显示合并引入的变化的完全差异。只有当遵守在一个整合分支上合并所有主题分支的严格策略时，这才有意义
<pre>
$ git log -p -m --first-parent
</pre>

* 显示文件```main.c```中函数```main()```随着时间的推移而演变
<pre>
$ git log -L '/int main/',/^}/:main.c
</pre>

* 显示最近3次提交
<pre>
$ git log -3
</pre>

* 根据提交ID查询日志
<pre>
#查询ID(如：6bab70a08afdbf3f7faffaff9f5252a2e4e2d552)之前的记录，包含commit
$ git log commit_id  　　

#查询commit1与commit2之间的记录，包括commit1和commit2
$ git log commit1_id commit2_id 

#同上，但是不包括commit1
$ git log commit1_id..commit2_id 
</pre>
其中，```commit_id```可以是提交哈希值的简写模式，也可以使用```HEAD```代替。```HEAD```代表最后一次提交，```HEAD^```为最后一个提交的父提交，等同于```HEAD~1```。```HEAD~2```代表倒数第二次提交。

```--pretty```按指定格式显示日志信息，可选项有：oneline、short、medium、full、fuller、email、raw以及format。默认为medium，可以通过修改配置文件来指定默认的方式。
<pre>
$ git log (--pretty=)oneline
</pre>

常见的```format```选项：
<pre>
#选项     #说明
%H      提交对象(commit)的完整哈希字串
%h      提交对象的简短哈希字串
%T      树对象(tree)的完整哈希字串
%t      树对象的简短哈希字串
%P      父对象(parent)的完整哈希字串
%p      父对象的简短哈希字串
%an     作者(author)的名字
%ae     作者的电子邮件地址
%ad     作者修订日期(可以用 -date= 选项定制格式)
%ar     作者修订日期，按多久以前的方式显示
%cn     提交者(committer)的名字
%ce     提交者的电子邮件地址
%cd     提交日期
%cr     提交日期，按多久以前的方式显示
%s      提交说明
</pre>
注： ```作者```是指最后一次修改文件的人； 而```提交者```是指提交该文件的人

<pre>
git log --pretty=format:"%an %ae %ad %cn %ce %cd %cr %s" --graph
</pre>

git的其他选项有：
{% highlight string %}
--mergs: 查看所有合并过的提交历史记录

--no-merges: 查看所有未被合并过的提交信息

--author=someonet: 查询指定作者的提交记录

--since，--affter: 仅显示指定时间之后的提交(不包含当前日期)

--until，--before:仅显示指定时间之前的提交(包含当前日期)
{% endhighlight %}

例如：
<pre>
$ git log --author=maxsu
$ git log --before={3,weeks, ago} --after={2018-04-18}
</pre>




## 6. git fetch命令

```git fetch```命令用于从另一个存储库下载对象和引用。基本语法格式如下：
{% highlight string %}
git fetch [<options>] [<repository> [<refspec>…]]
git fetch [<options>] <group>
git fetch --multiple [<options>] [(<repository> | <group>)…]
git fetch --all [<options>]
{% endhighlight %}

1) **描述**

从一个或多个其他存储库中获取分支和/或标签（统称为```引用```)以及完成其历史所必须的对象。远程跟踪分支已更新（Git术语叫做```commit```)，需要将这些更新取回本地，这时就要用到```git fetch```命令。


默认情况下，还会获取指向正在获取的历史记录的任何标签，效果是获取指向您感兴趣的分支的标签。可以使用```--tags```或```--no-tags```选项或通过配置远程```.<name>.tagOpt```来更改此默认行为。通过使用显式提取标签的```refspec```，可以获取不指向您感兴趣的分支的标签。

```git fetch```可以从单个命名存储库或URL中获取，也可以从多个存储库中获取。如果给定了```<group>```，并且配置文件中有一个远程```<group>```条目，获取参考名称以及它们所指向的对象名称被写入到```.git/FETCH_HEAD```中。此信息可能由脚本或其他git命令使用，如```git pull```。

2) **示例**

如下是一些使用示例：

* 更新远程跟踪分支
<pre>
$ git fetch origin
</pre>
上述命令从远程```refs/heads/```命名空间复制所有分支，并将它们存储到本地的*refs/remotes/origin/*命名空间中。除非分支使用```.<name>.fetch```选项来指定非默认的```refspec```。

* 明确使用refspec

<pre>
$ git fetch origin +pu:pu maint:tmp
</pre>

此更新（或根据需要创建）通过从远程存储库的分支```pu```和```maint```分别提取到本地存储库中的```pu```和```tmp```。即使没有快进，```pu```分支将被更新，因为它的前缀是加号； ```tmp```不会。

* 在远程分支上窥视，无需再本地存储库中配置远程
<pre>
$ git fetch git://git.kernel.org/pub/scm/git/git.git maint
$ git log FETCH_HEAD
</pre>
上面第一条命令用于从存储库中获取```maint```分支，第二条命令使用```FETCH_HEAD```来检查具有```git log```的分支。

* 获取某个远程主机的全部更新
{% highlight string %}
$ git fetch <远程主机名>
{% endhighlight %}
要更新所有分支，命令可以简写为：
<pre>
$ git fetch
</pre>

上面的命令将某个远程主机的更新，全部拉回本地。默认情况下，```git fetch```取回所有分支的更新。如果只想取回特定分支的更新，可以指定分支名，如下所示：
{% highlight string %}
$ git fetch <远程主机名> <分支名>
{% endhighlight %}

比如取回```origin```主机的```master```分支：
<pre>
$ git fetch origin master
</pre>

所取回的更新，在本地主机上要用```远程主机名/分支名```的形式读取。比如```origin```主机的```master```分支，就可以用```origin/master```读取。

```git branch```命令的```-r```选项，可以用来查看远程分支。```-a```选项可以用来查看所有分支。

{% highlight string %}
$ git branch -r
origin/master

$ git branch -a
* master
  remotes/origin/master
{% endhighlight %}
上面的命令表示，本地主机的当前分支是```master```，远程分支是```origin/master```。

取回远程主机的更新以后，可以在它的基础上，使用```git checkout```命令创建一个新的分支：
<pre>
$ git checkout -b newBranch origin/master
</pre>
上面的命令表示在```origin/master```的基础上，创建一个新的分支：newBranch。

此外，也可以使用```git merge```命令或者```git rebase```命令，在本地分支上合并远程分支：
<pre>
$ git merge origin/master
$ git rebase origin/master
</pre>
上面的命令表示在当前分支上合并```origin/master```。



## 7. git pull命令

```git pull```命令用于从另一个存储库或本地分支**获取并合并**。```git pull```命令的作用是： 取回远程主机某个分支的更新，再与本地的指定分支合并。它的完整格式稍微有点复杂。

具体的语法格式如下：
{% highlight string %}
git pull [options] [<repository> [<refspec>…]]
{% endhighlight %}

1) **描述**

将远程存储库中的更改合并到当前分支中。在默认模式下，```git pull```是```git fetch```后跟```git merge FETCH_HEAD```的缩写。更准确地说，*git pull*使用给定的参数运行*git fetch*，并调用*git merge*将检索到的分支头合并到当前分支中。使用```--rebase```，它运行```git rebase```而不是```git merge```。

2） **示例**

以下是一些示例：
{% highlight string %}
$ git pull <远程主机名> <远程分支名>:<本地分支名>
{% endhighlight %}
比如，要取回```origin```主机的```next```分支，与本地master分支合并，可以执行如下命令：
<pre>
$ git pull origin next:master
</pre>

如果远程分支(```next```)要与当前分支合并，则冒号后面的部分可以省略。上面的命令可以简写为：
<pre>
$ git pull origin next
</pre>

上面的命令表示，取回```origin/next```分支，再与当前分支合并。实质上，这等同于先做```git fetch```，再执行```git merge```：
<pre>
$ git fetch origin 
$ git merge origin/next
</pre>


在某些场合，Git会自动在本地分支与远程分支之间，建立一种追踪关系(tracking)。比如，在```git clone```的时候，所有本地分支默认与远程主机的同名分支建立追踪(track)关系，也就是说，本地的```master```分支默认追踪```orgin/master```分支。

Git也允许手动建立追踪关系：
<pre>
$ git branch --set-upstream master origin/next
</pre>
上面的命令指定```master```分支追踪```origin/next```分支。


如果当前分支与远程分支存在追踪关系（通过```git branch -vv```查看），```git pull```就可以省略分支名：
<pre>
$ git pull origin
</pre>
上面的命令表示，本地的当前分支自动与对应```origin```主机追踪分支进行合并。

如果当前分支只有一个追踪分支，连远程主机名都可以省略：
<pre>
$ git pull
</pre>
上面的命令表示，当前分支自动与唯一一个追踪分支进行合并。

如果合并需要采用```rebase```模式，可以使用```--rebase```选项:
{% highlight string %}
$ git pull --rebase <远程主机名> <远程分支名>:<本地分支名>
{% endhighlight %}

3) **git fetch 与 git pull的区别**

* git fetch: 相当于从远程获取最新版本到本地，不会自动合并
<pre>
$ git fetch origin master    
$ git log -p master..origin/master
$ git merge origin/master
</pre>
上述命令首先从远程```origin```的```master```分支上下载最新版本到```origin/master```分支上； 然后比较本地```master```分支和```orgin/master```分支的差别； 最后进行合并。

上述过程其实可以用以下更清晰的方式来进行：
<pre>
$ git fetch origin master:tmp
$ git diff tmp
$ git merge tmp
</pre>

* git pull: 相当于从远程获取最新版本并merge到本地
<pre>
$ git pull origin master
</pre>
上述命令其实相当于```git fetch```和```git merge```。在实际使用中， ```git fetch```更安全一些，因为在```merge```之前，我们可以查看更新状况，然后再决定是否合并。




## 8. git push命令

## 9. git remote命令

```git remote```命令管理一组跟踪的存储库。要参与任何一个Git项目的协作，必须要了解如何管理远程仓库。远程仓库是指托管在网络上的项目仓库，可能会有好多个，其中有些你只能读，另外有些可以写。同他人协作开发某个项目时，需要管理这些远程仓库，以便推送或拉取数据，分享各自的工作进程。管理远程仓库的工作，包括添加远程仓库，移除废弃的远程库，管理各式远程库分支，定义是否跟踪这些分支等等。

其基本的语法格式如下：
{% highlight string %}
git remote [-v | --verbose]
git remote add [-t <branch>] [-m <master>] [-f] [--[no-]tags] [--mirror=<fetch|push>] <name> <url>
git remote rename <old> <new>
git remote remove <name>
git remote set-head <name> (-a | --auto | -d | --delete | <branch>)
git remote set-branches [--add] <name> <branch>…​
git remote get-url [--push] [--all] <name>
git remote set-url [--push] <name> <newurl> [<oldurl>]
git remote set-url --add [--push] <name> <newurl>
git remote set-url --delete [--push] <name> <url>
git remote [-v | --verbose] show [-n] <name>…​
git remote prune [-n | --dry-run] <name>…​
git remote [-v | --verbose] update [-p | --prune] [(<group> | <remote>)…​]
{% endhighlight %}

1) **描述**

```git remote```命令管理一组跟踪的存储库。

2） **示例**

以下是一些示例：

* 查看当前的远程库

要查看当前配置有哪些远程仓库，可以用```git remote```命令，它会列出每个远程库的简短名字。在克隆完某个项目后，至少可以看到一个名为```origin```的远程库，Git默认使用这个名字来标识你所克隆的原始仓库：
<pre>
$ git clone http://git.oschina.net/yiibai/sample.git
$ cd sample
</pre>

a） ```git remote```不带参数，列出已经存在的远程分支
<pre>
$ git remote
origin
</pre>

b) ```git remote -v | --verbose```列出详细信息，在每一个名字后面列出其远程URL。此时，```-v```选项（译注： 此为```--verbose```的简写，取首字母），显示对应的克隆地址：
{% highlight string %}
$ git remote -v
origin  http://git.oschina.net/yiibai/sample.git (fetch)
origin  http://git.oschina.net/yiibai/sample.git (push)

Administrator@MY-PC /D/worksp/sample (master)
$ git remote --verbose
origin  http://git.oschina.net/yiibai/sample.git (fetch)
origin  http://git.oschina.net/yiibai/sample.git (push)
{% endhighlight %}

* 添加一个新的远程，抓取，并从它检出一个分支
{% highlight string %}
$ git remote
origin
$ git branch -r
  origin/HEAD -> origin/master
  origin/master
$ git remote add staging git://git.kernel.org/.../gregkh/staging.git
$ git remote
origin
staging
$ git fetch staging
...
From git://git.kernel.org/pub/scm/linux/kernel/git/gregkh/staging
 * [new branch]      master     -> staging/master
 * [new branch]      staging-linus -> staging/staging-linus
 * [new branch]      staging-next -> staging/staging-next
$ git branch -r
  origin/HEAD -> origin/master
  origin/master
  staging/master
  staging/staging-linus
  staging/staging-next
$ git checkout -b staging staging/master
...
{% endhighlight %}

* 添加远程仓库

要添加一个新的远程仓库，可以指定一个简单的名字，以便将来引用，运行```git remote add [shortname] [url]```:
{% highlight string %}
$ git remote
　　origin
$ git remote add pb http://git.oschina.net/yiibai/sample.git
$ git remote -v origin http://git.oschina.net/yiibai/sample.git
　　pb http://git.oschina.net/yiibai/sample2.git 

# 现在可以用字串 pb 指代对应的仓库地址了.比如说,要抓取所有 Paul 有的,但本地仓库没有的信息,可以运行 git fetch pb:

$ git fetch pb
　　remote: Counting objects: 58, done.
　　remote: Compressing objects: 100% (41/41), done.
　　remote: Total 44 (delta 24), reused 1 (delta 0)
　　Unpacking objects: 100% (44/44), done.
　　From http://git.oschina.net/yiibai/sample2.git
　　* [new branch] master -> pb/master
　　* [new branch] ticgit -> pb/ticgit
{% endhighlight %}


* 模仿git clone，但只跟踪选定的分支

{% highlight string %}
$ mkdir project.git
$ cd project.git
$ git init
$ git remote add -f -t master -m master origin git://example.com/git.git/
$ git merge origin

{% endhighlight %}


## 10. git submodule命令

```git submodule```命令用于初始化、更新或者检查子模块。其基本语法格式如下：
{% highlight string %}
git submodule [--quiet] add [<options>] [--] <repository> [<path>]
git submodule [--quiet] status [--cached] [--recursive] [--] [<path>…​]
git submodule [--quiet] init [--] [<path>…​]
git submodule [--quiet] deinit [-f|--force] (--all|[--] <path>…​)
git submodule [--quiet] update [<options>] [--] [<path>…​]
git submodule [--quiet] summary [<options>] [--] [<path>…​]
git submodule [--quiet] foreach [--recursive] <command>
git submodule [--quiet] sync [--recursive] [--] [<path>…​]
git submodule [--quiet] absorbgitdirs [--] [<path>…​]
{% endhighlight %}

1) **使用场景**

基于公司的项目会越来越多，常常需要提取一个公共的类库提供给多个项目使用，但是这个Library怎么和git在一起方便管理呢？ 

我们需要解决下面几个问题：

* 如何在git项目中导入Library库

* library库在其他项目中被修改了可以更新到远程的代码库中？

* 其他项目如何获取到library库最新的提交？

* 如何在clone的时候能够自动导入library库？

解决以上问题，可以考虑使用Git的```submodule```来解决。

2) **submodule是什么**

```git submodule```是一个很好的多项目使用共同类库的工具，它允许类库项目作为repository，子项目作为一个单独的Git项目存在于父项目中。子项目可以有自己独立的```commit```、```push```、```pull```。而父项目以submodule的形式包含子项目，父项目可以指定子项目header，父项目中的提交信息包含submodule的信息，在clone父项目的时候可以把submodule初始化。

3) **示例**

以下是一些示例：

* 添加子模块

在本例中，我们将会添加一个名为```DbConnector```的库：
{% highlight string %}
$ git submodule add http://github.com/chaconinc/DbConnector
Cloning into 'DbConnector'...
remote: Counting objects: 11, done.
remote: Compressing objects: 100% (10/10), done.
remote: Total 11 (delta 0), reused 11 (delta 0)
Unpacking objects: 100% (11/11), done.
Checking connectivity... done.
{% endhighlight %}

默认情况下，子模块会将子项目放到一个与仓库同名的目录中，本例中是```DbConnector```。如果你想要放到其他地方，那么可以在命令结尾添加一个不同的路径。

如果这时运行```git status```，你会注意到几个东西：
{% highlight string %}
$ git status
On branch master
Your branch is up-to-date with 'origin/master'.

Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

    new file:   .gitmodules
    new file:   DbConnector
{% endhighlight %}

首先应当注意到新的```.gitmodules```文件。该文件保存了项目URL与已经拉取的本地目录之间的映射：
<pre>
$ cat .gitmodules
[submodule "DbConnector"]
    path = DbConnector
    url = http://github.com/chaconinc/DbConnector
</pre>
如果有多个子模块，该文件中就会有多条记录。要重点注意的是，该文件也像```.gitignore```文件一样受到版本控制。它会和该项目的其他部分一样被拉取推送。这就是克隆该项目的人知道去哪获得子模块的原因。

在```git status```输出中列出的另一个是项目文件夹记录。如果你运行```git diff```，会看到类似下面的信息：
{% highlight string %}
$ git diff --cached DbConnector
diff --git a/DbConnector b/DbConnector
new file mode 160000
index 0000000..c3f01dc
--- /dev/null
+++ b/DbConnector
@@ -0,0 +1 @@
+Subproject commit c3f01dc8862123d317dd46284b05b6892c7b29bc
{% endhighlight %}
虽然```DbConnector```是工作目录中的一个子目录，但Git还是会将它视作一个子模块。当你不在那个目录中时，Git并不会跟踪它的内容，而是将它看作该仓库中的一个特殊提交。

如果你想看到更漂亮的差异输出，可以给```git diff```传递```--submodule```选项：
{% highlight string %}
$ git diff --cached --submodule
diff --git a/.gitmodules b/.gitmodules
new file mode 100644
index 0000000..71fc376
--- /dev/null
+++ b/.gitmodules
@@ -0,0 +1,3 @@
+[submodule "DbConnector"]
+       path = DbConnector
+       url = http://github.com/chaconinc/DbConnector
Submodule DbConnector 0000000...c3f01dc (new submodule)
{% endhighlight %}


* 克隆含有子模块的项目

接下来我们将会克隆一个含有子模块的项目。当你在克隆这样的项目时，默认会包含该子模块目录，但其中没有任何文件：
{% highlight string %}
$ git clone http://github.com/chaconinc/MainProject
Cloning into 'MainProject'...
remote: Counting objects: 14, done.
remote: Compressing objects: 100% (13/13), done.
remote: Total 14 (delta 1), reused 13 (delta 0)
Unpacking objects: 100% (14/14), done.
Checking connectivity... done.
$ cd MainProject
$ ls -la
total 16
drwxr-xr-x   9 schacon  staff  306 Sep 17 15:21 .
drwxr-xr-x   7 schacon  staff  238 Sep 17 15:21 ..
drwxr-xr-x  13 schacon  staff  442 Sep 17 15:21 .git
-rw-r--r--   1 schacon  staff   92 Sep 17 15:21 .gitmodules
drwxr-xr-x   2 schacon  staff   68 Sep 17 15:21 DbConnector
-rw-r--r--   1 schacon  staff  756 Sep 17 15:21 Makefile
drwxr-xr-x   3 schacon  staff  102 Sep 17 15:21 includes
drwxr-xr-x   4 schacon  staff  136 Sep 17 15:21 scripts
drwxr-xr-x   4 schacon  staff  136 Sep 17 15:21 src
$ cd DbConnector/
$ ls
{% endhighlight %}
其中有```DbConnector```目录，不过是空的。你必须运行两个命令：```git submodule init```用来初始化本地配置文件，而```git submodule update```则从该项目中抓取所有数据并检出父项目中列出的合适提交。

{% highlight string %}
$ git submodule init
Submodule 'DbConnector' (http://github.com/chaconinc/DbConnector) registered for path 'DbConnector'
$ git submodule update
Cloning into 'DbConnector'...
remote: Counting objects: 11, done.
remote: Compressing objects: 100% (10/10), done.
remote: Total 11 (delta 0), reused 11 (delta 0)
Unpacking objects: 100% (11/11), done.
Checking connectivity... done.
Submodule path 'DbConnector': checked out 'c3f01dc8862123d317dd46284b05b6892c7b29bc'
{% endhighlight %}
现在```DbConnector```子目录是处在和之前提交时相同的状态了。不过还有更简单一点的方式，如果给```git clone```命令传递```--recursive```选项，它就会自动初始化并更新仓库中的每一个子模块。
{% highlight string %}
$ git clone --recursive http://github.com/chaconinc/MainProject
Cloning into 'MainProject'...
remote: Counting objects: 14, done.
remote: Compressing objects: 100% (13/13), done.
remote: Total 14 (delta 1), reused 13 (delta 0)
Unpacking objects: 100% (14/14), done.
Checking connectivity... done.
Submodule 'DbConnector' (http://github.com/chaconinc/DbConnector) registered for path 'DbConnector'
Cloning into 'DbConnector'...
remote: Counting objects: 11, done.
remote: Compressing objects: 100% (10/10), done.
remote: Total 11 (delta 0), reused 11 (delta 0)
Unpacking objects: 100% (11/11), done.
Checking connectivity... done.
Submodule path 'DbConnector': checked out 'c3f01dc8862123d317dd46284b05b6892c7b29bc'
{% endhighlight %}

* 删除submodule

Git并不支持直接删除submodule，需要手动删除对应的文件：
{% highlight string %}
$ cd pod-project

$ git rm --cached pod-library
$ rm -rf pod-library
$ rm .gitmodules


# 更改git的配置文件config:
$ vim .git/config
{% endhighlight %}
可以看到```submodule```的配置信息：
<pre>
[submodule "pod-library"]
  url = git@github.com:jjz/pod-library.git
</pre>
删除submodule相关的内容，然后提交到远程服务器：
<pre>
$ git commit -a -m 'remove pod-library submodule'
</pre>


## 13. git describe命令

```git describe```命令显示离当前提交最近的标签。基本语法格式如下：
{% highlight string %}
git describe [--all] [--tags] [--contains] [--abbrev=<n>] [<commit-ish>…​]
git describe [--all] [--tags] [--contains] [--abbrev=<n>] --dirty[=<mark>]
{% endhighlight %}

1) **描述**

该命令查找可提交访问的最新标记。如果标签指向提交，则只显示标签。否则，它将标记名称与标记对象之上的其他提交数量以及最近提交的缩写对象名称后缀。

默认情况下（不包括```--all```或者```--tags```)Git描述只显示注释标签。

2） **示例**

如果符合条件的```tag```指向最新提交则只显示tag名字，否则会有相关后缀来描述该tag之后有多少次提交以及最新的提交```commit id```。不加任何参数的情况下，```git describe```只会列出带有注释的tag:
<pre>
$ git describle --tags
tag1-2-g026498b
</pre>
上面```2```表示自打```tag1```以来有2次提交(commit)。```g026498b```中```g```为git的缩写，在多种管理工具并存的环境中很有用处。




## 14. git rebase命令
```git rebase```命令在另一个分支基础之上重新应用某个提交，用于把一个分支的修改合并到当前分支。其基本语法格式如下：
{% highlight string %}
git rebase [-i | --interactive] [options] [--exec <cmd>] [--onto <newbase>]
    [<upstream> [<branch>]]
git rebase [-i | --interactive] [options] [--exec <cmd>] [--onto <newbase>]
    --root [<branch>]
git rebase --continue | --skip | --abort | --quit | --edit-todo
{% endhighlight %}


1) **示例**
假设你现在基于远程分支```origin```，创建一个叫做```mywork```的分支。
{% highlight string %}
$ git checkout -b mywork origin
{% endhighlight %}
结果如下所示：

![git-create-branch](https://ivanzz1001.github.io/records/assets/img/tools/git-create-branch.png)

现在我们在这个分支(mywork)上做一些修改，然后生成两个提交（commit):
{% highlight string %}
$ vi file.txt
$ git commit
$ vi otherfile.txt
$ git commit
...
{% endhighlight %}
但是与此同时，有些人也在```origin```分支上做了一些修改并且做了提交。这就意味着```origin```和```mywork```这两个分支各自```前进```了，它们之间```分叉```了。

![git-branch-split](https://ivanzz1001.github.io/records/assets/img/tools/git-branch-split.png)

在这里你可以用```git pull```命令把```origin```分支上的修改拉下来并且和你的修改合并，结果看起来就像一个新的```合并的提交```(merge commit)。

![git-rebase-merge](https://ivanzz1001.github.io/records/assets/img/tools/git-rebase-merge.png)


但是，如果你想让```mywork```分支历史看起来像没有经过任何合并一样，也可以使用```git rebase```，如下所示：
<pre>
$ git checkout mywork
$ git rebase origin
</pre>
这些命令会把你的```mywork```分支里的每个提交（commit）取消掉，并且把它们临时保存为补丁(patch)（这些补丁放到```.git/rebase```目录中），然后把```mywork```分支更新到最新的```origin```分支，最后把保存的这些补丁应用到```mywork```分支上。

![git-rebase](https://ivanzz1001.github.io/records/assets/img/tools/git-rebase.png)

当```mywork```分支更新之后，它会指向这些新创建的提交(commit)，而那些老的提交会被丢弃。如果运行垃圾收集命令(pruning garbage collection)，这些被丢弃的提交就会删除。

![git-rebase-abandon](https://ivanzz1001.github.io/records/assets/img/tools/git-rebase-abandon.png)


现在我们可以看一下用合并(merge)和用```rebase```所产生的历史的区别：

![git-rebase-diff1](https://ivanzz1001.github.io/records/assets/img/tools/git-rebase-diff1.png)

![git-rebase-diff2](https://ivanzz1001.github.io/records/assets/img/tools/git-rebase-diff2.png)

在```rebase```的过程中，也许会出现冲突(conflict)。在这种情况下，Git会停止```rebase```并会让你去解决冲突；在解决完冲突之后，用```git add```命令去更新这些内容的索引(index)，然后，你无需执行```git commit```，只要执行：
<pre>
$ git rebase --continue
</pre>
这样git会继续应用(apply)余下的补丁。

在任何时候，可以用```--abort```参数来终止```rebase```的操作，并且```mywork```分支会回到```rebase```开始前的状态：
{% highlight string %}
$ git rebase --abort
{% endhighlight %}


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

9. [如何加速国内Github访问](https://www.jianshu.com/p/66facbd8926a)
<br />
<br />
<br />

