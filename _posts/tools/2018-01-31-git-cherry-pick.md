---
layout: post
title: git cherry-pick的使用
tags:
- tools
categories: tools
description: git cherry-pick的使用
---

git cherry-pick 命令的作用是将指定的提交（commit）应用到其他分支上。这个命令允许你选择一个或多个已有的提交，并将它们作为新的提交引入到当前分支中。

这个过程不会改变项目的历史记录，因为它实际上是创建了这些提交的副本。




<!-- more -->

## 1 分散的提交

如果你想要批量```cherry-pick``` 一系列不连续的提交，可以将它们的哈希值列出来，用空格分隔：
<pre>
# git cherry-pick commitID1 commitID2 commitID3
</pre>


- commitID1, commitID2, commitID3 是你想要 cherry-pick 的提交的哈希值。

如果你有一系列连续的提交，你可以使用```..```语法来指定范围。如果提交是分散的，你可以通过空格分隔每个提交的哈希值来一次性应用它们。




## 2. cherry-pick 连续的提交
如果你想要批量 cherry-pick 一系列连续的提交，可以使用如下命令：
<pre>
# git cherry-pick startCommitID^..endCommitID
</pre>

- startCommitID 是你想要开始 cherry-pick 的第一个提交的哈希值。

- endCommitID 是你想要结束 cherry-pick 的最后一个提交的哈希值。

- 注意```^```符号是为了包含 startCommitID 在内。


<br />
<br />

**[参看]**

1. [每日一个Git命令: cherry-pick](https://www.jianshu.com/p/ed9d97a39ca0)


<br />
<br />
<br />

