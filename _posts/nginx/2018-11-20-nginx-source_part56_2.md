---
layout: post
title: nginx配置location
tags:
- nginx
categories: nginx
description: nginx使用基础
---


location指令的作用是根据用户请求的URI来执行不同的应用，也就是根据用户请求的网站URL进行匹配，匹配成功即进行相关的操作。因此本章就来讲述一下nginx的location匹配规则，主要包括如下两个方面：

* location语法

* pcre正则表达式


<br />


<!-- more -->

## 1. Nginx的location语法

<pre>
普通匹配：
location = URI { configuration } # 精确匹配
location ^~ URI { configuration } # 非正则匹配，表示URI以某个常规字符串开头
location [space] URI { configuration} # 前缀匹配, 匹配后，继续更长前缀匹配和正则匹配。
 
 
正则匹配
location ~ URI { configuration } # 区分大小写匹配
location ~* URI { configuration } # 不区分大小写匹配
location !~ URI { configuration } # 区分大小写不匹配
location !~* URI { configuration } # 不区分大小写不匹配


内部重定向
location @name { configuration } # 定义一个location，用于处理内部重定向
</pre>

各个匹配之间的优先级顺序为：
{% highlight string %}
(location =) > (location 完整路径) > (location ^~ 路径) > (location ~,~* 正则顺序) > (location 部分起始路径) > (/)
{% endhighlight %}

### 1.1 location正则表达式书写示例

1） **等号( = )**

表示完全匹配规则才执行操作
<pre>
location = /index{
	[configuration A]
}
</pre>
当URL为```http://{domain-name}/index```时，才会执行配置中操作。

2) **波浪号 ( ~ )**

表示执行正则匹配，但区分大小写
<pre>
location ~ /page/\d{1,2} {
    [ configuration B ]
}
</pre>
URL为```http://{domain-name}/page/1```匹配结尾数字为1~99时，配置生效

3) **波浪号与星号( ~* )**

表示执行正则匹配，但不区分大小写
<pre>
location ~* /\.(jpg|jpeg|gif) {
    [ configuration C ]
}
</pre>
匹配所有URL以```.jpg```、```.jpeg```、```.gif```结尾时， 配置生效

4) **脱字符与波浪号（ ^~ ）**

表示普通字符匹配，前缀匹配有效，配置生效
<pre>
location ^~ /images/ {
	[ cofigurations D ]
}
</pre>
URL为```http://{domain_name}/images/1.gif```时，配置生效。


5) **@符号**

定义一个location，用于处理内部重定向
<pre>
location @error {
    proxy_pass http://error;
}

error_page 404 @error;
</pre>

6) **一个综合示例**

{% highlight string %}
location  = / {
  # 精确匹配 / ，主机名后面不能带任何字符串
  [ configuration A ] 
}

location  / {
  # 因为所有的地址都以 / 开头，所以这条规则将匹配到所有请求
  # 但是正则和最长字符串会优先匹配
  [ configuration B ] 
}

location /documents/ {
  # 匹配任何以 /documents/ 开头的地址，匹配符合以后，还要继续往下搜索
  # 只有后面的正则表达式没有匹配到时，这一条才会采用这一条
  [ configuration C ] 
}

location ~ /documents/Abc {
  # 匹配任何以 /documents/ 开头的地址，匹配符合以后，还要继续往下搜索
  # 只有后面的正则表达式没有匹配到时，这一条才会采用这一条
  [ configuration CC ] 
}

location ^~ /images/ {
  # 匹配任何以 /images/ 开头的地址，匹配符合以后，停止往下搜索正则，采用这一条。
  [ configuration D ] 
}

location ~* \.(gif|jpg|jpeg)$ {
  # 匹配所有以 gif,jpg或jpeg 结尾的请求
  # 然而，所有请求 /images/ 下的图片会被 config D 处理，因为 ^~ 到达不了这一条正则
  [ configuration E ] 
}

location /images/ {
  # 字符匹配到 /images/，继续往下，会发现 ^~ 存在
  [ configuration F ] 
}

location /images/abc {
  # 最长字符匹配到 /images/abc，继续往下，会发现 ^~ 存在
  # F与G的放置顺序是没有关系的
  [ configuration G ] 
}

location ~ /images/abc/ {
  # 只有去掉 config D 才有效：先最长匹配 config G 开头的地址，继续往下搜索，匹配到这一条正则，采用
    [ configuration H ] 
}
{% endhighlight %}

### 2. pcre正则表达式




<br />
<br />

**[参看]**

1. [nginx配置location总结及rewrite规则写法](https://segmentfault.com/a/1190000002797606)

2. [nginx location配置详细解释](http://outofmemory.cn/code-snippet/742/nginx-location-configuration-xiangxi-explain)

3. [Nginx之Location配置详解(Location匹配顺序)](https://blog.csdn.net/RobertoHuang/article/details/70249007)

<br />
<br />
<br />

