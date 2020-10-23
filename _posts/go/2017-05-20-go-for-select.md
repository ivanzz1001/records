---
layout: post
title: golang for-select中的break、continue、return
tags:
- go-language
categories: go-language
description: golang for-select中的break、continue、return
---


这里我们对for-slect中的break/continue/return的使用做一个简单的记录。


<!-- more -->

## 1. break
* select中的break，类似于C语言中的break，break后的语句不执行；

* for和select一同使用，有坑： break只能跳出select，无法跳出for;

{% highlight string %}
package test

import (
	"fmt"
	"testing"
	"time"
)

func TestBreak(t *testing.T){
	tick := time.Tick(time.Second)
	for {
		select {
		case t := <-tick:
			fmt.Println(t)
			break
		}
	}
	fmt.Println("end")
}
{% endhighlight %}
执行结果：
<pre>
2020-10-23 18:13:59.6687771 +0800 CST m=+1.007057601
2020-10-23 18:14:00.6688343 +0800 CST m=+2.007114801
2020-10-23 18:14:01.6688915 +0800 CST m=+3.007172001
2020-10-23 18:14:02.6689487 +0800 CST m=+4.007229201
2020-10-23 18:14:03.6690059 +0800 CST m=+5.007286401
2020-10-23 18:14:04.6690631 +0800 CST m=+6.007343601
2020-10-23 18:14:05.6691203 +0800 CST m=+7.007400801
2020-10-23 18:14:06.6691775 +0800 CST m=+8.007458001
2020-10-23 18:14:07.6692347 +0800 CST m=+9.007515201
...
</pre>

###### 1.1 break无法跳出for的解决方案
1） **标签**
{% highlight string %}
package test

import (
	"fmt"
	"testing"
	"time"
)


func TestBreak(t *testing.T){
	tick := time.Tick(time.Second)

	//FOR是标签
FOR:
	for {
		select {
		case t := <-tick:
			fmt.Println(t)
			//break出FOR标签标识的代码
			break FOR
		}
	}
	fmt.Println("end")
}
{% endhighlight %}
执行结果如下：
<pre>
2020-10-23 18:17:04.8603694 +0800 CST m=+1.006057501
end
</pre>

2) **goto**
{% highlight string %}
package test

import (
	"fmt"
	"testing"
	"time"
)


func TestBreak(t *testing.T){
	tick := time.Tick(time.Second)
	for {
		select {
		case t := <-tick:
			fmt.Println(t)
			//跳到指定位置
			goto END
		}
	}
END:
	fmt.Println("end")
}
{% endhighlight %}
执行结果如下：
<pre>
2020-10-23 18:18:52.9695529 +0800 CST m=+1.006057501
end
</pre>

## 2. continue
单独在select中是不能使用continue的，会编译错误，只能用在for-select中。

continue的语义就类似for中的语义，select后的代码不会被执行到。
{% highlight string %}
package test

import (
	"fmt"
	"testing"
	"time"
)


func TestContinue(t *testing.T) {
	tick := time.Tick(time.Second)
	for {
		select {
		case t := <-tick:
			fmt.Println(t)
			continue
			fmt.Println("test")
		}
	}
	fmt.Println("end")
}
{% endhighlight %}
执行结果如下：
<pre>
2020-10-23 18:22:03.8014679 +0800 CST m=+1.006057601
2020-10-23 18:22:04.8015251 +0800 CST m=+2.006114801
2020-10-23 18:22:05.8015823 +0800 CST m=+3.006172001
2020-10-23 18:22:06.8016395 +0800 CST m=+4.006229201
2020-10-23 18:22:07.8016967 +0800 CST m=+5.006286401
2020-10-23 18:22:08.8017539 +0800 CST m=+6.006343601
...
</pre>

## 3. return
和函数中的return一样，跳出select和for，后续代码都不执行。


<br />
<br />
**[参看]：**



<br />
<br />
<br />

