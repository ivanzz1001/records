---
layout: post
title: cmake-20 find_package用法
tags:
- cplusplus
categories: cplusplus
description: cmake的使用
---

本文参考:

- [轻松搞定CMake”系列之find_package用法详解](https://blog.csdn.net/zhanghm1995/article/details/105466372)

- [cmake入门实战](https://rebootcat.com/2020/09/02/cmake/)

- [CMake模块化项目管理](https://zhuanlan.zhihu.com/p/631259689)

- [mastering cmake](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Finding%20Packages.html)


本文先从一个find_package()的例子出发，然后逐渐引出对find_package()命令搜包过程的介绍，以便深入理解find_package()的具体操作原理，帮助大家理解和消化。

<!-- more -->


## 1. 一个使用find_package()命令的例子

为了能够帮助大家理解find_package()命令的用法，此处首先用[opencv](https://opencv.org/)库举例子，示范如何通过find_package()命令找到opencv库并配置，从而能够在我们自己的项目中调用OpenCV库，实现特定的功能。


下面的代码主要实现了利用OpenCV载入一张图片并显示的简单功能：

>ps: 此处假设你已经安装了opencv库，并对opencv有稍许的了解即可

- opencv_test.cpp

```
#include <cstdio>
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char *argv[])
{
	Mat image;
	image = imread("./opencv_test.jpg");

	if(!image.data){
		printf("no image data\n");
		
		return -1;
	}

	namedWindow("Display Image", CV_WINDOW_AUTOSIZE);
	imShow("Display Image", image）；
	waitKey(0);

	return 0x0;
}
```

- CMakeLists.txt

```
cmake_mininum_required(VERSION 2.8)
project(find_package_learning)

find_package(OpenCV 3 REQUIRED)

message(STATUS "OpenCV_DIR = ${OpenCV_DIR}")
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS})")

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(opencv_test opencv_test.cpp)
target_link_libraries(opencv_test ${OpenCV_LIBS})
```

1) **编译运行**

在源码路径打开终端，执行:
<pre>
# mkdir build
# cd build
# cmake ..
# make -j4
# ./opencv_test

</pre>

2) **编译输出与分析**

我的Ubuntu18.04系统在usr/local路径下安装了OpencCV3.4.4，在执行上述```cmake ..```命令时输出为：

```
-- The C compiler identification is GNU 7.5.0
-- The CXX compiler identification is GNU 7.5.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found OpenCV: /usr/local (found suitable version "3.4.4", minimum required is "3") 
-- OpenCV_DIR = /usr/local/share/OpenCV
-- OpenCV_INCLUDE_DIRS = /usr/local/include;/usr/local/include/opencv
-- OpenCV_LIBS = opencv_calib3d;opencv_core;opencv_dnn;opencv_features2d;opencv_flann;opencv_highgui;opencv_imgcodecs;opencv_imgproc;opencv_ml;opencv_objdetect;opencv_photo;opencv_shape;opencv_stitching;opencv_superres;opencv_video;opencv_videoio;opencv_videostab;opencv_viz;opencv_aruco;opencv_bgsegm;opencv_bioinspired;opencv_ccalib;opencv_cvv;opencv_datasets;opencv_dnn_objdetect;opencv_dpm;opencv_face;opencv_freetype;opencv_fuzzy;opencv_hdf;opencv_hfs;opencv_img_hash;opencv_line_descriptor;opencv_optflow;opencv_phase_unwrapping;opencv_plot;opencv_reg;opencv_rgbd;opencv_saliency;opencv_stereo;opencv_structured_light;opencv_surface_matching;opencv_text;opencv_tracking;opencv_xfeatures2d;opencv_ximgproc;opencv_xobjdetect;opencv_xphoto
-- Configuring done
-- Generating done
-- Build files have been written to: /workspace/Programming/programming-learning-examples/cmake_learning/learn_cmake_easily/find_package_learning/build

```

重点看下其中```OpenCV_DIR```、```OpenCV_INCLUDE_DIRS```和```OpenCV_LIBS```打印的结果，这是我在CMakeLists.txt中用message命令输出这三个变量的值的结果。


可以看到在执行`find_package(OpenCV 3 REQUIRED)`命令后，CMake找到了我们安装在`/usr/local`下的OpenCV库，并设置了CMake变量`OpenCV_DIR`为OpenCV库的配置文件所在路径，正是通过载入这个路径下的`OpenCVConfig.cmake`配置文件才能配置好OpenCV库。然后在OpenCVConfig.cmake配置文件中定义了变量`OpenCV_INCLUDE_DIRS`为OpenCV库头文件包含路径，这样我们才能在代码中使用`#include <opencv2/opencv.hpp>`而不会出现编译错误，同时定义了变量`OpenCV_LIBS`为OpenCV链接库路径，这样我们才能正确链接到OpenCV中的库文件，而不会出现未定义的引用这样的链接错误。

所以整个过程可总结为：执行find_package()命令 => 进入到搜索路径/usr/local/share/OpenCV => 找到OpenCVConfig.cmake文件并执行该文件 => 定义OpenCV_INCLUDE_DIR、OpenCV_LIBS、OpenCV_DIR等变量 => 在CMakeLists.txt文件中引用变量完成编译。


通过上面这个例子就可以看出find_package()本质上就是一个搜包命令，通过一些特定的规则找到`<package_name>Config.cmake`包配置文件，然后执行该配置文件，从而定义了一系列的变量，通过这些变量就可以准确的定位到OpenCV库的头文件和库文件，完成编译。

那么关键的问题来了，find_package命令是怎么能够定位并载入指定库的配置文件呢？这就需要梳理一下find_package()命令的搜包过程。


## 2. find_package()的典型用法
绝大部分情况下我们可以使用如下形式来调用find_package():

```
find_package(<PackageName> [<version>] [REQUIRED] [COMPONENTS <components>...])
```

其中`<PackageName>`是唯一强制性的参数；`<version>`通常可以省略，`REQUIRED`参数用于指定工程的构建必须依赖于此包。一些更复杂的package支持`COMPONENTS`参数。

上面展示的是find_package()基本使用形式(basic signature)的简化版本，绝大部分情况下我们可以使用此形式来查找包。

通常情况下，理解find_package()的基本使用形式就已经足够了。而对于期望提供config package的工程维护人员来说，则需要对find_package()有更深的视角，我们会在下面的Full Signature一节进行说明。

## 3. find_package()的包搜索模式

find_package()命令有三种包搜索模式：

- Module Mode

- Config Mode

- FetchContent redirection mode

1) **Module Mode**

在此种模式下，CMake会查找一个名叫`Find<PackageName>.cmake`的文件。查找顺序如下：

- 在`CMAKE_MODULE_PATH`指定的路径下查找

- CMake安装时所提供的Find Modules路径(ps: 一般为/usr/local/cmake-3.28/modules，其中`3.28`为cmake版本号)


假如对应的`Find<PackageName>.cmake`文件找到了，CMake就会读取其中的内容并进行处理。CMake就是通过`Find<PackageName>.cmake`文件来负责package的查找、version的检查、以及产生其他相关必要的信息。

通常情况下package本身并不提供`Find<PackageName>.cmake`文件，反而经常能够看到由一些外部系统来提供（例如：操作系统、CMake自身、执行find_package命令的project）。通过由外部系统提供`Find<PackageName>.cmake`文件，这就使得CMake的Find Modules实现启发式查找，但同时也容易导致文件的过时。

>ps: Module Mode只支持在basic signature形式下使用

2） **Config Mode**

此种模式下，CMake会查找一个名叫`<lowercasePackageName>-config.cmake`或`<PackageName>Config.cmake`的文件。如果指定了version参数的话，则会查找名叫`<lowercasePackageName>-config-version.cmake`或`<PackageName>ConfigVersion.cmake`的文件。

在config模式下，该命令可以通过NAMES参数指定一系列的名称以用于包的查找。config模式下的查找路径也会比module模式复杂的多，我们会在后面的Config Mode Search Procedure一节进行说明。

通常情况下，前述的这些config与version文件会作为package的一部分被安装，因此这使得它们会比Find Modules更可靠。

>ps: Config Mode支持在basic signature和full signature形式下使用

3）**FetchContent redirection mode**

此中模式不做介绍。



----------
下面给出find_package()命令工作模式流程图：

> ps: 只有在find_package()中指定CONFIG、NO_MODULE等关键字，或者Module模式查找失败后才会进入到Config模式。

![cmake-find-package](https://ivanzz1001.github.io/records/assets/img/cplusplus/cmake/find_package_search.jpg)



### 3.1 Basic signature使用形式
```
find_package(<PackageName> [version] [EXACT] [QUIET] [MODULE]
             [REQUIRED] [[COMPONENTS] [components...]]
             [OPTIONAL_COMPONENTS components...]
             [REGISTRY_VIEW  (64|32|64_32|32_64|HOST|TARGET|BOTH)]
             [GLOBAL]
             [NO_POLICY_SCOPE]
             [BYPASS_PROVIDER])
```

basic signature使用形式同时支持Module、Config查找模式。其中```MODULE```关键字指示只使用Module Mode来查找包，而不会在查找失败的情况下回退到Config Mode。


在此种使用形式下不管采用哪种查找模式(Module Mode、Config Mode、FetchContent redirection Mode)，都会有一个<PackageName>_FOUND变量被设置，用于指示对应的package是否查找成功。当package查找成功，package特有的相关信息会通过其他变量来提供。如果指定了`QUIET`选项，会禁用相关的信息message，其中也包括那些指示package是否被找到的的信息(ps: 需要未指定`REQUIRED`选项）。当`REQUIRED`选项被指定，如果package未找到则会停止后续的流程，并打印出相关的error message


对于特定package所需要的components，则可以通过`COMPONENTS`关键字来列出。假如有任何一个component不被满足的话，则整个package会被认为是not found。假如同时指定了`REQUIRED`选项，则会被当做一个fatal error并停止后续的所有处理流程，否则后续的执行流程仍然会继续。一种简化的使用形式是，如果`REQUIRED`选项被指定，那么`COMPONENTS`关键字可以省略，可以直接在`REQUIRED`关键字后面列出对应的components。

`version`参数用于指定需要查找哪个版本的package，有两种不同的指定形式：

- 一个特定的单独版本号，格式为`major[.minor[.patch[.tweak]]]`

- 指定一个版本号范围，格式为```versionMin...[<]versionMax```


`EXACT`选项用于指明是否要严格匹配对应的version，该选项不能与范围版本号同时使用。



### 3.2 Full signature使用形式

```
find_package(<PackageName> [version] [EXACT] [QUIET]
             [REQUIRED] [[COMPONENTS] [components...]]
             [OPTIONAL_COMPONENTS components...]
             [CONFIG|NO_MODULE]
             [GLOBAL]
             [NO_POLICY_SCOPE]
             [BYPASS_PROVIDER]
             [NAMES name1 [name2 ...]]
             [CONFIGS config1 [config2 ...]]
             [HINTS path1 [path2 ... ]]
             [PATHS path1 [path2 ... ]]
             [REGISTRY_VIEW  (64|32|64_32|32_64|HOST|TARGET|BOTH)]
             [PATH_SUFFIXES suffix1 [suffix2 ...]]
             [NO_DEFAULT_PATH]
             [NO_PACKAGE_ROOT_PATH]
             [NO_CMAKE_PATH]
             [NO_CMAKE_ENVIRONMENT_PATH]
             [NO_SYSTEM_ENVIRONMENT_PATH]
             [NO_CMAKE_PACKAGE_REGISTRY]
             [NO_CMAKE_BUILDS_PATH] # Deprecated; does nothing.
             [NO_CMAKE_SYSTEM_PATH]
             [NO_CMAKE_INSTALL_PREFIX]
             [NO_CMAKE_SYSTEM_PACKAGE_REGISTRY]
             [CMAKE_FIND_ROOT_PATH_BOTH |
              ONLY_CMAKE_FIND_ROOT_PATH |
              NO_CMAKE_FIND_ROOT_PATH])
```

在纯CONFIG模式下，find_package()会跳过module搜索模式，并立即执行config模式搜索操作。

在Config模式下，find_package()会尝试查找对应的配置文件，并且会创建一个名叫`<PackageName>_DIR`的缓存条目来保存所查找到的配置文件目录。


## 4. Config Mode搜包流程
在config mode搜包模式下，CMake会为要搜索的package构建一系列可能的搜索路径。在每一个`<prefix>`前缀下会在多个目录下搜索配置文件，下面列出Windows、Unix操作系统环境下其会搜索的路径：
```
Entry                                                        Convention
------------------------------------------------------------------------
<prefix>/                                                         W
<prefix>/(cmake|CMake)/                                           W
<prefix>/<name>*/                                                 W
<prefix>/<name>*/(cmake|CMake)/                                   W
<prefix>/<name>*/(cmake|CMake)/<name>*/                           W

<prefix>/(lib/<arch>|lib*|share)/cmake/<name>*/                   U
<prefix>/(lib/<arch>|lib*|share)/<name>*/                         U
<prefix>/(lib/<arch>|lib*|share)/<name>*/(cmake|CMake)/           U

<prefix>/<name>*/(lib/<arch>|lib*|share)/cmake/<name>*/           W/U
<prefix>/<name>*/(lib/<arch>|lib*|share)/<name>*/                 W/U
<prefix>/<name>*/(lib/<arch>|lib*|share)/<name>*/(cmake|CMake)/   W/U
```

其中```<name>```不区分大小写，可以是`<PackageName>`或`NAMES`选项所指定的所有names。```<name>*```意思是包名后接一些版本号，比如name是`pcl`的话，那么pcl-1.9也会被找到。



搜素`<prefix>`按如下步骤来构建：

- 搜索以`<PackageName>`为前缀所定义的一些变量所指定的目录

  - `<PackageName>_ROOT` CMake变量所指定的目录，其中<PackageName>为要搜索的package的名称
  
  - `<PACKAGENAME>_ROOT` CMake变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写

  - `<PackageName>_ROOT` 环境变量所指定的目录，其中<PackageName>为要搜索的package的名称

  - `<PACKAGENAME>_ROOT` 环境变量所指定的目录，其中<PACKAGENAME>为要搜索的package的名称的大写


- 搜索cmake特定的缓存变量所指定的路径。这些缓冲变量通常倾向于通过命令行来传递(ps: 命令行传递方式为-DVAR=VALUE)。这些所传递的值以分号作为分隔。

   - CMAKE_PREFIX_PATH
   - CMAKE_FRAMEWORK_PATH
   - CMAKE_APPBUNDLE_PATH

- 搜索cmake特定的环境变量所指定的路径。这些变量倾向于通过用户shell来进行设置(比如Linux中export某个变量)

   - <PackageName>_DIR
   - CMAKE_PREFIX_PATH
   - CMAKE_FRAMEWORK_PATH
   - CMAKE_APPBUNDLE_PATH

- 搜索`HINTS`选项所指定的路径

- PATH环境变量路径

>ps: 还有一些其他不太常用的搜索路径，这里不再细述.








<br />
<br />
<br />


