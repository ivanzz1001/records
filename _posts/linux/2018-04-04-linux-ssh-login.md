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

## 2. 准备工作

### 2.1 建立主机名与IP地址对应
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
# hostname machine-AB      //临时修改，不需要重启即可生效
# hostname -s
machine-B
# hostnamectl --static --transient --pretty set-hostname machine-B    //永久修改，需要重启机器生效


# cat /etc/hosts
#127.0.0.1   localhost localhost.localdomain localhost4 localhost4.localdomain4

#::1         localhost localhost.localdomain localhost6 localhost6.localdomain6

10.133.146.59 machine-A

10.133.146.250 machine-B
</pre>

### 2.2 配置ssh

1) **修改sshd_config配置**

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

2） **重启ssh**

执行如下命令重启```机器B```上的ssh服务，并设置为开机启动:
<pre>
# systemctl restart sshd
# systemctl enable sshd
</pre>

## 3. 生成公私钥
这里因为我们只需要```机器A```免密码登录，因此我们需要在```机器A```上生成公私钥文件(如下在机器A上操作）：

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
id_rsa  id_rsa.pub
{% endhighlight %}

上面按三次回车之后，该目录下将会产生id_rsa、id_rsa.pub文件.

3) **生成授权文件**

在上面```机器A上```生成的```id_rsa.pub```其实是一个文本文件，我们可以直接将其导出到```authorized_keys```文件中。
<pre>
# cat id_rsa.pub >> authorized_keys
</pre>

4) **分发授权文件**

将```机器A```上生成的```authorized_keys```文件分发到要免密码登录的```机器B```上。这里我们可以拷贝过去，然后追加到```机器B```上的```authorized_keys```即可。

其实我们也可以跳过```第3)步```，直接在```机器A```上执行如下来分发生成的授权公钥信息：
<pre>
# ssh-copy -i ~/.ssh/id_rsa.pub root@machine-B 
</pre>


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






<br />
<br />
**[参看]:**

1. [centos7配置SSH免密码登录](https://blog.csdn.net/u014180504/article/details/83579192)

2. [集群环境ssh免密码登录设置](https://www.cnblogs.com/ivan0626/p/4144277.html)

3. [linux设置ssh免密登录和ssh-copy-id命令](https://blog.csdn.net/hochoy/article/details/80749309)



<br />
<br />
<br />





