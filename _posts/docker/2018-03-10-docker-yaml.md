---
layout: post
title: yaml语言教程
tags:
- docker
categories: docker
description: yaml语言教程
---


在docker的世界中，有很多地方都会采用yaml文件来作为配置文件。 
<pre>
YAML Ain’t Markup Language” (abbreviated YAML)
</pre>
上面说明YAML并不是一个标记语言，其经常用来写配置文件，非常简洁和强大，远比Json格式方便。本文介绍的```YAML```语法，以[JS-YAML](https://github.com/nodeca/js-yaml)的实现为例。你可以到[在线Demo](http://nodeca.github.io/js-yaml/)验证下面的例子。


<!-- more -->

## 1. 简介

YAML语言的设计目标，就是方便人类读写。它实质上是一种通用的数据串行化格式。它的基本语法规则如下：

* 大小写敏感

* 使用缩进表示层级关系

* 缩进时不允许使用tab键，只允许使用空格

* 缩进的空格数目不重要，只要相同层级的元素左侧对齐即可

```#```表示注释，从这个字符一直到行尾，都会被解析器忽略。

YAML支持的数据结构有三种：

* **对象**: 键值对的集合，又称为映射(mapping)/哈希(hashes)/字典(dictionary)

* **数组**: 一组按次序排列的值，又称为序列(sequence)/列表(list)

* **纯量(scalars)**: 单个的、不可再分的值

以下分别介绍这三种数据结构。

## 2. 对象

对象是一组键值对，使用冒号结构表示：
<pre>
animal: pets
</pre>
转换为JavaScript如下：
<pre>
{animal: 'pets'}
</pre>

YAML也允许另一种写法，将所有键值对写成一个行内对象：
<pre>
hash: { name: Steve, foo: bar }
</pre>
转换为JavaScript如下：
<pre>
{hash: {name: 'Steve', foo: 'bar'}}
</pre>

说明，上面写法冒号```:```后壁必须要跟一个空格

## 3. 数组
一组以```-```开头的行，构成一个数组：
<pre>
- Cat
- Dog
- Goldfish
</pre>
转换成JavaScript如下：
<pre>
[ 'Cat', 'Dog', 'Goldfish' ]
</pre>
如果数据结构的子成员是一个数组，则可以在该项下缩进一个空格(```缩进多个空格也可以，只不过子项要对齐```）：
<pre>
-
 - Cat
 - Dog
 - Goldfish
</pre>
转换成JavaScript如下：
<pre>
[ [ 'Cat', 'Dog', 'Goldfish' ] ]
</pre>

数组也可以采用行内表示法：
<pre>
animal: [ 'Cat', 'Dog' ]
</pre>
转换成JavaScript如下：
<pre>
{ animal: [ 'Cat', 'Dog' ] }
</pre>


## 4. 复合结构
对象和数组可以结合使用，形成复合结构。例如：
<pre>
languages:
 - Ruby
 - Perl
 - Python
Websites:
 YAML: yaml.org
 Ruby: ruby-lang.org
 Python: python.org
 Perl: use.perl.org
</pre>
转换成JavaScript如下：
<pre>
{ languages: [ 'Ruby', 'Perl', 'Python' ],
  Websites: 
   { YAML: 'yaml.org',
     Ruby: 'ruby-lang.org',
     Python: 'python.org',
     Perl: 'use.perl.org' } }
</pre>

## 5. 纯量
纯量是最基本的、不可再分的值。以下数据类型都属于JavaScript纯量。

* 字符串

* 布尔值

* 整数

* 浮点数

* Null

* 时间

* 日期

1) **数值直接以字面量的形式表示**

例如：
<pre>
number: 12.30
</pre>
转换成JavaScript如下：
<pre>
{ number: 12.30 }
</pre> 

2) **布尔值用```true```和```false```表示**

例如：
<pre>
isSet: true
</pre>
转换成JavaScript如下：
<pre>
{ isSet: true }
</pre>

3) **null用```~```来表示**

例如：
<pre>
parent: ~
</pre>
转换成JavaScript如下：
<pre>
{ parent: null }
</pre>

4) **时间采用 ISO8601 格式**

例如：
<pre>
iso8601: 2001-12-14t21:59:43.10-05:00 
</pre>
转换为JavaScript如下：
<pre>
{ ios8601: new Date('2001-12-14t21:59:43.10-05:00') }
</pre>

5) **日期采用复合ISO8601格式的年、月、日表示**

例如：
<pre>
date: 1976-07-31
</pre>
转换为JavaScript如下：
<pre>
{ date: new Date('1976-07-31') }
</pre>

6) **YAML强制转换数据类型**

YAML允许使用两个感叹号(```!```)，强制转换数据类型。例如：
<pre>
e: !!str 123
f: !!str true
</pre>
转换为JavaScript如下：
<pre>
{ e: '123', f: 'true' }
</pre>

## 6. 字符串

字符串是最常见，也是最复杂的一种数据类型。字符串默认不使用引号(```''```表示）。例如：
<pre>
str: 这是一行字符串
</pre>
转换为JavaScript如下：
<pre>
{ str: '这是一行字符串' }
</pre>


1) **如果字符串之中包含空格或特殊字符，需要放在引号之中**

例如：
<pre>
str: '内容： 字符串'
</pre>
转换成JavaScript如下：
<pre>
{ str: '内容: 字符串' }
</pre>

2) **单引号和双引号都可以使用，双引号不会对特殊字符转义**

例如：
<pre>
s1: '内容\n字符串'
s2: "内容\n字符串"
</pre>
转换为JavaScript如下：
<pre>
{ s1: '内容\\n字符串', s2: '内容\n字符串' }
</pre>

3) **单引号之中如果还有单引号，必须连续使用两个单引号转义**

例如：
<pre>
str: 'labor''s day'
</pre>
转换为JavaScript如下：
<pre>
{ str: 'labor\'s day' }
</pre>

4) **多行字符串**

字符串可以写成多行，从第二行开始，必须有一个单空格缩进(```也可以是多个空格```)。换行符会被转为空格。例如：
<pre>
str: 这是一段
  多行
  字符串
</pre>
转换为JavaScript如下：
<pre>
{ str: '这是一段 多行 字符串' }
</pre>






<br />
<br />

**[参考]**

1. [YAML 语言教程](http://www.ruanyifeng.com/blog/2016/07/yaml.html)

2. [YAML官网](http://www.yaml.org/)

3. [YAML-JS在线转换](http://nodeca.github.io/js-yaml/)

<br />
<br />
<br />

