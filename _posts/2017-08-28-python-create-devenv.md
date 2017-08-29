---
layout: post
title: python开发环境的搭建
tags:
- python
categories: python
description: python开发环境的搭建
---

本文主要简单讲述一下python开发环境的搭建，以便后边查询。

<!-- more -->


## 1. Windows上python的安装


### 1.1 下载python
到[python官方网站](https://www.python.org/downloads/windows/)下载windows操作系统下的最新python版本：python3.6.2。

### 1.2 安装python3.6.2
这里采用自定义安装：

![python-windows-install](https://ivanzz1001.github.io/records/assets/img/python/python-windows-install.png)



### 1.3 验证是否安装成功
在windows命令行输入查看：

![python-windows-running](https://ivanzz1001.github.io/records/assets/img/python/python-windows-running.png)

如若不能找到相应的python命令，在一般需要在PATH环境变量中添加截图中的这两个环境变量。


### 1.4 升级pip
python pip是python包管理工具，可以通过如下命令进行升级：
{% highlight string %}
python -m pip install --upgrade pip

# 或

pip install --upgrade pip
{% endhighlight %}

通过如下命令进行重装：
{% highlight string %}
python -m pip install --upgrade pip --force-reinstall
{% endhighlight %}
假如我们在Windows上同时安装了python2与python3，则可能需要安装两个版本的pip。但是一般情况下，我们在安装2个版本python的时候可能系统只会自动帮我们安装上一个版本的pip，此时我们可以通过上述命令，选择适当的python版本（python2/python3)来重新安装pip。后续我们就可以通过pip2/pip3来决定使用哪一个pip了。


此外，我们也可以到官方网站上下载来安装：https://pypi.python.org/pypi/pip。下载完成后，进入到对应的目录，执行如下命令进行安装：
{% highlight string %}
python setup.py install
{% endhighlight %}

### 1.5 安装集成开发环境

这里使用pycharm集成开发环境。到官方网站下载（http://www.jetbrains.com/pycharm/download/#section=windows)。专业版需要收费，这里下载社区版本。下载后直接安装即可。

![pycharm-install](https://ivanzz1001.github.io/records/assets/img/python/pycharm-install.png)

安装完成后，配置相应的Project Interpreter。 从"File" -> "Default Settings" -> "Project Interpreter"进行配置。




<br />
<br />
<br />

