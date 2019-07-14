---
layout: post
title: Linux中的软链接与硬链接
tags:
- LinuxOps
categories: linux
description: Linux中的软链接与硬链接
---

本文我们讲述一下Linux文件系统中的软链接与硬链接。



<!-- more -->

## 1. Linux的文件与目录
现代操作系统为解决信息能独立于进程之外被长期存储引入了文件，文件作为进程创建信息的逻辑单元可被多个进程并发使用。在Unix系统中，操作系统为磁盘上的文本与图像、鼠标与键盘等输入设备以及网络交互等IO操作设计了一组通用API，使它们被处理时均可统一使用字节流方式。换言之，Unix系统中除进程之外的一切皆是文件，而Linux保持了这一特性。为了便于文件的管理，Linux还引入了目录（有时亦被称为文件夹）这一概念。目录使文件可被分类管理，且目录的引入使Linux的文件系统形成一个层级结构的目录树。```清单1```所示的是普通Linux系统的顶层目录结构，其中```/dev```是存放了设备相关文件的目录。

```清单1``` Linux系统的顶层目录结构
<pre>
/              根目录
├── bin     存放用户二进制文件
├── boot    存放内核引导配置文件
├── dev     存放设备文件
├── etc     存放系统配置文件
├── home    用户主目录
├── lib     动态共享库
├── lost+found  文件系统恢复时的恢复文件
├── media   可卸载存储介质挂载点
├── mnt     文件系统临时挂载点
├── opt     附加的应用程序包
├── proc    系统内存的映射目录，提供内核与进程信息
├── root    root 用户主目录
├── sbin    存放系统二进制文件
├── srv     存放服务相关数据
├── sys     sys 虚拟文件系统挂载点
├── tmp     存放临时文件
├── usr     存放用户应用程序
└── var     存放邮件、系统日志等变化文件
</pre>

Linux与其他类Unix系统一样并不区分文件与目录：目录是记录了其他文件名的文件。使用命令```mkdir```创建目录时，若期望创建的目录的名称与现有的文件名（或目录名）重复，则会创建失败：
<pre>
# ls -F /usr/bin/zi* 
/usr/bin/zip*       /usr/bin/zipgrep*  /usr/bin/zipnote* 
/usr/bin/zipcloak*  /usr/bin/zipinfo*  /usr/bin/zipsplit* 
 
# mkdir -p /usr/bin/zip 
mkdir: cannot create directory `/usr/bin/zip': File exists
</pre>

Linux将设备当做文件进行处理，```清单2```展示了如何打开设备文件*/dev/input/event5*并读取文件内容。文件```event5```表示一种输入设备，其可能是鼠标或键盘等。查看文件*/proc/bus/input/devices*可知event5对应的设备类型。设备文件*/dev/input/event5*使用read()以字节流的方式被读取。结构体```input_event```被定义在内核头文件```linux/input.h```中：

```清单2``` 打开并读取设备文件
{% highlight string %}
int fd; 
struct input_event ie; 
fd = open("/dev/input/event5", O_RDONLY); 
read(fd, &ie, sizeof(struct input_event)); 
printf("type = %d  code = %d  value = %d\n", 
            ie.type, ie.code, ie.value); 
close(fd);
{% endhighlight %}


## 2. 硬链接与软连接的联系与区别
我们知道文件都有文件名与数据，这在Linux上被分成两个部分：用户数据(user data)与元数据(metadata)。用户数据,即文件数据块(data block)，数据块是记录文件真实内容的地方； 而元数据则是文件的附加属性，如文件大小、创建时间、所有者等信息。在Linux中，元数据中的```inode```号（inode是文件元数据的一部分，但其并不包含文件名，inode号即索引节点号）才是文件的唯一标识而非文件名。文件名仅是为了方便人们的记忆和使用，系统或程序通过```inode```号寻找正确的文件数据块。下图展示了程序通过文件名获取文件内容的过程：







<br />
<br />
**[参看]:**

1. [理解 Linux 的硬链接与软链接](https://www.ibm.com/developerworks/cn/linux/l-cn-hardandsymb-links/index.html)

2. [硬链接和软链接的区别](https://zhidao.baidu.com/question/1797277860057284227.html)


<br />
<br />
<br />





