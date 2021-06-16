---
layout: post
title: openresty中mysql连接池的实现
tags:
- lua
categories: lua
description: lua开发
---



本文介绍一下openresty中mysql连接池的实现


<!-- more -->

## 1. lua-resty-redis
我们可以到[lua-resty-mysql官网](https://github.com/openresty/lua-resty-mysql)下载对应的lua模块

## 2. 实现参考
{% highlight string %}
local mysql_c = require "resty.mysql"

local ok, new_tab = pcall(require, "table.new")
if not ok or type(new_tab) ~= "function" then
  new_tab = function (narr, nrec) return {} end
end

local _M = new_tab(0, 20)
_M._VERSION = '0.01'

local mt = {__index = _M}



function _M.connect_mod(self, client)
  
  client:set_timeout(self.timeout)
  local options = {
      host = self.host,
      port = self.port,
      database = self.database,
      user = self.username,
      password = self.password,
      charset = self.charset,
      pool_size = self.poolsize
    }
    
    
  local ok, err, errcode, sqlstate = client:connect(options)  
  if not ok then
    
    ngx.log(ngx.ERR, "failed to connect: ", err, ": ", errcode, "  ", sqlstate)
    
    return false, err 
  end  
  
  return true
end   


--[[

   this function do not really close the mysql connection,
   but put it into the pool
--]]
function _M.close_connection(self, client)
  if not client then
    return 
  end   
  
  local pool_max_idle_time = self.pool_max_idle_time           
  local pool_size = self.poolsize 
  
  local ok, err = client:set_keepalive(pool_max_idle_time, pool_size)
  if not ok then
    
    ngx.log(ngx.ERR, "set keeepalive err: ", err)
  end   
end   

 
function _M.query(self, sql)
  local client, err = mysql_c:new()
  if not client then
    ngx.log(ngx.ERR, "create mysql client failure: ", err)
    
    return false, nil, err
  end   
  
  local ok, err = self:connect_mod(client)
  if not ok then
    self:close_connection(client)
    return false, nil, err 
  end   
  
  
  -- the following just for debug
  local reusedtimes, err = client:get_reused_times()
  if not reusedtimes then
    ngx.log(ngx.ERR, "get reusedtimes failure: ", err)
    return 
  end   
  
  ngx.log(ngx.ERR, "reusedtimes: ", reusedtimes)

  
  local result, errmsg, errno, sqlstate = client:query(sql)
  self:close_connection(client)
  
  if not result then
    errmsg = "mysql query failed: " .. (errno or "nil") .. (errmsg or "nil")
    
    ngx.log(ngx.ERR, errmsg, "sqlstate: ", sqlstate)
    return false, nil, errmsg
  end   
  
  return true, result
end   
 
 
function _M.new(self, opts)
  opts = opts or {}
  
  local host = opts.host or '127.0.0.1'
  local port = opts.port or 3306
  local username = opts.username or 'ngx_test_usr'
  local password = opts.password or 'ngx_test_pwd'
  local database = opts.database or 'ngx_test_db'
  local poolsize = opts.poolsize or 10
  local charset = opts.charset or "_default"
  local timeout = opts.timeout or 1000
  local pool_max_idle_time = opts.pool_max_idle_time or 900000               -- 900000ms = 15min
  
  if charset == "" then
    charset = "_default"
  end   
  
  
   return setmetatable(
    {
      host = host,
      port = port,
      username = username,
      password = password,
      database = database,
      poolsize = poolsize,
      charset = charset,
      timeout = timeout,
      pool_max_idle_time = pool_max_idle_time
    },
    mt)
  
end



return _M
{% endhighlight %}









<br />
<br />

参看:

1. [lua-resty-mysql官网](https://github.com/openresty/lua-resty-mysql)

2. [OpenResty连接Mysql](https://www.cnblogs.com/forezp/p/9852101.html)

3. [mysql,redis 项目中的应用](https://www.cnblogs.com/pigzhu/p/3813019.html)

4. [Lua Mysql连接池](https://www.iteye.com/blog/leon1509-2401612)

<br />
<br />
<br />

