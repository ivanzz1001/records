---
layout: post
title: openresty中向kafka推送数据
tags:
- lua
categories: lua
description: lua开发
---



本文介绍一下openresty中mysql连接池的实现


<!-- more -->

## 1. lua-resty-redis
我们可以到[lua-resty-kafka官网](https://github.com/doujiang24/lua-resty-kafka)下载对应的lua模块

## 2. 实现参考
{% highlight string %}
local producer = require "resty.kafka.producer"
  

local ok, new_tab = pcall(require, "table.new")
if not ok or type(new_tab) ~= "function" then
  new_tab = function (narr, nrec) return {} end
end

local _M = new_tab(0, 20)
_M._VERSION = '0.01'

local mt = {__index = _M}



function _M.send_kafka_async(self, topic, key, message)
  if not _M[self.cluster_name] then 
    
    local p = producer:new(self.broker_list, self.producer_config, self.cluster_name)
    if not p then
      ngx.log(ngx.ERR, "create kafka producer '", self.cluster_name, "' failure")
      
      return false, "create kafka producer failure"
    end   
    
    _M[self.cluster_name] = p
  end   
  
  
  return _M[self.cluster_name]:send(topic, key, message)
  
end 


function _M.send_kafka_sync(self, topic, key, message)

  local p = producer:new(self.broker_list, self.producer_config, self.cluster_name)
  if not p then 
    ngx.log(ngx.ERR, "create kafka producer failure")
    
    return false, "create kafka producer failure"
    
  end 
  
  
  return p:send(topic, key, message)
  
end 


function _M.send_kafka(self, topic, key, message)
  if self.producer_config and self.producer_config.producer_type == "async" then
    return self:send_kafka_async(topic,key, message)
  end
  
 return  self:send_kafka_sync(topic, key, message)
end   


function _M.new(self, broker_list, producer_config, cluster_name)
  broker_list = broker_list or {}
  producer_config = producer_config or {}
  cluster_name = cluster_name or "default_cluster"
  

  return setmetatable(
    {
      broker_list = broker_list,
      producer_config = producer_config,
      cluster_name = cluster_name
    },
    mt)
  
end   

return _M
{% endhighlight %}









<br />
<br />

参看:

1. [lua-resty-kafka官网](https://github.com/doujiang24/lua-resty-kafka)

2. [openresty github官网](https://github.com/openresty)

3. [lua-cjson github官网](https://github.com/openresty/lua-cjson/)

4. [OpenResty + Lua + Kafka 实现日志收集系统](https://blog.csdn.net/weixin_34221036/article/details/92246913)

5. [OpenResty + Lua + Kafka 实现日志收集系统以及部署过程中遇到的坑](https://www.cnblogs.com/gxyandwmm/p/11298912.html)

6. [Openresty+Lua+Kafka实现日志实时采集](https://www.cnblogs.com/linzepeng/p/12643158.html)

7. [openresty+lua实现实时写kafka](https://blog.csdn.net/qq_29497387/article/details/101290378)

<br />
<br />
<br />

