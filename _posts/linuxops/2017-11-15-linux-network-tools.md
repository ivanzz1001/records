---
layout: post
title: Linux系统监测工具
tags:
- LinuxOps
categories: linuxOps
description: Linux系统监测工具
---


Linux提供了很多有用的工具，以方便开发人员调试和测评服务器程序。下面我们主要会介绍nc、strace、lsof、netstat、vmstat、ifstat和mpstat这些工具，它们通常都支持很多中选项，不过我们的讨论仅限于最常用、最实用的那些。

<!-- more -->

## 1. lsof命令
lsof(list open file)是一个列出当前系统打开的文件描述符的工具。通过它，我们可以了解感兴趣的进程打开了哪些文件描述符，或者我们感兴趣的文件描述符被哪些进程打开了。

lsof命令常用的选项包括：

* -i: 显示socket文件描述符。该选项的使用方法是
<pre>
# lsof -i [4|6] [protocol] [@hostname|ipaddr][:service|port]
</pre>
其中，4表示IPv4协议，6表示IPv6协议； protocol指定传输层协议，可以是TCP或者UDP； hostname指定主机名；ipaddr指定主机的IP地址；service指定服务名； port指定端口号。比如，要显示所有连接到主机**192.168.1.108**的ssh服务的socket文件描述符，可以使用命令（注： 可能需要切换成root权限才能查询出来）：
<pre>
# lsof -i@192.168.10.129:22
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
sshd    5274 root    3u  IPv4  41450      0t0  TCP 192.168.10.129:ssh->192.168.10.1:1131 (ESTABLISHED)
</pre>
如果```-i```选项后不指定任何参数，则lsof命令将会显示所有socket文件描述符

* -u: 显示指定用户启动的所有进程打开的所有文件描述符

* -c: 显示指定的命令打开的所有文件描述符。比如，要查看websrv程序打开了哪些文件描述符，可以使用如下命令
<pre>
# lsof -c websrv
</pre>

* -p: 显示指定进程打开的所有文件描述符

* -t：仅显示打开了目标文件描述符的进程的PID

我们还可以直接将文件名作为lsof命令的参数，以查看哪些进程打开了该文件。例如：
{% highlight string %}
# vi workspace/test.c
# lsof workspace/.test.c.swp 
lsof: WARNING: can't stat() fuse.gvfsd-fuse file system /run/user/1000/gvfs
      Output information may be incomplete.
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF    NODE NAME
vi      6284 root    4u   REG    8,1    12288 2097196 workspace/.test.c.swp

# lsof -e /run/user/1000/gvfs workspace/.test.c.swp 
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF    NODE NAME
vi      6433 root    4u   REG    8,1    12288 2097196 workspace/.test.c.swp

# ll /proc/6433/fd
total 0
dr-x------ 2 root root  0 Jan 12 01:11 ./
dr-xr-xr-x 9 root root  0 Jan 12 01:11 ../
lrwx------ 1 root root 64 Jan 12 01:11 0 -> /dev/pts/6
lrwx------ 1 root root 64 Jan 12 01:11 1 -> /dev/pts/6
lrwx------ 1 root root 64 Jan 12 01:11 2 -> /dev/pts/6
lrwx------ 1 root root 64 Jan 12 01:11 4 -> /root/workspace/.test.c.swp
{% endhighlight %}

上面注意两点：
<pre>
1) 通过vi命令打开一个文件时，并不是直接打开，而是会生成一个swp文件；

2) lsof 默认检查所有挂载的文件系统包括FUSE（这种文件系统使用用户空间实现，但却有指定的访问权限），
  因此打印出了上面的警告信息，我们通过添加'-e /run/usr/1000/gvfs'来剔除检查该类型的文件系统.
</pre>

通过上面所示，lsof命令的输出内容相当丰富，其中每行内容都包含如下字段：

* COMMAND: 执行程序所使用的终端命令（默认仅显示前9个字符）

* PID: 文件描述符所属进程的PID

* USER: 拥有该文件描述符的用户的用户名

* FD: 对于文件描述符的描述。其中，```cwd```表示进程的工作目录；```rtd```表示用户的根目录；```txt```表示进程运行的程序代码；```mem```表示直接映射到内存的文件。有的FD是以```'数字+访问权限'```表示的，其中数字是文件描述符的具体数值，访问权限包括```r```(可读)、```w```(可写）和```u```(读写）。

* TYPE: 文件描述符的类型。其中DIR是目录，REG是普通文件，CHR是字符设备文件，IPv4是IPv4类型的socket文件描述符，0000是未知类型。更多文件描述符的类型请参考lsof命令的man手册，这里不再赘述。

* DEVICE: 文件所属设备。对于字符设备和块设备，其表示方法是```主设备号，次设备号```。我们上面的示例中```.test.c.swp```存放在设备```8,1```中。其中```8```表示这是一个SCSI硬盘，```1```表示这是该硬盘上的第一个分区，即sda1。更多其他类型的设备编码，请参看[Linux官方设备](http://www.kernel.org/pub/linux/docs/lanana/device-list/devices-2.6.txt)。对于FIFO类型的文件，比如管道和socket，该字段将显示一个内核引用目标文件的地址，或者是其```i节点号```。


* SIZE/OFF: 文件大小或者偏移值。如果该字段显示为```0t*```或者```0x*```,就表示这是一个偏移值，否则就表示这是一个文件大小。对于字符设备或者FIFO类型的文件，定义文件大小是没有意义的，所以该字段将显示一个偏移值。

* NODE: 文件的i节点号。对于socket，则显示为协议类型，如TCP

* NAME: 文件的名字 



## 2. nc命令
nc(netcat)命令短小精干、功能强大，有着```瑞士军刀```的美誉。它主要被用来快速构建网络连接。我们可以让它以服务器方式运行，监听某个端口并接受客户连接，因此它可用来调试客户端程序；我们也可以使之以客户端方式运行，向服务器发起连接并收发数据，因此它可以用来调试服务器程序，此时它有点像telnet程序。

nc命令的基本用法如下：
{% highlight string %}
# nc -h
OpenBSD netcat (Debian patchlevel 1.105-7ubuntu1)
This is nc from the netcat-openbsd package. An alternative nc is available
in the netcat-traditional package.
usage: nc [-46bCDdhjklnrStUuvZz] [-I length] [-i interval] [-O length]
          [-P proxy_username] [-p source_port] [-q seconds] [-s source]
          [-T toskeyword] [-V rtable] [-w timeout] [-X proxy_protocol]
          [-x proxy_address[:port]] [destination] [port]
        Command Summary:
                -4              Use IPv4
                -6              Use IPv6
                -b              Allow broadcast
                -C              Send CRLF as line-ending
                -D              Enable the debug socket option
                -d              Detach from stdin
                -h              This help text
                -I length       TCP receive buffer length
                -i secs         Delay interval for lines sent, ports scanned
                -j              Use jumbo frame
                -k              Keep inbound sockets open for multiple connects
                -l              Listen mode, for inbound connects
                -n              Suppress name/port resolutions
                -O length       TCP send buffer length
                -P proxyuser    Username for proxy authentication
                -p port         Specify local port for remote connects
                -q secs         quit after EOF on stdin and delay of secs
                -r              Randomize remote ports
                -S              Enable the TCP MD5 signature option
                -s addr         Local source address
                -T toskeyword   Set IP Type of Service
                -t              Answer TELNET negotiation
                -U              Use UNIX domain socket
                -u              UDP mode
                -V rtable       Specify alternate routing table
                -v              Verbose
                -w secs         Timeout for connects and final net reads
                -X proto        Proxy protocol: "4", "5" (SOCKS) or "connect"
                -x addr[:port]  Specify proxy address and port
                -Z              DCCP mode
                -z              Zero-I/O mode [used for scanning]
        Port numbers can be individual or ranges: lo-hi [inclusive]
{% endhighlight %}
下面我们简要介绍几个常用的选项：

* -i: 设置数据包传送的时间间隔

* -l: 以服务器方式运行，监听指定的端口。nc命令默认以客户端方式运行

* -k: 重复接受并处理某个端口上的所有连接，必须与```-l```选项一起使用。用本选项强制 nc 待命链接，因为当客户端从服务端断开连接后，过一段时间服务端也会停止监听。但通过选项```-k```我们可以强制服务器保持连接并继续监听端口。

* -n: 使用IP地址表示主机，而不是主机名；使用数字表示端口号，而不是服务名称

* -p: 当nc命令以客户端方式运行时，强制其使用指定的端口号

* -s: 设置本地主机发送出的数据包的IP地址

* -C: 将CR和LF两个字符作为行结束符

* -U: 使用Unix本地域协议通信

* -u: 使用UDP协议。nc命令默认使用的传输层协议时TCP协议

* -w: 如果nc客户端在指定的时间内未检测到任何输入，则退出

* -X: 当nc客户端和代理服务器通信时，该选项指定它们之间使用的通信协议。目前nc支持的代理协议包括```SOCKS v.4```、```SOCKs v.5```和```connect```(HTTPS proxy)。nc默认使用的代理协议时```SOCKS v.5```。

* -x: 指定目标代理服务器的IP地址和端口号。比如，要从**Kongming20**连接到**ernest-laptop**上的squid代理服务器，并通过它来访问**www.baidu.com**的Web服务，可以使用如下命令
<pre>
# nc -x ernest-laptop:1080 -X connect www.baidu.com 80
</pre>

* -z: 扫描目标机器上的某个或某些服务是否开启（端口扫描）。比如，要扫描机器**ernest-laptop**上端口号在20~50之间的服务，可以使用如下命令
<pre>
# nc -z ernest-laptop 20-50
</pre>

举例来说，我们可以使用如下方式来连接```websrv```服务器并向它发送数据：
{% highlight string %}
# nc -C 127.0.0.1 13579   (服务器监听端口13579)
GET http://localhost/a.html HTTP/1.1(回车)
Host: localhost(回车)
(回车)
HTTP/1.1 404 Not found
Content-Length: 49
Connection: close

The requested file was not found on this server.
{% endhighlight %}

这里我们使用```-C```选项，这样每次我们按下回车键向服务器发送一行数据时，nc客户端程序都会给服务器额外发送一个```<CR><LF>```，而这正是```websrv```服务器期望的HTTP行结束符。发送完第三行数据之后，我们得到了服务器的响应，内容正是我们所期望的： 服务器没有找到被请求的资源文件a.html。可见，nc命令是一个很方便的快速测试工具，通过它我们能很快找出服务器的逻辑错误。





## 3. strace命令
strace是测试服务器性能的重要工具。它跟踪程序运行过程中执行的系统调用和接收到的信号，并将系统调用名、参数、返回值及信号名输出到标准输出或者指定的文件中。

strace命令常用的选项包括：

* -c: 统计每个系统调用执行时间、执行次数和出错次数

* -f: 跟踪由fork调用生成的子进程

* -t: 在输出的每一行信息前加上时间信息

* -e: 指定一个表达式，用来控制如何跟踪系统调用（或接收到的信号），其格式为
<pre>
[qualifier]=] [!]value1[,value2]...
</pre>

qualifier可以是trace、abbrev、verbose、raw、signal、read和write中之一，默认是trace。value是用于进一步限制被跟踪的系统调用的符号或数值。它的两个特殊取值是```all```和```none```，分别表示跟踪所有由qualifier指定类型的系统调用和不跟踪任何该类型的系统调用。关于value的其他取值，我们下面简单例举一些：
{% highlight string %}
1) -e trace=set: 只跟踪指定的系统调用。例如， -e trace=open,close,read,write表示只跟踪open、
   close、read和write这四种系统调用。

2) -e trace=file: 只跟踪与文件操作相关的系统调用

3) -e trace=process: 只跟踪与进程控制相关的系统调用

4） -e trace=network: 只跟踪与网络相关的系统调用

5) -e trace=signal: 只跟踪与信号相关的系统调用

6) -e trace=ipc: 只跟踪与进程间通信相关的系统调用

7) -e signal=set: 只跟踪指定的信号。比如， -e signal=!SIGIO表示跟踪除SIGIO之外的所有信号

8) -e read=set: 输出从指定文件中读入的数据。例如， -e read=3,5表示输出所有从文件描述符3和5读入的数据

{% endhighlight %}

* -o: 将strace的输出写入指定的文件

* -p: 指定要跟踪的进程号

strace命令的每一行输出都包含这些字段：系统调用名称、参数和返回值。例如下面的示例：
<pre>
# strace cat /dev/null
open("/dev/null", O_RDONLY|O_LARGEFILE) = 3
</pre>
这行输出表示： 程序```cat /dev/null```在运行过程中执行了open()系统调用。open调用以只读的方式打开了大文件/dev/null，然后返回了一个值为3的文件描述符。需要注意的是，其实上面示例命令将输出很多内容，这里我们忽略了很多次要的信息。

当系统调用发生错误时，strace命令将输出错误标识和描述，比如下面的示例：
<pre>
# strace cat /foo/bar 
open("/foo/bar", O_RDONLY|O_LARGEFILE)  = -1 ENOENT (No such file or directory)
</pre>




## 4. netstat命令
netstat是一个功能很强大的网络信息统计工具。它可以打印本地网卡接口上的全部连接、路由表信息、网卡接口信息等。这里我们主要用netstat命令来获取连接信息。毕竟，对于要获取路由表和网卡接口信息，我们可以使用输出内容更丰富的route和ifconfig命令。

netstat命令常用选项包括：

* -n: 使用IP地址表示主机，而不是主机名；使用数字表示端口号，而不是服务名称

* -a: 显示结果中也包含监听socket（default: connected)

* -t: 仅显示TCP连接

* -r: 显示路由信息

* -i: 显示网卡接口的数据流量

* -c: 每隔1秒输出一次

* -o: 显示socket定时器（如包活定时器）的信息

* -p: 显示socket所属的进程的PID和名字

* -l: 仅显示处于listen状态的服务器socket

参看如下示例：
<pre>
# netstat -nat
Active Internet connections (servers and established)
Proto Recv-Q Send-Q Local Address           Foreign Address         State      
tcp        0      0 127.0.1.1:53            0.0.0.0:*               LISTEN     
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN     
tcp        0      0 192.168.10.129:22       192.168.10.130:4046     ESTABLISHED
tcp        0     52 192.168.10.129:22       192.168.10.1:1131       ESTABLISHED
tcp6       0      0 :::22                   :::*                    LISTEN  
</pre>
由以上结果可知，netstat的每行输出都包含如下6个字段（默认情况）:

* Proto: 协议名

* Recv-Q: socket内核接收缓冲区中尚未被应用程序读取的数据量

* Send-Q: 尚未被对方确认的数据量

* Local Address: 本端的IP地址和端口号

* Foreign Address: 对方的IP地址和端口号

* State: socket的状态。对于无状态协议，比如UDP协议，这一字段将显示为空。而对面向连接的协议而言，netstat支持的State包括ESTABLISHED、SYN_SENT、SYN_RCVD、FIN_WAIT1、FIN_WAIT2、TIME_WAIT、CLOSE、CLOSE_WAIT、LAST_ACK、LISTEN、CLOSING、UKNOWN。


## 5. vmstat命令
vmstat是virtual memory statistics的缩写，它能实时输出系统的各种资源的使用情况，比如进程信息、内存使用、CPU使用率以及IO使用情况。vmstat命令的基本用法如下：
{% highlight string %}
# vmstat --help

Usage:
 vmstat [options] [delay [count]]

Options:
 -a, --active           active/inactive memory
 -f, --forks            number of forks since boot
 -m, --slabs            slabinfo
 -n, --one-header       do not redisplay header
 -s, --stats            event counter statistics
 -d, --disk             disk statistics
 -D, --disk-sum         summarize disk statistics
 -p, --partition <dev>  partition specific statistics
 -S, --unit <char>      define display unit
 -w, --wide             wide output
 -t, --timestamp        show timestamp

 -h, --help     display this help and exit
 -V, --version  output version information and exit

For more details see vmstat(8).
{% endhighlight %}
下面简单介绍一下各选项及参数的含义：


* -f: 显示系统自启动以来执行的fork次数

* -s: 显示内存相关的统计信息以及多种系统活动的数量（比如CPU上下文切换次数）

* -d: 显示硬盘相关统计信息

* -p: 显示指定磁盘分区的统计信息

* -S: 使用指定的单位来显示。参数k、K、m、M分别代表1000、1024、1000000和1048576字节

* delay: 采样间隔（单位： 秒），即每隔delay的时间输出一次统计信息

* count: 采样次数，即共输出count次统计信息

默认情况下```vmstat```命令输出的内容相当丰富。请参看如下示例：
<pre>
//每隔5秒输出一次，共输出三次
# vmstat 5 3
procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
 r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
 0  0      0 519648  78076 982796    0    0   419   442  319  335 58 11 28  3  0
 1  0      0 519452  78076 982796    0    0     0     0   46  120  1  1 98  0  0
 0  0      0 519420  78076 982796    0    0     0     2   52  142  1  1 97  0  0
</pre>
注意，第一行输出是自系统启动以来的平均结果，而后面的输出则是采样间隔内的平均结果。vmstat的每条输出都包含6个字段，它们的含义分别是：

* procs：进程信息。```r```表示等待运行的进程数目，```b```表示处于不可中断睡眠状态的进程数目

* memory: 内存信息，各项的单位是KB。```swpd```表示虚拟内存的使用数量；```free```表示空闲内存的数量；```buff```表示作为**buffer cache**的内存数量,从磁盘读入的数据可能被保持在**buffer cache**中，以便下一次快速访问；```cache```表示作为**page cache**的内存数量，待写入磁盘的数据首先被放到**page cache**中，然后由磁盘中断程序写入磁盘.

* swap: 交换分区（虚拟内存）的使用信息，各项的单位都是```KB/s```。```si```表示数据由磁盘交换至内存的速率；```so```表示数据由内存交换至磁盘的速率。如果这两个值经常发生变化，则说明内存不足

* io: 块设备的使用信息，单位是```block/s```。```bi```表示从块设备读入块的速率；```bo```表示向块设备写入块的速率。

* system: 系统信息。```in```表示每秒发生的中断次数；```cs```表示每秒发生的上下文切换（进程切换）次数

* cpu： CPU使用信息。```us```表示系统所有进程运行在用户空间的时间占CPU总运行时间的比例；```sy```表示系统所有进程运行在内核空间的时间占CPU总运行时间的比例；```id```表示CPU处于空闲状态的时间占CPU总运行时间的比例；```wa```表示CPU等待IO事件的时间占CPU总运行时间的比例

不过，我们可以使用iostat命令获得磁盘使用情况的更多信息，也可以使用mpstat命令获得CPU使用情况的更多信息。vmstat命令主要用于查看系统内存的使用情况。


## 6. ifstat命令
ifstat是interface statistics的缩写，它是一个简单的网络流量监测工具。常用的选项和参数包括：

* -a: 监测系统上的所有网卡接口

* -i: 指定要监测的网卡接口

* -t: 在每行输出信息前加上时间戳

* -b： 以```Kbit/s```为单位显示数据，而不是默认的```	KB/s```

* delay: 采样间隔（单位是: 秒)，即每隔delay的时间输出一次统计信息

* count： 采样次数，即共输出count次统计信息



参看如下示例（注： 实际使用时可能由于ifstat版本不一致，导致有些选项不能正常使用）：
<pre>
# ifstat -a 
#32051.1804289383 sampling_interval=2 time_const=60
Interface        RX Pkts/Rate    TX Pkts/Rate    RX Data/Rate    TX Data/Rate  
                 RX Errs/Drop    TX Errs/Drop    RX Over/Rate    TX Coll/Rate  
lo                 3693M 702       3693M 702       1688M 313K      1688M 313K   
                       0 0             0 0             0 0             0 0      
eth1                   0 0             0 0             0 0             0 0      
                       0 0             0 0             0 0             0 0      
eth2             839555K 865       1078M 844       1272M 431K    491797K 416K   
                       0 11120K        0 0             0 0             0 0      
eth3               1121M 1K        1008M 1K      899871K 442K    399558K 455K   
                       0 11120K        0 0             0 0             0 0      
</pre>


## 7. iftop命令
iftop是一款实时流量监测工具，监控TCP/IP连接等，缺点是无报表功能。必须以root身份才能运行。下面简单介绍一下该工具的使用：
<pre>
# iftop -h
iftop: display bandwidth usage on an interface by host

Synopsis: iftop -h | [-npblNBP] [-i interface] [-f filter code]
                               [-F net/mask] [-G net6/mask6]

   -h                  display this message
   -n                  don't do hostname lookups
   -N                  don't convert port numbers to services
   -p                  run in promiscuous mode (show traffic between other
                       hosts on the same network segment)
   -b                  don't display a bar graph of traffic
   -B                  Display bandwidth in bytes
   -i interface        listen on named interface
   -f filter code      use filter code to select packets to count
                       (default: none, but only IP packets are counted)
   -F net/mask         show traffic flows in/out of IPv4 network
   -G net6/mask6       show traffic flows in/out of IPv6 network
   -l                  display and count link-local IPv6 traffic (default: off)
   -P                  show ports as well as hosts
   -m limit            sets the upper limit for the bandwidth scale
   -c config file      specifies an alternative configuration file
   -t                  use text interface without ncurses

   Sorting orders:
   -o 2s                Sort by first column (2s traffic average)
   -o 10s               Sort by second column (10s traffic average) [default]
   -o 40s               Sort by third column (40s traffic average)
   -o source            Sort by source address
   -o destination       Sort by destination address

   The following options are only available in combination with -t
   -s num              print one single text output afer num seconds, then quit
   -L num              number of lines to print

iftop, version 1.0pre4
copyright (c) 2002 Paul Warren <pdw@ex-parrot.com> and contributors
</pre>
下面简要介绍一下几个```选项```的含义：

* -n: 直接显示IP，不进行DNS反解析

* -N: 直接显示端口号，并不解析为服务名称

* -F： 可以用于显示某个网段进出封包流量
<pre>
# iftop -F 192.168.1.0/24 
# iftop -F 192.168.1.0/255.255.255.0
</pre>

* -B: 以bytes为单位显示流量(默认是bits)

iftop命令默认显示第一块网卡的流量：
{% highlight string %}
# iftop
interface: eth0
IP address is: 10.17.253.170
MAC address is: 38:ffffff90:ffffffa5:7f:7c:ffffffac
 Press H or ? for help            191Mb                             381Mb                             572Mb                             763Mb                         954Mb
+---------------------------------+---------------------------------+---------------------------------+---------------------------------+----------------------------------
ceph001-05                                                     => ceph001-02                                                      2.54Mb  2.54Mb  2.54Mb
                                                                        <=                                                                          2.10Mb  2.10Mb  2.10Mb
ceph001-05                                                     => ceph001-01                                                      2.94Mb  2.94Mb  2.94Mb
                                                                        <=                                                                          1.55Mb  1.55Mb  1.55Mb
ceph001-05                                                     => ceph001-03                                                      1.55Mb  1.55Mb  1.55Mb
                                                                        <=                                                                          1.79Mb  1.79Mb  1.79Mb
ceph001-05                                                     => ceph001-07                                                      1.47Mb  1.47Mb  1.47Mb
                                                                        <=                                                                          1.68Mb  1.68Mb  1.68Mb
ceph001-05                                                     => ceph001-09                                                      1.19Mb  1.19Mb  1.19Mb
                                                                        <=                                                                          1.40Mb  1.40Mb  1.40Mb
ceph001-05                                                     => ceph001-08                                                       966Kb   966Kb   966Kb
                                                                        <=                                                                           981Kb   981Kb   981Kb
ceph001-05                                                     => ceph001-06                                                       272Kb   272Kb   272Kb
                                                                        <=                                                                           271Kb   271Kb   271Kb
ceph001-05                                                     => 10.17.154.52                                                              465Kb   465Kb   465Kb
                                                                        <=                                                                          4.17Kb  4.17Kb  4.17Kb
ceph001-05                                                     => 172.28.160.243                                                            442Kb   442Kb   442Kb
                                                                        <=                                                                          12.4Kb  12.4Kb  12.4Kb
ceph001-05                                                     => ceph001-04                                                       176Kb   176Kb   176Kb
                                                                        <=                                                                           181Kb   181Kb   181Kb
ceph001-05                                                     => 10.17.144.211                                                            11.7Kb  11.7Kb  11.7Kb
                                                                        <=                                                                           112Kb   112Kb   112Kb
ceph001-05                                                     => 10.17.144.197                                                            3.31Kb  3.31Kb  3.31Kb
                                                                        <=                                                                          6.77Kb  6.77Kb  6.77Kb
ceph001-05                                                     => 10.16.11.106                                                             3.64Kb  3.64Kb  3.64Kb
                                                                        <=                                                                          6.41Kb  6.41Kb  6.41Kb
ceph001-05                                                     => 10.17.144.217                                                            4.70Kb  4.70Kb  4.70Kb
                                                                        <=                                                                          3.48Kb  3.48Kb  3.48Kb
ceph001-05                                                     => 10.133.146.47                                                            5.22Kb  5.22Kb  5.22Kb
                                                                        <=                                                                          2.23Kb  2.23Kb  2.23Kb
ceph001-05                                                     => 10.17.147.26                                                             3.72Kb  3.72Kb  3.72Kb
                                                                        <=                                                                          2.53Kb  2.53Kb  2.53Kb
ceph001-05                                                     => 10.17.147.29                                                             3.49Kb  3.49Kb  3.49Kb
                                                                        <=                                                                          2.09Kb  2.09Kb  2.09Kb
ceph001-05                                                     => 10.17.152.88                                                              416b    416b    416b
                                                                        <=                                                                           416b    416b    416b
ceph001-05                                                     => 172.28.160.241                                                            208b    208b    208b
                                                                        <=                                                                           368b    368b    368b
ceph001-05                                                     => 10.58.8.126                                                               160b    160b    160b
                                                                        <=                                                                           320b    320b    320b

---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
TX:             cum:   4.27MB   peak:   12.0Mb                                                                                             rates:   12.0Mb  12.0Mb  12.0Mb
RX:                    3.80MB           10.1Mb                                                                                                      10.1Mb  10.1Mb  10.1Mb
Total:                 59.4MB           38.9Mb                                                                                                      34.1Mb  33.8Mb  34.0Mb
{% endhighlight %}

下面简要介绍一下上面打印信息的含义。iftop的输出从整体上可以分为三大部分：

* 第一部分是iftop输出中最上面的一行，此行是流量刻度，用于显示网卡带宽流量

* 第二部分是iftop输出中最大的一个部分，此部分又分为左中右三列，左列和中列记录了哪些IP或主机正在与本机的网络进行连接。其中中列的```=>```代表发送数据，而```<=```代表接收数据，通过这个指示箭头可以很清晰的知道两个IP之间的通信情况。最右列又分为三个小列，这些实时参数分别表示外部IP连接到本机2s、10s和40s的平均流量。另外这个部分还有一个流量图形条，流量图形条是对流量大小的动态展示，以第一部分中的流量刻度为基准。通过这个流量图形条可以很方便的看出哪个IP的流量最大，从而迅速定位网络中可能出现的流量问题。

* 第三部分位于iftop输出的最下面，可以分为三行，其中```TX```表示发送的数据，```RX```表示接收的数据，```TOTAL```表示发送和接收的全部流量。与这三行对应的有三列，其中```cum列```表示从运行iftop到目前的发送、接收和总数据流量； ```peak列```表示发送、接收以及总的流量峰值； ```rates列```表示过去2s、10s和40s的平均流量值；




## 8. Linux性能监控工具sysstat

sysstat提供了Linux性能监控的工具集，包括： sar、sadf、mpstat、iostat、pidstat等，这些工具可以监控系统性能和实用情况。各工具的作用如下：

* iostat: 用于监控系统的IO设备的平均负载；对于CPU的负载，一般只能查看所有CPU的平均信息

* mpstat: 提供单个或组合CPU相关统计

* pidstat: 提供Linux进程级别统计（IO、CPU、内存等）

* sar： 收集、报告、保存系统活动信息（CPU、内存、磁盘、中断、网络接口、TTY、内核表等）

* sadc: 系统活动数据收集器、作为sar后端使用

* sa1: 收集系统活动日常数据，并以二进制格式存储，它作为sadc工具的前端，可以通过cron来调用

* sa2: 生成系统每日活动报告，同样作为```sadc```工具的前端，可以通过cron来调用

* sadf: 可以CSV、XML等格式显示```sar```收集的性能数据，这样可非常方便的将系统数据导入到数据库，或导入Excel中来生成图表

* nfsiostat: sysstat工具集中提供的NFS IO统计

* cifsiostat: 提供CIFS统计

sysstat功能强大并且在不断增强，每个版本提供了一些不同的功能，用户可以到[sysstat官网](http://sebastien.godard.pagesperso-orange.fr/)了解工具最新发展状况和获得相应的帮助手册。




### 8.1 sysstat工具集的安装

通常情况下，我们在Centos操作系统上可以直接通过如下的命令进行安装：
<pre>
# yum install sysstat
</pre>
我们也可以在官网下载源代码来进行安装：
<pre>
# wget http://pagesperso-orange.fr/sebastien.godard/sysstat-11.0.5.tar.xz
# xz -d sysstat-11.05.tar.xz
# tar -xvf sysstat-11.05.tar
# cd sysstat-11.05/
# ./configure
# make
# make install
</pre>

### 8.2 iostat命令
iostat是IO statistics的缩写，iostat工具将对系统的磁盘操作活动进行监视。iostat首次运行时显示自系统启动开始的各项统计信息，之后运行iostat将显示自上次运行该命令以后的统计信息。用户可以通过指定统计的次数和时间来获得所需的统计信息。iostat也有一个弱点，就是它不能对某个进程进行深入分析，仅对系统的整体情况进行分析。iostat命令的基本用法如下：
{% highlight string %}
# iostat --help
Usage: iostat [ options ] [ <interval> [ <count> ] ]
Options are:
[ -c ] [ -d ] [ -h ] [ -k | -m ] [ -N ] [ -t ] [ -V ] [ -x ] [ -y ] [ -z ]
[ -j { ID | LABEL | PATH | UUID | ... } ]
[ [ -H ] -g <group_name> ] [ -p [ <device> [,...] | ALL ] ]
[ <device> [...] | ALL ]
{% endhighlight %}
下面简要介绍一下几个常用选项的含义：

* -d: 显示设备（磁盘）使用状态

* -c: 显示CPU的使用状况

* -k: 以KB/s为单位来显示统计信息

* -p: 显示某一块设备的使用状况。如sda、sdb

* -x: 显示更详细的扩展信息

参看如下示例：
<pre>
//每隔1s显示一次，总共显示2次
# iostat -d -k 1 2
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

Device:            tps    kB_read/s    kB_wrtn/s    kB_read    kB_wrtn
scd0              0.00         0.00         0.00         64          0
scd1              0.00         0.00         0.00         76          0
sda               1.58        25.91        28.56     619695     683108

Device:            tps    kB_read/s    kB_wrtn/s    kB_read    kB_wrtn
scd0              0.00         0.00         0.00          0          0
scd1              0.00         0.00         0.00          0          0
sda               0.00         0.00         0.00          0          0
</pre>
上面总共有6列，我们简单介绍一下各列的含义：

* Device: 设备名

* tps: 该设备每秒的传输次数。```一次传输```意思是一次IO请求。多个逻辑请求可能会被合并为一次IO请求。```一次传输```请求的大小是未知的。

* kB_read/s: 每秒从设备读取的数据量

* kB_wrtn/s: 每秒向设备写入的数据量

* kB_read: 读取的总数据量

* kB_wrtn: 写入的总数据量

如果我们使用```-x```选项，则可以获取更详细的信息：

<pre>
# iostat -d -k -x 1 2
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

Device:         rrqm/s   wrqm/s     r/s     w/s    rkB/s    wkB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
scd0              0.00     0.00    0.00    0.00     0.00     0.00     6.74     0.00   96.00   96.00    0.00  96.00   0.01
scd1              0.00     0.00    0.00    0.00     0.00     0.00     5.85     0.00   68.62   68.62    0.00  68.62   0.01
sda               0.02     0.68    1.23    0.31    25.37    27.97    69.02     0.11   69.43   26.03  239.55   8.53   1.32

Device:         rrqm/s   wrqm/s     r/s     w/s    rkB/s    wkB/s avgrq-sz avgqu-sz   await r_await w_await  svctm  %util
scd0              0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
scd1              0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
sda               0.00     0.00    0.00    0.00     0.00     0.00     0.00     0.00    0.00    0.00    0.00   0.00   0.00
</pre>
下面我们简要介绍一下各字段的含义：

* rrqm/s: 设备相关的读取请求每秒有多少被Merge了（当系统调用需要读取数据的时候，VFS将请求发到各个FS，如果FS发现不同的读取请求读取相同的Block的数据，FS会将这个请求合并)；

* wrqm/s：设备相关的写入请求每秒有多少被Merge了。

* r/s: 设备每秒钟完成的读请求数(merge之后)

* w/s: 设备每秒钟完成的写请求数(merge之后)

* rsec/s(rKB/s, rMB/s): 每秒读取的扇区数（KB字节数， MB字节数)

* wsec/s(wKB/s, wMB/s): 每秒写入的扇区数(KB字节数， MB字节数)

* avgrq-sz: 请求的平均大小(单位： 扇区)

* avgqu-sz: 平均请求队列的长度。毫无疑问，队列长度越短越好。

* await: 每一个IO请求处理的平均时间（单位：毫秒）。这里可以理解为IO的响应时间，一般系统IO响应时间应该低于5ms，如果大于10ms就比较大了。这个时间包括了队列时间和服务时间，也就是说，一般情况下，await大于svctm，它们的差值越小，则说明队列时间越短，反之差值越大，队列时间越长，说明系统出了问题。

* r_await: 每个读请求的平均时间（单位： 毫秒）

* w_await: 每个写请求的平均时间（单位： 毫秒）
    
* svctm: 表示平均每次设备I/O操作的服务时间（单位： 毫秒）。如果svctm的值与await很接近，表示几乎没有I/O等待，磁盘性能很好，如果await的值远高于svctm的值，则表示I/O队列等待太长，系统上运行的应用程序将变慢。

* %util: 在统计时间内所有处理IO的时间，除以总共统计时间。例如，如果统计间隔为1秒，该设备有0.8秒在处理IO，而0.2秒闲置，那么该设备的**%util=0.8/1=80%**，所以该参数暗示了设备的繁忙程度。一般地，如果该参数是100%表示设备已经接近满负荷运行了（当然如果是多磁盘，即使%util是100%，因为磁盘的并发能力，所以磁盘使用未必就到了瓶颈）。



### 8.2 mpstat命令
mpstat是multi-processor statistics的缩写，它能实时监测多处理器系统上每个CPU的实用情况。mpstat命令的典型用法是：
<pre>
# mpstat [ -A ] [ -u ] [ -V ] [ -I { SUM | CPU | SCPU | ALL } ] [ -P { cpu [,...] | ON | ALL } ] [ interval [ count ] ]
</pre>
选项```P```指定要监控的CPU号(范围是[0,CPU_cnt-1])，其中值```ALL```表示监测所有的CPU。```interval```参数是采样间隔（单位是:秒)，即每隔interval的时间输出一次统计信息。```count```参数是采样次数，即共输出count次统计信息，但最后还会输出这count次采样结果的平均值。与```vmstat```命令一样，mpstat命令输出的第一次结果是自系统启动以来的平均结果，而后面(count-1)次输出结果则是采样间隔内的平均结果。例如：
<pre>
//命令每隔5s输出一次结果，功输出2次
# mpstat -P ALL 5 2
Linux 3.10.0-327.el7.x86_64 (ceph001-node1)     01/11/2019      _x86_64_        (40 CPU)

07:17:34 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
07:17:39 PM  all    0.09    0.00    0.15    0.00    0.00    0.03    0.00    0.00    0.00   99.74
07:17:39 PM    0    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM    1    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM    2    0.20    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.40
07:17:39 PM    3    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM    4    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM    5    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM    6    0.20    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.40
07:17:39 PM    7    0.20    0.00    0.40    0.00    0.00    0.00    0.00    0.00    0.00   99.40
07:17:39 PM    8    0.40    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.40
07:17:39 PM    9    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   10    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   11    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   12    0.00    0.00    0.00    0.00    0.00    0.20    0.00    0.00    0.00   99.80
07:17:39 PM   13    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   14    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM   15    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   16    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   17    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   18    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM   19    0.00    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.60
07:17:39 PM   20    0.00    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.60
07:17:39 PM   21    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   22    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   23    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   24    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   25    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM   26    0.20    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.40
07:17:39 PM   27    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:39 PM   28    0.00    0.00    0.20    0.00    0.00    0.20    0.00    0.00    0.00   99.60
07:17:39 PM   29    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   30    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:39 PM   31    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   32    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   33    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:39 PM   34    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:39 PM   35    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   36    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   37    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:39 PM   38    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:39 PM   39    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80

07:17:39 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
07:17:44 PM  all    0.07    0.00    0.10    0.00    0.00    0.02    0.00    0.00    0.00   99.81
07:17:44 PM    0    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM    1    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM    2    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM    3    0.20    0.00    0.00    0.00    0.00    0.20    0.00    0.00    0.00   99.60
07:17:44 PM    4    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM    5    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM    6    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM    7    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM    8    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM    9    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM   10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   11    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   12    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   13    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   14    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   15    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   16    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   17    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   18    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   19    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   20    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   21    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   22    0.40    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM   23    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   24    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   25    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   26    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   27    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
07:17:44 PM   28    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   29    0.20    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   30    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   31    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
07:17:44 PM   32    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   33    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   34    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   35    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   36    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   37    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   38    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
07:17:44 PM   39    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00

Average:     CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest  %gnice   %idle
Average:     all    0.08    0.00    0.12    0.00    0.00    0.02    0.00    0.00    0.00   99.78
Average:       0    0.10    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:       1    0.10    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:       2    0.20    0.00    0.20    0.00    0.00    0.10    0.00    0.00    0.00   99.50
Average:       3    0.10    0.00    0.10    0.00    0.00    0.10    0.00    0.00    0.00   99.70
Average:       4    0.20    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.60
Average:       5    0.20    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:       6    0.20    0.00    0.10    0.00    0.00    0.10    0.00    0.00    0.00   99.60
Average:       7    0.10    0.00    0.30    0.00    0.00    0.00    0.00    0.00    0.00   99.60
Average:       8    0.30    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.50
Average:       9    0.10    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:      10    0.10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      11    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      12    0.00    0.00    0.10    0.00    0.00    0.10    0.00    0.00    0.00   99.80
Average:      13    0.00    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      14    0.10    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:      15    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      16    0.00    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      17    0.00    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      18    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      19    0.00    0.00    0.20    0.00    0.00    0.10    0.00    0.00    0.00   99.70
Average:      20    0.00    0.00    0.20    0.00    0.00    0.10    0.00    0.00    0.00   99.70
Average:      21    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      22    0.20    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:      23    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      24    0.00    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      25    0.10    0.00    0.20    0.00    0.00    0.00    0.00    0.00    0.00   99.70
Average:      26    0.10    0.00    0.20    0.00    0.00    0.10    0.00    0.00    0.00   99.60
Average:      27    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      28    0.00    0.00    0.10    0.00    0.00    0.10    0.00    0.00    0.00   99.80
Average:      29    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      30    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
Average:      31    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      32    0.10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      33    0.10    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.80
Average:      34    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
Average:      35    0.10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      36    0.10    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      37    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
Average:      38    0.00    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.90
Average:      39    0.00    0.00    0.10    0.00    0.00    0.00    0.00    0.00    0.00   99.90
</pre>

上面每条信息都包含如下几个字段：

* CPU: 指示该条信息是哪个CPU的数据。```0```表示是第一个CPU的数据，```1```表示是第二个CPU的数据，```all```则表示所有CPU数据的平均值

* %usr: 除了nice值为负的进程，系统上其他进程运行在用户空间的时间占CPU总运行时间的比例

* %nice: nice值为负的进程运行在用户空间的时间占CPU总运行时间的比例

* %sys: 系统上所有进程运行在内核空间的时间占CPU总运行时间的比例，但不包括硬件和软件中断消耗的CPU时间

* %iowait: CPU等待磁盘操作的时间占CPU总运行时间的比例

* %irq: CPU用于处理硬件中断的时间占CPU总运行时间的比例

* %soft: CPU用于处理软件中断的时间占CPU总运行时间的比例

* %steal: 一个物理CPU可以包含一对虚拟CPU，这一对虚拟CPU由超级管理程序管理。当超级管理程序在处理某个虚拟CPU时，另一个虚拟CPU则必须等待它处理完成才能运行。这部分时间就是所谓的steal时间。该字段表示steal时间占CPU总运行时间的比例

* %guest: 运行虚拟CPU的时间占CPU总运行时间的比例

* %idle： 系统空闲的时间占CPU总运行时间的比例

在所有这些输出字段中，我们最关心的是```%usr```、```%sys```以及```%idle```。它们基本上反映了我们的代码中业务逻辑代码和系统调用所占的比例，以及系统还能承受多大的负载。很显然，在上面的输出中，执行系统调用占用CPU的时间，以及用户业务逻辑调用占用的CPU时间都是极低的。


----------
我们在使用top或ps命令时，会输出PRI/PR、NI、%ni/%nice这三种指标值，如下我们简单介绍一下：

* PRI: 进程优先权，代表这个进程可被执行的优先级，其值越小，优先级就越高，越早被执行；

* NI： 进程nice值，代表这个进程的优先值

* %nice: 改变过优先级的进程占用CPU的百分比

```PRI```比较好理解，即进程优先级，或者通俗点说就是程序被CPU执行的先后顺序，此值越小进程的优先级别越高。那```NI```呢？就是我们要说的nice值了，其表示进程可被执行的优先级的修正值。如前面所说，PRI值越小越快被执行，那么加入nice值后，将会使得PRI变为：PRI(new) = PRI(old) + nice。由此看出，PR是根据nice排序的，规则是nice值越小PR越前，即其优先级会变高，则其越快被执行。如果nice相同，则进程uid是root的优先权更大。

在Linux系统中，nice值的范围从-20到+19（不同系统，值的取值范围可能不一样），正值表示低优先级，负值表示高优先级，值为0表示不会调整该进程的优先级。具有最高优先级的程序，其nice值最低，所以在Linux系统中，nice值为```-20```使得一项任务变得非常重要；与之相反，如果任务的nice值为```+19```，则表示它是一个高尚的、无私的任务，允许所有其他任务比自己享有更大份额的CPU使用时间，这也就是nice名称的来意。

进程在创建时被赋予不同的优先级，而如前面所说，nice值是表示进程优先级可被修正的数据值，因此，每个进程都在其计划执行时被赋予一个nice值，这样系统就可以根据各类资源的消耗情况，主动干预进程的优先级。在通常情况下，子进程会继承父进程的nice值，比如在系统启动的过程中，init进程会被赋予0，其他所有进程继承了这个nice值（因为其他进程都是init的子进程）。


### 8.3 sar命令

sar(System Activity Reporter, 系统活动情况报告）： 用于监控Linux系统各个性能的优秀工具，包括：文件的读写情况、系统调用的使用情况、磁盘IO、CPU效率、内存使用状况、进程活动及IPC有关的活动。sar命令的基本用法如下：
{% highlight string %}
# sar --help
Usage: sar [ options ] [ <interval> [ <count> ] ]
Options are:
[ -A ] [ -B ] [ -b ] [ -C ] [ -D ] [ -d ] [ -F [ MOUNT ] ] [ -H ] [ -h ]
[ -p ] [ -q ] [ -R ] [ -r [ ALL ] ] [ -S ] [ -t ] [ -u [ ALL ] ] [ -V ]
[ -v ] [ -W ] [ -w ] [ -y ] [ --sadc ]
[ -I { <int> [,...] | SUM | ALL | XALL } ] [ -P { <cpu> [,...] | ALL } ]
[ -m { <keyword> [,...] | ALL } ] [ -n { <keyword> [,...] | ALL } ]
[ -j { ID | LABEL | PATH | UUID | ... } ]
[ -f [ <filename> ] | -o [ <filename> ] | -[0-9]+ ]
[ -i <interval> ] [ -s [ <hh:mm[:ss]> ] ] [ -e [ <hh:mm[:ss]> ] ]
{% endhighlight %}
下面我们简要介绍一下一些常用的选项：

* -A: 所有报告的总和

* -u: 输出整体CPU使用情况的统计信息

* -v: 输出inode、文件和其他内核表的统计信息

* -d: 输出每一个块设备的活动信息

* -r: 输出内核和交换空间统计信息

* -b: 显示IO和传送速率的统计信息

* -a： 文件读写情况

* -c: 输出进程统计信息，每秒创建的进程数

* -R: 输出内存页面的统计信息

* -y: 终端设备活动情况

* -w: 输出系统交换活动信息

* -n: 报告网络相关信息

下文将说明如何使用sar获取以下性能分析数据：
* 整体CPU使用统计
* 各个CPU使用统计
* 内存使用情况统计
* 整体I/O情况
* 各个I/O设备情况
* 网络统计

**1） 整体CPU使用统计（-u)**

使用```-u```选项，sar输出整体CPU的使用情况，不加选项时，默认使用的就是```-u```选项。以下命令显示采样时间间隔为3s，采样次数为2次，整体CPU的使用情况：
<pre>
# sar 3 2
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

02:46:51 AM     CPU     %user     %nice   %system   %iowait    %steal     %idle
02:46:54 AM     all      0.00      0.00      0.00      0.00      0.00    100.00
02:46:57 AM     all      0.00      0.00      0.33      0.33      0.00     99.34
Average:        all      0.00      0.00      0.17      0.17      0.00     99.67
</pre>
各字段的含义参见上面mpstat.


**2） 各个CPU使用统计(-P)**

```-P ALL```选项指示对每个内核输出统计信息：
<pre>
# sar -P ALL 3 2
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

02:49:16 AM     CPU     %user     %nice   %system   %iowait    %steal     %idle
02:49:19 AM     all      0.00      0.00      0.00      0.00      0.00    100.00
02:49:19 AM       0      0.00      0.00      0.00      0.00      0.00    100.00

02:49:19 AM     CPU     %user     %nice   %system   %iowait    %steal     %idle
02:49:22 AM     all      0.00      0.00      0.00      0.00      0.00    100.00
02:49:22 AM       0      0.00      0.00      0.00      0.00      0.00    100.00

Average:        CPU     %user     %nice   %system   %iowait    %steal     %idle
Average:        all      0.00      0.00      0.00      0.00      0.00    100.00
Average:          0      0.00      0.00      0.00      0.00      0.00    100.00
</pre>


**3) 内存使用情况统计(-r)**

使用```-r```选项可显示内存统计信息:
<pre>
# sar -r 1 2
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

02:51:02 AM kbmemfree kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit  kbactive   kbinact   kbdirty
02:51:03 AM 6909468508349228 9226345614512488449     78.09 4221523355320324 9226345614515569168 13212300813261298768     74.16 2722098733442124 9226345614512488448 13212300813261298768
02:51:04 AM 6909468508349228 9226345614512488449     78.09 4221523355320324 9226345614515569168 13212300813261298752     74.16 2722098733442124 9226345614512488448 13212300813261298752
Average:       451372   1608736     78.09     81924    982900   3080720     74.16    844876    633788         0
</pre>
下面我们简单介绍一下各字段的含义：

* kbmemfree： 处于空闲状态的可用内存数量(单位: KB)

* kbmemused: 当前已使用的内存数量（单位: KB)。注意，这里并没有包含kernel本身所使用的内存

* %memused： 所使用内存占总内存大小的百分比

* kbbuffer: 表示作为**buffer cache**的内存数量,从磁盘读入的数据可能被保持在**buffer cache**中，以便下一次快速访问；

* kbcached: 表示作为**page cache**的内存数量，待写入磁盘的数据首先被放到**page cache**中，然后由磁盘中断程序写入磁盘.

* kbcommit: 当前工作负载所需要的内存总数(单位: KB)。 This is an estimate of how much RAM/swap is needed to guarantee that there never is out of memory.

* %commit: 当前工作负载所需要的内存占总内存大小（RAM+swap)的百分比。This number may be greater than 100% because the kernel usually overcommits memory.



**4) 整体I/O情况(-b)**

使用```-b```选项，可以显示磁盘I/O的使用情况:
<pre>
# sar -b 3 2 
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

02:54:07 AM       tps      rtps      wtps   bread/s   bwrtn/s
02:54:10 AM      0.00      0.00      0.00      0.00      0.00
02:54:13 AM      0.00      0.00      0.00      0.00      0.00
Average:         0.00      0.00      0.00      0.00      0.00
</pre>

输出项说明：

* tps: 每秒向磁盘设备请求数据的次数，包括读、写请求，为rtps与wtps的和。出于效率考虑，每一次IO下发后并不是立即处理请求，而是将请求合并(merge)，这里tps指请求合并后的请求计数。

* rtps: 每秒向磁盘设备的读请求次数

* wtps: 每秒向磁盘设备的写请求次数

* bread/s：每秒钟从物理设备读入的数据量，单位: 块/s

* bwrtn/s：每秒钟向物理设备写入的数据量，单位: 块/s

**5) 各个I/O设备情况(-d)**

使用```-d```选项可以显示各个磁盘的统计信息，再增加```-p```选项可以以```sdX```的形式显示设备名称:
<pre>
# sar -d -p 3 2 
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

02:56:22 AM       DEV       tps  rd_sec/s  wr_sec/s  avgrq-sz  avgqu-sz     await     svctm     %util
02:56:25 AM       sr0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
02:56:25 AM       sr1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
02:56:25 AM       sda      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00

02:56:25 AM       DEV       tps  rd_sec/s  wr_sec/s  avgrq-sz  avgqu-sz     await     svctm     %util
02:56:28 AM       sr0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
02:56:28 AM       sr1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
02:56:28 AM       sda      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00

Average:          DEV       tps  rd_sec/s  wr_sec/s  avgrq-sz  avgqu-sz     await     svctm     %util
Average:          sr0      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
Average:          sr1      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
Average:          sda      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
</pre>

输出项说明：
* rd_sec/s: 每秒从设备读取的扇区数

* wr_sec/s: 每秒往设备写入的扇区数

* avgrq-sz: 发送给设备的请求的平均大小（以扇区为单位）

* avgqu-sz: 发送给设备的请求队列的平均长度

* await ：服务等待I/O请求的平均时间，包括请求队列等待时间 (单位毫秒)

* svctm ：设备处理I/O请求的平均时间，不包括请求队列等待时间 (单位毫秒)

* %util ：一秒中有百分之多少的时间用于 I/O 操作，即被io消耗的cpu百分比

{% highlight string %}
备注：

- 如果 %util 接近 100%，说明产生的I/O请求太多，I/O系统已经满负荷，该磁盘可能存在瓶颈。

- 如果 svctm 比较接近 await，说明 I/O 几乎没有等待时间；如果 await 远大于 svctm，说明I/O 队列太长，io响应太慢，
  则需要进行必要优化。

- 如果avgqu-sz比较大，也表示有当量io在等待。
{% endhighlight %}


**6) 网络统计(-n)**

使用```-n```选项可以对网络使用情况进行显示，```-n```后接关键词**DEV**可显示eth0、eth1等网卡的信息:
<pre>
# sar -n DEV 1 1
Linux 4.8.0-36-generic (ubuntu)         01/12/2019      _i686_  (1 CPU)

03:01:47 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s   %ifutil
03:01:48 AM     ens33      1.00      0.00      0.06      0.00      0.00      0.00      0.00      0.00
03:01:48 AM     ens34      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
03:01:48 AM        lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00

Average:        IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s   %ifutil
Average:        ens33      1.00      0.00      0.06      0.00      0.00      0.00      0.00      0.00
Average:        ens34      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
Average:           lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00      0.00
</pre>

以上主要输出含义如下：

* IFACE: 网卡接口的名称

* rxpck/s: 每秒接收数据包的总数

* txpck/s: 每秒发送数据包的总数

* rxkB/s: 每秒接收数据包的总字节数(单位：kB/s)

* txkB/s: 每秒发送数据包的总字节数(单位:kB/s)

* rxcmp/s: 每秒接收到的```压缩包```(compressed packets)的数量。(for cslip etc.).

* txcmp/s: 每秒发送的```压缩包```(compressed packets)的数量。

* rxmcst/s: 每秒接收到的多播(multicast)数据包的数量


**7) sar日志保存(-o)**

最后讲一下如何保存sar日志，使用```-o```选项，我们可以把sar统计信息保存到一个指定的文件，对于保存的日志，我们可以使用```-f```选项读取：

<pre>
# sar -n DEV 1 10 -o sar.out

# sar -d 1 10 -f sar.out //查看历史的IO

# sar -u 1 10 -f sar.out //查看历史的cpu,单位1s, 采样10次
</pre>

相比将结果重定向到一个文件，使用-o选项，可以保存更多的系统资源信息

### 8.4 pidstat命令
pidstat命令被用于监视当前由Linux内核所管理的单个任务。下面简要说明一下pidstat命令的用法：

{% highlight string %}
# pidstat --help
Usage: pidstat [ options ] [ <interval> [ <count> ] ]
Options are:
[ -d ] [ -h ] [ -I ] [ -l ] [ -R ] [ -r ] [ -s ] [ -t ] [ -U [ <username> ] ]
[ -u ] [ -V ] [ -v ] [ -w ] [ -C <command> ] [ -G <process_name> ]
[ -p { <pid> [,...] | SELF | ALL } ] [ -T { TASK | CHILD | ALL } ]

Options:
  -C comm: 只显示出任务所对应的执行命令
 
  -d: 用于报告IO信息。使用此选项时会打印如下字段
      UID   ---  所监视任务的实际用户ID
      USER  ---  所监视任务的实际用户名称
      PID   ---  所监视任务的进程ID
      kB_rd/s
      kB_wr/s
      kB_ccwr/s
      iodelay --- 所监视任务的IO延迟，以clock ticks为单位。
      command

   -G process_name: 只打印那些执行命令名包含process_name的进程信息。假如和-t选项搭配使用的话，则
      还会打印属于该进程的线程信息

   -I: 对于SMP(多处理器)系统，本选项用于指明任务的CPU使用率应该除以当前的CPU总数

   -l: 用于显示该任务所对应的执行命令的名称及其参数

   -p {pid [,...] | SELF | ALL}: 用于选择监视哪些执行进程

   -R: 用于报告实时优先级和调用策略信息

   -r: 用于报告page faults以及内存使用信息

   -s: 用于报告栈使用信息

   -t: 打印所指定任务的线程信息

   -u: 报告CPU的使用信息
{% endhighlight %}


下面给出一个示例：
{% highlight string %}
# pidstat
Linux 2.6.32-642.4.2.el6.x86_64 (wille)     06/06/2018  _x86_64_    (4 CPU)

09:01:45 AM       PID    %usr %system  %guest    %CPU   CPU  Command
09:01:45 AM         1    0.00    0.00    0.00    0.00     1  init
09:01:45 AM      2033    0.00    0.00    0.00    0.00     0  sshd
09:01:45 AM      2044    0.00    0.00    0.00    0.00     0  ntpd
09:01:45 AM      2123    0.00    0.00    0.00    0.00     0  master
09:01:45 AM      2132    0.00    0.00    0.00    0.00     2  qmgr
09:01:45 AM      2137    0.00    0.00    0.00    0.00     0  crond
09:01:45 AM     15667    0.01    0.01    0.00    0.02     2  java
09:01:45 AM     19061    0.00    0.00    0.00    0.00     0  sshd
{% endhighlight %}
下面对上述各行的含义进行一个简单的说明：

* 第一行显示服务器内核信息、主机名、日期和CPU个数

* PID: 被监控任务的进程号

* Command: 这个任务的命令名称

1) **CPU统计数据(-u)**

* ```%usr```: 当在用户层执行（应用程序时）这个任务的CPU使用率，和nice优先级无关。注意这个字段计算的CPU时间不包括在虚拟处理器中花去的时间。

* ```%system```: 这个任务在系统层使用时的CPU使用率

* ```%guest```: 任务花费在虚拟机上的CPU使用率（运行在虚拟处理器）

* ```%cpu```: 任务总的CPU使用率。在SMP(多处理器）中，如果在命令行中输入```-I```参数的话，CPU使用率会除以你的CPU数量。

* CPU: 正在运行这个任务的处理器编号

2) **IO统计数据(-d)**

* kB_rd/s: 每秒进程从磁盘读取的数据量(以kB为单位)

* kB_rw/s: 每秒进程向磁盘写的数据量(以kB为单位)

* kB_ccwr/s: 任务写入磁盘被取消的速率(kb)

3) **内存使用统计(-r)**

* minflt/s: 每秒次缺页错误次数（minor page faults)，次缺页错误次数即虚拟内存地址映射成物理内存地址产生的Page fault次数。

* majflt/s: 每秒主缺页错误次数（major page faults)，当虚拟内存地址映射物理内存地址时，相应的page在swap中，这样的page在swap中，这样的page fault为major page fault，一般在内存使用紧张时产生。

* VSZ: 该进程所使用的虚拟内存(以kB为单位）

* RSS: 该进程所使用的物理内存（以kB为单位）

* ```%MEM```: 该进程使用内存的百分比

4) **上下文切换情况(-w)**

* cswch/s： 每秒任务主动(自愿的)切换上下文的次数，当某一任务处于阻塞等待状态，将主动让出自己的CPU资源。

* nvcswch/s: 每秒任务被动(不自愿的）切换上下文的次数，CPU分配给某一任务的时间片已经用完，因此将强迫该进程让出CPU的执行权。

<pre>
多处理器时我们可以添加-I参数，显示各个CPU的使用率。如: pidstat -w -I -G testp 1 10
</pre> 

## 4. 统计一个进程的线程数

* 使用top命令，具体用法是**top -H**。加上这个选项，top的每一行就不是现实一个进程，而是一个线程。
<pre>
# top -H

top - 23:59:52 up  3:52,  3 users,  load average: 0.00, 0.00, 0.00
Threads: 430 total,   1 running, 429 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.3 us,  0.7 sy,  0.0 ni, 99.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem :  2060812 total,  1014004 free,   387972 used,   658836 buff/cache
KiB Swap:  2094076 total,  2094076 free,        0 used.  1412768 avail Mem 

  PID USER      PR  NI    VIRT    RES    SHR S %CPU %MEM     TIME+ COMMAND                
 4610 root      20   0    8244   3524   2960 R  1.0  0.2   0:00.06 top                    
    1 root      20   0   23932   4900   3688 S  0.0  0.2   0:03.58 systemd                
    2 root      20   0       0      0      0 S  0.0  0.0   0:00.00 kthreadd               
    3 root      20   0       0      0      0 S  0.0  0.0   0:01.81 ksoftirqd/0            
    5 root       0 -20       0      0      0 S  0.0  0.0   0:00.00 kworker/0:0H
</pre>

此外，还可以通过```top -H -p <pid>```的方式查看某一个进程的线程信息。
{% highlight string %}
# top -H -p 11542
top - 10:40:01 up 302 days, 20:16,  1 user,  load average: 0.00, 0.04, 0.05
Threads: 11294 total,   0 running, 11294 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.2 us,  0.2 sy,  0.0 ni, 99.6 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem : 16449097+total, 81671952 free, 12521416 used, 70297600 buff/cache
KiB Swap:  4095996 total,  4095996 free,        0 used. 14957729+avail Mem 

  PID USER      PR  NI    VIRT    RES    SHR S %CPU %MEM     TIME+ COMMAND                                                                                                 
11543 root      20   0 86.671g 710276   7408 S  0.6  0.4  34:13.06 log                                                                                                     
21823 root      20   0 86.671g 710276   7408 S  0.3  0.4   0:23.52 safe_timer                                                                                              
24262 root      20   0 86.671g 710276   7408 S  0.3  0.4   5:48.86 ms_pipe_write                                                                                           
24330 root      20   0 86.671g 710276   7408 S  0.3  0.4   4:06.08 radosgw                                                                                                 
24340 root      20   0 86.671g 710276   7408 S  0.3  0.4   4:08.46 radosgw                                                                                                 
11542 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:00.67 radosgw                                                                                                 
11545 root      20   0 86.671g 710276   7408 S  0.0  0.4   1:16.85 service                                                                                                 
11546 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:00.00 admin_socket                                                                                            
11547 root      20   0 86.671g 710276   7408 S  0.0  0.4   7:29.43 radosgw                                                                                                 
11548 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:10.06 ms_dispatch                                                                                             
11549 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:00.00 ms_local                                                                                                
11550 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:05.58 ms_reaper                                                                                               
11551 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:24.16 safe_timer                                                                                              
11552 root      20   0 86.671g 710276   7408 S  0.0  0.4   0:00.00 fn_anonymous 
{% endhighlight %}

* 使用ps命令，具体用法是**ps -xH**。这样可以查看所有存在的线程，也可以使用grep作进一步的过滤
{% highlight string %}
# ps -xH
  PID TTY      STAT   TIME COMMAND
    1 ?        Ss     0:03 /sbin/init auto noprompt
    2 ?        S      0:00 [kthreadd]
    3 ?        S      0:01 [ksoftirqd/0]
    5 ?        S<     0:00 [kworker/0:0H]
    7 ?        S      0:00 [rcu_sched]
    8 ?        S      0:00 [rcu_bh]
    9 ?        S      0:00 [migration/0]
   10 ?        S<     0:00 [lru-add-drain]
   11 ?        S      0:00 [watchdog/0]
   12 ?        S      0:00 [cpuhp/0]
   13 ?        S      0:00 [kdevtmpfs]
  988 ?        SLsl   0:00 /usr/sbin/lightdm
  988 ?        SLsl   0:00 /usr/sbin/lightdm
  988 ?        SLsl   0:00 /usr/sbin/lightdm
{% endhighlight %}

* 使用ps命令，具体用法是**ps -mp pid**，这样可以看到指定进程产生的线程数目
<pre>
# ps -mp 988
  PID TTY          TIME CMD
  988 ?        00:00:00 lightdm
    - -        00:00:00 -
    - -        00:00:00 -
    - -        00:00:00 -
</pre>
其实也可以通过**ps -T -p pid**命令来查看某一个进程所创建的所有线程：
<pre>
# ps -T -p 988
  PID  SPID TTY          TIME CMD
  988   988 ?        00:00:00 lightdm
  988  1019 ?        00:00:00 gmain
  988  1021 ?        00:00:00 gdbus

# ps -T -p 17664 | grep ms_accepter
17664 32713 ?        00:02:54 ms_accepter
17664 32716 ?        00:00:03 ms_accepter
17664 32723 ?        00:00:00 ms_accepter
17664 32726 ?        00:00:00 ms_accepter
# strace -p 32726 -e trace=accept
</pre>
>注：上面17664就是我们所要查看的进程pid，而32713、32716、32723、32726为该进程的若干个线程


还可以通过```ps -o nlwp <pid>```命令来统计某一个进程所创建的线程数(这里,nlwp是number of light-weight process的缩写）：
<pre>
# ps -o nlwp 2270921
NLWP
130666
</pre>

* 使用proc文件系统，具体用法是```cat /proc/<pid>/status```
{% highlight string %}
# cat /proc/988/status
Name:   lightdm
Umask:  0022
State:  S (sleeping)
Tgid:   988
Ngid:   0
Pid:    988
PPid:   1
TracerPid:      0
Uid:    0       0       0       0
Gid:    0       0       0       0
FDSize: 256
Groups:
NStgid: 988
NSpid:  988
NSpgid: 988
NSsid:  988
VmPeak:    38540 kB
VmSize:    37600 kB
VmLck:        32 kB
VmPin:         0 kB
VmHWM:      7636 kB
VmRSS:      7636 kB
RssAnon:             708 kB
RssFile:            6928 kB
RssShmem:              0 kB
VmData:    25420 kB
VmStk:       136 kB
VmExe:       284 kB
VmLib:      9748 kB
VmPTE:        56 kB
VmPMD:        12 kB
VmSwap:        0 kB
HugetlbPages:          0 kB
Threads:        3
SigQ:   0/15793
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000000000
SigIgn: 0000000000001000
SigCgt: 0000000180014a03
CapInh: 0000000000000000
CapPrm: 0000003fffffffff
CapEff: 0000003fffffffff
CapBnd: 0000003fffffffff
CapAmb: 0000000000000000
Seccomp:        0
Cpus_allowed:   ff
Cpus_allowed_list:      0-7
Mems_allowed:   1
Mems_allowed_list:      0
voluntary_ctxt_switches:        136
nonvoluntary_ctxt_switches:     76
{% endhighlight %}


* 使用```pstree -p <pid>```命令也可以打印某一个进程的线程：
<pre>
# pstree -p 11542 | less
radosgw(11542)-+-{radosgw}(11543)
               |-{radosgw}(11545)
               |-{radosgw}(11546)
               |-{radosgw}(11547)
               |-{radosgw}(11548)
               |-{radosgw}(11549)
               |-{radosgw}(11550)
               |-{radosgw}(11551)
               |-{radosgw}(11552)
               |-{radosgw}(11557)
</pre>

<br />
<br />

**[参看]:**

1. [详解mpstat、iostat、sar、vmstat命令的使用](https://blog.csdn.net/qq_39591494/article/details/78418162)

2. [Linux性能监控工具sysstat系列：介绍与安装](https://blog.csdn.net/jinguangliu/article/details/47205347)

3. [Linux系统工具sar查看内存、CPU、IO](https://blog.csdn.net/ydyang1126/article/details/52794235)

4. [查看CPU性能参数（mpstat, iostat, sar、vmstat）等命令详解](https://blog.csdn.net/kangshuo2471781030/article/details/79319089)

5. [sysstat官网](http://sebastien.godard.pagesperso-orange.fr/)

6. [lsof 命令用法：查看已删除空间却没有释放的进程](https://blog.csdn.net/xyajia/article/details/80222825)

7. [Linux IO实时监控iostat命令详解](https://www.cnblogs.com/ggjucheng/archive/2013/01/13/2858810.html)

8. [Linux使用sar进行性能分析](https://blog.csdn.net/xusensen/article/details/54606401)

9. [pidstat（Linux 进程使用资源情况采样）](https://www.jianshu.com/p/348b6a81810d)

<br />
<br />
<br />


