---
layout: post
title: Linux环境下NVIDIA驱动的相关操作
tags:
- LinuxOps
categories: linuxOps
description: Linux环境下NVIDIA驱动的相关操作
---


本章我们首先介绍一下NVIDIA，然后再介绍一下显卡驱动的安装、查看显卡信息等操作。

<!-- more -->



## 1. NVIDIA介绍

NVIDIA是(NVIDIA Corporation, NASDAQ: NVDA，官方英文名称英伟达），是一家以设计智核```芯片组```为主的无晶圆(Fabless)IC半导体公司，公司创立于1993年1月，总部位于美国加利福尼亚州圣克拉拉市。

### 1.1 经营内容

NVIDIA公司是全球可编程图形处理技术领袖。与ATI（后被AMD收购）齐名，专注于打造能够增强个人和专业计算平台的人机交互体验的产品。公司的图形和通信处理器拥有广泛的市场，已被多种多样的计算平台采用，包括个人数字媒体PC、商用PC、专业工作站、数字内容创建系统、笔记本电脑、军用导航系统和视频游戏控制台等。NVIDIA产品和技术的基础是NVIDIA ForceWare，这是一种综合性软件套件，能够实现业内领先的图形、音频、通信、存储和安全功能。NVIDIA ForceWare可以提高采用NVIDIA GeForce图形芯片和NVIDIA nForce平台解决方案的各类台式和移动PC的工作效率、稳定性和功能。

NVIDIA公司专门打造面向计算机、消费电子和移动终端，能够改变整个行业的创新产品。这些产品家族正在改变视觉丰富和运算密集型应用例如视频游戏、电影产业、广播、工业设计、财政模型、空间探索以及医疗成像。

此外， NVIDIA致力于研发和提供引领行业潮流的先进技术，包括NVIDIA SLI技术----能够灵活地大幅提升系统性能的革命性技术和NVIDIA PureVideo高清视频技术。

NVIDIA已经开发出了5大产品系列，以满足特定细分市场需求，包括: GeForce、Tegra、ION、Quadro、Tesla。公司不断为视觉计算树立全新标准，其令人叹为观止的交互式图形产品可广泛应用于从平板电脑和便携式媒体播放器到笔记本与工作站等各种设备之上。


### 1.2 产品品牌

* **NVIDIA TNT**

* **NVIDIA GEFORCE**: 为图形和视频所设计的GPU。配有NVIDIA Geforce系列GPU的台式电脑和笔记本电脑带给用户无法比拟的性能，明快的照片，高清晰的视频回放和超真实效果的游戏。GeForce系列的笔记本GPU还包括先进的耗电管理技术，这种技术可以在不过分耗费电池的前提下保证高性能。

* **NVIDIA NFORCE**： 世界上最先进的核心逻辑解决方案

* **NVIDIA TEGRA**: 世界首款移动超级芯片


## 2. 与显卡相关的操作命令

**1) 查看GPU相关信息**

如果安装的是NVIDIA GPU，有一个很好用的命令：
{% highlight string %}
//显示NVIDIA GPU的相关信息（nvidia-smi即nvidia system management interface)


# nvidia-smi
Tue Jan 23 16:03:24 2018       
+-----------------------------------------------------------------------------+
| NVIDIA-SMI 384.111                Driver Version: 384.111                   |
|-------------------------------+----------------------+----------------------+
| GPU  Name        Persistence-M| Bus-Id        Disp.A | Volatile Uncorr. ECC |
| Fan  Temp  Perf  Pwr:Usage/Cap|         Memory-Usage | GPU-Util  Compute M. |
|===============================+======================+======================|
|   0  GeForce GTX 108...  Off  | 00000000:02:00.0  On |                  N/A |
| 21%   32C    P8    10W / 250W |   3915MiB / 11164MiB |      0%      Default |
+-------------------------------+----------------------+----------------------+
|   1  GeForce GTX 108...  Off  | 00000000:03:00.0 Off |                  N/A |
| 21%   34C    P8    10W / 250W |      2MiB / 11172MiB |      0%      Default |
+-------------------------------+----------------------+----------------------+
                                                                               
+-----------------------------------------------------------------------------+
| Processes:                                                       GPU Memory |
|  GPU       PID   Type   Process name                             Usage      |
|=============================================================================|
|    0      1512      G   /usr/lib/xorg/Xorg                           142MiB |
|    0      2588      C   ./szy.frs.face_set                          3760MiB |
+-----------------------------------------------------------------------------+

//说明：
//1) Fan: 表示风扇转速，从0到100%之间变动，这个速度是计算机期望的风扇转速。实际情况下可能达不到。另外有的设备并不返回转速，因为
//        它不依赖风扇冷却而是通过其他外设保持低温（比如我们实验室的服务器常年放在空调房间里）

//2) Temp: 表示温度

//3) Perf：表示性能状态，从P0到P12，P0表示最大性能,P12状态表示最小性能

//4) Pwr: 表示能耗

//5) Persistence-M: 表示持续模式的状态，持续模式虽然耗能大，但是在新的GPU应用启动时，花费的时间更少，这里显示为off状态

//6) Bus-Id: 涉及GPU总线方面的内容，domain:bus:device.function

//7) Disp.A: 是Display Active，表示GPU的显示是否初始化


# nvidia-smi -L
GPU 0: GeForce GTX 1080 Ti (UUID: GPU-5480eda9-37b3-98ad-ad76-f0b4a33301fa)
GPU 1: GeForce GTX 1080 Ti (UUID: GPU-049bd205-4e80-b2f4-8079-4b531d513ab1)
{% endhighlight %} 

另外，对于其他的GPU则并没有太好的命令，可以在PCI信息里面找，例如：
{% highlight string %}
//lspci即显示所有PCI设备信息

# lspci | grep VGA               
02:00.0 VGA compatible controller: NVIDIA Corporation Device 1b06 (rev a1)
03:00.0 VGA compatible controller: NVIDIA Corporation Device 1b06 (rev a1)

//再通过如下命令查看某一块显卡的详细信息
# lspci -v -s 02:00.0
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


注意：
<pre>
显存占用和GPU占用是两个不一样的东西，显卡是由GPU和显存等组成的，显存和GPU的关系有点类似于内存和CPU的关系
</pre>


**2) 查看显卡设备驱动信息**
{% highlight string %}
// 查看显卡相关的驱动信息(列出已载入系统的模块)
# lsmod | grep nvidia
nvidia_uvm            667648  4
nvidia_drm             45056  2
nvidia_modeset        860160  2 nvidia_drm
nvidia              13139968  571 nvidia_modeset,nvidia_uvm
drm_kms_helper        167936  1 nvidia_drm
drm                   356352  5 nvidia_drm,drm_kms_helper


//探测有没有装载相应的驱动程序(如果报错，则没有装载）
# modprobe -v nvidia-uvm
{% endhighlight %}



## 3. Ubuntu16.04 显卡驱动程序的安装

### 3.1 下载官方程序

[http://www.geforce.cn/drivers](http://www.geforce.cn/drivers)

这里我们结合当前GPU类型：
<pre>
# nvidia-smi -L
GPU 0: GeForce GTX 1080 Ti (UUID: GPU-5480eda9-37b3-98ad-ad76-f0b4a33301fa)
GPU 1: GeForce GTX 1080 Ti (UUID: GPU-049bd205-4e80-b2f4-8079-4b531d513ab1)
</pre>
![nvidia install](https://ivanzz1001.github.io/records/assets/img/linux/linux_gpu_download.jpg)

如果我们直接安装驱动的话，往往会报错：
<pre>
ERROR: The Nouveau kernel driver is currently in use by your system. This driver is incompatible with the NVIDIA driver
</pre>
请接着继续往下看。

### 3.2 禁止集成nouveau驱动
Ubuntu系统集成的显卡驱动程序是nouveau，它是第三方为NVIDIA开发的驱动，我们需要先将其屏蔽才能安装NVIDIA官方驱动。首先将驱动添加到黑名单blacklist.conf中：
<pre>
# sudo chmod 666 /etc/modprobe.d/blacklist.conf    //修改文件属性

//向blacklist.conf文件中添加如下
blacklist vga16fb
blacklist nouveau
blacklist rivafb
blacklist rivatv
blacklist nvidiafb
</pre>


### 3.3 开始安装
先按ctrl+Alt+F1到控制台，关闭当前图形环境。
<pre>
# sudo init 3
# sudo rm -r /tmp/.X* 
# sudo service lightdm stop

//再安装驱动程序
# ./NVIDIA-Linux-x86_64-384.111.run --no-x-check --no-nouveau-check --no-opengl-files

// 重启图形环境
# sudo service lightdm start
</pre>

### 3.4 查看显卡驱动版本
<pre>
# cat /proc/driver/nvidia/version 
NVRM version: NVIDIA UNIX x86_64 Kernel Module  384.111  Tue Dec 19 23:51:45 PST 2017
GCC version:  gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) 
</pre>

## 4. 显卡驱动的卸载
一般情况下，如果当前主机上仍保存有安装包，则可以通过该安装包自带的卸载程序进行卸载。例如：
<pre>
# ./NVIDIA-Linux-x86_64-384.111.run --uninstall
</pre>

也可以通过如下命令卸载，但是卸载后可能不能进入图形界面，后续可能需要重新安装驱动后才能进入：
<pre>
# sudo apt-get remove --purge nvidia*
</pre>


<br />
<br />
**[参看]:**

1. [Ubuntu16.04安装NVIDIA显卡驱动](https://www.cnblogs.com/alexanderkun/p/6905512.html)

2. [Ubuntu16.04安装NVIDIA驱动时的一些坑与解决方案](https://www.cnblogs.com/matthewli/p/6715553.html)

3. [ubuntu 16.04安装nvidia 专用驱动无法登录，一直返回登录界面？](http://forum.ubuntu.org.cn/viewtopic.php?t=477846)



<br />
<br />
<br />


