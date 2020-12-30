---
layout: post
title: go-redis读写锁实现参考
tags:
- go-language
categories: go-language
description: go proxy的设置
---


本文参考开源的redisson，简要的实现了一个go版本的redis读写锁。

<!-- more -->


## 1. go-redis读写锁

### 1.1 ReadWriteLock入口

参看如下代码实现(RReadWriteLock.go):
{% highlight string %}
package goredisson

import (
	"strings"
)

const(
	lockWatchdogTimeout = 10 * 60 * 1000      //10min
)


type RedissonReadWriteLock struct{
	name string
}

/*
 * Description: Create readwrite lock
 *
 * Note: for support redis-cluster, please reference
 *   1) https://redis.io/commands/cluster-keyslot
 *   2) https://blog.csdn.net/xixingzhe2/article/details/86167859
 */
func GetReadWriteLock(name string)(* RedissonReadWriteLock){
	rRWLock := new(RedissonReadWriteLock)

	rRWLock.name = name
	return rRWLock
}

/*
 * Description: get a read lock
 * id: the indicator
 *     (we could have many read locks in a 'rwlock', and most probably we can use an indicator to distinguwish them)
 */
func (lock* RedissonReadWriteLock)ReadLock(id string)(*RReadLock){
	if id == ""{
		id = "admin"
	}
	return createReadLock(lock.name, id)
}


/*
 * Description: get a write lock
 * id: the indicator
 */
func (lock *RedissonReadWriteLock)WriteLock(id string)(*RWriteLock){
	if id == ""{
		id = "admin"
	}
	return createWriteLock(lock.name, id)
}




func prefixName(prefix string, name string)(string){
	if strings.Contains(name, "{"){
		return prefix + ":" + name
	}

	return prefix + ":{" + name + "}"
}

func suffixName(name string, suffix string)(string){
	if strings.Contains(name, "{"){
		return name + ":" + suffix
	}

	return "{" + name + "}:" + suffix
}
{% endhighlight %}

### 1.2 读锁的实现
参看如下实现(RedissonReadLock.go):
{% highlight string %}
package goredisson

import(
	"strconv"
	"errors"
	"strings"
	goredis "github.com/go-redis/redis"
)

type RReadLock struct{
	name string
	id string
}

func createReadLock(name string, id string)(*RReadLock){
	rLock := new(RReadLock)

	rLock.name = name
	rLock.id = id

	return rLock
}


func (rlock *RReadLock)getName()string{
	return rlock.name
}

func (rlock *RReadLock)getIndicator()string{
	return rlock.id
}

func (rlock *RReadLock)getReadWriteTimeoutNamePrefix()string{
	return suffixName(rlock.name, rlock.id) + ":rwlock_timeout"
}

/*
 * Description: try to get a lock
 * leaseTime: the lock time (unit: milliseconds)
 */
func (rlock *RReadLock)doLock(leaseTime int64)(error){
	if leaseTime == -1{
		leaseTime = lockWatchdogTimeout
	}

	script := `
		local mode = redis.call('hget', KEYS[1], 'mode');
		if (mode == false) then 
			redis.call('hset', KEYS[1], 'mode', 'read');
			redis.call('hset', KEYS[1], ARGV[2], 1);
			redis.call('set', KEYS[2] .. ':1', 1);
			redis.call('pexpire', KEYS[2] .. ':1',ARGV[1]);
			redis.call('pexpire', KEYS[1], ARGV[1]);
			return nil;
		 end;
		 if (mode == 'read') then 
		 	local ind = redis.call('hincrby', KEYS[1], ARGV[2], 1);
		 	local key = KEYS[2] .. ':' .. ind;
		 	redis.call('set', key, 1);
		 	redis.call('pexpire', key, ARGV[1]);
		 	local remainTime = redis.call('pttl', KEYS[1]);
		 	redis.call('pexpire', KEYS[1], math.max(remainTime, ARGV[1]));
		 	return nil;
		 end;
		 return redis.call('pttl', KEYS[1]);
	`

	keys := []string{
		rlock.getName(),
		rlock.getReadWriteTimeoutNamePrefix(),
	}
	args := []string{
		strconv.FormatInt(leaseTime, 10),
		rlock.getIndicator(),
	}
	repl, err := Redis_handle.Eval(script, keys, args)

	if err != nil{
		return err
	}

	_, err = repl.StringValue()

	return err
}

func (rlock *RReadLock)doUnlock()error{
	timeoutNamePrefix := rlock.getReadWriteTimeoutNamePrefix()
	keyPrefix := strings.Split(timeoutNamePrefix, ":"+rlock.getIndicator())[0]

	/*
	 *  Note: here not check the mode type, it seems not a problem
	 */
	script := `
		local mode = redis.call('hget', KEYS[1], 'mode');
		if (mode == false) then 
			return 1;
		end;

		local lockExists = redis.call('hexists', KEYS[1], ARGV[1]);
		if (lockExists == 0) then
			return nil;
		end;
		
		local counter = redis.call('hincrby', KEYS[1], ARGV[1], -1);
		if (counter == 0) then
			redis.call('hdel', KEYS[1], ARGV[1]);
		end;
		redis.call('del', KEYS[2] .. ':' .. (counter+1));

		if (redis.call('hlen', KEYS[1]) > 1) then
			local maxRemainTime = -3;
			local keys = redis.call('hkeys', KEYS[1]);
			for n, key in ipairs(keys) do
				counter = tonumber(redis.call('hget', KEYS[1], key));
				if type(counter) == 'number' then
					for i=counter, 1, -1 do
						local remainTime = redis.call('pttl', KEYS[3] .. ':' .. key .. ':rwlock_timeout:' .. i);
						maxRemainTime = math.max(remainTime, maxRemainTime);
					end;
				end;
			end;

			if (maxRemainTime > 0) then
				redis.call('pexpire', KEYS[1], maxRemainTime);
				return 0;
			end;

			if (mode == 'write') then
				return 0;
			end;
		end;
		
		redis.call('del', KEYS[1]);
		return 1;
	`

	keys := []string{
		rlock.getName(),
		rlock.getReadWriteTimeoutNamePrefix(),
		keyPrefix,
	}
	args := []string{
		rlock.getIndicator(),
	}


	repl, err := Redis_handle.Eval(script, keys, args)
	if err != nil{
		return err
	}
	if repl.Type == goredis.ErrorReply{
		return errors.New(repl.Error)
	}

	if repl.Type == goredis.IntegerReply || repl.Type == goredis.BulkReply{
		return nil
	}

	return errors.New("invalid reply type")
}

func (rlock *RReadLock)Lock()(error){
	return rlock.doLock(-1)
}

func (rlock *RReadLock)Unlock()(error){
	return rlock.doUnlock()
}
{% endhighlight %}

### 1.3 写锁的实现

参看如下实现(RedissonWriteLock.go):
{% highlight string %}
package goredisson

import (
	"strconv"
	"errors"
	goredis "github.com/go-redis/redis"
)

type RWriteLock struct{
	name string
	id string
}

func createWriteLock(name string, id string)(*RWriteLock){
	wLock := new(RWriteLock)

	wLock.name = name
	wLock.id = id

	return wLock
}

func (wlock *RWriteLock)getName()(string){
	return wlock.name
}
func (wlock *RWriteLock)getIndicator()(string){
	return wlock.id
}

func (wlock *RWriteLock)getLockName()(string){
	return wlock.name + ":write"
}

func (wlock *RWriteLock)doLock(leaseTime int64)(error){
	if leaseTime == -1{
		leaseTime = lockWatchdogTimeout
	}

	script := `
		local mode = redis.call('hget', KEYS[1], 'mode');
		if (mode == false) then
			redis.call('hset', KEYS[1], 'mode', 'write');
			redis.call('hset', KEYS[1], ARGV[2], 1);
			redis.call('pexpire', KEYS[1], ARGV[1]);
			return nil;
		end;

		return redis.call('pttl', KEYS[1]);
	`
	keys := []string{
		wlock.getName(),
	}
	args := []string{
		strconv.FormatInt(leaseTime, 10),
		wlock.getIndicator(),
	}

	repl, err := util.Redis_handle.Eval(script, keys, args)

	if err != nil{
		return err
	}

	_, err = repl.StringValue()

	return err
}

func (wlock *RWriteLock)doUnlock()(error){

	script := `
		local mode = redis.call('hget', KEYS[1], 'mode');
		if (mode == false) then 
			return 1;
		end;

		if (mode == 'write') then
			local lockExists = redis.call('hget', KEYS[1], ARGV[1]);
			if (lockExists == 0) then
				return nil;
			else
				redis.call('hdel', KEYS[1], ARGV[1]);
				if (redis.call('hlen', KEYS[1]) == 1) then
					redis.call('del', KEYS[1]);
				end;
				return 1;
			end;
		end;
		
		return nil;
	`

	keys := []string{
		wlock.getName(),
	}
	args := []string{
		wlock.getIndicator(),
	}

	repl, err := Redis_handle.Eval(script, keys, args)
	if err != nil{
		return err
	}
	if repl.Type == goredis.ErrorReply{
		return errors.New(repl.Error)
	}
	if repl.Type == goredis.IntegerReply{
		return nil
	}
	return errors.New("the lock is busy")
}

func (wlock *RWriteLock)Lock()(error){
	return wlock.doLock(-1)
}

func (wlock *RWriteLock)Unlock()(error){
	return wlock.doUnlock()
}
{% endhighlight %}


<br />
<br />
**[参看]：**

1. [Redis Eval 命令](https://www.runoob.com/redis/scripting-eval.html)




<br />
<br />
<br />

