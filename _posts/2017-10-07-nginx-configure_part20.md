---
layout: post
title: auto/install脚本分析-part20
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

auto/install脚本主要处理在编译完成之后，进行安装的相关事宜。




<!-- more -->


## 1. auto/install脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $USE_PERL != NO ]; then

    cat << END                                                >> $NGX_MAKEFILE

install_perl_modules:
	cd $NGX_OBJS/src/http/modules/perl && \$(MAKE) install
END

    NGX_INSTALL_PERL_MODULES=install_perl_modules

fi


case ".$NGX_SBIN_PATH" in
    ./*)
    ;;

    *)
        NGX_SBIN_PATH=$NGX_PREFIX/$NGX_SBIN_PATH
    ;;
esac


case ".$NGX_MODULES_PATH" in
    ./*)
    ;;

    *)
        NGX_MODULES_PATH=$NGX_PREFIX/$NGX_MODULES_PATH
    ;;
esac

NGX_MODULES_PATH=`dirname $NGX_MODULES_PATH/.`


case ".$NGX_CONF_PATH" in
    ./*)
    ;;

    *)
        NGX_CONF_PATH=$NGX_PREFIX/$NGX_CONF_PATH
    ;;
esac


NGX_CONF_PREFIX=`dirname $NGX_CONF_PATH`


case ".$NGX_PID_PATH" in
    ./*)
    ;;

    *)
        NGX_PID_PATH=$NGX_PREFIX/$NGX_PID_PATH
    ;;
esac


case ".$NGX_ERROR_LOG_PATH" in
    ./* | .)
    ;;

    *)
        NGX_ERROR_LOG_PATH=$NGX_PREFIX/$NGX_ERROR_LOG_PATH
    ;;
esac


case ".$NGX_HTTP_LOG_PATH" in
    ./*)
    ;;

    *)
        NGX_HTTP_LOG_PATH=$NGX_PREFIX/$NGX_HTTP_LOG_PATH
    ;;
esac


if test -f man/nginx.8 ; then
    NGX_MAN=man/nginx.8
else
    NGX_MAN=docs/man/nginx.8
fi

if test -d html ; then
    NGX_HTML=html
else
    NGX_HTML=docs/html
fi

cat << END                                                    >> $NGX_MAKEFILE

manpage:	$NGX_OBJS/nginx.8

$NGX_OBJS/nginx.8:	$NGX_MAN $NGX_AUTO_CONFIG_H
	sed -e "s|%%PREFIX%%|$NGX_PREFIX|" \\
		-e "s|%%PID_PATH%%|$NGX_PID_PATH|" \\
		-e "s|%%CONF_PATH%%|$NGX_CONF_PATH|" \\
		-e "s|%%ERROR_LOG_PATH%%|${NGX_ERROR_LOG_PATH:-stderr}|" \\
		< $NGX_MAN > \$@

install:	build $NGX_INSTALL_PERL_MODULES
	test -d '\$(DESTDIR)$NGX_PREFIX' || mkdir -p '\$(DESTDIR)$NGX_PREFIX'

	test -d '\$(DESTDIR)`dirname "$NGX_SBIN_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_SBIN_PATH"`'
	test ! -f '\$(DESTDIR)$NGX_SBIN_PATH' \\
		|| mv '\$(DESTDIR)$NGX_SBIN_PATH' \\
			'\$(DESTDIR)$NGX_SBIN_PATH.old'
	cp $NGX_OBJS/nginx '\$(DESTDIR)$NGX_SBIN_PATH'

	test -d '\$(DESTDIR)$NGX_CONF_PREFIX' \\
		|| mkdir -p '\$(DESTDIR)$NGX_CONF_PREFIX'

	cp conf/koi-win '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/koi-utf '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/win-utf '\$(DESTDIR)$NGX_CONF_PREFIX'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/mime.types' \\
		|| cp conf/mime.types '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/mime.types '\$(DESTDIR)$NGX_CONF_PREFIX/mime.types.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi_params' \\
		|| cp conf/fastcgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/fastcgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi.conf' \\
		|| cp conf/fastcgi.conf '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/fastcgi.conf '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi.conf.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/uwsgi_params' \\
		|| cp conf/uwsgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/uwsgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/uwsgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/scgi_params' \\
		|| cp conf/scgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/scgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/scgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PATH' \\
		|| cp conf/nginx.conf '\$(DESTDIR)$NGX_CONF_PATH'
	cp conf/nginx.conf '\$(DESTDIR)$NGX_CONF_PREFIX/nginx.conf.default'

	test -d '\$(DESTDIR)`dirname "$NGX_PID_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_PID_PATH"`'

	test -d '\$(DESTDIR)`dirname "$NGX_HTTP_LOG_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_HTTP_LOG_PATH"`'

	test -d '\$(DESTDIR)$NGX_PREFIX/html' \\
		|| cp -R $NGX_HTML '\$(DESTDIR)$NGX_PREFIX'
END


if test -n "$NGX_ERROR_LOG_PATH"; then
    cat << END                                                >> $NGX_MAKEFILE

	test -d '\$(DESTDIR)`dirname "$NGX_ERROR_LOG_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_ERROR_LOG_PATH"`'
END

fi


if test -n "$DYNAMIC_MODULES"; then
    cat << END                                                >> $NGX_MAKEFILE

	test -d '\$(DESTDIR)$NGX_MODULES_PATH' \\
		|| mkdir -p '\$(DESTDIR)$NGX_MODULES_PATH'
END

fi


for ngx_module in $DYNAMIC_MODULES
do
    ngx_module=$ngx_module$ngx_modext

    cat << END                                                >> $NGX_MAKEFILE

	test ! -f '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module' \\
		|| mv '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module' \\
			'\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module.old'
	cp $NGX_OBJS/$ngx_module '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module'
END

done


# create Makefile

cat << END >> Makefile

build:
	\$(MAKE) -f $NGX_MAKEFILE

install:
	\$(MAKE) -f $NGX_MAKEFILE install

modules:
	\$(MAKE) -f $NGX_MAKEFILE modules

upgrade:
	$NGX_SBIN_PATH -t

	kill -USR2 \`cat $NGX_PID_PATH\`
	sleep 1
	test -f $NGX_PID_PATH.oldbin

	kill -QUIT \`cat $NGX_PID_PATH.oldbin\`
END
{% endhighlight %}


下面简要分析一下脚本：

<br />

**1) 检测是否安装perl modules**
{% highlight string %}
if [ $USE_PERL != NO ]; then

    cat << END                                                >> $NGX_MAKEFILE

install_perl_modules:
	cd $NGX_OBJS/src/http/modules/perl && \$(MAKE) install
END

    NGX_INSTALL_PERL_MODULES=install_perl_modules

fi
{% endhighlight %}

这里我们当前环境```USE_PERL```为```NO```，因此里面脚本并不执行。


<br />

**2) 处理相关路径**
{% highlight string %}
case ".$NGX_SBIN_PATH" in
    ./*)
    ;;

    *)
        NGX_SBIN_PATH=$NGX_PREFIX/$NGX_SBIN_PATH
    ;;
esac


case ".$NGX_MODULES_PATH" in
    ./*)
    ;;

    *)
        NGX_MODULES_PATH=$NGX_PREFIX/$NGX_MODULES_PATH
    ;;
esac

NGX_MODULES_PATH=`dirname $NGX_MODULES_PATH/.`


case ".$NGX_CONF_PATH" in
    ./*)
    ;;

    *)
        NGX_CONF_PATH=$NGX_PREFIX/$NGX_CONF_PATH
    ;;
esac


NGX_CONF_PREFIX=`dirname $NGX_CONF_PATH`


case ".$NGX_PID_PATH" in
    ./*)
    ;;

    *)
        NGX_PID_PATH=$NGX_PREFIX/$NGX_PID_PATH
    ;;
esac


case ".$NGX_ERROR_LOG_PATH" in
    ./* | .)
    ;;

    *)
        NGX_ERROR_LOG_PATH=$NGX_PREFIX/$NGX_ERROR_LOG_PATH
    ;;
esac


case ".$NGX_HTTP_LOG_PATH" in
    ./*)
    ;;

    *)
        NGX_HTTP_LOG_PATH=$NGX_PREFIX/$NGX_HTTP_LOG_PATH
    ;;
esac
{% endhighlight %}

在configure脚本执行时我们通过：
<pre>
--sbin-path=/usr/local/nginx/nginx
</pre>
指定了```NGX_SBIN_PATH```，匹配```./*```。

<br />

对于```NGX_MODULES_PATH```,在auto/options脚本中默认值为空，同时在该脚本中检测到为空时会被设置为```modules```; ```NGX_PREFIX```在auto/options中被设置为空，因此在configure脚本中会设置成默认值：```/usr/local/nginx```;此处```NGX_MODULES_PATH```值为：
<pre>
NGX_MODULES_PATH=/usr/local/nginx/modules
</pre>

<br />

```NGX_CONF_PATH```在configure脚本执行时我们通过：
<pre>
--conf-path=/usr/local/nginx/nginx.conf
</pre>
进行了设置。


<br />

```NGX_PID_PATH```在configure脚本执行时我们通过：
<pre>
--pid-path=/usr/local/nginx/nginx.pid
</pre>
进行了设置。

<br />

```NGX_ERROR_LOG_PATH```在auto/options脚本中默认值为空，同时在该脚本中检测到为空时会被设置为```logs/error.log```;然后通过如下脚本：
{% highlight string %}
NGX_ERROR_LOG_PATH=$NGX_PREFIX/$NGX_ERROR_LOG_PATH
{% endhighlight %}
会被设置为```/usr/local/nginx/logs/error.log```。

<br />

```NGX_HTTP_LOG_PATH```在auto/options脚本中默认值为空，同时在该脚本中检测到为空时会被设置为```logs/access.log```;然后通过如下脚本：
{% highlight string %}
NGX_HTTP_LOG_PATH=$NGX_PREFIX/$NGX_HTTP_LOG_PATH
{% endhighlight %}
会被设置为```/usr/local/nginx/logs/access.log```。



<br />



**3) 帮助文档相关路径**
{% highlight string %}
if test -f man/nginx.8 ; then
    NGX_MAN=man/nginx.8
else
    NGX_MAN=docs/man/nginx.8
fi

if test -d html ; then
    NGX_HTML=html
else
    NGX_HTML=docs/html
fi
{% endhighlight %}

由于在nginx-1.10.3主目录下存在```man/nginx.8```文件，因此这里会将```NGX_MAN```设置为:
<pre>
NGX_MAN=man/nginx.8
</pre>

同时在nginx-1.10.3主目录下也存在```html```目录，因此这里会将```NGX_HTML```设置为：
<pre>
NGX_HTML=html
</pre>


<br />

**4) 生成安装脚本**
{% highlight string %}
cat << END                                                    >> $NGX_MAKEFILE

manpage:	$NGX_OBJS/nginx.8

$NGX_OBJS/nginx.8:	$NGX_MAN $NGX_AUTO_CONFIG_H
	sed -e "s|%%PREFIX%%|$NGX_PREFIX|" \\
		-e "s|%%PID_PATH%%|$NGX_PID_PATH|" \\
		-e "s|%%CONF_PATH%%|$NGX_CONF_PATH|" \\
		-e "s|%%ERROR_LOG_PATH%%|${NGX_ERROR_LOG_PATH:-stderr}|" \\
		< $NGX_MAN > \$@

install:	build $NGX_INSTALL_PERL_MODULES
	test -d '\$(DESTDIR)$NGX_PREFIX' || mkdir -p '\$(DESTDIR)$NGX_PREFIX'

	test -d '\$(DESTDIR)`dirname "$NGX_SBIN_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_SBIN_PATH"`'
	test ! -f '\$(DESTDIR)$NGX_SBIN_PATH' \\
		|| mv '\$(DESTDIR)$NGX_SBIN_PATH' \\
			'\$(DESTDIR)$NGX_SBIN_PATH.old'
	cp $NGX_OBJS/nginx '\$(DESTDIR)$NGX_SBIN_PATH'

	test -d '\$(DESTDIR)$NGX_CONF_PREFIX' \\
		|| mkdir -p '\$(DESTDIR)$NGX_CONF_PREFIX'

	cp conf/koi-win '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/koi-utf '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/win-utf '\$(DESTDIR)$NGX_CONF_PREFIX'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/mime.types' \\
		|| cp conf/mime.types '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/mime.types '\$(DESTDIR)$NGX_CONF_PREFIX/mime.types.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi_params' \\
		|| cp conf/fastcgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/fastcgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi.conf' \\
		|| cp conf/fastcgi.conf '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/fastcgi.conf '\$(DESTDIR)$NGX_CONF_PREFIX/fastcgi.conf.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/uwsgi_params' \\
		|| cp conf/uwsgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/uwsgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/uwsgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PREFIX/scgi_params' \\
		|| cp conf/scgi_params '\$(DESTDIR)$NGX_CONF_PREFIX'
	cp conf/scgi_params \\
		'\$(DESTDIR)$NGX_CONF_PREFIX/scgi_params.default'

	test -f '\$(DESTDIR)$NGX_CONF_PATH' \\
		|| cp conf/nginx.conf '\$(DESTDIR)$NGX_CONF_PATH'
	cp conf/nginx.conf '\$(DESTDIR)$NGX_CONF_PREFIX/nginx.conf.default'

	test -d '\$(DESTDIR)`dirname "$NGX_PID_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_PID_PATH"`'

	test -d '\$(DESTDIR)`dirname "$NGX_HTTP_LOG_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_HTTP_LOG_PATH"`'

	test -d '\$(DESTDIR)$NGX_PREFIX/html' \\
		|| cp -R $NGX_HTML '\$(DESTDIR)$NGX_PREFIX'
END
{% endhighlight %}

这里：
{% highlight string %}
$NGX_OBJS/nginx.8:	$NGX_MAN $NGX_AUTO_CONFIG_H
	sed -e "s|%%PREFIX%%|$NGX_PREFIX|" \\
		-e "s|%%PID_PATH%%|$NGX_PID_PATH|" \\
		-e "s|%%CONF_PATH%%|$NGX_CONF_PATH|" \\
		-e "s|%%ERROR_LOG_PATH%%|${NGX_ERROR_LOG_PATH:-stderr}|" \\
		< $NGX_MAN > \$@
{% endhighlight %}
将```$NGX_MAN```文件中的```%%PREFIX```等替换为```$NGX_PREFIX```,然后将替换后的结果写入到```$NGX_OBJS/nginx.8```文件中。

```NGX_MAKEFILE```为objs/Makefile。

其他相对容易理解，这里给出生成的对应Makefile脚本作为参考：
{% highlight string %}
manpage:	objs/nginx.8

objs/nginx.8:	man/nginx.8 objs/ngx_auto_config.h
	sed -e "s|%%PREFIX%%|/usr/local/nginx|" \
		-e "s|%%PID_PATH%%|/usr/local/nginx/nginx.pid|" \
		-e "s|%%CONF_PATH%%|/usr/local/nginx/nginx.conf|" \
		-e "s|%%ERROR_LOG_PATH%%|/usr/local/nginx/logs/error.log|" \
		< man/nginx.8 > $@

install:	build 
	test -d '$(DESTDIR)/usr/local/nginx' || mkdir -p '$(DESTDIR)/usr/local/nginx'

	test -d '$(DESTDIR)/usr/local/nginx' \
		|| mkdir -p '$(DESTDIR)/usr/local/nginx'
	test ! -f '$(DESTDIR)/usr/local/nginx/nginx' \
		|| mv '$(DESTDIR)/usr/local/nginx/nginx' \
			'$(DESTDIR)/usr/local/nginx/nginx.old'
	cp objs/nginx '$(DESTDIR)/usr/local/nginx/nginx'

	test -d '$(DESTDIR)/usr/local/nginx' \
		|| mkdir -p '$(DESTDIR)/usr/local/nginx'

	cp conf/koi-win '$(DESTDIR)/usr/local/nginx'
	cp conf/koi-utf '$(DESTDIR)/usr/local/nginx'
	cp conf/win-utf '$(DESTDIR)/usr/local/nginx'

	test -f '$(DESTDIR)/usr/local/nginx/mime.types' \
		|| cp conf/mime.types '$(DESTDIR)/usr/local/nginx'
	cp conf/mime.types '$(DESTDIR)/usr/local/nginx/mime.types.default'

	test -f '$(DESTDIR)/usr/local/nginx/fastcgi_params' \
		|| cp conf/fastcgi_params '$(DESTDIR)/usr/local/nginx'
	cp conf/fastcgi_params \
		'$(DESTDIR)/usr/local/nginx/fastcgi_params.default'

	test -f '$(DESTDIR)/usr/local/nginx/fastcgi.conf' \
		|| cp conf/fastcgi.conf '$(DESTDIR)/usr/local/nginx'
	cp conf/fastcgi.conf '$(DESTDIR)/usr/local/nginx/fastcgi.conf.default'

	test -f '$(DESTDIR)/usr/local/nginx/uwsgi_params' \
		|| cp conf/uwsgi_params '$(DESTDIR)/usr/local/nginx'
	cp conf/uwsgi_params \
		'$(DESTDIR)/usr/local/nginx/uwsgi_params.default'

	test -f '$(DESTDIR)/usr/local/nginx/scgi_params' \
		|| cp conf/scgi_params '$(DESTDIR)/usr/local/nginx'
	cp conf/scgi_params \
		'$(DESTDIR)/usr/local/nginx/scgi_params.default'

	test -f '$(DESTDIR)/usr/local/nginx/nginx.conf' \
		|| cp conf/nginx.conf '$(DESTDIR)/usr/local/nginx/nginx.conf'
	cp conf/nginx.conf '$(DESTDIR)/usr/local/nginx/nginx.conf.default'

	test -d '$(DESTDIR)/usr/local/nginx' \
		|| mkdir -p '$(DESTDIR)/usr/local/nginx'

	test -d '$(DESTDIR)/usr/local/nginx/logs' \
		|| mkdir -p '$(DESTDIR)/usr/local/nginx/logs'

	test -d '$(DESTDIR)/usr/local/nginx/html' \
		|| cp -R html '$(DESTDIR)/usr/local/nginx'

{% endhighlight %}

<br />



**5) 生成Error Log相关Makefile脚本**
{% highlight string %}
if test -n "$NGX_ERROR_LOG_PATH"; then
    cat << END                                                >> $NGX_MAKEFILE

	test -d '\$(DESTDIR)`dirname "$NGX_ERROR_LOG_PATH"`' \\
		|| mkdir -p '\$(DESTDIR)`dirname "$NGX_ERROR_LOG_PATH"`'
END

fi
{% endhighlight %}

执行结果如下：
<pre>
test -d '$(DESTDIR)/usr/local/nginx/logs' \
	|| mkdir -p '$(DESTDIR)/usr/local/nginx/logs'
</pre>

<br />

**6) 生成Dynamic modules相关Makefile脚本**
{% highlight string %}
if test -n "$DYNAMIC_MODULES"; then
    cat << END                                                >> $NGX_MAKEFILE

	test -d '\$(DESTDIR)$NGX_MODULES_PATH' \\
		|| mkdir -p '\$(DESTDIR)$NGX_MODULES_PATH'
END

fi


for ngx_module in $DYNAMIC_MODULES
do
    ngx_module=$ngx_module$ngx_modext

    cat << END                                                >> $NGX_MAKEFILE

	test ! -f '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module' \\
		|| mv '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module' \\
			'\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module.old'
	cp $NGX_OBJS/$ngx_module '\$(DESTDIR)$NGX_MODULES_PATH/$ngx_module'
END

done
{% endhighlight %}

这里我们并未添加任何```DYNAMIC_MODULES```，因此脚本并不会得到执行。

<br />

**7) 生成主控Makefile文件**
{% highlight string %}
# create Makefile

cat << END >> Makefile

build:
	\$(MAKE) -f $NGX_MAKEFILE

install:
	\$(MAKE) -f $NGX_MAKEFILE install

modules:
	\$(MAKE) -f $NGX_MAKEFILE modules

upgrade:
	$NGX_SBIN_PATH -t

	kill -USR2 \`cat $NGX_PID_PATH\`
	sleep 1
	test -f $NGX_PID_PATH.oldbin

	kill -QUIT \`cat $NGX_PID_PATH.oldbin\`
END
{% endhighlight %}

在nginx-1.10.3主目录下生成Makefile文件，在该Makefile文件中执行```$NGX_MAKEFILE```文件（即objs/Makefile)。


<br />
```NOTE```:
<pre>
默认情况下，nginx会被安装在/usr/local/nginx目录下，并且在该目录下会创建conf,sbin,logs,modules等文件夹。
conf文件夹下存放配置文件；sbin文件夹下存放nginx可执行文件；logs文件夹下存放日志文件、pid文件、lock文件；
modules文件夹下存放dynamic modules的动态链接库文件。
</pre>


<br />
<br />
<br />

