---
layout: post
title: mysql防注入
tags:
- go-language
categories: go-language
description: mysql防注入
---


在数据库编程中，我们经常会遇到SQL注入的问题，这里我们通过查看Golang底层源代码的实现，来了解通用的防SQL做法。

<!-- more -->

## 1. Golang底层库中防SQL注入实现
{% highlight string %}
// reserveBuffer checks cap(buf) and expand buffer to len(buf) + appendSize.
// If cap(buf) is not enough, reallocate new buffer.
func reserveBuffer(buf []byte, appendSize int) []byte {
   newSize := len(buf) + appendSize
   if cap(buf) < newSize {
      // Grow buffer exponentially
      newBuf := make([]byte, len(buf)*2+appendSize)
      copy(newBuf, buf)
      buf = newBuf
   }
   return buf[:newSize]
}

func MySQLEscapeStringQuotes(buf []byte, v string) []byte{
   pos := len(buf)
   buf = reserveBuffer(buf, len(v)*2)

   for i := 0; i < len(v); i++ {
      c := v[i]
      if c == '\'' {
         buf[pos] = '\''
         buf[pos+1] = '\''
         pos += 2
      } else {
         buf[pos] = c
         pos++
      }
   }

   return buf[:pos]
}
{% endhighlight %}

如下为测试脚本：
{% highlight string %}
func TestMySQLEscapeStringQuotes(t *testing.T){
   var(
      buf [2048]byte
   )

   v := "aaaa'fff''bbbccc"
   v2 := "111'222'333'444'555'666'abc"

   result1 := string(MySQLEscapeStringQuotes(buf[:0], v))
   result2 := string(MySQLEscapeStringQuotes(buf[:0], v2))

   fmt.Printf("result1: %s result2: %s", result1, result2)
}
{% endhighlight %}








<br />
<br />
**[参看]：**





<br />
<br />
<br />

