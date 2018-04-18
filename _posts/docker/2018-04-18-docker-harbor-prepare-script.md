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

选项比较多，我们分几根部分来讲解：
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
{% endhighlight %}
在harbor.cfg配置文件中，reload_config默认是没有定义的，因此取值为```false```， 表示adminserver不会重新加载配置。







<br />
<br />
<br />

