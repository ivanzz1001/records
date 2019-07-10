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
1) **iptables基本命令**
<pre>
--append  -A chain                 添加一个规则到链的末尾
--check   -C chain                 检查某一条链是否存在
--delete  -D chain                 删除匹配的链
--delete  -D chain rulenum         删除指定链的某一条规则
--insert  -I chain [rulenum]       插入一条规则到指定的链
--replace -R chain rulenum         修改指定链中的某一条规则
--list    -L [chain [rulenum]]     列出指定链中的规则
--list-rules -S [chain [rulenum]]  打印出指定链中的规则
--flush   -F [chain]               删除指定链中的规则
--zero    -Z [chain [rulenum]]     将指定链中的规则计数器置为0
--new     -N chain                 创建一条用户自定义链
--delete-chain   -X [chain]        删除一条用户自定义链
--policy  -P chain target          该表某条链的策略
--rename-chain  -E old-chain new-chain  修改链的名称
</pre>

2) **iptables选项参数**
<pre>
</pre>

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
<pre>
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
<pre>
# iptables-save
</pre>
这样即使操作系统重启之后，我们仍可以恢复原来设置的规则。

## 2.5 修改iptables
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


<br />
<br />

**[参看]**

1. [linux系统中查看己设置iptables规则](https://www.cnblogs.com/zhaogaojian/p/8186220.html)

<br />
<br />
<br />


