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

**5) **





<br />
<br />

参看：

1. [Nginx 引入 Dynamic Module 架构](http://www.linuxidc.com/Linux/2016-02/128241.htm)

2. [Compiling Third-Party Dynamic Modules for NGINX and NGINX Plus](https://www.nginx.com/blog/compiling-dynamic-modules-nginx-plus/)

3. [New Config Shell File](https://www.nginx.com/resources/wiki/extending/new_config/)

4. [Dynamic modules](http://mailman.nginx.org/pipermail/nginx-devel/2016-February/007852.html)

5. [手把手教你开发Nginx模块](http://www.cnblogs.com/zolo/p/5857806.html)

6. [Nginx开发从入门到精通](http://tengine.taobao.org/book/chapter_02.html#nginx)

<br />
<br />
<br />

