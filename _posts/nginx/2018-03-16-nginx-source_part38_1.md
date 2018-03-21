---
layout: post
title: core/ngx_cycle.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx运行的一个总控型数据结构及相关操作函数。

<!-- more -->


## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2
{% endhighlight %}

下面我们对上面几个宏定义进行简单的说明：

* **NGX_CYCLE_POOL_SIZE**: 这里定义nginx cycle所关联的pool大小，默认值```NGX_DEFAULT_POOL_SIZE```，即16KB（该变量定义在core/palloc.h头文件中)

* **NGX_DEBUG_POINTS_STOP**: 定义程序在执行到一些关键错误点时，产生```SIGSTOP```信号。

* **NGX_DEBUG_POINTS_ABORT**: 定义程序在执行到一些关键错误点时，执行abort()函数。

<pre>
可以通过在nginx.conf配置文件使用debug_points指令，指定在一些关键错误点处的行为。
</pre>


## 2. ngx_shm_zone_t数据结构
{% highlight string %}
typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};
{% endhighlight %}
```ngx_shm_zone_s```代表一块内存共享区域。下面我们对其中各个字段做一个简要的说明：

* **data**: 这里```data```可以指向自定义的一个数据结构，主要是为了在数据初始化的时候用到，或通过共享内存直接拿到与共享内存相关的数据，它不一定指向共享内存中的地址。

* **shm**: 实际的共享内存结构

* **init**: 共享内存初始化函数

* **tag**: 这里tag是一个标志，区别于shm.name。 shm.name没法让nginx区分到底是想新创建一个共享内存，还是使用已存在的旧的共享内存，因此这里引入tag字段来解决该问题。tag一般指向当前模块的```ngx_module_t```变量，例如：
<pre>
ngx_shared_memory_add(cf, &value[1], 0, &ngx_http_fastcgi_module);
</pre>

这里再对tag字段解释一下，因为看上去它和name字段有点重复，而事实上，name字段主要用作共享内存的唯一标识，它能让nginx知道我想使用哪个共享内存，但它没办法让nginx区分到底是想新创建一个共享内存，还是使用那个已存在的旧的共享内存。举个例子，模块A创建了共享内存sa,模块A或者另外一个模块B再以同样的名称sa去获取共享内存，那么此时nginx是返回模块A已创建的那个共享内存sa给模块A/模块B，还是直接以共享内存名重复提示模块A/模块B出错呢？ 不管nginx采用哪种做法都有另外一种情况出错，所以新增一个tag字段做冲突标识，该字段一般指向当前模块的 ```ngx_module_t```变量即可。这样在上面的例子中，通过tag字段的帮助，如果模块A/模块B再以同样的名称去获取模块A已创建的共享内存sa，则模块A将获得它之前创建的共享内存的引用（因为模块A前后两次请求的tag相同），而模块B将获得共享内存已作他用的错误提示（因为模块B请求的tag与之前模块A请求的tag不同）。

* **noreuse**: 取值为0时，则表示可以对此共享内存进行复用；否则不能对此共享内存进行复用。一般用在系统升级时，表示是否可以复用前面创建的共享内存。

## 3. ngx_cycle_s数据结构
{% highlight string %}
struct ngx_cycle_s {
    void                  ****conf_ctx;
    ngx_pool_t               *pool;

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;
    ngx_connection_t         *free_connections;
    ngx_uint_t                free_connection_n;

    ngx_module_t            **modules;
    ngx_uint_t                modules_n;
    ngx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    ngx_queue_t               reusable_connections_queue;

    ngx_array_t               listening;
    ngx_array_t               paths;
    ngx_array_t               config_dump;
    ngx_list_t                open_files;
    ngx_list_t                shared_memory;

    ngx_uint_t                connection_n;
    ngx_uint_t                files_n;

    ngx_connection_t         *connections;
    ngx_event_t              *read_events;
    ngx_event_t              *write_events;

    ngx_cycle_t              *old_cycle;

    ngx_str_t                 conf_file;
    ngx_str_t                 conf_param;
    ngx_str_t                 conf_prefix;
    ngx_str_t                 prefix;
    ngx_str_t                 lock_file;
    ngx_str_t                 hostname;
};
{% endhighlight %}
```ngx_cycle_s```是一个总控型数据结构。一个```cycle```对象存放着从某个配置创建而来的nginx运行时上下文。我们可以通过全局变量```ngx_cycle```来引用到当前进程的cycle上下文（对于worker进程，在创建时也会继承得到该上下文）。每一次重新加载nginx配置文件时，都会从该配置文件重新创建出一个新的cycle对象；而原来老的cycle对象则会在新的cycle成功创建之后被删除掉。

一个cycle对象通常是由```ngx_init_cycle()```函数所创建，该函数以一个```previous cycle```作为其参数。函数首先会定位到```previous cycle```的配置文件，然后尽可能的从previous cycle继承相应的资源。在nginx启动时，首先会创建一个```init_cycle```占位符，然后会用一个从配置文件创建而来的cycle来替换该```init_cycle```。

下面我们再针对各个字段，做一个简要的介绍：

* **conf_ctx**: 本字段是一个4级指针结构，实际使用时可能当做2级指针来用，也可能当做4级指针来用。下面给出一个```conf_ctx```指针的一个大体结构：
![ngx-conf-ctx](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_conf_ctx.jpg)

* **pool**: 本cycle所关联的内存池对象。针对每一个新的cycle对象，都会创建一个相应的内存池对象。

* **log**: 本cycle所关联的日志对象。初始时，会从原来老的cycle继承而来；而当配置文件成功读取完成之后，```log```指针会指向```new_log```。

* **new_log**: 本cycle所关联的日志对象，在读取完配置文件时进行创建。其受到nginx配置文件一级配置(root-scope)error_log指令的影响。

* **log_use_stderr**: 主要用于指示当前配置文件中error_log指令的日志输出是否配置为stderr。```error_log```指令用法如下：
<pre>
Syntax:	error_log file [level];
Default:	error_log logs/error.log error;
Context:	main, http, mail, stream, server, location
</pre>

* **files**： 预先建立的```ngx_connection_t```类型的指针数组。每当一个连接建立，就从free_connections中获取到一个空闲的```ngx_connection_t```对象，然后将该对象存放到files对应的索引处（根据连接句柄fd）

* **free_connections**: 当前处于空闲状态的```ngx_connection_t```类型对象。假如当前并没有空闲连接的话，nginx工作进程会拒绝接受新的客户端连接或者连接到upstream服务器。

* **free_connection_n**： 当前处于空想状态的```ngx_connection_t```类型对象的个数。

* **modules**: 指向当前nginx内部的所有module。包括objs/ngx_modules.c中定义的modules，以及后面动态加载的modules.

* **modules_n**： 当前总的module的个数

* **modules_used**: 指示modules是否加载完成，处于使用当中。如果是，则会禁止加载。主要是为了防止在使用过程中在错误的时间加载动态模块。


* **reusable_connections_queue**: 可复用连接队列。（一般对于http长连接会设置为可复用。在当前连接紧张时，nginx会自动释放掉一些本队列中的连接，以用于接受更多http短连接）

* **listening**: 监听端口数组，元素类型为```ngx_listening_t```类型。通常在遇到不同模块的```listen```指令时，会调用```ngx_create_listening()```函数来加载监听对象。监听socket会依据```ngx_listening_t```对象来创建。

* **paths**: 保存着nginx所有要操作的目录。路径会在相应的模块通过调用```ngx_add_path()```函数来进行添加。这些路径在读取nginx配置文件时，如果不存在的话则会被创建。对于每一个路径，可以关联两个handlers:
<pre>
1) path loader: 在nginx启动或重启的60s内执行一次。通常情况下，path loader会读取该目录，然后将读取到的数据存放在nginx共享内存中。
                本handler一般是由nginx专用进程“nginx cache loader”来调用

2) path manager: 会周期性的执行。通常情况下，该manager会移除对应目录下的一些过时文件，然后更新内存以反应相应的变化情况。该handler一般是由nginx
                 专用进程“nginx cache manager”来调用。
</pre>

* **config_dump**： 保存```ngx_conf_dump_t```对象的数组。在Nginx对配置文件进行检查时，会将读取到的配置文件拷贝到```ngx_conf_dump_t.buffer```,后续再dump出来。


* **open_files**: 元素类型为```ngx_open_file_t```的数组对象，通过```ngx_conf_open_file()```函数打开的文件都会存入该数组。当前，nginx采用该类型的打开文件来记录日志。在读取完配置文件之后，nginx会打开```open_files```列表中的所有文件，然后将每一个打开文件的句柄存放在```ngx_open_file_s.fd```中。文件是以追加方式打开的，而如果文件不存在还会进行创建。在Nginx的worker进程接收到reopen信号(通常是```USR1```信号）时，worker进程会对该列表中的文件进行重新打开，在这种情况下，fd域将会被改变成一个新的值。

* **shared_memory**: 共享内存列表，元素类型为```ngx_shm_zone_s```。通过ngx_shared_memory_add()函数将共享内存加到此列表中。共享内存在nginx的所有进程中都被映射到相同的地址，通常被用于共享一些常用的数据，例如HTTP内存缓存树。

* **connection_n**: nginx配置文件中的worker_connection指令指定```connection_n```的值，从而决定每一个worker进程所应该创建的```ngx_connection_t```对象的个数（注意这不是实际的连接数，只是预先创建的ngx_connection_t对象）。

* **files_n**: 当前上面```files```数组中实际的连接个数

* **connections**: 元素类型为```ngx_connection_t```的数组，通常在nginx worker进程初始化时由对应的事件模块所创建。

* **read_events**: 当前进程中的所有读事件对象。每个网络连接关联着一个读事件

* **write_events**: 当前进程中的所有写事件对象。每个网络连接关联着一个写事件

* **old_cycle**: 用于引用上一个```ngx_cycle_t```类型对象。例如ngx_init_cycle()方法，在启动初期，需要建立一个临时的ngx_cycle_t对象保存一些一些变量，在调用ngx_init_cycle()方法时，就可以把旧的ngx_cycle_t对象传进去。

* **conf_file**: 配置文件的存放路径

* **conf_param**： 存放nginx启动时，通过```-g```选项传递进来的参数

* **conf_prefix**: 存放nginx配置文件路径前缀（一般是通过```-p```选项来指定nginx的工作路径，然后使用该路径下的配置文件）

* **prefix**： nginx路径前缀（后续如日志，pid等都会参考该前缀）

* **lock_file**： nginx使用锁机制来实现```accept mutex```，并且顺序的来访问共享内存。在大多数的系统上，锁都是通过原子操作来实现的，因此会忽略配置文件中的```lock_file file```指令；而对于其他的一些系统，```lock file```机制会被使用。

* **hostname**: gethostname()得到的主机名称

这里对于connections、free_connections、files、read_events、write_events，我们给出如下一副图：

![ngx-cycle](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_get_connection.jpg)

## 4. ngx_core_conf_t数据结构
{% highlight string %}
typedef struct {
    ngx_flag_t                daemon;
    ngx_flag_t                master;

    ngx_msec_t                timer_resolution;

    ngx_int_t                 worker_processes;
    ngx_int_t                 debug_points;

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;
    ngx_uid_t                 user;
    ngx_gid_t                 group;

    ngx_str_t                 working_directory;
    ngx_str_t                 lock_file;

    ngx_str_t                 pid;
    ngx_str_t                 oldpid;

    ngx_array_t               env;
    char                    **environment;
} ngx_core_conf_t;
{% endhighlight %}

```ngx_core_conf_t```是nginx的一个核心配置，主要对应于配置文件的主上下文。下面我们对其中相关字段进行简单讲解：

**1) daemon字段**

```daemon```指令的配置语法如下：
<pre>
Syntax:	daemon on | off;
Default:	daemon on;
Context:	main
</pre>

用于决定nginx是否作为一个daemon进程运行。

**2) master字段**

```master```指令的配置语法如下：
<pre>
Syntax:	master_process on | off;
Default:	master_process on;
Context:	main
</pre>
用于决定nginx是否以master方式工作。

**3)  timer_resolution字段**

```timer_resolution```指令的配置语法如下：
<pre>
Syntax:	timer_resolution interval;
Default:	—
Context:	main
</pre>
在nginx worker进程中降低```timer resolution```的话，就可以减少```gettimeofday()```函数的系统调用。默认情况下，在每一次接收到内核事件的情况下都会调用一次```gettimeofday()```。而当指定了timer resolution的话，则每```interval```时间内会调用一次gettimeofday()。例如：
{% highlight string %}
timer_resolution 100ms;
{% endhighlight %}
而内部如何实现这样一个时间间隔，主要依赖于nginx所采用的事件方式：

* 假如所采用的事件模型是```kqueue```，则使用```EVENT_TIMER```过滤器来实现

* 假如所采用的事件模型是```eventport```，则使用```timer_create()```来实现

* 其他情况使用setitimer()函数来实现

**4) worker_processes字段**

```worker_processes```指令用于指定工作进程的数目。最佳的worker processes数目受很多因素的影响，包括但不限于CPU核数，存储数据的硬盘驱动器数目以及加载模式等。当并不是很确定的时候，开始设置woker processes数目等于CPU核数(或设置为auto）将会获得一个较好的效果。配置语法如下：
<pre>
Syntax:	worker_processes number | auto;
Default:	worker_processes 1;
Context:	main
</pre>
注意：```auto参数值从1.3.8版本开始支持```

**5） debug_points字段**

```debug_points```指令的配置语法如下：
<pre>
Syntax:	debug_points abort | stop;
Default:	—
Context:	main
</pre>
本指令主要用于调试作用。当一个内部错误被检测到时，例如当woker进程重启时sockets泄露，此时enable debug_points会导致产生一个core dump文件（设置为abort时）或者停止进程的执行（设置为stop时），因此我们可以使用系统调试器进行进一步的分析。

**6) rlimit_nofile字段**

```worker_rlimit_nofile```指令的配置语法如下：
<pre>
Syntax:	worker_rlimit_nofile number;
Default:	—
Context:	main
</pre>
为worker进程改变打开文件的最大限制数(RLIMIT_NOFILE)。主要用于在不重启主进程的情况下，增加可打开文件的限制数。


**7) rlimit_core字段**

```worker_rlimit_core```指令的配置语法如下：
<pre>
Syntax:	worker_rlimit_core size;
Default:	—
Context:	main
</pre>
改变worker进程产生core dump文件的最大大小限制。主要用于在不重启主进程的情况下，增加产生core dump文件大小的上限值。

**8) priority字段**

```worker_priority```指令的配置语法如下：
<pre>
Syntax:	worker_priority number;
Default:	
worker_priority 0;
Context:	main
</pre>
用于定义worker进程的调度优先级，类似于```nice```命令： 一个负的```number```值具有较高的优先级。这里```number```的取值范围通常在-20到20之间。

<br />

**9) cpu_affinity_auto字段**

表明当前是否自动设置进程的CPU亲和性。

**10) cpu_affinity_n字段**

表明当前所设置的CPU亲和性字段的个数。

**11) cpu_affinity字段**

```worker_cpu_affinity```指令用于设置worker进程的CPU亲和性。指令的配置语法如下：
<pre>
Syntax:	worker_cpu_affinity cpumask ...;
worker_cpu_affinity auto [cpumask];
Default:	—
Context:	main
</pre>

绑定worker进程到CPU集上。每一个CPU集都由一个bitmask来表示。一般来说，针对每一个worker进程，都应该有一个单独的CPU集。默认情况下，worker进程并不会绑定到任何一个CPU上。如下给出一个配置样例：
{% highlight string %}
worker_processes    4;
worker_cpu_affinity 0001 0010 0100 1000;
{% endhighlight %}
上面对于每一个worker进程，都绑定到一个单独的CPU上。而对于如下：
{% highlight string %}
worker_processes    2;
worker_cpu_affinity 0101 1010;
{% endhighlight %}
会绑定第一个worker进程到CPU0/CPU2上；第二个worker进程会被绑定到CPU1/CPU3上。第二个示例比较适合用在超线程架构的cpu上（hyper-threading)。

而```auto```参数值（1.9.10版开始)允许自动的绑定worker进程到可用的CPU上：
{% highlight string %}
worker_processes auto;
worker_cpu_affinity auto;
{% endhighlight %}
此种情况下，可选的```mask```参数可以限制自动绑定的CPU集：
{% highlight string %}
worker_cpu_affinity auto 01010101;
{% endhighlight %}

注意： ```worker_cpu_affinity```指令只能用在FreeBSD和Linux系统上

<br />

**12) username/user/group字段**

用于设置nginx以某个用户/用户组方式运行。指令的配置语法如下：
<pre>
Syntax:	user user [group];
Default:	user nobody nobody;
Context:	main
</pre>
定义worker进程运行所使用的```user```及```group```身份。假如```group```缺省的话，则会采用组名与```user```相同的组。

**13) working_directory字段**

```working_directory```指令用于配置worker进程的工作目录。配置语法如下：
<pre>
Syntax:	working_directory directory;
Default:	—
Context:	main
</pre>
定义worker子进程的当前工作目录。主要用于写一个core文件时，worker进程应该要有该特定目录的访问权限。

**14) lock_file字段**

```lock_file```指令的语法如下：
<pre>
Syntax:	lock_file file;
Default:	
lock_file logs/nginx.lock;
Context:	main
</pre>
nginx使用锁机制来实现```accept mutex```，并且顺序的来访问共享内存。在大多数的系统上，锁都是通过原子操作来实现的，因此会忽略配置文件中的```lock_file file```指令；而对于其他的一些系统，```lock file```机制会被使用。

**15) pid/oldpid字段**

```pid```指令的语法如下：
<pre>
Syntax:	pid file;
Default:	pid logs/nginx.pid;
Context:	main
</pre>
定义在什么地方存储主进程的pid。

**16) env/environment字段**

```env```指令语法如下：
<pre>
Syntax:	env variable[=value];
Default:	env TZ;
Context:	main
</pre>
默认情况下，nginx会移除所有从父进程继承而来的环境变量（除了TZ环境变量外）。本指令允许保留一些继承而来的变量，改变它们的值，或者创建新的环境变量。这些环境变量将会被用在：

* 在进行```live upgrade```(热升级）时会被继承

* 被```ngx_http_perl_module```模块所使用

* 被worker进程所使用。有一点需要记住的是:想要通过此来控制一些系统库(system libraries),有时候可能并不能达到想要的效果。原因是很多的系统库是在初始化的时候会检查这些变量。（其中的一个例外情况是上面提到的live upgrade操作）

```TZ```环境变量总是会被继承，并且可以在```ngx_http_perl_module```中使用。

使用范例：
{% highlight string %}
env MALLOC_OPTIONS;
env PERL5LIB=/data/site/modules;
env OPENSSL_ALLOW_PROXY_CERTS=1;
{% endhighlight %}

注意： ```nginx环境变量是在nginx内部所使用，不应该由用户来直接进行设置。```

## 5. 相关函数定义

下面对相关函数做一个简单的说明：
{% highlight string %}
//判断cycle是否已经初始化
#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)



//初始化cycle
ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);

//创建进程pid文件
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);

//删除进程pid文件
void ngx_delete_pidfile(ngx_cycle_t *cycle);

//向master进程发送信号
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);

//以user用户身份打开或创建文件
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);

//设置一些环境变量
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);

//进行nginx热升级
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);

//获得进程CPU亲和性
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);

//添加一个共享内存
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);
{% endhighlight %}





## 6. 相关变量声明
{% highlight string %}
// 指向当前nginx运行上下文
extern volatile ngx_cycle_t  *ngx_cycle;      

// 所有旧的nginx运行上下文     
extern ngx_array_t            ngx_old_cycles;

// nginx核心模块
extern ngx_module_t           ngx_core_module;

//测试nginx配置文件(一般为nginx -t/-T选项时）
extern ngx_uint_t             ngx_test_config;

// dump nginx配置文件（一般为nginx -T选项时）
extern ngx_uint_t             ngx_dump_config;

//在测试配置文件时，抑制一些非错误消息的输出
extern ngx_uint_t             ngx_quiet_mode;
{% endhighlight %}


<br />
<br />

1. [nginx共享内存：共享内存的实现](http://b
2. log.csdn.net/wgwgnihao/article/details/37838837)

2. [Nginx内存管理及数据结构浅析–共享内存的实现](http://www.colaghost.net/web-server/246)

3. [nginx之共享内存](http://blog.csdn.net/evsqiezi/article/details/51785093)

4. [Nginx Cycle](http://nginx.org/en/docs/dev/development_guide.html#cycle)

5. [Nginx源码分析： 3张图看懂启动及进程工作原理](http://www.360doc.com/content/16/0220/10/30291625_535903478.shtml)

6. [nginx学习十 ngx_cycle_t 、ngx_connection_t 和ngx_listening_t](http://blog.csdn.net/xiaoliangsky/article/details/39831035)

7. [ngx_cycle_s](http://blog.csdn.net/yzt33/article/details/47087943)

8. [NGINX 加载动态模块（NGINX 1.9.11开始增加加载动态模块支持）](https://www.cnblogs.com/tinywan/p/6965467.html)

9. [nginx conf相关](http://blog.csdn.net/cschengvdn/article/details/25273541)

10. [Core Functionality](http://nginx.org/en/docs/ngx_core_module.html)
<br />
<br />
<br />

