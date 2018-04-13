---
layout: post
title: Harbor中repository的删除
tags:
- docker
categories: docker
description: Harbor中repository的删除
---


本节我们介绍一下Harbor中repository的删除操作。


<!-- more -->


## 1. 删除repositories
Repositories删除时，有两个步骤：

首先，在Harbor的UI上删除一个repository。这只是一个软删除(soft deletion)。你可以删除整个repository，或者只是该repository的一个标签。在软删除之后，该repository将不再由Harbor进行管理，然而，该repository的文件还仍然存放在Harbor的存储系统中：


![new-delete-repo](https://ivanzz1001.github.io/records/assets/img/docker/new_delete_repo.png)

![new-delete-tag](https://ivanzz1001.github.io/records/assets/img/docker/new_delete_tag.png)


<pre>
注意： 假如tag A和tag B都引用同一个镜像，在删除掉tag A之后， B也会被删除。假如你使能了content trust，在你删除镜像之前，
      你需要使用notary命令行工具来删除tag的signature
</pre>

下一步，你需要使用registry的garbage collection(GC)机制删除repository的实际文件。在你执行一个GC操作的时候，请确保不能推送任何镜像到registry， 或者直接不让Harbor运行。假如在GC的时候有人推送镜像， 有可能会存在image的layer被误删的情况，这样将会导致一个不可用的image。因此在运行GC之前，最好首先停止Harbor的访问。

首先在部署Harbor的主机上执行如下的命令，来查看有哪些files/images会受到影响：
<pre>
# docker-compose down

# docker run -it --name gc --rm --volumes-from registry vmware/registry:2.6.2-photon garbage-collect --dry-run /etc/registry/config.yml
</pre>

Note: 上面的```--dry-run```选项将会打印出相应的检查进程，而并不会删除任何数据

然后检查上面测试的打印结果，然后使用下面的命令来进行垃圾收集，并重启Harbor：
<pre>
# docker run -it --name gc --rm --volumes-from registry vmware/registry:2.6.2-photon garbage-collect  /etc/registry/config.yml

# docker-compose start
</pre>
更多关于GC的信息，请参看[GC](https://github.com/docker/docker.github.io/blob/master/registry/garbage-collection.md)



<br />
<br />

**[参看]**

1. [Harbor User Guide](https://github.com/vmware/harbor/blob/master/docs/user_guide.md)

<br />
<br />
<br />

