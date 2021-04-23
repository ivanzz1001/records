---
layout: post
title: linux系统中查看己设置iptables规则
tags:
- LinuxOps
categories: linuxOps
description: linux系统中查看己设置iptables规则
---

本章主要介绍一下Linux系统中如何查看已设置iptables规则。


<!-- more -->
## 1. iptables命令介绍
{% highlight string %}
# iptables --help
iptables v1.4.21

Usage: iptables -[ACD] chain rule-specification [options]
       iptables -I chain [rulenum] rule-specification [options]
       iptables -R chain rulenum rule-specification [options]
       iptables -D chain rulenum [options]
       iptables -[LS] [chain [rulenum]] [options]
       iptables -[FZ] [chain] [options]
       iptables -[NX] chain
       iptables -E old-chain-name new-chain-name
       iptables -P chain target [options]
       iptables -h (print this help information)
{% endhighlight %} 
### 1.1  iptables基本命令
<pre>
--append  -A chain                      添加一个规则到链的末尾
--check   -C chain                      检查某一条链是否存在
--delete  -D chain                      删除匹配的链
--delete  -D chain rulenum              删除指定链的某一条规则
--insert  -I chain [rulenum]            根据给出的规则序号向所选链中插入一条或更多规则。所以，如果规则序号为1，
                                        规则会被插入链的头部。这也是不指定规则序号时的默认方式。

--replace -R chain rulenum              修改指定链中的某一条规则
--list    -L [chain [rulenum]]          列出指定链中的规则
--list-rules -S [chain [rulenum]]       打印出指定链中的规则
--flush   -F [chain]                    删除指定链中的规则
--zero    -Z [chain [rulenum]]          把指定链，或者表中的所有链上的所有计数器清零
--new     -N chain                      创建一条用户自定义链
--delete-chain   -X [chain]             删除一条用户自定义链
--policy  -P chain target               该表某条链的策略
--rename-chain  -E old-chain new-chain  修改链的名称(只有用户自定义链的名称可以被修改）
</pre>

### 1.2 iptables选项参数
如下是iptables常用的一些选项参数：
<pre>
[!] --protocol  -p proto                规则或者包检查的协议。指定协议可以是tcp、udp、icmp中的一个或全部，也可以是数值，代表这些协议中的某一个。
                                        当然也可以使用在/etc/protocols中定义的协议名。在协议名前加'!'表示相反的规则。数字0相当于all。Protocol
                                        all会匹配所有协议，而且这是缺省的选项。在和check命令结合时，all可以不被使用

[!] --source    -s address[/mask][...]  指定源地址，可以是主机名、网络名或IP地址。mask说明可以是网络掩码或清楚的数字。标志--src是这个选项的简写。

[!] --destination -d address[/mask][...] 指定目标地址。标志--dst是这个选项的简写

--jump -j target                        执行指定的动作

--goto -g chain                         跳转到指定的链

--match  -m match                       扩展匹配

--numeric     -n                        以数字的形式显示IP地址和端口

[!] --in-interface -i input name[+]     匹配由指定网络接口进入的数据包
[!] --out-interface -o output name[+]   由指定接口发出的数据包

[!] --fragment  -f                      这意味着在分片的包中，规则只询问第二及以后的片
--exact       -x                        扩展数字。显示包和字节计数器的精确值，代替用K、M、G表示的约数。这个选项仅能用于-L选项

--line-numbers                          当列表显示规则时，在每个规则前面加上行号，与该规则在链中的位置相对应。
</pre>

如下是一些扩展选项，通常需要跟在```-m```选项的后面，以表示对其进行扩展：

1) **针对tcp的一些扩展选项**

当```--protocol tcp```被指定，且其他匹配的扩展未被指定时，这些扩展被装载。它提供以下选项：
<pre>
--source-port [!] [port[:port]]         源端口或端口范围指定，也可以使用服务名或端口号。如果使用端口范围，若首端口号忽略则默认为0，若尾端口号忽略则
                                        默认为65535。这个选项可以简写为--sport
--destionation-port [!] [port:[port]]   目标端口或端口范围指定。这个选项可以使用--dport别名来代替

--tcp-flags [!] mask comp               匹配指定的TCP标记。第一个参数是我们要检查的标记，一个用逗号分开的列表，第二个参数是用逗号分开的标记表,是必须
                                        被设置的。标记如下：SYN ACK FIN RST URG PSH ALL NONE。例如我们有如下这条命令：
                                                iptables -A FORWARD -p tcp --tcp-flags SYN,ACK,FIN,RST SYN
                                        上面这条命令只匹配那些SYN标志被设置而ACK、FIN和RST标记没有被设置的包

[!] --syn                               只匹配那些设置了SYN位而清除了ACK和FIN位的TCP包。这些包用于TCP连接初始化时发出请求。例如，大量的这种包进入一个
                                        接口发生堵塞时会阻止进入的TCP连接，而出去的TCP连接不会受到影响。这等于：--tcp-flags SYN,RST,ACK SYN

--tcp-option [!] number                 匹配设置了TCP选项的数据包
</pre>


2) **针对udp的一些扩展选项**

当```--protocol udp```被指定，且其他匹配的扩展未被指定时，这些扩展被装载。它提供以下选项：
<pre>
--source-port [!] [port:[port]]         源端口或端口范围指定  
--destionation-port [!] [port:[port]]   目标端口或端口范围指定
</pre>

3） **针对icmp的一些扩展选项**

当```protocol icmp```被指定,且其他匹配的扩展未被指定时,该扩展被装载。它提供以下选项：
<pre>
--icmp-type [!] typename               这个选项允许指定ICMP类型，可以是一个数值型的ICMP类型，或者是某个由命令iptables -p icmp -h所显示的ICMP类型名
</pre>

4) **针对mac的一些扩展选项**
<pre>
--mac-source [!] address               匹配物理地址。注意它只对来自以太设备并进入PREROUTING、FORWORD和INPUT链的包有效。
</pre>
例如，我们添加一条如下的iptables规则：
{% highlight string %}
# iptables -A PREROUTING -d 10.4.18.69/32 -p tcp -m tcp --dport 17443 -m mac ! --mac-source F8:98:EF:7E:9E:1B -j MARK --set-xmark 0x96be3/0xffffffff
{% endhighlight %}
上面为```-m tcp```匹配添加了```--dport```扩展选项； 另外为```-m mac```添加了```--mac-source```扩展选项。

上面设置的含义为： 对目标地址为10.4.18.69:17443的tcp数据包(且源mac地址不为```F8:98:EF:7E:9E:1B```），将其打上标记0x96be3。

对于```--set-xmark value/mask```，即Zero out the bits given by mask and XOR value into the ctmark。其操作结果为：
<pre>
ctmark = (ctmark AND NOT mask) XOR value
</pre>
```zero out```(ctmark AND NOT mask): 如果设置了mask中的某个位，则ctmark中相应位将为零（在XOR之前)。




5） **针对limit的扩展选项**
<pre>
--limit rate             最大平均匹配速率：可赋的值有'/second', '/minute', '/hour', or '/day'这样的单位，默认是3/hour

--limit-burst number     待匹配包初始个数的最大值:若前面指定的极限还没达到这个数值,则概数字加1.默认值为5
</pre>
例如我们添加一条如下的iptables规则：
{% highlight string %}
# iptables -R INPUT 1 -p tcp --dport 22 -m limit --limit 3/minute --limit-burst 8 -j LOG
{% endhighlight %}
上面为```-m limit```添加了两个扩展选项。

6） **针对multiport的扩展选项**

这个模块匹配一组源端口或目标端口,最多可以指定15个端口。只能和```-p tcp```或者```-p udp```连着使用。
<pre>
--source-port [port[, port]]          如果源端口是其中一个给定端口则匹配
--destination-port [port[, port]]     如果目标端口是其中一个给定端口则匹配
--port [port[, port]]                 若源端口和目的端口相等并与某个给定端口相等,则匹配
</pre>

7) **针对mark的扩展选项**

这个模块与netfilter过滤器标记字段匹配（就可以在下面设置为使用MARK标记）
<pre>
--mark value [/mask]                  匹配那些无符号标记值的包（如果指定mask，在比较之前会给掩码加上逻辑的标记）
</pre>

8) **针对owner的扩展选项**

此模块是为本地生成包匹配创建者的不同特征。只能用于OUTPUT链，而且即使这样一些包（如ICMP ping应答）还是有可能没有所有者，因此永远不会匹配
<pre>
--uid-owner userid                  如果给出有效的user id，那么匹配它的进程产生的包
--gid-owner groupid                 如果给出有效的group id，那么匹配它的进程产生的包
--sid-owner seessionid              根据给出的会话组匹配该进程产生的包
</pre>

9） **针对state的扩展选项**

此模块，当与连接跟踪结合使用时，允许访问包的连接跟踪状态
<pre>
--state state
</pre>
这里state是一个逗号分割的匹配连接状态列表。可能的状态是:```INVALID```表示包是未知连接，```ESTABLISHED```表示是双向传送的连接，```NEW```表示包为新的连接，否则是非双向传送的，而```RELATED```表示包由新连接开始，但是和一个已存在的连接在一起，如FTP数据传送，或者一个ICMP错误。

例如，我们可以通过如下方式丢弃非法链接：
{% highlight string %}
# iptables -A INPUT -m state –state INVALID -j DROP
# iptables -A OUTPUT -m state –state INVALID -j DROP
# iptables-A FORWARD -m state –state INVALID -j DROP
{% endhighlight %}

10) **针对unclean的扩展选项**

此模块没有可选项，不过它试着匹配那些奇怪的、不常见的包。处在实验中

11) **针对tos的扩展选项**

此模块匹配IP包首部的8位tos（服务类型）字段（也就是说，包含在优先位中）
<pre>
--tos tos
</pre>
这个参数可以是一个标准名称，（用iptables -m tos -h 察看该列表），或者数值。

### 1.3 targets介绍
iptables的```-j```选项后面对应的是要执行的target。其中有些target具有一些扩展选项，下面我们会一并介绍：

1） **ACCEPT**

表示接收匹配的数据包

2） **DROP**

表示丢弃匹配的数据包

3) **REJECT**

作为对匹配的包的响应，返回一个错误的包：其他情况下和DROP相同。

此目标只适用于INPUT、FORWARD和OUTPUT链，和调用这些链的用户自定义链。这几个选项控制返回的错误包的特性：
<pre>
--reject-with type     type可以是icmp-net-unreachable、icmp-host-unreachable、icmp-port-nreachable、icmp-proto-unreachable、
                       icmp-net-prohibited或者icmp-host-prohibited，该类型会返回相应的ICMP错误信息（默认是port-unreachable）

--echo-reply           它只能用于指定ICMP ping包的规则中，生成ping的回应

--tcp-reset            可以用于在INPUT链中,或自INPUT链调用的规则，只匹配TCP协议：将回应一个TCP RST包。
</pre>



4) **REDIRECT**

表示重定向匹配的数据包，只适用于nat表的PREROUTING和OUTPUT链，和只调用它们的用户自定义链。它修改包的目标IP地址来发送包到机器自身（本地生成的包被安置为地址127.0.0.1）。它包含一个选项：
<pre>
--to-ports [port-port]        指定使用的目的端口或端口范围：不指定的话，目标端口不会被修改。只能用于指定了-p tcp 或 -p udp的规则。
</pre>

5) **SNAT**

源地址转换，这个目标只适用于nat表的POSTROUTING链。它规定修改包的源地址（此连接以后所有的包都会被影响），停止对规则的检查。它包含选项：
<pre>
--to-source [-][:port-port]        可以指定一个单一的新的IP地址，一个IP地址范围，也可以附加一个端口范围（只能在指定-p tcp 或者-p udp的规则里）。
                                   如果未指定端口范围，源端口中512以下的（端口）会被安置为其他的512以下的端口；512到1024之间的端口会被安置为1024
                                   以下的，其他端口会被安置为1024或以上。如果可能，端口不会被修改。
</pre>
例如：
{% highlight string %}
iptables -t nat -A POSTROUTING -s 192.168.1.0/24 -j SNAT --to 202.106.18.8
{% endhighlight %}
将源IP地址为```192.168.1.0/24```的IP统一修改为```202.106.18.8```

6) **DNAT**

目标地址转换，这个目标只适用于nat表的PREROUTING链。它包含如下一些扩展选项：
<pre>
--to-destiontion [-][:port-port]   可以指定一个单一的新的IP地址，一个IP地址范围，也可以附加一个端口范围（只能在指定-p tcp 或者-p udp的规则里）。
                                   如果未指定端口范围，目标端口不会被修改。
</pre>
例如：
{% highlight string %}
# iptables -t nat -A PREROUTING -d 202.106.18.8 -j DNAT --to 192.168.1.8
{% endhighlight %}
将目标IP地址为```202.106.18.8```的IP统一修改为```192.168.1.8```。

7) **MASQUERAD**

只用于nat表的POSTROUTING链。只能用于动态获取IP（拨号）连接：如果你拥有静态IP地址，你要用SNAT。伪装相当于给包发出时所经过接口的IP地址设置一个映像，当接口关闭连接会终止。这是因为当下一次拨号时未必是相同的接口地址（以后所有建立的连接都将关闭）。它有一个选项：
<pre>
--to-ports [port-port]       指定使用的源端口范围，覆盖默认的SNAT源地址选择（见上面）。这个选项只适用于指定了-p tcp或者-p udp的规则
</pre>

8) **MARK**

用来设置包的netfilter标记值。只适用于mangle表。其有如下一些扩展选项：
<pre>
--set-mark mark
--set-xmark mark/mask
</pre>
例如，我们通过如下命令对进来的数据包设置标志：
{% highlight string %}
# iptables -t mangle -A PREROUTING -d 10.4.18.69/32 -p tcp -m tcp --dport 80 -m mac ! --mac-source F8:98:EF:7E:9E:1B -j MARK --set-xmark 0x2a8/0xffffffff
{% endhighlight %}



9） **LOG**

为匹配的包开启内核记录。当在规则中设置了这一选项后，linux内核会通过printk()打印一些关于全部匹配包的信息（诸如IP包头字段等）。其有如下一些扩展选项：
<pre>
--log-level level          记录级别（数字或参看 syslog.conf(5)）

--log-prefix prefix        在纪录信息前加上特定的前缀：最多14个字母长，用来和记录中其他信息区别

--log-tcp-sequence         记录TCP序列号。如果记录能被用户读取那么这将存在安全隐患

--log-tcp-options          记录来自TCP包头部的选项

--log-ip-options           记录来自IP包头部的选项
</pre>

10) **TOS**

用来设置IP包的首部八位tos。只能用于mangle表。其有如下扩展选项：
<pre>
--set-tos tos             你可以使用一个数值型的TOS 值，或者用'iptables -j TOS -h'来查看有效TOS名列表
</pre>

11) **MIRROR**

这是一个试验示范目标，可用于转换IP首部字段中的源地址和目标地址，再传送该包,并只适用于INPUT、FORWARD和OUTPUT链，以及只调用它们的用户自定义链

12) **DIAGNOSTICS**

诊断。不同的错误信息会打印成标准错误：退出代码0表示正确。类似于不对的或者滥用的命令行参数错误会返回错误代码2，其他错误返回代码为1

## 2. iptables的基本使用 
如下我们简单列出iptables命令的基本使用方法，以作参考。
### 2.1 查看iptables规则

1） **查看默认表中的规则**
<pre>
# iptables -L
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere             udp dpt:domain
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:domain
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootps
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:bootps

Chain FORWARD (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     all  --  anywhere             192.168.122.0/24     ctstate RELATED,ESTABLISHED
ACCEPT     all  --  192.168.122.0/24     anywhere            
ACCEPT     all  --  anywhere             anywhere            
REJECT     all  --  anywhere             anywhere             reject-with icmp-port-unreachable
REJECT     all  --  anywhere             anywhere             reject-with icmp-port-unreachable

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootpc
</pre>
默认情况下是查看```filter```表的iptables规则，包括所有的链。filter表包含*INPUT*、*OUTPUT*、*FORWARD*三个规则链

2) **查看指定表中的规则**

语法如下：
<pre>
# iptables [-t 表名] -L
</pre>
这里只查看指定表中的规则。表名一共有3个：```filter```、```nat```、```mangle```。如果没有指定表名，则默认为filter表

参看如下示例：
<pre>
# iptables -t nat -L
Chain PREROUTING (policy ACCEPT)
target     prot opt source               destination         

Chain INPUT (policy ACCEPT)
target     prot opt source               destination         

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         

Chain POSTROUTING (policy ACCEPT)
target     prot opt source               destination 
</pre>

3) **查看指定链的规则**

语法如下：
<pre>
# iptables [-t 表名] -L [链名]
</pre>

这里多了个链名，就是规则链的名称。iptables一共有5条链： INPUT、OUTPUT、FORWARD、PREROUTING、POSTROUTING，但注意并不是每个表都有所有这5条链的。

参看如下示例：
<pre>
# iptables -t filter  -L INPUT
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  anywhere             anywhere             udp dpt:domain
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:domain
ACCEPT     udp  --  anywhere             anywhere             udp dpt:bootps
ACCEPT     tcp  --  anywhere             anywhere             tcp dpt:bootps
</pre>

4) **以数字显示规则**

语法如下：
<pre>
# iptables -n -L
</pre>
如上命令将会以数字的方式显示```filter```表中的相应规则。如果没有```-n```，规则中可能会出现```anywhere```，有了```-n```，它会变成```0.0.0.0/0```。

注意，```-n```选项与```-L```选项不能合并写成```-Ln```。

5） **以更详细的方式查看iptables**

语法如下：
<pre>
# iptables -nv -L
</pre>

### 2.2 删除规则(-D选项)
我们可以通过```-D```选项删除一条规则。先执行如下命令添加一条规则：
<pre>
# iptables -A INPUT -s 192.168.1.5 -j DROP
# iptables -n -L INPUT
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:53
ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:53
ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:67
ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:67
DROP       all  --  192.168.1.5          0.0.0.0/0
</pre>
上面最后一条是我们新添加的那条规则。现在执行如下命令来删除：
<pre>
# iptables -D INPUT -s 192.168.1.5 -j DROP
# iptables -n -L INPUT
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         
ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:53
ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:53
ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:67
ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:67
</pre>
可以看到已经删除。但有时候要删除的规则太长，删除时要写一大串，既浪费时间又容易写错，这是我们可以使用```--line-number```找出该规则的行号，再通过行号删除规则：
<pre>
# iptables -A INPUT -s 192.168.1.5 -j DROP
# iptables -n -L INPUT --line-number
Chain INPUT (policy ACCEPT)
num  target     prot opt source               destination         
1    ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:53
2    ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:53
3    ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:67
4    ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:67
5    DROP       all  --  192.168.1.5          0.0.0.0/0 

# iptables -D INPUT 5
# iptables -n -L INPUT --line-number
Chain INPUT (policy ACCEPT)
num  target     prot opt source               destination         
1    ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:53
2    ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:53
3    ACCEPT     udp  --  0.0.0.0/0            0.0.0.0/0            udp dpt:67
4    ACCEPT     tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:67
</pre>

另外，还有一些方法删除规则链：
<pre>
# iptables -F      //清除预设表filter中的所有规则链的规则

# iptables -X      //清除预设表filter中使用者自定链中的规则

# iptables -Z      //清除预设表filter中的所有规则链的规则, 并规则链计数器置为0
</pre>
在执行```iptables -F```或```iptables -Z```时，应该要小心，假如某一条链的默认策略是```DROP```，那么执行前述命令可能导致我们再也远程连接不上服务器。所以在执行前我们可以查看一下filter表的默认策略：
<pre>
# iptables -L
Chain INPUT (policy ACCEPT)
target     prot opt source               destination         

Chain FORWARD (policy ACCEPT)
target     prot opt source               destination         

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         
</pre>
上面看到默认策略都是```ACCEPT```。如果默认策略是```DROP```，可以通过如下命令将其先设置为```ACCEPT```:
<pre>
# iptables -P INPUT ACCEPT
</pre>


### 2.3 添加规则(-A选项）

这里我们给出一些示例：

* 为WEB服务器开启80端口
<pre>
# iptables -A INPUT -p tcp --dport 80 -j ACCEPT
</pre>

* 允许icmp包通过，即允许ping

如果我们已经将```INPUT```链设置成了```DROP```，那么就需要设置如下以允许ping包的请求：
<pre>
# iptables -A INPUT -p icmp -j ACCEPT
</pre>

如果我们已经将```OUTPUT```链设置成了```DROP```，那么就需要设置如下以允许ping包的响应：
<pre>
# iptables -A OUTPUT -p icmp -j ACCEPT
</pre>

* 减少不安全的端口连接
<pre>
# iptables -A OUTPUT -p tcp --sport 31337 -j DROP
# iptables -A OUTPUT -p tcp --dport 31337 -j DROP
</pre>

### 2.4 保存iptables
执行如下命令将当前的iptables规则保存到*/etc/sysconfig/iptables*文件中：
{% highlight string %}
# iptables-save
# iptables-save > iptables.rule
{% endhighlight %}
会保存当前的防火墙规则设置，命令行下通过iptables配置的规则在下次重启后会失效，当然这也是为了防止错误的配置防火墙。默认读取和保存的配置文件地址为*/etc/sysconfig/iptables*。

### 2.5 修改iptables
先看当前规则：
<pre>
# iptables -nL --line-number

Chain INPUT (policy ACCEPT)
num  target     prot opt source               destination
1    DROP       all  --  192.168.1.1          0.0.0.0/0
2    DROP       all  --  192.168.1.2          0.0.0.0/0
3    DROP       all  --  192.168.1.5          0.0.0.0/0
</pre>
现在假如我们需要将第三条规则修改为```ACCEPT```，可以使用如下命令：
<pre>
# iptables -R INPUT 3 -j ACCEPT
# iptables -nL --line-number

Chain INPUT (policy ACCEPT)
num  target     prot opt source               destination
1    DROP       all  --  192.168.1.1          0.0.0.0/0
2    DROP       all  --  192.168.1.2          0.0.0.0/0
3    ACCEPT     all  --  0.0.0.0/0            0.0.0.0/0
</pre>

### 2.6 设置chain的默认策略
<pre>
# iptables -P INPUT DROP
# iptables -P FORWARD ACCEPT
# iptables -P OUTPUT ACCEPT
</pre>
将```INPUT```链默认处理策略设置为```DROP```，前提是已经存在一条可以访问22端口的规则。这里要说明的是，在添加这类拒绝访问的规则之前，一定要想好执行完，会不会把自己关在防火墙外面，不然就傻眼了。像下面这句。

## 3. iptables应用举例
### 3.1 端口转发
出于安全考虑，Linux系统默认是禁止数据包转发的。所谓转发即当主机拥有多于一块的网卡时，其中一块收到数据包，根据数据包的目的ip地址将数据包发往本机另一块网卡，该网卡根据路由表继续发送数据包。这通常是路由器所要实现的功能。

因此这里我们首先要开启端口转发功能，修改内核运行参数```ip_forward```，打开转发：
<pre>
# echo 1 > /proc/sys/net/ipv4/ip_forward   //此方法临时生效
或
# vi /ect/sysctl.conf                      //此方法永久生效
# sysctl -p
</pre>

1) **本机端口转发**
<pre>
# iptables -t nat -A PREROUTING -p tcp -m tcp --dport 80 -j REDIRECT --to-ports 8080
</pre>
根据```iptables```防火墙原理可知，实际上在数据包进入INPUT链之前，修改了目标地址（端口），于是不难理解在开放端口时需要设置的是放行8080端口，无需考虑80端口：
<pre>
# iptables -A INPUT -s 172.29.88.0/24 -p tcp -m state --state NEW -m tcp --dport 8080 -j ACCEPT
</pre>
此时外部访问http的80端口便可自动转到8080（浏览器地址栏不会变），而且又具有很高的性能。但是如果你通过服务器```本地主机```的*curl*或*firefox*浏览器访问*http://localhost:80*或*http://local-domain.com:80*都是不行的（假如你有这样的奇葩需求），这是因为本地数据包产生的目标地址不对，你需要额外添加这条```OUTPUT```规则：
<pre>
# iptables -t nat -A OUTPUT -p tcp --dport 80 -j REDIRECT --to-ports 8080
</pre>
下面的规则可以达到同样的效果：
<pre>
# iptables -t nat -A PREROUTING -p tcp -i eth0 -d $YOUR_HOST_IP --dport 80 -j DNAT --to $YOUR_HOST_IP:8080
# iptables -t nat -A OUTPUT -p tcp -d $YOUR_HOST_IP --dport 80 -j DNAT --to 127.0.0.1:8080
# iptables -t nat -A OUTPUT -p tcp -d 127.0.0.1  --dport 80 -j DNAT --to 127.0.0.1:8080
</pre>

2) **异机端口转发**

有些情况下企业内部网络隔离比较严格，但有一个跨网段访问的情况，此时只要转发用的中转服务器能够与另外的两个IP(服务器或PC)通讯就可以使用iptables实现转发。（端口转发的还有其他方法，请参考 linux服务器下各种端口转发技巧）

现在要实现的是所有访问*192.168.10.100:8000*的请求，转发到*172.29.88.56:80*上，在*192.168.10.100*主机上添加规则：
<pre>
# iptables -t nat -A PREROUTING -i eth0 -p tcp -d 192.168.10.100 --dport 8000 -j DNAT --to-destination 172.29.88.56:80
# iptables -t nat -A POSTROUTING -o eth0 -j SNAT --to-source 192.168.10.100
或者
# iptables -t nat -A PREROUTING -d 192.168.10.100 -p tcp --dport 8000 -j DNAT --to 172.29.88.56:80
# iptables -t nat -A POSTROUTING -d 172.29.88.56 -p tcp --dport 80 -j SNAT --to-source 192.168.10.100
</pre>

需要注意的是，如果你的```FORWARD```链默认为DROP，上面所有端口转发都必须建立在FORWARD链允许通行的情况下：
<pre>
# iptables -A FORWARD -d 172.29.88.56 -p tcp --dport 80 -j ACCEPT
# iptables -A FORWARD -s 172.29.88.56 -p tcp -j ACCEPT
</pre>

### 3.2 记录日志
为22端口的```INPUT```包增加日志功能，插在input的第一个规则前面，为避免日志信息塞满*/var/log/messages*，用```--limit```限制：
<pre>
# iptables -R INPUT 1 -p tcp --dport 22 -m limit --limit 3/minute --limit-burst 8 -j LOG
</pre>
编辑日志配置文件*/etc/rsyslog.conf*，添加*kern.=notice /var/log/iptables.log*，可以将日志记录到自定义文件中。修改完成之后，执行如下命令重启```rsyslog```:
<pre>
# service rsyslog restart #重启日志服务
</pre>

### 3.3 防止DDos攻击

SYN洪水是攻击者发送海量的SYN请求到目标服务器上的一种DoS攻击方法，下面的脚本用于预防轻量级的DoS攻击(脚本```ipt_tcp.sh```)：
{% highlight string %}
iptables -N syn-flood                    #(如果您的防火墙默认配置有“ :syn-flood - [0:0] ”则不许要该项，因为重复了)
iptables -A INPUT -p tcp --syn -j syn-flood   
iptables -I syn-flood -p tcp -m limit --limit 2/s --limit-burst 5 -j RETURN   
iptables -A syn-flood -j REJECT   

# 防止DOS太多连接进来,可以允许外网网卡每个IP最多15个初始连接,超过的丢弃
# 需要iptables v1.4.19以上版本：iptables -V 
iptables -A INPUT -p tcp --syn -i eth0 --dport 80 -m connlimit --connlimit-above 20 --connlimit-mask 24 -j DROP   

#用Iptables抵御DDOS (参数与上相同)   
iptables -A INPUT -p tcp --syn -m limit --limit 5/s --limit-burst 10 -j ACCEPT  
iptables -A FORWARD -p tcp --syn -m limit --limit 1/s -j ACCEPT 

iptables -A FORWARD -p icmp -m limit --limit 2/s --limit-burst 10 -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 0 -s ! 172.29.73.0/24 -j DROP
{% endhighlight %}

### 3.4 iptables设置端口转发
假设我们有如下网络拓扑：

![network-top](https://ivanzz1001.github.io/records/assets/img/linuxops/iptables_network_top.jpg)
<pre>
服务器A有两个网卡：
                  内网IP: 192.168.1.3
                  外网IP: 10.138.108.103
                  本地回环： 127.0.0.1

服务器B有一个网卡，8001端口提供服务：
                  内网IP: 192.168.1.1
</pre>
现在我们要使用户通过外网*10.138.108.103:8001*访问内网服务器*192.168.1.1:8001*

1) **思路**

![iptables-route-path](https://ivanzz1001.github.io/records/assets/img/linuxops/iptables_route_path.jpg)
如上图所示，端口转发走的是图下方的A路线，利用nat表中的```prerouting```做dnat，用```postrouting```作snat。

数据包流动分析如下：
<pre>
  时  期               操 作             源IP:PORT                     目标IP:PORT
--------------------------------------------------------------------------------------------------
packet in           用户访问             1.2.3.4:5                10.138.108.103:8001

prerouting          dnat                1.2.3.4:5                192.168.1.1:8001

routing decision   判断是否转发          1.2.3.4:5                192.168.1.1:8001

postrouting         snat               10.138.108.103:X          192.168.1.1:8001

packet out          转发包              10.138.108.103:X          192.168.1.1:8001
</pre>


2) **操作配置**

首先执行如下命令开启数据包转发：
<pre>
# echo 1 > /proc/sys/net/ipv4/ip_forward   //此方法临时生效
或
# vi /ect/sysctl.conf                      //此方法永久生效
# sysctl -p
</pre>
然后再执行如下脚本：
{% highlight string %}
#!/bin/bash
pro='tcp'

src_host1='192.168.1.3'
src_host2='10.138.108.103'
src_port=8001

Dst_Host='192.168.1.1'
Dst_Port=8001

# 清空规则
iptables -F
iptables -X
iptables -Z
iptables -t nat -F

# Destination network  address translate (dnat)

# 如图所示
iptables -t nat -A PREROUTING  -p $pro -d $src_host1  --dport $src_port -j DNAT --to $Dst_Host:$Dst_Port
iptables -t nat -A PREROUTING  -p $pro -d $src_host2  --dport $src_port -j DNAT --to $Dst_Host:$Dst_Port

# 转发链允许转发
iptables -A FORWARD -p $pro -d $Dst_Host --dport $Dst_Port -j ACCEPT

# 本地连接不经过prerouting，只经过output链，所以想要在服务器A通过本地ip访问服务器B需要在output 链增加dnat规则
iptables -t nat -A OUTPUT -p $pro -d $src_host1 --dport $src_port -j DNAT --to $Dst_Host:$Dst_Port
iptables -t nat -A OUTPUT -p $pro -d $src_host2 --dport $src_port -j DNAT --to $Dst_Host:$Dst_Port



# source network address translate (snat)
iptables -t nat -A POSTROUTING -p $pro -d $Dst_Host --dport $Dst_Port -j SNAT --to $src_host2

# 显示已有规则
iptables -t nat -L -n --line-number
{% endhighlight %}

3) **补充说明**

这里不知道大家有没有想过，到达```服务器B```的请求，其响应是怎么返回的呢？ 这是因为当我们在```PREROUTING```做DNAT时，系统会默认在```POSTROUTING```中设置反转项，以处理返回的响应消息， 所以```prerouting```与```postrouting```互为兄弟关系，同样```input```与```output```也互为兄弟关系。下面我们来看一下请求与响应的处理流程吧：
<pre>
数据流向     时  期               操 作             源IP:PORT                     目标IP:PORT
----------------------------------------------------------------------------------------------------
  请求     packet in             用户访问           1.2.3.4:5                   10.138.108.103:8001
  请求     prerouting            dnat              1.2.3.4:5                   192.168.1.1:8001        (1)
  请求     routing decision      判断是否转发        1.2.3.4:5                  192.168.1.1:8001
  请求     postrouting           snat              10.138.108.103:X            192.168.1.1:8001        (2)
  请求     packet out            转发包             10.138.108.103:X            192.168.1.1:8001

  响应     packet in             服务器响应         192.168.1.1:8001            10.138.108.103:X
  响应     prerouting            dnat              192.168.1.1:8001            1.2.3.4:5               (3)   
  响应     routing decision      判断是否转发       192.168.1.1:8001             1.2.3.4:5 
  响应     postrouting           snat              10.138.108.103:8001         1.2.3.4:5               (4)
  响应     packet out            转发包             10.138.108.103:8001         1.2.3.4:5 
</pre>
从上面我们看到,```(1)、(4)```互为兄弟反转关系， ```(2)、(3)```也互为兄弟反转关系。当我们在设置```(1)```的时候，系统默认会为我们添加```(4)```； 当我们设置```(2)```的时候，系统默认会为我们添加```(3)```。


<br />
<br />

**[参看]**

1. [linux系统中查看己设置iptables规则](https://www.cnblogs.com/zhaogaojian/p/8186220.html)

2. [iptables](https://blog.csdn.net/qq_42743215/article/details/81637090)

3. [iptables常用实例备查](http://seanlook.com/2014/02/26/iptables-example/)

4. [iptables 设置端口转发/映射](https://blog.csdn.net/light_jiang2016/article/details/79029661)

5. [Linux的iptables常用配置范例](http://www.ha97.com/3928.html)

6. [为啥使用ＳＮＡＴ设置了数据包的源地址之后，使用抓包工具没抓到源地址](https://www.cnblogs.com/honpey/p/9066494.html)

7. [关于--set-mark的一些问题](https://www.it1352.com/1516340.html)

<br />
<br />
<br />


