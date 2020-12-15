---
layout: post
title: openresty中redis连接池的实现
tags:
- lua
categories: lua
description: lua开发
---



本文介绍一下openresty中redis连接池的实现


<!-- more -->

## 1. lua-resty-redis
我们可以到[lua-resty-redis官网](https://github.com/openresty/lua-resty-redis)下载对应的lua模块

## 2. 实现参考
{% highlight string %}
local redis_c = require "resty.redis"


local ok, new_tab = pcall(require, "table.new")
if not ok or type(new_tab) ~= "function" then
  new_tab = function (narr, nrec) return {} end
end


local _M = new_tab(0, 155)
_M._VERSION = '0.01'

local commands = {
    "append",            "auth",              "bgrewriteaof",
    "bgsave",            "bitcount",          "bitop",
    "blpop",             "brpop",
    "brpoplpush",        "client",            "config",
    "dbsize",
    "debug",             "decr",              "decrby",
    "del",               "discard",           "dump",
    "echo",
    "eval",              "exec",              "exists",
    "expire",            "expireat",          "flushall",
    "flushdb",           "get",               "getbit",
    "getrange",          "getset",            "hdel",
    "hexists",           "hget",              "hgetall",
    "hincrby",           "hincrbyfloat",      "hkeys",
    "hlen",
    "hmget",              "hmset",      "hscan",
    "hset",
    "hsetnx",            "hvals",             "incr",
    "incrby",            "incrbyfloat",       "info",
    "keys",
    "lastsave",          "lindex",            "linsert",
    "llen",              "lpop",              "lpush",
    "lpushx",            "lrange",            "lrem",
    "lset",              "ltrim",             "mget",
    "migrate",
    "monitor",           "move",              "mset",
    "msetnx",            "multi",             "object",
    "persist",           "pexpire",           "pexpireat",
    "ping",              "psetex",            "psubscribe",
    "pttl",
    "publish",      --[[ "punsubscribe", ]]   "pubsub",
    "quit",
    "randomkey",         "rename",            "renamenx",
    "restore",
    "rpop",              "rpoplpush",         "rpush",
    "rpushx",            "sadd",              "save",
    "scan",              "scard",             "script",
    "sdiff",             "sdiffstore",
    "select",            "set",               "setbit",
    "setex",             "setnx",             "setrange",
    "shutdown",          "sinter",            "sinterstore",
    "sismember",         "slaveof",           "slowlog",
    "smembers",          "smove",             "sort",
    "spop",              "srandmember",       "srem",
    "sscan",
    "strlen",       --[[ "subscribe",  ]]     "sunion",
    "sunionstore",       "sync",              "time",
    "ttl",
    "type",         --[[ "unsubscribe", ]]    "unwatch",
    "watch",             "zadd",              "zcard",
    "zcount",            "zincrby",           "zinterstore",
    "zrange",            "zrangebyscore",     "zrank",
    "zrem",              "zremrangebyrank",   "zremrangebyscore",
    "zrevrange",         "zrevrangebyscore",  "zrevrank",
    "zscan",
    "zscore",            "zunionstore",       "evalsha"
}

local mt = { __index = _M }



local function is_redis_null(res)   
  if type(res) == "table" then
    for k,v in pairs(res) do
      if v ~= ngx.null then
        return false
      end
    end
    return true
  elseif res == ngx.null then
    return true
  elseif res == nil then
    return true
  end
  
  return false
end  


--[[
  this function doesn't really close the redis, but put it into the pool
--]]
function _M.close_redis(self, redis)
  if not redis then
    return 
  end  
  
    --[[
      release the redis pool
    --]]
    local pool_max_idle_time = self.pool_max_idle_time           
    local pool_size = self.pool_size 
    
    local ok, err = redis:set_keepalive(pool_max_idle_time, pool_size)
    if not ok then
      ngx.log(ngx.ERR, "set keepalive error: ", err)
    end   
  
end  



function _M.connect_mod(self, redis)
  
  -- set operation timeout
  redis:set_timeout(self.timeout)
  
  ngx.log(ngx.ERR, "ip: ", self.ip, " port: ", self.port)
  
  local ok, err = redis:connect(self.ip, self.port)
  if not ok then
    ngx.log(ngx.ERR, "connect to redis error: ", err)
    return self:close_redis(redis)
  end  
  
  
  if self.password then
    local count, err = redis:get_reused_times()
    if count == 0 then                             -- the new connections, need auth
      ok, err = redis:auth(self.password)
      
      if not ok then
        ngx.log(ngx.ERR, "failed to auth: ", err)
        return
      end
      
    elseif err then                                -- failed to get times
      ngx.log(ngx.ERR, "failed to get reused times: ", err)
      return
    end  
    
  end
  
  
  return ok, err 
end   


function _M.init_pipeline(self)
  self._reqs = {}
end


function _M.commit_pipeline(self)
  local reqs = self._reqs
  
  if nil == reqs or 0 == #reqs then
    return {}, "no pipeline"
    
  else
    self._reqs = nil 
    
  end  
  
  local redis, err = redis_c:new()
  if not redis then
    return nil, err 
  end    
  
  local ok, err = self:connect_mod(redis)
  if not ok then 
    return {}, err
  end  
  
  -- redis:select(self.dbidx)
  
  redis:init_pipeline()
  
  for _, vals in ipairs(reqs) do
    local fun = redis[vals[1]]
    table.remove(vals , 1)
    fun(redis, unpack(vals))
  end   
  
  local results, err = redis:commit_pipeline()
  if not results or err then
    return {}, err
  end
  if is_redis_null(results) then
    results = {}
    ngx.log(ngx.WARN, "result is null")
  end
  
  -- table.remove (results , 1)
  
  self:close_redis(redis)
  
  
  --[[
      should we set it to nil here??
  --]]
  for i, value in ipairs(results) do
    
    if is_redis_null(value) then
      results[i] = nil 
    end   
    
  end
  
  return results, err
 end 
 
 
 
function _M.subscribe(self, channel)
  local redis, err = redis_c:new()
  if not redis then
    return nil, err 
  end    
  
  local ok, err = self:connect_mod(redis)
  if not ok then 
    return {}, err
  end 
   
   
  local res, err = redis:subscribe(channel)
  if not res then
    return nil, err
  end   
  
  local function do_read_func(do_read)
      if do_read == nil or do_read == true then
        res, err = redis:read_reply()
        
        if not res then
          return nil, err 
        end 
        
        return res
      end 
      
      redis:unsubscribe(channel)
      self:close_redis(redis)
      return 
    end
    
  return do_read_func 
 end  
 
 
local function do_command(self, cmd, ...)
  if self._reqs then
    table.insert(self._reqs, {cmd, ...})               -- use the pipeline
    
    return 
  end   
    
  local redis, err = redis_c:new()
  if not redis then
    return nil, err
  end   
  
  local ok, err = self:connect_mod(redis)
  if not ok or err then
    return nil, err
  end   
  
  -- redis:select(self.dbidx)
  
  local fun = redis[cmd]
  local result, err = fun(redis, ...)
  
  if not result or err then
    -- ngx.log(ngx.ERR, "redis operater result: ", result, " err: ", err)
    
    return nil, err 
  end   
  
  if is_redis_null(result) then 
    result = nil 
  end   
  
  self:close_redis(redis)
  
  return result, err 
end 



--[[
  add the commands to _M
--]]
for i = 1, #commands do
  local cmd = commands[i]
  
  _M[cmd] = function(self, ...)
      return do_command(self, cmd, ...)
    end 
  
end   



function _M.new(self, opts)
  opts = opts or {}
  
  local ip = opts.ip or '127.0.0.1'
  local port = opts.port or 6379
  local password = (opts.password ~= "" and opts.password) or nil 
  local dbidx = opts.dbidx or 0
  local poolsize = opts.poolsize or 20
  local timeout = opts.timeout or 1000
  local pool_max_idle_time = opts.pool_max_idle_time or 60000
  

  return setmetatable(
    {
      ip = ip,
      port = port,
      password = password,
      dbidx = dbidx,
      poolsize = poolsize,
      timeout = timeout,
      pool_max_idle_time = pool_max_idle_time 
    },
    mt)
  
end   

return _M 
 
{% endhighlight %}

>注： 对于hmget等操作，返回的结果是table







<br />
<br />

参看:

1. [lua-resty-redis官网](https://github.com/openresty/lua-resty-redis)

2. [openresty best practise](https://www.showapi.com/book/view/2123/27)

3. [7openresty中封装redis操作](http://t.zoukankan.com/reblue520-p-11434632.html)

4. [访问有授权验证的Redis](https://github.com/moonbingbing/openresty-best-practices/blob/master/redis/auth_connect.md)

5. [Openresty+Lua Redis连接池实现](https://www.jianshu.com/p/e3cf5b92c370)

<br />
<br />
<br />

