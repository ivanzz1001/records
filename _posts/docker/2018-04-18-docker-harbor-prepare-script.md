---
layout: post
title: Harbor prepare脚本分析
tags:
- docker
categories: docker
description: Harbor prepare脚本分析
---


本文我们分析一下Harbor中的```prepare```脚本。 prepare脚本是一个用python语言来写的脚本，其主要用于产生配置脚本。这里我们简要分析一下该脚本。


<!-- more -->

## 1. validate()函数
{% highlight string %}
#!/usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import print_function, unicode_literals # We require Python 2.6 or later
from string import Template
import random
import string
import os
import sys
import argparse
import subprocess
import shutil
from io import open

if sys.version_info[:3][0] == 2:
    import ConfigParser as ConfigParser
    import StringIO as StringIO

if sys.version_info[:3][0] == 3:
    import configparser as ConfigParser
    import io as StringIO

def validate(conf, args): 
    if args.ha_mode:
        db_host = rcp.get("configuration", "db_host")
        if db_host == "mysql":
            raise Exception("Error: In HA mode, db_host in harbor.cfg needs to point to an external DB address.")
        registry_storage_provider_name = rcp.get("configuration",
                                                 "registry_storage_provider_name").strip()
        if registry_storage_provider_name == "filesystem" and not args.yes:
            msg = 'Is the Harbor Docker Registry configured to use shared storage (e.g. NFS, Ceph etc.)? [yes/no]:'
            if raw_input(msg).lower() != "yes":
                raise Exception("Error: In HA mode, shared storage configuration for Docker Registry in harbor.cfg is required. Refer to HA installation guide for details.")
        redis_url = rcp.get("configuration", "redis_url")
        if redis_url is None or len(redis_url) < 1:
            raise Exception("Error: In HA mode, redis_url in harbor.cfg needs to point to a Redis cluster.")
        if args.notary_mode:
            raise Exception("Error: HA mode doesn't support Notary currently")
        if args.clair_mode:
            clair_db_host = rcp.get("configuration", "clair_db_host")
            if "postgres" == clair_db_host:
                raise Exception("Error: In HA mode, clair_db_host in harbor.cfg needs to point to an external Postgres DB address.")

        cert_path = rcp.get("configuration", "ssl_cert")
        cert_key_path = rcp.get("configuration", "ssl_cert_key")
        shared_cert_key = os.path.join(base_dir, "ha", os.path.basename(cert_key_path))
        shared_cert_path = os.path.join(base_dir, "ha", os.path.basename(cert_path))
        if os.path.isfile(shared_cert_key):
            shutil.copy2(shared_cert_key, cert_key_path)
        if os.path.isfile(shared_cert_path):
            shutil.copy2(shared_cert_path, cert_path)

    protocol = rcp.get("configuration", "ui_url_protocol")
    if protocol != "https" and args.notary_mode:
        raise Exception("Error: the protocol must be https when Harbor is deployed with Notary")
    if protocol == "https":
        if not rcp.has_option("configuration", "ssl_cert"):
            raise Exception("Error: The protocol is https but attribute ssl_cert is not set")
        cert_path = rcp.get("configuration", "ssl_cert")
        if not os.path.isfile(cert_path):
            raise Exception("Error: The path for certificate: %s is invalid" % cert_path)
        if not rcp.has_option("configuration", "ssl_cert_key"):
            raise Exception("Error: The protocol is https but attribute ssl_cert_key is not set")
        cert_key_path = rcp.get("configuration", "ssl_cert_key")
        if not os.path.isfile(cert_key_path):
            raise Exception("Error: The path for certificate key: %s is invalid" % cert_key_path)
    project_creation = rcp.get("configuration", "project_creation_restriction")

    if project_creation != "everyone" and project_creation != "adminonly":
        raise Exception("Error invalid value for project_creation_restriction: %s" % project_creation)
{% endhighlight %}
在Harbor HA模式下，首先会检查harbor.cfg配置文件中的如下配置：

* ```db_host```: HA模式下必须是一个外部mysql IP地址

* ```registry_storage_provider_name```: HA模式下必须为一个共享存储系统（例如OSS等）

* ```redis_url```: 必须配置为一个redis集群

* ```notary_mode```、```clair_mode```： HA模式下并不支持这两种模式

* ```ssl_cert```、```ssl_cert_key```: HA模式下，如果在harbor.cfg中配置了这两个选项，则会将相应的文件拷贝到执行prepare脚本目录的```ha```文件夹下。

此外，还会进行如下检查。如果harbor工作在notary模式，则必须采用https； 如果采用https，则必须配置```ssl_cert```以及```ssl_cert_key```； harbor.cfg配置文件中project_creation_restriction的值必须为```everyone```或```adminonly```中的一个（可以在运行后通过Harbor管理后台来进行修改）。

## 2. prepare_ha()函数
{% highlight string %}
def prepare_ha(conf, args):
    #files under ha folder will have high prority
    protocol = rcp.get("configuration", "ui_url_protocol")
    if protocol == "https":
        #copy nginx certificate
        cert_path = rcp.get("configuration", "ssl_cert")
        cert_key_path = rcp.get("configuration", "ssl_cert_key")
        shared_cert_key = os.path.join(base_dir, "ha", os.path.basename(cert_key_path))
        shared_cert_path = os.path.join(base_dir, "ha", os.path.basename(cert_path))
        if os.path.isfile(shared_cert_key):
            shutil.copy2(shared_cert_key, cert_key_path)
        else: 
            if os.path.isfile(cert_key_path):
                shutil.copy2(cert_key_path, shared_cert_key)
        if os.path.isfile(shared_cert_path):
            shutil.copy2(shared_cert_path, cert_path)
        else:
            if os.path.isfile(cert_path):
                shutil.copy2(cert_path, shared_cert_path)
        #check if ca exsit
        cert_ca_path = "/data/ca_download/ca.crt"
        shared_ca_path = os.path.join(base_dir, "ha", os.path.basename(cert_ca_path))
        if os.path.isfile(shared_ca_path):
            shutil.copy2(shared_ca_path, cert_ca_path)
        else:
            if os.path.isfile(cert_ca_path):
                shutil.copy2(cert_ca_path, shared_ca_path)
    #check root.crt and priviate_key.pem
    private_key_pem = os.path.join(config_dir, "ui", "private_key.pem")
    root_crt = os.path.join(config_dir, "registry", "root.crt")
    shared_private_key_pem = os.path.join(base_dir, "ha", "private_key.pem")
    shared_root_crt = os.path.join(base_dir, "ha", "root.crt")
    if os.path.isfile(shared_private_key_pem):
        shutil.copy2(shared_private_key_pem, private_key_pem)
    else:
        if os.path.isfile(private_key_pem):
            shutil.copy2(private_key_pem, shared_private_key_pem)
    if os.path.isfile(shared_root_crt):
        shutil.copy2(shared_root_crt, root_crt)
    else:
        if os.path.isfile(root_crt):
            shutil.copy2(root_crt, shared_root_crt)
    #secretkey
    shared_secret_key = os.path.join(base_dir, "ha", "secretkey")
    secretkey_path = rcp.get("configuration", "secretkey_path") 
    secret_key = os.path.join(secretkey_path, "secretkey")
    if os.path.isfile(shared_secret_key):
        shutil.copy2(shared_secret_key, secret_key)
    else:
        if os.path.isfile(secret_key):
            shutil.copy2(secret_key, shared_secret_key)
{% endhighlight %}
在Harbor HA模式下，如果配置为https， 则会将所需要的证书、私钥拷贝到指定的文件。然后拷贝```root.crt```、```priviate_key.pem```以及```secretkey```到指定位置。

## 3. 其他辅助函数
{% highlight string %}
def get_secret_key(path):
    secret_key = _get_secret(path, "secretkey") 
    if len(secret_key) != 16:
        raise Exception("secret key's length has to be 16 chars, current length: %d" % len(secret_key))
    return secret_key

def get_alias(path):
    alias = _get_secret(path, "defaultalias", length=8)
    return alias

def _get_secret(folder, filename, length=16):
    key_file = os.path.join(folder, filename)
    if os.path.isfile(key_file):
        with open(key_file, 'r') as f:
            key = f.read()
            print("loaded secret from file: %s" % key_file)
        return key
    if not os.path.isdir(folder):
        os.makedirs(folder, mode=0o600)
    key = ''.join(random.choice(string.ascii_letters+string.digits) for i in range(length))  
    with open(key_file, 'w') as f:
        f.write(key)
        print("Generated and saved secret to file: %s" % key_file)
    os.chmod(key_file, 0o600)
    return key

def prep_conf_dir(root, name):
    absolute_path = os.path.join(root, name)
    if not os.path.exists(absolute_path):
        os.makedirs(absolute_path)
    return absolute_path

def render(src, dest, **kw):
    t = Template(open(src, 'r').read())
    with open(dest, 'w') as f:
        f.write(t.substitute(**kw))
    print("Generated configuration file: %s" % dest)
{% endhighlight %}

* 函数get_secret_key(): 用于获得秘钥的内容

* 函数get_alias(): 用于获得别名

* 函数_get_secret(): 用于创建或获取指定的文件。如果该文件存在，则获取指定长度的内容； 否则，创建一个新的文件，并随机产生指定长度的内容写到创建的文件中

* 函数prep_conf_dir(): 准备好配置文件了路径

* 函数render(): 按某种样式产生配置文件。这里用```kw```中的值替换src中的模板，然后写到dest文件中。我们举个例子：
{% highlight string %}
# python
Python 2.7.5 (default, Nov  6 2016, 00:28:07) 
[GCC 4.8.5 20150623 (Red Hat 4.8.5-11)] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> from string import Template
>>> s = Template('There  ${moneyType} is  ${money}')
>>> print s.substitute(moneyType = 'Dollar',money=12)
There  Dollar is  12
>>>
{% endhighlight %}

## 4. 参数检查及相关配置校验
{% highlight string %}
base_dir = os.path.dirname(__file__)
config_dir = os.path.join(base_dir, "common/config")
templates_dir = os.path.join(base_dir, "common/templates")
def delfile(src):
    if os.path.isfile(src):
        try:
            os.remove(src)
            print("Clearing the configuration file: %s" % src)
        except:
            pass
    elif os.path.isdir(src):
        for item in os.listdir(src):
            itemsrc=os.path.join(src,item)
            delfile(itemsrc)

parser = argparse.ArgumentParser()
parser.add_argument('--conf', dest='cfgfile', default=base_dir+'/harbor.cfg',type=str,help="the path of Harbor configuration file")
parser.add_argument('--with-notary', dest='notary_mode', default=False, action='store_true', help="the Harbor instance is to be deployed with notary")
parser.add_argument('--with-clair', dest='clair_mode', default=False, action='store_true', help="the Harbor instance is to be deployed with clair")
parser.add_argument('--ha', dest='ha_mode', default=False, action='store_true', help="the Harbor instance is to be deployed in HA mode")
parser.add_argument('--yes', dest='yes', default=False, action='store_true', help="Answer yes to all questions")
args = parser.parse_args()

delfile(config_dir)
#Read configurations
conf = StringIO.StringIO()
conf.write("[configuration]\n")
conf.write(open(args.cfgfile).read())
conf.seek(0, os.SEEK_SET)
rcp = ConfigParser.RawConfigParser()
rcp.readfp(conf)
validate(rcp, args)
{% endhighlight %}
这里产生的配置文件路径为prepare脚本路径下的```common/config```目录；模板文件路径为```common/templates```。然后定义了一个删除文件的函数delfile()。再接着处理执行prepare脚本传入的参数，当前我们在调用时并未传递任何参数。最后读取配置文件harbor并进行校验。

## 5. 处理相关选项

{% highlight string %}
reload_config = rcp.get("configuration", "reload_config") if rcp.has_option(
    "configuration", "reload_config") else "false"
hostname = rcp.get("configuration", "hostname")
protocol = rcp.get("configuration", "ui_url_protocol")
ui_url = protocol + "://" + hostname
email_identity = rcp.get("configuration", "email_identity")
email_host = rcp.get("configuration", "email_server")
email_port = rcp.get("configuration", "email_server_port")
email_usr = rcp.get("configuration", "email_username")
email_pwd = rcp.get("configuration", "email_password")
email_from = rcp.get("configuration", "email_from")
email_ssl = rcp.get("configuration", "email_ssl")
email_insecure = rcp.get("configuration", "email_insecure")
harbor_admin_password = rcp.get("configuration", "harbor_admin_password")
auth_mode = rcp.get("configuration", "auth_mode")
ldap_url = rcp.get("configuration", "ldap_url")
# this two options are either both set or unset
if rcp.has_option("configuration", "ldap_searchdn"):
    ldap_searchdn = rcp.get("configuration", "ldap_searchdn")
    ldap_search_pwd = rcp.get("configuration", "ldap_search_pwd")
else:
    ldap_searchdn = ""
    ldap_search_pwd = ""
ldap_basedn = rcp.get("configuration", "ldap_basedn")
# ldap_filter is null by default
if rcp.has_option("configuration", "ldap_filter"):
    ldap_filter = rcp.get("configuration", "ldap_filter")
else:
    ldap_filter = ""
ldap_uid = rcp.get("configuration", "ldap_uid")
ldap_scope = rcp.get("configuration", "ldap_scope")
ldap_timeout = rcp.get("configuration", "ldap_timeout")
ldap_verify_cert = rcp.get("configuration", "ldap_verify_cert")
db_password = rcp.get("configuration", "db_password")
db_host = rcp.get("configuration", "db_host")
db_user = rcp.get("configuration", "db_user")
db_port = rcp.get("configuration", "db_port")
self_registration = rcp.get("configuration", "self_registration")
if protocol == "https":
    cert_path = rcp.get("configuration", "ssl_cert")
    cert_key_path = rcp.get("configuration", "ssl_cert_key")
customize_crt = rcp.get("configuration", "customize_crt")
max_job_workers = rcp.get("configuration", "max_job_workers")
token_expiration = rcp.get("configuration", "token_expiration")
proj_cre_restriction = rcp.get("configuration", "project_creation_restriction")
secretkey_path = rcp.get("configuration", "secretkey_path")
if rcp.has_option("configuration", "admiral_url"):
    admiral_url = rcp.get("configuration", "admiral_url")
else:
    admiral_url = ""
clair_db_password = rcp.get("configuration", "clair_db_password")
clair_db_host = rcp.get("configuration", "clair_db_host")
clair_db_port = rcp.get("configuration", "clair_db_port")
clair_db_username = rcp.get("configuration", "clair_db_username")
clair_db = rcp.get("configuration", "clair_db")

uaa_endpoint = rcp.get("configuration", "uaa_endpoint")
uaa_clientid = rcp.get("configuration", "uaa_clientid")
uaa_clientsecret = rcp.get("configuration", "uaa_clientsecret")
uaa_verify_cert = rcp.get("configuration", "uaa_verify_cert")
uaa_ca_cert = rcp.get("configuration", "uaa_ca_cert")

secret_key = get_secret_key(secretkey_path)
log_rotate_count = rcp.get("configuration", "log_rotate_count")
log_rotate_size = rcp.get("configuration", "log_rotate_size")

if rcp.has_option("configuration", "redis_url"):
    redis_url = rcp.get("configuration", "redis_url")
else:
    redis_url = ""

storage_provider_name = rcp.get("configuration", "registry_storage_provider_name").strip()
storage_provider_config = rcp.get("configuration", "registry_storage_provider_config").strip()
# yaml requires 1 or more spaces between the key and value
storage_provider_config = storage_provider_config.replace(":", ": ", 1)

ui_secret = ''.join(random.choice(string.ascii_letters+string.digits) for i in range(16))  
jobservice_secret = ''.join(random.choice(string.ascii_letters+string.digits) for i in range(16))  
{% endhighlight %}
上面基本都是从harbor.cfg中读取配置文件。harbor.cfg配置文件可以分为几个部分，下面我们分别进行讲解（不一定按照上面的排列顺序）：

### 5.1 全局通用配置

全局通用配置主要有：

* ```hostname```:  用于设置访问admin UI以及registry service的IP地址或hostname。不要配置为localhost或者127.0.0.1，因为Harbor需要被外部clients所访问。

* ```ui_url_protocol```: 该协议用于访问UI、token/notification服务，默认情况下是http

* ```max_job_workers```: 在job service中最大的job worker数

* ```customize_crt```: 用于决定是否为registry的token产生证书。假如该字段被设置为```on```，则prepare脚本会产生新的```root cert```以及```private key```，其会被用于在访问registry的token时使用。假如设置为```off```，则会使用默认的key/cert。本选项也会控制notary signer的cert的产生。

* ```ssl_cert```、```ssl_cert_key```: 用于为nginx设置cert以及key的路径。只在protocol被设置为https时使用

* ```secretkey_path```: 秘钥的存放路径

* ```admiral_url```: 管理员URL, 一般不需要进行设置（直接注释掉或设置为NA）

* ```log_rotate_size```: 用于指定日志文件在被删除之前会被rotate的次数。假如本字段设置为0，则老版本的日志会被移除，而不是被rotate

* ```log_rotate_size```: 日志文件只有在达到log_rotate_size时，才会进行rotate。单位可以设置为K/M/G。

上面代码中我们还定义了一个reload_config字段， 该字段在harbor.cfg配置文件中并没有进行定义。因此取值为```false```， 表示adminserver不会重新加载配置(进行复位）


### 5.2 初始化属性配置

如下的一些属性配置只会在第一次启动的时候产生效果，而后续对这些属性的更改需要通过WebUI来完成.

**1） 邮箱设置**

这里邮箱用于发送密码重置的邮件。邮箱服务器通过与客户端主机建立TLS连接，使用给定的username、password来进行认证。

* ```email_identity```: 邮件标识。假如为空的话，则默认为username

* ```email_server```: 邮件服务器地址

* ```email_server_port```: 邮件服务器端口

* ```email_username```: 用户名

* ```email_password```: 密码

* ```email_from```: 显示邮件从什么地址发出

* ```email_ssl```: 是否使用ssl

* ```email_insecure```: 是否insecure

**2) harbor管理员密码**

* ```harbor_admin_password```: 用于设置Harbor admin用户的初始密码，只在Harbor第一次启动时有效。在Harbor第一次启动之后，修改本字段是无效的。如果第一次启动后，要修改Harbor admin密码，请通过WebUI来进行修改。

**3)  用户认证**

* ```auth_mode```: 默认的认证模式是db_auth，此时用户的credentials会存放在本地数据库中；如果要通过LDAP服务器来验证用户的credential, 可以配置为ldap_auth模式。

* ```ldap_url```: ldap endpoint 的URL

* ```ldap_searchdn```: 

* ```ldap_search_pwd```:

* ```ldap_basedn```:

* ```ldap_filter```:

* ```ldap_uid```:

* ```ldap_scope```:

* ```ldap_timeout```:

* ```ldap_verify_cert```:

* ```self_registration```: 是否允许用户自注册功能

* ```token_expiration```: token的过期时间，默认是30分钟

* ```project_creation_restriction```: 该字段用于控制哪些用户具有创建projects的权限。默认值为```everyone```，表示允许所有人创建工程，如果设置为```adminonly```的话，则只有admin用户可以创建工程。

### 5.3 Harbor数据库配置

* ```db_host```: 用于配置Harbor数据的地址，只在需要使用外部数据库的时候才要更改此字段

* ```db_password```: 用于设置Harbor DB的root账户密码， 在实际生产环境中请更改本值

* ```db_port```: 用于设置Harbor DB的端口

* ```db_user```: 用于设置Harbor数据库的用户名



### 5.4 Harbor HA模式Redis配置

* ```redis_url```: 在Harbor HA模式下，需要配置redis服务器的地址

### 5.5 Clair DB配置

* ```clair_db_host```: 用于配置Clair DB主机地址。只在当需要使用外部数据库时，才需要更改此字段

* ```clair_db_password```: Clair的 postgres数据库密码。只在Harbor启用Clair功能时有效。请在部署之前修改此密码，若部署后再进行修改可能会导致Clair API server与Habor无法访问Clair数据库

* ```clair_db_port```: Clair数据库端口

* ```clair_db_username```: Clair数据库用户名

* ```clair_db```: 采用的Clair数据类型

### 5.6 uaa_auth认证模式配置

当认证模式被配置为uaa_auth时，需要配置如下字段：```uaa_endpoint```、```uaa_clientid```、```uaa_clientsecret```、```uaa_verify_cert``` 、```uaa_ca_cert```


### 5.7 docker registry配置
如下是Docker Registry的相关配置：

* ```registry_storage_provider_name```: 本字段的可选值有```filesystem```、```s3```、```gcs```、```azure```等

* ```registry_storage_provider_config```: 本字段是一个以逗号分割的```key: value```对。例如：```key1: value1, key2: value2```。更多可用的配置请参看[https://docs.docker.com/registry/configuration/#storage](https://docs.docker.com/registry/configuration/#storage)

### 5.8 其他
<pre>
ui_secret = ''.join(random.choice(string.ascii_letters+string.digits) for i in range(16))  
jobservice_secret = ''.join(random.choice(string.ascii_letters+string.digits) for i in range(16))  
</pre>
这里还会产生相关秘钥，用于不同服务之间的通信

## 6. 产生配置文件存放目录
{% highlight string %}
adminserver_config_dir = os.path.join(config_dir,"adminserver")
if not os.path.exists(adminserver_config_dir):
    os.makedirs(os.path.join(config_dir, "adminserver"))

ui_config_dir = prep_conf_dir(config_dir,"ui")
ui_certificates_dir =  prep_conf_dir(ui_config_dir,"certificates")
db_config_dir = prep_conf_dir(config_dir, "db")
job_config_dir = prep_conf_dir(config_dir, "jobservice")
registry_config_dir = prep_conf_dir(config_dir, "registry")
nginx_config_dir = prep_conf_dir (config_dir, "nginx")
nginx_conf_d = prep_conf_dir(nginx_config_dir, "conf.d")
log_config_dir = prep_conf_dir (config_dir, "log")

adminserver_conf_env = os.path.join(config_dir, "adminserver", "env")
ui_conf_env = os.path.join(config_dir, "ui", "env")
ui_conf = os.path.join(config_dir, "ui", "app.conf")
ui_cert_dir = os.path.join(config_dir, "ui", "certificates")
jobservice_conf = os.path.join(config_dir, "jobservice", "app.conf")
registry_conf = os.path.join(config_dir, "registry", "config.yml")
db_conf_env = os.path.join(config_dir, "db", "env")
job_conf_env = os.path.join(config_dir, "jobservice", "env")
nginx_conf = os.path.join(config_dir, "nginx", "nginx.conf")
cert_dir = os.path.join(config_dir, "nginx", "cert")
log_rotate_config = os.path.join(config_dir, "log", "logrotate.conf") 
{% endhighlight %}
这里默认的```config_dir```为common/config， 默认的templates_dir为common/templates。因此:

* ```adminserver配置```: 存放路径为common/config/adminserver/env

* ```ui env配置```: 存放路径为common/config/ui/env

* ```ui配置```: 存放路径为common/config/ui/app.conf

* ```ui_cert_dir```: 用于存放UI证书的路径，为common/config/ui/certificates

* ```jobservice配置```: 存放路径为common/config/jobservice/app.conf

* ```registry配置```: 存放路径为common/config/registry/config.yml

* ```db env配置```: 存放路径为common/config/db/env 

* ```job env配置```: 存放路径为common/config/jobservice/env

* ```nginx配置```: 存放路径为common/config/nginx/nginx.conf

* ```cert存放目录```: 路径为common/config/nginx/cert

* ```日志配置```: 存放路径为common/config/log/logrotate.conf

## 7. 生成相应配置文件

**1) 生成nginx配置文件**
{% highlight string %}
if protocol == "https":
    target_cert_path = os.path.join(cert_dir, os.path.basename(cert_path))
    if not os.path.exists(cert_dir):
        os.makedirs(cert_dir)
    shutil.copy2(cert_path,target_cert_path)
    target_cert_key_path = os.path.join(cert_dir, os.path.basename(cert_key_path))
    shutil.copy2(cert_key_path,target_cert_key_path)
    render(os.path.join(templates_dir, "nginx", "nginx.https.conf"),
            nginx_conf,
            ssl_cert = os.path.join("/etc/nginx/cert", os.path.basename(target_cert_path)),
            ssl_cert_key = os.path.join("/etc/nginx/cert", os.path.basename(target_cert_key_path)))
else:
    render(os.path.join(templates_dir, "nginx", "nginx.http.conf"),
        nginx_conf)
{% endhighlight %}
这里如果配置为https模式，则会将harbor.cfg配置文件中指定的证书、秘钥拷贝到common/config/nginx/cert目录中，然后再以nginx.https.conf为模板生成nginx配置文件； 否则以nginx.http.conf为模板生成nginx配置文件。

**2) 生成admin server环境变量配置**
{% highlight string %}
render(os.path.join(templates_dir, "adminserver", "env"),
        adminserver_conf_env,
        reload_config=reload_config,
        ui_url=ui_url,
        auth_mode=auth_mode,
        self_registration=self_registration,
        ldap_url=ldap_url,
        ldap_searchdn =ldap_searchdn, 
        ldap_search_pwd =ldap_search_pwd,
        ldap_basedn=ldap_basedn,
        ldap_filter=ldap_filter,
        ldap_uid=ldap_uid,
        ldap_scope=ldap_scope,
        ldap_verify_cert=ldap_verify_cert,
        ldap_timeout=ldap_timeout,
        db_password=db_password,
        db_host=db_host,
        db_user=db_user,
        db_port=db_port,
        email_host=email_host,
        email_port=email_port,
        email_usr=email_usr,
        email_pwd=email_pwd,
        email_ssl=email_ssl,
        email_insecure=email_insecure,
        email_from=email_from,
        email_identity=email_identity,
        harbor_admin_password=harbor_admin_password,
        project_creation_restriction=proj_cre_restriction,
        max_job_workers=max_job_workers,
        ui_secret=ui_secret,
        jobservice_secret=jobservice_secret,
        token_expiration=token_expiration,
        admiral_url=admiral_url,
        with_notary=args.notary_mode,
        with_clair=args.clair_mode,
        clair_db_password=clair_db_password,
        clair_db_host=clair_db_host,
        clair_db_port=clair_db_port,
        clair_db_username=clair_db_username,
        clair_db=clair_db,
        uaa_endpoint=uaa_endpoint,
        uaa_clientid=uaa_clientid,
        uaa_clientsecret=uaa_clientsecret,
        uaa_verify_cert=uaa_verify_cert,
        storage_provider_name=storage_provider_name
	)

{% endhighlight %}
这是作为整个Harbor管理后台的相关配置。

**3) registry配置**
{% highlight string %}
registry_config_file = "config_ha.yml" if args.ha_mode else "config.yml"
if storage_provider_name == "filesystem":
    if not storage_provider_config:
        storage_provider_config = "rootdirectory: /storage"
    elif "rootdirectory:" not in storage_provider_config:
        storage_provider_config = "rootdirectory: /storage" + "," + storage_provider_config
# generate storage configuration section in yaml format
storage_provider_info = ('\n' + ' ' * 4).join(
    [storage_provider_name + ':'] + map(string.strip, storage_provider_config.split(",")))
render(os.path.join(templates_dir, "registry", registry_config_file),
    registry_conf,
    storage_provider_info=storage_provider_info,
    ui_url=ui_url,
    redis_url=redis_url)
{% endhighlight %}
这里用于产生Docker Registry的配置文件

**4) 产生DB配置文件**
{% highlight string %}
render(os.path.join(templates_dir, "db", "env"),
        db_conf_env,
        db_password=db_password)
{% endhighlight %}

**5) 产生jobservice env配置**
{% highlight string %}
render(os.path.join(templates_dir, "jobservice", "env"),
        job_conf_env,
        ui_secret=ui_secret,
        jobservice_secret=jobservice_secret)
{% endhighlight %}

**6) 产生日志服务配置**
{% highlight string %}
render(os.path.join(templates_dir, "log", "logrotate.conf"),
        log_rotate_config,
        log_rotate_count=log_rotate_count,
		log_rotate_size=log_rotate_size)
{% endhighlight %}

**7) 产生jobservice配置文件**
{% highlight string %}
print("Generated configuration file: %s" % jobservice_conf)
shutil.copyfile(os.path.join(templates_dir, "jobservice", "app.conf"), jobservice_conf)
{% endhighlight %}

**8) 产生ui配置文件**
{% highlight string %}
print("Generated configuration file: %s" % ui_conf)
shutil.copyfile(os.path.join(templates_dir, "ui", "app.conf"), ui_conf)
{% endhighlight %}


## 8. 产生证书、秘钥文件
{% highlight string %}
if auth_mode == "uaa_auth":
    if os.path.isfile(uaa_ca_cert):
        if not os.path.isdir(ui_cert_dir):
            os.makedirs(ui_cert_dir, mode=0o600)
        ui_uaa_ca = os.path.join(ui_cert_dir, "uaa_ca.pem")
        print("Copying UAA CA cert to %s" % ui_uaa_ca)
        shutil.copyfile(uaa_ca_cert, ui_uaa_ca)
    else:
        print("Can not find UAA CA cert: %s, skip" % uaa_ca_cert)


def validate_crt_subj(dirty_subj):
    subj_list = [item for item in dirty_subj.strip().split("/") \
        if len(item.split("=")) == 2 and len(item.split("=")[1]) > 0]
    return "/" + "/".join(subj_list)

FNULL = open(os.devnull, 'w')

from functools import wraps
def stat_decorator(func):
    @wraps(func)
    def check_wrapper(*args, **kw):
        stat = func(*args, **kw)
        message = "Generated certificate, key file: %s, cert file: %s" % (kw['key_path'], kw['cert_path']) \
                if stat == 0 else "Fail to generate key file: %s, cert file: %s" % (kw['key_path'], kw['cert_path'])
        print(message)
        if stat != 0:
            sys.exit(1)
    return check_wrapper

@stat_decorator
def create_root_cert(subj, key_path="./k.key", cert_path="./cert.crt"):
   rc = subprocess.call(["openssl", "genrsa", "-out", key_path, "4096"], stdout=FNULL, stderr=subprocess.STDOUT)
   if rc != 0:
        return rc
   return subprocess.call(["openssl", "req", "-new", "-x509", "-key", key_path,\
        "-out", cert_path, "-days", "3650", "-subj", subj], stdout=FNULL, stderr=subprocess.STDOUT)

@stat_decorator
def create_cert(subj, ca_key, ca_cert, key_path="./k.key", cert_path="./cert.crt"):
    cert_dir = os.path.dirname(cert_path)
    csr_path = os.path.join(cert_dir, "tmp.csr")
    rc = subprocess.call(["openssl", "req", "-newkey", "rsa:4096", "-nodes","-sha256","-keyout", key_path,\
        "-out", csr_path, "-subj", subj], stdout=FNULL, stderr=subprocess.STDOUT)
    if rc != 0:
        return rc
    return subprocess.call(["openssl", "x509", "-req", "-days", "3650", "-in", csr_path, "-CA", \
        ca_cert, "-CAkey", ca_key, "-CAcreateserial", "-out", cert_path], stdout=FNULL, stderr=subprocess.STDOUT)

def openssl_installed():
    shell_stat = subprocess.check_call(["which", "openssl"], stdout=FNULL, stderr=subprocess.STDOUT)
    if shell_stat != 0:
        print("Cannot find openssl installed in this computer\nUse default SSL certificate file")
        return False
    return True
        

if customize_crt == 'on' and openssl_installed():
    shell_stat = subprocess.check_call(["which", "openssl"], stdout=FNULL, stderr=subprocess.STDOUT)
    empty_subj = "/C=/ST=/L=/O=/CN=/"
    private_key_pem = os.path.join(config_dir, "ui", "private_key.pem")
    root_crt = os.path.join(config_dir, "registry", "root.crt")
    create_root_cert(empty_subj, key_path=private_key_pem, cert_path=root_crt)
    os.chmod(private_key_pem, 0o600)
    os.chmod(root_crt, 0o600)
else:
    print("Copied configuration file: %s" % ui_config_dir + "private_key.pem")
    shutil.copyfile(os.path.join(templates_dir, "ui", "private_key.pem"), os.path.join(ui_config_dir, "private_key.pem"))
    print("Copied configuration file: %s" % registry_config_dir + "root.crt")
    shutil.copyfile(os.path.join(templates_dir, "registry", "root.crt"), os.path.join(registry_config_dir, "root.crt"))

if args.notary_mode:
    notary_config_dir = prep_conf_dir(config_dir, "notary")
    notary_temp_dir = os.path.join(templates_dir, "notary") 
    print("Copying sql file for notary DB")
    if os.path.exists(os.path.join(notary_config_dir, "mysql-initdb.d")):
        shutil.rmtree(os.path.join(notary_config_dir, "mysql-initdb.d"))
    shutil.copytree(os.path.join(notary_temp_dir, "mysql-initdb.d"), os.path.join(notary_config_dir, "mysql-initdb.d")) 
    if customize_crt == 'on' and openssl_installed():
        try:
            temp_cert_dir = os.path.join(base_dir, "cert_tmp")
            if not os.path.exists(temp_cert_dir):
                os.makedirs(temp_cert_dir)
            ca_subj = "/C=US/ST=California/L=Palo Alto/O=VMware, Inc./OU=Harbor/CN=Self-signed by VMware, Inc."
            cert_subj = "/C=US/ST=California/L=Palo Alto/O=VMware, Inc./OU=Harbor/CN=notarysigner"
            signer_ca_cert = os.path.join(temp_cert_dir, "notary-signer-ca.crt")
            signer_ca_key = os.path.join(temp_cert_dir, "notary-signer-ca.key")
            signer_cert_path = os.path.join(temp_cert_dir, "notary-signer.crt")
            signer_key_path = os.path.join(temp_cert_dir, "notary-signer.key")
            create_root_cert(ca_subj, key_path=signer_ca_key, cert_path=signer_ca_cert)
            create_cert(cert_subj, signer_ca_key, signer_ca_cert, key_path=signer_key_path, cert_path=signer_cert_path)
            print("Copying certs for notary signer")
            os.chmod(signer_cert_path, 0o600)
            os.chmod(signer_key_path, 0o600)
            os.chmod(signer_ca_cert, 0o600)
            shutil.copy2(signer_cert_path, notary_config_dir)
            shutil.copy2(signer_key_path, notary_config_dir)
            shutil.copy2(signer_ca_cert, notary_config_dir)
        finally:
            srl_tmp = os.path.join(os.getcwd(), ".srl")
            if os.path.isfile(srl_tmp):
                os.remove(srl_tmp)
            if os.path.isdir(temp_cert_dir):
                shutil.rmtree(temp_cert_dir, True)
    else:
        print("Copying certs for notary signer")
        shutil.copy2(os.path.join(notary_temp_dir, "notary-signer.crt"), notary_config_dir)
        shutil.copy2(os.path.join(notary_temp_dir, "notary-signer.key"), notary_config_dir)
        shutil.copy2(os.path.join(notary_temp_dir, "notary-signer-ca.crt"), notary_config_dir)
    shutil.copy2(os.path.join(registry_config_dir, "root.crt"), notary_config_dir)
    print("Copying notary signer configuration file")
    shutil.copy2(os.path.join(notary_temp_dir, "signer-config.json"), notary_config_dir)
    render(os.path.join(notary_temp_dir, "server-config.json"),
        os.path.join(notary_config_dir, "server-config.json"),
        token_endpoint=ui_url)

    print("Copying nginx configuration file for notary")
    shutil.copy2(os.path.join(templates_dir, "nginx", "notary.upstream.conf"), nginx_conf_d)
    render(os.path.join(templates_dir, "nginx", "notary.server.conf"), 
            os.path.join(nginx_conf_d, "notary.server.conf"), 
            ssl_cert = os.path.join("/etc/nginx/cert", os.path.basename(target_cert_path)),
            ssl_cert_key = os.path.join("/etc/nginx/cert", os.path.basename(target_cert_key_path)))

    default_alias = get_alias(secretkey_path)
    render(os.path.join(notary_temp_dir, "signer_env"), os.path.join(notary_config_dir, "signer_env"), alias = default_alias)

if args.clair_mode:
    clair_temp_dir = os.path.join(templates_dir, "clair")
    clair_config_dir = prep_conf_dir(config_dir, "clair")
    if os.path.exists(os.path.join(clair_config_dir, "postgresql-init.d")):
        print("Copying offline data file for clair DB")
        shutil.rmtree(os.path.join(clair_config_dir, "postgresql-init.d"))
    shutil.copytree(os.path.join(clair_temp_dir, "postgresql-init.d"), os.path.join(clair_config_dir, "postgresql-init.d"))
    postgres_env = os.path.join(clair_config_dir, "postgres_env") 
    render(os.path.join(clair_temp_dir, "postgres_env"), postgres_env, password = clair_db_password)
    clair_conf = os.path.join(clair_config_dir, "config.yaml")
    render(os.path.join(clair_temp_dir, "config.yaml"),
            clair_conf,
            password = clair_db_password,
            username = clair_db_username,
            host = clair_db_host,
            port = clair_db_port)

if args.ha_mode:
    prepare_ha(rcp, args)

FNULL.close()
print("The configuration files are ready, please use docker-compose to start the service.")
{% endhighlight %}

如果认证模式为```uaa_auth```,则会将harbor.cfg中的```uaa_ca_cert```配置指定的cert文件拷贝到common/config/ui/certificates目录下。

另外，如果harbor.cfg中```customize_crt```被配置为on，且当前系统安装了openssl，则会自动产生一对cert/key(自签名证书）； 否则会采用模板中的cert/key。在Registry所连接的Auth token服务中，其用private_key来对所产生的token进行数字签名，然后Registry接收到Token后用root.crt来进行校验。




<br />
<br />

**[参考]**

1. [LDAP目录服务折腾之后的总结](https://www.cnblogs.com/wadeyu/p/ldap-search-summary.html)

2. [Docker Registry V2 Auth Server](https://hui.lu/docker-registry-v2-auth-server-with-python/)


<br />
<br />
<br />

