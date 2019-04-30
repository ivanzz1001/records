---
layout: post
title: Nginx支持https
tags:
- nginx
categories: nginx
description: Nginx支持https
---

本文主要讲述一下nginx对https的支持及相关的配置（本文针对CentOS7.4操作系统)。


<!-- more -->



## 1. 编译支持https nginx，并运行
这里因为前面我们讲述了nginx的编译（Ubuntu12.04)，这里虽然系统不一样，但是差异不大。
<pre>
# wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.40.tar.gz
# tar -zxvf pcre-8.40.tar.gz

# wget http://zlib.net/zlib-1.2.11.tar.gz
# tar -zxvf zlib-1.2.11.tar.gz

# yum install openssl openssl-devel      //头文件默认安装在/usr/include/openssl目录下

# wget http://nginx.org/download/nginx-1.10.3.tar.gz
# tar -zxvf nginx-1.10.3.tar.gz
# cd nginx-1.10.3/

# ./configure \
--prefix=/usr/local/nginx \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11 \
--with-http_gzip_static_module \
--with-http_stub_status_module \
--with-http_sub_module

# make
# make install

# /usr/local/nginx/sbin/nginx
# ps -aux | grep nginx | grep -v grep
root     103552  0.0  0.0  45996  1120 ?        Ss   17:29   0:00 nginx: master process /usr/local/nginx/sbin/nginx
nobody   103553  0.0  0.0  48520  1964 ?        S    17:29   0:00 nginx: worker process

# /usr/local/nginx/sbin/nginx -s stop
</pre>
这里要支持https，则必须添加```--with-http_ssl_module```; 另外这里还添加了```sub_status```模块，用于查看相应的访问记录。

## 2. 生成自签名证书
<pre>
# openssl req -newkey rsa:2048 -nodes -keyout rsa_private.key -x509 -days 365 -out cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test_name/emailAddress=11111111@qq.com"

# mkdir /usr/local/nginx/conf/openssl
# cp cert.crt rsa_private.key /usr/local/nginx/conf/openssl
</pre>
说明：加载SSL支持的Nginx并使用上述私钥时除去必须的口令。


## 3. 修改nginx配置文件

这里我们修改nginx.conf配置文件，使其支持https：
{% highlight string %}

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
        listen       443 ssl;
        ssl_certificate openssl/cert.crt;
        ssl_certificate_key openssl/rsa_private.key;
        ssl_session_timeout 5m;

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

        index  index.html index.htm;
}
{% endhighlight %}
注意上面我们如果要在同一个```server```块中同时配置支持http与https，则不能将```ssl on```写在配置块中, ```ssl```必须加在443后边。这里我们用http范文没有任何问题，但是当客户端采用https访问时，出现```不安全```的报错。

* 对于IE9浏览器，此时需要将该证书导入到```受信任的根证书颁发机构```，然后可以正常访问。

* 对于chrome浏览器，导入证书到```受信任的根证书颁发机构```，仍然会提示```不安全```，仍不能正常访问（貌似只支持域名证书，不支持IP地址证书)

* 对于360浏览器，此时需要将该证书导入到```受信任的根证书颁发机构```，然后可以正常访问。

* 使用curl命令访问
{% highlight string %}
# curl -X GET https://192.168.69.128 --cacert /usr/local/nginx/conf/openssl/cert.crt
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
{% endhighlight %}

## 4. nginx http代理的配置

如下我们配置一个nginx http代理：
{% highlight string %}
[root@localhost conf]# cat nginx.conf

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

    upstream uicps {
        server www.163.com;
    }


    server {
        listen       80;

        server_name  localhost;

        #charset koi8-r;

        #access_log  logs/host.access.log  main;

        location / {
            root   html;
            index  index.html index.htm;
        }
        
        location /ups {
            proxy_pass http://uicps;
            
            #Proxy Settings  
            proxy_redirect     off; 
            proxy_set_header   Host     www.163.com;     
            proxy_set_header   X-Real-IP        $remote_addr;  
            proxy_set_header   X-Forwarded-For  $proxy_add_x_forwarded_for;  
            proxy_next_upstream error timeout invalid_header http_500 http_502 http_503 http_504;  
            proxy_max_temp_file_size 0;
            proxy_connect_timeout      90;  
            proxy_send_timeout         90;  
            proxy_read_timeout         90;  
            proxy_buffer_size          4k;  
            proxy_buffers              4 32k;  
            proxy_busy_buffers_size    64k;
            proxy_temp_file_write_size 64k;  
        }

        #error_page  404              /404.html;

        # redirect server error pages to the static page /50x.html
        #
        error_page   500 502 503 504  /50x.html;
        location = /50x.html {
            root   html;
        }
    }

}
{% endhighlight %}







<pre>
注意： 上面我们为了测试，将自签名的证书导入到了"受信任的根证书颁发机构"，用完之后，请注意删除掉该证书。
</pre>



<br />
<br />

**[参看]:**

1. [http module参看](http://nginx.org/en/docs/)

2. [Nginx配置同一个域名http与https两种方式都可访问](https://www.cnblogs.com/fjping0606/p/6006552.html)

3. [添加自签发的 SSL 证书为受信任的根证书](http://cnzhx.net/blog/self-signed-certificate-as-trusted-root-ca-in-windows/)

4. [nginx的https和http共存反向代理配置](https://www.cnblogs.com/madyina/p/7735545.html)


<br />
<br />
<br />

