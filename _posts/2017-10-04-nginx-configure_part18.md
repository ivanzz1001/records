---
layout: post
title: auto/make脚本分析-part18
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


auto/make文件是作为nginx生成Makefile脚本最主要文件。下面我们来分析一下该脚本：


<!-- more -->

## auto/make脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo "creating $NGX_MAKEFILE"

mkdir -p $NGX_OBJS/src/core $NGX_OBJS/src/event $NGX_OBJS/src/event/modules \
         $NGX_OBJS/src/os/unix $NGX_OBJS/src/os/win32 \
         $NGX_OBJS/src/http $NGX_OBJS/src/http/v2 $NGX_OBJS/src/http/modules \
         $NGX_OBJS/src/http/modules/perl \
         $NGX_OBJS/src/mail \
         $NGX_OBJS/src/stream \
         $NGX_OBJS/src/misc


ngx_objs_dir=$NGX_OBJS$ngx_regex_dirsep
ngx_use_pch=`echo $NGX_USE_PCH | sed -e "s/\//$ngx_regex_dirsep/g"`


cat << END                                                     > $NGX_MAKEFILE

CC =	$CC
CFLAGS = $CFLAGS
CPP =	$CPP
LINK =	$LINK

END


if test -n "$NGX_PERL_CFLAGS"; then
    echo NGX_PERL_CFLAGS = $NGX_PERL_CFLAGS                   >> $NGX_MAKEFILE
    echo NGX_PM_CFLAGS = $NGX_PM_CFLAGS                       >> $NGX_MAKEFILE
fi


# ALL_INCS, required by the addons and by OpenWatcom C precompiled headers

ngx_incs=`echo $CORE_INCS $NGX_OBJS $HTTP_INCS $MAIL_INCS $STREAM_INCS\
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

cat << END                                                    >> $NGX_MAKEFILE

ALL_INCS = $ngx_include_opt$ngx_incs

END


ngx_all_srcs="$CORE_SRCS"


# the core dependencies and include paths

ngx_deps=`echo $CORE_DEPS $NGX_AUTO_CONFIG_H $NGX_PCH \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

ngx_incs=`echo $CORE_INCS $NGX_OBJS \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

cat << END                                                    >> $NGX_MAKEFILE

CORE_DEPS = $ngx_deps


CORE_INCS = $ngx_include_opt$ngx_incs

END


# the http dependencies and include paths

if [ $HTTP = YES ]; then

    ngx_all_srcs="$ngx_all_srcs $HTTP_SRCS"

    ngx_deps=`echo $HTTP_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $HTTP_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

HTTP_DEPS = $ngx_deps


HTTP_INCS = $ngx_include_opt$ngx_incs

END

fi


# the mail dependencies and include paths

if [ $MAIL != NO ]; then

    if [ $MAIL = YES ]; then
        ngx_all_srcs="$ngx_all_srcs $MAIL_SRCS"
    fi

    ngx_deps=`echo $MAIL_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $MAIL_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

MAIL_DEPS = $ngx_deps


MAIL_INCS = $ngx_include_opt$ngx_incs

END

fi


# the stream dependencies and include paths

if [ $STREAM != NO ]; then

    if [ $STREAM = YES ]; then
        ngx_all_srcs="$ngx_all_srcs $STREAM_SRCS"
    fi

    ngx_deps=`echo $STREAM_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $STREAM_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

STREAM_DEPS = $ngx_deps


STREAM_INCS = $ngx_include_opt$ngx_incs

END

fi


ngx_all_srcs="$ngx_all_srcs $MISC_SRCS"


if test -n "$NGX_ADDON_SRCS"; then

cat << END                                                >> $NGX_MAKEFILE

ADDON_DEPS = \$(CORE_DEPS) $NGX_ADDON_DEPS

END

fi


# nginx

ngx_all_srcs=`echo $ngx_all_srcs | sed -e "s/\//$ngx_regex_dirsep/g"`

for ngx_src in $NGX_ADDON_SRCS
do
    ngx_obj="addon/`basename \`dirname $ngx_src\``"

    test -d $NGX_OBJS/$ngx_obj || mkdir -p $NGX_OBJS/$ngx_obj

    ngx_obj=`echo $ngx_obj/\`basename $ngx_src\` \
        | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_all_srcs="$ngx_all_srcs $ngx_obj"
done

ngx_all_objs=`echo $ngx_all_srcs \
    | sed -e "s#\([^ ]*\.\)cpp#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)cc#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)c#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)S#$NGX_OBJS\/\1$ngx_objext#g"`

ngx_modules_c=`echo $NGX_MODULES_C | sed -e "s/\//$ngx_regex_dirsep/g"`

ngx_modules_obj=`echo $ngx_modules_c | sed -e "s/\(.*\.\)c/\1$ngx_objext/"`


if test -n "$NGX_RES"; then
   ngx_res=$NGX_RES
else
   ngx_res="$NGX_RC $NGX_ICONS"
   ngx_rcc=`echo $NGX_RCC | sed -e "s/\//$ngx_regex_dirsep/g"`
fi

ngx_deps=`echo $ngx_all_objs $ngx_modules_obj $ngx_res $LINK_DEPS \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

ngx_objs=`echo $ngx_all_objs $ngx_modules_obj \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_long_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

ngx_libs=
if test -n "$NGX_LD_OPT$CORE_LIBS"; then
    ngx_libs=`echo $NGX_LD_OPT $CORE_LIBS \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`
fi

ngx_link=${CORE_LINK:+`echo $CORE_LINK \
    | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}

ngx_main_link=${MAIN_LINK:+`echo $MAIN_LINK \
    | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}


cat << END                                                    >> $NGX_MAKEFILE

build:	binary modules manpage

binary:	$NGX_OBJS${ngx_dirsep}nginx$ngx_binext

$NGX_OBJS${ngx_dirsep}nginx$ngx_binext:	$ngx_deps$ngx_spacer
	\$(LINK) $ngx_long_start$ngx_binout$NGX_OBJS${ngx_dirsep}nginx$ngx_long_cont$ngx_objs$ngx_libs$ngx_link$ngx_main_link
	$ngx_rcc
$ngx_long_end

modules:
END


# ngx_modules.c

if test -n "$NGX_PCH"; then
    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
else
    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS)"
fi

cat << END                                                    >> $NGX_MAKEFILE

$ngx_modules_obj:	\$(CORE_DEPS)$ngx_cont$ngx_modules_c
	$ngx_cc$ngx_tab$ngx_objout$ngx_modules_obj$ngx_tab$ngx_modules_c$NGX_AUX

END


# the core sources

for ngx_src in $CORE_SRCS
do
    ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
    ngx_obj=`echo $ngx_src \
        | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

done


# the http sources

if [ $HTTP = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(HTTP_INCS)"
        ngx_perl_cc="\$(CC) $ngx_compile_opt \$(NGX_PERL_CFLAGS)"
        ngx_perl_cc="$ngx_perl_cc \$(CORE_INCS) \$(HTTP_INCS)"
    fi

    for ngx_source in $HTTP_SRCS
    do
        ngx_src=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        if [ $ngx_source = src/http/modules/perl/ngx_http_perl_module.c ]; then

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(HTTP_DEPS)$ngx_cont$ngx_src
	$ngx_perl_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
        else

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(HTTP_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

        fi
     done

fi


# the mail sources

if [ $MAIL = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(MAIL_INCS)"
    fi

    for ngx_src in $MAIL_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(MAIL_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi


# the stream sources

if [ $STREAM = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(STREAM_INCS)"
    fi

    for ngx_src in $STREAM_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(STREAM_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi


# the misc sources

if test -n "$MISC_SRCS"; then

    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"

    for ngx_src in $MISC_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) $ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi


# the addons sources

if test -n "$NGX_ADDON_SRCS"; then

    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"

    for ngx_src in $NGX_ADDON_SRCS
    do
        ngx_obj="addon/`basename \`dirname $ngx_src\``"

        ngx_obj=`echo $ngx_obj/\`basename $ngx_src\` \
            | sed -e "s/\//$ngx_regex_dirsep/g"`

        ngx_obj=`echo $ngx_obj \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(ADDON_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi


# the addons config.make

if test -n "$NGX_ADDONS$DYNAMIC_ADDONS"; then

    for ngx_addon_dir in $NGX_ADDONS $DYNAMIC_ADDONS
    do
        if test -f $ngx_addon_dir/config.make; then
            . $ngx_addon_dir/config.make
        fi
    done
fi


# Win32 resource file

if test -n "$NGX_RES"; then

    ngx_res=`echo "$NGX_RES:	$NGX_RC $NGX_ICONS" \
                 | sed -e "s/\//$ngx_regex_dirsep/g"`
    ngx_rcc=`echo $NGX_RCC | sed -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_res
	$ngx_rcc

END

fi


# the precompiled headers

if test -n "$NGX_PCH"; then
    echo "#include <ngx_config.h>" > $NGX_OBJS/ngx_pch.c

    ngx_pch="src/core/ngx_config.h $OS_CONFIG $NGX_OBJS/ngx_auto_config.h"
    ngx_pch=`echo "$NGX_PCH:	$ngx_pch" | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_src="\$(CC) \$(CFLAGS) $NGX_BUILD_PCH $ngx_compile_opt \$(ALL_INCS)"
    ngx_src="$ngx_src $ngx_objout$NGX_OBJS/ngx_pch.obj $NGX_OBJS/ngx_pch.c"
    ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_pch
	$ngx_src

END

fi


# dynamic modules

if test -n "$NGX_PCH"; then
    ngx_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
else
    ngx_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(CFLAGS) \$(ALL_INCS)"
    ngx_perl_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(NGX_PERL_CFLAGS)"
    ngx_perl_cc="$ngx_perl_cc \$(ALL_INCS)"
fi

ngx_obj_deps="\$(CORE_DEPS)"
if [ $HTTP != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(HTTP_DEPS)"
fi
if [ $MAIL != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(MAIL_DEPS)"
fi
if [ $STREAM != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(STREAM_DEPS)"
fi

for ngx_module in $DYNAMIC_MODULES
do
    eval ngx_module_srcs="\$${ngx_module}_SRCS"
    eval eval ngx_module_libs="\\\"\$${ngx_module}_LIBS\\\""

    eval ngx_module_modules="\$${ngx_module}_MODULES"
    eval ngx_module_order="\$${ngx_module}_ORDER"

    ngx_modules_c=$NGX_OBJS/${ngx_module}_modules.c

    cat << END                                    > $ngx_modules_c

#include <ngx_config.h>
#include <ngx_core.h>

END

    for mod in $ngx_module_modules
    do
        echo "extern ngx_module_t  $mod;"         >> $ngx_modules_c
    done

    echo                                          >> $ngx_modules_c
    echo 'ngx_module_t *ngx_modules[] = {'        >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    &$mod,"                         >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_names[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_order[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_order
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    ngx_modules_c=`echo $ngx_modules_c | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_modules_obj=`echo $ngx_modules_c \
        | sed -e "s/\(.*\.\)c/\1$ngx_objext/"`

    ngx_module_objs=
    for ngx_src in $ngx_module_srcs
    do
        case "$ngx_src" in
            src/*)
                ngx_obj=$ngx_src
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_src\``"
                mkdir -p $NGX_OBJS/$ngx_obj
                ngx_obj="$ngx_obj/`basename $ngx_src`"
                ;;
        esac

        ngx_module_objs="$ngx_module_objs $ngx_obj"
    done

    ngx_module_objs=`echo $ngx_module_objs \
        | sed -e "s#\([^ ]*\.\)cpp#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)cc#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)c#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)S#$NGX_OBJS\/\1$ngx_objext#g"`

    ngx_deps=`echo $ngx_module_objs $ngx_modules_obj $LINK_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_objs=`echo $ngx_module_objs $ngx_modules_obj \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_long_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_obj=$NGX_OBJS$ngx_dirsep$ngx_module$ngx_modext

    if [ "$NGX_PLATFORM" = win32 ]; then
        ngx_module_libs="$CORE_LIBS $ngx_module_libs"
    fi

    ngx_libs=
    if test -n "$NGX_LD_OPT$ngx_module_libs"; then
        ngx_libs=`echo $NGX_LD_OPT $ngx_module_libs \
            | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`
    fi

    ngx_link=${CORE_LINK:+`echo $CORE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}

    ngx_module_link=${MODULE_LINK:+`echo $MODULE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}


    cat << END                                            >> $NGX_MAKEFILE

modules:	$ngx_obj

$ngx_obj:	$ngx_deps$ngx_spacer
	\$(LINK) $ngx_long_start$ngx_binout$ngx_obj$ngx_long_cont$ngx_objs$ngx_libs$ngx_link$ngx_module_link
$ngx_long_end

$ngx_modules_obj:	\$(CORE_DEPS)$ngx_cont$ngx_modules_c
	$ngx_cc$ngx_tab$ngx_objout$ngx_modules_obj$ngx_tab$ngx_modules_c$NGX_AUX

END

    for ngx_source in $ngx_module_srcs
    do
        case "$ngx_source" in
            src/*)
                ngx_obj=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_source\``"
                ngx_obj=`echo $ngx_obj/\`basename $ngx_source\` \
                    | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
        esac

        ngx_obj=`echo $ngx_obj \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        ngx_src=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`

        if [ $ngx_source = src/http/modules/perl/ngx_http_perl_module.c ]; then

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_perl_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
        else

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

        fi
    done
done

{% endhighlight %}


**1) 构造相应目录**
{% highlight string %}
echo "creating $NGX_MAKEFILE"

mkdir -p $NGX_OBJS/src/core $NGX_OBJS/src/event $NGX_OBJS/src/event/modules \
         $NGX_OBJS/src/os/unix $NGX_OBJS/src/os/win32 \
         $NGX_OBJS/src/http $NGX_OBJS/src/http/v2 $NGX_OBJS/src/http/modules \
         $NGX_OBJS/src/http/modules/perl \
         $NGX_OBJS/src/mail \
         $NGX_OBJS/src/stream \
         $NGX_OBJS/src/misc


ngx_objs_dir=$NGX_OBJS$ngx_regex_dirsep
ngx_use_pch=`echo $NGX_USE_PCH | sed -e "s/\//$ngx_regex_dirsep/g"`
{% endhighlight %}

这里```NGX_MAKEFILE```在auto/init脚本中被赋值为objs/Makefile。

在auto/cc/conf脚本中ngx_regex_dirsep="\/",即斜杠目录分隔符。

```NGX_USE_PCH```为空，因此ngx_use_pch也为空。

<br />

**2) 设置编译、链接器**
{% highlight string %}
cat << END                                                     > $NGX_MAKEFILE

CC =	$CC
CFLAGS = $CFLAGS
CPP =	$CPP
LINK =	$LINK

END
{% endhighlight %}

这里```$CC```为在auto/options脚本中被置为```cc```。

```CFLAGS```在auto/cc/gcc脚本中被置为：
<pre>
CFLAGS =  -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g 
</pre>

```CPP```在auto/cc/gcc脚本中被设置为```cc -E```。

```LINK```在auto/cc/conf脚本中被设置为```$(CC)```。

<br />


**3) 处理PERL CFLAGS**
{% highlight string %}
if test -n "$NGX_PERL_CFLAGS"; then
    echo NGX_PERL_CFLAGS = $NGX_PERL_CFLAGS                   >> $NGX_MAKEFILE
    echo NGX_PM_CFLAGS = $NGX_PM_CFLAGS                       >> $NGX_MAKEFILE
fi
{% endhighlight %}
```NGX_PERL_CFLAGS```为空，不进行处理。

<br />


**4) 处理依赖的头文件**
{% highlight string %}
# ALL_INCS, required by the addons and by OpenWatcom C precompiled headers

ngx_incs=`echo $CORE_INCS $NGX_OBJS $HTTP_INCS $MAIL_INCS $STREAM_INCS\
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

cat << END                                                    >> $NGX_MAKEFILE

ALL_INCS = $ngx_include_opt$ngx_incs

END


ngx_all_srcs="$CORE_SRCS"


# the core dependencies and include paths

ngx_deps=`echo $CORE_DEPS $NGX_AUTO_CONFIG_H $NGX_PCH \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

ngx_incs=`echo $CORE_INCS $NGX_OBJS \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

cat << END                                                    >> $NGX_MAKEFILE

CORE_DEPS = $ngx_deps


CORE_INCS = $ngx_include_opt$ngx_incs

END
{% endhighlight %}

我们先来看如下脚本：
<pre>
sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" 
</pre>
该脚本的含义为匹配任意两个空格之间的字符，且这两个空格之间本身不能为空格，这样讲匹配到的结果赋值给```\1```。例如：
<pre>
# str=`echo "fff ggg   qqq" | sed -e "s/  *\([^ ][^ ]*\)/-I\1/g"`
# echo $str
fff-Iggg-Iqqq
</pre>

另外在auto/cc/conf脚本中：
{% highlight string %}
ngx_include_opt="-I "

ngx_regex_dirsep="\/"

ngx_regex_cont=' \\\
	'
{% endhighlight %}
上述ngx_regex_cont变量中，第一、第二个反斜杠表示对```\```进行转义，使得第二个反斜杠作为普通的字符使用。最后一个反斜杠作为shell脚本中连接下一行使用。


```NGX_OBJS```值为objs。


最后生成的各变量情况如下：
{% highlight string %}
ALL_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I ../pcre-8.40 \
	-I ../zlib-1.2.11 \
	-I objs \
	-I src/http \
	-I src/http/modules


CORE_DEPS = src/core/nginx.h \
	src/core/ngx_config.h \
	src/core/ngx_core.h \
	src/core/ngx_log.h \
	src/core/ngx_palloc.h \
	src/core/ngx_array.h \
	src/core/ngx_list.h \
	src/core/ngx_hash.h \
	src/core/ngx_buf.h \
	src/core/ngx_queue.h \
	src/core/ngx_string.h \
	src/core/ngx_parse.h \
	src/core/ngx_parse_time.h \
	src/core/ngx_inet.h \
	src/core/ngx_file.h \
	src/core/ngx_crc.h \
	src/core/ngx_crc32.h \
	src/core/ngx_murmurhash.h \
	src/core/ngx_md5.h \
	src/core/ngx_sha1.h \
	src/core/ngx_rbtree.h \
	src/core/ngx_radix_tree.h \
	src/core/ngx_rwlock.h \
	src/core/ngx_slab.h \
	src/core/ngx_times.h \
	src/core/ngx_shmtx.h \
	src/core/ngx_connection.h \
	src/core/ngx_cycle.h \
	src/core/ngx_conf_file.h \
	src/core/ngx_module.h \
	src/core/ngx_resolver.h \
	src/core/ngx_open_file_cache.h \
	src/core/ngx_crypt.h \
	src/core/ngx_proxy_protocol.h \
	src/core/ngx_syslog.h \
	src/event/ngx_event.h \
	src/event/ngx_event_timer.h \
	src/event/ngx_event_posted.h \
	src/event/ngx_event_connect.h \
	src/event/ngx_event_pipe.h \
	src/os/unix/ngx_time.h \
	src/os/unix/ngx_errno.h \
	src/os/unix/ngx_alloc.h \
	src/os/unix/ngx_files.h \
	src/os/unix/ngx_channel.h \
	src/os/unix/ngx_shmem.h \
	src/os/unix/ngx_process.h \
	src/os/unix/ngx_setaffinity.h \
	src/os/unix/ngx_setproctitle.h \
	src/os/unix/ngx_atomic.h \
	src/os/unix/ngx_gcc_atomic_x86.h \
	src/os/unix/ngx_thread.h \
	src/os/unix/ngx_socket.h \
	src/os/unix/ngx_os.h \
	src/os/unix/ngx_user.h \
	src/os/unix/ngx_dlopen.h \
	src/os/unix/ngx_process_cycle.h \
	src/os/unix/ngx_linux_config.h \
	src/os/unix/ngx_linux.h \
	src/event/ngx_event_openssl.h \
	src/core/ngx_regex.h \
	../pcre-8.40/pcre.h \
	objs/ngx_auto_config.h


CORE_INCS = -I src/core \
	-I src/event \
	-I src/event/modules \
	-I src/os/unix \
	-I ../pcre-8.40 \
	-I ../zlib-1.2.11 \
	-I objs
{% endhighlight %}


从上面我们看到，```ALL_INCS```相对于```CORE_INCS```多包含了http相关的头文件路径。

<br />

**5) 处理http相关依赖**
{% highlight string %}
# the http dependencies and include paths

if [ $HTTP = YES ]; then

    ngx_all_srcs="$ngx_all_srcs $HTTP_SRCS"

    ngx_deps=`echo $HTTP_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $HTTP_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

HTTP_DEPS = $ngx_deps


HTTP_INCS = $ngx_include_opt$ngx_incs

END

fi
{% endhighlight %}
当前```HTTP```为```YES```。最后生成的各变量情况如下：
{% highlight string %}
HTTP_DEPS = src/http/ngx_http.h \
	src/http/ngx_http_request.h \
	src/http/ngx_http_config.h \
	src/http/ngx_http_core_module.h \
	src/http/ngx_http_cache.h \
	src/http/ngx_http_variables.h \
	src/http/ngx_http_script.h \
	src/http/ngx_http_upstream.h \
	src/http/ngx_http_upstream_round_robin.h \
	src/http/modules/ngx_http_ssi_filter_module.h \
	src/http/modules/ngx_http_ssl_module.h


HTTP_INCS = -I src/http \
	-I src/http/modules
{% endhighlight %}

<br />

**6) 处理MAIL相关依赖项**
{% highlight string %}
# the mail dependencies and include paths

if [ $MAIL != NO ]; then

    if [ $MAIL = YES ]; then
        ngx_all_srcs="$ngx_all_srcs $MAIL_SRCS"
    fi

    ngx_deps=`echo $MAIL_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $MAIL_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

MAIL_DEPS = $ngx_deps


MAIL_INCS = $ngx_include_opt$ngx_incs

END

fi
{% endhighlight %}

当前```MAIL```在auto/options脚本中默认被设置为```NO```，我们也并未手动启用。

<br />

**7) 处理STREAM相关依赖项**
{% highlight string %}
# the stream dependencies and include paths

if [ $STREAM != NO ]; then

    if [ $STREAM = YES ]; then
        ngx_all_srcs="$ngx_all_srcs $STREAM_SRCS"
    fi

    ngx_deps=`echo $STREAM_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_incs=`echo $STREAM_INCS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont$ngx_include_opt\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

STREAM_DEPS = $ngx_deps


STREAM_INCS = $ngx_include_opt$ngx_incs

END

fi
{% endhighlight %}

当前```STREAM```在auto/options脚本中默认被设置为```NO```,且我们也并未手动启用。

<br />

**8) 处理MISC_SRCS及ADDON_SRCS**
{% highlight string %}
ngx_all_srcs="$ngx_all_srcs $MISC_SRCS"


if test -n "$NGX_ADDON_SRCS"; then

cat << END                                                >> $NGX_MAKEFILE

ADDON_DEPS = \$(CORE_DEPS) $NGX_ADDON_DEPS

END

fi
{% endhighlight %}

当前```MISC_SRCS```及```NGX_ADDON_SRCS```均为空。

<br />

**9) 处理Nginx相关源文件、依赖文件、库文件**

{% highlight string %}
# nginx

ngx_all_srcs=`echo $ngx_all_srcs | sed -e "s/\//$ngx_regex_dirsep/g"`

for ngx_src in $NGX_ADDON_SRCS
do
    ngx_obj="addon/`basename \`dirname $ngx_src\``"

    test -d $NGX_OBJS/$ngx_obj || mkdir -p $NGX_OBJS/$ngx_obj

    ngx_obj=`echo $ngx_obj/\`basename $ngx_src\` \
        | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_all_srcs="$ngx_all_srcs $ngx_obj"
done
{% endhighlight %}
上面首先处理所有源文件的目录分隔符，然后对```NGX_ADDON_SRCS```进行遍历，生成对应的编译目录，并将相应的编译目录添加到ngx_all_srcs中。

<br />

{% highlight string %}
ngx_all_objs=`echo $ngx_all_srcs \
    | sed -e "s#\([^ ]*\.\)cpp#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)cc#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)c#$NGX_OBJS\/\1$ngx_objext#g" \
          -e "s#\([^ ]*\.\)S#$NGX_OBJS\/\1$ngx_objext#g"`
{% endhighlight %}
将ngx_all_srcs目录下后缀名为```.cpp```,```.cc```,```.c```,```.S```文件替换成```.0```后缀保存在ngx_all_objs变量中。我们在auto/cc/conf脚本中将```ngx_object```定义为```o```。

<br />

{% highlight string %}
ngx_modules_c=`echo $NGX_MODULES_C | sed -e "s/\//$ngx_regex_dirsep/g"`

ngx_modules_obj=`echo $ngx_modules_c | sed -e "s/\(.*\.\)c/\1$ngx_objext/"`
{% endhighlight %}
我们在auto/init脚本中将```NGX_MODULES_C```定义为：
<pre>
NGX_MODULES_C=$NGX_OBJS/ngx_modules.c
</pre>
这里注意```sed -e "s/\(.*\.\)c/\1$ngx_objext/```,其中```.*```会在第一个匹配后在匹配0个或多个字符,这可以处理类似于test.helloworld.c这样的文件名。

<br />


{% highlight string %}
if test -n "$NGX_RES"; then
   ngx_res=$NGX_RES
else
   ngx_res="$NGX_RC $NGX_ICONS"
   ngx_rcc=`echo $NGX_RCC | sed -e "s/\//$ngx_regex_dirsep/g"`
fi
{% endhighlight %}

此处```NGX_RES```,```NGX_RC```,```NGX_ICONS```,```NGX_RCC```均为空。

<br />

{% highlight string %}
ngx_deps=`echo $ngx_all_objs $ngx_modules_obj $ngx_res $LINK_DEPS \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`

ngx_objs=`echo $ngx_all_objs $ngx_modules_obj \
    | sed -e "s/  *\([^ ][^ ]*\)/$ngx_long_regex_cont\1/g" \
          -e "s/\//$ngx_regex_dirsep/g"`
{% endhighlight %}
ngx_deps变量保存所有生成nginx所需要依赖的```.o```文件,```.a```库文件；而ngx_objs变量保存所有```.o```文件.

<br />

{% highlight string %}
ngx_libs=
if test -n "$NGX_LD_OPT$CORE_LIBS"; then
    ngx_libs=`echo $NGX_LD_OPT $CORE_LIBS \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`
fi

ngx_link=${CORE_LINK:+`echo $CORE_LINK \
    | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}

ngx_main_link=${MAIN_LINK:+`echo $MAIN_LINK \
    | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}
{% endhighlight %}

上面处理ngx_libs，ngx_link,ngx_main_link。当前```CORE_LINK```值为空；```MAIN_LINK```值为```-Wl,-E```。


这里注意：
<pre>
sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"
</pre>
后半段的含义为在一行的开始加上```$ngx_long_regex_cont```。

<br />

**10) Makefile中生成build、binary等target**
{% highlight string %}
cat << END                                                    >> $NGX_MAKEFILE

build:	binary modules manpage

binary:	$NGX_OBJS${ngx_dirsep}nginx$ngx_binext

$NGX_OBJS${ngx_dirsep}nginx$ngx_binext:	$ngx_deps$ngx_spacer
	\$(LINK) $ngx_long_start$ngx_binout$NGX_OBJS${ngx_dirsep}nginx$ngx_long_cont$ngx_objs$ngx_libs$ngx_link$ngx_main_link
	$ngx_rcc
$ngx_long_end

modules:
END
{% endhighlight %}

其中```NGX_OBJS```值为objs;```ngx_binout```在auto/cc/conf脚本中被设置为```-o ```。最后给出具体的生成结果以做参考：
<pre>
build:	binary modules manpage

binary:	objs/nginx

objs/nginx:	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/src/core/ngx_palloc.o \
	objs/src/core/ngx_array.o \
	objs/src/core/ngx_list.o \
	objs/src/core/ngx_hash.o \
	objs/src/core/ngx_buf.o \
	objs/src/core/ngx_queue.o \
	objs/src/core/ngx_output_chain.o \
	objs/src/core/ngx_string.o \
	objs/src/core/ngx_parse.o \
	objs/src/core/ngx_parse_time.o \
	objs/src/core/ngx_inet.o \
	objs/src/core/ngx_file.o \
	objs/src/core/ngx_crc32.o \
	objs/src/core/ngx_murmurhash.o \
	objs/src/core/ngx_md5.o \
	objs/src/core/ngx_rbtree.o \
	objs/src/core/ngx_radix_tree.o \
	objs/src/core/ngx_slab.o \
	objs/src/core/ngx_times.o \
	objs/src/core/ngx_shmtx.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_spinlock.o \
	objs/src/core/ngx_rwlock.o \
	objs/src/core/ngx_cpuinfo.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/core/ngx_module.o \
	objs/src/core/ngx_resolver.o \
	objs/src/core/ngx_open_file_cache.o \
	objs/src/core/ngx_crypt.o \
	objs/src/core/ngx_proxy_protocol.o \
	objs/src/core/ngx_syslog.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/event/ngx_event_connect.o \
	objs/src/event/ngx_event_pipe.o \
	objs/src/os/unix/ngx_time.o \
	objs/src/os/unix/ngx_errno.o \
	objs/src/os/unix/ngx_alloc.o \
	objs/src/os/unix/ngx_files.o \
	objs/src/os/unix/ngx_socket.o \
	objs/src/os/unix/ngx_recv.o \
	objs/src/os/unix/ngx_readv_chain.o \
	objs/src/os/unix/ngx_udp_recv.o \
	objs/src/os/unix/ngx_send.o \
	objs/src/os/unix/ngx_writev_chain.o \
	objs/src/os/unix/ngx_udp_send.o \
	objs/src/os/unix/ngx_channel.o \
	objs/src/os/unix/ngx_shmem.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_setproctitle.o \
	objs/src/os/unix/ngx_posix_init.o \
	objs/src/os/unix/ngx_user.o \
	objs/src/os/unix/ngx_dlopen.o \
	objs/src/os/unix/ngx_process_cycle.o \
	objs/src/os/unix/ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/os/unix/ngx_linux_sendfile_chain.o \
	objs/src/event/ngx_event_openssl.o \
	objs/src/event/ngx_event_openssl_stapling.o \
	objs/src/core/ngx_regex.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/ngx_http_special_response.o \
	objs/src/http/ngx_http_request.o \
	objs/src/http/ngx_http_parse.o \
	objs/src/http/modules/ngx_http_log_module.o \
	objs/src/http/ngx_http_request_body.o \
	objs/src/http/ngx_http_variables.o \
	objs/src/http/ngx_http_script.o \
	objs/src/http/ngx_http_upstream.o \
	objs/src/http/ngx_http_upstream_round_robin.o \
	objs/src/http/ngx_http_file_cache.o \
	objs/src/http/ngx_http_write_filter_module.o \
	objs/src/http/ngx_http_header_filter_module.o \
	objs/src/http/modules/ngx_http_chunked_filter_module.o \
	objs/src/http/modules/ngx_http_range_filter_module.o \
	objs/src/http/modules/ngx_http_gzip_filter_module.o \
	objs/src/http/ngx_http_postpone_filter_module.o \
	objs/src/http/modules/ngx_http_ssi_filter_module.o \
	objs/src/http/modules/ngx_http_charset_filter_module.o \
	objs/src/http/modules/ngx_http_userid_filter_module.o \
	objs/src/http/modules/ngx_http_headers_filter_module.o \
	objs/src/http/ngx_http_copy_filter_module.o \
	objs/src/http/modules/ngx_http_not_modified_filter_module.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/http/modules/ngx_http_autoindex_module.o \
	objs/src/http/modules/ngx_http_index_module.o \
	objs/src/http/modules/ngx_http_auth_basic_module.o \
	objs/src/http/modules/ngx_http_access_module.o \
	objs/src/http/modules/ngx_http_limit_conn_module.o \
	objs/src/http/modules/ngx_http_limit_req_module.o \
	objs/src/http/modules/ngx_http_geo_module.o \
	objs/src/http/modules/ngx_http_map_module.o \
	objs/src/http/modules/ngx_http_split_clients_module.o \
	objs/src/http/modules/ngx_http_referer_module.o \
	objs/src/http/modules/ngx_http_rewrite_module.o \
	objs/src/http/modules/ngx_http_ssl_module.o \
	objs/src/http/modules/ngx_http_proxy_module.o \
	objs/src/http/modules/ngx_http_fastcgi_module.o \
	objs/src/http/modules/ngx_http_uwsgi_module.o \
	objs/src/http/modules/ngx_http_scgi_module.o \
	objs/src/http/modules/ngx_http_memcached_module.o \
	objs/src/http/modules/ngx_http_empty_gif_module.o \
	objs/src/http/modules/ngx_http_browser_module.o \
	objs/src/http/modules/ngx_http_upstream_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
	objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
	objs/src/http/modules/ngx_http_upstream_zone_module.o \
	objs/ngx_modules.o \
	../pcre-8.40/.libs/libpcre.a \
	../zlib-1.2.11/libz.a

	$(LINK) -o objs/nginx \
	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/src/core/ngx_palloc.o \
	objs/src/core/ngx_array.o \
	objs/src/core/ngx_list.o \
	objs/src/core/ngx_hash.o \
	objs/src/core/ngx_buf.o \
	objs/src/core/ngx_queue.o \
	objs/src/core/ngx_output_chain.o \
	objs/src/core/ngx_string.o \
	objs/src/core/ngx_parse.o \
	objs/src/core/ngx_parse_time.o \
	objs/src/core/ngx_inet.o \
	objs/src/core/ngx_file.o \
	objs/src/core/ngx_crc32.o \
	objs/src/core/ngx_murmurhash.o \
	objs/src/core/ngx_md5.o \
	objs/src/core/ngx_rbtree.o \
	objs/src/core/ngx_radix_tree.o \
	objs/src/core/ngx_slab.o \
	objs/src/core/ngx_times.o \
	objs/src/core/ngx_shmtx.o \
	objs/src/core/ngx_connection.o \
	objs/src/core/ngx_cycle.o \
	objs/src/core/ngx_spinlock.o \
	objs/src/core/ngx_rwlock.o \
	objs/src/core/ngx_cpuinfo.o \
	objs/src/core/ngx_conf_file.o \
	objs/src/core/ngx_module.o \
	objs/src/core/ngx_resolver.o \
	objs/src/core/ngx_open_file_cache.o \
	objs/src/core/ngx_crypt.o \
	objs/src/core/ngx_proxy_protocol.o \
	objs/src/core/ngx_syslog.o \
	objs/src/event/ngx_event.o \
	objs/src/event/ngx_event_timer.o \
	objs/src/event/ngx_event_posted.o \
	objs/src/event/ngx_event_accept.o \
	objs/src/event/ngx_event_connect.o \
	objs/src/event/ngx_event_pipe.o \
	objs/src/os/unix/ngx_time.o \
	objs/src/os/unix/ngx_errno.o \
	objs/src/os/unix/ngx_alloc.o \
	objs/src/os/unix/ngx_files.o \
	objs/src/os/unix/ngx_socket.o \
	objs/src/os/unix/ngx_recv.o \
	objs/src/os/unix/ngx_readv_chain.o \
	objs/src/os/unix/ngx_udp_recv.o \
	objs/src/os/unix/ngx_send.o \
	objs/src/os/unix/ngx_writev_chain.o \
	objs/src/os/unix/ngx_udp_send.o \
	objs/src/os/unix/ngx_channel.o \
	objs/src/os/unix/ngx_shmem.o \
	objs/src/os/unix/ngx_process.o \
	objs/src/os/unix/ngx_daemon.o \
	objs/src/os/unix/ngx_setaffinity.o \
	objs/src/os/unix/ngx_setproctitle.o \
	objs/src/os/unix/ngx_posix_init.o \
	objs/src/os/unix/ngx_user.o \
	objs/src/os/unix/ngx_dlopen.o \
	objs/src/os/unix/ngx_process_cycle.o \
	objs/src/os/unix/ngx_linux_init.o \
	objs/src/event/modules/ngx_epoll_module.o \
	objs/src/os/unix/ngx_linux_sendfile_chain.o \
	objs/src/event/ngx_event_openssl.o \
	objs/src/event/ngx_event_openssl_stapling.o \
	objs/src/core/ngx_regex.o \
	objs/src/http/ngx_http.o \
	objs/src/http/ngx_http_core_module.o \
	objs/src/http/ngx_http_special_response.o \
	objs/src/http/ngx_http_request.o \
	objs/src/http/ngx_http_parse.o \
	objs/src/http/modules/ngx_http_log_module.o \
	objs/src/http/ngx_http_request_body.o \
	objs/src/http/ngx_http_variables.o \
	objs/src/http/ngx_http_script.o \
	objs/src/http/ngx_http_upstream.o \
	objs/src/http/ngx_http_upstream_round_robin.o \
	objs/src/http/ngx_http_file_cache.o \
	objs/src/http/ngx_http_write_filter_module.o \
	objs/src/http/ngx_http_header_filter_module.o \
	objs/src/http/modules/ngx_http_chunked_filter_module.o \
	objs/src/http/modules/ngx_http_range_filter_module.o \
	objs/src/http/modules/ngx_http_gzip_filter_module.o \
	objs/src/http/ngx_http_postpone_filter_module.o \
	objs/src/http/modules/ngx_http_ssi_filter_module.o \
	objs/src/http/modules/ngx_http_charset_filter_module.o \
	objs/src/http/modules/ngx_http_userid_filter_module.o \
	objs/src/http/modules/ngx_http_headers_filter_module.o \
	objs/src/http/ngx_http_copy_filter_module.o \
	objs/src/http/modules/ngx_http_not_modified_filter_module.o \
	objs/src/http/modules/ngx_http_static_module.o \
	objs/src/http/modules/ngx_http_autoindex_module.o \
	objs/src/http/modules/ngx_http_index_module.o \
	objs/src/http/modules/ngx_http_auth_basic_module.o \
	objs/src/http/modules/ngx_http_access_module.o \
	objs/src/http/modules/ngx_http_limit_conn_module.o \
	objs/src/http/modules/ngx_http_limit_req_module.o \
	objs/src/http/modules/ngx_http_geo_module.o \
	objs/src/http/modules/ngx_http_map_module.o \
	objs/src/http/modules/ngx_http_split_clients_module.o \
	objs/src/http/modules/ngx_http_referer_module.o \
	objs/src/http/modules/ngx_http_rewrite_module.o \
	objs/src/http/modules/ngx_http_ssl_module.o \
	objs/src/http/modules/ngx_http_proxy_module.o \
	objs/src/http/modules/ngx_http_fastcgi_module.o \
	objs/src/http/modules/ngx_http_uwsgi_module.o \
	objs/src/http/modules/ngx_http_scgi_module.o \
	objs/src/http/modules/ngx_http_memcached_module.o \
	objs/src/http/modules/ngx_http_empty_gif_module.o \
	objs/src/http/modules/ngx_http_browser_module.o \
	objs/src/http/modules/ngx_http_upstream_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
	objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
	objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
	objs/src/http/modules/ngx_http_upstream_zone_module.o \
	objs/ngx_modules.o \
	-ldl -lpthread -lcrypt ../pcre-8.40/.libs/libpcre.a -lssl -lcrypto -ldl ../zlib-1.2.11/libz.a \
	-Wl,-E
</pre>


<br />


**11) 处理ngx_modules.c**
{% highlight string %}
# ngx_modules.c

if test -n "$NGX_PCH"; then
    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
else
    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS)"
fi

cat << END                                                    >> $NGX_MAKEFILE

$ngx_modules_obj:	\$(CORE_DEPS)$ngx_cont$ngx_modules_c
	$ngx_cc$ngx_tab$ngx_objout$ngx_modules_obj$ngx_tab$ngx_modules_c$NGX_AUX

END
{% endhighlight %}
```NGX_PCH```在auto/init脚本中被初始化为空，并且在我们当前环境中也未在其他脚本对其做相应修改。

最后给出具体的生成结果以作参考：
<pre>
objs/ngx_modules.o:	$(CORE_DEPS) \
	objs/ngx_modules.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/ngx_modules.o \
		objs/ngx_modules.c
</pre>


**12) 生成编译CORE_SRCS源文件的编译代码**
{% highlight string %}
# the core sources

for ngx_src in $CORE_SRCS
do
    ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
    ngx_obj=`echo $ngx_src \
        | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
              -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

done
{% endhighlight %}
遍历```CORE_SRC```目录下的每一个以```.cpp```,```.cc```,```.c```,```.S```结尾的源文件，然后生成编译该源文件的代码;```ngx_objout```在auto/cc/conf脚本中被定义为```-o ```;```ngx_tab```在auto/cc/conf脚本中被定义为：
<pre>
ngx_tab=' \
		'
</pre>
最后我们给出一个示例(src/core/nginx.c)：
<pre>
objs/src/core/nginx.o:	$(CORE_DEPS) \
	src/core/nginx.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) \
		-o objs/src/core/nginx.o \
		src/core/nginx.c
</pre>

<br />


**13) 生成编译HTTP源文件的代码**
{% highlight string %}
# the http sources

if [ $HTTP = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(HTTP_INCS)"
        ngx_perl_cc="\$(CC) $ngx_compile_opt \$(NGX_PERL_CFLAGS)"
        ngx_perl_cc="$ngx_perl_cc \$(CORE_INCS) \$(HTTP_INCS)"
    fi

    for ngx_source in $HTTP_SRCS
    do
        ngx_src=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        if [ $ngx_source = src/http/modules/perl/ngx_http_perl_module.c ]; then

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(HTTP_DEPS)$ngx_cont$ngx_src
	$ngx_perl_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
        else

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(HTTP_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

        fi
     done

fi
{% endhighlight %}
此处```HTTP```在auto/options脚本中被默认设置为```YES```.然后针对HTTP模块分别生成了ngx_cc与ngx_perl_cc。

遍历```HTTP_SRC```目录下的所有源文件，然后生成编译脚本。这里给出一个示例：
<pre>
objs/src/http/ngx_http.o:	$(CORE_DEPS) $(HTTP_DEPS) \
	src/http/ngx_http.c
	$(CC) -c $(CFLAGS) $(CORE_INCS) $(HTTP_INCS) \
		-o objs/src/http/ngx_http.o \
		src/http/ngx_http.c
</pre>

<br />

**14) 生成编译MAIL模块源文件的代码**
{% highlight string %}
# the mail sources

if [ $MAIL = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(MAIL_INCS)"
    fi

    for ngx_src in $MAIL_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(MAIL_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi
{% endhighlight %}
当前```MAIL```模块并未启用。

<br />


**15) 生成编译STREAM模块源文件的代码**
{% highlight string %}
# the stream sources

if [ $STREAM = YES ]; then

    if test -n "$NGX_PCH"; then
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
    else
        ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) \$(CORE_INCS) \$(STREAM_INCS)"
    fi

    for ngx_src in $STREAM_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) \$(STREAM_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi

{% endhighlight %}
当前```STREAM```模块并未启用。

<br />

**16) 生成编译MISC模块源文件的代码**
{% highlight string %}
# the misc sources

if test -n "$MISC_SRCS"; then

    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"

    for ngx_src in $MISC_SRCS
    do
        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`
        ngx_obj=`echo $ngx_src \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(CORE_DEPS) $ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi
{% endhighlight %}

当前```MISC_SRCS```为空，因此本段代码其实并不执行。

<br />

**17) 生成编译ADDON_SRCS目录源文件的代码**
{% highlight string %}
# the addons sources

if test -n "$NGX_ADDON_SRCS"; then

    ngx_cc="\$(CC) $ngx_compile_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"

    for ngx_src in $NGX_ADDON_SRCS
    do
        ngx_obj="addon/`basename \`dirname $ngx_src\``"

        ngx_obj=`echo $ngx_obj/\`basename $ngx_src\` \
            | sed -e "s/\//$ngx_regex_dirsep/g"`

        ngx_obj=`echo $ngx_obj \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`

        cat << END                                            >> $NGX_MAKEFILE

$ngx_obj:	\$(ADDON_DEPS)$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
     done

fi
{% endhighlight %}

此脚本主要针对外部静态模块。外部静态模块一般提供config脚本，config脚本调用auto/module完成相应的配置，将需要编译的源文件等存放在```NGX_ADDON_SRCS```变量中。

此处可能还特别针对那些不是通过```--add-module=*```方式传入的外部静态模块。

当前我们并没有添加任何外部静态模块。


<br />


**19) 处理外部静/动态模块配置**
{% highlight string %}
# the addons config.make

if test -n "$NGX_ADDONS$DYNAMIC_ADDONS"; then

    for ngx_addon_dir in $NGX_ADDONS $DYNAMIC_ADDONS
    do
        if test -f $ngx_addon_dir/config.make; then
            . $ngx_addon_dir/config.make
        fi
    done
fi
{% endhighlight %}
针对外部静/动态模块，在进行编译之前可能会需要进行相应的配置，此处执行相应目录的config.make配置文件。

<br />


**20) 处理WIN32资源文件**
{% highlight string %}
# Win32 resource file

if test -n "$NGX_RES"; then

    ngx_res=`echo "$NGX_RES:	$NGX_RC $NGX_ICONS" \
                 | sed -e "s/\//$ngx_regex_dirsep/g"`
    ngx_rcc=`echo $NGX_RCC | sed -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_res
	$ngx_rcc

END

fi
{% endhighlight %}
当前我们是在Linux平台，因此不会使用到。

<br />


**21) 处理预编译头**
{% highlight string %}
# the precompiled headers

if test -n "$NGX_PCH"; then
    echo "#include <ngx_config.h>" > $NGX_OBJS/ngx_pch.c

    ngx_pch="src/core/ngx_config.h $OS_CONFIG $NGX_OBJS/ngx_auto_config.h"
    ngx_pch=`echo "$NGX_PCH:	$ngx_pch" | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_src="\$(CC) \$(CFLAGS) $NGX_BUILD_PCH $ngx_compile_opt \$(ALL_INCS)"
    ngx_src="$ngx_src $ngx_objout$NGX_OBJS/ngx_pch.obj $NGX_OBJS/ngx_pch.c"
    ngx_src=`echo $ngx_src | sed -e "s/\//$ngx_regex_dirsep/g"`

    cat << END                                                >> $NGX_MAKEFILE

$ngx_pch
	$ngx_src

END

fi
{% endhighlight %}
针对当前我们Linux环境，```NGX_PCH```为空，因此本段代码并不会执行。


**22) 处理动态模块**
{% highlight string %}
if test -n "$NGX_PCH"; then
    ngx_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(CFLAGS) $ngx_use_pch \$(ALL_INCS)"
else
    ngx_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(CFLAGS) \$(ALL_INCS)"
    ngx_perl_cc="\$(CC) $ngx_compile_opt $ngx_pic_opt \$(NGX_PERL_CFLAGS)"
    ngx_perl_cc="$ngx_perl_cc \$(ALL_INCS)"
fi
{% endhighlight %}
这里```NGX_PCH```为空，因此执行else分支，生成ngx_cc与ngx_perl_cc两个略有不同的编译选项。

<br />

{% highlight string %}
ngx_obj_deps="\$(CORE_DEPS)"
if [ $HTTP != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(HTTP_DEPS)"
fi
if [ $MAIL != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(MAIL_DEPS)"
fi
if [ $STREAM != NO ]; then
    ngx_obj_deps="$ngx_obj_deps \$(STREAM_DEPS)"
fi
{% endhighlight string %}

如上生成所需要的依赖项。此处```HTTP```为```YES```，而```MAIL```与```STREAM```均为启用。

<br />
<br />

{% highlight string %}
for ngx_module in $DYNAMIC_MODULES
do
    eval ngx_module_srcs="\$${ngx_module}_SRCS"
    eval eval ngx_module_libs="\\\"\$${ngx_module}_LIBS\\\""

    eval ngx_module_modules="\$${ngx_module}_MODULES"
    eval ngx_module_order="\$${ngx_module}_ORDER"

    ngx_modules_c=$NGX_OBJS/${ngx_module}_modules.c

    cat << END                                    > $ngx_modules_c

#include <ngx_config.h>
#include <ngx_core.h>

END

    for mod in $ngx_module_modules
    do
        echo "extern ngx_module_t  $mod;"         >> $ngx_modules_c
    done

    echo                                          >> $ngx_modules_c
    echo 'ngx_module_t *ngx_modules[] = {'        >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    &$mod,"                         >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_names[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_order[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_order
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    ngx_modules_c=`echo $ngx_modules_c | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_modules_obj=`echo $ngx_modules_c \
        | sed -e "s/\(.*\.\)c/\1$ngx_objext/"`

    ngx_module_objs=
    for ngx_src in $ngx_module_srcs
    do
        case "$ngx_src" in
            src/*)
                ngx_obj=$ngx_src
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_src\``"
                mkdir -p $NGX_OBJS/$ngx_obj
                ngx_obj="$ngx_obj/`basename $ngx_src`"
                ;;
        esac

        ngx_module_objs="$ngx_module_objs $ngx_obj"
    done

    ngx_module_objs=`echo $ngx_module_objs \
        | sed -e "s#\([^ ]*\.\)cpp#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)cc#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)c#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)S#$NGX_OBJS\/\1$ngx_objext#g"`

    ngx_deps=`echo $ngx_module_objs $ngx_modules_obj $LINK_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_objs=`echo $ngx_module_objs $ngx_modules_obj \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_long_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_obj=$NGX_OBJS$ngx_dirsep$ngx_module$ngx_modext

    if [ "$NGX_PLATFORM" = win32 ]; then
        ngx_module_libs="$CORE_LIBS $ngx_module_libs"
    fi

    ngx_libs=
    if test -n "$NGX_LD_OPT$ngx_module_libs"; then
        ngx_libs=`echo $NGX_LD_OPT $ngx_module_libs \
            | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`
    fi

    ngx_link=${CORE_LINK:+`echo $CORE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}

    ngx_module_link=${MODULE_LINK:+`echo $MODULE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}


    cat << END                                            >> $NGX_MAKEFILE

modules:	$ngx_obj

$ngx_obj:	$ngx_deps$ngx_spacer
	\$(LINK) $ngx_long_start$ngx_binout$ngx_obj$ngx_long_cont$ngx_objs$ngx_libs$ngx_link$ngx_module_link
$ngx_long_end

$ngx_modules_obj:	\$(CORE_DEPS)$ngx_cont$ngx_modules_c
	$ngx_cc$ngx_tab$ngx_objout$ngx_modules_obj$ngx_tab$ngx_modules_c$NGX_AUX

END

    for ngx_source in $ngx_module_srcs
    do
        case "$ngx_source" in
            src/*)
                ngx_obj=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_source\``"
                ngx_obj=`echo $ngx_obj/\`basename $ngx_source\` \
                    | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
        esac

        ngx_obj=`echo $ngx_obj \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        ngx_src=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`

        if [ $ngx_source = src/http/modules/perl/ngx_http_perl_module.c ]; then

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_perl_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
        else

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

        fi
    done
done

{% endhighlight %}
如上遍历```$DYNAMIC_MODULES```:

<br />


首先在循环中求得每一个module的相关信息：
{% highlight string %}
    eval ngx_module_srcs="\$${ngx_module}_SRCS"
    eval eval ngx_module_libs="\\\"\$${ngx_module}_LIBS\\\""

    eval ngx_module_modules="\$${ngx_module}_MODULES"
    eval ngx_module_order="\$${ngx_module}_ORDER"

    ngx_modules_c=$NGX_OBJS/${ngx_module}_modules.c
{% endhighlight %}
此处获得对应```ngx_module```所相关的：
<pre>
ngx_module_srcs: 源文件

ngx_module_libs: 所依赖的库文件

ngx_module_modules: 所依赖的所有modules

ngx_module_order: 模块的加载顺序

ngx_modules_c: 需要生成的一个接口文件名
</pre>

<br />


上面我们指定了需要生成的接口文件名```$ngx_modules_c```，如下我们生成该文件：
{% highlight string %}
    cat << END                                    > $ngx_modules_c

#include <ngx_config.h>
#include <ngx_core.h>

END

    for mod in $ngx_module_modules
    do
        echo "extern ngx_module_t  $mod;"         >> $ngx_modules_c
    done

    echo                                          >> $ngx_modules_c
    echo 'ngx_module_t *ngx_modules[] = {'        >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    &$mod,"                         >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_names[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_modules
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END

    echo 'char *ngx_module_order[] = {'           >> $ngx_modules_c

    for mod in $ngx_module_order
    do
        echo "    \"$mod\","                      >> $ngx_modules_c
    done

    cat << END                                    >> $ngx_modules_c
    NULL
};

END
{% endhighlight %}
生成过程比较简单，最终生成一个类似如下的文件：
{% highlight string %}

#include <ngx_config.h>
#include <ngx_core.h>

extern ngx_module_t  $mod;  
ngx_module_t *ngx_modules[] = {
	&$mod,
	NULL
};
char *ngx_module_names[] = {
	"$mod",
    NULL
};
char *ngx_module_order[] = {
	"$mod_order"
	NULL
};
{% endhighlight %}


<br />


如下在Makefile中生成编译该模块的相关代码：
{% highlight string %}
    ngx_modules_c=`echo $ngx_modules_c | sed -e "s/\//$ngx_regex_dirsep/g"`

    ngx_modules_obj=`echo $ngx_modules_c \
        | sed -e "s/\(.*\.\)c/\1$ngx_objext/"`

    ngx_module_objs=
    for ngx_src in $ngx_module_srcs
    do
        case "$ngx_src" in
            src/*)
                ngx_obj=$ngx_src
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_src\``"
                mkdir -p $NGX_OBJS/$ngx_obj
                ngx_obj="$ngx_obj/`basename $ngx_src`"
                ;;
        esac

        ngx_module_objs="$ngx_module_objs $ngx_obj"
    done

    ngx_module_objs=`echo $ngx_module_objs \
        | sed -e "s#\([^ ]*\.\)cpp#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)cc#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)c#$NGX_OBJS\/\1$ngx_objext#g" \
              -e "s#\([^ ]*\.\)S#$NGX_OBJS\/\1$ngx_objext#g"`

    ngx_deps=`echo $ngx_module_objs $ngx_modules_obj $LINK_DEPS \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_objs=`echo $ngx_module_objs $ngx_modules_obj \
        | sed -e "s/  *\([^ ][^ ]*\)/$ngx_long_regex_cont\1/g" \
              -e "s/\//$ngx_regex_dirsep/g"`

    ngx_obj=$NGX_OBJS$ngx_dirsep$ngx_module$ngx_modext

    if [ "$NGX_PLATFORM" = win32 ]; then
        ngx_module_libs="$CORE_LIBS $ngx_module_libs"
    fi

    ngx_libs=
    if test -n "$NGX_LD_OPT$ngx_module_libs"; then
        ngx_libs=`echo $NGX_LD_OPT $ngx_module_libs \
            | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`
    fi

    ngx_link=${CORE_LINK:+`echo $CORE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}

    ngx_module_link=${MODULE_LINK:+`echo $MODULE_LINK \
        | sed -e "s/\//$ngx_regex_dirsep/g" -e "s/^/$ngx_long_regex_cont/"`}


    cat << END                                            >> $NGX_MAKEFILE

modules:	$ngx_obj

$ngx_obj:	$ngx_deps$ngx_spacer
	\$(LINK) $ngx_long_start$ngx_binout$ngx_obj$ngx_long_cont$ngx_objs$ngx_libs$ngx_link$ngx_module_link
$ngx_long_end

$ngx_modules_obj:	\$(CORE_DEPS)$ngx_cont$ngx_modules_c
	$ngx_cc$ngx_tab$ngx_objout$ngx_modules_obj$ngx_tab$ngx_modules_c$NGX_AUX

END
{% endhighlight %}

上面首先遍历```ngx_module_srcs```文件夹下的所有文件，找出对应的源文件，从而求得所依赖的```.o```名存放在ngx_module_objs变量中。


接着求的依赖文件：ngx_deps


将上面求的的```ngx_module_objs```合并上```ngx_modules_obj```(注意这两个变量不同），即所有的```.o```文件保存到变量ngx_objs中。

然后再找出所有的依赖库文件、链接文件：```ngx_libs```,```ngx_link```,```ngx_module_link```。

最后生成Makefile中相应的target。


<br />


如下是生成编译成```ngx_module_objs```的Makefile脚本：
{% highlight string %}
    for ngx_source in $ngx_module_srcs
    do
        case "$ngx_source" in
            src/*)
                ngx_obj=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
            *)
                ngx_obj="addon/`basename \`dirname $ngx_source\``"
                ngx_obj=`echo $ngx_obj/\`basename $ngx_source\` \
                    | sed -e "s/\//$ngx_regex_dirsep/g"`
                ;;
        esac

        ngx_obj=`echo $ngx_obj \
            | sed -e "s#^\(.*\.\)cpp\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)cc\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)c\\$#$ngx_objs_dir\1$ngx_objext#g" \
                  -e "s#^\(.*\.\)S\\$#$ngx_objs_dir\1$ngx_objext#g"`

        ngx_src=`echo $ngx_source | sed -e "s/\//$ngx_regex_dirsep/g"`

        if [ $ngx_source = src/http/modules/perl/ngx_http_perl_module.c ]; then

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_perl_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END
        else

            cat << END                                        >> $NGX_MAKEFILE

$ngx_obj:	$ngx_obj_deps$ngx_cont$ngx_src
	$ngx_cc$ngx_tab$ngx_objout$ngx_obj$ngx_tab$ngx_src$NGX_AUX

END

        fi
    done
{% endhighlight %}
脚本比较简单，主要就是遍历```ngx_module_srcs```文件夹，找出其中源文件，然后生成对应的编译代码。

<br />
<br />


至此，我们完成了整个auto/make脚本的分析。这是我们最后要生成的Makefile的主要部分，但是由于还剩下其他一些部分，等所有部分讲解完成，我们会贴出该Makefile文件，以作参考。





<br />
<br />
<br />

