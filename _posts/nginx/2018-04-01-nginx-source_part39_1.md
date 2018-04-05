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
重命名文件所对应相关数据结构

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

这里是nginx遍历目录的相关数据结构，下面我们简要介绍一下其中的各个字段：

* ```size```: 遍历到的文件的大小

* ```fs_size```: 指的是遍历到的文件所占磁盘块数目乘以512的值与```size```中的最大值，即fs_size = ngx_max(size,st_blocks*512)

* ```access```: 指的是遍历到的文件的访问权限

* ```mtime```: 指的是遍历到的文件上次被修改的时间

* ```init_handler```: 与下面的```data```、```alloc```字段相关，用于初始化遍历过程中的相关数据结构。一般如果```alloc```字段不为0的话，表示要分配alloc大小的空间，此时会调用init_handler回调函数初始化。

* ```file_handler```: 处理普通文件的回调函数

* ```pre_tree_handler```: 进入一个目录前的回调函数

* ```post_tree_handler```: 离开一个目录后的回调函数

* ```spec_handler```: 处理特殊文件的回调函数，比如socket、FIFO等

* ```data```: 传递一些数据结构，可以在不同的目录下使用相同的数据结构，或者也可以重新分配，前提是alloc不为0

* ```alloc```: 如果需要分配一些数据结构，这里指定分配数据结构的大小，并由```init_handler```进行初始化

* ```log```: 主要用于日志的记录


## 7. 相关函数声明
{% highlight string %}
//获得某个文件的全路径
ngx_int_t ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix,
    ngx_str_t *name);


//将buffer chain写入到temp文件
ssize_t ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain);

//创建temp文件
ngx_int_t ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path,
    ngx_pool_t *pool, ngx_uint_t persistent, ngx_uint_t clean,
    ngx_uint_t access);

//创建hash文件名
void ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len);

//创建路径
ngx_int_t ngx_create_path(ngx_file_t *file, ngx_path_t *path);

//创建全路径
ngx_err_t ngx_create_full_path(u_char *dir, ngx_uint_t access);

//添加路径到cf->cycle->paths数组中
ngx_int_t ngx_add_path(ngx_conf_t *cf, ngx_path_t **slot);

//创建cycle->paths数组中的所有路径
ngx_int_t ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user);

//重命名文件
ngx_int_t ngx_ext_rename_file(ngx_str_t *src, ngx_str_t *to,
    ngx_ext_rename_file_t *ext);

//拷贝文件
ngx_int_t ngx_copy_file(u_char *from, u_char *to, ngx_copy_file_t *cf);

//遍历某一目录下(tree目录)的文件
ngx_int_t ngx_walk_tree(ngx_tree_ctx_t *ctx, ngx_str_t *tree);


//获得下一个临时number值
ngx_atomic_uint_t ngx_next_temp_number(ngx_uint_t collision);


//设置某一模块中的路径指令
char *ngx_conf_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

//合并路径值
char *ngx_conf_merge_path_value(ngx_conf_t *cf, ngx_path_t **path,
    ngx_path_t *prev, ngx_path_init_t *init);

//设置某一个文件的所有者、所属组、其他人的访问权限
char *ngx_conf_set_access_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
{% endhighlight %}



## 8. 相关变量声明
{% highlight string %}
//全局的临时值变量，主要用于产生临时文件名使用
extern ngx_atomic_t      *ngx_temp_number;

//nginx内部维持的一个随机值变量
extern ngx_atomic_int_t   ngx_random_number;
{% endhighlight %}

<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

