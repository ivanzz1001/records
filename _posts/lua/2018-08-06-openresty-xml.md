---
layout: post
title: LuaXML的安装及使用
tags:
- lua
categories: lua
description: lua开发
---



本文介绍一下openresty中LuaXML的安装及使用


<!-- more -->

## 1. LuaXML
我们可以到[LuaXML官网](https://github.com/LuaDist/luaxml)下载对应的lua模块
<pre>
# mkdir luaxml-inst && cd luaxml-inst
# git clone https://github.com/LuaDist/luaxml.git 
</pre>

### 1.1 编译LuaXML

首先修改编译脚本Makefile:
{% highlight string %}
# 2009-03-16 by gf

# generic compiler and linker settings:
CC     = gcc
INCDIR = -I/usr/local/openresty/luajit/include/luajit-2.1
LIB    = 
LIBDIR =  -L. -L/usr/local/openresty/luajit/lib
CFLAGS = -Os -Wall -c #-g

# generic platform specific rules:
ARCH            = $(shell uname -s)
ifeq ($(ARCH),Linux)
  CFLAGS += -fPIC
  LFLAGS =  -fPIC -shared
  LIBS          = $(LIBDIR) $(LIB) -lluajit-5.1 -ldl
  EXESUFFIX =
  SHLIBSUFFIX = .so

else  
  ifeq ($(ARCH),Darwin) # MacOSX
    LFLAGS = -bundle 
    LIBS          = $(LIBDIR) -L/usr/local/lib $(LIB) -lluajit-5.1
    EXESUFFIX = .app
    SHLIBSUFFIX = .so
    
  else  # windows, MinGW
    LFLAGS =  -shared
    LIBS          = $(LIBDIR) $(LIB) -llua51 -mconsole -s
    EXESUFFIX = .exe
    SHLIBSUFFIX = .dll

  endif
endif

# project specific targets:
all:  LuaXML_lib$(SHLIBSUFFIX)

# project specific link rules:
LuaXML_lib$(SHLIBSUFFIX): LuaXML_lib.o
    $(CC) -o $@ $(LFLAGS) $^ $(LIBS) 

# project specific dependencies:
LuaXML_lib.o:  LuaXML_lib.c

# generic rules and targets:
.c.o:
    $(CC) $(CFLAGS) $(INCDIR) -c $<
clean:
    rm -f *.o *~ LuaXML_lib.so LuaXML_lib.dll
{% endhighlight %}
所修改的地方主要有如下：
<pre>
INCDIR = -I/usr/local/openresty/luajit/include/luajit-2.1
LIBDIR =  -L. -L/usr/local/openresty/luajit/lib
LIBS = $(LIBDIR) $(LIB) -lluajit-5.1 -ldl
LIBS = $(LIBDIR) -L/usr/local/lib $(LIB) -lluajit-5.1
LIBS = $(LIBDIR) $(LIB) -llua51 -mconsole -s
</pre>

执行如下命令进行编译：
<pre>
# make clean && make
</pre>

接着将编译出来的```LuaXML_lib.so```与```LuaXML.lua```拷贝到openresty的lualib目录：
<pre>
# cp LuaXML_lib.so /usr/local/openresty/lualib/
# cp LuaXML.lua /usr/local/openresty/lualib/
</pre>


### 1.2 测试验证
1） **测试示例1**
{% highlight string %}
require("LuaXML")
xml = require("xml")

local function parse_xml_1()
  local xmldata = [[
<?xml version="1.0" encoding="UTF-8"?>
<AccessControlPolicy xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Owner>
    <ID>test_user</ID>
    <DisplayName>app_test_user</DisplayName>
  </Owner>
  <AccessControlList>
    <Grant>
      <Grantee xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:type="Group">
        <URI>http://acs.amazonaws.com/groups/global/AllUsers</URI>
      </Grantee>
      <Permission>READ</Permission>
    </Grant>
    <Grant>
      <Grantee xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:type="CanonicalUser">
         <ID>test_user</ID>
         <DisplayName>app_test_user</DisplayName>
      </Grantee>
      <Permission>FULL_CONTROL</Permission>
    </Grant>
  </AccessControlList>
</AccessControlPolicy> 
  ]]
  
  local xres = xml.eval(xmldata)
  if not xres then
     ngx.status = ngx.HTTP_INTERNAL_SERVER_ERROR
     
     ngx.say("parse xml failure:", errmsg)
     ngx.exit(ngx.HTTP_OK)
     return
  end   
  
  ngx.say("parse xml success")
  
  local xowner = xres:find("Owner")
  local xacl = xres:find("AccessControlList")
  
  if xowner then
    local xusrid = xowner:find("ID")
    local xdisplayname = xowner:find("DisplayName")
    
    if xusrid and xdisplayname then
      ngx.say("user_id: ", xusrid[1], " displayname:", xdisplayname[1])
    end   
  end
  
  if xacl then
    local xgrant0 = xacl[1]
    local xgrant1 = xacl[2]
    if xgrant0 and xgrant1 then
      ngx.say("uri: ", xgrant0[1][1][1], " permission:", xgrant0[2][1])
    
      ngx.say("uri: ", xgrant1[1][1][1], " permission:", xgrant1[2][1])
    end
    
  end   
  
  ngx.exit(ngx.HTTP_OK)
  
  return 
end   
{% endhighlight %}

![lua-xml-1](https://ivanzz1001.github.io/records/assets/img/lua/lua-xml-1.jpg)

注： 如果在开头不加如下
<pre>
xml = require("xml")
</pre>
而直接使用xml的话，则第二次请求会报错
<pre>
attempt to index global 'xml'(a nil value)
</pre>


2) **测试示例2**

{% highlight string %}
require("LuaXML")
xml = require("xml")

local function parse_xml_1()
  local xmldata = [[
<?xml version="1.0" encoding="GB2312"?>
<computer>
    <id value="1" name="thinkpad">
        <size>17</size>
        <height>250</height>
        <factory name="xxxx"/>
    </id>
    <id value="2" name="thinkpad">
        <size>12</size>
        <height>150</height>
    </id>
    <id value="3" name="thinkpad">
        <size>14</size>
        <height>200</height>
    </id>
</computer>
  ]]

local xfile = xml.load(filePath)
--xfile[0]对应的就是"computer"

print(xfile[1][0], xfile[1]["value"], xfile[1]["name"])                -- id 1 thinkpad
print(xfile[1][1][0], xfile[1][1][1])                                  -- size 17
print(xfile[1][2][0], xfile[1][2][1])                                  -- height 250


end
{% endhighlight %}

![lua-xml-2](https://ivanzz1001.github.io/records/assets/img/lua/lua-xml-2.jpg)


<br />
<br />

参看:

1. [openresty安装及使用LuaXml](https://segmentfault.com/a/1190000008426022)

2. [luaXml库的使用方法](https://blog.csdn.net/shaonian_wuya/article/details/16120143)

3. [lua-cjson官网](https://github.com/openresty/lua-cjson)


<br />
<br />
<br />

