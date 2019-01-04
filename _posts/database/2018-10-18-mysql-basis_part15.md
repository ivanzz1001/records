---
layout: post
title: MySQL主从复制示例
tags:
- database
categories: database
description: MySQL主从复制示例
---


在前面的章节中我们介绍了MySQL主从复制的一些基础知识，这里我们给出相应的示例。


<!-- more -->


## 1. 配置参考
<pre>
# cat /etc/my.cnf
[mysql]
[mysqld]
sql_mode = NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION
skip-character-set-client-handshake = true
character_set_server = utf8mb4
collation_server = utf8mb4_general_ci
init_connect = 'SET NAMES utf8mb4'
lower_case_table_names = 1
server-id = 197
datadir = /var/lib/mysql
socket = /var/lib/mysql/mysql.sock
log-error = /var/log/mysqld.log
pid-file = /var/run/mysqld/mysqld.pid
slow_query_log = 1
long_query_time = 1
log-bin
binlog_format = ROW
expire_logs_days = 7
symbolic-links = 0
skip-name-resolve = 1
back_log = 600
max_connections = 15000
max_connect_errors = 6000
open_files_limit = 65535
table_open_cache = 2048
table_definition_cache = 2048
table_open_cache_instances = 64
max_allowed_packet = 64M
binlog_cache_size = 4M
max_binlog_size = 1G
max_binlog_cache_size = 2G
max_heap_table_size = 96M
tmp_table_size = 96M
read_buffer_size = 8M
read_rnd_buffer_size = 16M
sort_buffer_size = 16M
join_buffer_size = 16M
thread_cache_size = 64
thread_stack = 512K
query_cache_size = 0
query_cache_type = 0
query_cache_limit = 8M
key_buffer_size = 32M
ft_min_word_len = 4
transaction_isolation = REPEATABLE-READ
performance_schema = 1
explicit_defaults_for_timestamp = true
skip-external-locking
default-storage-engine = InnoDB
innodb_autoinc_lock_mode = 2
innodb_locks_unsafe_for_binlog = 0
innodb_rollback_on_timeout = 1
innodb_buffer_pool_size = 6144M
innodb_buffer_pool_instances = 8
innodb_buffer_pool_load_at_startup = 1
innodb_buffer_pool_dump_at_shutdown = 1
innodb_flush_method = O_DIRECT
innodb_page_cleaners = 4
innodb_file_per_table = 1
innodb_open_files = 500
innodb_write_io_threads = 8
innodb_read_io_threads = 8
innodb_io_capacity = 4000
innodb_io_capacity = 8000
innodb_buffer_pool_dump_pct = 40
innodb_thread_concurrency = 0
innodb_flush_log_at_trx_commit = 2
innodb_log_buffer_size = 32M
innodb_log_file_size = 128M
innodb_log_files_in_group = 3
innodb_max_dirty_pages_pct = 80
innodb_lock_wait_timeout = 120 
innodb_spin_wait_delay = 30
innodb_file_format = Barracuda
innodb_purge_threads = 4
innodb_print_all_deadlocks = 1
bulk_insert_buffer_size = 64M
myisam_sort_buffer_size = 128M
myisam_max_sort_file_size = 10G
myisam_repair_threads = 1
interactive_timeout = 28800
wait_timeout = 28800
ignore-db-dir = lost+found
ignore-db-dir = zabbix.history
ignore-db-dir = zabbix.trends
log_slow_slave_statements = 1
sync_binlog = 1
master_info_repository = TABLE
relay_log_info_repository = TABLE
log_slave_updates
relay_log_recovery = 1
relay_log_purge = 1
innodb_monitor_enable = "module_innodb"
innodb_monitor_enable = "module_server"
innodb_monitor_enable = "module_dml"
innodb_monitor_enable = "module_ddl"
innodb_monitor_enable = "module_trx"
innodb_monitor_enable = "module_os"
innodb_monitor_enable = "module_purge"
innodb_monitor_enable = "module_log"
innodb_monitor_enable = "module_lock"
innodb_monitor_enable = "module_buffer"
innodb_monitor_enable = "module_index"
innodb_monitor_enable = "module_ibuf_system"
innodb_monitor_enable = "module_buffer_page"
innodb_monitor_enable = "module_adaptive_hash"
[mysqld_safe]
pid-file = /run/mysqld/mysqld.pid
syslog
[mysqldump]
quick
max_allowed_packet = 64M
[mysqladmin]
socket=/var/lib/mysql/mysql.sock
[client]
!includedir /etc/my.cnf.d
</pre>







<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL的binlog日志](https://www.cnblogs.com/martinzhang/p/3454358.html)

3. [mysql （master/slave）复制原理及配置](https://www.cnblogs.com/jirglt/p/3549047.html)

4. [MySQL主从复制(Master-Slave)实践](https://www.cnblogs.com/gl-developer/p/6170423.html)

5. [MySQL 设置基于GTID的复制](http://blog.51cto.com/13540167/2086045)

6. [MySQL 在线开启/关闭GTID](https://blog.csdn.net/jslink_l/article/details/54574066)

7. [mysql在线开启或禁用GTID模式](http://www.cnblogs.com/magmell/p/9223556.html)
<br />
<br />
<br />

