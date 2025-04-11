---
layout: post
title: 清除无service文件的unit
tags:
- LinuxOps
categories: linuxOps
description: 清除无service文件的unit
---


如果`systemctl list-units --all`仍然列出了已经删除`.service`文件的unit(且LOAD=not-found)，说明systemd仍然缓存了该unit，但找不到对应的文件。

如下记录一下如何清理此种unit。

<!-- more -->

# 清除无.service文件的unit

1. **运行systemctl reset-failed**

    ```bash
    # sudo systemctl reset-failed
    ```

    适用于：如果 unit处于FAILED 状态，这个命令会清除 systemd 的记录，使systemctl list-units不再显示它

1. **运行systemctl daemon-reload**

    如果 reset-failed 仍然无法移除，尝试重新加载 systemd:

    ```bash
    # sudo systemctl daemon-reload
    ```

    适用于：`.service` 文件已删除，但 systemd 仍然保留了它的缓存

1. **手动删除unit残留**

    如果 unit 仍然在 list-units 里，检查 systemd 是否有残留的 unit 配置:

    ```bash
    # ls -l /etc/systemd/system/ | grep <unit_name>
    # ls -l /usr/lib/systemd/system/ | grep <unit_name>
    # ls -l /run/systemd/system/ | grep <unit_name>
    ```

    如果找到类似 `xxx.service` 的文件或软链接，可以手动删除：

    ```bash
    # sudo rm -f /etc/systemd/system/<unit_name>.service
    # sudo systemctl daemon-reload
    ```

1. **彻底禁用并移除 unit**

    如果`systemctl disable <unit>` 仍然显示 unit 存在，尝试：

    ```bash
    # sudo systemctl disable --now <unit_name>
    # sudo rm -f /etc/systemd/system/<unit_name>.service
    # sudo systemctl daemon-reload
    ```

    然后检查：

    ```bash
    # systemctl list-units --all | grep <unit_name>
    ```

1. **直接强制删除 symlink**

    如果 disable 无效，手动清除 unit 的 symlink：

    ```bash
    # sudo find /etc/systemd/system/ -name "*<unit_name>*" -delete
    # sudo systemctl daemon-reload
    ```
