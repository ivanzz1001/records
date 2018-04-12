---
layout: post
title: Harbor使用手册
tags:
- docker
categories: docker
description: Harbor使用手册
---


本章将会介绍一下Harbor的基本使用， 主要包括以下方面的内容：

* Harbor中工程(projects)的管理

* project中成员(members)的管理

* 复制工程到远程仓库(remote registry)

* 查询工程(projects)及repositories

* 系统管理员身份对Harbor系统的管理

* 使用docker client拉取及推送镜像

* 为repositories添加描述

* 删除repositories及镜像

* Content Trust


<!-- more -->

## 1. 基于角色的访问控制

Harbor提供了基于角色的访问控制（Role Based Access Control, RBAC):

![docker-harbor-rbac](https://ivanzz1001.github.io/records/assets/img/docker/docker_harbor_rbac.png)


Harbor通过工程（projects)的方式来管理镜像(images)。在添加用户到工程中时，可以是如下三种角色中的一种：

* ```Guest```: 指定工程（project)中的Guest用户只有只读访问权限

* ```Developer```: 工程中的Developer用户具有读写访问权限

* ```ProjectAdmin```: 当创建一个新的工程时，你就会被默认指定为该工程的```ProjectAdmin```角色。除了具有读写权限之外，```ProjectAdmin```还有一些管理特权，例如向该工程添加/移除成员、启动一个vulnerability扫描

另外除了这上面这三种角色，还有两个系统层面的角色：

* ```SysAdmin```: SysAdmin角色的用户具有最高管理权限。除了上面提到的一些权限之外，```SysAdmin```用户可以列出所有的工程(projects)； 将一个普通用户设置为管理员； 删除用户； 针对所有镜像设置vulnerability扫描策略。公有工程```library```也是由系统管理员所拥有。

* ```Anonymous```: 当一个用户并没有登录的时候，该用户会被认为是一个匿名用户。一个匿名用户并没有访问私有工程(private project)的权限, 而对于公有工程(public project)拥有只读访问权限。


视频示例: [Tencent Video](https://v.qq.com/x/page/l0553yw19ek.html)



<br />
<br />

**[参看]**

1. [Harbor User Guide](https://github.com/vmware/harbor/blob/master/docs/user_guide.md)


<br />
<br />
<br />

