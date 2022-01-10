---
layout: post
title: ceph源代码编译(2)
tags:
- ceph
categories: ceph
description: ceph源代码编译
---


当前生产环境中我们所使用的ceph版本为jewel版本：
<pre>
# ceph --version
ceph version 10.2.10 (5dc1e4c05cb68dbf62ae6fce3f0700e4654fdbbe)
</pre>
因此，这里我们也以该版本为例来介绍ceph源代码的编译。当前我们的操作系统环境如下(ceph编译时需要耗费极大量的内存，建议内存至少4G以上)：
<pre>
# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# free -h
              total        used        free      shared  buff/cache   available
Mem:           3.7G        1.2G        520M         65M        2.0G        2.0G
Swap:          4.9G          0B        4.9G
</pre>
所使用的编译器版本为：
<pre>
# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-16)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>

Ceph的源码可以去Github之上clone下来，或者去Ceph官网下载。这里重点提一下Ceph的版本问题，Ceph在Hammer版本之后，采取了新的版本命名规则：

* x.0.z - 开发版

* x.1.z - 候选版

* x.2.z - 稳定、修正版


<!-- more -->

## 1. cmake编译jewel ceph

### 1.1 下载ceph源码

这里我们下载ceph-10.2.10版本。具体下载方法，略。

### 1.2 编译ceph
下载完ceph源码，我们先参看一下README.md的说明(或查看官网ceph编译说明)，以了解大体编译步骤（请保持环境干净，否则可能引起不必要的麻烦。笔者编译时就因为事先安装过rocksdb而出现问题)。

1) 安装依赖项

执行如下命令检测并安装相应的依赖项：
<pre>
# ./install-deps.sh
</pre>

2) 检查cmake版本

当前我们编译ceph10.2.10需要cmake的版本 >= 2.8.11:
<pre>
# cmake --version
cmake version 2.8.12.2
</pre>

3） 编译ceph

在编译时我们最好将src目录设置到PATH中，以防在编译过程中对ceph-authtool等工具找不到：
<pre>
# pwd
/root/ceph-inst/ceph
# export PATH=$PATH:/root/ceph-inst/ceph/src
</pre>

然后执行如下命令创建编译工作目录```build```:
<pre>
# pwd
/root/ceph-inst/ceph
# mkdir build
# cd build
</pre>

接着执行如下命令查看ceph下支持哪些编译选项：
<pre>
# cmake .. -LH
</pre>

之后再执行如下的命令进行编译(jewel目前用cmake似乎编译不了）：
<pre>
# cmake -DCMAKE_C_FLAGS="-O0 -g3 -gdwarf-4" -DCMAKE_CXX_FLAGS="-O0 -g3 -gdwarf-4"  -DWITH_TESTS=ON -DWITH_FUSE=OFF -DWITH_DPDK=OFF -DWITH_RDMA=OFF -DCMAKE_INSTALL_PREFIX=/opt/ceph ..
</pre>

### 1.3 补充
可能需要修改```install-deps.sh```:
{% highlight string %}
function activate_virtualenv() {
    local top_srcdir=$1
    local interpreter=$2
    local env_dir=$top_srcdir/install-deps-$interpreter

    if ! test -d $env_dir ; then
        # Make a temporary virtualenv to get a fresh version of virtualenv
        # because CentOS 7 has a buggy old version (v1.10.1)
        # https://github.com/pypa/virtualenv/issues/463
        virtualenv ${env_dir}_tmp
        ${env_dir}_tmp/bin/pip install --upgrade pip==20.2.4
        ${env_dir}_tmp/bin/pip install --upgrade setuptools
        ${env_dir}_tmp/bin/pip install --upgrade virtualenv
        ${env_dir}_tmp/bin/virtualenv --python $interpreter $env_dir
        rm -rf ${env_dir}_tmp

        . $env_dir/bin/activate
        if ! populate_wheelhouse install ; then
            rm -rf $env_dir
            return 1
        fi
    fi
    . $env_dir/bin/activate
}

{% endhighlight %}
在activate_virtualenv()中增加了如下两行：
{% highlight string %}
${env_dir}_tmp/bin/pip install --upgrade pip==20.2.4
${env_dir}_tmp/bin/pip install --upgrade setuptools
{% endhighlight %}

>注：python包索引网址https://mirrors.bfsu.edu.cn/pypi/web/simple/

<br />
<br />

**参考**:

1. [Developer Guide](https://github.com/ceph/ceph/blob/jewel-next/doc/dev/quick_guide.rst)

2. [ceph编译+ gdb调试](https://m.sohu.com/a/198350012_100011803)

3. [Ceph编译：L版本及其之后的版本](https://www.cnblogs.com/powerrailgun/p/12133107.html)

4. [ceph官方调试环境搭建](https://github.com/ceph/ceph/blob/jewel/doc/dev/quick_guide.rst)

<br />
<br />
<br />

