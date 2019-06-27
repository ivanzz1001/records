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


## 3. 具体解决操作

1） **查看当前分支状况**
{% highlight string %}
$ git status
HEAD detached at e8c14b2
nothing to commit, working directory clean
$ git branch -v
* (HEAD detached at e8c14b2) e8c14b2 add module.py
  master                     d4d8f8a [ahead 2] Resolved conflict
  topic/wip                  d4d8f8a Resolved conflict
  wchar_support              954bdfd add new function: count_len(obj)
{% endhighlight %}

2） **新建一个temp分支，把当前提交的代码放到整个分支**
{% highlight string %}
$ git branch temp

$ git checkout temp
Switched to branch 'temp'

$ git status
On branch temp
nothing to commit, working directory clean
{% endhighlight %}

3) **checkout要回到的那个分支**

这里我们要回到```master```分支：
<pre>
$ git checkout master
Switched to branch 'master'
Your branch is ahead of 'origin/master' by 2 commits.
  (use "git push" to publish your local commits)
</pre>

4) **然后merge刚才创建的临时分支**

通过```merge```操作把临时分支上的提交合并到```master```分支上来：
{% highlight string %}
$ git merge temp
Merge made by the 'recursive' strategy.
 detached_head.txt | 1 +
 1 file changed, 1 insertion(+)
 create mode 100644 detached_head.txt

{% endhighlight %}


5) **查看合并后的结果，有冲突就解决**
{% highlight string %}
$ git status
On branch master
Your branch is ahead of 'origin/master' by 4 commits.
  (use "git push" to publish your local commits)
nothing to commit, working directory clean
{% endhighlight %}

6) **合并OK，就提交到远程**
{% highlight string %}
$ git push origin master
Username for 'https://github.com': ivanzz1001
Password for 'https://ivanzz1001@github.com':
To https://github.com/ivanzz1001/sample.git
 ! [rejected]        master -> master (fetch first)
error: failed to push some refs to 'https://github.com/ivanzz1001/sample.git'
hint: Updates were rejected because the remote contains work that you do
hint: not have locally. This is usually caused by another repository pushing
hint: to the same ref. You may want to first integrate the remote changes
hint: (e.g., 'git pull ...') before pushing again.
hint: See the 'Note about fast-forwards' in 'git push --help' for details.
{% endhighlight %}

7) **删除刚才创建的临时分支**
{% highlight string %}
$ git branch -d temp
Deleted branch temp (was 5266593).
{% endhighlight %}




<br />
<br />

**[参看]**

1. [Git HEAD detached](https://blog.csdn.net/u011240877/article/details/76273335)




<br />
<br />
<br />

