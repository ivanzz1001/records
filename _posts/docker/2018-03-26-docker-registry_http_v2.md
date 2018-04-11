---
layout: post
title: docker镜像仓库HTTP API V2
tags:
- docker
categories: docker
description: docker镜像仓库HTTP API V2
---


本节我们介绍一下docker registry HTTP API V2，然后给出几个调用API的实例。在这里为了方便，我们首先dump下docker的官方文档：
<pre>
# docker pull docs/docker.github.io:latest
# docker run -itd -p 80:4000 docs/docker.github.io:latest
</pre>



<!-- more -->


## 1. docker镜像仓库HTTP API V2

|   Method   |         Path                    |    Entity   |                     Description            |
|:----------:|:-------------------------------:|:-----------:|:-------------------------------------------:
|     GET    |/v2/                             |Base         |                                            |
|     GET    |/v2/<name>/tags/list             |Tags         |                                            |
|     GET    |/v2/<name>/manifests/<reference> |Manifest     |                                            |




## 2. htpasswd认证条件下API测试

这里为了测试方便，我们暂时去掉https，只在htpasswd认证条件下对API进行测试。如下是我们的启动脚本：
{% highlight string %}
/usr/bin/docker run -d -p 5000:5000 --privileged=true --restart=always \
 -v /opt/docker-registry:/var/lib/registry \
 -v /opt/docker-auth:/auth \
 -e "REGISTRY_AUTH=htpasswd" \
 -e "REGISTRY_AUTH_HTPASSWD_REALM=Registry Realm" \
 -e REGISTRY_AUTH_HTPASSWD_PATH=/auth/htpasswd \
 --name $registry_name \
 registry:2.6.2
{% endhighlight %}

首先我们测试一下```/v2```:
<pre>
# curl -X GET http://10.17.153.196:5000/v2 -k -IL
HTTP/1.1 301 Moved Permanently
Docker-Distribution-Api-Version: registry/2.0
Location: /v2/
Date: Mon, 26 Mar 2018 11:33:38 GMT
Content-Length: 39
Content-Type: text/html; charset=utf-8

HTTP/1.1 401 Unauthorized
Content-Type: application/json; charset=utf-8
Docker-Distribution-Api-Version: registry/2.0
Www-Authenticate: Basic realm="Registry Realm"
X-Content-Type-Options: nosniff
Date: Mon, 26 Mar 2018 11:33:38 GMT
Content-Length: 87
</pre>
我们发现提示```Unauthorized```。查看官方文档，我们只找到基于```auth token```方式的认证。这里我们通过执行docker login命令，采用tcpdump抓取到如下包：
<pre>
GET /v2/ HTTP/1.1
Host: 10.17.153.196:5000
User-Agent: docker/17.12.1-ce go/go1.9.4 git-commit/7390fc6 kernel/3.10.0-514.el7.x86_64 os/linux arch/amd64 UpstreamClient(Docker-Client/17.12.1-ce \(linux\))
Authorization: Basic YWRtaW46TWlkZWFAMTIz
Accept-Encoding: gzip
Connection: close
</pre>
从上面我们可以看到在发送到docker registry中的请求中有```Authorization: Basic YWRtaW46TWlkZWFAMTIz```头。我们猜想```YWRtaW46TWlkZWFAMTIz```正是我们输入的用户名密码。但是该用户名密码到底是如何变成该字符串的呢？ 

这里我们分析docker官方源代码docker/registry/auth.go，发现有如下函数：
{% highlight string %}
func loginV2(authConfig *types.AuthConfig, endpoint APIEndpoint, userAgent string) (string, string, error) {
     ...
     loginClient, foundV2, err := v2AuthHTTPClient(endpoint.URL, authTransport, modifiers, creds, nil)
     ....
}
{% endhighlight %}
再跟踪v2AuthHTTPClient()函数,最后会在vendor/github.com/docker/distribution/registry/client/auth/session.go中找到如下：
{% highlight string %}
func (bh *basicHandler) AuthorizeRequest(req *http.Request, params map[string]string) error {
	if bh.creds != nil {
		username, password := bh.creds.Basic(req.URL)
		if username != "" && password != "" {
			req.SetBasicAuth(username, password)
			return nil
		}
	}
	return ErrNoBasicAuthCredentials
}
{% endhighlight %}
其实就是对```username + ":" + password```做base64。

有了上面的基础我们再对docker registry的HTTP API做一些测试：

**1) 获得docker registry API版本**
<pre>
# curl -X GET -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" http://10.17.153.196:5000/v2 -k -IL
HTTP/1.1 301 Moved Permanently
Docker-Distribution-Api-Version: registry/2.0
Location: /v2/
Date: Mon, 26 Mar 2018 11:56:38 GMT
Content-Length: 39
Content-Type: text/html; charset=utf-8

HTTP/1.1 200 OK
Content-Length: 2
Content-Type: application/json; charset=utf-8
Docker-Distribution-Api-Version: registry/2.0
X-Content-Type-Options: nosniff
Date: Mon, 26 Mar 2018 11:56:38 GMT
</pre>
上面我们看到返回```200 OK```，测试成功，说明API版本是v2

**2) /v2/_catalog获得当前仓库中的镜像**
<pre>
# curl -X GET -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" -H "Accept: application/json" http://10.17.153.196:5000/v2/_catalog
{"repositories":["busybox","centos-sshd"]}
</pre>

**3) ```/v2/<name>/tags/list```获得镜像标签**
<pre>
# curl -X GET -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" -H "Accept: */*" http://10.17.153.196:5000/v2/busybox/tags/list
{"name":"busybox","tags":["latest"]}

# curl -X GET -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" -H "Accept: */*" http://10.17.153.196:5000/v2/centos-sshd/tags/list
{"name":"centos-sshd","tags":["latest"]}
</pre>

4) **获得一个镜像的Manifest**
{% highlight string %}
# curl -X GET -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" http://10.17.153.196:5000/v2/busybox/manifests/latest
{
   "schemaVersion": 1,
   "name": "busybox",
   "tag": "latest",
   "architecture": "amd64",
   "fsLayers": [
      {
         "blobSum": "sha256:a3ed95caeb02ffe68cdd9fd84406680ae93d633cb16422d00e8a7c22955b46d4"
      },
      {
         "blobSum": "sha256:d070b8ef96fc4f2d92ff520a4fe55594e362b4e1076a32bbfeb261dc03322910"
      }
   ],
   "history": [
      {
         "v1Compatibility": "{\"architecture\":\"amd64\",\"config\":{\"Hostname\":\"\",\"Domainname\":\"\",\"User\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"sh\"],\"ArgsEscaped\":true,\"Image\":\"sha256:8cae5980d887cc55ba2f978ae99c662007ee06d79881678d57f33f0473fe0736\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"OnBuild\":null,\"Labels\":null},\"container\":\"8d2c840a1a9b2544fe713c2e24b6757d52328f09bdfc9c2ef6219afbf7ae6b59\",\"container_config\":{\"Hostname\":\"8d2c840a1a9b\",\"Domainname\":\"\",\"User\":\"\",\"AttachStdin\":false,\"AttachStdout\":false,\"AttachStderr\":false,\"Tty\":false,\"OpenStdin\":false,\"StdinOnce\":false,\"Env\":[\"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin\"],\"Cmd\":[\"/bin/sh\",\"-c\",\"#(nop) \",\"CMD [\\\"sh\\\"]\"],\"ArgsEscaped\":true,\"Image\":\"sha256:8cae5980d887cc55ba2f978ae99c662007ee06d79881678d57f33f0473fe0736\",\"Volumes\":null,\"WorkingDir\":\"\",\"Entrypoint\":null,\"OnBuild\":null,\"Labels\":{}},\"created\":\"2018-02-28T22:14:49.023807051Z\",\"docker_version\":\"17.06.2-ce\",\"id\":\"02d3847f0b0fb7acd4419040cc53febf91cb112db2451d9b27a245dee5b227c0\",\"os\":\"linux\",\"parent\":\"b033c560078fa8427a78f311ec37bcac1c0d141178e51e036785db6cd1a60d8c\",\"throwaway\":true}"
      },
      {
         "v1Compatibility": "{\"id\":\"b033c560078fa8427a78f311ec37bcac1c0d141178e51e036785db6cd1a60d8c\",\"created\":\"2018-02-28T22:14:48.759033366Z\",\"container_config\":{\"Cmd\":[\"/bin/sh -c #(nop) ADD file:327f69fc1ac9a7b6e56e9032f7b8fbd7741dd0b22920761909c6c8e5fa9c5815 in / \"]}}"
      }
   ],
   "signatures": [
      {
         "header": {
            "jwk": {
               "crv": "P-256",
               "kid": "2C7J:6J3S:G3GH:SE5S:P7AL:N7YT:2P5S:Q2Z4:3R5H:ZD7U:WNV4:HZLW",
               "kty": "EC",
               "x": "R507jc8T0HuY-bizsQiRx6FHBKXb8TmIDOdD0ya13ow",
               "y": "J7pKRgnyI3dn2NXKIWXOejTQOaUPGHIh91goKZdTMjw"
            },
            "alg": "ES256"
         },
         "signature": "sXgkd8FyLE7pDa9h3herqNaFTyxDBbeSpLbK8BWOlgvkGXHOYUJFeQO5s8HCD6YuEYIpPBqZwin809_cScu0tQ",
         "protected": "eyJmb3JtYXRMZW5ndGgiOjIxMjQsImZvcm1hdFRhaWwiOiJDbjAiLCJ0aW1lIjoiMjAxOC0wMy0yNlQxMjo1ODowMVoifQ"
      }
   ]
}
{% endhighlight %}

或者：
{% highlight string %}
# curl  -H "Accept: application/vnd.docker.distribution.manifest.v2+json" -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" -I -X GET http://10.17.153.196:5000/v2/busybox/manifests/latest
HTTP/1.1 200 OK
Content-Length: 527
Content-Type: application/vnd.docker.distribution.manifest.v2+json
Docker-Content-Digest: sha256:c7b0a24019b0e6eda714ec0fa137ad42bc44a754d9cea17d14fba3a80ccc1ee4
Docker-Distribution-Api-Version: registry/2.0
Etag: "sha256:c7b0a24019b0e6eda714ec0fa137ad42bc44a754d9cea17d14fba3a80ccc1ee4"
X-Content-Type-Options: nosniff
Date: Mon, 26 Mar 2018 13:30:55 GMT
{% endhighlight %}


5) **删除镜像**

通过上面的方式，我们可以获得镜像的digest。例如上面的busybox镜像的digest为：
<pre>
Docker-Content-Digest: sha256:200c94f114bee03ecf6cc7e671473a30d99d212599c3ad53b822d6a55eec5be5
</pre>
因此，我们可以用如下的命令来删除：
{% highlight string %}
# curl -I -X DELETE http://10.17.153.196:5000/v2/busybox/manifests/sha256:c7b0a24019b0e6eda714ec0fa137ad42bc44a754d9cea17d14fba3a80ccc1ee4 -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz"

//或者如下命令
# curl -X DELETE -H "Authorization: Basic YWRtaW46TWlkZWFAMTIz" http://10.17.153.196:5000/v2/busybox/manifests/latest -I
HTTP/1.1 405 Method Not Allowed
Content-Type: application/json; charset=utf-8
Docker-Distribution-Api-Version: registry/2.0
X-Content-Type-Options: nosniff
Date: Mon, 26 Mar 2018 13:39:52 GMT
Content-Length: 78
{% endhighlight %}
这里是因为我们禁止了docker registry的删除功能，因此提示Unsupported.

说明：在删除registry2.3或以上版本的docker镜像时，我们通过使用GET或HEAD获取镜像的digest时，必须要加如下参数：
<pre>
Accept: application/vnd.docker.distribution.manifest.v2+json
</pre>

## 3. 附录
下面给出Basic认证条件下，求授权信息信息的方法实现：
{% highlight string %}
package main

import (
	"encoding/base64"
	"fmt"
)

func main(){

	username := "admin"
	password := "Harbor12345"

	auth := username + ":" + password

	str := base64.StdEncoding.EncodeToString([]byte(auth))

	fmt.Println(str)

}
{% endhighlight %}

运行输出```YWRtaW46SGFyYm9yMTIzNDU=```。
<br />
<br />

**[参看]**

1. [Docker Registry HTTP API V2](http://192.168.69.128/registry/spec/api/#detail)


2. [centos7 Docker私有仓库搭建及删除镜像](https://www.cnblogs.com/Tempted/p/7768694.html)

3. [docker registry 镜像删除](https://blog.csdn.net/l6807718/article/details/52886546)

<br />
<br />
<br />

