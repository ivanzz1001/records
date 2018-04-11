---
layout: post
title: docker registry与repository的区别
tags:
- docker
categories: docker
description: docker registry与repository的区别
---


在docker的开发过程中，我们经常会遇到```registry```与```repository```这两个单词。特别是对于后者，我们经常会有些不清楚其含义。


<!-- more -->

## 1. docker registry

下面是对docker registry的一个基本定义：
<pre>
Docker registry is a service that is storing your docker images
</pre>
Docker registry可以是部署在第三方平台上的一个公有或私有仓库。例如有如下一些仓库：

* [Docker Hub](https://hub.docker.com/)

* [Quary](https://quay.io/)

* [Google Container Registry](https://cloud.google.com/container-registry/)

* [AWS Container Registry](https://aws.amazon.com/ecr/)

你也可以在你自己的主机上部署一套镜像仓库。

## 2. docker repository

下面是对docker repository的一个基本定义：
<pre>
Docker repository is a collection of different images with same name, that have different tags
</pre>

在一个repository中一个镜像(image)的tag就是一个```alphanumeric```标识。

例如[https://hub.docker.com/r/library/python/tags/](https://hub.docker.com/r/library/python/tags/), 官方的python镜像就有很多不同的tag，这些tag都是Docker Hub中官方python repository的成员。Docker Hub是由Docker公司维护的一个Docker Registry。









<br />
<br />

**[参看]**

1. [Difference between Docker registry and repository](https://stackoverflow.com/questions/34004076/difference-between-docker-registry-and-repository)

2. [Repository和Registry区别](https://blog.csdn.net/rickiyeat/article/details/74509737)

<br />
<br />
<br />

