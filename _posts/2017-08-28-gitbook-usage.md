---
layout: post
title: gitbook的使用
tags:
- tools
categories: tools
description: gitbook的使用
---

本文主要简单讲述一下gitbook工具的使用，做一个记录。

<!-- more -->


## 1. 简介
{% highlight string %}
Modern book format and toolchain using Git and Markdown
{% endhighlight %}
这是 gitbook 项目主页上对 gitbook 的定义。

gitbook 首先是一个软件，正如上面定义的那样，它使用 Git 和 Markdown 来编排书本，如果用户没有听过 Git 和 Markdown，那么 gitbook 可能不适合你！

本书也是使用 gitbook 生成，所以在看到这里的时候，你应该对 gitbook 的魔力有了初步印象！

## 2. 安装
gitbook 的安装非常简单，详细指南可以参考 [gitbook 文档](https://github.com/GitbookIO/gitbook)。

这里的安装只需要一步就能完成！
{% highlight string %}
$ npm install gitbook -g
{% endhighlight %}
需要注意的是：用户首先需要安装 nodejs，以便能够使用 npm 来安装 gitbook。


## 3. 使用
gitbook 的基本用法非常简单，基本上就只有两步：

* 使用 gitbook init 初始化书籍目录
* 使用 gitbook serve 编译书籍

下面将结合一个非常简单的实例，来介绍 gitbook 的基本用法。

### 3.1 gitbook init
首先，创建如下目录结构：
<pre>
$ tree book/
book/
├── README.md
└── SUMMARY.md

0 directories, 2 files
</pre>
README.md 和 SUMMARY.md 是两个必须文件，README.md 是对书籍的简单介绍：



<pre>
$ cat book/README.md 
# README

This is a book powered by [GitBook](https://github.com/GitbookIO/gitbook).
</pre>


SUMMARY.md 是书籍的目录结构。内容如下：
<pre>
$ cat book/SUMMARY.md 
# SUMMARY

* [Chapter1](chapter1/README.md)
  * [Section1.1](chapter1/section1.1.md)
  * [Section1.2](chapter1/section1.2.md)
* [Chapter2](chapter2/README.md)
</pre>



创建了这两个文件后，使用 gitbook init，它会为我们创建 SUMMARY.md 中的目录结构。
<pre>
$ cd book
$ gitbook init
$ tree
.
├── README.md
├── SUMMARY.md
├── chapter1
│   ├── README.md
│   ├── section1.1.md
│   └── section1.2.md
└── chapter2
    └── README.md

2 directories, 6 files
</pre>
注意：在我的实验中，gitbook init 只支持两级目录！


### 3.2 gitbook serve
书籍目录结构创建完成以后，就可以使用 gitbook serve 来编译和预览书籍了：

<pre>
$ gitbook serve
Press CTRL+C to quit ...

Live reload server started on port: 35729
Starting build ...
Successfully built!

Starting server ...
Serving book on http://localhost:4000
</pre>
gitbook serve 命令实际上会首先调用 gitbook build 编译书籍，完成以后会打开一个 web 服务器，监听在本地的 4000 端口。

现在，可以用浏览器打开 http://127.0.0.1:4000 查看书籍的效果，如下图：

![gitbook serve](https://ivanzz1001.github.io/records/assets/img/tools/gitbook-sample.png)

现在，gitbook 为我们创建了书籍目录结构后，就可以向其中添加真正的内容了，文件的编写使用 markdown 语法，在文件修改过程中，每一次保存文件，gitbook serve 都会自动重新编译，所以可以持续通过浏览器来查看最新的书籍效果！

另外，用户还可以下载 gitbook 编辑器，做到所见即所得的编辑，如下图所示：

![gitbook editor](https://ivanzz1001.github.io/records/assets/img/tools/gitbook-editor.png)

gitbook editor 的使用非常简单，这里不再介绍！



## 4. 发布到 GitHub Pages
除了能够将书籍发布到 GitBook.com 外，还可以将书籍发布到 GitHub Pages，由于没有找到官方文档，所以这里记录的是我自己正在使用的一种方法。

如果读者不了解 GitHub Pages 为何物，简单说就是一个可以托管静态网站的 Git 项目，支持使用 markdown 语法以及 Jekyll 来构建，或者直接使用已经生成好的静态站点。详细可以参考 GitHub Pages 主页。

由于 gitbook 书籍可以通过 gitbook 本地构建出 site 格式，所以可以直接将构建好的书籍直接放到 GitHub Pages 中托管，之后，可以通过如下地址访问书籍：
{% highlight string %}
<username>.github.io/<project>
{% endhighlight %}

例如：这本书中使用的例子 'test' 项目可以通过地址：chengweiv5.github.io/test 来访问。

当访问 chengweiv5.github.io/test 时，会访问 chengweiv5/test 项目的 gh-pages 分支的内容，所以需要为项目创建一个 gh-pages 分支，并且将静态站点内容放入其中。也就是说，test 项目将有如下两个分支：

master, 保存书籍的源码
gh-pages, 保存书籍编译后的 HTML 文件


### 4.1 构建书籍
首先，使用 gitbook build 将书籍内容输出到默认目录，也就是当前目录下的 _book 目录。
<pre>
$ gitbook build
Starting build ...
Successfully built!

$ ls _book
GLOSSARY.html       chapter1            chapter2            gitbook             glossary_index.json index.html          search_index.json
</pre>


### 4.2 发布书籍
<pre>
$ gitbook serve
</pre>

### 4.3 创建 gh-pages 分支
执行如下命令来创建分支，并且删除不需要的文件：

<pre>
$ git checkout --orphan gh-pages
$ git rm --cached -r .
$ git clean -df
$ rm -rf *~
</pre>


现在，目录下应该只剩下 _book 目录了，首先，忽略一些文件：
{% highlight string %}
$ echo "*~" > .gitignore
$ echo "_book" >> .gitignore
$ git add .gitignore
$ git commit -m "Ignore some files"
{% endhighlight %}


然后，加入 _book 下的内容到分支中：
<pre>
$ cp -r _book/* .
$ git add .
$ git commit -m "Publish book"
</pre>


<br />
<br />
参看：http://www.chengweiyang.cn/gitbook/github-pages/README.html


<br />
<br />
<br />

