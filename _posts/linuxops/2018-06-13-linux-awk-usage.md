---
layout: post
title: awk命令的使用
tags:
- LinuxOps
categories: linuxOps
description: awk命令的使用
---

本节主要介绍一下awk/gawk命令的使用。


<!-- more -->
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux sz-oss-01.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>


## 1. awk

向awk传递参数：
{% highlight string %}
for i in sda sdb sdc sdd sde sdf sdg sdh sdi sdj sdk; do path=`udevadm info -q path -n /dev/$i`; udevadm info -q env -p $path | grep ID_WWN= | awk 'BEGIN{FS="="} {print disk,"win-"$2}' disk=$i;done >> ./disk-id.txt
{% endhighlight %}







<br />
<br />

**[参看]**





<br />
<br />
<br />


