---
layout: post
title: docker compose的使用(1)
tags:
- docker
categories: docker
description: docker compose的使用(1)
---

本文简单的介绍docker compose的使用。



<!-- more -->



## 1. 简介

docker compose是一个工具集，用于定义和运行同时依赖于多个docker容器的应用程序。在使用docker compose时，通常会用到YAML文件来配置你的应用程序服务。然后通过该配置文件，你就可以很简单的创建和启动配置中的所有服务。

使用docker compse通常有如下三个步骤：

* 使用```Dockerfile```定义应用程序运行环境，这使得可以很方便的扩充移植

* 在```docker-compose.yml```文件中定义应用程序所依赖的service，这使得在一个隔离的环境中所有的服务得以运行

* 运行```docker compose up```命令来启动你的应用服务程序

docker compose有相应的命令来管理应用程序的整个生命周期：

* Start、Stop和Rebuild services

* 查看所运行服务的状态

* 打印出所运行服务的日志

* 在Service上运行一次性命令

## 2. docker compose特性

docker compose具有如下特性：

* 通过使用```工程名称(project name)```来达到环境隔离。 默认的project name是工程路径的basename值，你也可以通过```-p```命令行选项或者```COMPOSE_PROJECT_NAME```环境变量来进行更改。


## 3. docker compose的安装

在安装docker compose之前，首先得安装docker。

### 3.1 docker compose安装
linux环境下安装步骤如下：

1) **下载最新版docker compose**
<pre>
# sudo curl -L https://github.com/docker/compose/releases/download/1.20.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
</pre>
说明，我们上面使用的版本是```1.20.1```是当前最新版本，在为了某段时间可能过时。可以到[Compose repository release page on GitHub](https://github.com/docker/compose/releases)去检查最新版本。

2) **使docker compose有可执行权限**
<pre>
# sudo chmod +x /usr/local/bin/docker-compose
</pre>

3) **测试**
<pre>
# docker-compose --version
docker-compose version 1.20.1, build 5d8c71b
</pre>
 
### 3.1 卸载
通过上面curl方式安装的docker compose，可以通过直接删除的方式来卸载：
<pre>
# sudo rm /usr/local/bin/docker-compose
</pre>


## 4. docker compose使用示例
本例子我们会构建一个简单的Python Web应用程序，然后运行在docker compose上。该应用程序使用Flask框架，并且将每一次点击数据存放到redis里面。

### 4.1 编写Python Web应用程序

1) **创建工程目录**
<pre>
# mkdir composetest
# cd composetest
</pre>

2） **编写程序**

编写app.py程序：
{% highlight string %}
import time

import redis
from flask import Flask


app = Flask(__name__)
cache = redis.Redis(host='redis', port=6379)


def get_hit_count():
    retries = 5
    while True:
        try:
            return cache.incr('hits')
        except redis.exceptions.ConnectionError as exc:
            if retries == 0:
                raise exc
            retries -= 1
            time.sleep(0.5)


@app.route('/')
def hello():
    count = get_hit_count()
    return 'Hello World! I have been seen {} times.\n'.format(count)

if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)
{% endhighlight %}

说明： 在本例子中，```redis```是应用程序网络中redis容器的主机名(根据下面docker-compose.yml中service）。我们使用redis默认的端口6379


### 4.2 创建Dockfile
我们在composetest目录下创建Dockefile文件：
{% highlight string %}
FROM python:3.4-alpine
ADD . /code
WORKDIR /code
RUN pip install -r requirements.txt
CMD ["python", "app.py"]
{% endhighlight %}

上面Dockerfile告诉docker:

* 从python:3.4镜像开始构建

* 添加当前目录```.```到镜像的/code路径

* 设置当前工作目录为/code

* 安装Python依赖

* 为空气设置默认的执行命令

### 4.3 在Compose文件中定义service
在composetest目录下创建创建```docker-compose.yml```:
{% highlight string %}
version: '3'
services:
  web:
    build: .
    ports:
     - "5000:5000"
  redis:
    image: "redis:alpine"
{% endhighlight %}

上面compose文件定义了两个服务，```web```和```redis```。该Web服务：

* 使用一个当前目录Dockerfile构建出的镜像

* 将容器暴露的5000端口映射到主机的5000端口。这里我们使用Flask Web服务默认的5000端口

redis服务使用Docker Hub镜像仓库中的公有镜像。

### 4.3 使用Compose构建并运行

1） **运行```docker compose up```命令构建并运行应用程序**
<pre>
[root@bogon composetest]# docker-compose up
Building web
Step 1/5 : FROM python:3.4-alpine
3.4-alpine: Pulling from library/python
81033e7c1d6a: Pull complete
9b61101706a6: Pull complete
415e2a07c89b: Pull complete
f22df7a3f000: Pull complete
af78bda78f1f: Pull complete
Digest: sha256:989b6044c434ffadf4dbc116719d73e7e31f5ac0f75f59b7591aeb766c874e26
Status: Downloaded newer image for python:3.4-alpine
 ---> 6610ae9fa51a
Step 2/5 : ADD . /code
 ---> 6a1e88758361
Step 3/5 : WORKDIR /code
Removing intermediate container a85d40ee91d1
</pre>

docker compose会拉取一个redis镜像，并根据我们的Dockerfile构建出应用程序镜像，然后运行。

2) **测试**

我们可以用浏览器或者curl命令来进行测试：
<pre>
# curl -X GET http://192.168.69.128:5000 
Hello World! I have been seen 1 times.

# curl -X GET http://192.168.69.128:5000 
Hello World! I have been seen 2 times.

# curl -X GET http://192.168.69.128:5000 
Hello World! I have been seen 3 times.
</pre>


3) **docker镜像及容器名**

这里注意我们构建出的镜像名称：
<pre>
# docker images
REPOSITORY              TAG                 IMAGE ID            CREATED             SIZE
composetest_web         latest              510e2782b0d9        About an hour ago   94.6MB
python                  3.4-alpine          6610ae9fa51a        4 days ago          83.6MB
redis                   alpine              c27f56585938        13 days ago         27.7MB

//通过如下命令查看镜像的详细信息
# docker inspect composetest_web
</pre>
可以看到名称为```projectname_servicename```的形式。而运行的容器的名字为：
<pre>
# docker ps -a
CONTAINER ID        IMAGE                          COMMAND                  CREATED             STATUS                    PORTS                          NAMES
e38e7c842df1        redis:alpine                   "docker-entrypoint.s…"   About an hour ago   Up About an hour          6379/tcp                       composetest_redis_1
6135b45d9068        composetest_web                "python app.py"          About an hour ago   Up About an hour          0.0.0.0:5000->5000/tcp         composetest_web_1


//通过如下命令查看容器的详细信息
# docker inspect composetest_web_1
</pre>

### 4.4 

<br />
<br />
<br />

