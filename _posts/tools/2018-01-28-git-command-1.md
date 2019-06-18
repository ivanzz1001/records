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

可以通过```--add```选项位某一个设置项添加多行（注意： 最后的行会覆盖前面设置的行）。假如你想要更新或取消设置的多行选项，则你需要给出一个POSIX正则表达式```value_regex```，只有那些匹配正则表达式的选项值会被更新或取消设置。相反，假如你想处理那些与正则表达式不匹配的设置，则只需在前面加一个感叹号。

```type```指示符可以为```--int```或者```--bool```，这样git config命令可以确保给定的变量为指定的类型，并将该值转换成对应类型的标准形式（```int```是简单的十进制数，```true```或```false```是bool类型的字符串表示）；或者```--path```，它进行一些路径扩展。如果没有类型说明符传递，则不会对该值进行检查或转换。

默认情况下会从system、global以及local三个地方的配置文件中读取相应的配置值，可以通过```--system```、```--global```、```--local```和```--file <filename>```选项来指定git config命令从什么地方读取配置参数。

当进行写入操作时，默认情况下会被写入到```local```配置文件。你也可以通过```--system```、```--global```、```--local```以及```--file <filename>```来指定写入到什么地方。

```git config```命令在操作成功时通常返回0， 失败时返回非零值，如下是一些相应的错误码：

* ret=3: 配置文件无效

* ret=4: 不能写入到配置文件

* ret=2: 为提供section或name

* ret=1: section或者key无效

* ret=5: 你正在尝试unset一个不存在的选项

* ret=5: 你正在尝试unset/set一个匹配多行的选项

* ret=6: 你正在尝试一个无效的正则表达式

3） **配置文件的存储位置**

这些变量可以被存储在三个不同的位置：

* ```/etc/gitconfig```文件： 包含了适用于系统所有用户和所有库的值。如果你传递的参数选项为```--system```给git config，它将明确的读写这个文件

* ```~/.gitconfig```文件或```~/.config/git/config```文件：具体到对应的登录用户。你可以通过```--global```选项使git config读写这个文件

* 位于git仓库目录下的```.git/config```文件： 只针对当前库


4) **配置用户名和email**

当安装Git后首先要做的事情就是设置用户名称和email地址。这是非常重要的，因为每次Git提交都会使用该信息。它被永久的嵌入到了你的提交中。
<pre>
$ git config --global user.name "ivanzz1001"
$ git config --global user.email "111111111@qq.com"
</pre>
重申一遍，只需要做一次这个设置。因为传递了```--global```选项，Git总是会使用该信息来处理在系统中所做的一切操作。如果希望在一个特定项目中使用不同的名称或```email```地址，可以在该项目中运行如下命令：
<pre>
$ git config --local user.name "ivanzz1001"
$ git config --local user.email "111111111@qq.com"
</pre>

5) **配置编辑器**

可以配置默认的文本编辑器，Git在需要你输入一些消息时会使用该文本编辑器。缺省情况下，Git使用系统的缺省编辑器，这通常可能是 vi 或者 vim 。如果想使用一个不同的文本编辑器，例如：Emacs，可以按照如下操作：
<pre>
$ git config --global core.editor emacs
</pre>

6) **配置比较工具**

另外一个你可能需要配置的有用的选项是缺省的比较工具，它用来解决合并时的冲突。例如，想使用```vimdiff```作为比较工具，你可以如下配置：
<pre>
$ git config --global merge.tool vimdiff
</pre>
Git可以接受```kdiff3```，```tkdiff```，```meld```，```xxdiff```，```emerge```，```vimdiff```，```gvimdiff```，```ecmerge```和```opendiff```作为有效的合并工具。也可以设置一个客户端的工具。

7) **检查配置**

如果想检查你的设置，可以使用```git config --list```命令列出Git可以找到的所有设置：
<pre>
$ git config --list
</pre>
可能会看到一个关键字出现多次，这是因为Git从不同的文件中读取相同的关键字。在这种情况下，对每个唯一的关键字，Git使用最后的那个值。

此外，也可以查看具体的某一个关键字的值，语法为```git config {key}```。例如：
<pre>
$ git config user.name
ivanzz1001
</pre>

8) **添加/删除配置项**

* 添加配置项

使用```--add```选项添加配置项，格式为: *git config [--local|--global|--system] --add section.key value*，默认是添加在```local```配置中：
<pre>
$ git config --add site.name github
</pre>

* 删除配置项

使用```--unset```选项删除配置项，格式为：*git config [--local|--global|--system] --unset section.key*。例如：
<pre>
$ git config --unset site.name
</pre>

9) **获取帮助**

如果在使用Git时需要帮助，有三种方法可以获得任何Git命令的手册页(man page)帮助信息：
{% highlight string %}
$ git help <verb>
$ git <verb> --help
$ man git-<verb>
{% endhighlight %}
例如，你想要查看有关```git config```如何使用，那么可以使用以下命令：
<pre>
$ git help config
</pre>



## 2. git help命令
```git help```命令显示有关Git的帮助信息。基本语法格式如下：
<pre>
git help [-a|--all] [-g|--guide]
                  [-i|--info|-m|--man|-w|--web] [COMMAND|GUIDE]
</pre>
在不指定任何选项和```COMMAND```或```GUIDE```情况下，git命令的概要和最常用的Git命令列表打印在标准输出上； 如果给出```--all```或```-a```选项，则所有可用的命令都将打印在标准输出上； 如果给出了```--guide```或者```-g```选项，那么在标准输出中也会列出有用的Guide指南。

要显示git手册页，请使用命令： ```git help git```。

## 3. git init命令
```git init```命令创建一个空的Git仓库或重新初始化一个现有仓库。本命令的基本语法格式如下：
{% highlight string %}
git init [-q | --quiet] [--bare] [--template=<template_directory>]
         [--separate-git-dir <git dir>]
         [--shared[=<permissions>]] [directory]
{% endhighlight %}

1) **描述**

该命令创建一个空的Git仓库，基本上是创建一个具有```objects```、```refs/heads```、```refs/tags```和模板文件的```.git```目录。还创建了引用主分支```HEAD```的一个文件（名称为```HEAD```)。

如果通过```$GIT_OBJECT_DIRECTORY```环境变量指定了对象存储目录，那么将在该目录下面创建```sha1```目录； 否则，将使用默认的```$GIT_DIR/objects```目录。

在现有存储库中运行```git init```命令是安全的，它将不会覆盖已经存在的东西。重新运行```git init```的主要原因是拾取新添加的模板（或者如果给出了```--separate-git-dir```，则将存储库移动到另一个地方）

2) **示例**

为现有的代码库启动一个新的Git仓库：
<pre>
$ cd /path/to/my/codebase
$ git init                            # 在对应目录创建一个.git目录
$ git add .                           # 将所有现有文件添加索引
$ git commit -m "a commit message"    # 将原始状态记录为历史的第一个提交
</pre>

## 4. git add命令
```git add```命令用于将文件添加到索引(将修改添加到暂存区)。其基本用法如下：
{% highlight string %}
git add [-n] [-v] [--force | -f] [--interactive | -i] [--patch | -p]
        [--edit | -e] [--[no-]all | --[no-]ignore-removal | [--update | -u]]
        [--intent-to-add | -N] [--refresh] [--ignore-errors] [--ignore-missing]
        [--] [<pathspec>...]
{% endhighlight %}

1) **描述**

此命令将要提交的文件信息添加到索引库中（将修改添加到暂存区），以准备好下一次提交分段的内容。它通常将现有路径的当前内容作为一个整体添加，但是通过一些选项，其也可以被用于添加工作树中的某些被修改的文件，或者移除工作树中那些不存在的目录。

```索引```保存了工作树内容的快照，并且将该快照作为下一个提交的内容。因此，在对工作树进行任何更改之后，并且在运行```git commit```命令之前，必须使用```git add```命令将任何新的或修改的文件添加到索引。

该命令可以在提交之前多次执行。它只在运行```git add```命令时添加指定文件的内容；如果希望随后的更改包含在下一个提交中，那么必须再次运行```git add```将新的内容添加到索引。

```git status```命令可用于获取哪些文件具有为下一次提交分段的更改的摘要。

默认情况下，```git add```不会添加忽略的文件。如果在命令行上显示指定了任何忽略的文件，```git add```命令都将失败，并显示一个忽略的文件列表。由Git执行的目录递归或文件名遍历所导致的忽略文件将被默认忽略。```git add```命令可用```-f(force)```选项添加被忽略的文件。

2) **示例**

以下是一些示例：

* 添加```ducumentation```目录及其子目录下所有```.txt```文件
<pre>
$ git add ducumentation/*.txt
</pre>

注意，在这个例子中，星号(```*```)是从shell引用的。这允许命令包含来自```ducumentation```目录和子目录的文件。

* 将所有```git-*.sh```脚本添加到索引
<pre>
$ git add git-*.sh
</pre>
因为这个例子让shell扩展星号(即明确列出文件），所以它不考虑子目录中的文件，如```subdir/git-foo.sh```这样的文件将不会被添加

3） **基本用法**

{% highlight string %}
$ git add <path>
{% endhighlight %}
通常是通过```git add <path>```的形式把```<path>```添加到索引库中，其中```<path>```可以是文件也可以是目录。Git不仅能判断出```<path>```中已修改的文件（不包括已删除），还能判断出新添加的文件，并把它们的信息添加到索引库中。
{% highlight string %}
$ git add .            # 将所有修改添加到暂存区
$ git add *            # Ant风格添加修改
$ git add *Controller  # 将以Controller结尾的文件的所有修改添加到暂存区
$ git add Hello*       # 将所有以Hello开头的文件的修改添加到暂存区 例如:HelloWorld.txt,Hello.java,HelloGit.txt ...
{% endhighlight %}

* *git add -u <path>*: 把```<path>```中所有跟踪文件中被修改过或已删除文件的信息添加到索引库。它不会处理那些不被跟踪的文件。省略```<path```表示```.```，即当前目录。

* *git add -A*: 表示将目录中所有跟踪文件中被修改过或已删除文件和所有未被跟踪的文件信息添加到索引库。省略```<path>```表示```.```，即当前目录

* *git add -i <path>*: 可以通过本命令查看所有被修改过或已删除但没有提交的文件，并通过其```revert```子命令可以查看```<path>```中所有未被跟踪的文件，同时进入一个子命令系统。

例如：
{% highlight string %}
$ git add -i
           staged     unstaged path
  1:        +0/-0      nothing branch/t.txt
  2:        +0/-0      nothing branch/t2.txt
  3:    unchanged        +1/-0 readme.txt

*** Commands ***
  1: [s]tatus     2: [u]pdate     3: [r]evert     4: [a]dd untracked
  5: [p]atch      6: [d]iff       7: [q]uit       8: [h]elp
{% endhighlight %}

这里的```t.txt```和```t2.txt```表示已经被执行了```git add```，待提交，即已经添加到索引库中。```readme.txt```表示已经处于tracked下，它被修改了，但是还没有执行```git add```，即还没有添加到索引库中。

## 5. git clone命令
```git clone```命令将存储库克隆到新目录中。其基本语法如下：
{% highlight string %}
git clone [--template=<template_directory>]
          [-l] [-s] [--no-hardlinks] [-q] [-n] [--bare] [--mirror]
          [-o <name>] [-b <name>] [-u <upload-pack>] [--reference <repository>]
          [--separate-git-dir <git dir>]
          [--depth <depth>] [--[no-]single-branch]
          [--recursive | --recurse-submodules] [--] <repository>
          [<directory>]
{% endhighlight %}

1) **描述**

将存储库克隆到新创建的目录中，为克隆的存储库中每个分支创建远程跟踪分支（可通过```git branch -v```查看），并把克隆检出的存储库作为当前活动分支的初始分支。

在克隆之后，不带参数的```git fetch```将会更新所有远程跟踪分支(remote-tracking branch)， 而不带参数的```git pull```会将远程的master分支合并到当前的master分支。

默认配置下是通过在 refs/remotes/origin目录下创建对远程分支头的引用，以及初始化remote.origin.url和remote.origin.fetch。

执行远程操作的第一步通常是从远程主机克隆一个版本库，这时就要用到```git clone```命令：
{% highlight string %}
$ git clone <版本库地址>
{% endhighlight %}
例如，克隆JQuery的版本库：
<pre>
$ git clone http://github.com/jquery/jquery.git
</pre>
该命令会在本地主机生成一个目录，与远程主机的版本库同名。如果要指定不同的目录名，可以将目录名作为```git clone```命令的第二个参数：
{% highlight string %}
$ git clone <版本库地址> <本地目录名>
{% endhighlight %}

```git clone```支持多种协议，除了```HTTP(s)```协议置之外，还支持Git、SSH、本地文件协议等。

2） **常用选项介绍**

* ```--branch <name>, -b <name>```: 用于克隆指定的分支，而不是远程仓库HEAD所指示的分支

* ```--depth <depth>```: 创建一个只包含特定版本历史的浅克隆(shallow clone)。 注意，一个```shallow```仓库会有一系列的限制（你不能从该仓库进行clone或fetch，页不能够堆该仓库执行push操作），但是假如你只对一个拥有很长历史的大型项目的最新情况感兴趣的话，这很适合。

* ```--recursive, --recursive-submodules```: 在克隆被创建之后，使用默认的设置初始化其中的所有```submodules```，这等价于在克隆完成之后马上运行如下命令：
<pre>
$ git submodule update --init --recursive
</pre>

3) **示例**

以下是所支持协议的一些示例：
{% highlight string %}
$ git clone http[s]://example.com/path/to/repo.git
$ git clone http://git.oschina.net/yiibai/sample.git
$ git clone ssh://example.com/path/to/repo.git
$ git clone git://example.com/path/to/repo.git
$ git clone /opt/git/project.git 
$ git clone file:///opt/git/project.git
$ git clone ftp[s]://example.com/path/to/repo.git
$ git clone rsync://example.com/path/to/repo.git
{% endhighlight %}

## 5. git status命令
```git status```命令用于显示工作目录和暂存区的状态。使用此命令能看到哪些文件被修改但未被暂存、哪些文件被修改且已暂存、哪些文件没有被跟踪。```git status```不显示已经```commit```到项目历史中去的信息。要查看项目历史的信息，请使用```git log```命令。本命令的基本语法格式如下：
{% highlight string %}
git status [<options>...] [--] [<pathspec>...]
{% endhighlight %}


1) **描述**

显示索引文件和当前HEAD提交之间的差异，在工作树和索引文件之间有差异的路径以及工作树中没有被Git跟踪的路径。```git status```相对来说是一个简单的命令，它简单的展示状态信息。输出内容可以分为3类/组，如下所示：
{% highlight string %}
# On branch master
# Changes to be committed:  (已经在stage区, 等待添加到HEAD中的文件)
# (use "git reset HEAD <file>..." to unstage)
#
#modified: hello.py
#
# Changes not staged for commit: (有修改, 但是没有被添加到stage区的文件)
# (use "git add <file>..." to update what will be committed)
# (use "git checkout -- <file>..." to discard changes in working directory)
#
#modified: main.py
#
# Untracked files:(没有tracked过的文件, 即从没有add过的文件)
# (use "git add <file>..." to include in what will be committed)
#
#hello.pyc
{% endhighlight %}

2) **忽略文件**

没有```tracked```的文件分为两类，一是已经被放在工作目录下但还没有执行```git add```的文件，另一类是编译了的程序文件（如```.pyc```、```.obj```、```.exe```等）。当这些不想add的文件一多起来，```git status```的输出简直没法看了，一大堆的状态信息怎么看？

基于这个原因，Git让我们能在一个特殊文件```.gitignore```中把要忽略的文件放在其中，每一个想忽略的文件应该独占一行，```*```这个符号作为通配符使用。例如，在项目根目录下的```.gitignore```文件中加入下面内容能阻止```.pyc```和```.tmp```文件出现在```git status```中：
{% highlight string %}
*.pyc
*.tmp
{% endhighlight %}


## 6. git diff命令

```git diff```命令用于显示提交和工作树之间的更改。此命令比较的是工作目录中当前文件和暂存区域快照之间的差异，也就是修改之后还没暂存起来的变化内容。基本语法格式如下：
{% highlight string %}
git diff [options] [<commit>] [--] [<path>...]
git diff [options] --cached [<commit>] [--] [<path>...]
git diff [options] <commit> <commit> [--] [<path>...]
git diff [options] <blob> <blob>
git diff [options] [--no-index] [--] <path> <path>
{% endhighlight %}

1) **描述**

用于显示工作树与索引或版本库之间的更改，索引与版本库之间的更改，两个版本库之间的更改，磁盘上两个文件之间的更改。

2） **示例**

以下是一些示例：
{% highlight string %}
git diff <file>               # 比较当前文件和暂存区文件差异

git diff <id1> <id2>          # 比较两次提交之间的差异

git diff <branch1> <branch2>  # 比较两个分支之间的差异

git diff --stat               # 仅仅比较统计信息

$ git diff                    # 比较工作目录与暂存区之间的差异

$ git diff --cached           # 比较暂存区和版本库之间的差异，即你执行git commit所提交的内容(不带-a选项)

$ git diff --staged           # 比较暂存区和版本库之间的差异

$ gid diff HEAD               # 比较工作目录和上一次提交之间的差异，即你执行git commit -a所提交的内容太
{% endhighlight %}

## 7. git commit命令
```git commit```命令用于将更改记录提交到存储库。
{% highlight string %}
git commit [-a | --interactive | --patch] [-s] [-v] [-u<mode>] [--amend]
           [--dry-run] [(-c | -C | --fixup | --squash) <commit>]
           [-F <file> | -m <msg>] [--reset-author] [--allow-empty]
           [--allow-empty-message] [--no-verify] [-e] [--author=<author>]
           [--date=<date>] [--cleanup=<mode>] [--[no-]status]
           [-i | -o] [-S[<keyid>]] [--] [<file>…​]

{% endhighlight %}

1) **描述**

将索引中的当前内容连同用户的日志消息一起存储到一个新的提交中。所提交的内容可以通过多种方式指定：

* 在使用```git commit```命令之前，通过```git add```命令增量的添加修改到索引中（注意，即使修改的文件也必须要执行```git add``来添加）

* 通过使用```git rm```命令来从工作树和索引中移除的文件

* 通过在```git commit```命令后显式的指定要提交的文件，在这种情况下```git commit```将会忽略存储在索引中的暂存文件，而只提交当前所列出的文件（注意列出的文件必须是已经被跟踪）

* 通过在执行```git commit -a```时自动添加或删除的文件

* 通过使用```--interactive```或```--patch```选项与```git commit```命令一起确定除了索引中的内容之外哪些文件或hunks应该是提交的一部分，然后才能完成操作

如果您提交，然后立即发现错误，可以使用```git reset```命令恢复。

2) **示例**

以下是一些示例。提交已被```git add```进来的改动：
{% highlight string %}
$ git add . 
$ git add newfile.txt
$ git commit -m "the commit message" #
$ git commit -a                      # 会先把所有已经track的文件的改动'git add'进来，然后提交(有点像svn的一次提交,不用先暂存)。
                                     # 对于没有track的文件,还是需要执行'git add <file>'命令。
$ git commit --amend                 # 增补提交，会使用与当前提交节点相同的父节点进行一次新的提交，旧的提交将会被取消。
{% endhighlight %}
录制自己的工作时，工作树中修改后的文件内容将临时存储到使用```git add```命名为```索引```的暂存区域。一个文件只能在索引中恢复，而不是在工作树中，使用```git reset HEAD <file>```可以恢复对文件的修改。




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

