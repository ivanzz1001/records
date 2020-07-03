---
layout: post
title: Nginx的命令行控制
tags:
- nginx
categories: nginx
description: Nginx的命令行控制
---


本章我们主要介绍Nginx命令行的使用。




<!-- more -->


## 1. Nginx命令行控制

在Linux中，需要使用命令行来控制Nginx服务器的启动与停止、重载配置文件、回滚日志文件、平滑升级等行为。针对我们当前Nginx的安装情况，我们将nginx安装在/usr/local/nginx目录：
<pre>
root@ubuntu:/usr/local/nginx# ls
client_body_temp        html                nginx                scgi_temp
fastcgi.conf            koi-utf             nginx.conf           uwsgi_params
fastcgi.conf.default    koi-win             nginx.conf.default   uwsgi_params.default
fastcgi_params          logs                proxy_temp           uwsgi_temp
fastcgi_params.default  mime.types          scgi_params          win-utf
fastcgi_temp            mime.types.default  scgi_params.default
</pre>

默认情况下nginx.conf配置情况如下：
<pre>
root@ubuntu:/usr/local/nginx# cat nginx.conf

#user  nobody;
worker_processes  1;

#error_log  logs/error.log;
#error_log  logs/error.log  notice;
#error_log  logs/error.log  info;

#pid        logs/nginx.pid;


events {
    worker_connections  1024;
}


http {
    include       mime.types;
    default_type  application/octet-stream;

    #log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
    #                  '$status $body_bytes_sent "$http_referer" '
    #                  '"$http_user_agent" "$http_x_forwarded_for"';

    #access_log  logs/access.log  main;

    sendfile        on;
    #tcp_nopush     on;

    #keepalive_timeout  0;
    keepalive_timeout  65;

    #gzip  on;

    server {
        listen       80;
        server_name  localhost;

        #charset koi8-r;

        #access_log  logs/host.access.log  main;

        location / {
            root   html;
            index  index.html index.htm;
        }

        #error_page  404              /404.html;

        # redirect server error pages to the static page /50x.html
        #
        error_page   500 502 503 504  /50x.html;
        location = /50x.html {
            root   html;
        }

        # proxy the PHP scripts to Apache listening on 127.0.0.1:80
        #
        #location ~ \.php$ {
        #    proxy_pass   http://127.0.0.1;
        #}

        # pass the PHP scripts to FastCGI server listening on 127.0.0.1:9000
        #
        #location ~ \.php$ {
        #    root           html;
        #    fastcgi_pass   127.0.0.1:9000;
        #    fastcgi_index  index.php;
        #    fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
        #    include        fastcgi_params;
        #}

        # deny access to .htaccess files, if Apache's document root
        # concurs with nginx's one
        #
        #location ~ /\.ht {
        #    deny  all;
        #}
    }


    # another virtual host using mix of IP-, name-, and port-based configuration
    #
    #server {
    #    listen       8000;
    #    listen       somename:8080;
    #    server_name  somename  alias  another.alias;

    #    location / {
    #        root   html;
    #        index  index.html index.htm;
    #    }
    #}


    # HTTPS server
    #
    #server {
    #    listen       443 ssl;
    #    server_name  localhost;

    #    ssl_certificate      cert.pem;
    #    ssl_certificate_key  cert.key;

    #    ssl_session_cache    shared:SSL:1m;
    #    ssl_session_timeout  5m;

    #    ssl_ciphers  HIGH:!aNULL:!MD5;
    #    ssl_prefer_server_ciphers  on;

    #    location / {
    #        root   html;
    #        index  index.html index.htm;
    #    }
    #}

}
</pre>

我们如下操作均是针对这一环境。

注： 对于日志我们一般配置为如下即可
{% highlight string %}
log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
                      '$status $body_bytes_sent "$http_referer" '
                      '"$http_user_agent" "$http_x_forwarded_for"';
access_log  /var/log/nginx/access.log combined buffer=512k flush=5m;
{% endhighlight %}
对于上面打印出的日志，这里```time_local```是响应时的时间戳，而不是请求时的时间戳。

### 1.1 默认启动方式

直接执行Nginx二进制程序：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx 

root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      1940     1  0 06:23 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    1941  1940  0 06:23 ?        00:00:00 nginx: worker process
root      1945  1906  0 06:24 pts/8    00:00:00 grep --color=auto nginx
{% endhighlight %}

启动完成后，我们会看到在/usr/local/nginx/logs目录下生成如下两个文件：
{% highlight string %}
root@ubuntu:/usr/local/nginx# ls /usr/local/nginx/logs
access.log  error.log
{% endhighlight %}

同时由于我们在configure时指定了pid文件为/usr/local/nginx/nginx.pid，因此我们会在该目录下看到这个文件：
{% highlight string %}
root@ubuntu:/usr/local/nginx# cat nginx.pid 
2289
{% endhighlight %}

### 1.2 另行指定配置文件的启动方式

使用```-c```参数指定配置文件：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -c /tmp/nginx.conf
{% endhighlight %}

这时，会读取-c参数后指定的nginx.conf配置文件来启动nginx.

### 1.3 另行指定安装目录的启动方式

使用```-p```参数指定Nginx的安装目录。这里我们将/usr/local/nginx/nginx可执行文件拷贝到另外一个目录来做此实验：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /tmp/nginx -p /usr/local/nginx

root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2323     1  0 06:45 ?        00:00:00 nginx: master process /tmp/nginx -p /usr/local/nginx
nobody    2324  2323  0 06:45 ?        00:00:00 nginx: worker process
root      2327  1906  0 06:46 pts/8    00:00:00 grep --color=auto nginx

root@ubuntu:/usr/local/nginx# cat /usr/local/nginx/nginx.pid 
2323
{% endhighlight %}

### 1.4 另行指定全局配置项的启动方式

可以通过```-g```参数临时指定一些全局配置项，以使新的配置项生效。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -g "pid /var/nginx.pid;"
root@ubuntu:/usr/local/nginx# cat /var/nginx.pid 
2400
{% endhighlight %}

上面这条命令意味着会把pid文件写到/var/nginx.pid中。

```-g```参数的约束条件是指定的配置项不能与默认路径下nginx.conf中的配置项相冲突，否则无法启动。就如上例那样，类似这样的配置项： pid logs/nginx.pid是不能存在于默认的nginx.conf中的。

另一个约束条件是，以```-g```方式启动的nginx服务执行其他命令时，需要把```-g```参数也带上，否则可能出现配置项不匹配的情形。例如，如果要停止nginx服务，那么需要执行下面代码：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -g "pid /var/nginx.pid;" -s stop
{% endhighlight %}


如果不带上```-g "pid /var/nginx.pid;"```，那么找不到pid文件，也会出现无法停止服务的情况。

### 1.5 测试配置信息是否有误

在不启动nginx的情况下，使用-t参数测试配置文件是否有错误。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -c ./nginx.conf -t 
nginx: the configuration file /usr/local/nginx/./nginx.conf syntax is ok
nginx: configuration file /usr/local/nginx/./nginx.conf test is successful
{% endhighlight %}

执行结果显示配置是否正确。

### 1.6 在测试配置阶段不输出信息

测试配置选项时，使用```-q```参数可以不把error级别以下的信息输出到屏幕。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -c ./nginx.conf -t -q
root@ubuntu:/usr/local/nginx# 
{% endhighlight %}

### 1.7 显示版本信息
使用```-v```参数显示Nginx的版本信息。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -v
nginx version: nginx/1.10.3
{% endhighlight %}


### 1.8 显示编译阶段的参数
使用```-V```参数除了可以显示Nginx版本信息外，还可以显示配置编译阶段的信息，如GCC编译器版本、操作系统版本、执行configure时的参数等。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -V
nginx version: nginx/1.10.3
built by gcc 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) 
built with OpenSSL 1.0.2g  1 Mar 2016
TLS SNI support enabled
configure arguments: --sbin-path=/usr/local/nginx/nginx --conf-path=/usr/local/nginx/nginx.conf --pid-path=/usr/local/nginx/nginx.pid --with-http_ssl_module --with-pcre=../pcre-8.40 --with-zlib=../zlib-1.2.11
{% endhighlight %}


### 1.9 快速地停止服务
使用```-s stop```可以强制停止nginx服务。```-s```参数其实是告诉nginx程序向正在运行的Nginx服务发送信号，Nginx程序通过nginx.pid文件中得到的master进程的进程ID，再向运行中的master进程发送```TERM```信号来快速关闭Nginx服务。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -s stop
{% endhighlight %}

实际上，如果通过kill命令直接向nginx master进程发送```TERM```或```INT```信号，效果是一样的。例如，先通过ps命令查看Nginx master进程ID：
{% highlight string %}
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2521     1  0 08:07 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    2522  2521  0 08:07 ?        00:00:00 nginx: worker process
root      2524  1906  0 08:07 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# kill -s SIGTERM 2521
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2526  1906  0 08:08 pts/8    00:00:00 grep --color=auto nginx
{% endhighlight %}
或者：
{% highlight string %}
root@ubuntu:/usr/local/nginx# kill -s SIGINT 2521
{% endhighlight %}

上述两条命令的效果与执行/usr/local/nginx/nginx -s stop是完全一样的。

### 1.10 "优雅" 地停止服务
如果希望Nginx服务可以正常的处理完成当前所有请求再停止服务，那么可以使用```-s quit```参数来停止服务。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -s quit
{% endhighlight %}

该命令与快速停止Nginx服务是有区别的。当快速停止服务时，worker进程与master进程在收到信号后会立刻跳出循环，退出进程。而“优雅”地停止服务时，首先会关闭监听端口，停止接收新的连接，然后把当前正在处理的连接全部处理完成，最后再退出进程。

与快速停止服务相似，可以直接发送```QUIT```信号给master进程来停止服务，其效果与执行```-s quit```命令是一样的。例如：
{% highlight string %}
# kill -s SIGQUIT <nginx master pid>
{% endhighlight %}

如果希望“优雅”的停止某个worker进程，那么可以通过向该进程发送```WINCH```信号来停止服务。例如：
{% highlight string %}
# kill -s SIGWINCH <nginx worker pid>
{% endhighlight %}

### 1.11 使运行中的nginx重读配置项并生效
使用```-s reload```参数可以使运行中的Nginx服务重新加载nginx.conf文件。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -s reload
{% endhighlight %}

事实上，Nginx会首先检查新的配置项是否有误，如果全部正确就以“优雅”的方式关闭。在重新启动Nginx来实现这个目的。类似的，```-s```是发送信号，仍然可以用kill命令发送```HUP```信号来达到相同的效果。
{% highlight string %}
# kill -s SIGHUP <nginx master pid>
{% endhighlight %}

### 1.12 日志文件回滚
使用```-s reopen```参数可以重新打开日志文件，这样可以先把当前日志文件改名或转移到其他目录中进行备份，再重新打开时就会生成新的日志文件。这个功能使得日志文件不至于过大。例如：
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx -s reopen
{% endhighlight %}
当然也可以使用kill命令发送```USR1```信号来达到同样的效果。
{% highlight string %}
# kill -s SIGUSR1 <nginx master pid>
{% endhighlight %}

由于nginx是通过inode指向日志文件的，inode和文件名无关，所以即使把日志文件重命名，nginx还是将日志文件写入原文件，只有用上面的命令重新开启日志文件才能将日志写入新的日志文件。


### 1.13 平滑升级Nginx
当Nginx服务升级到新的版本时，必须要将旧的二进制文件Nginx替换掉，通常情况下这需要重启服务的，但是Nginx支持不重启服务来完成新版本的平滑升级。

升级时包括以下步骤：

1) 通知正在运行的旧版本Nginx准备升级，通过向master进程发送```USR2```信号可以达到目的。例如：
{% highlight string %}
# kill -s SIGUSR2 <nginx master pid>
{% endhighlight %}
这里我们执行后：
{% highlight string %}
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2599     1  0 08:41 ?        00:00:00 nginx: master process ./nginx
nobody    2600  2599  0 08:41 ?        00:00:00 nginx: worker process
root      2603  1906  0 08:41 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# ls
client_body_temp        html                nginx               scgi_params.default
fastcgi.conf            koi-utf             nginx.conf          scgi_temp
fastcgi.conf.default    koi-win             nginx.conf.default  uwsgi_params
fastcgi_params          logs                nginx.pid           uwsgi_params.default
fastcgi_params.default  mime.types          proxy_temp          uwsgi_temp
fastcgi_temp            mime.types.default  scgi_params         win-utf

root@ubuntu:/usr/local/nginx# kill -s SIGUSR2 2599
root@ubuntu:/usr/local/nginx# ls
client_body_temp        koi-utf             nginx.conf.default   uwsgi_params
fastcgi.conf            koi-win             nginx.pid            uwsgi_params.default
fastcgi.conf.default    logs                nginx.pid.oldbin     uwsgi_temp
fastcgi_params          mime.types          proxy_temp           win-utf
fastcgi_params.default  mime.types.default  scgi_params
fastcgi_temp            nginx               scgi_params.default
html                    nginx.conf          scgi_temp
{% endhighlight %}
这时，我们看到运行中的Nginx会将pid文件重命名，如将/usr/local/nginx/nginx.pid重命名为/usr/local/nginx/nginx.pid.oldbin, 这样新的Nginx才有可能启动成功。

2) 启动新版本的Nginx，可以使用以上介绍过的任意一种启动方法。这时可以发现新旧版本的Nginx在同时运行。
{% highlight string %}
root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2631     1  0 08:49 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    2632  2631  0 08:49 ?        00:00:00 nginx: worker process
root      2634  1906  0 08:49 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# kill -s SIGUSR2 2631
root@ubuntu:/usr/local/nginx# ls
client_body_temp        koi-utf             nginx.conf.default   uwsgi_params
fastcgi.conf            koi-win             nginx.pid            uwsgi_params.default
fastcgi.conf.default    logs                nginx.pid.oldbin     uwsgi_temp
fastcgi_params          mime.types          proxy_temp           win-utf
fastcgi_params.default  mime.types.default  scgi_params
fastcgi_temp            nginx               scgi_params.default
html                    nginx.conf          scgi_temp

root@ubuntu:/usr/local/nginx# /usr/local/nginx/nginx
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] bind() to 0.0.0.0:80 failed (98: Address already in use)
nginx: [emerg] still could not bind()
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2631     1  0 08:49 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    2632  2631  0 08:49 ?        00:00:00 nginx: worker process
root      2635  2631  0 08:49 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    2636  2635  0 08:49 ?        00:00:00 nginx: worker process
root      2640  1906  0 08:50 pts/8    00:00:00 grep --color=auto nginx
{% endhighlight %}

3) 通过kill命令向旧版本的master进程发送SIGQUIT信号，以“优雅”的方式关闭旧版本的Nginx。随后将只有新版本Nginx服务运行，此时平滑升级完毕。
{% highlight string %}
root@ubuntu:/usr/local/nginx# kill -s SIGQUIT 2631
root@ubuntu:/usr/local/nginx# ps -ef | grep nginx
root      2635     1  0 08:49 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    2636  2635  0 08:49 ?        00:00:00 nginx: worker process
root      2642  1906  0 08:51 pts/8    00:00:00 grep --color=auto nginx
root@ubuntu:/usr/local/nginx# ls
client_body_temp        html                nginx               scgi_params.default
fastcgi.conf            koi-utf             nginx.conf          scgi_temp
fastcgi.conf.default    koi-win             nginx.conf.default  uwsgi_params
fastcgi_params          logs                nginx.pid           uwsgi_params.default
fastcgi_params.default  mime.types          proxy_temp          uwsgi_temp
fastcgi_temp            mime.types.default  scgi_params         win-utf
{% endhighlight %}

### 1.14 显示命令行帮助
使用```-h```或者```-?```参数会显示支持的所有命令行参数。
{% highlight string %}
root@ubuntu:/usr/local/nginx# ./nginx -?
nginx version: nginx/1.10.3
Usage: nginx [-?hvVtTq] [-s signal] [-c filename] [-p prefix] [-g directives]

Options:
  -?,-h         : this help
  -v            : show version and exit
  -V            : show version and configure options then exit
  -t            : test configuration and exit
  -T            : test configuration, dump it and exit
  -q            : suppress non-error messages during configuration testing
  -s signal     : send signal to a master process: stop, quit, reopen, reload
  -p prefix     : set prefix path (default: /usr/local/nginx/)
  -c filename   : set configuration file (default: /usr/local/nginx/nginx.conf)
  -g directives : set global directives out of configuration file
{% endhighlight %}



<br />
<br />
<br />

