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

## 1. GPT分区与MBR分区
全局唯一标识分区表(GUID Partition Table，缩写： GPT）是一个实体硬盘的分区结构。它是```可扩展固件接口标准```的一部分，用来替代BIOS中的主引导记录分区表(MBR)。 传统的主启动记录(MBR)磁盘分区支持最大卷为2.2TB(terabytes)，每个磁盘最多有4个主分区（或3个主分区，1个扩展分区和无限制的逻辑驱动器）。与MBR分区方法相比，GPT具有更多的优点，因为它允许每个磁盘有多达128个分区，支持高达18千兆兆字节(exabytes, 1EB=10^6TB)的卷大小，允许将主磁盘分区表和备份磁盘分区表用于冗余，还支持唯一的磁盘和分区ID(GUID)。

与MBR分区的磁盘不同，GPT的分区信息是在分区中，而不像MBR一样在主引导扇区。为保护GPT不受MBR类磁盘管理软件的危害，GPT在主引导扇区建立了一个保护分区(Protective MBR)的MBR分区表，这种分区的类型标识为```0xEE```，这个保护分区的大小在Windows下为128MB，Mac OS X下为200MB，在Windows磁盘管理器中名称为GPT保护分区，可让MBR类磁盘管理软件把GPT看成一个未知格式的分区，而不是错误地当成一个未分区的磁盘。另外GPT分区磁盘有多余的主要及备份分区表来提高分区数据结构的完整性。

在MBR硬盘中，分区信息直接存储在主引导记录（MBR）中（主引导记录中还存储着系统的引导程序）。但在GPT硬盘中，分区表的位置信息存储在GPT头中。但出于兼容性考虑，硬盘的第一个扇区仍然用作MBR，之后才是GPT头。跟现代的MBR一样，GPT也使用逻辑区块地址(LBA)取代了早期的CHS寻址方式。传统的MBR信息存储于LBA 0， GPT头存储于LBA1，接下来才是分区表本身。64位Windows操作系统使用16384字节(或32扇区)作为GPT分区表，接下来的```LBA 34```是硬盘上第一个分区的开始。为了减少分区表损坏的风险，GPT在硬盘最后保存了一份分区表的副本。与主启动记录(MBR)分区方法相比，GPT具有更多的优点，因为它允许每个磁盘多达128个分区，支持高达18千兆兆字节的卷大小，允许将主磁盘分区表和备份磁盘分区表用于冗余，还支持唯一的磁盘和分区ID(GUID)。

1） **传统MBR(LBA 0)**

在GPT分区表的最开头，出于兼容性考虑仍然存储了一份传统的MBR，用来防止不支持GPT的硬盘管理工具错误识别并破坏硬盘中的数据，这个MBR也叫做保护MBR。在支持从GPT启动的操作系统中，这里也用于存储第一阶段的启动代码。在这个MBR中，只有一个标识为```0xEE```的分区，以此来表示这块硬盘使用GPT分区表。不能识别GPT硬盘的操作系统通常会识别出一个位置类型的分区，并且拒绝对硬盘进行操作，除非用户特别要求删除这个分区。这就避免了以外删除分区的危险。另外，能够识别GPT分区表的操作系统会检查保护MBR中的分区表，如果分区类型不是```0xEE```或者MBR分区表中有多个项，也会拒绝对硬盘进行操作。在使用MBR/GPT混合分区表的硬盘中，这部分存储了GPT分区表的一部分分区（通常是前四个分区），可以使不支持从GPT启动的操作系统从这个MBR启动，启动后只能操作MBR分区表中的分区。如Boot Camp就是使用这种方式启动Windows。


2) **分区表头(LBA 1)**

分区表头定义了硬盘的可用空间以及组成分区表的项的大小和数量。在使用64位Windows Server 2003的机器上，最多可以创建128个分区，即分区表中保留了128个项，其中每个都是128字节。(EFI标准要求分区表最小要有16384字节，即128个分区项的大小）。

分区表头还记录了这块硬盘的GUID，记录了分区表头本身的位置和大小（位置总是在LBA 1)以及备份分区表头和分区表的位置和大小（在硬盘的最后）。它还存储着它本身和分区表的CRC32校验。固件、引导程序和操作系统在启动时可以根据这个校验值来判断分区表是否出错，如果出错了，可以使用软件从硬盘最后的本分GPT中恢复整个分区表； 如果备份GPT也校验错误，硬盘将不可使用。所以GPT硬盘的分区表不可以直接使用16进制编辑器修改。


## 2. sgdisk工具的使用

通过如下命令删除所有分区：
<pre>
# sgdisk -Zog /dev/sdi
</pre>

## 3. fdisk工具的使用

1） **使用fdisk查看分区信息**

我们可以使用```fdisk```命令来显示你的所有磁盘或闪存的信息以及它们的```分区```信息。
<pre>
# fdisk -l /dev/sdj
WARNING: fdisk GPT support is currently new, and therefore in an experimental phase. Use at your own discretion.

Disk /dev/sdj: 1200.2 GB, 1200243695616 bytes, 2344225968 sectors
Units = sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 33553920 bytes
Disk label type: gpt


#         Start          End    Size  Type            Name
 1        65535     23461529   11.2G  Microsoft basic primary
 2     23461530   2344186949    1.1T  Microsoft basic primary
</pre>
上面我们看到最新版的```fdisk```工具也支持GPT分区了。并且看到第一个分区是从65535这个扇区开始的（这里进行了对齐，请参看后面parted)。

2) **使用badblocks工具进行坏块扫描**

<pre>
# badblocks -v /dev/sdj1 >> badblocks.txt
Checking blocks 0 to 11697996
Checking for bad blocks (read-only test): 
done                                                 
Pass completed, 0 bad blocks found. (0/0/0 errors)
</pre>

3) **使用df来查看查看文件系统的磁盘使用情况**

我们可以使用```df```命令来查看文件系统的磁盘使用情况，例如：
<pre>
# df
Filesystem                 1K-blocks       Used  Available Use% Mounted on
/dev/mapper/test-lv_root   20961280    1941848   19019432  10% /
devtmpfs                   131917224          0  131917224   0% /dev
tmpfs                      131928216        212  131928004   1% /dev/shm
tmpfs                      131928216    1009116  130919100   1% /run
tmpfs                      131928216          0  131928216   0% /sys/fs/cgroup
/dev/sdk2                     607980     140004     467976  24% /boot
/dev/mapper/test-lv_var  7781410816  260187916 7521222900   4% /var
/dev/sda2                 7801452868 3159489912 4641962956  41% /var/lib/ceph/osd/ceph-16
/dev/sdb2                 7801452868 3462296992 4339155876  45% /var/lib/ceph/osd/ceph-38
/dev/sdc2                 7801452868 2856510324 4944942544  37% /var/lib/ceph/osd/ceph-54
/dev/sdd2                 7801452868 2901124256 4900328612  38% /var/lib/ceph/osd/ceph-59
/dev/sde2                 7801452868 3588682536 4212770332  47% /var/lib/ceph/osd/ceph-65
/dev/sdf2                 7801452868 3328774296 4472678572  43% /var/lib/ceph/osd/ceph-70
tmpfs                       26385644          0   26385644   0% /run/user/0
/dev/sdg2                 7801452868 3810297232 3991155636  49% /var/lib/ceph/osd/ceph-76
/dev/sdh2                 7801452868 2681698220 5119754648  35% /var/lib/ceph/osd/ceph-81
/dev/sdi2                 7801452868 2724485808 5076967060  35% /var/lib/ceph/osd/ceph-89
/dev/sdl2                  458140932   11186764  446954168   3% /var/lib/ceph/osd/ceph-96
/dev/sdm2                  458140932    7706308  450434624   2% /var/lib/ceph/osd/ceph-103
</pre>

4) **使用lvdisplay命令显示逻辑卷**

lvdisplay命令用于显示LVM逻辑卷空间大小、读写状态和快照信息等属性。如果省略```逻辑卷```参数，则vdisplay命令显示所有的逻辑卷属性，否则，仅显示指定的逻辑卷属性：
<pre>
# lvdisplay 
  --- Logical volume ---
  LV Path                /dev/test/lv_var
  LV Name                lv_var
  VG Name                test
  LV UUID                oXHqWi-VMw9-6xrV-GQSO-VRHx-JA9J-TZVQpq
  LV Write Access        read/write
  LV Creation host, time ceph-173.test.net, 2017-10-24 13:38:14 +0800
  LV Status              available
  # open                 1
  LV Size                7.25 TiB
  Current LE             1900268
  Segments               1
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     256
  Block device           253:1
   
  --- Logical volume ---
  LV Path                /dev/test/lv_root
  LV Name                lv_root
  VG Name                test
  LV UUID                79go9I-8Dqa-eihG-jB9q-vEZV-VwrR-zt04by
  LV Write Access        read/write
  LV Creation host, time ceph-173.test.net, 2017-10-24 13:39:33 +0800
  LV Status              available
  # open                 1
  LV Size                20.00 GiB
  Current LE             5120
  Segments               1
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     256
  Block device           253:0
   
# lvdisplay /dev/test/lv_root
  --- Logical volume ---
  LV Path                /dev/test/lv_root
  LV Name                lv_root
  VG Name                test
  LV UUID                79go9I-8Dqa-eihG-jB9q-vEZV-VwrR-zt04by
  LV Write Access        read/write
  LV Creation host, time ceph-173.test.net, 2017-10-24 13:39:33 +0800
  LV Status              available
  # open                 1
  LV Size                20.00 GiB
  Current LE             5120
  Segments               1
  Allocation             inherit
  Read ahead sectors     auto
  - currently set to     256
  Block device           253:0
</pre>
之后，我们再可以通过```mount```命令查看对应的硬盘挂载到了哪个目录，例如：
<pre>
# mount | grep "lv_root"
/dev/mapper/test-lv_root on / type xfs (rw,relatime,attr2,inode64,noquota)
</pre>
可以看到挂载到了根目录。

## 4. parted工具

GNU Parted是一个用于创建与操作分区表的小应用程序。我们前面介绍了GPT，由于常见的fdisk不支持GPT(在硬盘容量大于2TB的时候无法使用fdisk进行分区管理），故在```IA64```平台上管理磁盘时parted还是相当实用的，GNU Parted具有丰富的功能，它除了能够进行分区的添加、删除等常见操作外，还可以进行移动分区、创建文件系统、调整文件系统大小、复制文件系统等操作。它可以处理最常见的分区格式，包括：ext2、ext3、fat16、fat32、NTFS、ReiserFS、JFS、XFS、UFS、HFS以及Linux交换分区。

下面我们简单看一下```parted```命令的基本用法：
<pre>
# parted --help
Usage: parted [OPTION]... [DEVICE [COMMAND [PARAMETERS]...]...]
Apply COMMANDs with PARAMETERS to DEVICE.  If no COMMAND(s) are given, run in
interactive mode.

OPTIONs:
  -h, --help                      displays this help message
  -l, --list                      lists partition layout on all block devices
  -m, --machine                   displays machine parseable output
  -s, --script                    never prompts for user intervention
  -v, --version                   displays the version
  -a, --align=[none|cyl|min|opt]  alignment for new partitions

COMMANDs:
  align-check TYPE N                        check partition N for TYPE(min|opt)
        alignment
  help [COMMAND]                           print general help, or help on
        COMMAND
  mklabel,mktable LABEL-TYPE               create a new disklabel (partition
        table)
  mkpart PART-TYPE [FS-TYPE] START END     make a partition
  name NUMBER NAME                         name partition NUMBER as NAME
  print [devices|free|list,all|NUMBER]     display the partition table,
        available devices, free space, all found partitions, or a particular
        partition
  quit                                     exit program
  rescue START END                         rescue a lost partition near START
        and END
  rm NUMBER                                delete partition NUMBER
  select DEVICE                            choose the device to edit
  disk_set FLAG STATE                      change the FLAG on selected device
  disk_toggle [FLAG]                       toggle the state of FLAG on selected
        device
  set NUMBER FLAG STATE                    change the FLAG on partition NUMBER
  toggle [NUMBER [FLAG]]                   toggle the state of FLAG on partition
        NUMBER
  unit UNIT                                set the default unit to UNIT
  version                                  display the version number and
        copyright information of GNU Parted

Report bugs to bug-parted@gnu.org
</pre>

通过如下的命令查看当前的所有硬盘分区信息：
<pre>
# lsblk
NAME             MAJ:MIN RM    SIZE RO TYPE MOUNTPOINT
sda                8:0    0    1.1T  0 disk 
├─sda1             8:1    0   11.2G  0 part 
└─sda2             8:2    0    1.1T  0 part /var/lib/ceph/osd/ceph-18
sdb                8:16   0    1.1T  0 disk 
├─sdb1             8:17   0   11.2G  0 part 
└─sdb2             8:18   0    1.1T  0 part /var/lib/ceph/osd/ceph-19
sdc                8:32   0    1.1T  0 disk 
├─sdc1             8:33   0   11.2G  0 part 
└─sdc2             8:34   0    1.1T  0 part /var/lib/ceph/osd/ceph-20
sdd                8:48   0    1.1T  0 disk 
├─sdd1             8:49   0   11.2G  0 part 
└─sdd2             8:50   0    1.1T  0 part /var/lib/ceph/osd/ceph-21
sde                8:64   0    1.1T  0 disk 
├─sde1             8:65   0   11.2G  0 part 
└─sde2             8:66   0    1.1T  0 part /var/lib/ceph/osd/ceph-22
sdf                8:80   0    1.1T  0 disk 
├─sdf1             8:81   0   11.2G  0 part 
└─sdf2             8:82   0    1.1T  0 part /var/lib/ceph/osd/ceph-23
sdg                8:96   0    1.1T  0 disk 
├─sdg1             8:97   0   11.2G  0 part 
└─sdg2             8:98   0    1.1T  0 part /var/lib/ceph/osd/ceph-24
sdh                8:112  0    1.1T  0 disk 
├─sdh1             8:113  0   11.2G  0 part 
└─sdh2             8:114  0    1.1T  0 part /var/lib/ceph/osd/ceph-25
sdi                8:128  0    1.1T  0 disk 
├─sdi1             8:129  0   11.2G  0 part 
└─sdi2             8:130  0    1.1T  0 part /var/lib/ceph/osd/ceph-26
sdj                8:144  0    1.1T  0 disk 
sdk                8:160  0    1.1T  0 disk 
sdl                8:176  0    1.1T  0 disk 
sdm                8:192  0    1.1T  0 disk 
sdn                8:208  0    1.1T  0 disk 
├─sdn1             8:209  0    200M  0 part /boot
└─sdn2             8:210  0    1.1T  0 part 
  ├─VG01-lv_root 253:0    0    100G  0 lvm  /
  ├─VG01-lv_swap 253:1    0      8G  0 lvm  [SWAP]
  └─VG01-lv_data 253:2    0 1008.5G  0 lvm  /data
</pre>

然后我们可以通过如下命令查看当前各分区的详细信息：
{% highlight string %}
# parted /dev/sdj print all
Error: /dev/sdj: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 

Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sda: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdb: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdc: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdd: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sde: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdf: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: Linux device-mapper (linear) (dm)
Disk /dev/mapper/VG01-lv_data: 1083GB
Sector size (logical/physical): 512B/512B
Partition Table: loop
Disk Flags: 

Number  Start  End     Size    File system  Flags
 1      0.00B  1083GB  1083GB  xfs


Model: Linux device-mapper (linear) (dm)
Disk /dev/mapper/VG01-lv_swap: 8590MB
Sector size (logical/physical): 512B/512B
Partition Table: loop
Disk Flags: 

Number  Start  End     Size    File system     Flags
 1      0.00B  8590MB  8590MB  linux-swap(v1)


Model: Linux device-mapper (linear) (dm)
Disk /dev/mapper/VG01-lv_root: 107GB
Sector size (logical/physical): 512B/512B
Partition Table: loop
Disk Flags: 

Number  Start  End    Size   File system  Flags
 1      0.00B  107GB  107GB  xfs


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdg: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdh: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdi: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB  xfs          primary


Error: /dev/sdk: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdk: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 

Error: /dev/sdl: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdl: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 

Error: /dev/sdm: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdm: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 

Model: LSI LSI (scsi)
Disk /dev/sdn: 1199GB
Sector size (logical/physical): 512B/512B
Partition Table: msdos
Disk Flags: 

Number  Start   End     Size    Type     File system  Flags
 1      1049kB  211MB   210MB   primary  xfs          boot
 2      211MB   1199GB  1199GB  primary               lvm
{% endhighlight %}


也可以通过如下命令查看某一块硬盘的详细信息：
<pre>
# parted /dev/sdj print unit s print unit chs print
Error: /dev/sdj: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 
Error: /dev/sdj: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdj: 2344225968s
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 
Error: /dev/sdj: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdj: 145921,80,62
Sector size (logical/physical): 512B/512B
BIOS cylinder,head,sector geometry: 145921,255,63.  Each cylinder is 8225kB.
Partition Table: unknown
Disk Flags: 
</pre>

```parted```命令支持交互式与非交互式两种工作方式，通过示例的方式展示一下对应的使用方法。

### 4.1 交互式parted分区命令
{% highlight string %}
# parted /dev/sdj
GNU Parted 3.1
Using /dev/sdj
Welcome to GNU Parted! Type 'help' to view a list of commands.
(parted) help                                                             
  align-check TYPE N                        check partition N for TYPE(min|opt) alignment
  help [COMMAND]                           print general help, or help on COMMAND
  mklabel,mktable LABEL-TYPE               create a new disklabel (partition table)
  mkpart PART-TYPE [FS-TYPE] START END     make a partition
  name NUMBER NAME                         name partition NUMBER as NAME
  print [devices|free|list,all|NUMBER]     display the partition table, available devices, free space, all found partitions, or a particular partition
  quit                                     exit program
  rescue START END                         rescue a lost partition near START and END
  rm NUMBER                                delete partition NUMBER
  select DEVICE                            choose the device to edit
  disk_set FLAG STATE                      change the FLAG on selected device
  disk_toggle [FLAG]                       toggle the state of FLAG on selected device
  set NUMBER FLAG STATE                    change the FLAG on partition NUMBER
  toggle [NUMBER [FLAG]]                   toggle the state of FLAG on partition NUMBER
  unit UNIT                                set the default unit to UNIT
  version                                  display the version number and copyright information of GNU Parted
(parted)
(parted)                                                                  
(parted)                                                                  
(parted) p           							//打印出当前硬盘的相关信息                                                     
Error: /dev/sdj: unrecognised disk label
Model: SEAGATE ST1200MM0007 (scsi)                                        
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: unknown
Disk Flags: 
(parted)      
(parted)  
(parted)  
(parted) mklabel                       			//在没有Partition Table之前需要先创建label,用mklabel或mktable                                   
New disk label type? gpt 
(parted) p                                                                
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start  End  Size  File system  Name  Flags
(parted)      
(parted)  
(parted)  
(parted) mkpart                 				//创建第一个分区                                          
Partition name?  []? primary                                              
File system type?  [ext2]? xfs                                            
Start? 0%                                                                 
End? 10GiB
Warning: The resulting partition is not properly aligned for best performance.
Ignore/Cancel? I                                                          
(parted) p                                                                
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      17.4kB  10.7GB  10.7GB               primary

(parted)
(parted)      
(parted) 
(parted) mkpart              				//创建第二个分区                                             
Partition name?  []? primary                                              
File system type?  [ext2]? xfs                                            
Start? 10GiB                                                              
End? 100%                                                                 
Warning: The resulting partition is not properly aligned for best performance.
Ignore/Cancel? I  
(parted)                                                                  
(parted)                                                                  
(parted)
(parted) p           						//打印当前分区信息                                                     
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      17.4kB  10.7GB  10.7GB               primary
 2      10.7GB  1200GB  1190GB               primary
(parted)                                                                  
(parted)                                                                  
(parted)                                                                  
(parted)                                                                  
(parted) rm 1           					//删除硬盘分区                                                  
(parted) p
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 2      10.7GB  1200GB  1190GB               primary

(parted) rm 2
(parted) p                                                                
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start  End  Size  File system  Name  Flags
(parted)                                                                  
(parted)                                                                  
(parted)                                                                  
(parted) 
(parted) mkpart           				//删除分区后重新分区                                                
Partition name?  []? primary
File system type?  [ext2]? xfs                                            
Start? 0%                                                                 
End? 1%                                                                   
(parted) mkpart                                                           
Partition name?  []? primary                                              
File system type?  [ext2]? xfs                                            
Start? 1%                                                                 
End? 100%                                                                 
(parted) p                                                                
Model: SEAGATE ST1200MM0007 (scsi)
Disk /dev/sdj: 1200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      33.6MB  12.0GB  12.0GB               primary
 2      12.0GB  1200GB  1188GB               primary                                                        
(parted)                                                                  
(parted)                                                                  
(parted) 
(parted) q                    	//退出parted                                            
Information: You may need to update /etc/fstab.
{% endhighlight %}

一般执行完分区后，可以执行如下命令以使操作系统了解到相应的分区信息：
<pre>
# partprobe                                   
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
</pre>


### 4.2 非交互式parted分区命令
下面的示例我们对```/dev/sdj```硬盘进行分区：

* 查看分区

因为前面我们已经对/dev/sdj进行过分区，下面我们先查看一下分区信息：
<pre>
# lsblk /dev/sdj
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sdj      8:144  0  1.1T  0 disk 
├─sdj1   8:145  0 11.2G  0 part 
└─sdj2   8:146  0  1.1T  0 part 
</pre>

* 删除分区

通过如下的命令删除分区sdj1和sdj2:
<pre>
# parted /dev/sdj rm 1
Information: You may need to update /etc/fstab.
# parted /dev/sdj rm 2
Information: You may need to update /etc/fstab.
# lsblk /dev/sdj                                         
NAME MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sdj    8:144  0  1.1T  0 disk 
</pre>

* 对硬盘进行GPT分区
<pre>
# parted -s /dev/sdj mklabel gpt
# parted -s /dev/sdj mkpart primary 0% 1%
# parted -s /dev/sdj mkpart primary 1% 100%
# lsblk /dev/sdj
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sdj      8:144  0  1.1T  0 disk 
├─sdj1   8:145  0 11.2G  0 part 
└─sdj2   8:146  0  1.1T  0 part 
</pre>

* 对分区后的文件系统进行格式化
<pre>
# mkfs -t xfs -f -i size=2048 /dev/sdj1
meta-data=/dev/sdj1              isize=2048   agcount=4, agsize=731125 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=2924499, imaxpct=25
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=2560, version=2
         =                       sectsz=512   sunit=0 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
# mkfs -t xfs -f -i size=2048 /dev/sdj2
meta-data=/dev/sdj2              isize=2048   agcount=4, agsize=72522670 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=290090677, imaxpct=5
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=141645, version=2
         =                       sectsz=512   sunit=0 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
</pre>

分区完成之后，执行如下命令以使操作系统了解到相应的分区信息：
<pre>
# partprobe                                   
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
</pre>

## 4.3 parted命令中的unit
```parted```工具中很多地方可能会要求输入分区```开始位置```、```结束位置```，而这时还需要输入单位。下面我们列出parted命令中的一些```unit```:
<pre>
unit may be one of:
s   : sector (n bytes depending on the sector size, often 512)
B   : byte
KiB : kibibyte (1024 bytes)
MiB : mebibyte (1048576 bytes)
GiB : gibibyte (1073741824 bytes)
TiB : tebibyte (1099511627776 bytes)
KB  : kilobyte (1000 bytes)
MB  : megabyte (1000000 bytes)
GB  : gigabyte (1000000000 bytes)
TB  : terabyte (1000000000000 bytes)
%   : percentage of the device (between 0 and 100)
cyl : cylinders (related to the BIOS CHS geometry)
chs : cylinders, heads, sectors addressing (related to the BIOS CHS geometry)

compact:  This is a special unit that defaults to megabytes for input, and picks a
	unit that gives a compact human readable representation for output.
</pre>


### 4.4 如何对齐partitions以获得最好的性能
在Linux操作系统上，当我们在大型存储阵列上创建分区的时候通常会遇到两个常见的问题。第一个问题类似于如下：
<pre>
WARNING: The size of this disk is 8.0 TB (7970004230144 bytes).
DOS partition table format can not be used on drives for volumes
larger than (2199023255040 bytes) for 512-byte sectors. Use parted(1) and GUID 
partition table format (GPT).
</pre>
因为```fdisk```不能对大于2TB的硬盘进行分区，因此这里我们需要使用```parted```来进行GPT分区。

第二个问题是我们在分区的过程中，通常会得到如下的警告信息：
<pre>
(parted) mklabel gpt
(parted) mkpart primary 0 10GiB
Warning: The resulting partition is not properly aligned for best performance.
Ignore/Cancel?
</pre>
很多时候我们将相应的值进行了多次调整，但仍然得到同样的警告。上面```parted```正在尝试进行忽略，但如果要获得较好的性能，还是不建议这么做。

下面是一个```step-by-step```的guide来说明如何正确的对齐```partitions```。对于大部分存储阵列来说，下面的方法都适用（实际上，目前还未遇到不适用的）。

1） **获得对齐参数**

我们可以通过如下的命令来获取对应存储阵列的参数（记得将```sdj```替换为你当前对应的硬盘名称):
<pre>
# cat /sys/block/sdj/queue/optimal_io_size
33553920

# cat /sys/block/sdj/queue/minimum_io_size
512

# cat /sys/block/sdj/alignment_offset
0

# cat /sys/block/sdj/queue/physical_block_size
512
</pre>

2) **计算从哪个扇区开始进行分区**

通过使用如下的公式计算从哪个扇区开始进行分区：
<pre>
开始分区扇区 = (optimal_io_size + alignment_offset) / physical_block_size
</pre>
在我们上面的例子中，我们计算出的开始分区的扇区是： (33553920 + 0) / 512 = 65535

3) 从指定扇区进行分区

上面我们算得了应该从哪个扇区开始进行分区，因此：
<pre>
# mkpart primary 65535s 100%
</pre>
请注意上面的```s```，其用于告诉```parted```应该以sectors为单位。

4) 假如所有都工作正常的话，我们在创建分区时就不会再出现上面提到的警告信息了。你可以通过使用如下的命令来进行对齐检查（如果要检查其他的分区，请用对应的数字替换```1```)
<pre>
(parted) align-check optimal 1                                            
1 aligned
</pre>


**Important!!**:

通常情况下，我们可以使用```%```来让parted自动的进行扇区对齐以获得最好的性能。


<br />
<br />

**[参看]**

1. [Linux下的parted工具的使用 GPT分区安装系统](http://blog.51cto.com/tlinux/1739407)

2. [parted分区和挂载及非交互式操作](https://www.cnblogs.com/kaishirenshi/p/7850247.html)

3. [GPT和parted命令详解(原创)](http://czmmiao.iteye.com/blog/1751408)

4. [在 Linux 上检测硬盘上的坏道和坏块](https://linux.cn/article-7961-1.html)

5. [parted分区工具用法](https://www.cnblogs.com/yinzhengjie/p/6844372.html)

6. [GNU Parted](https://www.gnu.org/software/parted/)

7. [Parted User's Manual](https://www.gnu.org/software/parted/manual/)

8. [如何使用parted对齐分区以得到最优性能](https://blog.csdn.net/open_data/article/details/44828741)

9. [sgdisk基本用法](https://blog.csdn.net/ygtlovezf/article/details/76269800)

10. [UEFI+GPT与BIOS+MBR各自有什么优缺点](https://blog.csdn.net/yang2716210363/article/details/78581388)

<br />
<br />
<br />


