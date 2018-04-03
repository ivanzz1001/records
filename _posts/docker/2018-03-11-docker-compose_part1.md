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

### 4.4 使用Compose构建并运行

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

### 4.5 修改Compose文件，以添加bind mount
在工程目录修改docker-compose.yml文件，为```web```服务添加```bind mount```:
<pre>
version: '3'
services:
  web:
    build: .
    ports:
     - "5000:5000"
    volumes:
     - .:/code
  redis:
    image: "redis:alpine"
</pre>

上面关键词```volumes```会将当前主机上的工程目录挂载到容器中的```/code```目录，这样就允许在不重新构建镜像的基础上，动态的修改代码。

### 4.6 用docker compose重新构建并运行应用程序
在工程目录针对修改后的docker-compose.yml文件，重新构建并运行应用程序：
<pre>
# docker-compose up
Starting composetest_redis_1 ... done
Recreating composetest_web_1 ... done
Attaching to composetest_redis_1, composetest_web_1
redis_1  | 1:C 28 Mar 01:32:37.726 # oO0OoO0OoO0Oo Redis is starting oO0OoO0OoO0Oo
redis_1  | 1:C 28 Mar 01:32:37.726 # Redis version=4.0.8, bits=64, commit=00000000, modified=0, pid=1, just started
redis_1  | 1:C 28 Mar 01:32:37.726 # Warning: no config file specified, using the default config. In order to specify a config file use redis-server /path/to/redis.conf
redis_1  | 1:M 28 Mar 01:32:37.733 * Running mode=standalone, port=6379.
redis_1  | 1:M 28 Mar 01:32:37.733 # WARNING: The TCP backlog setting of 511 cannot be enforced because /proc/sys/net/core/somaxconn is set to the lower value of 128.
redis_1  | 1:M 28 Mar 01:32:37.733 # Server initialized
redis_1  | 1:M 28 Mar 01:32:37.734 # WARNING overcommit_memory is set to 0! Background save may fail under low memory condition. To fix this issue add 'vm.overcommit_memory = 1' to /etc/sysctl.conf and then reboot or run the command 'sysctl vm.overcommit_memory=1' for this to take effect.
redis_1  | 1:M 28 Mar 01:32:37.734 # WARNING you have Transparent Huge Pages (THP) support enabled in your kernel. This will create latency and memory usage issues with Redis. To fix this issue run the command 'echo never > /sys/kernel/mm/transparent_hugepage/enabled' as root, and add it to your /etc/rc.local in order to retain the setting after a reboot. Redis must be restarted after THP is disabled.
redis_1  | 1:M 28 Mar 01:32:37.734 * DB loaded from disk: 0.000 seconds
redis_1  | 1:M 28 Mar 01:32:37.734 * Ready to accept connections
web_1    |  * Running on http://0.0.0.0:5000/ (Press CTRL+C to quit)
web_1    |  * Restarting with stat
web_1    |  * Debugger is active!
web_1    |  * Debugger PIN: 225-078-826
</pre>
通过如下命令进行测试：
{% highlight string %}
# curl -X GET http://192.168.69.128:5000 
Hello World! I have been seen 4 times.
# curl -X GET http://192.168.69.128:5000 
Hello World! I have been seen 5 times.
{% endhighlight %}
这里我们可以看到```Redis```加载了以前的数据。

### 4.7 更新应用程序

因为当前使用一个volume将应用程序代码挂载到了容器中，因此我们可以直接修改应用程序代码，然后不需要经过重新构建即可看到相应的改变。

1) **修改app.py代码**

这里将```Hello World!```修改为```Hello from Docker!```:
<pre>
return 'Hello from Docker! I have been seen {} times.\n'.format(count)
</pre>

2) **测试**
<pre>
# curl -X GET http://192.168.69.128:5000 
Hello from Docker! I have been seen 6 times.
</pre>
可以看到这里修改马上就生效了。


### 4.8 测试一些其他的命令

通过在运行时传递```-d```（detached模式）选项，可以使服务以后台的方式运行。使用```docker-compose ps```命令来查看当前有哪些服务在运行：
{% highlight string %}
# docker-compose up -d
Starting composetest_redis_1 ... done
Starting composetest_web_1   ... done
# docker-compose ps
       Name                      Command               State           Ports         
-------------------------------------------------------------------------------------
composetest_redis_1   docker-entrypoint.sh redis ...   Up      6379/tcp              
composetest_web_1     python app.py                    Up      0.0.0.0:5000->5000/tcp
{% endhighlight %}


```docker-compose run```命令允许你为某个service运行一次性命令。例如，我们可以通过如下命令来查看```web```服务的环境变量：
<pre>
# docker-compose run web env
PATH=/usr/local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
HOSTNAME=c3648dbe964e
TERM=xterm
LANG=C.UTF-8
GPG_KEY=97FC712E4C024BBEA48A61ED3A5CA953F73C700D
PYTHON_VERSION=3.4.8
PYTHON_PIP_VERSION=9.0.3
HOME=/root
</pre>


假如你是通过```docker-compose up -d```来启动的服务，那么你可以通过如下方式停止他们：
<pre>
# docker-compose stop
</pre>

你也可以通过使用```docker-compose down```命令将所有服务都关掉，并且移除整个容器。通过传递```--volumes```参数来移除Redis容器所使用的数据卷。
<pre>
# docker-compose down --volumes
</pre>

## 5. docker-compose基本命令

相关命令选项：
{% highlight string %}
# docker-compose --help
Define and run multi-container applications with Docker.

Usage:
  docker-compose [-f <arg>...] [options] [COMMAND] [ARGS...]
  docker-compose -h|--help

Options:
  -f, --file FILE             Specify an alternate compose file
                              (default: docker-compose.yml)
  -p, --project-name NAME     Specify an alternate project name
                              (default: directory name)
  --verbose                   Show more output
  --log-level LEVEL           Set log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)
  --no-ansi                   Do not print ANSI control characters
  -v, --version               Print version and exit
  -H, --host HOST             Daemon socket to connect to

  --tls                       Use TLS; implied by --tlsverify
  --tlscacert CA_PATH         Trust certs signed only by this CA
  --tlscert CLIENT_CERT_PATH  Path to TLS certificate file
  --tlskey TLS_KEY_PATH       Path to TLS key file
  --tlsverify                 Use TLS and verify the remote
  --skip-hostname-check       Don't check the daemon's hostname against the
                              name specified in the client certificate
  --project-directory PATH    Specify an alternate working directory
                              (default: the path of the Compose file)
  --compatibility             If set, Compose will attempt to convert deploy
                              keys in v3 files to their non-Swarm equivalent

Commands:
  build              Build or rebuild services
  bundle             Generate a Docker bundle from the Compose file
  config             Validate and view the Compose file
  create             Create services
  down               Stop and remove containers, networks, images, and volumes
  events             Receive real time events from containers
  exec               Execute a command in a running container
  help               Get help on a command
  images             List images
  kill               Kill containers
  logs               View output from containers
  pause              Pause services
  port               Print the public port for a port binding
  ps                 List containers
  pull               Pull service images
  push               Push service images
  restart            Restart services
  rm                 Remove stopped containers
  run                Run a one-off command
  scale              Set number of containers for a service
  start              Start services
  stop               Stop services
  top                Display the running processes
  unpause            Unpause services
  up                 Create and start containers
  version            Show the Docker-Compose version information
{% endhighlight %}

### 5.1 指定Compose文件路径

你可以使用```-f```选项指定一个或多个compose配置文件路径。

1) **指定多个Compose文件**

你可以使用多个```-f```选项来传递配置文件。当提供多个配置文件时，Compose会将这些传递进来的配置合并成一个配置。Compose会按传入的顺序来构建配置，后续的配置会覆盖前面的配置。例如：
<pre>
# docker-compose -f docker-compose.yml -f docker-compose.admin.yml run backup_db
</pre>


### 5.2 使用```-p```选项

每一个配置都有一个工程名。你可以通过```-p```选项来指定一个工程名称。假如你并未指定该选项的话，Compose会使用当前的目录名称作为工程名。例如：
<pre>
# docker-compose -p pythonweb up

# docker-compose -p pythonweb down --volumes
</pre>

### 5.3 设置环境变量
你可以为docker compose的许多选项设置环境变量，包括```-f```选项和```-p```选项。例如你可以为```-f```选项设置```COMPOSE_FILE```环境变量；可以为```-p```选项设置```COMPOSE_PROJECT_NAME```环境变量。


<br />
<br />
<br />

