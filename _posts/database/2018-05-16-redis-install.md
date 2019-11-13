---
layout: post
title: redis安装及简单使用
tags:
- database
categories: database
description: redis安装及简单使用
---



本文简要记录一下redis的安装及简单使用。具体的安装环境如下：


<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 


# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
当前我们使用较多的版本是： redis 3.2.11

## 1. redis安装

1） 下载redis

到redis官方网站下载对应版本的redis安装包，这里下载```redis 3.2.11```版本：
<pre>
# wget http://download.redis.io/releases/redis-3.2.11.tar.gz
</pre>

2) 解压并安装
<pre>
# tar -zxvf redis-3.2.11.tar.gz
# cd redis-3.2.11

# mkdir -p /usr/local/redis-3.2.11
# make 
# make PREFIX=/usr/local/redis-3.2.11 install
cd src && make install
make[1]: Entering directory `/root/redis-inst/pkgs/redis-3.2.11/src'

Hint: It's a good idea to run 'make test' ;)

    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
make[1]: Leaving directory `/root/redis-inst/pkgs/redis-3.2.11/src'

# ls /usr/local/redis-3.2.11/bin/
redis-benchmark  redis-check-aof  redis-check-rdb  redis-cli  redis-sentinel  redis-server
</pre>
可以看到上面将redis安装熬了/usr/local/redis-3.2.11目录。


3） 拷贝默认的redis.conf到安装目录

下面我们在redis源代码目录下拷贝一个范本redis.conf到安装目录：
<pre>
# cp redis.conf /usr/local/redis-3.2.11/bin/
</pre>



4) 启动redis
<pre>
# /usr/local/redis-3.2.11/bin/redis-server
5298:C 16 May 14:15:32.427 # Warning: no config file specified, using the default config. In order to specify a config file use /usr/local/bin/redis-server /path/to/redis.conf
5298:M 16 May 14:15:32.428 * Increased maximum number of open files to 10032 (it was originally set to 1024).
                _._                                                  
           _.-``__ ''-._                                             
      _.-``    `.  `_.  ''-._           Redis 3.2.11 (00000000/0) 64 bit
  .-`` .-```.  ```\/    _.,_ ''-._                                   
 (    '      ,       .-`  | `,    )     Running in standalone mode
 |`-._`-...-` __...-.``-._|'` _.-'|     Port: 6379
 |    `-._   `._    /     _.-'    |     PID: 5298
  `-._    `-._  `-./  _.-'    _.-'                                   
 |`-._`-._    `-.__.-'    _.-'_.-'|                                  
 |    `-._`-._        _.-'_.-'    |           http://redis.io        
  `-._    `-._`-.__.-'_.-'    _.-'                                   
 |`-._`-._    `-.__.-'    _.-'_.-'|                                  
 |    `-._`-._        _.-'_.-'    |                                  
  `-._    `-._`-.__.-'_.-'    _.-'                                   
      `-._    `-.__.-'    _.-'                                       
          `-._        _.-'                                           
              `-.__.-'                                               

</pre>

上面是以前台工作方式启动redis，下面我们修改配置文件redis.conf：
<pre>
# cp /usr/local/redis-3.2.11/bin/redis.conf /usr/local/redis-3.2.11/bin/redis-default.conf
# mv /usr/local/redis-3.2.11/bin/redis.conf  /usr/local/redis-3.2.11/bin/redis_6379.conf

# vi /usr/local/redis-3.2.11/bin/redis_6379.conf
##bind 127.0.0.1
protected-mode no
daemonize yes
</pre>

上面把```bind 127.0.0.1```注释掉，并且将```daemonize no```修改为```daemonize yes```。另外将protected-mode设置然后再启动redis_server:
<pre>
# /usr/local/redis-3.2.11/bin/redis-server /usr/local/redis-3.2.11/bin/redis_6379.conf

# ps -ef | grep redis-server
root      8555     1  0 14:37 ?        00:00:00 /usr/local/redis-3.2.11/bin/redis-server *:6379
root      8559  2109  0 14:37 pts/1    00:00:00 grep --color=auto redis-server
</pre>


5) 简单测试redis
{% highlight string %}
# /usr/local/redis-3.2.11/bin/redis-cli -h localhost -p 6379
localhost:6379> ping
PONG
localhost:6379> 
{% endhighlight %}





<br />
<br />

**[参看]**

1. [CENTOS7下安装REDIS](https://www.cnblogs.com/zuidongfeng/p/8032505.html)


<br />
<br />
<br />

