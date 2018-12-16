---
layout: post
title: Ubuntu16.04操作系统环境下修改时区
tags:
- LinuxOps
categories: linuxOps
description: Ubuntu16.04操作系统环境下修改时区
---


本文主要介绍如何在Ubuntu16.04操作系统环境下修改时区。

<!-- more -->


## 1. 时区相关操作

如果你的Linux系统时区配置不正确，必须要手动调整到正确的当地时区。NTP对时间的同步处理只计算当地时间与UTC时间的偏移量，因此配置一个NTP对时间进行同步并不能解决时区不正确的问题。所以大家在用了国外云计算服务商如Microsoft Azure 或者其他vps、虚拟机时，需要注意是否与中国大陆的时区一致。

**1） 查看Linux当前时区**

你可以使用如下命令非常容易地就查看到Linux系统的当前时区：
{% highlight string %}
# date
Tue Feb  6 20:37:28 CST 2018

# timedatectl
      Local time: Sun 2018-12-16 02:58:35 PST
  Universal time: Sun 2018-12-16 10:58:35 UTC
        RTC time: Sun 2018-12-16 10:58:35
       Time zone: America/Los_Angeles (PST, -0800)
 Network time on: yes
NTP synchronized: yes
 RTC in local TZ: no

# ls -al /etc/localtime 
lrwxrwxrwx. 1 root root 33 Feb  6 20:38 /etc/localtime -> /usr/share/zoneinfo/Asia/Shanghai
{% endhighlight %}
上面```CST```可视为美国、澳大利亚、古巴或中国的标准时间。

```CST```可以为如下4个不同的时区的缩写：

* 美国中部时间：Central Standard Time (USA) UT-6:00

* 澳大利亚中部时间：Central Standard Time (Australia) UT+9:30

* 中国标准时间：China Standard Time UT+8:00

* 古巴标准时间：Cuba Standard Time UT-4:00

<br />

**2) 获取时区TZ值**

要更改Linux系统时区首先得获知你所在当地时区的TZ值，使用```tzselect```命令即可查看并选择已安装的时区文件：
{% highlight string %}
[root@localhost test-src]# tzselect
Please identify a location so that time zone rules can be set correctly.
Please select a continent or ocean.
 1) Africa
 2) Americas
 3) Antarctica
 4) Arctic Ocean
 5) Asia
 6) Atlantic Ocean
 7) Australia
 8) Europe
 9) Indian Ocean
10) Pacific Ocean
11) none - I want to specify the time zone using the Posix TZ format.
#? 5
Please select a country.
 1) Afghanistan           18) Israel                35) Palestine
 2) Armenia               19) Japan                 36) Philippines
 3) Azerbaijan            20) Jordan                37) Qatar
 4) Bahrain               21) Kazakhstan            38) Russia
 5) Bangladesh            22) Korea (North)         39) Saudi Arabia
 6) Bhutan                23) Korea (South)         40) Singapore
 7) Brunei                24) Kuwait                41) Sri Lanka
 8) Cambodia              25) Kyrgyzstan            42) Syria
 9) China                 26) Laos                  43) Taiwan
10) Cyprus                27) Lebanon               44) Tajikistan
11) East Timor            28) Macau                 45) Thailand
12) Georgia               29) Malaysia              46) Turkmenistan
13) Hong Kong             30) Mongolia              47) United Arab Emirates
14) India                 31) Myanmar (Burma)       48) Uzbekistan
15) Indonesia             32) Nepal                 49) Vietnam
16) Iran                  33) Oman                  50) Yemen
17) Iraq                  34) Pakistan
#? 13

The following information has been given:

        Hong Kong

Therefore TZ='Asia/Hong_Kong' will be used.
Local time is now:      Tue Feb  6 20:51:41 HKT 2018.
Universal Time is now:  Tue Feb  6 12:51:41 UTC 2018.
Is the above information OK?
1) Yes
2) No
#? 1    

You can make this change permanent for yourself by appending the line
        TZ='Asia/Hong_Kong'; export TZ
to the file '.profile' in your home directory; then log out and log in again.

Here is that TZ value again, this time on standard output so that you
can use the /usr/bin/tzselect command in shell scripts:
Asia/Hong_Kong
{% endhighlight %}
通过上面我们知道香港时区值的写法为```Asia/Hong_Kong```

<br />
此外，我们也可以通过如下方式来获得各时区的写法：
<pre>
# ls /usr/share/zoneinfo/
Africa      Asia       Canada   Cuba   EST      GB       GMT-0      HST      iso3166.tab  Kwajalein  MST      NZ-CHAT   posix       right      Turkey     UTC       Zulu
America     Atlantic   CET      EET    EST5EDT  GB-Eire  GMT+0      Iceland  Israel       Libya      MST7MDT  Pacific   posixrules  ROC        UCT        WET
Antarctica  Australia  Chile    Egypt  Etc      GMT      Greenwich  Indian   Jamaica      MET        Navajo   Poland    PRC         ROK        Universal  W-SU
Arctic      Brazil     CST6CDT  Eire   Europe   GMT0     Hongkong   Iran     Japan        Mexico     NZ       Portugal  PST8PDT     Singapore  US         zone.tab

# ls /usr/share/zoneinfo/Asia/
Aden           Bahrain        Choibalsan     Dushanbe       Jakarta        Khandyga       Makassar       Pontianak      Seoul          Thimbu         Vientiane
Almaty         Baku           Chongqing      Gaza           Jayapura       Kolkata        Manila         Pyongyang      Shanghai       Thimphu        Vladivostok
Amman          Bangkok        Chungking      Harbin         Jerusalem      Krasnoyarsk    Muscat         Qatar          Singapore      Tokyo          Yakutsk
Anadyr         Barnaul        Colombo        Hebron         Kabul          Kuala_Lumpur   Nicosia        Qyzylorda      Srednekolymsk  Tomsk          Yangon
Aqtau          Beirut         Dacca          Ho_Chi_Minh    Kamchatka      Kuching        Novokuznetsk   Rangoon        Taipei         Ujung_Pandang  Yekaterinburg
Aqtobe         Bishkek        Damascus       Hong_Kong      Karachi        Kuwait         Novosibirsk    Riyadh         Tashkent       Ulaanbaatar    Yerevan
Ashgabat       Brunei         Dhaka          Hovd           Kashgar        Macao          Omsk           Saigon         Tbilisi        Ulan_Bator     
Ashkhabad      Calcutta       Dili           Irkutsk        Kathmandu      Macau          Oral           Sakhalin       Tehran         Urumqi         
Baghdad        Chita          Dubai          Istanbul       Katmandu       Magadan        Phnom_Penh     Samarkand      Tel_Aviv       Ust-Nera
</pre>

**3) 更改每个用户的时区**

Linux是一个多用户系统，每个用户都可以配置自己所需的时区，你可以为自己新增一个TZ环境变量：
<pre>
export TZ='Asia/Shanghai'
</pre>
执行完成后，需要重新登录系统或刷新~/.bashrc生效。
<pre>
# source ~/.bashrc
</pre>

<br />

**4) 更改系统时区**

要更改Linux系统整个系统范围的时区可以使用如下命令：
<pre>
# ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
</pre>

注意： /usr/share/zoneinfo/Asia/Shanghai中的具体时区请用自己获取的TZ值进行替换。

<br />

**5) 使用Systemd更改Linux系统时区**

如果你使用的Linux系统使用Systemd，还可以使用```timedatectl```命令来更改Linux系统范围的时区。在Systemd下有一个名为systemd-timedated的系统服务负责调整系统时钟和时区，我们可以使用timedatectl命令对此系统服务进行配置：
<pre>
# sudo timedatectl set-timezone 'Asia/Shanghai'
</pre>


<br />
<br />
**[参看]:**

1. [如何调整Linux系统为正确时区](http://blog.csdn.net/linuxnews/article/details/51325180)

2. [Linux下设置时区（通过shell设置和程序中设置）及程序中设置环境变量](https://www.cnblogs.com/dongzhiquan/archive/2012/01/09/2317633.html)

3. [显示各个时区](http://blog.csdn.net/youcharming/article/details/43700287)

4. [ubuntu修改时区和时间的方法](https://blog.csdn.net/u012839224/article/details/78705622)

<br />
<br />
<br />


