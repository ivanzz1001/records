---
layout: post
title: auto/module脚本分析-part15
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们介绍auto/module脚本，该脚本的主要功能是为模块指定对应的源文件、头文件、依赖的库文件等。这里模块分为3种类型：


<!-- more -->

* 外部动态加载模块(Dynamic Modules)
* 内部静态模块
* 外部静态模块(AddOn Modules)


## 1. auto/module脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Ruslan Ermilov
# Copyright (C) Nginx, Inc.


case $ngx_module_type in
    HTTP_*) ngx_var=HTTP ;;
    *)      ngx_var=$ngx_module_type ;;
esac


if [ "$ngx_module_link" = DYNAMIC ]; then

    for ngx_module in $ngx_module_name; do
        # extract the first name
        break
    done

    DYNAMIC_MODULES="$DYNAMIC_MODULES $ngx_module"
    eval ${ngx_module}_SRCS=\"$ngx_module_srcs\"

    eval ${ngx_module}_MODULES=\"$ngx_module_name\"

    if [ -z "$ngx_module_order" -a \
         \( "$ngx_module_type" = "HTTP_FILTER" \
         -o "$ngx_module_type" = "HTTP_AUX_FILTER" \) ]
    then
        eval ${ngx_module}_ORDER=\"$ngx_module_name \
                                   ngx_http_copy_filter_module\"
    else
        eval ${ngx_module}_ORDER=\"$ngx_module_order\"
    fi

    if test -n "$ngx_module_incs"; then
        CORE_INCS="$CORE_INCS $ngx_module_incs"
    fi

    libs=
    for lib in $ngx_module_libs
    do
        case $lib in

            LIBXSLT | LIBGD | GEOIP | PERL)
                libs="$libs \$NGX_LIB_$lib"

                if eval [ "\$USE_${lib}" = NO ] ; then
                    eval USE_${lib}=DYNAMIC
                fi
            ;;

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB)
                eval USE_${lib}=YES
            ;;

            *)
                libs="$libs $lib"
            ;;

        esac
    done
    eval ${ngx_module}_LIBS=\'$libs\'

elif [ "$ngx_module_link" = YES ]; then

    eval ${ngx_module_type}_MODULES=\"\$${ngx_module_type}_MODULES \
                                      $ngx_module_name\"

    eval ${ngx_var}_SRCS=\"\$${ngx_var}_SRCS $ngx_module_srcs\"

    if test -n "$ngx_module_incs"; then
        eval ${ngx_var}_INCS=\"\$${ngx_var}_INCS $ngx_module_incs\"
    fi

    if test -n "$ngx_module_deps"; then
        eval ${ngx_var}_DEPS=\"\$${ngx_var}_DEPS $ngx_module_deps\"
    fi

    for lib in $ngx_module_libs
    do
        case $lib in

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB | LIBXSLT | LIBGD | PERL | GEOIP)
                eval USE_${lib}=YES
            ;;

            *)
                CORE_LIBS="$CORE_LIBS $lib"
            ;;

        esac
    done

elif [ "$ngx_module_link" = ADDON ]; then

    eval ${ngx_module_type}_MODULES=\"\$${ngx_module_type}_MODULES \
                                      $ngx_module_name\"

    NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_module_srcs"

    if test -n "$ngx_module_incs"; then
        eval ${ngx_var}_INCS=\"\$${ngx_var}_INCS $ngx_module_incs\"
    fi

    if test -n "$ngx_module_deps"; then
        NGX_ADDON_DEPS="$NGX_ADDON_DEPS $ngx_module_deps"
    fi

    for lib in $ngx_module_libs
    do
        case $lib in

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB | LIBXSLT | LIBGD | PERL | GEOIP)
                eval USE_${lib}=YES
            ;;

            *)
                CORE_LIBS="$CORE_LIBS $lib"
            ;;

        esac
    done
fi
{% endhighlight %}

上面我们看到根据```$ngx_module_link```的不同，这里module可以分为3种类型：

* 外部动态加载模块(Dynamic Modules)
* 内部静态模块
* 外部静态模块(AddOn Modules)

其中外部动态加载模块通过在执行configure脚本时传入```--add-dynamic-module```选项来添加；外部静态模块通过在执行configure脚本时传入```--add-module```来添加。

```内部静态模块``` 与 ```外部静态模块```基本类似，都是会在编译时直接编译进nginx可执行文件中。两者的区别主要为：内部静态模块一般为nginx官方源代码自带的模块；而外部静态模块一般为用户自己开发的模块，需要添加进nginx内核中。后面我们会看到对这两种模块的处理方式的脚本也基本类似，只有细微差别。

```外部动态加载模块```一般是会编译成一个动态链接库，要使用该模块时一般需要在nginx配置文件中通过```load_module```指令来加载的。




### 1.1 涉及到的变量介绍

auto/module脚本的主要功能就是为了设置相应变量的值，因此如果我们熟悉的话可以不用通过调用此脚本，直接设置对应的变量值就会自动的完成编译。例如：
{% highlight string %}
ngx_addon_name=ngx_http_hello_world_module

if test -n "$ngx_module_link"; then
    ngx_module_type=HTTP
    ngx_module_name=ngx_http_hello_world_module
    ngx_module_srcs="$ngx_addon_dir/ngx_http_hello_world_module.c"

    . auto/module
else
    HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
    NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_hello_world_module.c"
fi
{% endhighlight %}
在else分支我们就是直接设置相应的变量值,然后会在```auto/modules```脚本中通过如下命令：
{% highlight string %}
if [ $HTTP = YES ]; then
    modules="$modules $HTTP_MODULES $HTTP_FILTER_MODULES \
             $HTTP_AUX_FILTER_MODULES $HTTP_INIT_FILTER_MODULES"

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(HTTP_DEPS)"
fi
{% endhighlight %}
将所需要编译的模块、源代码都编译进来。


下面我们就来讲述一下auto/module脚本所涉及到的一些变量：

**1) ngx_module_type**

指定所需要构建的module类型。可选项有：

* HTTP
* CORE
* HTTP_FILTER
* HTTP_INIT_FILTER
* HTTP_AUX_FILTER
* MAIL
* STREAM 
* MISC

**2) ngx_module_name**

模块名称，它应该与该模块中ngx_module_t类型的全局变量名称相同。


**3) ngx_module_incs**

构建模块时所需要包含的路径。

**4) ngx_module_deps**

模块中的.h头文件列表，在构建模块时需要用到。

**5) ngx_module_srcs**

一系列由空格分开的模块源文件列表。```$ngx_addon_dir```变量可以作为模块源文件路径的一个占位符。

**6) ngx_module_libs**

该模块链接时所需要用到的lib列表。例如我们可以通过使用```ngx_module_libs=-lpthread```来链接libpthread库。如下的宏定义可以被用于链接与```NGINX```相同的链接库：

* LIBXSLT (XSL转换相应的库)
* LIBGD (是一个开源的图像处理库)
* GEOIP (基于IP查询的地理位置信息）
* PCRE (是一个用C语言编写的、兼容perl语言的轻量级的正则表达式库)
* OPENSSL
* MD5
* SHA1
* ZLIB 
* PERL (perl 库）

注：Centos下可以通过如下命令来安装perl工具
<pre>
yum install perl perl-devel perl-ExtUtils-Embed
</pre>

**7) ngx_addon_name**

主要用于在```Configure```脚本执行时在控制台输出对应的模块名称

**8) ngx_module_link**

由构建系统设置，值可以为```DYNAMIC```,```YES```,```ADDON```。其中```DYNAMIC```用于构建一个动态模块,后两者用于构建一个静态模块。

**9） ngx_module_order**

设置模块的加载顺序，对于```HTTP_FILTER```和```HTTP_AUX_FILTER```这样的模块类型很有作用。

该顺序关系存放在一个逆向链表中。```ngx_http_copy_filter_module ```接近于链表的底部，因此它会被最先执行。它会读取其他filter的数据。而```ngx_http_write_filter_module```接近于链表的头部，因此它会被最后执行将数据write out出去。

一般情况下，这个选项的格式为当前模块名称，后接由空格分割的一系列模块（这些模块会被插入到当前模块的前面，因此其会在当前模块的后边被执行）。

默认情况下，对于filter模块来说一般会被设置为```$ngx_module_name ngx_http_copy_filter```,这样会导致```ngx_module_name```被插入到```ngx_http_copy_filter```的前面，因此其会落后与copy filter的执行。而对于其他的模块一般本选项值为空。


### 1.2 处理ngx_module_type
{% highlight string %}
case $ngx_module_type in
    HTTP_*) ngx_var=HTTP ;;
    *)      ngx_var=$ngx_module_type ;;
esac
{% endhighlight %}

主要是针对```内部静态模块```与```外部静态模块```进行``HTTP```类的划分。


### 1.3 处理外部动态加载模块
{% highlight string %}

if [ "$ngx_module_link" = DYNAMIC ]; then

    for ngx_module in $ngx_module_name; do
        # extract the first name
        break
    done

    DYNAMIC_MODULES="$DYNAMIC_MODULES $ngx_module"
    eval ${ngx_module}_SRCS=\"$ngx_module_srcs\"

    eval ${ngx_module}_MODULES=\"$ngx_module_name\"

    if [ -z "$ngx_module_order" -a \
         \( "$ngx_module_type" = "HTTP_FILTER" \
         -o "$ngx_module_type" = "HTTP_AUX_FILTER" \) ]
    then
        eval ${ngx_module}_ORDER=\"$ngx_module_name \
                                   ngx_http_copy_filter_module\"
    else
        eval ${ngx_module}_ORDER=\"$ngx_module_order\"
    fi

    if test -n "$ngx_module_incs"; then
        CORE_INCS="$CORE_INCS $ngx_module_incs"
    fi

    libs=
    for lib in $ngx_module_libs
    do
        case $lib in

            LIBXSLT | LIBGD | GEOIP | PERL)
                libs="$libs \$NGX_LIB_$lib"

                if eval [ "\$USE_${lib}" = NO ] ; then
                    eval USE_${lib}=DYNAMIC
                fi
            ;;

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB)
                eval USE_${lib}=YES
            ;;

            *)
                libs="$libs $lib"
            ;;

        esac
    done
    eval ${ngx_module}_LIBS=\'$libs\'

elif [ "$ngx_module_link" = YES ]; then

elif [ "$ngx_module_link" = ADDON ]; then

fi

{% endhighlight %}

前面我们讲过，```外部动态加载模块```需要通过```load_module```指令来进行加载。```外部动态加载模块```可能会依赖于其他的模块，这样就会形成一个链，因为是动态加载的关系，因此就必须通过适当的方法来指定加载顺序。
<pre>
注：对于其他两种模块，是直接编译进可执行程序的，编译的时候就已确定顺序。
</pre>

上述脚本首先求得该```外部动态加载模块```的模块源文件、模块顺序、模块头文文件保存在对应的模块变量中。然后根据所依赖的库文件做如下处理：

**1) LIBXSLT、LIBGD、GEOIP、PERL库**

因为这些库nginx一般自己提供对应的编译方法，但这些库相对来说较大且使用频率较小，因此采用动态链接库的方式来加载

**2) PCRE、OPENSSL、MD5、SHA1、ZLIB**

因为这些库nginx一般自己提供对应的编译方法，这些库相对较小，一般可以直接编译进nginx可执行程序，因此采用静态编译的方式。

**3) 其他库**

其他库一般需要用户自己提供，然后在编译时告诉编译器进行链接。

{% highlight string %}
注意：

内部动态加载模块其实也是用本段脚本进行处理，只是内部动态加载不需要跑到对应的目录下去执行config脚本（这在auto/modules脚本中对这两种情况已经做了区分）。
{% endhighlight %}

### 1.4 处理内部静态模块
{% highlight string %}
if [ "$ngx_module_link" = DYNAMIC ]; then

elif [ "$ngx_module_link" = YES ]; then

    eval ${ngx_module_type}_MODULES=\"\$${ngx_module_type}_MODULES \
                                      $ngx_module_name\"

    eval ${ngx_var}_SRCS=\"\$${ngx_var}_SRCS $ngx_module_srcs\"

    if test -n "$ngx_module_incs"; then
        eval ${ngx_var}_INCS=\"\$${ngx_var}_INCS $ngx_module_incs\"
    fi

    if test -n "$ngx_module_deps"; then
        eval ${ngx_var}_DEPS=\"\$${ngx_var}_DEPS $ngx_module_deps\"
    fi

    for lib in $ngx_module_libs
    do
        case $lib in

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB | LIBXSLT | LIBGD | PERL | GEOIP)
                eval USE_${lib}=YES
            ;;

            *)
                CORE_LIBS="$CORE_LIBS $lib"
            ;;

        esac
    done

elif [ "$ngx_module_link" = ADDON ]; then

fi
{% endhighlight %}

对于内部加载模块，因为程序源代码、头文件等集成在nginx内部，因此只需要在编译时将相应的头文件、源文件包含进来即可。


### 1.5 处理外部静态模块
{% highlight string %}
if [ "$ngx_module_link" = DYNAMIC ]; then

elif [ "$ngx_module_link" = YES ]; then

elif [ "$ngx_module_link" = ADDON ]; then

    eval ${ngx_module_type}_MODULES=\"\$${ngx_module_type}_MODULES \
                                      $ngx_module_name\"

    NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_module_srcs"

    if test -n "$ngx_module_incs"; then
        eval ${ngx_var}_INCS=\"\$${ngx_var}_INCS $ngx_module_incs\"
    fi

    if test -n "$ngx_module_deps"; then
        NGX_ADDON_DEPS="$NGX_ADDON_DEPS $ngx_module_deps"
    fi

    for lib in $ngx_module_libs
    do
        case $lib in

            PCRE | OPENSSL | MD5 | SHA1 | ZLIB | LIBXSLT | LIBGD | PERL | GEOIP)
                eval USE_${lib}=YES
            ;;

            *)
                CORE_LIBS="$CORE_LIBS $lib"
            ;;

        esac
    done
fi
{% endhighlight %}

与```内部静态模块```基本类似，唯一的不同是头文件、源文件可能在外部，需要通过对应的变量引用进来。





<br />
<br />

参看：

1. [Nginx 引入 Dynamic Module 架构](http://www.linuxidc.com/Linux/2016-02/128241.htm)

2. [Compiling Third-Party Dynamic Modules for NGINX and NGINX Plus](https://www.nginx.com/blog/compiling-dynamic-modules-nginx-plus/)

3. [New Config Shell File](https://www.nginx.com/resources/wiki/extending/new_config/)

4. [Dynamic modules](http://mailman.nginx.org/pipermail/nginx-devel/2016-February/007852.html)

5. [手把手教你开发Nginx模块](http://www.cnblogs.com/zolo/p/5857806.html)

6. [Nginx开发从入门到精通](http://tengine.taobao.org/book/chapter_02.html#nginx)

7. [NGINX 加载动态模块（NGINX 1.9.11开始增加加载动态模块支持）](https://www.cnblogs.com/tinywan/p/6965467.html)
<br />
<br />
<br />

