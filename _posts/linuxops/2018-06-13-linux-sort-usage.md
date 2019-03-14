---
layout: post
title: sort命令的使用
tags:
- LinuxOps
categories: linuxOps
description: sort命令的使用
---

本节主要介绍一下sort命令的使用。

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux sz-oss-01.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>


<!-- more -->


## 1. sort命令
sort命令用于对文本文件中的```行```进行排序。其基本语法如下：
<pre>
sort [OPTION]... [FILE]...
sort [OPTION]... --files0-from=F
</pre>

### 1.1 相关选项
sort命令会将排序后的结果输出到```标准输出```。下面我们介绍一下常用选项：
{% highlight string %}
1) -b, --ignore-leading-blanks: 忽略开头的空白字符

2) -d, --dictionary-order: 按字典顺序进行排序，只处理英文字母、数字和空格

3)  -f, --ignore-case: 忽略大小写。在排序时会将小写通通转换为大写

4） -g, --general-numeric-sort： 根据通常的数字值来进行比较

5) -i, --ignore-nonprinting: 忽略不可打印字符

6)  -M, --month-sort: 将前面3个字母依照月份的缩写进行排序。compare (unknown) < 'JAN' < ... < 'DEC'

7） -h, --human-numeric-sort： compare human readable numbers (e.g., 2K 1G)

8） -n, --numeric-sort： 依照数值的大小进行排序

9) -r, --reverse: 以相反顺序进行排序。默认情况下是按升序进行排序

10） --sort=WORD： 根据某种指定的方式来进行排序。WORD的值可以为general-numeric -g, human-numeric -h, month -M, numeric -n, random -R, version -V

11) -V, --version-sort: 对文本中的版本号按自然顺序排列

12） --batch-size=NMERGE： 至多可以合并NMERGE个文件输入

13) -c, --check, --check=diagnose-first: 检查文件是否已经按照顺序排列

14) -C, --check=quiet, --check=silent: 与-c选项类似，但是并不会报告第一个失配行

15) -k, --key=KEYDEF: 通过一个指定的key来进行排序。KEYDEF指定location和type

16） -m, --merge： 合并已经排好序的文件。并不进行排序

17) -o, --output=FILE: 将输出写入到文件，而不是到标准输出

18) -s, --stable: 进行稳定排序

19) -t, --field-separator=SEP: 指定排序时所用的栏位分隔字符；
{% endhighlight %}


## 2. sort使用示例
1) sort将文件/文本的每一行作为一个单位，相互比较，比较原则是从首字符向后，依次按ASCII码值进行比较，最后将他们按升序输出：
<pre>
# cat sort.txt
aaa:10:1.1
ccc:30:3.3
ddd:40:4.4
bbb:20:2.2
eee:50:5.5
eee:50:5.5

# sort sort.txt
aaa:10:1.1
bbb:20:2.2
ccc:30:3.3
ddd:40:4.4
eee:50:5.5
eee:50:5.5
</pre>


2) 忽略相同行，使用```-u```选项或者```uniq```
<pre>
cat sort.txt
aaa:10:1.1
ccc:30:3.3
ddd:40:4.4
bbb:20:2.2
eee:50:5.5
eee:50:5.5
aaa:10:1.1
# sort -u sort.txt
aaa:10:1.1
bbb:20:2.2
ccc:30:3.3
ddd:40:4.4
eee:50:5.5
</pre>

3) sort的```-n```、```-r```、```-k```、```-t```选项的使用
{% highlight string %}
# cat sort.txt
AAA:BB:CC
aaa:30:1.6
ccc:50:3.3
ddd:20:4.2
bbb:10:2.5
eee:40:5.4
eee:60:5.1

//将BB列按照数字从小到大进行排序
# sort -t: -nk 2 sort.txt
AAA:BB:CC
bbb:10:2.5
ddd:20:4.2
aaa:30:1.6
eee:40:5.4
ccc:50:3.3
eee:60:5.1

//将CC列按数字从大到小进行排列
# sort -t: -nrk 3 sort.txt 
eee:40:5.4
eee:60:5.1
ddd:20:4.2
ccc:50:3.3
bbb:10:2.5
aaa:30:1.6
AAA:BB:CC

//-n是按照数字大小排序，-r是以相反顺序，-k是指定需要排序的栏位，-t指定栏位分隔符为冒号
{% endhighlight %}

3) ```-k```选项的具体语法格式

```-k```选项的语法格式如下：
<pre>
FStart.CStart Modifier,FEnd.CEnd Modifier
-------Start--------,-------End--------
 FStart.CStart 选项,  FEnd.CEnd 选项
</pre>
这个语法格式可以被其中的```逗号```分为两大部分，**Start部分**和**End部分**。 Start部分也是由三部分组成， 其中的```Modifier```部分就是我们之前说过的类似```n```和```r```的选项部分。我们重点说说**Start部分**的```FStart```和```CStart```。

```CStart```也是可以省略的，省略的话就表示从本域的开头部分开始。```FStart.CStart```，其中**FStart**就是表示使用的域，而**CStart**则表示在**FStart**域中从第几个字符开始算```排序首字符```。同理，在**End**部分中，你可以设定**FEnd.CEnd**，如果你省略```.CEnd```，则表示到```域尾```，即本域的最后一个字符，或者如果你将```CEnd```设定为0（零），也是表示结尾到```域尾```。


参看如下示例:

* 从公司英文名称的第二个字母开始进行排序：
<pre>
# cat facebook.txt 
google 110 5000
guge 50 3000
baidu 100 500
sohu 100 4500

# sort -t ' ' -k 1.2 facebook.txt 
baidu 100 500
sohu 100 4500
google 110 5000
guge 50 3000
</pre>

上面使用了```-k 1.2```，表示对第一个域的第二个字符开始到本域的最后一个字符为止的字符串进行排序。你会发现```baidu```因为第二个字母是```a```而名列榜首。```sohu```和```google```第二个字符都是```o```，但是```sohu```的**h**在```google```的**o**前面，所以两者分别排在第二和第三。```guge```只能屈居第四了。

* 只针对公司英文名称的第二个字母进行排序，如果相同的按照员工工资进行降序排序
<pre>
# sort -t ' ' -k 1.2,1.2 -nrk 3,3 facebook.txt
google 110 5000
sohu 100 4500
guge 50 3000
baidu 100 500

# sort -t ' ' -k 1.2r,1.2r -nk 3r,3r  facebook.txt
guge 50 3000
google 110 5000
sohu 100 4500
baidu 100 500
</pre>
由于只对第二个字母进行排序，所以我们使用了```-k 1.2,1.2```的表示方式，表示我们**只**对第二个字母进行排序。（如果你问我使用```-k 1.2```怎么不行？ 当然不行，因为你省略了End部分，这就意味着你将对从第二个字母起到本域最后一个字符为止的字符串进行排序）。对于员工工资进行排序，我们也使用了```-k 3,3```，这是最准确的表述，表示我们只对本域进行排序，因为如果你省略了后面的3，就变成了我们**对第3个域开始到最后一个域位置的内容进行排序**了。







<br />
<br />

**[参看]**

1. [sort命令文件过滤分割与合并](http://man.linuxde.net/sort)

2. [linux sort 命令整理](https://www.jianshu.com/p/c4d159a98dd8)


<br />
<br />
<br />


