---
layout: post
title: netrc文件的使用
tags:
- LinuxOps
categories: linux
description: linux调试
---


本文介绍一下`.netrc`文件的使用。


<!-- more -->


## 1. netrc文件

自动登录程序会使用```.netrc```文件来指定登录和初始化相关信息。该文件通常放置于用户的home目录, 如果要放置于其他目录则必须添加到```NETRC```环境变量中。`.netrc`文件是一个regular文件，否则访问可能被拒绝。

`.netrc`文件识别如下token，token之间需要使用空格、tab、或者新行(new-line)来进行分割：


- ### 'machine name'

    指定一台远程主机名。自动登录程序(如: ftp、ssh)会搜索`.netrc`文件来获得所匹配的远程目标主机的用户名、密码。一旦匹配成功，则会继续处理后面的子token，直到遇见文件结束、或者另一个machine token、或者是default token。


- ### 'default'

    'default'可以匹配任何的名称，除此之外与'machine name'完全一致。一个`.netrc`文件中最多只能含有一个'default' token，并且其必须放置在所有'machine' token之后。通常用法类似如下：

{% highlight string %}
default login anonymous password user@site
{% endhighlight %}

    通过上面用户就可以匿名登录相关的远程主机。


- ### 'login name'

    用于指定远程主机上的一个用户。假如此token存在的话，则自动登录程序会使用此token指定的用户名来进行登录。


- ### 'password string'

    用于指定密码。假如此token存在的话，自动登录程序则会使用本token指定的密码来登录远程目标主机(ps: 假如登录时需要密码的话）。值得注意的是：假如此token在`.netrc`文件中存在的话，自动登录程序会abort掉除'login name'用户和匿名用户之外的所有其他用户的登录流程。

- ### 'account string'

    用于指定一个额外的账号密码。假如此token存在，且自动登录程序在登录远程主机时如果需要额外账户密码，则会使用此字段指定的密码。

- ### 'macdef name'
    用于顶一个macro。







<br />
<br />
**[参看]:**

1. [The .netrc file](https://www.gnu.org/software/inetutils/manual/html_node/The-_002enetrc-file.html)


<br />
<br />
<br />





