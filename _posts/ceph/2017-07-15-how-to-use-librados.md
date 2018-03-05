---
layout: post
title: librados相关介绍
tags:
- ceph
- librados
categories: ceph
description: librados相关介绍及使用范例
---

本文简单介绍librados，并给出相应的使用范例。参看：http://docs.ceph.com/docs/master/rados/api/librados-intro/

<!-- more -->



## 1. librados介绍
ceph存储集群提供的基本存储服务，使ceph能够在一个联合文件系统中独一无二的实现对象存储(object storage)、块存储(block storage)以及文件存储(file storage)。然而，你并不一定需要通过RESTful API、block API或者POSIX文件系统接口才能访问ceph存储集群。依赖于RADOS，你可以直接通过librados API来访问ceph存储集群。

librados API可以访问ceph存储集群的两类守护进程：
* Ceph Monitor: 维持cluster map的一份主拷贝
* Ceph OSD Daemon: 在存储节点上存储对象数据

![librados](https://ivanzz1001.github.io/records/assets/img/ceph/rados/librados.jpg)




## 2. 安装librados开发环境 
客户端应用程序必须通过```librados```来连接到ceph存储集群。在你使用```librados```的时候必须安装librados及其依赖库。 librados API由C++写成，但是也提供了C、python、java、PHP接口。这里我们重点介绍一下C/C++接口：

对于Debian/Unbuntu系统：
{% highlight string %}
sudo apt-get install librados-dev
{% endhighlight %}

对于RHEL/Centos系统：
{% highlight string %}
sudo yum install librados2-devel
{% endhighlight %}

安装之后可以在/usr/include/rados目录下找到。


## 3. 配置一个cluster handler 
通过librados创建的Ceph Client可以直接和OSD来进行交互来存取数据。要想和OSD进行交互，client app必须要调用librados并且连接到Ceph Monitor。一旦连接成功，librados就可以从Ceph Monitor处获得Cluster Map。当client app想要读写数据的时候，其需要创建一个IO context并且绑定到一个pool上。pool是和rule相关联的，其定义了如何将数据存入集群。通过IO Context，client向librados提供object名称，librados采用该名称及获取到的cluster map就能够计算出需要将数据存放到哪个PG和OSD。client app并不需要直接的了解到集群的拓扑结构：
![librados](https://ivanzz1001.github.io/records/assets/img/ceph/rados/rados-rw.png)


ceph storage cluster handle封装了客户端的配置，这包括：
* user ID(针对rados_create())或user name(针对rados_create2())
* cephx身份认证的key
* monitor ID和IP地址
* 日志级别
* 调试级别

这样要使用librados的步骤就是：1） 创建cluster handler 2）使用cluster handler连接到存储集群。 要想连接到集群，app必须提供monitor IP，username和身份认证的key(假如启用了cephx的话）

<pre>
NOTE: 与不同的ceph存储集群交互，或以不同的用户名和同一个ceph存储集群交互都需要创建不同的cluster handler。
</pre>

rados提供了多种方式来设置这些值。对于monitor及身份认证的key的设置，一种简单的方法就是将它们写在配置文件中。配置文件中包含至少一个monitor IP及keyring文件路径。例如：
{% highlight string %}
[global]
mon host = 192.168.1.1
keyring = /etc/ceph/ceph.client.admin.keyring
{% endhighlight %}

一旦你成功创建handle，你就可以读取该配置文件来配置handle。你也可以通过向你的app client传递参数然后对应的函数来解析命令行参数（例如：rados_conf_parse_argv())，或者解析ceph环境变量（例如：rados_conf_parse_env())的方式来配置handle。下图提供了初始化连接的一个高层视图：

![initial-conn](https://ivanzz1001.github.io/records/assets/img/ceph/rados/rados-initial-conn.png)

一旦handle连接成功，你就可以采用该handle来操作整个ceph集群了：
* 获得集群数据信息
* 使用pool相关操作（exists,create,list,delete等）
* 获取或设置配置


如下给出一个C版本的例子：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>

int main (int argc, const char **argv)
{

        /* Declare the cluster handle and required arguments. */
        rados_t cluster;
        char cluster_name[] = "ceph";
        char user_name[] = "client.admin";
        uint64_t flags;

        /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
        int err;
        err = rados_create2(&cluster, cluster_name, user_name, flags);

        if (err < 0) {
                fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated a cluster handle.\n");
        }


        /* Read a Ceph configuration file to configure the cluster handle. */
        err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
        if (err < 0) {
                fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the config file.\n");
        }

        /* Read command line arguments */
        err = rados_conf_parse_argv(cluster, argc, argv);
        if (err < 0) {
                fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the command line arguments.\n");
        }

        /* Connect to the cluster */
        err = rados_connect(cluster);
        if (err < 0) {
                fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nConnected to the cluster.\n");
        }

        rados_ioctx_t io;
        char *poolname = "data";

        err = rados_ioctx_create(cluster, poolname, &io);
        if (err < 0) {
                fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_shutdown(cluster);
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated I/O context.\n");
        }

        /* Write data to the cluster synchronously. */
        err = rados_write(io, "hw", "Hello World!", 12, 0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write object \"hw\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote \"Hello World\" to object \"hw\".\n");
        }

        char xattr[] = "en_US";
        err = rados_setxattr(io, "hw", "lang", xattr, 5);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write xattr to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote \"en_US\" to xattr \"lang\" for object \"hw\".\n");
        }

        /*
         * Read data from the cluster asynchronously.
         * First, set up asynchronous I/O completion.
         */
        rados_completion_t comp;
        err = rados_aio_create_completion(NULL, NULL, NULL, &comp);
        if (err < 0) {
                fprintf(stderr, "%s: Could not create aio completion: %s\n", argv[0], strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nCreated AIO completion.\n");
        }

        /* Next, read data using rados_aio_read. */
        char read_res[100];
        err = rados_aio_read(io, "hw", comp, read_res, 12, 0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read object. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead object \"hw\". The contents are:\n %s \n", read_res);
        }

        /* Wait for the operation to complete */
        rados_aio_wait_for_complete(comp);

        /* Release the asynchronous I/O complete handle to avoid memory leaks. */
        rados_aio_release(comp);


        char xattr_res[100];
        err = rados_getxattr(io, "hw", "lang", xattr_res, 5);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read xattr. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead xattr \"lang\" for object \"hw\". The contents are:\n %s \n", xattr_res);
        }

        err = rados_rmxattr(io, "hw", "lang");
        if (err < 0) {
                fprintf(stderr, "%s: Cannot remove xattr. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRemoved xattr \"lang\" for object \"hw\".\n");
        }

        err = rados_remove(io, "hw");
        if (err < 0) {
                fprintf(stderr, "%s: Cannot remove object. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRemoved object \"hw\".\n");
        }

}

{% endhighlight %}

采用如下命令进行编译：
<pre>
gcc ceph-client.c -lrados -o ceph-client
</pre>


## 4. 创建一个IO context 
一旦app client已经有了cluster handle并且连接上了ceph存储集群，你就可以创建一个IO Context，然后开始读写数据。IO Context将连接绑定到一个特定的pool。用户必须有适当的```CAPS```权限来访问该pool。IO Context功能包括：
* 读写数据和extended属性
* 遍历出对象和extended属性
* pools快照，列出快照

![rados-rw-2](https://ivanzz1001.github.io/records/assets/img/ceph/rados/rados-rw-2.png)

rados同时支持同步/异步访问集群。一旦创建了IO Context之后，只需要通过object/xatrr名称即可进行读写操作。封装在librados中的CRUSH算法使用crush map选择合适的OSD进行操作。 OSD Daemons负责复制数据的副本。

如下的例子采用默认的```data```存储池来进行演示。对于写操作，我们采用synchronous模式；对于读操作，我们采用asynchronous模式。
<pre>
Note: 与上一个例子不同，我们这里还演示了通过解析命令行参数的方式来配置handle。rados_conf_parse_argv()函数使用起来有些怪异，这里特意给出例子。
</pre>

请参看如下代码：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rados/librados.h>

int main (int argc, const char **argv)
{

        /* Declare the cluster handle and required arguments. */
        rados_t cluster;
        char cluster_name[] = "ceph";
        char user_name[] = "client.admin";
        uint64_t flags;

        /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
        int err;
        err = rados_create2(&cluster, cluster_name, user_name, flags);

        if (err < 0) {
                fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated a cluster handle.\n");
        }


        /* Read a Ceph configuration file to configure the cluster handle. */
#if 0
        err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
        if (err < 0) {
                fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the config file.\n");
        }
#else
#if 1
       const char *cluster_args[]={
            argv[0],
            "--mon_host",
             "172.20.30.182,172.20.30.183,172.20.30.184",
             "--keyring",
             "admin.keyring"
        };
	err = rados_conf_parse_argv(cluster,sizeof(cluster_args)/sizeof(const char *),cluster_args);
        if (err < 0){
		fprintf(stderr,"%s: parse cluster args failure: %s\n",argv[0],strerror(-err));
		exit(EXIT_FAILURE);
        }else{
		printf("\nParse the cluster args.\n");
        }
#endif
#endif


        /* Read command line arguments */
        err = rados_conf_parse_argv(cluster, argc, argv);
        if (err < 0) {
                fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the command line arguments.\n");
        }

        /* Connect to the cluster */
        err = rados_connect(cluster);
        if (err < 0) {
                fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nConnected to the cluster.\n");
        }

        rados_ioctx_t io;
        char *poolname = "data";

        err = rados_ioctx_create(cluster, poolname, &io);
        if (err < 0) {
                fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_shutdown(cluster);
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated I/O context.\n");
        }

        /* Write data to the cluster synchronously. */
        err = rados_write(io, "hw", "Hello World!", 12, 0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write object \"hw\" to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote \"Hello World\" to object \"hw\".\n");
        }

        char xattr[] = "en_US";
        err = rados_setxattr(io, "hw", "lang", xattr, 5);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot write xattr to pool %s: %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nWrote \"en_US\" to xattr \"lang\" for object \"hw\".\n");
        }

        /*
         * Read data from the cluster asynchronously.
         * First, set up asynchronous I/O completion.
         */
        rados_completion_t comp;
        err = rados_aio_create_completion(NULL, NULL, NULL, &comp);
        if (err < 0) {
                fprintf(stderr, "%s: Could not create aio completion: %s\n", argv[0], strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nCreated AIO completion.\n");
        }

        /* Next, read data using rados_aio_read. */
        char read_res[100];
        err = rados_aio_read(io, "hw", comp, read_res, 12, 0);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read object. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead object \"hw\". The contents are:\n %s \n", read_res);
        }

        /* Wait for the operation to complete */
        rados_aio_wait_for_complete(comp);

        /* Release the asynchronous I/O complete handle to avoid memory leaks. */
        rados_aio_release(comp);


        char xattr_res[100];
        err = rados_getxattr(io, "hw", "lang", xattr_res, 5);
        if (err < 0) {
                fprintf(stderr, "%s: Cannot read xattr. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRead xattr \"lang\" for object \"hw\". The contents are:\n %s \n", xattr_res);
        }

        err = rados_rmxattr(io, "hw", "lang");
        if (err < 0) {
                fprintf(stderr, "%s: Cannot remove xattr. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRemoved xattr \"lang\" for object \"hw\".\n");
        }

        err = rados_remove(io, "hw");
        if (err < 0) {
                fprintf(stderr, "%s: Cannot remove object. %s %s\n", argv[0], poolname, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        } else {
                printf("\nRemoved object \"hw\".\n");
        }
	
        rados_ioctx_destroy(io);
	rados_shutdown(cluster);

}
{% endhighlight %}

## 关闭sessions 

一旦app client完成了相关操作，用户需要关闭连接和handle。针对asynchronous IO，用户也应确保该异步操作已经完成。
{% highlight string %}
rados_ioctx_destroy(io);
rados_shutdown(cluster);
{% endhighlight %}

<br />
<br />
<br />