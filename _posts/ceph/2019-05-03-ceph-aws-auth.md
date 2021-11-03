---
layout: post
title: AWS Authenticating 
tags:
- ceph
categories: ceph
description: AWS Authenticating 
---


本章介绍一下AWS v2及v4版本鉴权的实现。


<!-- more -->


## 1. Reference

**1) Amazon official website**

[AWS Authenticating v4](https://docs.aws.amazon.com/zh_cn/AmazonS3/latest/API/sig-v4-authenticating-requests.html)

[AWS 相关SDK](https://aws.amazon.com/cn/tools/#sdk)

[sigv4: create canonical request](https://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html)

[AWS Authenticating v2](https://docs.aws.amazon.com/AmazonS3/latest/userguide/auth-request-sig-v2.html)

**2) ceph reference**

rgw/rgw_main.cc

rgw/rgw_civetweb.cc       RGWMongoose::init_env()

rgw/rgw_rest.cc           int RGWREST::preprocess()

rgw/rgw_rest_s3.cc        int RGW_Auth_S3::authorize()







<br />
<br />

**[参看]**




<br />
<br />
<br />

