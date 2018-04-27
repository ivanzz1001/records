---
layout: post
title: Harbor Makefile脚本分析
tags:
- docker
categories: docker
description: Harbor Makefile脚本分析
---

本节我们分析一下harbor编译的Makefile脚本，在这里并不会详细解释Makefile中每一行代码，而是会挑选几个```target```，然后以这些```target```作为入口点来介绍一下Harbor的编译流程。



<!-- more -->


## 1. Makefile中常用target介绍
{% highlight string %}
# Makefile for Harbor project
#
# Targets:
#
# all:			prepare env, compile binarys, build images and install images
# prepare: 		prepare env
# compile: 		compile adminserver, ui and jobservice code
#
# compile_golangimage:
#			compile from golang image
#			for example: make compile_golangimage -e GOBUILDIMAGE= \
#							golang:1.7.3
# compile_adminserver, compile_ui, compile_jobservice: compile specific binary
#
# build:	build Harbor docker images from photon baseimage
#
# install:		include compile binarys, build images, prepare specific \
#				version composefile and startup Harbor instance
#
# start:		startup Harbor instance
#
# down:			shutdown Harbor instance
#
# package_online:
#				prepare online install package
#			for example: make package_online -e DEVFLAG=false\
#							REGISTRYSERVER=reg-bj.eng.vmware.com \
#							REGISTRYPROJECTNAME=harborrelease
#
# package_offline:
#				prepare offline install package
#
# pushimage:	push Harbor images to specific registry server
#			for example: make pushimage -e DEVFLAG=false REGISTRYUSER=admin \
#							REGISTRYPASSWORD=***** \
#							REGISTRYSERVER=reg-bj.eng.vmware.com/ \
#							REGISTRYPROJECTNAME=harborrelease
#				note**: need add "/" on end of REGISTRYSERVER. If not setting \
#						this value will push images directly to dockerhub.
#						 make pushimage -e DEVFLAG=false REGISTRYUSER=vmware \
#							REGISTRYPASSWORD=***** \
#							REGISTRYPROJECTNAME=vmware
#
# clean:        remove binary, Harbor images, specific version docker-compose \
#               file, specific version tag and online/offline install package
# cleanbinary:	remove adminserver, ui and jobservice binary
# cleanimage: 	remove Harbor images
# cleandockercomposefile:
#				remove specific version docker-compose
# cleanversiontag:
#				cleanpackageremove specific version tag
# cleanpackage: remove online/offline install package
#
# other example:
#	clean specific version binarys and images:
#				make clean -e VERSIONTAG=[TAG]
#				note**: If commit new code to github, the git commit TAG will \
#				change. Better use this commond clean previous images and \
#				files with specific TAG.
#   By default DEVFLAG=true, if you want to release new version of Harbor, \
#		should setting the flag to false.
#				make XXXX -e DEVFLAG=false
{% endhighlight %}

## 2. target instal

一般编译时我们都会简单的使用```make install```来完成。下面我们就以```install```这个target作为入口点来分析一下Harbor的编译流程:
{% highlight string %}
install: compile version build modify_sourcefiles prepare modify_composefile start
{% endhighlight %}

可以看到其依赖于众多其他的target。

### 2.1 target compile
{% highlight string %}
compile:check_environment compile_golangimage
{% endhighlight %}
在这里，```compile```这个target依赖于```check_environment```以及```compile_golangimage```。

(1) **check_environment**
<pre>
check_environment:
	@$(MAKEPATH)/$(CHECKENVCMD)
</pre>
这里是执行```make/checkenv.sh```脚本，主要是负责检查```golang```环境、```docker```环境以及```docker-compose```环境。

(2) **compile_golangimage**
{% highlight string %}
compile_golangimage: compile_clarity
	@echo "compiling binary for adminserver (golang image)..."
	@echo $(GOBASEPATH)
	@echo $(GOBUILDPATH)
	@$(DOCKERCMD) run --rm -v $(BUILDPATH):$(GOBUILDPATH) -w $(GOBUILDPATH_ADMINSERVER) $(GOBUILDIMAGE) $(GOIMAGEBUILD) -v -o $(GOBUILDMAKEPATH_ADMINSERVER)/$(ADMINSERVERBINARYNAME)
	@echo "Done."

	@echo "compiling binary for ui (golang image)..."
	@echo $(GOBASEPATH)
	@echo $(GOBUILDPATH)
	@$(DOCKERCMD) run --rm -v $(BUILDPATH):$(GOBUILDPATH) -w $(GOBUILDPATH_UI) $(GOBUILDIMAGE) $(GOIMAGEBUILD) -v -o $(GOBUILDMAKEPATH_UI)/$(UIBINARYNAME)
	@echo "Done."

	@echo "compiling binary for jobservice (golang image)..."
	@$(DOCKERCMD) run --rm -v $(BUILDPATH):$(GOBUILDPATH) -w $(GOBUILDPATH_JOBSERVICE) $(GOBUILDIMAGE) $(GOIMAGEBUILD) -v -o $(GOBUILDMAKEPATH_JOBSERVICE)/$(JOBSERVICEBINARYNAME)
	@echo "Done."
{% endhighlight %}
在这里首先会依赖于```compile_clarity```:
<pre>
compile_clarity:
	@echo "compiling binary for clarity ui..."
	@if [ "$(HTTPPROXY)" != "" ] ; then \
		$(DOCKERCMD) run --rm -v $(BUILDPATH)/src:$(CLARITYSEEDPATH) $(CLARITYIMAGE) $(SHELL) $(CLARITYBUILDSCRIPT) -p $(HTTPPROXY); \
	else \
		$(DOCKERCMD) run --rm -v $(BUILDPATH)/src:$(CLARITYSEEDPATH) $(CLARITYIMAGE) $(SHELL) $(CLARITYBUILDSCRIPT); \
	fi
	@echo "Done."
</pre>
因为当前我们并未设置```HTTPPROXY```，因此这里执行```else```分支。这里主要是运行起```vmware/harbor-clarity-ui-builder:1.4.0```这个镜像，然后执行其中的```/entrypoint.sh```脚本。 





<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [harbor compile](https://github.com/vmware/harbor/blob/master/docs/compile_guide.md)



<br />
<br />
<br />

