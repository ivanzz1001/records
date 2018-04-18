---
layout: post
title: Harbor install脚本分析
tags:
- docker
categories: docker
description: Harbor install脚本分析
---


本文我们分析一下Harbor中的```install.sh```脚本。


<!-- more -->


## 1. Harbor install.sh脚本
{% highlight string %}
#!/bin/bash

#docker version: 1.11.2 
#docker-compose version: 1.7.1 
#Harbor version: 0.4.0 

set +e
set -o noglob

#
# Set Colors
#

bold=$(tput bold)
underline=$(tput sgr 0 1)
reset=$(tput sgr0)

red=$(tput setaf 1)
green=$(tput setaf 76)
white=$(tput setaf 7)
tan=$(tput setaf 202)
blue=$(tput setaf 25)

#
# Headers and Logging
#

underline() { printf "${underline}${bold}%s${reset}\n" "$@"
}
h1() { printf "\n${underline}${bold}${blue}%s${reset}\n" "$@"
}
h2() { printf "\n${underline}${bold}${white}%s${reset}\n" "$@"
}
debug() { printf "${white}%s${reset}\n" "$@"
}
info() { printf "${white}➜ %s${reset}\n" "$@"
}
success() { printf "${green}✔ %s${reset}\n" "$@"
}
error() { printf "${red}✖ %s${reset}\n" "$@"
}
warn() { printf "${tan}➜ %s${reset}\n" "$@"
}
bold() { printf "${bold}%s${reset}\n" "$@"
}
note() { printf "\n${underline}${bold}${blue}Note:${reset} ${blue}%s${reset}\n" "$@"
}

set -e
set +o noglob

usage=$'Please set hostname and other necessary attributes in harbor.cfg first. DO NOT use localhost or 127.0.0.1 for hostname, because Harbor needs to be accessed by external clients.
Please set --with-notary if needs enable Notary in Harbor, and set ui_url_protocol/ssl_cert/ssl_cert_key in harbor.cfg bacause notary must run under https. 
Please set --with-clair if needs enable Clair in Harbor'
item=0

# notary is not enabled by default
with_notary=$false
# clair is not enabled by default
with_clair=$false
# HA mode is not enabled by default
harbor_ha=$false
while [ $# -gt 0 ]; do
        case $1 in
            --help)
            note "$usage"
            exit 0;;
            --with-notary)
            with_notary=true;;
            --with-clair)
            with_clair=true;;
            --ha)
            harbor_ha=true;;
            *)
            note "$usage"
            exit 1;;
        esac
        shift || true
done

workdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $workdir

# The hostname in harbor.cfg has not been modified
if grep 'hostname = reg.mydomain.com' &> /dev/null harbor.cfg
then
	warn "$usage"
	exit 1
fi

function check_docker {
	if ! docker --version &> /dev/null
	then
		error "Need to install docker(1.10.0+) first and run this script again."
		exit 1
	fi
	
	# docker has been installed and check its version
	if [[ $(docker --version) =~ (([0-9]+).([0-9]+).([0-9]+)) ]]
	then
		docker_version=${BASH_REMATCH[1]}
		docker_version_part1=${BASH_REMATCH[2]}
		docker_version_part2=${BASH_REMATCH[3]}
		
		# the version of docker does not meet the requirement
		if [ "$docker_version_part1" -lt 1 ] || ([ "$docker_version_part1" -eq 1 ] && [ "$docker_version_part2" -lt 10 ])
		then
			error "Need to upgrade docker package to 1.10.0+."
			exit 1
		else
			note "docker version: $docker_version"
		fi
	else
		error "Failed to parse docker version."
		exit 1
	fi
}

function check_dockercompose {
	if ! docker-compose --version &> /dev/null
	then
		error "Need to install docker-compose(1.7.1+) by yourself first and run this script again."
		exit 1
	fi
	
	# docker-compose has been installed, check its version
	if [[ $(docker-compose --version) =~ (([0-9]+).([0-9]+).([0-9]+)) ]]
	then
		docker_compose_version=${BASH_REMATCH[1]}
		docker_compose_version_part1=${BASH_REMATCH[2]}
		docker_compose_version_part2=${BASH_REMATCH[3]}
		
		# the version of docker-compose does not meet the requirement
		if [ "$docker_compose_version_part1" -lt 1 ] || ([ "$docker_compose_version_part1" -eq 1 ] && [ "$docker_compose_version_part2" -lt 6 ])
		then
			error "Need to upgrade docker-compose package to 1.7.1+."
                        exit 1
		else
			note "docker-compose version: $docker_compose_version"
		fi
	else
		error "Failed to parse docker-compose version."
		exit 1
	fi
}

h2 "[Step $item]: checking installation environment ..."; let item+=1
check_docker
check_dockercompose

if [ -f harbor*.tar.gz ]
then
	h2 "[Step $item]: loading Harbor images ..."; let item+=1
	docker load -i ./harbor*.tar.gz
fi
echo ""

h2 "[Step $item]: preparing environment ...";  let item+=1
if [ -n "$host" ]
then
	sed "s/^hostname = .*/hostname = $host/g" -i ./harbor.cfg
fi
prepare_para=
if [ $with_notary ] && [ ! $harbor_ha ]
then
	prepare_para="${prepare_para} --with-notary"
fi
if [ $with_clair ]
then
	prepare_para="${prepare_para} --with-clair"
fi
if [ $harbor_ha ]
then
    prepare_para="${prepare_para} --ha"
fi
./prepare $prepare_para
echo ""

h2 "[Step $item]: checking existing instance of Harbor ..."; let item+=1
docker_compose_list='-f docker-compose.yml'
if [ $with_notary ] && [ ! $harbor_ha ]
then
	docker_compose_list="${docker_compose_list} -f docker-compose.notary.yml"
fi
if [ $with_clair ]
then
	docker_compose_list="${docker_compose_list} -f docker-compose.clair.yml"
fi

if [ -n "$(docker-compose $docker_compose_list ps -q)"  ]
then
	note "stopping existing Harbor instance ..." 
	docker-compose $docker_compose_list down -v
fi
echo ""

h2 "[Step $item]: starting Harbor ..."
if [ $harbor_ha ]
then
    mv docker-compose.yml docker-compose.yml.bak 
    cp ha/docker-compose.yml docker-compose.yml
    mv docker-compose.clair.yml docker-compose.clair.yml.bak
    cp ha/docker-compose.clair.yml docker-compose.clair.yml
fi
docker-compose $docker_compose_list up -d

protocol=http
hostname=reg.mydomain.com

if [[ $(cat ./harbor.cfg) =~ ui_url_protocol[[:blank:]]*=[[:blank:]]*(https?) ]]
then
protocol=${BASH_REMATCH[1]}
fi

if [[ $(grep 'hostname[[:blank:]]*=' ./harbor.cfg) =~ hostname[[:blank:]]*=[[:blank:]]*(.*) ]]
then
hostname=${BASH_REMATCH[1]}
fi
echo ""

success $"----Harbor has been installed and started successfully.----

Now you should be able to visit the admin portal at ${protocol}://${hostname}. 
For more details, please visit https://github.com/vmware/harbor .
"
{% endhighlight %}

下面我们来分析一下install.sh这个shell脚本。

### 1.1 set相关命令

set命令的作用主要是显示系统中已经存在的shell变量，以及设置shell变量的新变量值。使用set更改shell特性时，符号```+```和```-```的作用分别是```打开```和```关闭```指定的模式。set命令不能够定义新的shell变量。如果要定义新的变量，可以使用declare命令以```variable=value```的格式进行定义即可。用法如下：
<pre>
set [--abefhkmnptuvxBCEHPT] [-o option-name] [arg ...]
set [+abefhkmnptuvxBCEHPT] [+o option-name] [arg ...]
</pre>
```set```命令在不添加任何```选项```的时候，即直接执行```# set```，则将会打印出shell中定义的每一个变量的名称及对应的值。如果不添加任何```选项```，还可以用于重置当前```set variable```的值。只读变量的值不能被reset。 如果后面跟了对应的```选项```，则会```set```或```unset```对应的shell属性。

下面再介绍一下常用的相关```选项```:
{% highlight string %}
-a   自动标示已修改的变量， 并输出至环境变量
-e   若指令传回值不等于0，则立即退出shell (注意并不会作用于if/elif/while/until等条件判断上面)
-f   禁止路径名展开
-o noglob  与-f选项相同
{% endhighlight %}

例如：
<pre>
# declare mylove='Visual C++'
# set -a mylove
# env | grep mylove
</pre>

### 1.2 设置相关打印样式
{% highlight string %}
#
# Set Colors
#

bold=$(tput bold)
underline=$(tput sgr 0 1)
reset=$(tput sgr0)

red=$(tput setaf 1)
green=$(tput setaf 76)
white=$(tput setaf 7)
tan=$(tput setaf 202)
blue=$(tput setaf 25)

#
# Headers and Logging
#

underline() { printf "${underline}${bold}%s${reset}\n" "$@"
}
h1() { printf "\n${underline}${bold}${blue}%s${reset}\n" "$@"
}
h2() { printf "\n${underline}${bold}${white}%s${reset}\n" "$@"
}
debug() { printf "${white}%s${reset}\n" "$@"
}
info() { printf "${white}➜ %s${reset}\n" "$@"
}
success() { printf "${green}✔ %s${reset}\n" "$@"
}
error() { printf "${red}✖ %s${reset}\n" "$@"
}
warn() { printf "${tan}➜ %s${reset}\n" "$@"
}
bold() { printf "${bold}%s${reset}\n" "$@"
}
note() { printf "\n${underline}${bold}${blue}Note:${reset} ${blue}%s${reset}\n" "$@"
}
{% endhighlight %}
这里比较简单，我们举个例子test.sh：
<pre>
#!/bin/bash

set +e
set -o noglob

bold=$(tput bold)
underline=$(tput sgr 0 1)
reset=$(tput sgr0)

red=$(tput setaf 1)

underline() { printf "${underline}${bold}${red}%s${reset}\n" "$@"
}

underline hello,world
</pre>
执行：
<pre>
# chmod 777 ./test.sh
# ./test.sh
hello,world
</pre>
注意上面可能需要直接在Linux shell伪终端执行才能看到效果.

### 1.3 相关命令行选项解析
{% highlight string %}
set -e
set +o noglob

usage=$'Please set hostname and other necessary attributes in harbor.cfg first. DO NOT use localhost or 127.0.0.1 for hostname, because Harbor needs to be accessed by external clients.
Please set --with-notary if needs enable Notary in Harbor, and set ui_url_protocol/ssl_cert/ssl_cert_key in harbor.cfg bacause notary must run under https. 
Please set --with-clair if needs enable Clair in Harbor'
item=0

# notary is not enabled by default
with_notary=$false
# clair is not enabled by default
with_clair=$false
# HA mode is not enabled by default
harbor_ha=$false
while [ $# -gt 0 ]; do
        case $1 in
            --help)
            note "$usage"
            exit 0;;
            --with-notary)
            with_notary=true;;
            --with-clair)
            with_clair=true;;
            --ha)
            harbor_ha=true;;
            *)
            note "$usage"
            exit 1;;
        esac
        shift || true
done

workdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $workdir

# The hostname in harbor.cfg has not been modified
if grep 'hostname = reg.mydomain.com' &> /dev/null harbor.cfg
then
	warn "$usage"
	exit 1
fi
{% endhighlight %}
默认情况下Harbor并不支持```Notary```、```Clair```、```HA```。

* **Notary**: 是一个用于发布及管理```可信任内容集```(trusted collections of content)的工具。发布者可以对所发布的内容进行数字化签名，而消费者可以验证数据的完整性及内容源

* **Clair**: 是一个用于静态分析```appc```及```docker```容器脆弱性的一个开源项目

* **HA**: 关于Harbor的高可用性方案，我们后面来介绍

在```/bin/bash```脚本中，```${BASH_SOURCE[0]}```为当前执行脚本的文件名称。

这里后面还会检查harbor.cfg配置文件中，hostname是否进行了修改：
{% highlight string %}
## 这里&>是将标准输出及标准错误重定向到/dev/null中
grep 'hostname = reg.mydomain.com' &> /dev/null harbor.cfg  
{% endhighlight %}

### 1.4 docker及docker compose版本检查
{% highlight string %}
function check_docker {
	if ! docker --version &> /dev/null
	then
		error "Need to install docker(1.10.0+) first and run this script again."
		exit 1
	fi
	
	# docker has been installed and check its version
	if [[ $(docker --version) =~ (([0-9]+).([0-9]+).([0-9]+)) ]]
	then
		docker_version=${BASH_REMATCH[1]}
		docker_version_part1=${BASH_REMATCH[2]}
		docker_version_part2=${BASH_REMATCH[3]}
		
		# the version of docker does not meet the requirement
		if [ "$docker_version_part1" -lt 1 ] || ([ "$docker_version_part1" -eq 1 ] && [ "$docker_version_part2" -lt 10 ])
		then
			error "Need to upgrade docker package to 1.10.0+."
			exit 1
		else
			note "docker version: $docker_version"
		fi
	else
		error "Failed to parse docker version."
		exit 1
	fi
}

function check_dockercompose {
	if ! docker-compose --version &> /dev/null
	then
		error "Need to install docker-compose(1.7.1+) by yourself first and run this script again."
		exit 1
	fi
	
	# docker-compose has been installed, check its version
	if [[ $(docker-compose --version) =~ (([0-9]+).([0-9]+).([0-9]+)) ]]
	then
		docker_compose_version=${BASH_REMATCH[1]}
		docker_compose_version_part1=${BASH_REMATCH[2]}
		docker_compose_version_part2=${BASH_REMATCH[3]}
		
		# the version of docker-compose does not meet the requirement
		if [ "$docker_compose_version_part1" -lt 1 ] || ([ "$docker_compose_version_part1" -eq 1 ] && [ "$docker_compose_version_part2" -lt 6 ])
		then
			error "Need to upgrade docker-compose package to 1.7.1+."
                        exit 1
		else
			note "docker-compose version: $docker_compose_version"
		fi
	else
		error "Failed to parse docker-compose version."
		exit 1
	fi
}

h2 "[Step $item]: checking installation environment ..."; let item+=1
check_docker
check_dockercompose
{% endhighlight %}
这里检查docker的版本不能低于```1.10```; docker-compose的版本不能低于```1.6```。

### 1.5 加载Harbor运行所需要的镜像
{% highlight string %}
if [ -f harbor*.tar.gz ]
then
	h2 "[Step $item]: loading Harbor images ..."; let item+=1
	docker load -i ./harbor*.tar.gz
fi
echo ""
{% endhighlight %}
在解压后的Harbor文件夹下：
<pre>
# pwd
/opt/harbor-inst/harbor

# ls
bakup-harbor.cfg  docker-compose.clair.yml   docker-compose.yml  harbor.cfg            install.sh  NOTICE
common            docker-compose.notary.yml  ha                  harbor.v1.4.0.tar.gz  LICENSE     prepare
</pre>
我们看到有```harbor.v1.4.0.tar.gz```，这里面有Harbor运行所需要的全部镜像。加载完成后，我们可以看到：
<pre>
# docker images
REPOSITORY                        TAG                 IMAGE ID            CREATED             SIZE
vmware/clair-photon               v2.0.1-v1.4.0       a1df3526fe43        2 months ago        300MB
vmware/notary-server-photon       v0.5.1-v1.4.0       3edfddb8ece2        2 months ago        211MB
vmware/notary-signer-photon       v0.5.1-v1.4.0       cc70a05cdb6a        2 months ago        209MB
vmware/registry-photon            v2.6.2-v1.4.0       8920f621ddd1        2 months ago        198MB
vmware/nginx-photon               v1.4.0              20c8a01ac6ab        2 months ago        135MB
vmware/harbor-log                 v1.4.0              9e818c7a27ab        2 months ago        200MB
vmware/harbor-jobservice          v1.4.0              29c14d91b043        2 months ago        191MB
vmware/harbor-ui                  v1.4.0              6cb4318eda6a        2 months ago        210MB
vmware/harbor-adminserver         v1.4.0              8145970fa013        2 months ago        182MB
vmware/harbor-db                  v1.4.0              c38da34727f0        2 months ago        521MB
vmware/mariadb-photon             v1.4.0              8457013cf6e3        2 months ago        521MB
vmware/postgresql-photon          v1.4.0              59aa61520094        2 months ago        221MB
vmware/harbor-db-migrator         1.4                 7a4d871b612e        3 months ago        1.15GB
vmware/photon                     1.0                 9b411d78ad9e        3 months ago        130MB
</pre>

加载完成后，我们看到总共有14个镜像， 其中：

* ```vmware/clair-photon```、```vmware/postgresql-photon```： 是用于抗脆弱性检查的

*  ```vmware/notary-server-photon```、```vmware/notary-signer-photon```、```vmware/mariadb-photon```： 是用于notary功能的

* ```vmware/photon```: 是VMware为容器化应用而设计的轻量级的操作系统。类似RedHat的 Atomic项目，以及Ubuntu的Snappy项目。Project Photon主要用于运行VMware vSphere和VMware vCloud，它使得企业可以在单一平台上同时启动容器与虚拟机，而且还可以实现在虚拟机中运行许多不同的容器 (当前我们并不需要使用到）

* ```vmware/harbor-db-migrator ```: 是Harbor的一个数据库迁移工具（用于早期版本数据库的迁移，暂时不用）

剩余的7个镜像，在默认情况下我们都会被使用到：

* ```vmware/registry-photon```: 是docker仓库镜像

* ```vmware/nginx-photon```: 是用于Harbor反向代理的nginx镜像

* ```vmware/harbor-log```: 是harbor的日志镜像

* ```vmware/harbor-jobservice```: 是用于Harbor之间数据同步的镜像

* ```vmware/harbor-ui```: 是Harbor的Web管理镜像

* ```vmware/harbor-adminserver```: Harbor后台管理程序

* ```harbor-db```: harbor数据库，实际是一个mysql数据库镜像


### 1.6 准备Harbor运行环境
{% highlight string %}
h2 "[Step $item]: preparing environment ...";  let item+=1
if [ -n "$host" ]
then
	sed "s/^hostname = .*/hostname = $host/g" -i ./harbor.cfg
fi
prepare_para=
if [ $with_notary ] && [ ! $harbor_ha ]
then
	prepare_para="${prepare_para} --with-notary"
fi
if [ $with_clair ]
then
	prepare_para="${prepare_para} --with-clair"
fi
if [ $harbor_ha ]
then
    prepare_para="${prepare_para} --ha"
fi
./prepare $prepare_para
echo ""
{% endhighlight %}
这里首先检查有没有定义```host```环境变量，如果定义了，则用其来修改```harbor.cfg```中hostname字段的值。这里默认情况下，我们执行的```prepare```脚本不携带任何参数。

### 1.7 检查是否已经存在Harbor实例
{% highlight string %}
h2 "[Step $item]: checking existing instance of Harbor ..."; let item+=1
docker_compose_list='-f docker-compose.yml'
if [ $with_notary ] && [ ! $harbor_ha ]
then
	docker_compose_list="${docker_compose_list} -f docker-compose.notary.yml"
fi
if [ $with_clair ]
then
	docker_compose_list="${docker_compose_list} -f docker-compose.clair.yml"
fi

if [ -n "$(docker-compose $docker_compose_list ps -q)"  ]
then
	note "stopping existing Harbor instance ..." 
	docker-compose $docker_compose_list down -v
fi
echo ""
{% endhighlight %}
这里通过```docker-compose -f docker-compose.yml ps -q```命令检查是否已经有harbor实例（包括正在运行及已经停止的）， 如果有则调用```docker-compose -f docker-compose.yml down -v```命令来删除实例。

### 1.8 启动Harbor
{% highlight string %}
h2 "[Step $item]: starting Harbor ..."
if [ $harbor_ha ]
then
    mv docker-compose.yml docker-compose.yml.bak 
    cp ha/docker-compose.yml docker-compose.yml
    mv docker-compose.clair.yml docker-compose.clair.yml.bak
    cp ha/docker-compose.clair.yml docker-compose.clair.yml
fi
docker-compose $docker_compose_list up -d

protocol=http
hostname=reg.mydomain.com

if [[ $(cat ./harbor.cfg) =~ ui_url_protocol[[:blank:]]*=[[:blank:]]*(https?) ]]
then
protocol=${BASH_REMATCH[1]}
fi

if [[ $(grep 'hostname[[:blank:]]*=' ./harbor.cfg) =~ hostname[[:blank:]]*=[[:blank:]]*(.*) ]]
then
hostname=${BASH_REMATCH[1]}
fi
echo ""

success $"----Harbor has been installed and started successfully.----

Now you should be able to visit the admin portal at ${protocol}://${hostname}. 
For more details, please visit https://github.com/vmware/harbor .
"
{% endhighlight %}

因为这里我们默认并没有使用```Harbor HA```，因此这里采用默认的docker-compose.yml来启动Harbor系统：
<pre>
# docker-compose -f docker-compose.yml up -d
</pre>

然后再从harbor.cfg中读取我们的配置，打印出相关读取的启动信息。





<br />
<br />

**[参看]**

1. [set命令](http://man.linuxde.net/set)

2. [Clair 2.0.1 Documentation](https://coreos.com/clair/docs/latest/)

3. [VMware 发布开源项目 Lightwave 和 Photon](https://www.oschina.net/news/61704/vmware-opensource-lightware-and-photon)
<br />
<br />
<br />

