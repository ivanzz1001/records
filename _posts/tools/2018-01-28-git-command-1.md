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

* ```git add -u <path>```: 把```<path>```中所有跟踪文件中被修改过或已删除文件的信息添加到索引库。它不会处理那些不被跟踪的文件。省略```<path```表示```.```，即当前目录。

* ```git add -A```: 表示将目录中所有跟踪文件中被修改过或已删除文件和所有未被跟踪的文件信息添加到索引库。省略```<path>```表示```.```，即当前目录

* ```git add -i <path>```: 可以通过本命令查看所有被修改过或已删除但没有提交的文件，并通过其```revert```子命令可以查看```<path>```中所有未被跟踪的文件，同时进入一个子命令系统。

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


## 8. git reset命令

```git reset```命令用于将当前```HEAD```复位到指定状态。一般用于撤销之前的一些操作（如： ```git add```、```git commit```等）。基本语法格式如下：
{% highlight string %}
git reset [-q] [<tree-ish>] [--] <paths>...                                 # form-1
git reset (--patch | -p) [<tree-sh>] [--] [<paths>...]                      # form-2
git reset [--soft | --mixed | --hard | --merge | --keep] [-q] [<commit>]    # form-3
{% endhighlight %}

### 8.1 描述

对于上面```form-1```与```form-2```两种形式，会从```<tree-ish>```拷贝对应的条目到暂存区；而对于```form-3```这种形式，会将当前分支的```HEAD```设置为某一个提交ID，并通过相应的选项修改对应的暂存区和工作树。
<pre>
注： 对于tree-ish, 既可以是一个提交ID，也可以是一个树ID，当然也可以是HEAD。

更多关于tree-ish信息，请参看http://gitbook.liuhui998.com/4_6.html
</pre>

* form-1: 这种形式会将```<path>```指定的**暂存区条目**复位到```<tree-ish>```指定的状态。其并不会影响到工作树，也不会对当前分支造成影响。```git reset <paths>```具有与```git add <paths>```相反的含义。

在运行```git reset <paths>```命令更新索引条目(index entry)之后，你可以使用```git checkout```命令从索引中将相应的内容检出到工作树。另外，通过使用```git checkout <commit-id>```命令，你可以从提交中将指定路径的文件检出到索引区和工作树。

* form-2: 这里不做介绍

* form-3: 这种形式分支的head复位到某个commit，并根据相应的mode可能会更新暂存区（将其复位到某个```提交树```)和工作树。假如```mode```省略的话，则默认为```--mixed```。```mode```的可选值可以为如下几种：
{% highlight string %}
--soft: 根本不会涉及到暂存区和工作树，仅仅只会将head复位到某个commit。这会使得你当前工作目录中的被修改的文件可以
        进行提交。（注意，假如我们进行提交，默认会创建一个匿名分支。所以，通常我们应该先创建一个新的分支，然后
        在新的分支中进行提交）。

--mixed: 重置索引（即暂存区），但是不会重置工作树（通常被修改的文件将会保留，但是没有被标记为准备提交状态），并且
         报告有哪些没有被修改。这是默认的选项值。

--hard: 重置索引和工作树，目录树中自从<commit>之后的所有更改都将会丢失，并将HEAD指向<commit>

--merge: 重置暂存区，并更新工作树中<commit>与HEAD之间不同的文件，但是会保留工作树与暂存区之间的不同的文件内容（例如：
         被修改的文件尚未添加到暂存区）
{% endhighlight %}

### 8.2 应用场景
下面列出一些```git reset```的典型应用场景：

1) **回滚添加操作**

{% highlight string %}
$ edit    file1.c file2.c                   # (a) 
$ git add file1.c file1.c                   # (a.1) 添加两个文件到暂存
$ mailx                                     # (b) 
$ git reset                                 # (c) 
$ git pull git://info.example.com/ nitfol   # (d)
{% endhighlight %}

a) 编辑文件```file1.c```、```file2.c```，做了些更改，并把更改添加到暂存区。

b) 查看邮件，发现某人要您执行```git pull```，有些文件需要合并下来

c) 然而，您已经把暂存区搞乱了，因为暂存区同```HEAD commit```不匹配了，但是即将```git pull```下来的东西不会影响已修改的```file1.c```、```file2.c```，因此可以```revert```这两个文件的改变。在revert后，那些改变应该依旧在工作目录中，因此执行```git reset```

d) 然后执行了```git pull```之后，自动合并，```file1.c```和```file2.c```这些改变依然在工作目录中。

2） **回滚最近一次提交**

{% highlight string %}
$ git commit -a -m "这是提交的备注信息"
$ git reset --soft HEAD^                #(a) 
$ edit code                             #(b) 编辑代码操作
$ git commit -a -c ORIG_HEAD            #(c)
{% endhighlight %}

a) 当提交了之后，又发现代码没有提交完整，或者想重新编辑一下提交的信息，可执行```git reset --soft HEAD^```，让工作目录还跟reset之前一样，不作任何改变。```HEAD^```表示HEAD之前最近一次提交。

b) 对工作目录下的文件做修改，比如：修改文件中的代码等

c) 然后使用```reset```之前那次提交的注释、作者、日期等信息重新提交。注意：当执行```git reset```命令时，Git会把老的HEAD拷贝到文件```.git/ORIG_HEAD```中，在命令中可以使用```ORIG_HEAD```引用这个提交。```git commit```命令中```-a```参数的意思是告诉git，自动把所有修改的和删除的文件都放进暂存区，未被git跟踪的新建文件不受影响。```commit```命令中```-c <commit>```或者```-C <commit>```意思是拿已经提交的对象中的信息（作者、提交者、注释、时间戳等）提交，那么这条```git commit```命令的意思就非常明确清晰了，把所有更改的文件加入暂存区，并使用上次的提交信息重新提交。

3） **回滚最近几次提交，并把这几次提交放到指定分支中**

回滚最近几次提交，并把这几次提交放到叫做```topic/wip```的分支上去。
{% highlight string %}
$ git branch topic/wip       (a) 
$ git reset --hard HEAD~3    (b) 
$ git checkout topic/wip     (c)
{% endhighlight %}
a) 假设已经提交了一些代码，但是此时发现这些提交还不够成熟，不能进入master分支，希望在新的branch上暂存这些改动。因此执行了```git branch```命令在当前HEAD上建立了新的叫做```topic/wip```的分支。

b) 然后回滚```master```分支上的最近三次提交。```HEAD~3```指向当前```HEAD-3```个提交，命令
<pre>
$ git reset --hard HEAD~3
</pre>
表示删除最近的三个提交（删除```HEAD```、```HEAD^```、```HEAD~2```)，将HEAD指向```HEAD~3```

4) **永久删除最后几个提交**
{% highlight string %}
$ git commit        ##执行一些提交
$ git reset --hard HEAD~3   (a)
{% endhighlight %}

a) 最后三个提交（即```HEAD```、```HEAD^```和```HEAD~2```)提交有问题，想永久删除这三个提交。

5) **回滚merge和pull操作**
{% highlight string %}
$ git pull                              (a) 
Auto-merging nitfol 
CONFLICT (content): Merge conflict in nitfol 
Automatic merge failed; fix conflicts and then commit the result. 

$ git reset --hard                      (b) 
$ git pull . topic/branch               (c) 
Updating from 41223... to 13134... 
Fast-forward 
$ git reset --hard ORIG_HEAD            (d)
{% endhighlight %}
a) 从```origin```拉取下来一些更新，但是产生了很多冲突，但您暂时没有那么多时间去解决这些冲突，因此决定稍后有空的时候再重新执行```git pull```操作。

b) 由于```git pull```操作产生了冲突，因此所有拉取下来的改变尚未提交，仍然在暂存区中。这种情况下```git reset --hard```与```git reset --hard HEAD```意思相同，即都是清除索引和工作区中被搞乱的东西。

c) 将```topic/branch```分支合并到当前的分支，这次没有产生冲突，并且合并后的更改自动提交。

d) 但此时你又发现将```topic/branch```合并过来为时尚早，因此决定退滚合并，执行```git reset --hard ORIG_HEAD```回滚刚才的```pull/merge```操作。说明： 前面讲过，执行```git reset```时，Git会把reset之前的```HEAD```放入```ORIG_HEAD```文件中，命令行中使用ORIG_HEAD引用这个提交。同样的，执行```git pull```和```git merge```操作时，git都会把执行操作前的HEAD放入```ORIG_HEAD```中，以用作回滚操作。

6) **在污染的工作区中回滚合并或者拉取**

{% highlight string %}
$ git pull                         (a) 
Auto-merging nitfol 
Merge made by recursive. 
nitfol                |   20 +++++---- 
... 
$ git reset --merge ORIG_HEAD      (b)

{% endhighlight %}
a) 即便你已经在本地更改了工作区中的一些东西，可安全的执行```git pull```操作，前提是要知道将要```git pull```下面的内容不会覆盖工作区中的内容。

b） ```git pull```完成后，发现这次拉取下来的修改不满意，想要回滚到```git pull```之前的状态，从前面的介绍知道，我们可以执行```git reset --hard ORIG_HEAD```，但是这个命令有个副作用就是清空工作区，即丢弃本地未使用```git add```的那些改变。为了避免丢弃工作区中的内容，可以使用```git reset --merge ORIG_HEAD```，注意其中的```--hard```换成了```--merge```，这样就可以避免在回滚时清除工作区。

7) **中断的工作流程处理**

在实际的开发过程中经常出现这样的情形： 你正在开发一个新的功能（工作在分支： ```feature```中），此时来了一个紧急的bug需要修复，但是目前在工作区中的内容还没有成型，还不足以提交，但是又必须切换到另外的分支去修改bug。请看下面的例子：
{% highlight string %}
$ git checkout feature ;# you were working in "feature" branch and 
$ work work work       ;# got interrupted 
$ git commit -a -m "snapshot WIP"                 (a) 
$ git checkout master 
$ fix fix fix 
$ git commit ;# commit with real log 
$ git checkout feature 
$ git reset --soft HEAD^ ;# go back to WIP state  (b) 
$ git reset                                       (c)
{% endhighlight %}
a) 这次属于临时提交，因此随便添加一个临时注释即可。

b) 这次```reset```删除了WIP commit，并且把工作区设置成提交WIP快照之前的状态。

c) 此时，在索引中依然遗留着```snapshot WIP```提交时所做的未提交变化，```git reset```将会清理索引成尚未提交```snapshot WIP```时的状态便于接下来继续工作。

8) **重置单独的一个文件**

假如你已经添加了一个文件进入索引，但是而后又不打算把这个文件提交，此时可以使用```git reset```把这个文件从索引中去除：
{% highlight string %}
$ git reset -- frotz.c                      (a) 
$ git commit -m "Commit files in index"     (b) 
$ git add frotz.c                           (c)
{% endhighlight %}
a) 把文件```frotz.c```从索引中去除

b) 把索引中的文件提交

c) 再次把```frotz.c```添加到索引

9) **保留工作区并丢弃一些之前的提交**

假设你正在编辑一些文件，并且已经提交，接着继续工作。但是现在你发现当前在工作区中的内容应该属于另一个分支，与之前的提交没有什么关系。此时，可以开启一个新的分支，并且保留着工作区中的内容：
{% highlight string %}
$ git tag start 
$ git checkout -b branch1 
$ edit 
$ git commit ...                            (a) 
$ edit 
$ git checkout -b branch2                   (b) 
$ git reset --keep start                    (c)
{% endhighlight %}

a) 这次是把在```branch1```中的改变提交了。

b) 此时发现，之前的提交不属于这个分支，此时新建了```branch2```分支，并切换到```branch2```上

c) 此时可以使用```git reset --keep```把在start之后的提交清除，但保持工作区不变。



## 9. git rm命令
```git rm```命令用于从工作区和索引中删除文件。基本语法格式如下：
{% highlight string %}
git rm [-f | --force] [-n] [-r] [--cached] [--ignore-unmatch] [--quiet] [--] <file>...
{% endhighlight %}

1) **描述**

从索引中删除文件，或者从索引与工作树中删除文件。```git rm```不会仅仅只移除工作目录中的文件，一定会同时移除索引中的文件。被移除的文件必须与分支的提示相同，并且在索引中不能对其内容进行更新，尽管可以通过```-f```选项来覆盖默认的行为。当指定了```--cache```选项时，暂存区的内容必须与分支的提示或磁盘上的文件相匹配，从而将文件从索引中删除。

使用```git rm```来删除文件，同时还会将这个删除操作记录下来；而使用```rm```来删除文件，仅仅是删除了物理文件，没有将其从git的记录中剔除。

直观的来讲，```git rm```删除过的文件，执行：
<pre>
git commit -m "commit message or mark"
</pre>
提交时，会自动将删除该文件的操作提交上去; 而对于用```rm```命令直接删除的文件，执行上面的提交命令时，则不会将删除该文件的操作提交上去。不过不要紧，即使你已经通过```rm```将某个文件删除掉了，你也可以通过执行```git rm```命令重新将该文件从git的记录中移除掉，这样的话，再执行上面的提交命令，也能够将这个删除操作提交上去。

如果之前不小心用```rm```命令删除了一大批文件呢？如此时用```git rm```逐个的再删除一次就显得相当繁琐了。可如下方式做提交：
<pre>
$ git commit -a -m "commit message or mark"
</pre>

2) **示例**

在Git中我们可以通过```git rm```命令把一个文件删除，并把它从git仓库管理系统中移除。但是注意最后要执行```git commit```才真正提交到git仓库。

* 示例1

删除```text1.txt```文件，并把它从git的仓库管理系统中移除：
<pre>
$ git rm test1.txt
</pre>

* 示例2

删除文件夹```mydir```，并把它从git的仓库管理系统中移除：
<pre>
$ git rm -r mydir
</pre>

* 示例3

{% highlight string %}
$ git rm Ducumentation/\*.txt
{% endhighlight %}
从```Ducumentation```目录及其子目录下的索引中删除所有```.txt```文件。

## 10. git mv命令
```git mv```命令用于移动或重命名文件、目录或符号链接。基本语法格式如下：
{% highlight string %}
git mv <options>... <args>...
{% endhighlight %}

1) **描述**

移动或重名文件、目录或符号链接：
{% highlight string %}
git mv [-v] [-f] [-n] [-k] <source> <destination>
git mv [-v] [-f] [-n] [-k] <source> ... <destination directory>
{% endhighlight %}

在第一种形式中，它将重命名```<source>```为```<destination>```，```<source>```必须存在，并且是文件、符号链接或目录。在第二种形式中，最后一个参数必须是现有的目录，给定的源```<source>```将被移动到这个目录中。

索引在成功完成后更新，但仍必须提交更改。

2） **示例**

把一个文件```text.txt```移动到```mydir```，可以执行以下操作：
<pre>
$ git mv text.txt mydir/
</pre>
运行上面的```git mv```其实就相当于运行了三条命令：
<pre>
$ mv text.txt mydir/
$ git rm text.txt
$ git add mydir
</pre>

## 11 git branch命令

```git branch```命令用于列出、创建或删除分支。其基本语法格式如下：
{% highlight string %}
git branch [--color[=<when>] | --no-color] [-r | -a]
           [--list] [-v [--abbrev=<length> | --no-abbrev]]
           [--column[=<options>] | --no-column]
           [(--merged | --no-merged | --contains) [<commit>]] [<pattern>...]
git branch [--set-upstream | --track | --no-track] [-l] [-f] <branchname> [<start-point>]
git branch (--set-upstream-to=<upstream> | -u <upstream>) [<branchname>]
git branch --unset-upstream [<branchname>]
git branch (-m | -M) [<oldbranch>] <newbranch>
git branch (-d | -D) [-r] <branchname>...
git branch --edit-description [<branchname>]
{% endhighlight %}

1) **描述**

如果给出了```--list```，或者没有非选项参数，则列出现有的分支；当前分支将以星号突出显示。选项```-r```导致远程跟踪分支被列出， 而选项```-a```显示本地和远程分支。如果给出了一个```<pattern>```，它将被用作一个shell通配符，将输出限制为匹配的分支。如果给出多个模式，如果匹配任何模式，则显示分支。请注意，提供```<pattern>```时，必须使用```--list```,

2） **使用示例**

* 查看当前有哪些分支
<pre>
$ git branch 
  master
* wchar_support
</pre>

上面的显示结果中，当前有两个分支：```master```和```wchar_support```。当前在```wchar_support```分支上，它前面有一个星号


* 新建一个分支

下面的命令将创建一个分支```dev2```:
<pre>
$ git branch dev2
</pre>

* 切换到指定的分支

下面的命令将切换到指定的分支：
<pre>
$ git checkout dev2
$ git branch
* dev2
  master
  wchar_support
</pre>

* 查看本地和远程分支
{% highlight string %}
$ git branch -a
* dev2
  master 
  wchar_support
  remotes/origin/HEAD -> origin/master
  remotes/orgin/master
  remotes/orgin/wchar_support
{% endhighlight %}


* 将更改添加到新分支上
{% highlight string %}
$ git status
On branch dev2
Untracked files:
  (use "git add <file>..." to include in what will be committed)

        newfile.txt

nothing added to commit but untracked files present (use "git add" to track)

Administrator@MY-PC /D/worksp/sample (dev2)

$ git add newfile.txt

Administrator@MY-PC /D/worksp/sample (dev2)

$ git commit newfile.txt -m "commit a new file: newfile.txt"
[dev2 c5f8a25] commit a new file: newfile.txt
 1 file changed, 2 insertions(+)
 create mode 100644 newfile.txt

Administrator@MY-PC /D/worksp/sample (dev2)

$ git push origin dev2
Username for 'http://git.oschina.net': 769728683@qq.com
Password for 'http://769728683@qq.com@git.oschina.net':
Counting objects: 12, done.
Delta compression using up to 4 threads.
Compressing objects: 100% (8/8), done.
Writing objects: 100% (11/11), 965 bytes | 0 bytes/s, done.
Total 11 (delta 3), reused 0 (delta 0)
To http://git.oschina.net/yiibai/sample.git
 * [new branch]      dev2 -> dev2
{% endhighlight %}


* 修改分支名字
{% highlight string %}
$ git branch
* dev2
  master
  wchar_support

Administrator@MY-PC /D/worksp/sample (dev2)
$ git branch -m dev2 version.2

Administrator@MY-PC /D/worksp/sample (version.2)
$ git branch -r
  origin/HEAD -> origin/master
  origin/dev2
  origin/master
  origin/wchar_support

Administrator@MY-PC /D/worksp/sample (version.2)
$ git branch
  master
* version.2
  wchar_support

{% endhighlight %}

* 删除远程分支

执行如下命令删除一个名称为```dev2```的远程分支：
{% highlight string %}
$ git branch
  master
* version.2
  wchar_support

Administrator@MY-PC /D/worksp/sample (version.2)
$ git push origin --delete dev2
Username for 'http://git.oschina.net': 769728683@qq.com
Password for 'http://769728683@qq.com@git.oschina.net':
To http://git.oschina.net/yiibai/sample.git
 - [deleted]         dev2
{% endhighlight %}


* 合并某个分支到当前分支

执行如下的命令将```version2```分支合并到当前分支```master```:
{% highlight string %}
$ git branch
  master
* version.2
  wchar_support

Administrator@MY-PC /D/worksp/sample (version.2)
$ git checkout master
Switched to branch 'master'
Your branch is up-to-date with 'origin/master'.

Administrator@MY-PC /D/worksp/sample (master)
$ git status
On branch master
Your branch is up-to-date with 'origin/master'.

nothing to commit, working directory clean

Administrator@MY-PC /D/worksp/sample (master)
$ git merge version.2
Updating e7d1734..c5f8a25
Fast-forward
 mydir/text.txt | 0
 newfile.txt    | 2 ++
 src/string.py  | 5 ++++-
 3 files changed, 6 insertions(+), 1 deletion(-)
 create mode 100644 mydir/text.txt
 create mode 100644 newfile.txt
{% endhighlight %}

* 查看分支的追踪关系
{% highlight string %}
$ git branch -vv
* master        032de42 [origin/master: ahead 4] Merge branch 'temp'
  topic/wip     d4d8f8a Resolved conflict
  wchar_support 954bdfd add new function: count_len(obj)
{% endhighlight %}
从上面我们看到，只有本地分支```master```追踪远程的```origin/master```分支。

## 12. git checkout命令
```git checkout```命令用于切换分支或者恢复工作树文件。```git checkout```是git最常用的命令之一，同时也是一个很危险的命令，因为这条命令会重写工作区。基本语法格式如下：
{% highlight string %}
git checkout [-q] [-f] [-m] [<branch>]
git checkout [-q] [-f] [-m] [--detach] [<commit>]
git checkout [-q] [-f] [-m] [[-b|-B|--orphan] <new_branch>] [<start_point>]
git checkout [-f|--ours|--theirs|-m|--conflict=<style>] [<tree-ish>] [--] <paths>...
git checkout [-p|--patch] [<tree-ish>] [--] [<paths>...]
{% endhighlight %}


1) **描述**

更新工作树中的文件以匹配索引或指定树中的版本。如果没有给出路径，```git checkout```还会更新```HEAD```，将指定的分支设置为当前分支。

2) **示例**

* 示例1

以下顺序检查主分支，将```Makefile```还原为两个修订版本，错误地删除```hello.c```，并从索引中取回：
{% highlight string %}
$ git checkout master               #(a)
$ git checkout master~2 Makefile    #(b)
$ rm -f hello.c
$ git checkout hello.c              #(c)
{% endhighlight %}

a) 切换到```master```分支

b) 从另一个提交中取出文件

c) 从索引中恢复```hello.c```

如果想要检出索引中的所有```C```源文件，可以使用如下命令：
<pre>
$ git checkout -- '*.c'
</pre>
注意： ```*.c```是使用引号的。文件```hello.c```也将被检出，即使它不再工作树中，因为文件```globbing```用于匹配索引中的条目（而不是shell的工作树中）

如果有一个分支的名称也刚好为```hello.c```，这一步将被混淆为切换到分支的指令。此时，我们应该写为：
<pre>
$ git checkout -- hello.c
</pre>

* 示例2

在错误的分支工作后，想切换到正确的分支，则使用：
<pre>
$ git checkout mytopic
</pre>
但是，你的```错误```分支和正确的```mytopic```分支可能会在本地修改的文件中有所不同，在这种情况下，上述检出将会失败：
<pre>
$ git checkout mytopic
error: You have local changes to 'frotz'; not switching branches.
</pre>
可以将```-m```标志赋给命令，这样将尝试三路（当前分支、工作树内容、切换到的目标分支）合并：
<pre>
$ git checkout -m mytopic
Auto-merging frotz
</pre>
在这种三路合并之后，本地的修改没有在索引文件中注册，所以```git diff```会显示从新的分支的提示之后所做的修改。

* 示例3

当使用```-m```选项切换分支发生合并冲突时，会看到如下所示：
{% highlight string %}
$ git checkout -m mytopic
Auto-merging frotz
ERROR: Merge conflict in frotz
fatal: merge program failed
{% endhighlight %}
此时，```git diff```会显示上一个示例中干净合并的更改以及冲突文件中的更改。编辑并解决冲突，并用常规方式```git add```来标识它：
<pre>
$ edit frotz          # 编辑 frotz 文件中内容，然后重新添加
$ git add frotz
</pre>


* 其他示例

```git checkout```命令的主要功能就是签出一个分支的特定版本。默认是签出分支的```HEAD```版本。如下是一些用法示例：
{% highlight string %}
$ git checkout master                # 取出master版本的head。
$ git checkout tag_name              # 在当前分支上 取出 tag_name 的版本
$ git checkout master file_name      # 放弃当前对文件file_name的修改
$ git checkout  commit_id file_name  # 取文件file_name的 在commit_id是的版本。commit_id为 git commit 时的sha值。


# 从远程dev/1.5.4分支取得到本地分支/dev/1.5.4
$ git checkout -b dev/1.5.4 origin/dev/1.5.4  

#这条命令把hello.rb从HEAD中签出.
$ git checkout -- hello.rb   

#这条命令把当前目录所有修改的文件 从HEAD中签出并且把它恢复成未修改时的样子.
$ git checkout .

{% endhighlight %}
注意：在使用```git checkout```时，如果其对应的文件被修改过，那么该修改会被覆盖掉。


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

