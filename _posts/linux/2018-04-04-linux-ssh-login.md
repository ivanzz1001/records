---
layout: post
title: 集群环境ssh免密码登录设置
tags:
- LinuxOps
categories: linux
description: 集群环境ssh免密码登录设置
---


本章我们介绍一下如何在集群环境中设置ssh免密码登录。假设我们有如下环境：
<pre>
machine-A         10.133.146.59        Centos7.3
machine-B         10.133.146.250       Centos7.3 
</pre>


<!-- more -->

## 1. ssh免密码登录配置和ssh原理
在网上看到很多关于如何配置ssh免密码登录的文章，但很多都有一些问题。如下我们给出相应的原理图：

![ssh-login](https://ivanzz1001.github.io/records/assets/img/linux/linux_ssh_login.png)

如下我们以```A机器```免密码登录到```B机器```上为例进行讲解，此时```A机器```就是客户端，```B机器```就是服务器端。

>注：这里我们讲述的是从A机器的root账户登录到B机器的root账户。如果要从A机器的www账户登录到B机器的guest账户，则首先需要在主机A上创建账户www，在B机器上创建guest账户，之后相应的很多操作都要切换到对应的账户下执行。

## 2. 准备工作

### 2.1 关闭iptables及SELinux
在```machine-A```及```machine-B```主机上的root账户下执行如下命令关闭防火墙：
{% highlight string %}
# systemctl stop firewalld.service 
# systemctl disable firewalld.service 
# setenforce 0 
# sed -i 's/SELINUX=enforcing/SELINUX=disabled/' /etc/selinux/config 
{% endhighlight %}



### 2.2 建立主机名与IP地址对应
* 在```机器A```上执行如下命令修改主机名
<pre>
# hostname machine-A	      //临时修改，不需要重启即可生效
# hostname -s
machine-A
# hostnamectl --static --transient --pretty set-hostname machine-A    //永久修改，需要重启机器生效


# cat /etc/hosts
#127.0.0.1   localhost localhost.localdomain localhost4 localhost4.localdomain4

#::1         localhost localhost.localdomain localhost6 localhost6.localdomain6

10.133.146.59 machine-A

10.133.146.250 machine-B
</pre>


* 在```机器B```上执行如下命令修改主机名
<pre>
# hostname machine-B      //临时修改，不需要重启即可生效
# hostname -s
machine-B
# hostnamectl --static --transient --pretty set-hostname machine-B    //永久修改，需要重启机器生效


# cat /etc/hosts
#127.0.0.1   localhost localhost.localdomain localhost4 localhost4.localdomain4

#::1         localhost localhost.localdomain localhost6 localhost6.localdomain6

10.133.146.59 machine-A

10.133.146.250 machine-B
</pre>


### 2.3 配置ssh

1) **安装sshd服务**

在```machine-A```及```machine-B```上执行如下命令安装sshd服务：
<pre>
# yum install openssh-server
# ps -ef | grep ssh
</pre>

2) **修改sshd_config配置**

这里我们要从```机器A```免密码登录到```机器B```，这通常需要我们修改```机器B```上的ssh配置：
{% highlight string %}
# vi /etc/ssh/sshd_config 

#禁用root账户登录，如果是用root用户登录请开启
PermitRootLogin yes

# 是否让 sshd 去检查用户家目录或相关档案的权限数据，
# 这是为了担心使用者将某些重要档案的权限设错，可能会导致一些问题所致。
# 例如使用者的 ~.ssh/ 权限设错时，某些特殊情况下会不许用户登入
StrictModes no

# 是否允许用户自行使用成对的密钥系统进行登入行为，仅针对 version 2。
# 至于自制的公钥数据就放置于用户家目录下的 .ssh/authorized_keys 内
RSAAuthentication yes
PubkeyAuthentication yes
AuthorizedKeysFile      .ssh/authorized_keys

# 有了证书登录了，就禁用密码登录吧，安全要紧
PasswordAuthentication no
{% endhighlight %}

3） **重启ssh**

执行如下命令重启```机器B```上的ssh服务，并设置为开机启动:
<pre>
# systemctl restart sshd
# systemctl enable sshd
</pre>




## 3. 生成公私钥
这里因为我们只需要```机器A```免密码登录，因此我们需要在```机器A```上生成公私钥文件(如下在机器A上操作）：

>注：由于我们是采用机器A的root账户免密登录到机器B的root账户，对于机器A来说我们需要在root账户的Home目录下执行如下操作。

1) **清除旧的公私钥文件**
<pre>
# cd ~/.ssh/
# rm -rf id_rsa id_rsa.pub
</pre>

2) **产生新的公私钥文件**
{% highlight string %}
# ls -al ~/.ssh
ls: cannot access /root/.ssh: No such file or directory
# mkdir ~/.ssh
# cd ~/.ssh

# ssh-keygen --help
# ssh-keygen -t rsa
Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa): 
Enter passphrase (empty for no passphrase): 
Enter same passphrase again: 
Your identification has been saved in /root/.ssh/id_rsa.
Your public key has been saved in /root/.ssh/id_rsa.pub.
The key fingerprint is:
0b:aa:9e:7a:5f:7d:b5:e9:35:49:27:f4:d6:ba:83:73 root@machine-A
The key's randomart image is:
+--[ RSA 2048]----+
|                 |
|                 |
|              .  |
|             . ..|
|      . S   . o =|
|     . o . . + * |
|    . . o . o.=  |
|  .o .   . .o.Eo |
|.++..       .o.. |
+-----------------+
# ls
authorized_keys  id_rsa  id_rsa.pub
{% endhighlight %}

上面按三次回车之后，该目录下将会产生id_rsa、id_rsa.pub文件:

* authorized_keys: 存放远程免密登录的公钥,主要通过这个文件记录多台机器的公钥。

* id_rsa: 生成的私钥文件

* id_rsa.pub: 生成的公钥文件

* known_hosts: 已知的主机公钥清单


3) **分发公钥**

这里我们需要将```机器A```上生成的公钥id_rsa.pub分发给```机器B```。有三种方法，这里我们建档介绍一下：

* 方法1

使用```ssh-copy-id命令```将主机A上的id_rsa.pub拷贝到主机B：
<pre>
# ssh-copy-id -i ~/.ssh/id_rsa.pub root@machine-B 
</pre>

* 方法2

在机器A上生成的```id_rsa.pub```其实是一个文本文件，我们可以直接通过```scp```命令将其拷贝到主机B，然后将其追加到```主机B```的```authorized_keys```文件中。

在主机A上执行如下命令：
<pre>
# scp /root/.ssh/id_rsa.pub root@machine-B:/root/.ssh/machine-a-id_rsa.pub
</pre>

在主机B上执行：
{% highlight string %}
# cat /root/.ssh/machine-a-id_rsa.pub
# cat /root/.ssh/machine-a-id_rsa.pub >> authorized_keys
{% endhighlight %}

* 方式3

采用纯手工复制。将本地id_rsa.pub文件(这里为```主机A```上的id_rsa.pub文件)的内容拷贝至远程服务器的```~/.ssh/authorized_keys```文件中也完全可以的。先使用cat命令查看当前的公钥，然后复制，在到目标服务器上去粘贴。



## 4. 调整目录及文件访问权限
在```机器A```上执行如下命令调整目录的访问权限：
<pre>
# chmod 0700 ~/.ssh/
</pre>

在```机器B```上执行如下命令调整目录及文件的访问权限：
<pre>
# chmod 0700 ~/.ssh/
# chmod 0600 ~/.ssh/authorized_keys
</pre>

## 5. 测试验证
在```机器A```上执行如下命令登录```机器B```:
<pre>
# ssh root@machine-B
</pre>
自此，免密码登录已经设定完成。注意第一次ssh登录时需要输入密码，再次访问时即可免密码登录。


## 6. 附录
用OpenSSH的人都知ssh会把你每个你访问过计算机的公钥(public key)都记录在```~/.ssh/known_hosts```。当下次访问相同计算机时，OpenSSH会核对公钥，如果公钥不同，OpenSSH会发出警告。假设我们重新安装系统，其公钥信息还在，连接会出现如下所示：
{% highlight string %}
# ssh root@192.169.0.100
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!
Someone could be eavesdropping on you right now (man-in-the-middle attack)!
It is also possible that a host key has just been changed.
The fingerprint for the ECDSA key sent by the remote host is
SHA256:xv9d+LHzS3VcVK4PVwMcmzoGRQ5ZPqvDGmdJ0CjCB9o.
Please contact your system administrator.
Add correct host key in /Users/faker/.ssh/known_hosts to get rid of this message.
Offending ECDSA key in /Users/faker/.ssh/known_hosts:2
ECDSA host key for 192.169.0.100 has changed and you have requested strict checking.
Host key verification failed.
{% endhighlight %}

问题处理：

1） **方法1**
<pre>
# rm -rf ~/.ssh/known_hosts
</pre>

优点：干净利索

缺点：把其他正确的公钥信息也删除，下次链接要全部重新经过认证

2) **方法2**
<pre>
# vi ~/.ssh/known_hosts
</pre>

删除对应ip的相关rsa信息.

优点：其他正确的公钥信息保留

缺点：还要```vi```，还要找到对应信息，稍微优点繁琐

3) **方法3**

清除旧的公钥信息：
<pre>
# ssh-keygen -R 192.168.0.100
</pre>

优点：快、稳、狠

缺点：没有缺点


<br />
<br />
**[参看]:**

1. [centos7配置SSH免密码登录](https://blog.csdn.net/u014180504/article/details/83579192)

2. [集群环境ssh免密码登录设置](https://www.cnblogs.com/ivan0626/p/4144277.html)

3. [linux设置ssh免密登录和ssh-copy-id命令](https://blog.csdn.net/hochoy/article/details/80749309)

4. [SSH配置—Linux下实现免密码登录](https://www.cnblogs.com/hanwen1014/p/9048717.html)

5. [Linux初窥：Linux下SSH免密码登录配置](https://blog.csdn.net/pengjunlee/article/details/80919833)

6. [Linux使用ssh公钥实现免密码登录另外一台Linux](http://php-note.com/article/1464.html)

<br />
<br />
<br />





