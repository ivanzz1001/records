---
layout: post
title: Harbor基于Bearer的验证
tags:
- docker
categories: docker
description: Harbor基于Bearer的验证
---

这里我们介绍一下Harbor基于Bearer的验证。

<!-- more -->


## 1. Bearer简介



## 2. Harbor中Bearer认证条件下API测试

Harbor默认安装时，采用Bearer认证。下面我们来测试一下Harbore-registry如下三个API，并借以了解Bearer认证的大体流程：

|   Method   |         Path                    |    Entity   |                     Description            |
|:----------:|:-------------------------------:|:-----------:|:-------------------------------------------:
|     GET    |/v2/                             |Base         | API版本检测                                 |
|     GET    |/v2/<name>/tags/list             |Tags         | 列出镜像标签                                 |
|     GET    |/v2/<name>/manifests/<reference> |Manifest     | 拉取一个镜像的MANIFEST                       |
|     GET    |/v2/_catalog                     |Base         | 查询一个仓库中的镜像                          |

首先我们测试一下```/v2```:
<pre>
# curl -ikL -X GET http://192.168.69.128/v2
HTTP/1.1 301 Moved Permanently
Server: nginx
Date: Mon, 09 Apr 2018 09:09:26 GMT
Content-Type: text/html
Content-Length: 178
Location: http://192.168.69.128/v2/
Connection: keep-alive

HTTP/1.1 401 Unauthorized
Server: nginx
Date: Mon, 09 Apr 2018 09:09:26 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 87
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=f7c73dc9e006a967b95c514014ac49c1; Path=/; HttpOnly
Www-Authenticate: Bearer realm="http://192.168.69.128/service/token",service="harbor-registry"

{"errors":[{"code":"UNAUTHORIZED","message":"authentication required","detail":null}]}
</pre>
我们发现提示```Unauthorized```。通过查看相关文档及上面的错误提示，我们应该先获取token，然后再进行访问。

### 2.1  查询当前registry API版本号

1) **获取token**
<pre>
# curl -ikL -X GET -u admin:Harbor12345 http://192.168.69.128/service/token?account=admin\&service=harbor-registry
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:22:53 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1100
Connection: keep-alive
Set-Cookie: beegosessionID=946e433f64d7b3f6f25f1c194de1573b; Path=/; HttpOnly

{
  "token": "_ThyL4OfJUCg",
  "expires_in": 1800,
  "issued_at": "2018-04-09T09:22:53Z"
}
</pre>
注意上面为了显示，我们对```token```字段进行了适当的裁剪。

2) **查询API版本号**
<pre>
[root@localhost test]# curl -ikL -X GET -H "Content-Type: application/json" -H "Authorization: Bearer _ThyL4OfJUCg" http://192.168.69.128/v2
HTTP/1.1 301 Moved Permanently
Server: nginx
Date: Mon, 09 Apr 2018 09:27:01 GMT
Content-Type: text/html
Content-Length: 178
Location: http://192.168.69.128/v2/
Connection: keep-alive

HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:27:01 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 2
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=192d5078479411d2c59fa4318b31a3ea; Path=/; HttpOnly
</pre>
可以看到上面返回```200 OK```，表明当前所用registry API确实为v2版本。(注意上面```Bearer```后面为完整的token值，这里进行了适当裁剪)

### 2.2 查询一个仓库中的镜像
<pre>
# curl -ikL -X GET http://192.168.69.128/v2/_catalog
HTTP/1.1 401 Unauthorized
Server: nginx
Date: Mon, 09 Apr 2018 09:31:43 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 145
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=7456246aef9ca966c37848f3232f16f8; Path=/; HttpOnly
Www-Authenticate: Bearer realm="http://192.168.69.128/service/token",service="harbor-registry",scope="registry:catalog:*"

{"errors":[{"code":"UNAUTHORIZED","message":"authentication required","detail":[{"Type":"registry","Class":"","Name":"catalog","Action":"*"}]}]}
</pre>

这里我们看到提示```Unauthorized```错误。因此下面我们要获取相应token，然后再访问。

**1) 获取token**
<pre>
# curl -ikL -X GET -u admin:Harbor12345 http://192.168.69.128/service/token?account=admin\&service=harbor-registry\&scope=registry:catalog:*
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:33:52 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1166
Connection: keep-alive
Set-Cookie: beegosessionID=648fd5a5ec4f06389d45c02f7f5971b4; Path=/; HttpOnly

{
  "token": "A7yfEdUBYD3bDhLM",
  "expires_in": 1800,
  "issued_at": "2018-04-09T09:33:52Z"
</pre>
注意上面为了显示，我们对```token```字段进行了适当的裁剪。

**2） 查询仓库中的镜像**
<pre>
# curl -ikL -X GET -H "Content-Type: application/json" -H "Authorization: Bearer LA7yfEdUBYD3bDhLM" http://192.168.69.128/v2/_catalog
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:36:35 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 34
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=1b84e760ab0234045f06680e56e28818; Path=/; HttpOnly

{"repositories":["library/redis"]}
</pre>
上面我们看到返回仓库中的镜像有```library/redis```。(注意上面```Bearer```后面为完整的token值，这里进行了适当裁剪)

### 2.3  查看镜像标签
<pre>
# curl -ikL -X GET http://192.168.69.128/v2/library/redis/tags/list
HTTP/1.1 401 Unauthorized
Server: nginx
Date: Mon, 09 Apr 2018 09:41:32 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 156
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=c3054d54b29b37572ae507b7a39341a7; Path=/; HttpOnly
Www-Authenticate: Bearer realm="http://192.168.69.128/service/token",service="harbor-registry",scope="repository:library/redis:pull"

{"errors":[{"code":"UNAUTHORIZED","message":"authentication required","detail":[{"Type":"repository","Class":"","Name":"library/redis","Action":"pull"}]}]}
</pre>

**1) 获取token**
<pre>
# curl -ikL -X GET -u admin:Harbor12345 http://192.168.69.128/service/token?account=admin\&service=harbor-registry\&scope=repository:library/redis:pull
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:42:37 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1196
Connection: keep-alive
Set-Cookie: beegosessionID=2fb20f4b6188c6c5aaafcffe2408bc88; Path=/; HttpOnly

{
  "token": "PZxiZYBNkaxp78fs",
  "expires_in": 1800,
  "issued_at": "2018-04-09T09:42:37Z"
}
</pre>
注意上面为了显示，我们对```token```字段进行了适当的裁剪。

**2) 查询镜像标签**
<pre>
# curl -ikL -X GET -H "Content-Type: application/json" -H "Authorization: Bearer PZxiZYBNkaxp78fs" http://192.168.69.128/v2/library/redis/tags/list
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:44:25 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 43
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=bae4b316f5ffe1f41df9ac45b51736fa; Path=/; HttpOnly

{"name":"library/redis","tags":["alpine"]}
</pre>
上面我们看到```library/redis```镜像的标签为```alpine```。(注意上面```Bearer```后面为完整的token值，这里进行了适当裁剪)

### 2.4 获取镜像Manifest
<pre>
# curl -ikL -X GET  http://192.168.69.128/v2/library/redis/manifests/latest
HTTP/1.1 401 Unauthorized
Server: nginx
Date: Mon, 09 Apr 2018 09:48:41 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 156
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=68be9fa8be88c2627f1c2a7b73aff7ab; Path=/; HttpOnly
Www-Authenticate: Bearer realm="http://192.168.69.128/service/token",service="harbor-registry",scope="repository:library/redis:pull"

{"errors":[{"code":"UNAUTHORIZED","message":"authentication required","detail":[{"Type":"repository","Class":"","Name":"library/redis","Action":"pull"}]}]}
</pre>

**1) 获得token**
<pre>
# curl -ikL -X GET -u admin:Harbor12345 http://192.168.69.128/service/token?account=admin\&service=harbor-registry\&scope=repository:library/redis:pull
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:49:51 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1196
Connection: keep-alive
Set-Cookie: beegosessionID=ce243085ff01770bad8aa8751a9b4e7a; Path=/; HttpOnly

{
  "token": "DGYtO4VfXttRh_WNs",
  "expires_in": 1800,
  "issued_at": "2018-04-09T09:49:51Z"
}
</pre>
注意上面为了显示，我们对```token```字段进行了适当的裁剪。

**2) 获取镜像Manifest**
<pre>
# curl -ikL -X GET -H "Accept: application/vnd.docker.distribution.manifest.v2+json" -H "Authorization: Bearer DGYtO4VfXttRh_WNs" http://192.168.69.128/v2/library/redis/manifests/alpine
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:57:51 GMT
Content-Type: application/vnd.docker.distribution.manifest.v2+json
Content-Length: 1568
Connection: keep-alive
Docker-Content-Digest: sha256:9d017f829df3d0800f2a2582c710143767f6dda4df584b708260e73b1a1b6db3
Docker-Distribution-Api-Version: registry/2.0
Etag: "sha256:9d017f829df3d0800f2a2582c710143767f6dda4df584b708260e73b1a1b6db3"
Set-Cookie: beegosessionID=0bcee40b8b46feaffa29d024e32f8d5c; Path=/; HttpOnly

{
   "schemaVersion": 2,
   "mediaType": "application/vnd.docker.distribution.manifest.v2+json",
   "config": {
      "mediaType": "application/vnd.docker.container.image.v1+json",
      "size": 5084,
      "digest": "sha256:c27f565859388a7a6b4666c7861d9a8cac3f6eec6a2fd296a39fd4895275344d"
   },
   "layers": [
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 2065537,
         "digest": "sha256:ff3a5c916c92643ff77519ffa742d3ec61b7f591b6b7504599d95a4a41134e28"
      },
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 1252,
         "digest": "sha256:aae70a2e60279ffae89150a59b81fe10d1d81f341ef6f31b9714ea6cc3418577"
      },
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 8554,
         "digest": "sha256:87c655da471c9a7d8f946ec7b04a6a72a98ae8c1734bddf4b950861b5638fe20"
      },
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 8497514,
         "digest": "sha256:6c09203c8aba31fcd20a3a434a3ee9b94fd7a0a2bc52e1f1cbfc4f1db053da08"
      },
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 98,
         "digest": "sha256:90b6d4891e7fceff0dad2e9dc885d06b932ab6095f34f72ddc774e93fe4258ab"
      },
      {
         "mediaType": "application/vnd.docker.image.rootfs.diff.tar.gzip",
         "size": 402,
         "digest": "sha256:ffb22fabb597331e68f3edea917d3dba9cb8868d31dd6cf5b9330a9e3e1c8e4e"
      }
   ]
}
</pre>
上面我们获取到了```library/redis:alpine```镜像的manifests。(注意上面```Bearer```后面为完整的token值，这里进行了适当裁剪)


<br />
<br />

**[参考]**

1. [Harbor FAQs](https://github.com/vmware/harbor/wiki/Harbor-FAQs)

2. [Docker Registry v2 Bearer token specification](http://192.168.69.128:9000/registry/spec/auth/jwt/)

3. [The OAuth 2.0 Authorization Framework: Bearer Token Usage](http://www.rfcreader.com/#rfc6750)

4. [The OAuth 2.0 Authorization Framework](http://www.rfcreader.com/#rfc6749)

<br />
<br />
<br />

