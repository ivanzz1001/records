---
layout: post
title: Linux命令行与shell脚本
tags:
- LinuxOps
categories: linuxOps
description: Linux命令行与shell脚本
---


这里我们简单记录一下常见的Linux命令行与Shell脚本。

<!-- more -->

## 1. shell算术运算

这里主要有以下四种方法：

(1) 使用expr外部程式
<pre>
[root@localhost test-src]# r=`expr 2 + 3`         //注意操作数和运算符之间要有空白
[root@localhost test-src]# echo $r
5
[root@localhost test-src]# x=`expr 4 \* 5`
[root@localhost test-src]# echo $x
20
[root@localhost test-src]# y=`expr \( 5 - 3 \) \* 3 + 1`
[root@localhost test-src]# echo $y
7
</pre>

(2) 使用```使用 $(())```
<pre>
[root@localhost test-src]# a=$((1+1))             //这里数与运算符之间没有格式要求
[root@localhost test-src]# echo $a
2
[root@localhost test-src]# b=$(((2+2)*3))
[root@localhost test-src]# echo $b
12
</pre>


(3) 使用```$[]```
<pre>
[root@localhost test-src]# a=$[2+3]              //这里数与运算符之间没有格式要求
[root@localhost test-src]# echo $a
5
[root@localhost test-src]# b=$[ 5 + 6 ]
[root@localhost test-src]# echo $b
11
</pre>

(4) 使用let命令
<pre>
[root@localhost test-src]# n=20
[root@localhost test-src]# let n=n+1
[root@localhost test-src]# echo $n
21
</pre>


## 2. shell字符串处理的掐头去尾法则

```#```表示掐头， ```%```表示去尾.(助记: 键盘的排列上，# 在前面， %在后面)

单个```#```或```%```表示最小匹配，双个```#```或```%```表示最大匹配。也就是说，当匹配有多种方案的时候，选择匹配最大程度还是最小长度。
 
**例1：**
{% highlight string %}
[root@localhost test-src]# workspace=/home/user/test.txt

(下面%/*最小匹配会去除/test.txt)
[root@localhost test-src]# fullpath=${workspace%/*}
[root@localhost test-src]# echo $fullpath
/home/user

(而%%/*会去除最长字符串)
[root@localhost test-src]# base=${workspace%%/*}
[root@localhost test-src]# echo $base            //整个字符串都被去除了

(下面掐掉fullpath头部)
[root@localhost test-src]# name=${workspace#*"${fullpath}"/}
[root@localhost test-src]# echo $name
test.txt

(下面对比一下最小匹配与最大匹配)
[root@localhost test-src]# test1=${workspace#*/}
[root@localhost test-src]# echo $test1
home/user/test.txt
[root@localhost test-src]# test2=${workspace##*/}
[root@localhost test-src]# echo $test2
test.txt
{% endhighlight %}

**例2:**

我们有如下文件people.txt：
{% highlight string %}
{"usertag":{"tag":"支配欲强、有责任感","name":"zhangsan 张三","deptname":"集团职能|Headquarter of Group"},"userno":"1001"}
{"usertag":{"tag":"精力旺盛、不服输","name":"lisi 李四","deptname":"集团职能|Headquarter of Group"},"userno":"1002"}
{"usertag":{"tag":"性情天真、热情奔放","name":"wangwu 王五","deptname":"集团职能|Headquarter of Group"},"userno":"1003"}
{% endhighlight %}
现在要将其userno这一列取出。则可以用如下命令行来完成：
{% highlight string %}
# cat people.txt | while read line; do str1=${line##*\"userno\"}; str2=${str1#*\"}; usrno=${str2%%\"*};echo $usrno;done
1001
1002
1003
{% endhighlight %}
下面我们演示一下各个步骤的打印输出:
{% highlight string %}
# cat people.txt | while read line; do str1=${line##*\"userno\"};echo $str1;done
:"1001"}
:"1002"}
:"1003"}
# cat people.txt | while read line; do str1=${line##*\"userno\"}; str2=${str1#*\"}; echo $str2;done
1001"}
1002"}
1003"}
# cat people.txt | while read line; do str1=${line##*\"userno\"}; str2=${str1#*\"}; usrno=${str2%%\"*}; echo $usrno;done
1001
1002
1003
{% endhighlight %}







<br />
<br />
<br />


