---
layout: post
title: Harbor使用手册
tags:
- docker
categories: docker
description: Harbor使用手册
---


本章将会介绍一下Harbor的基本使用， 主要包括以下方面的内容：

* Harbor中工程(projects)的管理

* project中成员(members)的管理

* 复制工程到远程仓库(remote registry)

* 查询工程(projects)及repositories

* 系统管理员身份对Harbor系统的管理

* 使用docker client拉取及推送镜像

* 为repositories添加描述

* 删除repositories及镜像

* Content Trust


<!-- more -->

## 1. 基于角色的访问控制

Harbor提供了基于角色的访问控制（Role Based Access Control, RBAC):

![docker-harbor-rbac](https://ivanzz1001.github.io/records/assets/img/docker/docker_harbor_rbac.png)


Harbor通过工程（projects)的方式来管理镜像(images)。在添加用户到工程中时，可以是如下三种角色中的一种：

* ```Guest```: 指定工程（project)中的Guest用户只有只读访问权限

* ```Developer```: 工程中的Developer用户具有读写访问权限

* ```ProjectAdmin```: 当创建一个新的工程时，你就会被默认指定为该工程的```ProjectAdmin```角色。除了具有读写权限之外，```ProjectAdmin```还有一些管理特权，例如向该工程添加/移除成员、启动一个vulnerability扫描

另外除了这上面这三种角色，还有两个系统层面的角色：

* ```SysAdmin```: SysAdmin角色的用户具有最高管理权限。除了上面提到的一些权限之外，```SysAdmin```用户可以列出所有的工程(projects)； 将一个普通用户设置为管理员； 删除用户； 针对所有镜像设置vulnerability扫描策略。公有工程```library```也是由系统管理员所拥有。

* ```Anonymous```: 当一个用户并没有登录的时候，该用户会被认为是一个匿名用户。一个匿名用户并没有访问私有工程(private project)的权限, 而对于公有工程(public project)拥有只读访问权限。


视频示例: [Tencent Video](https://v.qq.com/x/page/l0553yw19ek.html)


## 2. 用户账户(User Account)
Harbor支持两种身份验证方式：

* **Database(db_auth)**： 这种情况下，所有用户被存放在一个本地数据库

在这种模式下，一个用户可以在Harbor系统中进行注册。如果要禁止```自行注册```功能，在初始化配置时请参看[installation guide](https://github.com/vmware/harbor/blob/master/docs/installation_guide.md)； 或者是在```Administraor Options```中禁止该特性。当self-registration被禁止后，系统管理员可以添加用户到Harbor中。

当注册或添加一个新的用户到Harbor中时，Harbor系统中的用户名、email必须是唯一的。密码至少要有8个字符长度，并且至少要包含一个大写字母(uppercase letter)、一个小写字母(lowercase letter)以及一个数字(numeric character)

当你忘记密码的时候，你可以按如下的步骤来重设密码:
<pre>
1) 在注册页面点击"Forgot Password" 链接

2) 输入你注册时所用的邮箱地址， 然后Harbor系统会发送一封邮件给你来进行密码重设

3) 在收到邮件之后，单击邮件中给出的链接地址，然后会跳转到一个密码重设的Web页面

4) 输入新的密码并单击"Save"按钮
</pre>

* **LDAP/Active Directory(ldap_auth)**: 在这种认证模式下，用户的credentials都被存放在外部的LDAP或AD服务器中，用户在那边完成认证后可以直接登录到Harbor系统。

当一个LDAP/AD用户通过```username```和```password```的方式登录进系统时，Harbor会用```LDAP Search DN```及```LDAP Search Password```绑定LDAP/AD服务器（请参看[installation guide](https://github.com/vmware/harbor/blob/master/docs/installation_guide.md))。假如成功的话，Harbor会在LDAP的```LDAP Base DN```目录及其子目录来查询该用户。通过```LDAP UID```指定的一些属性(比如: uid、cn)会与```username```一起共同来匹配一个用户。假如被成功匹配，用户的密码会通过一个发送到LDAP/AD服务器的bind request所验证。假如LDAP/AD服务器使用自签名证书(self-signed certificate)或者不受信任的证书的话，请不要检查```LDAP Verify Cert```。

在LDAP/AD认证模式下，不支持```self-registration```、删除用户、修改密码、重设密码等功能，这是因为用户是由LDAP/AD系统所管理.


## 3. Harbor工程管理

Harbor中的一个工程包含了一个应用程序所需要的所有repositories。在工程创建之前，并不允许推送镜像到Harbor中。Harbor对于project采用基于角色的访问控制。在Harbor中projects有两种类型：

* ```Public```: 所有的用户都具有读取public project的权限， 其主要是为了方便你分享一些repositories

* ```Private```: 一个私有project只能被特定用户以适当的权限进行访问


在登录之后，你就可以创建一个工程(project)。默认情况下，创建的工程都是私有的，你可以通过在创建时选中```Access Level```复选框来使创建的工程变为public的：

![harbor-create-project](https://ivanzz1001.github.io/records/assets/img/docker/harbor_create_project.png)


在工程被创建完成之后，你可以通过导航标签浏览```repositories```，```members```, ```logs```,```replication```以及```configuration```:

![harbor-browse-project](https://ivanzz1001.github.io/records/assets/img/docker/harbor_browse_project.png)

你可以通过单击```Logs```选项卡来查看所有的日志。你也可以使用```username```, ```operations```以及```Advanced Search```中的日期来过滤日志信息：

![log-search-advanced](https://ivanzz1001.github.io/records/assets/img/docker/log_search_advanced.png)

可以通过单击```Configuration```选项卡设置工程相关属性：

* 选中```Public```复选框，你就可以将该工程下的所有repositories设置为公有访问权限

* 通过使能```Enable content trust```复选框来阻止从工程拉取未被登记的镜像

* 通过选择```Prevent vulnerable images from running```复选框和改变vulnerabilities的安全级别，以阻止vulnerable镜像被拉取。假如一个镜像的安全级别高于大于等于当前所设置的级别的话，Image将不能够被拉取。

* 为了激活对push到工程中的新镜像进行vulnerability扫描，可以选中```Automatically scan images```复选框

![harbor-project-config](https://ivanzz1001.github.io/records/assets/img/docker/harbor_project_config.png)


## 4. 管理工程中的用户成员

**1） 添加成员**

你可以向一个已存在的工程添加项目成员，并为其指定相应的角色。在LDAP/AD认证模式下，你也可以添加一个LDAP/AD用户使其成为工程中的一员：

![new-add-member](https://ivanzz1001.github.io/records/assets/img/docker/new_add_member.png)

**2) 更新及移除成员**

可以批量选中工程中的成员，然后修改他们的角色； 也可以删除某些成员：

![harbor-update-member](https://ivanzz1001.github.io/records/assets/img/docker/harbor_update_member.png)

## 5. 复制镜像

镜像复制被用于从一个Harbor实例向另一个Harbor实例复制repositories。

该功能是面向工程的(project-oriented), 而一旦系统管理员为该工程设置了相应的规则，当[触发条件](https://github.com/vmware/harbor/blob/master/docs/user_guide.md#trigger-mode)被触发的话该工程下所有匹配相应规则的repositories都会被复制到远程仓库中。
每一个repository都会启动一个```job```来进行复制。假如该工程在远程镜像仓库中并不存在，则会自动的创建出一个新的工程。但是假如该工程已经存在并且被用户配置为没有写权限的话，则这个同步过程会失败。这里注意：```项目成员信息将不会被复制```

根据不同的网络状况，这个复制过程可能会有一定的延迟。假若是因为网络的原因导致镜像同步失败，则在几分钟之后Harbor会尝试再进行同步（该同步过程会一直进行，直到网络恢复，同步成功）

注意： Harbor 0.35版本之前和之后的不同实例不能进行相互同步。


### 5.1 创建复制规则
可以通过创建规则来配置复制策略。点击```Administration->Replications```页面的```NEW REPLICATION RULE```按钮，然后填写一些必须的字段。你可以根据不同的需要选择不同的Image filters和trigger mode。假如当前并没有可用的远程registry，则你需要创建一个远程registry。然后单击```SAVE```按钮为指定的工程创建复制规则。假如勾选了```Replicate existing images immediately```复选框，则该工程下已存在的镜像将会马上复制到远程镜像仓库。

**Image filter**

对镜像我们支持两种不同的image filter:

* ```Repository```: 复制时会根据镜像名称的repository部分来进行

* ```Tag```: 复制时会根据镜像名称的标签(tag)部分来进行

在过滤器中支持两种不同的过滤匹配模式：

* ```*```: 匹配除```/```之外的任何字符

* ```?```: 匹配除```/```之外的任何单个字符

**Trigger mode**

* ```Manual```: 当需要的时候，通过手工方式来触发repositories的复制。 Note： 对镜像的删除操作并不会被复制

* ```Immediate```: 当有一个新的repository被push到工程中时，其就会被马上复制到远程registry。假如还勾选了```Delete remote images when locally deleted```复选框，则在本地镜像被删除时也会马上复制到远程registry。

* ```Scheduled```: 每天或者每周复制repositories。Note： 对镜像的删除操作并不会被复制。

![harbor-create-rule](https://ivanzz1001.github.io/records/assets/img/docker/harbor_create_rule.png)

### 5.2 列出和停止replication jobs

选中一个```rule```，则属于该规则的所有jobs都会被列出。一个job代表复制repository到远程registry的一个工作进程。可以单击```STOP JOBS```，则这个rule规则下的所有处于```pending and retrying job```将会被马上停止， 而处于```running```工作状态的job会在下一个checkpoint被取消。

![list-stop-jobs](https://ivanzz1001.github.io/records/assets/img/docker/list_stop_jobs.png)

### 5.3 手动启动replication进程
选中一个复制规则，然后点击```REPLICATE```按钮， 则该工程下匹配相应规则的镜像将会被马上复制到远程registry。假设当前已经有匹配该规则的处于pending/running状态的job，则新的复制将不会被启动。

![harbor-start-replicate](https://ivanzz1001.github.io/records/assets/img/docker/harbor_start_replicate.png)


### 5.4 删除replication rule
选中一个replication rule，然后单击```DELETE```按钮来将其删除。只有那些没有工作任务的rule会被删除（如果一个rule下有处于pending/running/retrying状态的job,则该rule不能被删除）

![harbor-delete-rule](https://ivanzz1001.github.io/records/assets/img/docker/harbor_delete_rule.png)


系统管理员也可以在```Projects```视图下，选中```Replication```标签来为来为指定的project设置复制规则。而工程管理员(Project Manager，请参看上面的RBAC)只有只读访问权限：

![rule-under-project-view](https://ivanzz1001.github.io/records/assets/img/docker/rule_under_project_view.png)

### 5.5 查询工程及repositories
在页面顶部输入搜索关键字，然后点击搜索就可以查询出所有匹配的```projects```以及```repositories```。这些搜索结果包含你有权限访问的public及private repositories。

![harbor-new-search](https://ivanzz1001.github.io/records/assets/img/docker/harbor_new_search.png)



## 6. Administrator Options

### 6.1 用户管理
```Administrator```可以将一个或多个普通的用户设置为```administrator```角色。也可以进行用户的删除（注意： 只有在数据库认证模式下，才支持删除用户）:

![harbor-remove-user](https://ivanzz1001.github.io/records/assets/img/docker/new_set_admin_remove_user.png)


### 6.2 仓库管理(Managing endpoint)
在```Administration->Registries```项下，你可以列出、添加、修改或者删除远程的endpoint。只有那些没有被任何rules所引用的endpoint才能被删除：

![harbor-manage-endpoint](https://ivanzz1001.github.io/records/assets/img/docker/harbor_manage_endpoint.png)

### 6.3 复制管理(Managing replication)

在```Administration->Replications```项下，你可以添加、删除、或修改replication rules:

![harbor-manage-replication](https://ivanzz1001.github.io/records/assets/img/docker/harbor_manage_replication.png)

### 6.4 认证管理(Managing authentication)
在没有添加任何用户之前，你可以修改认证模式（Database模式或者LDAP模式）, 当Harbor系统中已经有至少一个用户之后（出admin用户外），将不能够修改认证模式：

![harbor-new-auth](https://ivanzz1001.github.io/records/assets/img/docker/harbor_new_auth.png)

当使用LDAP模式的时候，用户的自注册功能会被禁止。LDAP服务器的相关参数必须要被填写。更多信息，请参看[User account](https://github.com/vmware/harbor/blob/master/docs/user_guide.md#user-account).

![harbor-ldap-auth](https://ivanzz1001.github.io/records/assets/img/docker/harbor_ldap_auth.png)

### 6.5 工程创建管理(Managing project creation)
可以使用```Project Creation```下拉菜单来设置哪些用户可以创建工程。选择```Everyone```的话，则允许所有的用户来创建工程。如果选择```Admin Only```，则只允许拥有```Administrator```角色的用户创建工程：

![new-proj-create](https://ivanzz1001.github.io/records/assets/img/docker/new_proj_create.png)

### 6.6 用户自注册管理(Managing self-registration)
你可以管理是否允许用户自己注册一个新的账户。该选项在使用LDAP认证的方式下将不可用。

![new-self-reg](https://ivanzz1001.github.io/records/assets/img/docker/new_self_reg.png)


### 6.7 邮箱设置管理
你可以改变Harbor的邮箱设置，该邮箱服务器被用于发送响应给重置密码的用户。

![new-config-email](https://ivanzz1001.github.io/records/assets/img/docker/new_config_email.png)



<br />
<br />

**[参看]**

1. [Harbor User Guide](https://github.com/vmware/harbor/blob/master/docs/user_guide.md)


<br />
<br />
<br />

