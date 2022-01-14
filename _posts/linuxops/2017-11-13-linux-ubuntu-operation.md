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

ldd命令通常用来查看程序运行所需的共享库，常用来解决程序因缺少某个库文件而不能运行的一些问题。
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
上面输出结果总共有3列，下面介绍一下各列的含义：

* 第一列： 程序需要依赖什么库

* 第二列： 程序所需要的库的存放位置

* 第三列： 库加载的开始地址

通过上面的信息，我们可以得到如下：

* 通过对比第一列和第二列，我们可以分析程序需要依赖的库和系统实际提供的，是否相匹配

* 通过观察第三列，我们可以知道在当前库中的符号在对应进程地址空间中的开始位置。

如果依赖的某个库找不到，通过这个命令可以迅速定位问题所在。


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


全字匹配替换：
[root@localhost test-src]# sed -i 's/\<1000\>/99999/g' mydata.txt 
[root@localhost test-src]# cat mydata.txt 
99999
1001
1111100000
1003
1004
1111100000
1005
1004
1007
</pre>

**17) 命令行for语句**
{% highlight string %}
[root@localhost test-src]# for i in {1..10}; do echo "index:$i"; done
index:1
index:2
index:3
index:4
index:5
index:6
index:7
index:8
index:9
index:10
{% endhighlight %}

**18) Linux下查看显卡类型**
{% highlight string %}
# lspci | grep -i vga
02:00.0 VGA compatible controller: NVIDIA Corporation Device 1b06 (rev a1)
03:00.0 VGA compatible controller: NVIDIA Corporation Device 1b06 (rev a1)
# ls -al NVIDIA-Linux-x86_64-384.98.run 


# lspci -v -s 02:00.0            //如下查看显卡详细信息
02:00.0 VGA compatible controller: NVIDIA Corporation Device 1b06 (rev a1) (prog-if 00 [VGA controller])
        Subsystem: Gigabyte Technology Co., Ltd Device 376b
        Physical Slot: 2
        Flags: bus master, fast devsel, latency 0, IRQ 55
        Memory at f4000000 (32-bit, non-prefetchable) [size=16M]
        Memory at c0000000 (64-bit, prefetchable) [size=256M]
        Memory at d0000000 (64-bit, prefetchable) [size=32M]
        I/O ports at 2000 [size=128]
        [virtual] Expansion ROM at 000c0000 [disabled] [size=128K]
        Capabilities: [60] Power Management version 3
        Capabilities: [68] MSI: Enable+ Count=1/1 Maskable- 64bit+
        Capabilities: [78] Express Legacy Endpoint, MSI 00
        Capabilities: [100] Virtual Channel
        Capabilities: [250] Latency Tolerance Reporting
        Capabilities: [128] Power Budgeting <?>
        Capabilities: [420] Advanced Error Reporting
        Capabilities: [600] Vendor Specific Information: ID=0001 Rev=1 Len=024 <?>
        Capabilities: [900] #19
        Kernel driver in use: nvidia
        Kernel modules: nvidiafb, nouveau, nvidia_drm, nvidia
{% endhighlight %}
参看：[linux下显卡信息的查看](http://blog.csdn.net/wind19/article/details/17095541)

**19) 去除文件中的"\r"换行**
{% highlight string %}
# cat test_id.txt | while read line; do awk 'BEGIN{FS="\r"} {print $1}' >> def.txt; done
{% endhighlight %}

**20) 遍历指定的文件是否存在**
{% highlight string %}
// abc.txt存放指定的文件名列表， photos文件夹下存放所有照片文件 

# cat abc.txt | while read line; do filename=photos/${line}_*.jpg; result=$(find $filename);if [[ -n $result ]]; then echo $line >> test.txt; fi done




//JR.txt是一个类似于如下的问题件：
{"usertag":{"tag":"支配欲强、有责任感","name":"zhangsan 张三","deptname":"集团职能|Headquarter of Group"},"userno":"1001"}
{"usertag":{"tag":"精力旺盛、不服输","name":"lisi 李四","deptname":"集团职能|Headquarter of Group"},"userno":"1002"}
{"usertag":{"tag":"性情天真、热情奔放","name":"wangwu 王五","deptname":"集团职能|Headquarter of Group"},"userno":"1003"}

//如下找出JR.txt文件中有照片的人员信息
# cat JR.txt | while read line; do str1=${line##*\"userno\"}; str2=${str1#*\"}; usrno=${str2%%\"*};filename=photos/${usrno}_*.jpg; result=$(find $filename);if [[ -n $result ]]; then echo $line >> have_photo_person.txt; fi done

{% endhighlight %}

**21) 文件批量重命名**

假设我们在当前目录下有如下文件：
<pre>
# ls -al *.bak
-rw-r--r-- 1 root root 20227 12月 29 2014 luaconf.h.bak
-rw-r--r-- 1 root root 14734 12月 27 2014 lua.h.bak
-rw-r--r-- 1 root root   191 12月 23 2004 lua.hpp.bak
-rw-r--r-- 1 root root  1173 2月   7 2014 lualib.h.bak
</pre>
现在我们需要将其中的所有```.bak```文件重命名回原来的名字，即将```.bak```后缀去掉：
{% highlight string %}
# ls -al *.bak | while read line; do filename=`echo $line | awk '{print $9}'`; newfilename=`echo $filename | awk -F .bak '{print $1}'`; echo "prepare to rename file: $filename == > $newfilename"; mv ./$filename ./$newfilename;done
prepare to rename file: luaconf.h.bak == > luaconf.h
prepare to rename file: lua.h.bak == > lua.h
prepare to rename file: lua.hpp.bak == > lua.hpp
prepare to rename file: lualib.h.bak == > lualib.h

# ls -al lua
luaconf.h  lua.h      lua.hpp    lualib.h  
{% endhighlight %}



**22) ubuntu16.04 修改配置文件 禁止系统自动更新**

可以通过修改配置文件或者通过界面操作来禁止系统自动更新。

**方式1： 修改配置文件**
<pre>
// 修改配置文件/etc/apt/apt.conf.d/10periodic， "0"是关闭，"1"是开启。这里将所有都关闭

# cat /etc/apt/apt.conf.d/10periodic
APT::Periodic::Update-Package-Lists "0";
APT::Periodic::Download-Upgradeable-Packages "0";
APT::Periodic::AutocleanInterval "0";
</pre>

**方式2： 界面操作**
{% highlight string %}
菜单栏点 系统 --> 首选项 --> 启动应用程序 --> 更新提示 前面的钩打掉,从不更新 关闭即可
{% endhighlight %}

**23) 批量重命名文件**
{% highlight string %}
// 将当前文件夹下的所有.jpg文件命名为.png文件

# for files in `ls *.txt`; do filename=`echo $files | sed 's/.jpg/.png/'`; mv $files $filename; done
{% endhighlight %}

**24) tee命令**

tee命令用于从标准输入读取数据，然后写到标准输出及文件中去。基本语法如下：
{% highlight string %}
SYNOPSIS
       tee [OPTION]... [FILE]...

DESCRIPTION
       Copy standard input to each FILE, and also to standard output.

       -a, --append
              append to the given FILEs, do not overwrite
{% endhighlight %}

例如：
<pre>
# tee test.txt <<- 'EOF'
> hello,world 0
> hello,world 1
> EOF
hello,world 0
hello,world 1
</pre>

**25） 计算一个文本文件中所有数字之和**
{% highlight string %}
# cat qq.txt
100,200,300,400
200,300

# cat ./qq.txt | awk -F ',' 'BEGIN{s=0} {for(i=1;i<=NF;i++) {s+=$i}} END{print s}' 
1500
# awk -F ',' 'BEGIN{s=0} {for(i=1;i<=NF;i++) {s+=$i}} END{print s}' qq.txt
1500
{% endhighlight %}

**26) 统计一个文本中数字出现的次数，并按出现次数从大到小排序**
{% highlight string %}
# cat test.txt
100
200
300
400
200
300

# sort ./test.txt | uniq -c | sort -rn     //当重复的行并不相邻时，uniq 命令是不起作用的，因此需要先进行排序
      2 300
      2 200
      1 400
      1 100
{% endhighlight %}


**27） size 查看程序内存映像大小**

```size```可用于查看程序被映射到内存中的映像所占用的大小信息。程序映射到内存中，从低地址到高地址依次为下列段：

* 代码段： 只读，可共享。代码段(code segment/text segment)通常是指用来存放程序执行代码的一块内存区域。这部分区域的大小在程序运行前就已经确定，并且内存区域通常数据只读，某些架构也允许代码段为可写，即允许修改程序。在代码段中，也有可能包含一些只读的常数变量，例如字符串常量等。

* 数据段： 存储已被初始化了的静态数据。数据段(data segment)通常是指用来存放程序中已初始化的全局变量的一块内存区域。数据段数据静态内存分配。

* BSS段： 未初始化的数据段。BSS段（bss segment）通常是指用来存放程序中未初始化的全局变量的一块内存区域。BSS是英文Block Started by Symbol的简称。BSS段属于静态内存分配

* 堆(heap): 堆用于存放进程运行过程中动态分配的内存段，它的大小并不固定，可动态扩张或缩减。当进程调用malloc等函数分配内存时，新分配的内存就被动态添加到堆上（堆被扩张）；当利用free等函数释放内存时，被释放的内存就从堆中被剔除（堆被缩减）

* 栈(stack): 栈又被称为```堆栈```，是用来存放程序临时创建的局部变量，也就是我们函数括弧```{}```中定义的变量（注： 但不包括static声明的变量，static意味着在数据段中存放变量）。除此以外，在函数被调用时，其参数也会被压入发起调用的进程栈中，并且等到调用结束后，函数的返回值也```可能```会被存放回栈中。从这个意义上讲，我们可以把栈看成一个寄存、交换临时数据的内存区。

另外，在高地址还存储了命令行参数及环境变量。

因为内存程序映像中的各段可能位于不同的地址空间中，它们不一定位于连续的内存块中。操作系统将程序映像映射到地址空间时，通常将内存程序映像划分为大小相同的块(也就是page，页）。只有该页被引用时，它才被加载到内存中。不过对于程序员来说，可以视内存程序映像在逻辑上是连续的。

```size```命令的用法如下：
<pre>
#size main
text    data     bss     dec     hex filename
1259     540      16    1815     717 main
</pre>

**28) file文件类型查询**

可以用```file```命令查看文件的类型。比如我们在64位机器上发现了一个32位的库，链接不上，这就有问题了。

<pre>
# file a.out
a.out: ELF 64-bit LSB executable, AMD x86-64, version 1 (SYSV), for GNU/Linux 2.6.9, dynamically linked (uses shared libs), for GNU/Linux 2.6.9, not stripped
</pre>

**29) strings 查询数据中的文本信息**

一个文件中包含二进制数据和文本数据，如果只需要查看其文本信息，使用这个命令就很方便。可以过滤掉非字符数据，将文本信息输出：
{% highlight string %}
# strings <objfile>
{% endhighlight %}


**30) shell awk 统计重复个数**

现有如下```file.log```文件：
{% highlight string %}
# cat file.log
http://www.sohu.com/aaa
http://www.sina.com/111
http://www.sohu.com/bbb
http://www.sina.com/222
http://www.sohu.com/ccc
http://www.163.com/zzz
http://www.sohu.com/ddd
{% endhighlight %}
现在要统计每个域名出现的次数，并按倒序排列：
{% highlight string %}
# awk -F / '{a[$3]++} END{for(i in a) {print i, a[i] | "sort -rnk 2"} }' ./file.log 
www.sohu.com 4
www.sina.com 2
www.163.com 1
{% endhighlight %}

**31) shell去除空行**

有时我们在处理和查看文件时，经常会有很多空行，为了美观或是有需要时，就有必要把这些空行去除掉。例如我们有如下文件````file.log```：
{% highlight string %}
#  tee << 'EOF' > file.log
> http://www.sohu.com/aaa 
> 
> http://www.sina.com/111 
> 
> http://www.sohu.com/bbb 
> 
> http://www.sina.com/222 
> 
> http://www.sohu.com/ccc 
> 
> http://www.163.com/zzz 
> 
> http://www.sohu.com/ddd 
> 
> EOF
# cat file.log
http://www.sohu.com/aaa 

http://www.sina.com/111 

http://www.sohu.com/bbb 

http://www.sina.com/222 

http://www.sohu.com/ccc 

http://www.163.com/zzz 

http://www.sohu.com/ddd 

{% endhighlight %}
我们可以用如下4种方法来去除里面的空行：

* tr命令
<pre>
# cat file.log | tr -s '\r\n'
http://www.sohu.com/aaa 
http://www.sina.com/111 
http://www.sohu.com/bbb 
http://www.sina.com/222 
http://www.sohu.com/ccc 
http://www.163.com/zzz 
http://www.sohu.com/ddd 
</pre>

* sed命令
<pre>
# cat file.log | sed '/^$/d'
http://www.sohu.com/aaa 
http://www.sina.com/111 
http://www.sohu.com/bbb 
http://www.sina.com/222 
http://www.sohu.com/ccc 
http://www.163.com/zzz 
http://www.sohu.com/ddd
</pre>

* awk命令
{% highlight string %}
# cat file.log | awk '{if ($0 != "") print}'
http://www.sohu.com/aaa 
http://www.sina.com/111 
http://www.sohu.com/bbb 
http://www.sina.com/222 
http://www.sohu.com/ccc 
http://www.163.com/zzz 
http://www.sohu.com/ddd 

# cat file.log | awk '{if (length != 0) print $0}'
http://www.sohu.com/aaa 
http://www.sina.com/111 
http://www.sohu.com/bbb 
http://www.sina.com/222 
http://www.sohu.com/ccc 
http://www.163.com/zzz 
http://www.sohu.com/ddd 
{% endhighlight %}

* grep命令
{% highlight string %}
# grep -v "^$" ./file.log 
http://www.sohu.com/aaa 
http://www.sina.com/111 
http://www.sohu.com/bbb 
http://www.sina.com/222 
http://www.sohu.com/ccc 
http://www.163.com/zzz 
http://www.sohu.com/ddd
{% endhighlight %}

**32) shell去除空格**

假设我们有如下文件:
{% highlight string %}
# tee << 'EOF' > mytest.txt
>    abcd   efg  
> EOF
{% endhighlight %}

* 去除行首空格
{% highlight string %}
# sed 's/^[ \t]*//g' ./mytest.txt 
abcd   efg
{% endhighlight %}
说明：

第一个```/```的左边是s表示替换，即将空格替换为空。

第一个```/```的右边是表示后面的以xx开头。

中括号表示“或”，空格或tab中的任意一种。这是正则表达式的规范。中括号右边是```*```，表示一个或多个。

第二个和第三个```/```中间没有东西，表示空

整体的意思是：用空字符去替换一个或多个用空格或tab开头的本体字符串


* 去除行尾空格
{% highlight string %}
# sed 's/[ \t]*$//g' ./mytest.txt 
   abcd   efg
{% endhighlight %}
上面```$```符号表示以xx结尾的字符串为对象

* 去除所有空格
{% highlight string %}
# sed 's/[ \t]*//g' ./mytest.txt 
abcdefg
{% endhighlight %}


**33) 以后台方式启动程序**
{% highlight string %}
# nohup ./test_program > /dev/null &
{% endhighlight %}

**34) 修改root密码不过期**
<pre>
# sudo chage -M 9999 root
# sudo chage -I root
</pre>

**35) 创建用户，并赋予用户sudo操作权限**
{% highlight string %}
# useradd -d /home/ivan1001 -m ivan1001
# passwd ivan1001

# echo "ivan1001 ALL = (root) NOPASSWD:ALL" | sudo tee /etc/sudoers.d/ivan1001
# chmod 0440 /etc/sudoers.d/ivan1001
{% endhighlight %}



<br />
<br />
<br />


