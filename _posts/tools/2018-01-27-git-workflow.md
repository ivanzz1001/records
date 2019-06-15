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

现在，Git添加操作(```git add```命令）将文件添加到暂存区域：
{% highlight string %}
$ git add main.py
{% endhighlight %}
在执行上述命令后，已将文件添加到存储区域，```git status```命令将显示临时区域中存在的文件：
{% highlight string %}
$ git status -s
A  main.py
{% endhighlight %}
上面文件中，看到文件名称前面多了一个大写字母```A```，表示该文件已经添加到Git临时区域中了。

要提交更改，可以使用```git commit```命令，后跟```-m```选项。如果忽略了```-m```选项，Git将打开一个文本编辑器，我们可以在其中编写多行的提交备注信息。（注意：在执行```git commit```命令前，一定要先执行```git add```命令）
{% highlight string %}
Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git commit -m "this is main.py file commit mark use -m option"
[master 4862de1] this is main.py file commit mark use -m option
 1 file changed, 4 insertions(+)
 create mode 100644 main.py
{% endhighlight %}
在提交后，我们可以使用```git log```命令查看日志详细信息。它将使用提交ID、提交作者、提交日期和提交的SHA-1哈希显示所有提交的信息：
{% highlight string %}
$ git log
commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit
{% endhighlight %}
前面我们说过，要提交修改过的文件，首先使用*git add <file>*，然后再执行*git commit -m <message>*，但是这两步也可以一步完成，如下所示：
{% highlight string %}
$ git commit -a -m "this is main.py file commit mark use -m option"
{% endhighlight %}


## 4. Git查看更改
本章我们将演示如何查看Git存储库的文件和提交记录，并对存储库中的文件作修改和提交。比如，我们查看提交详细信息后，需要修改代码，或者添加更多的代码，或者对比提交结果。

西面使用```git log```命令查看详细日志信息：
{% highlight string %}
$ git log
commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit
{% endhighlight %}
使用```git show```命令查看某一次提交的详细信息。```git show```命令采用SHA-1提交ID作为参数：
{% highlight string %}
$ git show 4862de19cfce5e99292747e4a23027cc78d22f7c
commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

diff --git a/main.py b/main.py
new file mode 100644
index 0000000..478b6cd
--- /dev/null
+++ b/main.py
@@ -0,0 +1,4 @@
+#!/usr/bin/python3
+#coding=utf-8
+
+print ("Life is short, you need Python !")
\ No newline at end of file
{% endhighlight %}
上面显示的结果中，可以看到符号```+```，表示添加的内容。如果有```-```，则表示删除的内容，现在我们打开```main.py```，修改后内容如下：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")

a = 10
b = 20
{% endhighlight %}
然后使用命令```git status```查看当前工作区状态：
{% highlight string %}
$ git status
On branch master
Your branch is ahead of 'origin/master' by 1 commit.
  (use "git push" to publish your local commits)
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

        modified:   main.py

no changes added to commit (use "git add" and/or "git commit -a")
{% endhighlight %}
测试代码后，通过运行```git diff```命令来回顾它的更改：
{% highlight string %}
$ git diff
diff --git a/main.py b/main.py
index 478b6cd..5b909ce 100644
--- a/main.py
+++ b/main.py
@@ -1,4 +1,7 @@
 #!/usr/bin/python3
 #coding=utf-8

-print ("Life is short, you need Python !")
\ No newline at end of file
+print ("Life is short, you need Python !")
+
+a = 10
+b = 20
\ No newline at end of file
{% endhighlight %}
可以看到符号```+```表示添加的内容，如果有```-```，则表示删除的内容。

现在使用以下命令将文件```main.py```添加到Git暂存区，然后提交代码：
{% highlight string %}
$ git add main.py

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git commit -m "define two var a & b"
[master 50db2e9] define two var a & b
 1 file changed, 4 insertions(+), 1 deletion(-)
{% endhighlight %}

这样，最新修改的代码就提交完成。

## 5. 提交更改
在本章，我们将演示如何查看Git存储库的文件和提交文件记录，并对存储库中的文件作修改和提交。

在上一步中，我们已经修改了```main.py```文件中的代码，在代码中定义了两个变量并提交代码，但是要再次添加和修改```main.py```文件中的代码，实现新功能：求两个变量相加值。修改提交的操作更改包含提交消息的最后一个提交；它创建一个新的提交ID。

在修改操作之前，检查提交日志，如下命令所示：
{% highlight string %}
$ git log
commit 50db2e9c18fe8e9b96391c1ba28df4fefac45400
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 11:05:18 2019 +0800

    define two var a & b

commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit
{% endhighlight %}
下面我们打开文件，main.py中加入以下两行：
<pre>
c = a + b
print("The value of c is ", c)
</pre>
更正操作提交新的更改，并查看提交日志。首先查看状态，如下命令：
{% highlight string %}
$ git status
On branch master
Your branch is ahead of 'origin/master' by 2 commits.
  (use "git push" to publish your local commits)
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

        modified:   main.py

no changes added to commit (use "git add" and/or "git commit -a")
{% endhighlight %}

添加文件并查看状态，如下命令：
{% highlight string %}
$ git add main.py

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git status
On branch master
Your branch is ahead of 'origin/master' by 2 commits.
  (use "git push" to publish your local commits)
Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

        modified:   main.py

{% endhighlight %}
提交更改的文件，如下所示：
{% highlight string %}
$ git commit --amend -m "add the sum of a & b"
[master 9589658] add the sum of a & b
 Date: Sat Jun 15 11:05:18 2019 +0800
 1 file changed, 7 insertions(+), 1 deletion(-)
{% endhighlight %}

现在，使用```git log```命令将显示新的提交消息与新的提交ID（*958965822323fa47b61c34b5380c5268569aadfc*)，最近一次提交的放前面：
{% highlight string %}
$ git log
commit 958965822323fa47b61c34b5380c5268569aadfc
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 11:05:18 2019 +0800

    add the sum of a & b

commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit

{% endhighlight %}

## 6. Git推送（push）操作

在上面的章节中，都要在本地编写文件代码和提交，维护管制自己的文件版本，然而这种```自娱自乐```的方式，意义不是很大。在这里将介绍如何与其他开发人员协同开发工作：每个开发人员都可以提交自己贡献的代码，并让其他人看到和修改。

要协同多人一起工作，可通过修改操作将代码最后一个确定版本提交，然后再推送变更。推送(Push)操作将数据永久存储到Git仓库。成功的推送操作后，其他开发人员可以看到新提交的变化。

执行```git log```命令查看提交的详细信息。最后一次提交的代码的ID是(*958965822323fa47b61c34b5380c5268569aadfc*):
{% highlight string %}
$ git log
commit 958965822323fa47b61c34b5380c5268569aadfc
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 11:05:18 2019 +0800

    add the sum of a & b

commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit
{% endhighlight %}
在推送操作之前，如果想要检查文件代码变化，可使用```git show```命令指定提交ID来查看具体的变化：
{% highlight string %}
$ git show  958965822323fa47b61c34b5380c5268569aadfc
commit 958965822323fa47b61c34b5380c5268569aadfc
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 11:05:18 2019 +0800

    add the sum of a & b

diff --git a/main.py b/main.py
index 478b6cd..1356cab 100644
--- a/main.py
+++ b/main.py
@@ -1,4 +1,10 @@
 #!/usr/bin/python3
 #coding=utf-8

-print ("Life is short, you need Python !")
\ No newline at end of file
+print ("Life is short, you need Python !")
+
+a = 10
+b = 20
+
+c = a + b
+print("The value of c is ", c)
\ No newline at end of file
{% endhighlight %}

如果对上面的提交修改没有疑义，则我们就可以将文件代码推送到远程存储库中，从而让其他开发人员可查看和修改这些代码，现在就来看看怎么提交这些写好的代码，使用以下命令：
{% highlight string %}
$ git push origin master
Username for 'https://github.com': ivanzz1001
Counting objects: 6, done.
Delta compression using up to 2 threads.
Compressing objects: 100% (6/6), done.
Writing objects: 100% (6/6), 665 bytes | 0 bytes/s, done.
Total 6 (delta 1), reused 0 (delta 0)
remote: Resolving deltas: 100% (1/1), done.
To https://github.com/ivanzz1001/sample.git
   e038ad8..9589658  master -> master
{% endhighlight %}

如上所示，现在代码已经成功地提交到了远程存储库中了。我们可以登录GitHub来查看校验。

## 7.Git更新操作

假设另一个用户从GitHub上克隆下来了```sample```仓库，执行：
<pre>
$ git clone https://github.com/ivanzz1001/sample.git
$ cd sample
$ git log
</pre>
观察日志后，可以看到添加了文件代码来实现两个变量```a```和```b```之和，现在我们打开```main.py```，并修改优化代码。通过定义一个函数来实现两个变量之和。修改后，代码如下所示：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")

a = 10
b = 20

def sum(a, b)
   return (a+b)
   
c = sum(a, b)
print("The value of c is ", c)
{% endhighlight %}
使用```git diff```命令查看更改，如下所示：
{% highlight string %}
$ git diff
diff --git a/main.py b/main.py
index 1356cab..9467363 100644
--- a/main.py
+++ b/main.py
@@ -6,5 +6,8 @@ print ("Life is short, you need Python !")
 a = 10
 b = 20

-c = a + b
+def sum(a, b)
+   return (a+b)
+
+c = sum(a, b)
 print("The value of c is ", c)
\ No newline at end of file
{% endhighlight %}

经过测试，代码没有问题，提交了上面的更改。
{% highlight string %}
$ git status
On branch master
Your branch is up-to-date with 'origin/master'.
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git checkout -- <file>..." to discard changes in working directory)

        modified:   main.py

no changes added to commit (use "git add" and/or "git commit -a")

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git add main.py

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git commit -m "add a new function sum(a,b)"
[master 30d3ff5] add a new function sum(a,b)
 1 file changed, 4 insertions(+), 1 deletion(-)
{% endhighlight %}
再次查看提交记录信息：
{% highlight string %}
$ git log
commit 30d3ff5dc7eed0e81897f537bb531368654b2940
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 23:34:47 2019 +0800

    add a new function sum(a,b)

commit 958965822323fa47b61c34b5380c5268569aadfc
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 11:05:18 2019 +0800

    add the sum of a & b

commit 4862de19cfce5e99292747e4a23027cc78d22f7c
Author: ivanzz1001 <782740456@qq.com>
Date:   Sat Jun 15 10:20:09 2019 +0800

    this is main.py file commit mark use -m option

commit e038ad893f1e07aa58e834923607c777c8e43230
Author: ivanzz1001 <782740456@qq.com>
Date:   Fri Jun 14 19:23:57 2019 +0800

    Initial commit
{% endhighlight %}

经过上面添加和提交修改之后，其他开发人员并不能看到代码中定义的```sum(a,b)```函数，还需要将这里提交到本地的代码推送到远程存储库，使用以下命令：
{% highlight string %}
$ git remote -v
origin  https://github.com/ivanzz1001/sample.git (fetch)
origin  https://github.com/ivanzz1001/sample.git (push)

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git push origin master
Username for 'https://github.com': ivanzz1001
Counting objects: 3, done.
Delta compression using up to 2 threads.
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 419 bytes | 0 bytes/s, done.
Total 3 (delta 0), reused 0 (delta 0)
To https://github.com/ivanzz1001/sample.git
   9589658..30d3ff5  master -> master
{% endhighlight %}

现在代码已经成功提交到远程库中了，只要其他开发人员使用```git clone```或者```git pull```就可以得到这些新提交的代码了。

下面我们将学习其他一些更为常用的```git```命令，经过下面的学习，你就可以使用这些基本的git操作命令来对文件和代码进行管理和协同开发了。

1) **添加新函数**

在开发人员```A```修改文件```main.py```中代码的同时，开发人员```B```在文件```main.py```中实现两个变量的乘积函数。修改后，```main.py```文件如下所示：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")


a = 10

b = 20
c = a + b
print("The value of c is  ", c)

def mul(a, b):
    return (a * b)
{% endhighlight %}
检查测试通过之后，执行如下命令添加到暂存区，并提交到仓库：
<pre>
$ git status
$ git add 
$ git commit -m "add a mul(a,b) function"
</pre>

好了，现在要将上面的代码推送到远程存储仓库，使用以下命令：
{% highlight string %}
$ git push origin master
Username for 'https://github.com': ivanzz1001
Counting objects: 3, done.
Delta compression using up to 2 threads.
To https://github.com/ivanzz1001/sample.git
 ! [rejected]        master -> master (fetch first)
error: failed to push some refs to 'https://github.com/ivanzz1001/sample.git'
hint: Updates were rejected because the remote contains work that you do
hint: not have locally. This is usually caused by another repository pushing
hint: to the same ref. You may want to first integrate the remote changes
hint: (e.g., 'git pull ...') before pushing again.
hint: See the 'Note about fast-forwards' in 'git push --help' for details.

{% endhighlight %}
但Git推送失败，因为Git确定远程存储库和本地存储不同步。为什么呢？因为在向```main.py```添加以下片段时：
{% highlight string %}
def mul(a, b)
    return (a+b)
{% endhighlight %}
另外一个开发人员```A```已经向远程仓库推送了修改的代码，所以这里在向远程仓库推送代码时，发现上面的新的推送代码，现在这个要推送的代码和远程仓库中的代码不一致，如果强行推送上去，Git不知道应该以谁的为准了。

所以，必须先更新本地存储库，只有在经过此步骤之后，才能推送自己的改变。

2) **获取最新更改**

执行```git pull```命令以以将其本地存储库与远程存储库同步：
{% highlight string %}
$ git pull
remote: Counting objects: 3, done.
remote: Compressing objects: 100% (3/3), done.
remote: Total 3 (delta 0), reused 0 (delta 0)
Unpacking objects: 100% (3/3), done.
From https://github.com/ivanzz1001/sample.git
   51de0f0..01c5462  master     -> origin/master
Auto-merging main.py
CONFLICT (content): Merge conflict in main.py
Automatic merge failed; fix conflicts and then commit the result.

{% endhighlight %}
现在打开```main.py```内容如下：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")


a = 10

b = 20
<<<<<<< HEAD
c = a + b
print("The value of c is  ", c)

def mul(a, b):
    return (a * b)
=======

def sum(a, b):
    return (a+b)

c = sum(a, b)
print("The value of c is  ", c)
>>>>>>> 01c54624879782e4657dd6c166ce8818f19e8251
{% endhighlight %}

拉取操作后，检查日志消息，并发现其他开发人员的提交的详细信息，提交ID为*01c54624879782e4657dd6c166ce8818f19e8251*。

打开```main.py```文件，修改其中的代码，修改完成后保存，文件的代码如下所示：
{% highlight string %}
#!/usr/bin/python3
#coding=utf-8

print ("Life is short, you need Python !")

a = 10
b = 20

def mul(a, b)
   return (a*b)

def sum(a, b)
   return (a+b)
   
c = sum(a, b)
print("The value of c is ", c)
{% endhighlight %}
再次添加提交，最后推送，如下命令：
{% highlight string %}
$ git add main.py

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git commit -m "synchronized with the remote repository"
[master f260043] synchronized with the remote repository
 1 file changed, 3 insertions(+)

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git status
On branch master
Your branch is ahead of 'origin/master' by 1 commit.
  (use "git push" to publish your local commits)
nothing to commit, working tree clean

Administrator@2NIJJZIFGF14CRQ MINGW32 /f/worksp/sample (master)
$ git push origin master
Username for 'https://github.com': ivanzz1001
Counting objects: 3, done.
Delta compression using up to 2 threads.
Compressing objects: 100% (3/3), done.
Writing objects: 100% (3/3), 350 bytes | 0 bytes/s, done.
Total 3 (delta 1), reused 0 (delta 0)
remote: Resolving deltas: 100% (1/1), completed with 1 local object.
To https://github.com/ivanzz1001/sample.git
   30d3ff5..f260043  master -> master
{% endhighlight %}
现在，一个新的代码又提交并推送到远程仓库中了。



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

