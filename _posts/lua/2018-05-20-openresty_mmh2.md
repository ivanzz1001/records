---
layout: post
title: openResty中计算murmurhash
tags:
- lua
categories: lua
description: openresty中计算murmurhash
---


openresty中甚至没有为我们提供一个方便计算hash的函数，这里我们介绍一下如何通过调用nginx内部的ngx_murmur_hash2()来实现此功能。



<!-- more -->

## 1. lua-resty-murmurhash2

其实lua-resty-murmurhash2.lua也只是简单的将nginx的内部函数ngx_murmur_hash2()导出，代码比较简单，我们来看一下：
{% highlight string %}
local ffi      = require "ffi"
local ffi_cast = ffi.cast
local C        = ffi.C
local tonumber = tonumber

ffi.cdef[[
typedef unsigned char u_char;
uint32_t ngx_murmur_hash2(u_char *data, size_t len);
]]

return function(value)
    return tonumber(C.ngx_murmur_hash2(ffi_cast('uint8_t *', value), #value))
end

{% endhighlight %}


### 1.1 调用murmurhash2函数
我们可以通过如下的方式来调用：
{% highlight string %}
local mmh2 = require("resty.murmurhash2")


local str = "hello, world"
local h = mmh2(str)

print("hash(str): ", h)
{% endhighlight %}

>注：如果用到外部的动态链接库，可以将其拷贝到/usr/lib64/、/usr/local/openresty/lualib/ 目录下，或者拷贝到/usr/local/openresty/luajit/lib/ 目录下


<br />
<br />

**[参考]**

1. [murmurhash2官网](https://github.com/bungle/lua-resty-murmurhash2)

2. [在nginx中采用lua对请求的url进行hash取模](https://blog.csdn.net/imlsz/article/details/44750945)

<br />
<br />
<br />





