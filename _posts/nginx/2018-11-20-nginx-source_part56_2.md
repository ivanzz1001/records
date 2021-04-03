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

>注： 前缀匹配时，Nginx 不对url做编码，因此请求为```/static/20%/aa```，可以被规则 ```^~ /static//aa```匹配到（注意是空格）

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
location ~* /\.(jpg|jpeg|gif)$ {
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

下表包含了元字符的完整列表以及它们在正则表达式上下文中的行为：

{% highlight string %}
 字符                                  描述
--------------------------------------------------------------------------------------
  \              将下一个字符标记为特殊字符、或一个原义字符、或一个向后引用、或一个八进制转义符。
                 例如： 'n'匹配字符"n"； 而'\n'匹配一个换行符； 序列'\\'匹配"\"，而'\('匹配"("。

  ^              匹配输入字符串的开始位置。如果设置了RegExp对象的Multiline属性，^也匹配'\n'或'\r'之后的位置

  $              匹配输入字符串的结束位置。如果设置了RegExp对象的Multiline属性，$也匹配'\n'或'\r'之前的位置

  *              匹配前面的子表达式零次或多次。例如，zo*能匹配"z"或"zoo"。 *等价于{0,}

  +              匹配前面的子表达式一次或多次。例如，'zo+'能匹配"zo"或"zoo"，但不能匹配"z"。+等价于{1,}

  ?              匹配前面的子表达式零次或一次。例如，"do(es)?"可以匹配"do"或者"does"。?等价于{0,1}

 {n}             n是一个非负整数，匹配确定的n次。例如，"o{2}"不能匹配"Bob"中的'o'，但是能匹配"food"中的两个'o'

 {n,}            n是一个非负整数，至少匹配n次。例如，"o{2}"不能匹配"Bob"中的'o'，但能匹配"foooood"中的所有'o'
                 "o{1,}"等价于"o+"；"o{0,}"等价于"o*"

 {n,m}           m和n均为非负整数，其中 n<=m。最少匹配 n 次且最多匹配 m 次。例如，"o{1,3}" 将匹配 "fooooood"
                 中的前三个'o'。"o{0,1}" 等价于"o?"。请注意在逗号和两个数之间不能有空格。

  ?              当该字符紧跟在任何一个其他限制符 (*, +, ?, {n}, {n,}, {n,m}) 后面时，匹配模式是非贪婪的。
                 非贪婪模式尽可能少的匹配所搜索的字符串，而默认的贪婪模式则尽可能多的匹配所搜索的字符串。例如，
                 对于字符串 "oooo"，'o+?' 将匹配单个 "o"，而 'o+' 将匹配所有 'o'。

  .              匹配除换行符（\n、\r）之外的任何单个字符。要匹配包括 '\n' 在内的任何字符，请使用像"(.|\n)"的模式

 (pattern)       匹配 pattern 并获取这一匹配。所获取的匹配可以从产生的 Matches 集合得到，在VBScript 中使用 
                 SubMatches 集合，在JScript 中则使用 $0…$9 属性。要匹配圆括号字符，请使用 '\(' 或 '\)'

 (?:pattern)     匹配 pattern 但不获取匹配结果，也就是说这是一个非获取匹配，不进行存储供以后使用。这在使用 "或" 
                 字符 (|) 来组合一个模式的各个部分是很有用。例如， 'industr(?:y|ies) 就是一个比 'industry|
                 industries' 更简略的表达式。

 (?=pattern)     正向肯定预查（look ahead positive assert），在任何匹配pattern的字符串开始处匹配查找字符串。
                 这是一个非获取匹配，也就是说，该匹配不需要获取供以后使用。例如，"Windows(?=95|98|NT|2000)"能
                 匹配"Windows2000"中的"Windows"，但不能匹配"Windows3.1"中的"Windows"。预查不消耗字符，也就
                 是说，在一个匹配发生后，在最后一次匹配之后立即开始下一次匹配的搜索，而不是从包含预查的字符之后开始。

 (?!pattern)     正向否定预查(negative assert)，在任何不匹配pattern的字符串开始处匹配查找字符串。这是一个非获取
                 匹配，也就是说，该匹配不需要获取供以后使用。例如"Windows(?!95|98|NT|2000)"能匹配"Windows3.1"中
                 的"Windows"，但不能匹配"Windows2000"中的"Windows"。预查不消耗字符，也就是说，在一个匹配发生后，
                 在最后一次匹配之后立即开始下一次匹配的搜索，而不是从包含预查的字符之后开始。

 (?<=pattern)    反向(look behind)肯定预查，与正向肯定预查类似，只是方向相反。例如，"(?<=95|98|NT|2000)Windows"
                 能匹配"2000Windows"中的"Windows"，但不能匹配"3.1Windows"中的"Windows"。

 (?<!pattern)    反向否定预查，与正向否定预查类似，只是方向相反。例如"(?<!95|98|NT|2000)Windows"能匹配"3.1Windows"
                 中的"Windows"，但不能匹配"2000Windows"中的"Windows"。

   x|y           匹配 x 或 y。例如，"z|food"能匹配"z"或"food"。"(z|f)ood"能匹配"zood"或"food"

  [xyz]          字符集合。匹配所包含的任意一个字符。例如，"[abc]"可以匹配"plain"中的'a'

  [^xyz]         负值字符集合，匹配未包含的任意字符。例如"[^abc]"可以匹配"plain"中的'p'、'l'、'i'、'n'

  [a-z]          字符范围。匹配指定范围内的任意字符。例如，'[a-z]' 可以匹配 'a' 到 'z' 范围内的任意小写字母字符

  [^a-z]         负值字符范围。匹配任何不在指定范围内的任意字符。例如，'[^a-z]' 可以匹配任何不在 'a' 到 'z' 范围内
                 的任意字符。

  \b             匹配一个单词边界，也就是指单词和空格间的位置。例如， 'er\b' 可以匹配"never" 中的 'er'，但不能匹配
                 "verb" 中的 'er'。
                
  \B             匹配非单词边界。'er\B' 能匹配 "verb" 中的 'er'，但不能匹配 "never" 中的 'er'。

  \cx            匹配由 x 指明的控制字符。例如， \cM 匹配一个 Control-M 或回车符。x 的值必须为 A-Z 或 a-z 之一。
                 否则，将 c 视为一个原义的 'c' 字符。  

  \d             匹配一个数字字符。等价于 [0-9]。

  \D             匹配一个非数字字符。等价于 [^0-9]。

  \f             匹配一个换页符。等价于 \x0c 和 \cL

  \n             匹配一个换行符。等价于 \x0a 和 \cJ。

  \r             匹配一个回车符。等价于 \x0d 和 \cM

  \s             匹配任何空白字符，包括空格、制表符、换页符等等。等价于 [ \f\n\r\t\v]

  \S             匹配任何非空白字符。等价于 [^ \f\n\r\t\v]

  \t             匹配一个制表符。等价于 \x09 和 \cI

  \v             匹配一个垂直制表符。等价于 \x0b 和 \cK

  \w             匹配字母、数字、下划线。等价于'[A-Za-z0-9_]'

  \W             匹配非字母、数字、下划线。等价于 '[^A-Za-z0-9_]'

  \xn            匹配 n，其中 n 为十六进制转义值。十六进制转义值必须为确定的两个数字长。例如，'\x41' 匹配 "A"。
                 '\x041' 则等价于 '\x04' & "1"。正则表达式中可以使用 ASCII 编码

  \num           匹配 num，其中 num 是一个正整数。对所获取的匹配的引用。例如，'(.)\1' 匹配两个连续的相同字符

  \n             标识一个八进制转义值或一个向后引用。如果 \n 之前至少 n 个获取的子表达式，则 n 为向后引用。
                 否则，如果 n 为八进制数字 (0-7)，则 n 为一个八进制转义值。

  \nm            标识一个八进制转义值或一个向后引用。如果 \nm 之前至少有 nm 个获得子表达式，则 nm 为向后引用。
                 如果 \nm 之前至少有 n 个获取，则 n 为一个后跟文字 m 的向后引用。如果前面的条件都不满足，若 n 和
                 m 均为八进制数字 (0-7)，则 \nm 将匹配八进制转义值 nm。

  \nml           如果 n 为八进制数字 (0-3)，且 m 和 l 均为八进制数字 (0-7)，则匹配八进制转义值 nml

  \un            匹配 n，其中 n 是一个用四个十六进制数字表示的 Unicode 字符。例如， \u00A9 匹配版权符号 (?)
{% endhighlight %}

另外，我们再介绍几个模式修正符(Pattern Modifiers)。模式修正符在忽略大小写、匹配多行中使用特别多，掌握了这一个修正符，往往能解决我们遇到的很多问题

* ```(?i)```: 可同时匹配大小写字母

* ```(?m)```: 将字符串视为多行

* ```(?s)```: 将字符串视为单行，换行符做普通字符看待，使.匹配任何字符

* ```(?X)```: 忽略无效的转义

* ```(?U)```: 匹配到最近的字符串

* ```(?e)```: 将替换的字符串作为表达式使用

例如下面可以匹配以Hel开头的单词（忽略大小写):
<pre>
char src[] = "hello,world, I am looking a job";
char pattern[] = "(?i)\\AHel";
</pre>


### 2.1 实例
{% highlight string %}
location /proxy {
    internal;

    if ($request_uri ~* "(/\w+)+/upload/([^/]+)/([^/]+)(/.*)") {
            set $modified_uri /$3$4;
            proxy_pass http://rgw$modified_uri;
            break;
    }

    if ($request_uri ~* "(/\w+)+/uploadpart/([^/]+)/([^/]+)(/.*)") {
            set $modified_uri /$3$4;
            proxy_pass http://rgw$modified_uri;
            break;
    }

    if ($request_uri ~* "(/\w+)+/download/([^/]+)/([^/]+)(/.*)") {
            proxy_pass http://rgw/$3$4;
            break;
    }
    
    if ($request_uri ~* "/userDownload/([^/]+)/([^/]+)(/.*)") {
            proxy_pass http://rgw/$2$3;
            break;
    }

}

server {
  listen 80 default_server;
  server_name ceph_s3;
  chunked_transfer_encoding on;
  
  location /{
      deny all;
  }

  location ~* /proxy_my_backend{
    internal;

    proxy_pass http://backend$request_uri
  }
  
  location ~* ^/([a-z][^/]+)/(.*)$ {
    # this is the object operation
    set $bucket $1;
    set $object $2;
    echo $bucket;
    echo $2;
       
       
  }
   
  location ~* ^/([a-z][^/]+) {
    # this is the bucket operations(Note: must be placed after object operations)
    set $bucket $1;
    
    echo "bucket operations";
    echo $1;
  }
   
}

{% endhighlight %}

<br />
<br />

**[参看]**

1. [nginx配置location总结及rewrite规则写法](https://segmentfault.com/a/1190000002797606)

2. [nginx location配置详细解释](http://outofmemory.cn/code-snippet/742/nginx-location-configuration-xiangxi-explain)

3. [Nginx之Location配置详解(Location匹配顺序)](https://blog.csdn.net/RobertoHuang/article/details/70249007)

4. [正则表达式](http://www.runoob.com/regexp/regexp-metachar.html)

<br />
<br />
<br />

