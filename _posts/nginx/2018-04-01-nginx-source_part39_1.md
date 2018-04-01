---
layout: post
title: core/ngx_file.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中对所涉及到的文件操作的封装。


<!-- more -->

## 1. ngx_file_s数据结构
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILE_H_INCLUDED_
#define _NGX_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_file_s {
    ngx_fd_t                   fd;
    ngx_str_t                  name;
    ngx_file_info_t            info;

    off_t                      offset;
    off_t                      sys_offset;

    ngx_log_t                 *log;

#if (NGX_THREADS)
    ngx_int_t                (*thread_handler)(ngx_thread_task_t *task,
                                               ngx_file_t *file);
    void                      *thread_ctx;
    ngx_thread_task_t         *thread_task;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_event_aio_t           *aio;
#endif

    unsigned                   valid_info:1;
    unsigned                   directio:1;
};
{% endhighlight %}
ngx_file_s数据结构是对打开的文件的一个封装：

* ```fd```: 所打开文件对应的句柄

* ```name```: 所打开文件的名称

* ```info```: 所打开文件所对应的struct stat信息

* ```offset```: 当前要操作的文件指针偏移

* ```sys_offset```: 当前该打开文件的实际指针偏移

* ```log```: 该打开文件所对应的log

* ```thread_handler/thread_ctx/thread_task```: 当前我们并不支持多线程操作文件，这里暂不介绍

* ```aio```: 这里我们暂不支持```NGX_HAVE_FILE_AIO```

* ```valid_info```: 本字段暂时未能用到

* ```directio```: 对于当前文件是否使用directio

## 2. ngx_path_t相关数据结构
{% highlight string %}
#define NGX_MAX_PATH_LEVEL  3


typedef time_t (*ngx_path_manager_pt) (void *data);
typedef void (*ngx_path_loader_pt) (void *data);


typedef struct {
    ngx_str_t                  name;        //文件名
    size_t                     len;         //level中3个数据之和
    size_t                     level[3];    //每一层子目录的长度

    //以下字段只在文件缓冲管理模块被使用
    ngx_path_manager_pt        manager;     //文件缓冲管理回调     
    ngx_path_loader_pt         loader;      //文件缓冲loader回调
    void                      *data;        //回调上下文

    u_char                    *conf_file;   //所关联的配置文件名称
    ngx_uint_t                 line;        //所在配置文件的行数
} ngx_path_t;


typedef struct {
    ngx_str_t                  name;
    size_t                     level[3];
} ngx_path_init_t;
{% endhighlight %}
```ngx_path_t```是一个目录对象结构:

![ngx-path-t](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_path_t.jpg)

上图是nginx创建的临时文件结构。nginx在根目录下最多创建3层目录结构。例如：
<pre>
/home/nginx/tmpfile/9/32/198
</pre>
其中`home/nginx/tmpfile```为根目录，```9/32/198```则为3层子目录结构。

```ngx_path_t```是用于初始时候的一个路径。

## 3. ngx_temp_file_t数据结构
{% highlight string %}
typedef struct {
    ngx_file_t                 file;        //文件结构
    off_t                      offset;      //当前操作偏移
    ngx_path_t                *path;        //临时文件所对应的路径
    ngx_pool_t                *pool;        //所对应的缓冲池
    char                      *warn;        //岁对应的警告信息

    ngx_uint_t                 access;      //临时文件的访问权限

    unsigned                   log_level:8;  //所对应的日志级别
    unsigned                   persistent:1;  //是否需要对当前临时文件进行持久化
    unsigned                   clean:1;       //退出时，临时文件是否要删除
    unsigned                   thread_write:1; //是否开启多线程写
} ngx_temp_file_t;
{% endhighlight %}

```ngx_temp_file_t```代表一个临时文件结构。

## 4. ngx_ext_rename_file_t数据结构
{% highlight string %}
typedef struct {
    ngx_uint_t                 access;        //文件访问权限
    ngx_uint_t                 path_access;   //路径访问权限
    time_t                     time;          //重命名时间
    ngx_fd_t                   fd;            //对应的文件句柄

    unsigned                   create_path:1;  //是否创建路径
    unsigned                   delete_file:1;  //如果重命名失败，是否要删除文件

    ngx_log_t                 *log;            //所对应的日志
} ngx_ext_rename_file_t;

{% endhighlight %}

## 5. ngx_copy_file_t数据结构
{% highlight string %}
typedef struct {
    off_t                      size;           //要拷贝的文件大小
    size_t                     buf_size;       //拷贝时新开临时缓冲区大小

    ngx_uint_t                 access;         //新拷贝文件的访问权限
    time_t                     time;           //新拷贝文件的最近访问时间和最近修改时间

    ngx_log_t                 *log;
} ngx_copy_file_t;
{% endhighlight %}


## 6. ngx_tree_ctx_s数据结构
{% highlight string %}
typedef struct ngx_tree_ctx_s  ngx_tree_ctx_t;

typedef ngx_int_t (*ngx_tree_init_handler_pt) (void *ctx, void *prev);
typedef ngx_int_t (*ngx_tree_handler_pt) (ngx_tree_ctx_t *ctx, ngx_str_t *name);

struct ngx_tree_ctx_s {
    off_t                      size;
    off_t                      fs_size;
    ngx_uint_t                 access;
    time_t                     mtime;

    ngx_tree_init_handler_pt   init_handler;
    ngx_tree_handler_pt        file_handler;
    ngx_tree_handler_pt        pre_tree_handler;
    ngx_tree_handler_pt        post_tree_handler;
    ngx_tree_handler_pt        spec_handler;

    void                      *data;
    size_t                     alloc;

    ngx_log_t                 *log;
};

{% endhighlight %}

<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

