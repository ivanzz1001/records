---
layout: post
title: auto/lib/make脚本分析-part19
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

auto/lib/make脚本主要用于编译nginx所依赖的一些外部库文件。


<!-- more -->



## 1. auto/lib/make脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $PCRE != NONE -a $PCRE != NO -a $PCRE != YES ]; then
    . auto/lib/pcre/make
fi

if [ $MD5 != NONE -a $MD5 != NO -a $MD5 != YES ]; then
    . auto/lib/md5/make
fi

if [ $SHA1 != NONE -a $SHA1 != NO -a $SHA1 != YES ]; then
    . auto/lib/sha1/make
fi

if [ $OPENSSL != NONE -a $OPENSSL != NO -a $OPENSSL != YES ]; then
    . auto/lib/openssl/make
fi

if [ $ZLIB != NONE -a $ZLIB != NO -a $ZLIB != YES ]; then
    . auto/lib/zlib/make
fi

if [ $NGX_LIBATOMIC != NO -a $NGX_LIBATOMIC != YES ]; then
    . auto/lib/libatomic/make
fi

if [ $USE_PERL != NO ]; then
    . auto/lib/perl/make
fi

{% endhighlight %}


**1) PCRE库**
{% highlight string %}
if [ $PCRE != NONE -a $PCRE != NO -a $PCRE != YES ]; then
    . auto/lib/pcre/make
fi
{% endhighlight %}
在auto/options中，我们通过在执行configure传入:
<pre>
--with-pcre=../pcre-8.40
</pre>
因此这里会执行auto/lib/pcre/make脚本。

<br />

**2) MD5库**
{% highlight string %}
if [ $MD5 != NONE -a $MD5 != NO -a $MD5 != YES ]; then
    . auto/lib/md5/make
fi
{% endhighlight %}

在auto/options脚本中，```MD5```默认被初始化为```NONE```。我们这里使用Openssl中的MD5。因此并不会执行对应的make脚本。

<br />

**3) SHA1库**
{% highlight string %}
if [ $SHA1 != NONE -a $SHA1 != NO -a $SHA1 != YES ]; then
    . auto/lib/sha1/make
fi
{% endhighlight %}
在auto/options脚本中```SHA1```默认被初始化为```NONE```;但是在auto/lib/conf脚本中会被设置为```YES```，使用Openssl中的```SHA1```。因此并不会执行对应的make脚本。

<br />

**4) Openssl库**
{% highlight string %}
if [ $OPENSSL != NONE -a $OPENSSL != NO -a $OPENSSL != YES ]; then
    . auto/lib/openssl/make
fi
{% endhighlight %}
在auto/options脚本中```OPENSSL```默认被初始化为```NONE```。这里使用系统提供的openssl,并不需要额外执行对应的make脚本。

<br />



**5) ZLIB库**
{% highlight string %}
if [ $ZLIB != NONE -a $ZLIB != NO -a $ZLIB != YES ]; then
    . auto/lib/zlib/make
fi
{% endhighlight %}

在auto/options中，我们通过在执行configure传入:
<pre>
--with-zlib=../zlib-1.2.11
</pre>
因此这里会执行auto/lib/zlib/make脚本。


<br />

**6) libatomic库**
{% highlight string %}
if [ $NGX_LIBATOMIC != NO -a $NGX_LIBATOMIC != YES ]; then
    . auto/lib/libatomic/make
fi
{% endhighlight %}
在auto/options脚本中,```NGX_LIBATOMIC```默认被初始化为```NO```，此后也并未做相应修改。因此并不需要执行对应的make脚本。

**7) PERL库**
{% highlight string %}
if [ $USE_PERL != NO ]; then
    . auto/lib/perl/make
fi
{% endhighlight %}

在auto/options脚本中,```USE_PERL```默认被初始化为```NO```，此后也并未做相应修改。因此并不需要执行对应的make脚本。



## 2. auto/lib/pcre/make脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


case "$NGX_CC_NAME" in

    msvc)
        ngx_makefile=makefile.msvc
        ngx_opt="CPU_OPT=\"$CPU_OPT\" LIBC=$LIBC"
        ngx_pcre="PCRE=\"$PCRE\""
    ;;

    owc)
        ngx_makefile=makefile.owc
        ngx_opt="CPU_OPT=\"$CPU_OPT\""
        ngx_pcre=`echo PCRE=\"$PCRE\" | sed -e "s/\//$ngx_regex_dirsep/g"`
    ;;

    bcc)
        ngx_makefile=makefile.bcc
        ngx_opt="-DCPU_OPT=\"$CPU_OPT\""
        ngx_pcre=`echo \-DPCRE=\"$PCRE\" | sed -e "s/\//$ngx_regex_dirsep/g"`
    ;;

    *)
        ngx_makefile=
    ;;

esac


if [ -n "$ngx_makefile" ]; then

    cat << END                                                >> $NGX_MAKEFILE

`echo "$PCRE/pcre.lib:	$PCRE/pcre.h $NGX_MAKEFILE"			\
	| sed -e "s/\//$ngx_regex_dirsep/g"`
	\$(MAKE) -f auto/lib/pcre/$ngx_makefile $ngx_pcre $ngx_opt

`echo "$PCRE/pcre.h:" | sed -e "s/\//$ngx_regex_dirsep/g"`
	\$(MAKE) -f auto/lib/pcre/$ngx_makefile $ngx_pcre pcre.h

END

else

    cat << END                                                >> $NGX_MAKEFILE

$PCRE/pcre.h:	$PCRE/Makefile

$PCRE/Makefile:	$NGX_MAKEFILE
	cd $PCRE \\
	&& if [ -f Makefile ]; then \$(MAKE) distclean; fi \\
	&& CC="\$(CC)" CFLAGS="$PCRE_OPT" \\
	./configure --disable-shared $PCRE_CONF_OPT

$PCRE/.libs/libpcre.a:	$PCRE/Makefile
	cd $PCRE \\
	&& \$(MAKE) libpcre.la

END

fi
{% endhighlight %}

我们当前```NGX_CC_NAME```被置为```gcc```，因此变量```ngx_makefile```为空。因此会执行else部分代码声场Makefile：
{% highlight string %}
    cat << END                                                >> $NGX_MAKEFILE

$PCRE/pcre.h:	$PCRE/Makefile

$PCRE/Makefile:	$NGX_MAKEFILE
	cd $PCRE \\
	&& if [ -f Makefile ]; then \$(MAKE) distclean; fi \\
	&& CC="\$(CC)" CFLAGS="$PCRE_OPT" \\
	./configure --disable-shared $PCRE_CONF_OPT

$PCRE/.libs/libpcre.a:	$PCRE/Makefile
	cd $PCRE \\
	&& \$(MAKE) libpcre.la

END
{% endhighlight %}
向objs/Makefile文件写入：
{% highlight string %}
../pcre-8.40/pcre.h:	../pcre-8.40/Makefile

../pcre-8.40/Makefile:	objs/Makefile
	cd ../pcre-8.40 \
	&& if [ -f Makefile ]; then $(MAKE) distclean; fi \
	&& CC="$(CC)" CFLAGS="-O2 -fomit-frame-pointer -pipe " \
	./configure --disable-shared 

../pcre-8.40/.libs/libpcre.a:	../pcre-8.40/Makefile
	cd ../pcre-8.40 \
	&& $(MAKE) libpcre.la
{% endhighlight %}



## 3. auto/lib/zlib/make脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


case "$NGX_CC_NAME" in

    msvc)
        ngx_makefile=makefile.msvc
        ngx_opt="CPU_OPT=\"$CPU_OPT\" LIBC=$LIBC"
        ngx_zlib="ZLIB=\"$ZLIB\""

    ;;

    owc)
        ngx_makefile=makefile.owc
        ngx_opt="CPU_OPT=\"$CPU_OPT\""
        ngx_zlib=`echo ZLIB=\"$ZLIB\" | sed -e "s/\//$ngx_regex_dirsep/g"`
    ;;

    bcc)
        ngx_makefile=makefile.bcc
        ngx_opt="-DCPU_OPT=\"$CPU_OPT\""
        ngx_zlib=`echo \-DZLIB=\"$ZLIB\" | sed -e "s/\//$ngx_regex_dirsep/g"`
    ;;

    *)
        ngx_makefile=
    ;;

esac


done=NO


case "$NGX_PLATFORM" in

    win32)

        if [ -n "$ngx_makefile" ]; then
            cat << END                                        >> $NGX_MAKEFILE

`echo "$ZLIB/zlib.lib:	$NGX_MAKEFILE" | sed -e "s/\//$ngx_regex_dirsep/g"`
	\$(MAKE) -f auto/lib/zlib/$ngx_makefile $ngx_opt $ngx_zlib

END

        else

            cat << END                                        >> $NGX_MAKEFILE

$ZLIB/libz.a:	$NGX_MAKEFILE
	cd $ZLIB \\
	&& \$(MAKE) distclean \\
	&& \$(MAKE) -f win32/Makefile.gcc \\
		CFLAGS="$ZLIB_OPT" CC="\$(CC)" \\
		libz.a

END

        fi

        done=YES
    ;;

    # FreeBSD: i386
    # Linux: i686

    *:i386 | *:i686)
        case $ZLIB_ASM in
            pentium)

                cat << END                                    >> $NGX_MAKEFILE

$ZLIB/libz.a:	$NGX_MAKEFILE
	cd $ZLIB \\
	&& \$(MAKE) distclean \\
	&& cp contrib/asm586/match.S . \\
	&& CFLAGS="$ZLIB_OPT -DASMV" CC="\$(CC)" \\
		./configure \\
	&& \$(MAKE) OBJA=match.o libz.a

END

                done=YES
            ;;

            pentiumpro)

                cat << END                                    >> $NGX_MAKEFILE

$ZLIB/libz.a:	$NGX_MAKEFILE
	cd $ZLIB \\
	&& \$(MAKE) distclean \\
	&& cp contrib/asm686/match.S . \\
	&& CFLAGS="$ZLIB_OPT -DASMV" CC="\$(CC)" \\
		./configure \\
	&& \$(MAKE) OBJA=match.o libz.a

END

                done=YES
            ;;

            NO)
            ;;

            *)
                echo "$0: error: invalid --with-zlib-asm=$ZLIB_ASM option."
                echo "The valid values are \"pentium\" and \"pentiumpro\" only".
                echo

                exit 1;
            ;;
        esac
    ;;

esac


if [ $done = NO ]; then

    cat << END                                                >> $NGX_MAKEFILE

$ZLIB/libz.a:	$NGX_MAKEFILE
	cd $ZLIB \\
	&& \$(MAKE) distclean \\
	&& CFLAGS="$ZLIB_OPT" CC="\$(CC)" \\
		./configure \\
	&& \$(MAKE) libz.a

END

fi
{% endhighlight %}

我们当前```NGX_CC_NAME```被置为```gcc```，因此变量```ngx_makefile```为空。

我们在configure脚本中通过如下命令：
<pre>
NGX_SYSTEM=`uname -s 2>/dev/null`
NGX_RELEASE=`uname -r 2>/dev/null`
NGX_MACHINE=`uname -m 2>/dev/null`

echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";
</pre>
求得```NGX_PLATFORM```值为：
<pre>
# echo $NGX_PLATFORM
Linux:4.10.0-35-generic:i686
</pre>

此处，```ZLIB_ASM```在auto/options默认被初始化为```NO```，因此执行如下脚本：
{% highlight string %}
if [ $done = NO ]; then

    cat << END                                                >> $NGX_MAKEFILE

$ZLIB/libz.a:	$NGX_MAKEFILE
	cd $ZLIB \\
	&& \$(MAKE) distclean \\
	&& CFLAGS="$ZLIB_OPT" CC="\$(CC)" \\
		./configure \\
	&& \$(MAKE) libz.a

END
{% endhighlight %}

向objs/Makefile文件写入：
{% highlight string %}
../zlib-1.2.11/libz.a:	objs/Makefile
	cd ../zlib-1.2.11 \
	&& $(MAKE) distclean \
	&& CFLAGS="-O2 -fomit-frame-pointer -pipe " CC="$(CC)" \
		./configure \
	&& $(MAKE) libz.a
{% endhighlight %}










<br />
<br />
<br />

