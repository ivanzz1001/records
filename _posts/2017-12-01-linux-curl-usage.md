---
layout: post
title: Linux下curl工具的使用
tags:
- LinuxOps
categories: linux
description: Linux下curl工具的使用
---

这里我们简要讲解一下Linux环境下curl工具的使用。curl命令是一个功能强大的网络工具，支持通过http、ftp等方式下载、上传文件。还可以用来抓取网页、网络监控等方面的开发，解决开发过程中遇到的问题。



<!-- more -->

## 1. curl工具的安装

在Linux环境下安装curl工具很简单，直接通过如下的命令安装即可：
{% highlight string %}
// centos环境
# a=`yum list installed | grep "\<curl.x86_64\>"`
# if [ $a -eq "" ]; then yum install curl.x86_64; fi


//ubuntu环境
# sudo apt-get install curl
{% endhighlight %}


## 2. curl命令的基本使用

如下我们列出curl工具使用时的相关参数：
{% highlight string %}
[root@localhost test-src]# curl --help
Usage: curl [options...] <url>
Options: (H) means HTTP/HTTPS only, (F) means FTP only
     --anyauth       Pick "any" authentication method (H)
 -a, --append        Append to target file when uploading (F/SFTP)
     --basic         Use HTTP Basic Authentication (H)
     --cacert FILE   CA certificate to verify peer against (SSL)
     --capath DIR    CA directory to verify peer against (SSL)
 -E, --cert CERT[:PASSWD] Client certificate file and password (SSL)
     --cert-type TYPE Certificate file type (DER/PEM/ENG) (SSL)
     --ciphers LIST  SSL ciphers to use (SSL)
     --compressed    Request compressed response (using deflate or gzip)
 -K, --config FILE   Specify which config file to read
     --connect-timeout SECONDS  Maximum time allowed for connection
 -C, --continue-at OFFSET  Resumed transfer offset
 -b, --cookie STRING/FILE  String or file to read cookies from (H)
 -c, --cookie-jar FILE  Write cookies to this file after operation (H)
     --create-dirs   Create necessary local directory hierarchy
     --crlf          Convert LF to CRLF in upload
     --crlfile FILE  Get a CRL list in PEM format from the given file
 -d, --data DATA     HTTP POST data (H)
     --data-ascii DATA  HTTP POST ASCII data (H)
     --data-binary DATA  HTTP POST binary data (H)
     --data-urlencode DATA  HTTP POST data url encoded (H)
     --delegation STRING GSS-API delegation permission
     --digest        Use HTTP Digest Authentication (H)
     --disable-eprt  Inhibit using EPRT or LPRT (F)
     --disable-epsv  Inhibit using EPSV (F)
 -D, --dump-header FILE  Write the headers to this file
     --egd-file FILE  EGD socket path for random data (SSL)
     --engine ENGINGE  Crypto engine (SSL). "--engine list" for list
 -f, --fail          Fail silently (no output at all) on HTTP errors (H)
 -F, --form CONTENT  Specify HTTP multipart POST data (H)
     --form-string STRING  Specify HTTP multipart POST data (H)
     --ftp-account DATA  Account data string (F)
     --ftp-alternative-to-user COMMAND  String to replace "USER [name]" (F)
     --ftp-create-dirs  Create the remote dirs if not present (F)
     --ftp-method [MULTICWD/NOCWD/SINGLECWD] Control CWD usage (F)
     --ftp-pasv      Use PASV/EPSV instead of PORT (F)
 -P, --ftp-port ADR  Use PORT with given address instead of PASV (F)
     --ftp-skip-pasv-ip Skip the IP address for PASV (F)
     --ftp-pret      Send PRET before PASV (for drftpd) (F)
     --ftp-ssl-ccc   Send CCC after authenticating (F)
     --ftp-ssl-ccc-mode ACTIVE/PASSIVE  Set CCC mode (F)
     --ftp-ssl-control Require SSL/TLS for ftp login, clear for transfer (F)
 -G, --get           Send the -d data with a HTTP GET (H)
 -g, --globoff       Disable URL sequences and ranges using {} and []
 -H, --header LINE   Custom header to pass to server (H)
 -I, --head          Show document info only
 -h, --help          This help text
     --hostpubmd5 MD5  Hex encoded MD5 string of the host public key. (SSH)
 -0, --http1.0       Use HTTP 1.0 (H)
     --ignore-content-length  Ignore the HTTP Content-Length header
 -i, --include       Include protocol headers in the output (H/F)
 -k, --insecure      Allow connections to SSL sites without certs (H)
     --interface INTERFACE  Specify network interface/address to use
 -4, --ipv4          Resolve name to IPv4 address
 -6, --ipv6          Resolve name to IPv6 address
 -j, --junk-session-cookies Ignore session cookies read from file (H)
     --keepalive-time SECONDS  Interval between keepalive probes
     --key KEY       Private key file name (SSL/SSH)
     --key-type TYPE Private key file type (DER/PEM/ENG) (SSL)
     --krb LEVEL     Enable Kerberos with specified security level (F)
     --libcurl FILE  Dump libcurl equivalent code of this command line
     --limit-rate RATE  Limit transfer speed to this rate
 -l, --list-only     List only names of an FTP directory (F)
     --local-port RANGE  Force use of these local port numbers
 -L, --location      Follow redirects (H)
     --location-trusted like --location and send auth to other hosts (H)
 -M, --manual        Display the full manual
     --mail-from FROM  Mail from this address
     --mail-rcpt TO  Mail to this receiver(s)
     --mail-auth AUTH  Originator address of the original email
     --max-filesize BYTES  Maximum file size to download (H/F)
     --max-redirs NUM  Maximum number of redirects allowed (H)
 -m, --max-time SECONDS  Maximum time allowed for the transfer
     --metalink      Process given URLs as metalink XML file
     --negotiate     Use HTTP Negotiate Authentication (H)
 -n, --netrc         Must read .netrc for user name and password
     --netrc-optional Use either .netrc or URL; overrides -n
     --netrc-file FILE  Set up the netrc filename to use
 -N, --no-buffer     Disable buffering of the output stream
     --no-keepalive  Disable keepalive use on the connection
     --no-sessionid  Disable SSL session-ID reusing (SSL)
     --noproxy       List of hosts which do not use proxy
     --ntlm          Use HTTP NTLM authentication (H)
 -o, --output FILE   Write output to <file> instead of stdout
     --pass PASS     Pass phrase for the private key (SSL/SSH)
     --post301       Do not switch to GET after following a 301 redirect (H)
     --post302       Do not switch to GET after following a 302 redirect (H)
     --post303       Do not switch to GET after following a 303 redirect (H)
 -#, --progress-bar  Display transfer progress as a progress bar
     --proto PROTOCOLS  Enable/disable specified protocols
     --proto-redir PROTOCOLS  Enable/disable specified protocols on redirect
 -x, --proxy [PROTOCOL://]HOST[:PORT] Use proxy on given port
     --proxy-anyauth Pick "any" proxy authentication method (H)
     --proxy-basic   Use Basic authentication on the proxy (H)
     --proxy-digest  Use Digest authentication on the proxy (H)
     --proxy-negotiate Use Negotiate authentication on the proxy (H)
     --proxy-ntlm    Use NTLM authentication on the proxy (H)
 -U, --proxy-user USER[:PASSWORD]  Proxy user and password
     --proxy1.0 HOST[:PORT]  Use HTTP/1.0 proxy on given port
 -p, --proxytunnel   Operate through a HTTP proxy tunnel (using CONNECT)
     --pubkey KEY    Public key file name (SSH)
 -Q, --quote CMD     Send command(s) to server before transfer (F/SFTP)
     --random-file FILE  File for reading random data from (SSL)
 -r, --range RANGE   Retrieve only the bytes within a range
     --raw           Do HTTP "raw", without any transfer decoding (H)
 -e, --referer       Referer URL (H)
 -J, --remote-header-name Use the header-provided filename (H)
 -O, --remote-name   Write output to a file named as the remote file
     --remote-name-all Use the remote file name for all URLs
 -R, --remote-time   Set the remote file's time on the local output
 -X, --request COMMAND  Specify request command to use
     --resolve HOST:PORT:ADDRESS  Force resolve of HOST:PORT to ADDRESS
     --retry NUM   Retry request NUM times if transient problems occur
     --retry-delay SECONDS When retrying, wait this many seconds between each
     --retry-max-time SECONDS  Retry only within this period
 -S, --show-error    Show error. With -s, make curl show errors when they occur
 -s, --silent        Silent mode. Don't output anything
     --socks4 HOST[:PORT]  SOCKS4 proxy on given host + port
     --socks4a HOST[:PORT]  SOCKS4a proxy on given host + port
     --socks5 HOST[:PORT]  SOCKS5 proxy on given host + port
     --socks5-hostname HOST[:PORT] SOCKS5 proxy, pass host name to proxy
     --socks5-gssapi-service NAME  SOCKS5 proxy service name for gssapi
     --socks5-gssapi-nec  Compatibility with NEC SOCKS5 server
 -Y, --speed-limit RATE  Stop transfers below speed-limit for 'speed-time' secs
 -y, --speed-time SECONDS  Time for trig speed-limit abort. Defaults to 30
     --ssl           Try SSL/TLS (FTP, IMAP, POP3, SMTP)
     --ssl-reqd      Require SSL/TLS (FTP, IMAP, POP3, SMTP)
 -2, --sslv2         Use SSLv2 (SSL)
 -3, --sslv3         Use SSLv3 (SSL)
     --ssl-allow-beast Allow security flaw to improve interop (SSL)
     --stderr FILE   Where to redirect stderr. - means stdout
     --tcp-nodelay   Use the TCP_NODELAY option
 -t, --telnet-option OPT=VAL  Set telnet option
     --tftp-blksize VALUE  Set TFTP BLKSIZE option (must be >512)
 -z, --time-cond TIME  Transfer based on a time condition
 -1, --tlsv1         Use => TLSv1 (SSL)
     --tlsv1.0       Use TLSv1.0 (SSL)
     --tlsv1.1       Use TLSv1.1 (SSL)
     --tlsv1.2       Use TLSv1.2 (SSL)
     --trace FILE    Write a debug trace to the given file
     --trace-ascii FILE  Like --trace but without the hex output
     --trace-time    Add time stamps to trace/verbose output
     --tr-encoding   Request compressed transfer encoding (H)
 -T, --upload-file FILE  Transfer FILE to destination
     --url URL       URL to work with
 -B, --use-ascii     Use ASCII/text transfer
 -u, --user USER[:PASSWORD]  Server user and password
     --tlsuser USER  TLS username
     --tlspassword STRING TLS password
     --tlsauthtype STRING  TLS authentication type (default SRP)
     --unix-socket FILE    Connect through this UNIX domain socket
 -A, --user-agent STRING  User-Agent to send to server (H)
 -v, --verbose       Make the operation more talkative
 -V, --version       Show version number and quit
 -w, --write-out FORMAT  What to output after completion
     --xattr        Store metadata in extended file attributes
 -q                 If used as the first parameter disables .curlrc
{% endhighlight %}

上面我们可以看到curl命令参数有很多，这里我们说明一下如下几个：

* **-v, --verbose:** 小写的v参数，用于打印更多信息，包括发送的请求信息，这在调试脚本特别有用。

* **-m, --max-time SECONDS:** 指定处理的最大时长

* **-H, --header LINE:** 指定请求头参数

* **-s, --silent:** 减少输出信息，比如进度等

* **--connect-timeout SECONDS:** 指定尝试连接的最大时长

* **-x, --proxy [PROTOCOL://]HOST[:PORT]:** 指定代理服务器的地址和端口，默认端口为1080

* **-T, --upload-file FILE:** 传输文件到目标地址

* **-o, --output FILE:** 指定输出文件名称

* **-d, --data DATA:** 指定http post的内容

* **--retry NUM:** 假如产生错误的情况下，执行的重试次数

* **-e, --referer:** 指定引用地址

* **-I, --head:** 仅返回头部信息，使用head请求

## 3. GET请求
如下我们给出一些示例，展示如何使用Get请求：
<pre>
# curl http://www.baidu.com   //回车之后，HTML内容打印在屏幕上；如果这里的URL指向的是一个文件或者一副图则可以直接下载到本地

# curl -i http://www.baidu.com  // 在输出信息中打印包括头部消息在内的整个消息

# curl -I http://www.baidu.com  // 只显示头部信息，其实这里采用的是HEAD请求，可以通过添加-v选项来查看请求与响应详细消息

# curl -v http://www.baidu.com  // 显示get请求全过程解析
</pre>

这里我们可以同时用tcpdump进行抓包，来观察响应的请求与响应：
{% highlight string %}
// 窗口1启动tcpdump抓取数据包
# tcpdump -i ens33 -v -x -w curl_output.pcap host www.baidu.com

// 窗口2 curl请求目标地址
# curl http://www.baidu.com

//也可以通过如下方式模拟curl向目标地址请求数据(注意：这个百度服务器根据User-Agent不同，可能返回不同的结果)
# nc www.baidu.com 80 > abc.txt << EOF
> GET / HTTP/1.1
> User-Agent: curl/7.29.0
> Host: www.baidu.com
> Accept: */*
> 
> EOF
{% endhighlight %}

如下用wireshark打开上面抓取到的```curl_output.pcap```数据包：

![curl output](https://ivanzz1001.github.io/records/assets/img/linux/linux-curl-get.png)








<br />
<br />

**[参看]:**

1. [curl工具使用指南](http://blog.csdn.net/flyingfalcon/article/details/2026587)

2. [curl 工具的使用](https://www.cnblogs.com/z-sm/p/5631352.html)

<br />
<br />
<br />





