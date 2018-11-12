---
layout: post
title: GDB的使用(part 1)
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```，详细的介绍一下GDB的使用方法。

<!-- more -->


## 1. GDB的启动与退出
本节我们主要讲述一下如何启动以及退出GDB，主要包含如下：

* 输入```gdb```来启动gdb调试

* 输入```quit```或者```Ctrl-D```来退出GDB

### 1. 启动GDB
下面描述的命令行选项在覆盖大部分使用场景；但是在有一些环境中，有一些选项可能会不能够使用。通常情况下，在启动gdb时传入```可执行程序名```这样一个参数即可：
<pre>
# gdb program
</pre>
你也可以在启动时同时指定一个```可执行程序名```和一个```core```文件：
<pre>
# gdb program core
</pre>
另外，假如你想调试一个正在运行的进程的话，也可以在启动时指定一个进程ID作为第二个参数：
<pre>
# gdb program 1234
</pre>
上面的命令会将进程1234绑定到GDB（除非你恰好有一个名称为1234的文件，这种情况下将会优先将1234作为一个core文件来看待）。

如果要使用第二个命令行参数，一般要求有一个完整的操作系统环境；而当你使用GDB来远程调试一个主板的时候，这很可能没有```进程```的概念，并且通常也无法获取到一个coredump文件。假如gdb并不能attach到一个进程，或者并不能读取到core dump文件的时候，其都会给出相应的警告。


此外，我们也可以通过```--args```选项来为gdb所调试的可执行程序传递参数。```注意```：此选项会使得gdb不再处理其他的选项了。例如：
<pre>
# gdb --args gcc -O2 -c foo.c
</pre>
上面的命令将会导致gdb调试gcc这个可执行程序，并且设置gcc的命令行参数为```-O2 -c foo.c```。


在启动GDB时，我们可以通过传递```--silent```(或```-q```/```--quiet```)来禁止gdb的一些前置消息的打印（例如gdb版本号之类的）。我们还可以使用```gdb --help```或者```gdb -h```来获得更多关于gdb的帮助信息。

通常情况下，gdb都是按命令行传递的顺序来处理```参数```和```选项```的。但是你可以使用```-x```选项来修改顺序。



### 1.2 选择文件
当gdb启动之后，其会读取任何参数（而不是选项）以作为可执行文件和core dump文件（或进程pid)。这与分别通过```-se```、```-c```(或```-p```）选项来指定是一样的。（gdb会读取第一个不带选项的参数，将其看做是带了```-se```选项； 接着读取第二个不带选项打的参数，将其看做是带了```-c```或```-p```选项）。假如第二个参数是一个```十进制数```，则gdb会尝试将其attach到一个进程，假如attach失败，则尝试将其当做一个core dump文件来看待。因此，假如你刚好有一个名称以数字开头的coredump文件，你可以通过类似于```./1234```这样的方式来避免gdb将其当成是一个pid。

在大多数嵌入式系统当中，gdb可能并未配置有包含core dump功能的支持，这时如遇到第二个参数，则会打印相应的警告信息并且忽略该参数。



下面我们列出gdb支持的一些选项：

* ```--symbols file```(或```-s file```): 从file文件中读取符号表

* ```--exec file```(或```-e file```): 将file作为可执行文件来执行，并将该参数后的另外一个不带选项的参数作为core dump文件

* ```--se file```: 从文件中读取符号表，并将其作为一个可执行文件

* ```--core file```(或```-c file```): 将file作为一个core dump文件

* ```--pid number```(或```-p number```): 连接到指定pid的进程，相当于执行了一个```attach```命令。

* ```--command file```(或```-x file```): 从file文件中执行相应的命令。对文件内容的执行严格参照```source```命令的执行。

* ```--eval-command command```(或```-ex command```): 执行一个单独的gdb命令。本选项可多次使用，以调用多个命令。也可以搭配```--command```来使用。例如：
<pre>
# gdb -ex 'target sim' -ex 'load' \
  -x setbreakpoints -ex 'run' a.out
</pre>

* ```--init-command file```(或```-ix file```): 在加载可执行文件之前先执行file文件中指定的命令。

* ```-init-eval-command command```(或```-iex command```): 在加载可执行文件之前先执行一个单独的gdb命令

* ```-directory directory```(或```-d directory```): 将directory添加到gdb搜寻源文件及脚本文件的路径当中

* ```--readnow```(或```-r```): 马上读取每一个符号文件的所有符号表。默认情况下，读取符号表是根据所需以递增的方式来执行的。这会使得加载速度变慢，但是后续的执行速度会更快。





### 1.3 选择gdb的执行模式
你可以以不同的模式来运行gdb，例如```batch```模式或```quiet```模式。下面介绍几个常用的：

* ```-nx```或```-n```: 并不要执行任何```初始化文件```中的命令。3个初始化文件会按照如下的顺序来加载
<pre>
1) system.gdbinit: 这是系统级别的初始化文件，可以通过--with-system-gdbinit选项来指定。一般在GDB
    启动之后，在处理任何选项之前就会被加载

2） ~/.gdbinit: 这是当前home目录的初始化文件，其会在system.gdbinit之后但在处理任何选项之前被加载。

3） ./.gdbinit: 这是当前目录下的初始化文件。其一般会在最后加载（但是注意会在-x或-ex选项之前加载）
</pre>


* ```--quiet```或```--silent```或```-q```: 表示以安静模式启动gdb，不要打印相关的介绍信息和版权信息。

* ```--args```:用于改变对命令行的解释，使得在可执行文件后面的参数会作为可执行文件的参数，而不是gdb的参数来处理。注意： 本选项会停止gdb的选项处理，因为gdb会认为这些选项都是属于可执行文件的。

* ```--version```: 用于使gdb在使用时打印相关的版本信息



### 1.4 GDB启动流程
下面我们讲述一下GDB的一个启动流程：

1） 根据命令行的指定初始化对应的命令解释器（参看上一节```GDB的执行模式```)

2) 读取系统级别的```init```文件（即在编译gdb时通过```--with-system-gdbinit```选项指定的文件），并执行其中的命令

3) 读取home目录的```init```文件并执行其中的命令

4） 按顺序执行由```-iex```选项指定的命令，或```-ix```选项指定的文件中的命令。你也可以使用```-ex```或者```-x```来替换前面的选项，但是在这种情况下其会在gdbinit之前被执行

5） 处理命令行选项和参数

6）假如当前目录并不是```home```目录，且```set auto-load local-gdbinit```值为```on```时，则会读取和执行当前工作目录下的gdbinit文件。

7） 如果命令行指定了要调试的```program```，或者要attach到一个进程，或者一个core dump文件，那么gdb会加载该```program```所需要的自动加载文件或共享库。假如你想要在gdb启动时禁止这个自动加载，那么类似于如下：
<pre>
# gdb -iex "set auto-load python-scripts off" myprogram
</pre>
说明： 这里并不能用```-ex```选项，因为这会使得关闭```auto-load```太晚

8) 执行```-ex```选项指定的命令，或```-x```选项指定的文件中的命令

9） 读取```history file```中的历史命令




### 1.4 退出GDB

```quit [expression]```或```q```: 我们可以通过```quit```或```q```来退出GDB，也可以通过```End of File```字符（一般为```Ctrl-d```)来退出。假如你并未指定```[expression]```的话，则GDB会正常的退出； 否则会使用该表达式的结果作为退出码。

```Ctrl-c```并不能退出GDB，只是会中断当前GDB正在执行的命令并返回到GDB命令行。

另外，假如你当前正在使用gdb来控制一个attach的进程或者device，那么你可以通过detach命令来进行释放。


### 1.5 shell命令
假如你想在gdb调试期间执行shell命令，可以不用退出或挂起GDB，你可以直接使用shell命令：```shell command-string```或者```!command-string```。





## 2. 在GDB下运行程序
当你需要在GDB下运行程序的时候，你必须在编译的时候产生相应的调试信息。你可以根据自身的环境，在启动GDB时指定相应的参数。假如你进行的是本地调试，你还可以对所要调试的程序输入输出进行重定向； 可以调试一个正在运行的程序；或者kill掉一个子进程。



### 2.1 编译调试程序
为了有效的调试程序，你需要在编译的时候产生相应的调试信息。这些调试信息被存放在对象文件件中(```.o```文件）： 其描述了每一个变量及函数的数据类型，在源文件中的行号以及在可执行文件中对应的地址。


要产生调试信息，需要在编译的时候指定```-g```选项。通常情况下，交付给客户的程序在编译时都通过```-o```编译选项进行了优化。然而，有一些编译器并不能同时处理```-g```与```-o```两个选项，因此，当使用这些编译器的时候，并不能生成带有调试信息的优化版本的可执行文件。


对于GCC(GNU C/C++编译器），是支持```-g```与```-o```两个选项同时出现的，这就使得gdb能够调试优化过的代码。通常情况下，我们建议在编译程序的时候都带上```-g```选项。另外，对于一些老版本的GNU C编译器，其支持一个变体的```-gg```选项用于产生调试信息，但是当前gdb并不支持该格式。因此当你的GNU C编译器即使支持该选项时，也不要使用。

GDB可以识别预处理宏定义，并能显示这些宏定义的扩展。但是大多数的编译器在你单独指定```-g```选项产生的调试信息中并不包含预处理宏定义。对于GCC3.1及其之后的版本，假如使用```DWARF```调试格式并指定```-g3```的情况下，是可以提供宏信息的。

当你所用的编译器支持最新版本的```DWARF```调试格式的时候，你将能够获得最好的调试体验。```DWARF```是当前GDB支持最好的调试格式。





### 2.2 运行程序
在GDB下，可以使用```run```或者```r```命令来启动程序。假如你当前是在一个支持```进程```的执行环境下运行程序，```run```命令首先会创建一个进程，然后采用该进程来运行的可执行程序。而对于有一些不支持进程的环境，```run```则会直接跳转到你的可执行程序的开始位置处。对于其他的target，例如```remote target```，这些target在你运行gdb时总是处于运行状态。因此假如你获得类似于如下的错误消息的话：
<pre>
The "remote" target does not support "run".
Try "help target" or "continue".
</pre>
则之后你可以使用```continue```来运行程序。但是这可能需要你首先load。


通常，一个程序的执行会受到管理进程的多方面的影响。一般你可以在启动可执行程序之前，通过GDB以多种方式提供相应的信息以影响你所要调试程序的执行。这些信息通常可以分成4类：

* 参数(arguments)： 为可执行程序指定参数，并将此作为```run```命令的参数。

* 环境(environment): 通常你所运行的程序会从GDB继承相应的```environment```，但是你也可以使用GDB命令```set environment```和```unset environment```来更改部分对你所运行程序有影响的环境变量（请参```2.4 程序的环境变量```)。

* 工作目录(working directory): 你所运行的程序会从GDB那里继承其工作目录(working dir)，你可以通过在GDB中使用```cd```命令来修改GDB的工作目录(参看```2.5 程序的工作目录```)

* 标准输入输出(standard input and output): 通常你所运行的程序会使用与GDB相同的标准输入/输出。你可以通过```run```命令来重定向输入/输出，也可以通过```tty```命令来为程序设置一个不同的输入/输出设备。（参看```2.6 程序的输入/输出```) ```注意```: 当输入/输出被重定向之后，你将不能使用管道(pipe)来你所调试程序的输出再传递到另一个程序；假如你尝试这样做的话，gdb则可能会出现异常，变成调试另外一个错误的程序。


当你使用```run```命令时，你所调试的程序将会马上被执行。对于如何```stop and continue```，我们后续会进行介绍。而一旦你所调试的程序暂停下来，你就可以使用```print```或者```call```命令。

假如你所调试程序的符号表的```mtime```(修改时间）已经发生了改变，则GDB会丢弃原来所读取到的符号表，再重新读取。在这一过程中，gdb仍然会尝试保留当前的```断点```(breakpoints).

* **start**: 对于不同的语言，主调用过程（main procedure)的名称也可能会不同。对于C/C++而言，主调用过程（main procedure)的名称始终为```main```，但是对于如Ada这样的一些其他语言，其并不需要为主调用过程(main procedure)指定一个特别的名字。因此，GDB提供了一种很方便的方法来启动程序的执行，并在主调用过程(main procedure)开始位置暂停下来。

```start```命令等价于在主调用过程(main procedure)的起始位置设置一个临时的断点，然后再调用run命令。对于有一些程序来说，在主调用过程(main procedure)执行之前，还会先执行一段启动代码(startup code)。这通常取决于你所编写的程序所使用的编程语言。例如，对于C++来说，静态对象(static object)与全局对象(global object)的构造函数会在调用main()函数之前就被调用。因此存在着这样一种可能： 在调试程序时，debugger会在到达```main procedure```之前就暂停在某处。然而，在后续到达临时breakpoints时仍然还会继续暂停程序的执行。


另外，你可以在执行```start```命令时指定相应的参数，这些参数将会作为后续```run```命令的参数。值得注意的是，假如你后续再重新执行```start```或者```run```命令时，并未再指定参数，那么其会继承上一次执行该命令时所采用的参数。例如：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


int main(int argc,char *argv[])
{
        int i;

        for(i = 0;i<argc; i++)
                printf("%s\n", argv[i]);

        return 0x0;
}
{% endhighlight %}
编译并进行调试：
{% highlight string %}
# gcc -c -g test.c
# gcc -o test test.o

# gdb --silent ./test 
Reading symbols from /root/workspace/test...(no debugging symbols found)...done.
(gdb) start 1 2 3 4 5
Temporary breakpoint 1 at 0x400531
Starting program: /root/workspace/./test 1 2 3 4 5

Temporary breakpoint 1, 0x0000000000400531 in main ()
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) c
Continuing.
/root/workspace/./test
1
2
3
4
5
[Inferior 1 (process 3332) exited normally]
(gdb) r
Starting program: /root/workspace/./test 1 2 3 4 5
/root/workspace/./test
1
2
3
4
5
[Inferior 1 (process 3336) exited normally]
{% endhighlight %}
上面我们可以看到，我们通过```start```直接传递参数，后续通过```continue```执行时就把相应的参数打印出来了。


有时候，我们想要在```elaboration```期间(即在进入主调用过程之前）就调试程序。在这种情况下，我们使用```start```则可能会使得暂停在过晚的位置，因为这时程序已经完成了```elaboration phase```。因此，我们可以在运行程序之前，在```elaboration```代码处先插入断点(breakpoints)。

* **set exec-wrapper wrapper / show exec-wrapper / unset exec-wrapper**: 当```exec-wrapper```被设置之后，则在启动调试程序的时候指定的```wrapper```就会被使用。GDB会以```exec wrapper program```这样的shell命令格式启动可执行程序（说明： exec并不会启动一个新的shell来执行程序，因此program会继承wrapper的执行环境）。在启动时，会对```program```及参数加上双引号，但是对于wrapper则不会自动加上，因此适当的情况下，你可能需要对wrapper自己加上双引号。wrapper会在你运行程序的时候执行，之后才把控制权交给GDB。

你可以使用任何程序```wrapper```(甚至是```execve```)。有多个标准的```unix```工具可作为wrapper(例如```env```和```nohup```)。例如，你可以使用```env```来向调试程序传递环境变量，这样你就可以不用在shell环境中设置这些变量：
<pre>
(gdb) set exec-wrapper env 'LD_PRELOAD=libtest.so'
(gdb) run
</pre>
上述命令在大多数平台上调试本地程序时都是有效的，除了```DJGPP```、```Cygwin```、```MS Windows```以及```QNX Neutrino```之外。

* **set startup-with-shell / set startup-with-shell on / set startup-with-shell off / show set startup-with-shell**: 在Unix系统上，默认情况下，假如对应的target有可用的shell的话，GDB都会使用该Shell来启动程序。GDB的```run```命令会将参数传递给shell，然后shell可能会做```变量替换```(variable substitution)、扩展通配字符并进行IO重定向。然而在有一些情况下，我们可能需要禁止shell做这样的替换，例如，当调试shell本身或者诊断一些启动错误时：
<pre>
(gdb) run
Starting program: ./a.out
During startup program terminated with signal SIGSEGV, Segmentation fault.
</pre>
上面显示shell或者通过```exec-wrapper```指定的wrapper崩溃了，而不是你所运行的程序出了问题。在大多数情况下，这可能是由于你的shell初始化文件出了写问题，比如C-shell的```.cshrc```、Z-shell的```.zshenv```、BASH的```BASH_ENV```环境变量指定的文件出了问题。


* **set auto-connect-native-target / set auto-connect-native-target on / set auto-connect-native-target off / show auto-connect-native-target**

* **set disable-randomization / set disable-randomization on / set disable-randomization off** : 通常情况下GDB会在一个随机的虚拟地址启动程序。我们编译生成的程序也最好通过```gcc -fPIE -pie```这样的选项来生成位置独立的可执行程序。


### 2.3 程序运行参数
要想为你所调试的程序指定运行参数的话，你可以在执行```run```命令时附加相应的参数。这些参数会被传递给shell，然后shell会对相应的通配符进行展开并进行IO重定向，处理完成之后才会被传递给可执行程序。GDB所使用的shell是受```SHELL```环境变量(假如存在的话）控制的，假如当前并未定义```SHELL```环境变量，则Unix上默认为```/bin/sh```。

在一些非Unix系统上，程序通常是由GDB直接来调用的，这样GDB会通过适当的系统调用来模拟IO重定向，并且通配字符会由程序的启动代码所展开。

当```run```命令在执行时并未指定参数，那么其会沿用上一次```run```执行时所采用的参数。

* **set args**: 指定下一次你的程序运行时所使用的参数。假如```set args```并未指定参数，则在下一次使用```run```来运行程序时也不会携带参数。一旦你已经使用过参数来运行程序，那么在下一次使用```run```命令运行程序之前只能通过```set args```来取消参数。


* **show args**: 用于显示你给可执行程序所传递的参数

### 2.4 程序的执行环境
```environment```是由一系列的```环境变量```及```值```组成。环境变量通常用于记录```用户名```、```home目录```、```终端类型```、```可执行程序的搜索路径```等。通常你使用shell来建立环境变量，并且所有你运行的其他程序都会继承这些环境变量。当进行调试的时候，我们可能需要在不重启GDB的情况下修改程序执行时的环境变量。

* **path directory**: 在```PATH```环境变量(用于指定可执行文件的搜索路径)之前添加```directory```， 并将相应的值传递给你所调试的程序。GDB所使用的```PATH```并不会发生改变。你可以通过本命令指定多个目录名称，以空格或者是```system-dependent```字符来分割（在Unix系统上一般用```:```来分割； 在MS-DOS上一般用```;```来分割）。假如```directory```当前已经在path中，则会移动到前面，这样就可以加速搜索。

你可以使用```$cwd```来引用当前的工作目录，以作为GDB的搜索路径。假如你使用```.```的话，则其所引用的是你执行path命令时所在的目录。例如：
{% highlight string %}
# gdb --silent ./test
Reading symbols from /root/workspace/test...done.
(gdb) show paths
Executable and object file path: /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/root/bin
(gdb) path .
Executable and object file path: /root/workspace:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/root/bin
{% endhighlight %}

* **show paths**: 打印出可执行文件的搜索路径

* **show environment [varname]**: 打印出环境变量```varname```的值。假如你并未指定```varname```的话，则打印出所有传递给所调试程序的环境变量及值。这里```environment```可以简写为```env```。

* **set environment varname [=value]**: 用于设置环境变量```varname```的值为```value```。这只是改变传递给你所调试的程序的环境变量的值，而并不是改变GDB其本身的环境变量。例如：
<pre>
set env USER = foo
</pre>
上面命令用于告诉调试程序，当在后续执行```run```命令时，其所对应的用户时```foo```。

值得注意的是在Unix系统上，GDB是通过shell来执行你所调试的程序，其也会继承使用```set environment```命令设定的环境变量。假如有必要的话，你也可以通过使用```exec-wrapper```来替代本命令。

* **unset environment varname**: 用于从环境中移除该命令。


### 2.5 程序的工作目录
每一次你使用```run```命令启动程序的时候，程序的工作目录都继承于当前GDB的工作目录。而GDB的工作目录初始状态下会继承自其父进程（一般是shell)，但是你也可以在GDB中通过```cd```命令来指定一个新的工作目录。

GDB的工作目录也作为GDB的其他命令所操作文件的默认目录。

* **cd [directory]**: 用于将GDB的工作目录设置为```directory```，假如并未指定```directory```，则```directory```默认为```~```。

* **pwd**: 打印GDB的工作目录

通常情况下，我们并不能找到所调试进程的```当前工作目录```（因为在程序运行过程中，程序可能会更改其工作目录）。但是假如你所使用的系统上，GDB被配置为支持```/proc```的话，那么你可以通过使用```info proc```命令来找出所调试程序的当前工作目录。


### 2.6 程序的输入/输出
默认情况下，在GDB下所运行的程序的输入/输出与GDB使用同一个终端。GDB会切换到其自己的终端模式与外界进行交互，但是它会记录调试程序所使用的终端模式，以在你继续运行程序的时候来使用。

* **info terminal**: 用于打印当前GDB所记录到的执行程序所使用的终端模式。你可以在执行```run```命令时通过使用shell重定向来改变程序的输入/输出。例如：
{% highlight string %}
run > outfile
{% endhighlight %}
这样可以将输出重定向到```outfile```。

另一种方式指定调试程序的输入/输出就是使用```tty```命令。该命令可以接受一个```文件名```作为参数，并且会使得该文件作为后续执行```run```命令的默认输入/输出。另外，其也会重置后续通过```run```命令所运行程序的子进程的控制终端。例如：
<pre>
tty /dev/ttyb
</pre>

上面命令会使得后续执行```run```命令时将```/dev/ttyb```作为默认的输入/输出，并且作为该运行程序的控制终端。

在执行```run```命令时显示的重定向会覆盖```tty```命令所指定的输入/输出，但是并不会影响到控制终端。

当你使用```tty```命令或者```run```命令来重定向输入的时候，只有你所调试的程序的输入会受到影响。而对于GDB其本身的输入仍来自于你先前的终端。```tty```命令是```set inferior-tty```的别名。你也可以使用```show inferior-tty```命令打印出后续你索要执行程序所采用的终端。
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


int main(int argc,char *argv[])
{
        int i;

        for(i = 0;i<argc; i++)
                printf("%s\n", argv[i]);

        return 0x0;
}
{% endhighlight %}
编译运行：
{% highlight string %}
# gcc -c -g test.c
# gcc -o test test.o

# gdb --silent ./test
Reading symbols from /root/workspace/test...done.
(gdb) info terminal
No saved terminal information.
(gdb) run 1 2 3 4 5 > output.txt
Starting program: /root/workspace/./test 1 2 3 4 5 > output.txt
[Inferior 1 (process 9082) exited normally]
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) shell cat output.txt
/root/workspace/./test
1
2
3
4
5
(gdb) 
{% endhighlight %}

### 2.7 调试已经在运行的进程

* **attach process-id**: 该命令用于attach到一个正在运行的进程，一个在GDB外启动的进程。（可以通过```info files```来显示该active targets)。本命令接受一个```process ID```作为参数。在Unix系统中，查找进程ID的方法通产是使用```ps```工具，或者使用```jobs -l```shell命令。跟一般命令不同的是，```attach```命令执行后，你按```RET```按键，该命令并不会重复执行。

要使用```attach```，那么你的程序必须要运行在支持进程的环境中；例如，attach命令在没有操作系统的主板上则不能正常工作。另外，你还必须有权限向进程发送相应的信号(signal)。

当你使用```attach```时，debugger首先会在当前工作目录来查找该正在运行的进程，若没有找到，则使用```源文件查找路径```(source file search path)来查找。你也可以使用```file```命令来加载该程序。

GDB要调试该指定的进程的第一步就是要```暂停```进程的执行。你可以使用GDB在执行```run```命令后可用的所有命令来检查和修改一个```attached```的进程。你可以插入断点```breakpoints```; 也可以执行```step```和```continue```命令；也可以修改其中的内存值。假如你想要进程继续执行的话，那么在GDB ```attach```到该进程之后，你可以执行continue命令。
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
        int i, sum = 0;

        for(i = 0; i< 1000; i++)
        {
                sum += i;
                sleep(1);
        }

        return 0x0;
}
{% endhighlight %}
编译运行并调试：
{% highlight string %}
# gcc -c -g test.c
# gcc -o test test.o -lrt
# ./test &


# ps -ef | grep test
root       9607   2548  0 06:24 pts/0    00:00:00 ./test

# gdb --silent
(gdb) attach 9607
Attaching to process 9607
Reading symbols from /root/workspace/test...done.
Reading symbols from /lib64/librt.so.1...(no debugging symbols found)...done.
Loaded symbols for /lib64/librt.so.1
Reading symbols from /lib64/libc.so.6...(no debugging symbols found)...done.
Loaded symbols for /lib64/libc.so.6
Reading symbols from /lib64/libpthread.so.0...(no debugging symbols found)...done.
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
Loaded symbols for /lib64/libpthread.so.0
Reading symbols from /lib64/ld-linux-x86-64.so.2...(no debugging symbols found)...done.
Loaded symbols for /lib64/ld-linux-x86-64.so.2
0x00007efebe956650 in __nanosleep_nocancel () from /lib64/libc.so.6
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) b test.c:11
Breakpoint 1 at 0x4005fc: file test.c, line 11.
(gdb) c
Continuing.

Breakpoint 1, main (argc=1, argv=0x7fff02163f68) at test.c:11
11                      sum += i;
(gdb) p i
$1 = 89
(gdb) s
12                      sleep(1);
(gdb) n
9               for(i = 0; i< 1000; i++)
(gdb) n

Breakpoint 1, main (argc=1, argv=0x7fff02163f68) at test.c:11
11                      sum += i;
(gdb) p i
$2 = 90
{% endhighlight %}

* **detach**: 当你已经调试完所```attach```的进程，你就可以使用```detach```命令来释放GDB的对其的控制。```Detaching```会使得进程继续执行。在```detach```命令执行后，则该进程和GDB又变成各自独立的了，此时你可以attach到另一个进程，或者使用```run```方法来启动调试程序。注意： 在执行```detach```命令之后，按```RET```按键该命令(detach)并不会重复执行。


假如你在```attach```一个进程之后就退出了GDB，那么则会自动```detach```该进程。假如你使用```run```命令的话，则会杀死该进程。针对此情形，在默认情况下，假如你要退出GDB或者执行```run```命令，GDB都会要你进行再次的确认。

### 2.8 Kill掉子进程
* **kill**: kill掉在GDB下运行的某一个程序的子进程

该命令在你想要调试一个coredump，而不是一个运行的进程的情况下是很有用的。假如程序正在运行的情况下，GDB会忽略任何coredump文件。

此外在有一些操作系统上，假如该可执行程序在GDB中被设置了断点，那么该可执行文件将不能在GDB外执行。这种情况下，你可以在GDB中kill掉该子进程以允许可执行文件在GDB外被执行。

假如你想要重新编译和链接程序的话，那么你可以使用GDB的```kill```命令将该调试程序先杀死，因为对于很多系统来说在可以执行程序正在运行的过程中是不允许对该```可执行文件```进行修改的。在重新编译及链接之后，之后你可以继续执行```run```命令，这时GDB会通知你可执行文件已经发生了改变，然后GDB就会再重新读取符号表（但是会尝试保留当前的breakpoints设置）

### 2.9 Debugging Multiple Inferiors and Programs

**1) inferior基本概念**
 
GDB可以让你在一个session中运行和调试多个程序。另外，在有些系统上GDB允许你同时运行多个程序（否则你可能需要先退出GDB，然后才能再开始调试一个新的程序）。更一般的情况是，你有多个可执行文件同时运行，每一个可执行文件运行时会产生多个进程，而每一个进程又可能有多个线程。

GDB会使用一个名叫```inferior```的对象来代表每一个程序执行时的状态。典型情况下，一个```inferior```对应一个进程，更一般的情况也适用于那些没有进程概念的```targets```。```Inferiors```可能会在进程运行之前就被创建，并且可能在进程退出之后仍会被保留。每个```inferior```有与对应进程ID不同的唯一标识号。通常情况下，每一个```inferior```也有其自身独立的地址空间，尽管对于一些嵌入式```targets```来说可能会存在多个不同的```inferior```运行在同一个地址空间中。每一个```inferior```中也可能会有多个线程在运行。

如果要找出当前存在哪些```inferiors```，可以使用```info inferiors```命令：

* **info inferiors**: 用于打印GDB当前所管理的所有```inferiors```。GDB会按如下顺序来打印每一个```inferior```
{% highlight string %}
1) 由GDB所指定的inferior

2) 目标系统的inferior标识

3) 该inferior所对应的可执行文件的名称

（注： 其中GDB inferior number前面的'*'标识当前正在使用的inferior
{% endhighlight %}
例如：
<pre>
(gdb) info inferiors
Num Description Executable
2 process 2307 hello
* 1 process 3401 goodbye
</pre>

要想切换到另一个```inferior```，可以使用```inferior```命令。

* **inferior infno**: 将指定```infno```的inferior设置为```current inferior```。参数```infno```是由GDB所指定的```inferior```编号。

**2) 使用示例**

这里，在介绍具体的示例之前，我们先简单说明一下如下两个指令：
<pre>
1) set detach-on-fork off: 该指令用于告诉GDB在fork之后不要与子进程分离，这就意味着GDB会暂停子进程，并允许你之后再切换到该
   子进程上；

2） detach inferior ID: 与指定的inferior分离，这可以使得该inferior可以完全的执行
</pre>

我们通过如下的示例来展示```multi-inferior```调试：
{% highlight string %}
#include <stdio.h>
#include <unistd.h>


int main(int argc,char *argv[])
{
        pid_t pid = fork();
        
        printf("Hello\n");

        return 0x0;
}
{% endhighlight %}
编译调试。这里首先会在```printf```语句那里设置一个断点：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list
1       #include <stdio.h>
2       #include <unistd.h>
3
4
5       int main(int argc,char *argv[])
6       {
7               pid_t pid = fork();
8
9               printf("Hello\n");
10
(gdb) b test.c:9
Breakpoint 1 at 0x400594: file test.c, line 9.
{% endhighlight %}

接着设置```detach-on-fork```值为```off```，并执行```run```命令：
{% highlight string %}
(gdb) set detach-on-fork off
(gdb) run
Starting program: /root/workspace/./test 
[New process 21332]
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64

Breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:9
9               printf("Hello\n");
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
{% endhighlight %}

下面我们列出所有的```inferiors```:
{% highlight string %}
(gdb) info inferiors
  Num  Description       Executable        
  2    process 21332     /root/workspace/./test 
* 1    process 21328     /root/workspace/./test 
(gdb) shell ps -ef | grep "test"
root       9607   2548  0 06:24 pts/0    00:00:00 ./test
root      21309   2548  0 23:31 pts/0    00:00:00 gdb -q ./test
root      21328  21309  0 23:33 pts/0    00:00:00 /root/workspace/./test
root      21332  21328  0 23:33 pts/0    00:00:00 /root/workspace/./test
root      21436  21309  0 23:40 pts/0    00:00:00 bash -c ps -ef | grep "test"
root      21438  21436  0 23:40 pts/0    00:00:00 grep test
{% endhighlight %}

上面的```*```指示当前GDB正在检视父进程。

如下我们可以通过```inferior```命令切换到子进程，然后让其继续运行：
{% highlight string %}
(gdb) inferior 2
[Switching to inferior 2 [process 21332] (/root/workspace/./test)]
[Switching to thread 2 (process 21332)] 
#0  0x00007ffff7ada74c in fork () from /lib64/libc.so.6
(gdb) c
Continuing.

Breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:9
9               printf("Hello\n");
{% endhighlight %}

如下我们执行```n```命令，以step over到下一行，然后通过```detach```命令与该```inferior 2```分离，使其可以完全的执行：
{% highlight string %}
(gdb) n
Hello
11              return 0x0;
(gdb) detach inferior 2
Detaching from program: /root/workspace/./test, process 21332
(gdb) shell ps -ef | grep test
root      21309   2548  0 23:31 pts/0    00:00:00 gdb -q ./test
root      21328  21309  0 23:33 pts/0    00:00:00 /root/workspace/./test
root      21332  21328  0 23:33 pts/0    00:00:00 [test] <defunct>
root      21531  21309  0 23:51 pts/0    00:00:00 bash -c ps -ef | grep test
root      21533  21531  0 23:51 pts/0    00:00:00 grep test
(gdb) info inferiors
  Num  Description       Executable        
* 2    <null>            /root/workspace/./test 
  1    process 21328     /root/workspace/./test 
{% endhighlight %}
上面我们可以看到子进程已经执行完成，并退出，但仍处于```defunct```僵尸状态，这是因为父进程并未处理子进程的退出信号。

此时，我们在切回到```inferior 1```,然后让其完成运行：
{% highlight string %}
(gdb) inferior 1
[Switching to inferior 1 [process 21328] (/root/workspace/./test)]
[Switching to thread 1 (process 21328)] 
#0  main (argc=1, argv=0x7fffffffe638) at test.c:9
9               printf("Hello\n");
(gdb) c
Continuing.
Hello
[Inferior 1 (process 21328) exited normally]
(gdb) info inferiors
  Num  Description       Executable        
* 1    <null>            /root/workspace/./test 
(gdb) shell ps -ef | grep test
root      21309   2548  0 23:31 pts/0    00:00:00 gdb -q ./test
root      21617  21309  0 23:56 pts/0    00:00:00 bash -c ps -ef | grep test
root      21619  21617  0 23:56 pts/0    00:00:00 grep test
{% endhighlight %}
到此为止，父进程与子进程都已经退出。

**3） inferior相关的其他命令**

在一个```debug session```中，你可以通过```add-inferior```和```clone-inferior```命令获得多个```executables```。在有一些系统上，通过调用```fork()```或```exec()```可以自动的添加```inferior```到一个```debug session```中（如上面我们给出的例子）。如果要从一个```debug session```移除```inferior```，那么可以使用```remove inferiors```命令。

* **add-inferior [ -copies n ] [ -exec executable ]**: 使用```executable```向一个debug session中添加```n```个inferior； ```n```在默认情况下的值为1。假如并未指定```executable```，那么该新添加的```inferiors```将会为empty，并不包含任何程序。后续你仍可以通过使用```file```命令指定或更改该```inferior```所指向的程序。
{% highlight string %}
]# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) info inferiors
  Num  Description       Executable        
* 1    <null>            /root/workspace/./test 
(gdb) add-inferior -copies 3
Added inferior 2
Added inferior 3
Added inferior 4
(gdb) info inferiors
  Num  Description       Executable        
  4    <null>                              
  3    <null>                              
  2    <null>                              
* 1    <null>            /root/workspace/./test 
(gdb) inferior 3
[Switching to inferior 3 [<null>] (<noexec>)]
(gdb) file ./test
Reading symbols from /root/workspace/test...done.
(gdb) info inferiors
  Num  Description       Executable        
  4    <null>                              
* 3    <null>            /root/workspace/./test 
  2    <null>                              
  1    <null>            /root/workspace/./test 
{% endhighlight %}

* **clone-inferior [-copies n] [ infno ]**: 拷贝标识号为```infno```的```inferior```。默认情况下，```n```的值为1， ```infno```的值为```current inferior```的标识号。假如你想运行一个```inferior```的另一个实例，那么使用本命令就可以很容易的做到
{% highlight string %}
(gdb) info inferiors
Num Description Executable
* 1 process 29964 helloworld
(gdb) clone-inferior
Added inferior 2.
1 inferiors added.
(gdb) info inferiors
Num Description Executable
2 <null> helloworld
* 1 process 29964 helloworld
{% endhighlight %}
现在你就可以将```focus```切换到```inferior 2```上面来运行。

* **remove-inferiors infno...**: 移除指定的```inferior```。注意通过本命令并不能移除一个正在运行的```inferior```。如果要移除当前正在运行的```inferior```，那么需要首先执行```kill inferiors```或者```detach inferior```命令。


<br />
要想退出调试正处于运行状态非```current inferior```的```inferiors```，你可以使用```detach inferior```命令（这样使的该进程不受gdb的控制，自由独立的运行），或者使用```kill inferiors```命令：

* **detach inferior infno...**: 使GDB与指定的```inferior(s)```处于分离状态，这样这些```inferior```就不受GDB的控制，可以自由完全的执行。注意： 那些处于分离状态的```inferior```仍然会显示在```info inferiors```列表中，但是其描述将会被显示为NULL

* **kill inferiors infno...**: kill掉指定的```inferior(s)```。注意： 那些kill掉的inferior仍然会显示在```info inferiors```列表中，但是其描述将会被显示为NULL





<br />
<br />

**[参看]**


1. [GDB Inferior Tutorial](http://moss.cs.iit.edu/cs351/gdb-inferiors.html)

2. [gdb调试多进程与多线程](https://blog.csdn.net/snow_5288/article/details/72982594)


<br />
<br />
<br />





