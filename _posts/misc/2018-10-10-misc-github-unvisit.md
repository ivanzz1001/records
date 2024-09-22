---
layout: post
title: 访问raw.githubusercontent.com失败问题处理
tags:
- misc
categories: misc
description: githubusercontent
---


本文记录一下通过raw.githubusercontent.com无法正常显示图片的方法。



<!-- more -->

## 1. githubusercontent无法正常显示图片

GitHub上的项目的有些资源是放在 raw.githubusercontent.com 上的，通常我们在安装某些软件的时候会从该地址下载资源，直接访问的话经常容易失败。这篇文章会介绍两种处理方法。

### 1.1 方法1：修改Hosts文件

无法访问某些域名的情况可能有多种原因，比如网站本身的问题、不可描述的原因、网络运营商的问题、DNS相关的问题等待。如果是网站本身的问题那什么办法都没有；如果是网络运营商的问题那就投诉或者换运营商；如果是DNS相关的问题那可以通过修改hosts文件或是修改自身DNS服务器来解决。目前阶段 raw.githubusercontent.com 访问异常主要是因为DNS相关问题，可以通过修改hosts来处理。

hosts文件中有用的内容都以一行一行的形式排布，格式为```ip地址 + 空格 + 域名``` ，比如:
<pre>
140.82.114.4 github.com 
</pre>
这样设备在访问 github.com 时就会直接访问 140.82.114.4 这个地址了，而不用向DNS服务器查询。所以这里关键的是要知道域名真实的IP地址，这个可以通过下面网站查询：

- [https://www.ipaddress.com/](https://www.ipaddress.com/)

- [http://whoissoft.com/](http://whoissoft.com/)

- [http://tool.chinaz.com/dns](http://tool.chinaz.com/dns)

上面已经查询到了 raw.githubusercontent.com 的真实IP，接下来就可以向hosts文件添加信息了：

- windows下hosts文件在```C:\Windows\System32\drivers\etc``` 目录下，可以手动打开该文件添加条目；


- linux下hosts文件在```/etc/```目录下，修改hosts文件需要root权限，可以在shell中使用如下语句来添加
<pre>
# echo "185.199.108.133 raw.githubusercontent.com" 
</pre>


### 1.2 方法2：替换githubusercontent

直接把http://raw.githubusercontent.com换成raw.gitmirror(raw.githubusercontent.com的镜像网站)即可(后面的不用变) 


### 1.3 总结

总的来说修改hosts文件来解决一些网站因为DNS原因无法访问或者访问缓慢的情况还是非常方便的。这个方法也可以用于改善 github.com 、github.global.ssl.fastly.net 等网站的访问情况。



<br />
<br />


**[参考]**

1. [访问raw.githubusercontent.com失败问题处理](https://blog.csdn.net/Naisu_kun/article/details/118957940)

2. [解决https://raw.githubusercontent.com/无法访问](https://blog.csdn.net/m0_46669450/article/details/139115545)

<br />
<br />
<br />

