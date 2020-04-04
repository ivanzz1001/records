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

```(注：windows xp上最后支持的python版本是python3.4)```

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

```（windows xp下使用pycharm-2016.2版本)）```

![pycharm-install](https://ivanzz1001.github.io/records/assets/img/python/pycharm-install.png)

安装完成后，配置相应的Project Interpreter。 从"File" -> "Default Settings" -> "Project Interpreter"进行配置。

### 1.6 编写一个hello,world程序

注意我们这里使用的是Python3:

![python-helloworld](https://ivanzz1001.github.io/records/assets/img/python/python-helloworld.png)


编译运行，打印出hello,world


## 2. Linux上安装python

我们系统上默认安装有python2.7:
{% highlight string %}
[root@localhost install-pkg]# python -V
Python 2.7.5
{% endhighlight %}

下面我们来安装python3。

### 2.1 下载python3源码

到官方网站下载python源码：https://www.python.org/downloads/source/
{% highlight string %}
#  wget https://www.python.org/ftp/python/3.6.2/Python-3.6.2.tgz
{% endhighlight %}

### 2.2 安装编译环境（依赖包）
{% highlight string %}
# yum install zlib-devel bzip2-devel  openssl-devel ncurses-devel
{% endhighlight %}


### 2.3 编译安装Python
{% highlight string %}
# tar -xvf Python-3.6.2.tgz

# cd Python-3.6.2/

# ./configure --prefix=/usr/local/python3

# make
# make test
# sudo make install
{% endhighlight %}

建立软链接：
{% highlight string %}
# ln -s /usr/local/python3/bin/python3.6 /usr/local/bin/python3
# ln -s /usr/local/python3/bin/pip3.6 /usr/local/bin/pip3
{% endhighlight %}

如果提示：Ignoring ensurepip failure: pip 7.1.2 requires SSL/TLS

这是原因没有安装或升级oenssl:```yum install openssl-devel```。再次重复编译方案python3.5


### 2.4 pip升级
要使用wheel来安装python包，需要```pip >= 1.4```以及```setuptools >= 0.8```。我们可以执行如下命令来升级pip:
{% highlight string %}
# pip -V
pip 19.3.1 from /usr/lib/python2.7/site-packages/pip (python 3.6)
# pip install --upgrade pip
{% endhighlight %}
查看setuptools版本：
<pre>
# ls /usr/lib/python2.7/site-packages/setuptools*
</pre>
如果setuptools版本不符合要求，可用如下方式进行卸载，并安装指定版本：
<pre>
# pip uninstall setuptools
# pip install setuptools==39.1.0
</pre>



### 2.5 使用pip安装包
示例：

{% highlight string %}
//requests包：
# pip install requests

//pyquery包：
# pip install pyquery

//pymysql包：
# pip install pymysql
{% endhighlight %}

默认情况下，pip会从https://pypi.python.org/pypi来安装包。根据系统不同，在windows环境下通过pip安装的扩展模块一般位于python目录下Lib\site-packages目录； 而在linux环境下通过pip安装的扩展模块一般位于/usr/local/lib/python/dist-packages/目录。win7下pip下载的安装包缓存位置为：c:\用户\(你的用户名)\AppData\Local\pip\cache\； Linux下pip下载的安装包缓存位置为：~/.cache/pip


<br />
<br />

**参考**

1. [Linux安装python2.7、pip和setuptools](https://www.cnblogs.com/xiaowenshu/p/10239834.html)

2. [将python包发布到PyPI和制作whl文件](https://blog.csdn.net/winycg/article/details/80025432)

<br />
<br />
<br />

