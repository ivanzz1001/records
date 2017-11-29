---
layout: post
title: Linux ubuntu操作系统常用命令
tags:
- LinuxOps
categories: linuxOps
description: Linux ubuntu操作系统常用命令
---


本文简要记录一下ubuntu操作系统上常用的一些命令及操作。


<!-- more -->

## 1. 常用命令

**1) ubuntu启用root用户**

<pre>
# sudo passwd root
</pre>

**2) 更换apt源**
<pre>
# cd /etc/apt && ls
# mv sources.list sources.list.ori
# wget http://mirrors.163.com/.help/sources.list.trusty
# cp sources.list.trusty /etc/apt/sources.list

# cat sources.list
deb http://mirrors.163.com/ubuntu/ trusty main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ trusty-security main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ trusty-updates main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb http://mirrors.163.com/ubuntu/ trusty-backports main restricted universe multiverse
deb-src http://mirrors.163.com/ubuntu/ trusty main restricted universe multiverse
deb-src http://mirrors.163.com/ubuntu/ trusty-security main restricted universe multiverse
deb-src http://mirrors.163.com/ubuntu/ trusty-updates main restricted universe multiverse
deb-src http://mirrors.163.com/ubuntu/ trusty-proposed main restricted universe multiverse
deb-src http://mirrors.163.com/ubuntu/ trusty-backports main restricted universe multiverse 

# sudo apt-get update
</pre>


**3) 安装lrzsz上传下载程序**
<pre>
# sudo apt-get install lrzsz

# rz                //上传

# sz                //下载
</pre>


**4) 启用ssh服务**
<pre>
# sudo apt-get update
# sudo apt-get install openssh-server

# sudo ps -ef | grep ssh
root      4495     1  0 12:09 ?        00:00:00 /usr/sbin/sshd -D

# sudo vi /etc/ssh/sshd_config
//此处修改sshd_config文件,将"PermitRootLogin without-password"注释掉，并且添加"PermitRootLogin yes"

# service --status-all | grep ssh
 [ ? ]  apport
 [ ? ]  console-setup
 [ ? ]  dns-clean
 [ ? ]  irqbalance
 [ ? ]  killprocs
 [ ? ]  kmod
 [ ? ]  lightdm
 [ ? ]  mysql
 [ ? ]  networking
 [ ? ]  ondemand
 [ ? ]  pppd-dns
 [ ? ]  rc.local
 [ ? ]  sendsigs
 [ ? ]  speech-dispatcher
 [ + ]  ssh
 [ ? ]  thermald
 [ ? ]  umountfs
 [ ? ]  umountnfs.sh
 [ ? ]  umountroot

# sudo service ssh restart
ssh stop/waiting
ssh start/running, process 12784
</pre>


**5) 解压文件**
<pre>
//将dist.tar.gz解压到父命令的server文件夹
# tar -xzvf dist.tar.gz -C ../server
</pre>

**6) 查看所依赖的库**
{% highlight string %}
# ldd /bin/ls
        linux-vdso.so.1 =>  (0x00007fff587ba000)
        libselinux.so.1 => /lib/x86_64-linux-gnu/libselinux.so.1 (0x00007f417fc6b000)
        libacl.so.1 => /lib/x86_64-linux-gnu/libacl.so.1 (0x00007f417fa63000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f417f69e000)
        libpcre.so.3 => /lib/x86_64-linux-gnu/libpcre.so.3 (0x00007f417f460000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f417f25c000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f417fe8e000)
        libattr.so.1 => /lib/x86_64-linux-gnu/libattr.so.1 (0x00007f417f057000)
{% endhighlight %}

**7) 求文件md5值**
<pre>
# md5sum sources.list.trusty 
62c83f05b4f10e058018da436ac7eb2f  sources.list.trusty
</pre> 

其实Windows上也同样有类似与```md5sum```的命令行工具certutil，使用方法如下：
<pre>
certutil -hashfile filename MD5  
certutil -hashfile filename SHA1  
certutil -hashfile filename SHA256
</pre>

**8) 杀死所有名称带有vap字段的进程**
<pre>
# ps -ef | grep vap | grep -v grep | awk '{print $2}' | xargs kill -9
</pre>

**9) pgrep根据程序名称来查询进程**

pgrep是通过程序的名字来查询进程的工具，一般是用来判断程序是否正常运行。在服务器的配置和管理中，这个工具常被应用，简单明了。

用法为：
{% highlight string %}
Usage:
 pgrep [options] <pattern>
{% endhighlight %}
常用的选项有：

* -l, --list-name           list PID and process name

* -f, --full                use full process name to match

* -o, --oldest              select least recently started

* -n, --newest              select most recently started

举例：
<pre>
# pgrep -l vap
12195 vap2.face.captu
12196 vap2.face.serv
</pre>


**10) 安装mysql-server**
<pre>
# sudo apt-get install mysql-server
# mysql -uroot -p1234
</pre>

**11) 软链接文件**
{% highlight string %}
//创建测试文件
# mkdir src-dir dst-dir && cd src-dir && cat << EOF > test.txt
> aaa
> bbb
> ccc
> ddd
> EOF

//创建软链接文件
# ln -sf test.txt softlink.txt
# ls -al 
total 8
drwxr-xr-x 2 root root   40 11月 13 16:56 .
drwx------ 6 root root 4096 11月 13 16:50 ..
lrwxrwxrwx 1 root root    8 11月 13 16:56 softlink.txt -> test.txt
-rw-r--r-- 1 root root   16 11月 13 16:55 test.txt

//拷贝带软链接的文件夹
# cd .. && cp -ar src-dir dst-dir
{% endhighlight %}

**12） 统计网络流量**
<pre>

root@ai-test:~# sar -n DEV 10                  //每10s钟统计一次
Linux 3.13.0-32-generic (ai-test)       11/17/2017      _x86_64_        (16 CPU)

10:56:22 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s   %ifutil
10:56:32 AM      eth0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
10:56:32 AM        lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00

10:56:32 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s   %ifutil
10:56:42 AM      eth0      1.20      1.20      0.07      0.15      0.00      0.00      0.00      0.00
10:56:42 AM        lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
</pre>

**13） at命令**

定时执行 1 次任务：
<pre>
[root@localhost test-src]# at 4pm + 3 days   // 3天后的下午4点执行一个任务
at> echo "hello,world"
at>  Ctrl+D


[root@localhost test-src]# at 10am Jul 31    // 7月31号上午10点执行一个任务
at> echo "hello,world"
at>  Ctrl+D

[root@localhost test-src]#  at 1am tomorrow    // 明天凌晨1点执行一个任务
at> echo "hello,world"
at>  Ctrl+D

[root@localhost test-src]# at -l               // 列出任务


[root@localhost test-src]# at -d [task-id]     // 删除任务
</pre>

**14) xargs与管道的区别**

管道是实现： 将前面的标准输出作为后面的标准输入

xargs是实现：将标准输入作为命令的参数。

举个例子：
<pre>
[root@localhost test-src]# echo "--help" | cat
--help
[root@localhost test-src]# echo "--help" | xargs cat
Usage: cat [OPTION]... [FILE]...
Concatenate FILE(s), or standard input, to standard output.

  -A, --show-all           equivalent to -vET
  -b, --number-nonblank    number nonempty output lines, overrides -n
  -e                       equivalent to -vE
  -E, --show-ends          display $ at end of each line
  -n, --number             number all output lines
  -s, --squeeze-blank      suppress repeated empty output lines
  -t                       equivalent to -vT
  -T, --show-tabs          display TAB characters as ^I
  -u                       (ignored)
  -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB
      --help     display this help and exit
      --version  output version information and exit

With no FILE, or when FILE is -, read standard input.

Examples:
  cat f - g  Output f's contents, then standard input, then g's contents.
  cat        Copy standard input to standard output.

GNU coreutils online help: <http://www.gnu.org/software/coreutils/>
For complete documentation, run: info coreutils 'cat invocation'
</pre>
如果你直接在命令行输入cat而不输入其余的任何东西，这时候的cat会等待标准输入，因此这时候可以通过键盘并按回车来让cat读取输入，cat会原样返回； 而如果你输入--help，那么cat程序会在标准输出上打印自己的帮助文档。**也就是说**，管道符 | 所传递给程序的不是你简单地在程序名后面输入的参数，它会被程序内部的读取功能如scanf和gets等接收，而xargs则是将内容作为普通的参数传递给程序。如上面的例子相当于你手写了```cat --help```。


**15) gawk去除一个文件中所有重复的数字**
{% highlight string %}
[root@localhost test-src]# cat mydata.txt
1000
1001
1002
1003
1004
1002
1005
1004
1007
[root@localhost test-src]# gawk '!a[$0]++' mydata.txt 
1000
1001
1002
1003
1004
1005
1007
{% endhighlight %}

**16) sed替换文件内容**
<pre>
[root@localhost test-src]# cat mydata.txt 
1000
1001
1002
1003
1004
1002
1005
1004
1007
[root@localhost test-src]# sed -i 's/1002/100000000/g' mydata.txt
[root@localhost test-src]# cat mydata.txt 
1000
1001
100000000
1003
1004
100000000
1005
1004
1007
</pre>


<br />
<br />







<br />
<br />
<br />


