---
layout: post
title: 创建OAuth token
tags:
- tools
categories: tools
description: 创建OAuth token
---

本文记录一下GitHub如何创建一个OAuth token。

<!-- more -->

## 创建GitHub OAuth token
1. 登录GitHub账户
访问 [GitHub](https://github.com/) 并使用你的账户登录

1. 进入开发者设置
   - 点击右上角的头像，选择`Settings`
   - 在左侧菜单中，找到并点击`Developer settings`
  
1. 创建 OAuth Token
   - 在 Developer settings 页面，选择 Personal access tokens
   - 点击 Generate new token

1. 配置 Token 权限
   - 为 Token 设置一个描述性名称（例如 "My OAuth Token"）
   - 选择 Token 的有效期
     - No expiration（永不过期）：适合长期使用的 Token
     - Custom expiration（自定义有效期）：设置具体的过期时间
   - 根据需要勾选权限（Scopes）。常见的权限包括：
     - repo：访问私有和公共仓库
     - admin:org ：管理组织
     - user：访问用户信息
     - workflow：管理 GitHub Actions
     - 其他权限根据需求选择

1. 生成 Token
   - 点击 Generate token（生成令牌）。
   - 生成后，Token 会显示在页面上。务必立即复制并保存，因为关闭页面后将无法再次查看
  
1. 使用 Token
   - 将生成的 Token 用于 API 请求或 Git 操作中。
   - 例如，使用 Git 克隆私有仓库时，可以用 Token 代替密码：
     ```bash
     git clone https://<TOKEN>@github.com/username/repo.git
     ```
