---
layout: post
title: Linux下相关硬盘工具的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux下相关硬盘工具的使用
---

本文我们主要会介绍```sgdisk```、```parted```等工具的使用。


<!-- more -->


## 1. sgdisk工具的使用

<pre>
# sgdisk -Zog /dev/sdi
</pre>


## 2. parted工具的使用
{% highlight string %}
# parted /dev/sdi
GNU Parted 3.1
Using /dev/sdi
Welcome to GNU Parted! Type 'help' to view a list of commands.
(parted) p
Model: SEAGATE ST8000NM0075 (scsi)
Disk /dev/sdi: 8002GB
Sector size (logical/physical): 512B/4096B
Partition Table: gpt
Disk Flags: 

Number  Start  End  Size  File system  Name  Flags

(parted) mkpart                                                           
Partition name?  []? 1                                                    
File system type?  [ext2]? xfs                                            
Start? 0%
End? 10Gib                                                                
(parted) p                                                                
Model: SEAGATE ST8000NM0075 (scsi)
Disk /dev/sdi: 8002GB
Sector size (logical/physical): 512B/4096B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name  Flags
 1      1049kB  10.7GB  10.7GB               1

(parted) mkpart                                                           
Partition name?  []? 2                                                    
File system type?  [ext2]? xfs                                            
Start? 10GiB                                                              
End? 100%                                                                 
(parted) p
Model: SEAGATE ST8000NM0075 (scsi)
Disk /dev/sdi: 8002GB
Sector size (logical/physical): 512B/4096B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name  Flags
 1      1049kB  10.7GB  10.7GB               1
 2      10.7GB  8002GB  7991GB               2

(parted) q                                                                
Information: You may need to update /etc/fstab.
{% endhighlight %}

分区完成之后，执行如下命令以使操作系统了解到相应的分区信息：
<pre>
# partprobe                                   
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
</pre>


<br />
<br />

**[参看]**

1. [Linux下的parted工具的使用 GPT分区安装系统](http://blog.51cto.com/tlinux/1739407)

2. [parted分区和挂载及非交互式操作](https://www.cnblogs.com/kaishirenshi/p/7850247.html)

3. [GPT和parted命令详解(原创)](http://czmmiao.iteye.com/blog/1751408)

4. [在 Linux 上检测硬盘上的坏道和坏块](https://linux.cn/article-7961-1.html)

5. [parted分区工具用法](https://www.cnblogs.com/yinzhengjie/p/6844372.html)

<br />
<br />
<br />


