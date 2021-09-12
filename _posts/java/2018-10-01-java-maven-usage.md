---
layout: post
title: maven的安装及使用
tags:
- java-language
categories: java-language
description: maven的安装及使用
---

本文介绍以下maven的安装及使用。

<!-- more -->


## 1. Maven介绍
[Apache maven](https://maven.apache.org/)是一个用于管理和理解软件工程(software project)的工具。基于```工程对象模型```(POM: project object model)的概念，maven可用于管理整个工程的构建，并可提供整个工程的报告及文档信息。

### 1.1 Maven起源

Maven其实是一个[Yiddish word](https://en.wikipedia.org/wiki/Maven)，其含义为积累知识，最早用于Jakarta Turbine项目中来简化项目的构建。当时Jakarta Turbine项目由多个工程组成，每一个工程都有自己的Ant构建文件，且均不相同，然后在工程构建出来后需要将Jar包放到CVS上。我们想要以标准的方式来完成项目的构建，清晰地展现项目的组成结构，便捷地发布项目信息，并实现在不同地项目间jar包的共享。

针对此目标，就开发了maven这样一个工具来```构建```(build)和```管理```(manage)任何基于java的项目，并希望能借此工具来更好的帮助Java开发人员理解整个项目。

### 1.2 Maven目标

Maven的主要目标是能够帮助开发人员尽快的理解整个项目的开发状态，为了实现此目标，Mave解决了以下几个方面的问题：

* Making the build process easy

* Providing a uniform build system

* Providing quality project information

* Encouraging better development practices 

1）简化构建过程

使用maven后，开发人员只需要对底层机制有一个基本的了解，对其中的一些细节可以忽略。

2）提供统一的构建系统

Maven使用其```工程对象模型```(POM)和一系列插件来完成项目的构建。一旦你熟悉了一个Maven工程之后，那么你将知道如何构建所有Maven工程。当你有许多不同的项目时，由于Maven提供了统一的构建系统，那么这可以极大的节省构建时间。

3） 提供高质量的工程信息

Maven可以从POM及项目源代码中提取有用的工程信息。比如，Maven可实现：

* Change log created directly from source control

* Cross referenced sources

* Mailing lists managed by the project

* Dependencies used by the project

* Unit test reports including coverage


4） 提供最佳开发实践指导

Maven瞄准于收集当前软件开发最佳实践，然后将其应用于项目开发中。比如，在日常的项目构建中，使用Maven可以完成单元测试的执行、报告。目前，单元测试的最佳实践有如下一些准则：

* 将测试源代码存放于一个并行，但独立的目录树中

* 使用一致的命名规则来索引(locate)和执行单元测试

* 提供独立的测试用例构建环境，而并不需要在预构建时对每一个测试用例的环境加以定制

在项目流程管理方面Maven也提供很好支持，如项目发布、issue管理等。

此外，对于工程的目录结构，Maven也提供了相应参考。一旦你学会了此种目录组织形式，你将可以更好的阅读其他的Maven项目。

## 2. Maven的特性

如下时Maven的一些关键特性总结：

* 遵循最佳实践的工程创建

* 工程间一致的使用方式

* 优越的依赖管理方式，这包括原子更新、依赖闭包

* 可轻易地同时管理多个工程

* 拥有大规模且持续增长地lib仓库和元数据仓库可使用，可随时获取到最大开源仓库中lib库地最新版本

* 可以通过简单地写插件地方式实现扩展

* 几乎无需任何额外地配置即可快速访问到最新特性

* 采用独立于Maven地ant来管理和部署依赖项

* 基于模型地构建方式：Maven可以将任意数量地工程构建为指定好地输出类型，比如JAR、WAR

* 清晰地工程信息：使用与构建进程(build process)相同地元数据，Maven就能生成web或PDF形式的文档，并将工程的开发状态添加到这些报告信息当中

* 版本管理及发布： 在并不需要太多额外配置的情况下，maven就可以与版本控制系统集成（如与subversion、git等的集成），然后基于特定的tag来管理工程的发布。其也可以发布到一个指定的地方供其他项目使用。Maven可以单独发布构建输出（如Jar包），也可以发布包含依赖项和文档信息的压缩包，甚至可以直接发布源代码。

* 依赖管理：Maven鼓励使用中央仓库的Jar包或依赖项。当你的工程发布给客户时，可以直接使用Maven来从中央仓库下载必要的依赖项，从而完成项目的构建。这就使得Maven用户可以跨工程复用Jar包。

## 3. 简单使用


### 3.1 Maven的下载及安装

这里我们首先安装JDK: 可以到[JDK官网](https://www.oracle.com/java/technologies/javase/javase-jdk8-downloads.html)去下载最近版本的jdk来进行安装。我们下载Linux下最新版本```jdk-8u301-linux-x64.rpm```，具体的安装步骤，这里不再细述。
<pre>
# javac -version
javac 1.8.0_301
</pre>



接下来我们讲述以下Maven的安装步骤。

1） Maven的下载

我们可以到[maven官网](https://maven.apache.org/download.cgi)去下载相应版本的Maven来安装，这里我们下载最新版本```Maven 3.8.2```:
<pre>
# mkdir maven-inst
# cd maven-inst
# wget https://dlcdn.apache.org/maven/maven-3/3.8.2/binaries/apache-maven-3.8.2-bin.tar.gz
</pre>

2）安装

其实maven不用特意进行安装，直接解压即可。这里我们将上述下载的maven包解压到/opt/目录：
<pre>
# tar -zxvf apache-maven-3.8.2-bin.tar.gz  -C /opt
# ls /opt/apache-maven-3.8.2/
bin  boot  conf  lib  LICENSE  NOTICE  README.txt
# ls /opt/apache-maven-3.8.2/conf
logging  settings.xml  toolchains.xml
</pre>
>注： 关于maven的配置文件settings.xml，我们后面会进行详细讲解


3) 将maven添加到PATH环境变量

这里我们修改/etc/profile文件，将上述maven路径设置到PATH中：
<pre>
# export PATH=$PATH:/opt/apache-maven-3.8.2/bin
</pre>

4) 验证

执行如下命令查看maven是否安装成功：
<pre>
# mvn -v
Apache Maven 3.8.2 (ea98e05a04480131370aa0c110b8c54cf726c06f)
Maven home: /opt/apache-maven-3.8.2
Java version: 1.8.0_301, vendor: Oracle Corporation, runtime: /usr/java/jdk1.8.0_301-amd64/jre
Default locale: en_US, platform encoding: UTF-8
OS name: "linux", version: "3.10.0-514.el7.x86_64", arch: "amd64", family: "unix"
</pre>

### 3.2 运行Maven

maven的基本使用语法如下：
{% highlight string %}
# mvn --help

usage: mvn [options] [<goal(s)>] [<phase(s)>]
{% endhighlight %}

在构建一个maven工程时，典型的做法是调用其生命周期中的一个phase，例如：
<pre>
# mvn package
</pre>

下面我们按顺序列出maven的内置生命周期，及对应周期内的phases:


* clean: pre-clean, clean, post-clean

* default: validate, initialize, generate-sources, process-sources, generate-resources, process-resources, compile, process-classes, generate-test-sources, process-test-sources, generate-test-resources, process-test-resources, test-compile, process-test-classes, test, prepare-package, package, pre-integration-test, integration-test, post-integration-test, verify, install, deploy

* site: pre-site, site, post-site, site-deploy

###### 3.2.1 用法举例

如果我们要全新的构建一个工程，产生所有的打包输出，提供文档site信息，并将其部署到仓库管理器中，我们执行如下：
<pre>
# mvn clean deploy site-deploy
</pre>

如果我们仅仅只想产生打包输出，然后将其安装到本地仓库中以供其他项目使用，我们可执行如下：
<pre>
# mvn verify
</pre>

上面这是我们构建一个maven工程最常见的调用方式。

此外，在一些其他使用场景下，我们想要使用maven的相关组件来实现一个特定的任务，比如要生成一个maven原型我们可以执行：
<pre>
# mvn archetype:generate
</pre>
或
<pre>
# mvn checkstyle:check
</pre>

### 3.3 Maven的配置

Apache Maven在自身使用以及构建项目时的一些配置信息主要会放在如下一些地方：

1） ```MAVEN_OPTS```环境变量 

该环境变量包含了JVM启动maven时的相关参数，我们可以通过该环境变量来提供一些额外的选项信息。例如，设置jvm的内存值为```-Xms256m -Xmx512m```.


2) settings.xml文件

存放于```USER_HOME/.m2```目录下的settings.xml文件可以跨工程被使用。

>注：其实在maven的安装目录下，有一个conf文件夹，里面也含有settings.xml。关于settings.xml文件，我们后面会单独讲解 

3） ```.mvn```目录

该目录位于相应project的顶级目录下，目录中的maven.config、jvm.conf、extensions.xml配置文件只会影响到本工程的maven运行。该目录是作为对应project的一部分，并会提交到版本控制系统。

* .mvn/extensions.xml文件

当需要使用一些扩展jar包时，旧的方式(maven 3.2.5之前版本）是手动将其放入```${MAVEN_HOME}/lib/ext```目录中。这意味着会修改maven安装目录。这造成的结果就是任何想要使用扩展jar包的用户都需要改变maven安装目录，这造成了极大的不便。另一种方式就是通过```-D```选项来指定扩展jar包的路径：
<pre>
# mvn -Dmaven.ext.class.path=extension.jar
</pre>
但这使用起来同样不太方便。

之后，我们可以使用一种更maven的方式来实现此功能，在```${maven.projectBasedir}/.mvn/extensions.xml```目录下添加一个内容类似如下的为念：
{% highlight string %}
<extensions xmlns="http://maven.apache.org/EXTENSIONS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/EXTENSIONS/1.0.0 http://maven.apache.org/xsd/core-extensions-1.0.0.xsd">
  <extension>
    <groupId/>
    <artifactId/>
    <version/>
  </extension>
</extensions>
{% endhighlight %}

通过如上我们就可以简单的使用扩展。更重要的是，maven可以自动的从仓库下载扩展(extension)及其依赖。

* .mvn/maven.config文件

在实际的场景中，其实是很难定义一个通用的选项来启动maven的。因此，从maven 3.3.1+开始，就可以将相应的选项放入一个脚本，而现在则可以直接将选项放入${maven.projectBasedir}/.mvn/maven.config文件中即可。


例如，有```-T3 -U --fail-at-end```选项。将其放入${maven.projectBasedir}/.mvn/maven.config文件中后，我们就只需要简单执行```mvn clean package```即可自动包含前面的选项，而不必每一次都执行如下：
<pre>
# mvn -T3 -U --fail-at-end clean package
</pre>

>注：在一个project中有多个modules需要构建时其仍然有效。

* .mvn/jvm.config文件

从maven 3.3.1+开始，可以通过${maven.projectBasedir}/.mvn/jvm.config来定义启动maven时的jvm参数。这样该文件成为整个工程的一部分，可以直接由版本控制工具管理。因此，可以不再需要```MAVEN_OPTS```环境变量，或者```.mavenrc```文件了。比如，我们可以将如下选项放入${maven.projectBasedir}/.mvn/jvm.config文件中：
<pre>
-Xmx2048m -Xms1024m -XX:MaxPermSize=512m -Djava.awt.headless=true
</pre>

### 3.4 maven与IDE集成

Maven可以与Apache NetBeans IDE、Eclipse IDE - M2Eclipse、JetBrains IntelliJ IDEA，这里不详细介绍。

参看：

* [Apache Maven IDE Integration](https://maven.apache.org/ide.html)

* [feature-rich integration for Maven](https://www.jetbrains.com/idea/help/maven.html)


## 4. Maven Settings

在Maven安装之后，在我们的安装目录/opt/apache-maven-3.8.2/conf下，有Maven的配置文件settings.xml:
<pre>
# tree
.
├── logging
│   └── simplelogger.properties
├── settings.xml
└── toolchains.xml
</pre>

下面我们先简单看一下安装后默认的settings.xml的内容：
{% highlight string %}
<?xml version="1.0" encoding="UTF-8"?>

<!--
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-->

<!--
 | This is the configuration file for Maven. It can be specified at two levels:
 |
 |  1. User Level. This settings.xml file provides configuration for a single user,
 |                 and is normally provided in ${user.home}/.m2/settings.xml.
 |
 |                 NOTE: This location can be overridden with the CLI option:
 |
 |                 -s /path/to/user/settings.xml
 |
 |  2. Global Level. This settings.xml file provides configuration for all Maven
 |                 users on a machine (assuming they're all using the same Maven
 |                 installation). It's normally provided in
 |                 ${maven.conf}/settings.xml.
 |
 |                 NOTE: This location can be overridden with the CLI option:
 |
 |                 -gs /path/to/global/settings.xml
 |
 | The sections in this sample file are intended to give you a running start at
 | getting the most out of your Maven installation. Where appropriate, the default
 | values (values used when the setting is not specified) are provided.
 |
 |-->
<settings xmlns="http://maven.apache.org/SETTINGS/1.2.0"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
          xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.2.0 https://maven.apache.org/xsd/settings-1.2.0.xsd">
  <!-- localRepository
   | The path to the local repository maven will use to store artifacts.
   |
   | Default: ${user.home}/.m2/repository
  <localRepository>/path/to/local/repo</localRepository>
  -->

  <!-- interactiveMode
   | This will determine whether maven prompts you when it needs input. If set to false,
   | maven will use a sensible default value, perhaps based on some other setting, for
   | the parameter in question.
   |
   | Default: true
  <interactiveMode>true</interactiveMode>
  -->

  <!-- offline
   | Determines whether maven should attempt to connect to the network when executing a build.
   | This will have an effect on artifact downloads, artifact deployment, and others.
   |
   | Default: false
  <offline>false</offline>
  -->

  <!-- pluginGroups
   | This is a list of additional group identifiers that will be searched when resolving plugins by their prefix, i.e.
   | when invoking a command line like "mvn prefix:goal". Maven will automatically add the group identifiers
   | "org.apache.maven.plugins" and "org.codehaus.mojo" if these are not already contained in the list.
   |-->
  <pluginGroups>
    <!-- pluginGroup
     | Specifies a further group identifier to use for plugin lookup.
    <pluginGroup>com.your.plugins</pluginGroup>
    -->
  </pluginGroups>

  <!-- proxies
   | This is a list of proxies which can be used on this machine to connect to the network.
   | Unless otherwise specified (by system property or command-line switch), the first proxy
   | specification in this list marked as active will be used.
   |-->
  <proxies>
    <!-- proxy
     | Specification for one proxy, to be used in connecting to the network.
     |
    <proxy>
      <id>optional</id>
      <active>true</active>
      <protocol>http</protocol>
      <username>proxyuser</username>
      <password>proxypass</password>
      <host>proxy.host.net</host>
      <port>80</port>
      <nonProxyHosts>local.net|some.host.com</nonProxyHosts>
    </proxy>
    -->
  </proxies>

  <!-- servers
   | This is a list of authentication profiles, keyed by the server-id used within the system.
   | Authentication profiles can be used whenever maven must make a connection to a remote server.
   |-->
  <servers>
    <!-- server
     | Specifies the authentication information to use when connecting to a particular server, identified by
     | a unique name within the system (referred to by the 'id' attribute below).
     |
     | NOTE: You should either specify username/password OR privateKey/passphrase, since these pairings are
     |       used together.
     |
    <server>
      <id>deploymentRepo</id>
      <username>repouser</username>
      <password>repopwd</password>
    </server>
    -->

    <!-- Another sample, using keys to authenticate.
    <server>
      <id>siteServer</id>
      <privateKey>/path/to/private/key</privateKey>
      <passphrase>optional; leave empty if not used.</passphrase>
    </server>
    -->
  </servers>

  <!-- mirrors
   | This is a list of mirrors to be used in downloading artifacts from remote repositories.
   |
   | It works like this: a POM may declare a repository to use in resolving certain artifacts.
   | However, this repository may have problems with heavy traffic at times, so people have mirrored
   | it to several places.
   |
   | That repository definition will have a unique id, so we can create a mirror reference for that
   | repository, to be used as an alternate download site. The mirror site will be the preferred
   | server for that repository.
   |-->
  <mirrors>
    <!-- mirror
     | Specifies a repository mirror site to use instead of a given repository. The repository that
     | this mirror serves has an ID that matches the mirrorOf element of this mirror. IDs are used
     | for inheritance and direct lookup purposes, and must be unique across the set of mirrors.
     |
    <mirror>
      <id>mirrorId</id>
      <mirrorOf>repositoryId</mirrorOf>
      <name>Human Readable Name for this Mirror.</name>
      <url>http://my.repository.com/repo/path</url>
    </mirror>
     -->
    <mirror>
      <id>maven-default-http-blocker</id>
      <mirrorOf>external:http:*</mirrorOf>
      <name>Pseudo repository to mirror external repositories initially using HTTP.</name>
      <url>http://0.0.0.0/</url>
      <blocked>true</blocked>
    </mirror>
  </mirrors>

  <!-- profiles
   | This is a list of profiles which can be activated in a variety of ways, and which can modify
   | the build process. Profiles provided in the settings.xml are intended to provide local machine-
   | specific paths and repository locations which allow the build to work in the local environment.
   |
   | For example, if you have an integration testing plugin - like cactus - that needs to know where
   | your Tomcat instance is installed, you can provide a variable here such that the variable is
   | dereferenced during the build process to configure the cactus plugin.
   |
   | As noted above, profiles can be activated in a variety of ways. One way - the activeProfiles
   | section of this document (settings.xml) - will be discussed later. Another way essentially
   | relies on the detection of a system property, either matching a particular value for the property,
   | or merely testing its existence. Profiles can also be activated by JDK version prefix, where a
   | value of '1.4' might activate a profile when the build is executed on a JDK version of '1.4.2_07'.
   | Finally, the list of active profiles can be specified directly from the command line.
   |
   | NOTE: For profiles defined in the settings.xml, you are restricted to specifying only artifact
   |       repositories, plugin repositories, and free-form properties to be used as configuration
   |       variables for plugins in the POM.
   |
   |-->
  <profiles>
    <!-- profile
     | Specifies a set of introductions to the build process, to be activated using one or more of the
     | mechanisms described above. For inheritance purposes, and to activate profiles via <activatedProfiles/>
     | or the command line, profiles have to have an ID that is unique.
     |
     | An encouraged best practice for profile identification is to use a consistent naming convention
     | for profiles, such as 'env-dev', 'env-test', 'env-production', 'user-jdcasey', 'user-brett', etc.
     | This will make it more intuitive to understand what the set of introduced profiles is attempting
     | to accomplish, particularly when you only have a list of profile id's for debug.
     |
     | This profile example uses the JDK version to trigger activation, and provides a JDK-specific repo.
    <profile>
      <id>jdk-1.4</id>

      <activation>
        <jdk>1.4</jdk>
      </activation>

      <repositories>
        <repository>
          <id>jdk14</id>
          <name>Repository for JDK 1.4 builds</name>
          <url>http://www.myhost.com/maven/jdk14</url>
          <layout>default</layout>
          <snapshotPolicy>always</snapshotPolicy>
        </repository>
      </repositories>
    </profile>
    -->

    <!--
     | Here is another profile, activated by the system property 'target-env' with a value of 'dev',
     | which provides a specific path to the Tomcat instance. To use this, your plugin configuration
     | might hypothetically look like:
     |
     | ...
     | <plugin>
     |   <groupId>org.myco.myplugins</groupId>
     |   <artifactId>myplugin</artifactId>
     |
     |   <configuration>
     |     <tomcatLocation>${tomcatPath}</tomcatLocation>
     |   </configuration>
     | </plugin>
     | ...
     |
     | NOTE: If you just wanted to inject this configuration whenever someone set 'target-env' to
     |       anything, you could just leave off the <value/> inside the activation-property.
     |
    <profile>
      <id>env-dev</id>

      <activation>
        <property>
          <name>target-env</name>
          <value>dev</value>
        </property>
      </activation>

      <properties>
        <tomcatPath>/path/to/tomcat/instance</tomcatPath>
      </properties>
    </profile>
    -->
  </profiles>

  <!-- activeProfiles
   | List of profiles that are active for all builds.
   |
  <activeProfiles>
    <activeProfile>alwaysActiveProfile</activeProfile>
    <activeProfile>anotherAlwaysActiveProfile</activeProfile>
  </activeProfiles>
  -->
</settings>
{% endhighlight %}

### 4.1 Quick Overview
在settings.xml文件中，```<settings>```节点为为Maven的执行配置一些全局变量的值（注： 对于project级别的配置，一般是通过POM文件来完成）。我们可以在settings.xml文件中配置本地仓库地址、远程仓库服务器、以及相关的认证信息等。

settings.xml可能存在于如下两个地方：

* Maven安装目录： ${maven.home}/conf/settings.xml

* 用户home目录：${user.home}/.m2/settings.xml

其中，Maven安装目录的settings.xml是作为全局配置；而用户home目录的settings.xml是作为用户级别的配置。假如两个配置文件都存在的话，相关的内容将会被合并(merged)，并且用户Home目录的settings.xml中配置的值优先。

>提示： 假如你想要创建一个用户级别的settings.xml配置，最简单的方法是拷贝Maven安装目录下的settings.xml文件，然后对其中的内容做修改。

如下是settings.xml配置文件的整体结构：
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  <localRepository/>
  <interactiveMode/>
  <offline/>
  <pluginGroups/>
  <servers/>
  <mirrors/>
  <proxies/>
  <profiles/>
  <activeProfiles/>
</settings>
{% endhighlight %}
在settings.xml文件中可以使用如下一些表达式：

* ${user.home}和其他的一些系统参数（since Maven 3.0)

* ${env.HOME}等环境变量值

### 4.2 Setting Details

1) **Simple Values**

在settings.xml文件中，超过一半的顶级(top-level)元素都是simple values，这些值在全时段都是有效的：
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  <localRepository>${user.home}/.m2/repository</localRepository>
  <interactiveMode>true</interactiveMode>
  <offline>false</offline>
  ...
</settings>
{% endhighlight %}

* localRepository: 该元素用于设置构建系统的本地仓库路径。默认值为```${user.home}/.m2/repository```。我们可以通过该字段为所有登录的用户设置一个公共的本地仓库地址

* interactiveMode： 假如我们想Maven可以接受用户的交互性输入的话，设置为true；否则设置为false。默认值为true

* offline: 假如maven构建系统应该工作在offline模式，则设置为true； 否则设置为false。假如由于网络或者安全等某种原因，导致构建服务器并不能连接上远程仓库，则可以通过本元素来进行设置。

2） **Plugin Groups**

本节点可以一系列的```pluginGroup```元素。当某个plugin被使用，且未在命令行指定对应groupId时，就会查找此plugin Groups列表。此列表会自动包含org.apache.maven.plugins与org.codehaus.mojo。

{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <pluginGroups>
    <pluginGroup>org.eclipse.jetty</pluginGroup>
  </pluginGroups>
  ...
</settings>
{% endhighlight %}
例如，给定上面的设置，通过maven命令行我们就可以如下精简方式执行```org.eclipse.jetty:jetty-maven-plugin:run```：
<pre>
# mvn jetty:run
</pre>


3) **Servers**

我们可以通过POM文件的```repositories```和```distributionManagement```来设置下载和部署仓库。然而，很多设置（如用户名、密码）可能并不适合放在pom.xml中随项目一起发布。这种类型的信息应该配置在build server级别的settings.xml文件比较何时：
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <servers>
    <server>
      <id>server001</id>
      <username>my_login</username>
      <password>my_password</password>
      <privateKey>${user.home}/.ssh/id_dsa</privateKey>
      <passphrase>some_passphrase</passphrase>
      <filePermissions>664</filePermissions>
      <directoryPermissions>775</directoryPermissions>
      <configuration></configuration>
    </server>
  </servers>
  ...
</settings>
{% endhighlight %}

* id: Maven所要连接的repository/mirror服务器的ID值(注意：这不是登录用户)

* username/password: 登录相应服务器的用户名/密码

* privateKey/passphrase: 与上面的username/password类似，本字段是用于指定私钥的路径（默认值为：${user.home}/.ssh/id_dsa）以及私钥密码(```passphrase```)。未来，paasphrase与password这两个字段可能会外部化，但目前是将他们明文设置在settings.xml文件中。

* filePermissions/directoryPermissions: 当在部署时，需要创建一个仓库文件/目录，则会使用本字段给定的权限来创建。其合法的值与Linux文件系统权限类似。

>注： 假如你使用私钥来登录服务器的话，确保省略```<password>```元素。否则，key会被忽略

4）**Password Encryption** 

这是Maven的一个新特性，从2.1.0版本开始，maven已经支持服务器password和passphrase加密

5） **Mirrors**
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <mirrors>
    <mirror>
      <id>planetmirror.com</id>
      <name>PlanetMirror Australia</name>
      <url>http://downloads.planetmirror.com/pub/maven2</url>
      <mirrorOf>central</mirrorOf>
    </mirror>
  </mirrors>
  ...
</settings>
{% endhighlight %}
* id/name: mirror的唯一标识符(identifier)和用户友好的名称字段。在不同mirror之间，是通过id来进行区分的，并且以此来在```servers```元素（上面介绍的Servers）中查找对应的鉴权信息。

* url: mirror的URL基础地址。maven构建系统会使用此地址来连接到仓库，而不是使用原始的仓库地址。

* mirrorOf: 本镜像所对应的原始仓库的ID。例如，指定一个镜像映射到Maven的中央仓库（https://repo.maven.apache.org/maven2/），那么可以将本字段设置为```central```。

关于mirrors更详细的介绍，请参看[Guide to Mirror Settings](https://maven.apache.org/guides/mini/guide-mirror-settings.html)

6) **Proxies**
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <proxies>
    <proxy>
      <id>myproxy</id>
      <active>true</active>
      <protocol>http</protocol>
      <host>proxy.somewhere.com</host>
      <port>8080</port>
      <username>proxyuser</username>
      <password>somepassword</password>
      <nonProxyHosts>*.google.com|ibiblio.org</nonProxyHosts>
    </proxy>
  </proxies>
  ...
</settings>
{% endhighlight %}

* id: 该代理的唯一标识符。不同proxy元素通过此字段来区分

* active: 设置此proxy为active状态。我们可以同时设置多个代理，但一次只能有一个处于active状态。

* protocol/host/port: 连接代理时所采用的协议、地址和端口（protocol://host:port）

* username/password: 连接代理时的用户名/密码

* nonProxyHosts: 指定禁止被代理的主机列表。主机列表间的分隔符是根据proxy server来进行设置的

7） **Profiles**

settings.xml中的```profile```元素其实是```pom.xml```中profile元素的精简。其包含activation、repositories、pluginRepositories、properties子元素。```profile```节点只包含前面列的4个子节点，这是因为其作为一个整体只关乎构建系统自身，而不需要关心project级别的POM设置

假如settings中的一个profile处于active状态，那么这些值将会覆盖POM文件或```profiles.xml```文件中对应ID的profile


7.1) Activation

activations是一个profile的主要部分。与POM中的profiles类似，profile的强大能力来源于其只能在特定条件下修改某些值。这些特定条件是通过```activation```元素来指定的：
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <profiles>
    <profile>
      <id>test</id>
      <activation>
        <activeByDefault>false</activeByDefault>
        <jdk>1.5</jdk>
        <os>
          <name>Windows XP</name>
          <family>Windows</family>
          <arch>x86</arch>
          <version>5.1.2600</version>
        </os>
        <property>
          <name>mavenVersion</name>
          <value>2.0.3</value>
        </property>
        <file>
          <exists>${basedir}/file2.properties</exists>
          <missing>${basedir}/file1.properties</missing>
        </file>
      </activation>
      ...
    </profile>
  </profiles>
  ...
</settings>
{% endhighlight %}
当所有特定的准则（注：并不是所有准则）都满足时，对应的activation才会触发。

* jdk: activation有一个内置的jdk版本检查。假如测试所运行的jdk版本低于所设定的匹配值，该activation将会被触发。在本例子中，版本```1.5.0_06```将会匹配上面的设定值。此外，本字段还支持范围(range)的设置，具体range设置可查看[ maven-enforcer-plugin](https://maven.apache.org/enforcer/enforcer-rules/versionRanges.html)

* os: 本节点定义一些操作系统相关的属性。具体的OS属性可查看[ maven-enforcer-plugin](https://maven.apache.org/enforcer/enforcer-rules/versionRanges.html)

* property: 假如Maven侦测到一个属性值相同的话，activation将会被激活。

* file: 假如某个文件存在/不存在，对应的activation将会被激活。

activation元素并不是profile被激活的唯一方法。settings.xml文件中的```activeProfile```元素可能包含profile的ID。我们也可以通过命令行的方式来显式激活profile。

如果想查哪一个profile在构建期间会被激活，使用```maven-help-plugin```:
<pre>
# mvn help:active-profiles
</pre>


7.2) Properties

Maven properties其实是一个值占位符，与Ant中的properties类似。这些值可以在POM中的任何位置被访问到，访问形式为```${X}```，其中```X```为属性名称。properties在settings.xml文件中有5种不同的类型。

* ```env.X```: 通过"env."这样一个前缀可以访问shell环境变量的值。比如```${env.PATH}```就可以读取系统的PATH环境变量

* ```project.X```: 在POM中可以通过以"."分割的路径来访问元素的值。例如，```<project><version>1.0</version></project>```可以通过```${project.version}```来访问。

* ```settings.X```: settings.xml中以"."分割的路径访问对应元素的值。例如，```<settings><offline>false</offline></settings>```可以通过```${settings.offline}```来访问。

* Java的系统properties: 所有通过java.lang.System.GetProperties()访问的属性都可在POM中被访问，例如: ```${java.home}```

* ```X```: <properties />节点中的元素都可以通过```${somevar}```来访问

{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <profiles>
    <profile>
      ...
      <properties>
        <user.install>${user.home}/our-project</user.install>
      </properties>
      ...
    </profile>
  </profiles>
  ...
</settings>
{% endhighlight %}

假如该profile处于active状态，那么在POM中可以通过```${user.install}```来访问。

7.3) Repositories

Repositories其实是远程projects的一个集合，Maven可以使用其来扩充构建系统的本地仓库。不同的远程仓库可能会包含不同的projects，在active profile中会从这些仓库中搜索匹配的artifact版本。
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <profiles>
    <profile>
      ...
      <repositories>
        <repository>
          <id>codehausSnapshots</id>
          <name>Codehaus Snapshots</name>
          <releases>
            <enabled>false</enabled>
            <updatePolicy>always</updatePolicy>
            <checksumPolicy>warn</checksumPolicy>
          </releases>
          <snapshots>
            <enabled>true</enabled>
            <updatePolicy>never</updatePolicy>
            <checksumPolicy>fail</checksumPolicy>
          </snapshots>
          <url>http://snapshots.maven.codehaus.org/maven2</url>
          <layout>default</layout>
        </repository>
      </repositories>
      <pluginRepositories>
        <pluginRepository>
          <id>myPluginRepo</id>
          <name>My Plugins repo</name>
          <releases>
            <enabled>true</enabled>
          </releases>
          <snapshots>
            <enabled>false</enabled>
          </snapshots>
          <url>https://maven-central-eu....com/maven2/</url>
        </pluginRepository>
      </pluginRepositories>
      ...
    </profile>
  </profiles>
  ...
</settings>
{% endhighlight %}

* releases, snapshots: 每一种类型的artifact对应的策略。通过这两个集合，POM有能力修改对应的策略以单独支持其中的任何一个类型。例如，由于某种开发目的，我们只打算支持snapshot下载

* enabled: 指明对应的仓库是否支持相应的类型(releases/snapshots)

* updatePolicy: 本节点指明更新策略。Maven会将本地的POM时间戳(存放于仓库的maven-metadata文件中）与远程仓库中的进行对比。可选项有： always、daily(default)、```interval:x```(其中x是一个表示分钟的整数）、never。

* checksumPolicy： 当Maven部署文件到仓库时，其也会部署对应的checksum文件。当对应的checksum丢失或不正确时，可根据本字段设置的选项(ignore/fail/warn)产生相应的动作

* layout: 在上面对repositories的描述中，这些仓库都遵循一个common layout。这通常是可以正常工作的。Maven2对其仓库也有一个默认的layout；然而Maven 1.x有一个不同的layout。可以使用此节点来设置layout选项(default/legacy)

7.4) Plugin Repositories

repositories主要存放两种类型的artifacts。其中第一种artifacts是作为其他artifacts的依赖，这类artifacts在中央仓库中占绝大多数。另一种类型的artifacts即为plugins。Maven plugins其本身也是一种特殊的artifact。正因为如此，plugin仓库也可以是一个单独的仓库。在任何情况下，pluginRepositories节点与repositories节点都相似。Maven会从pluginRepositories所配置的远程仓库中查找新的plugin


8) **Active Profiles**
{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <activeProfiles>
    <activeProfile>env-test</activeProfile>
  </activeProfiles>
</settings>
{% endhighlight %}

settings.xml文件的最后一节就是```activeProfiles```。其包含了一系列的```activeProfile```元素，其中每一个元素的值都是一个profile id。不管环境如何设置，任何定义在activeProfile中的profile id都是active的。假如通过profile id没有找到对应的profile，则忽略。例如，在本例子中，假如```env-test```是一个activeProfile，那么任何定义在```pom.xml```或```profile.xml```中ID为env-test的profile都是active的。假如在这些文件中并没有找到对应的profile，则忽略。



## 5. Maven可用插件

Maven本质上其实是一个插件执行架构，所有的工作都是由插件来完成的。如何找到一个特定的目标(goal)来执行呢？本章将会列出maven的核心插件。Maven的插件主要分为两类，分别是build plugins和reporting plugins：

* Build plugins: 此类插件会在构建的时候被执行，其应该配置于POM文件的```<build/>```节点处；

* Reporting plugings: 此类插件会在site generation阶段被执行，其应该配置于POM文件的```<reporting/>```节点处。由于reporting plugins是产生相关文档信息，因此其应该要支持国际化和本地化。


### 5.1 Maven所支持的plugins

我们可以在[org/apache/maven/plugins](https://repo.maven.apache.org/maven2/org/apache/maven/plugins/)子目录下查看Maven最新的仓库。下面我们按分类介绍以下Maven常用的一些plugins:

1) Core plugins

* clean plugin: 用于执行build之后的清理工作（属于build plugin)

* compiler plugin: 用于编译java源代码（属于build plugin)

* deploy plugin: 将build的输出结果部署到远程仓库(属于build plugin)

* failsafe plugin: 在一个隔离的ClassLoader中执行JUnit集成测试(属于build plugin)

* install plugin: 安装build的输出结果到本地仓库(属于build plugin)

* resources plugin: 将资源文件拷贝到输出目录，以打包在jar文件中（属于build plugin)

* site plugin: 为当前project产生site信息（属于build plugin)

* surefire plugin: 在一个隔离的classloader中执行junit单元测试(属于build plugin)

* verfier plugin: 用于集成测试，校验特定的条件是否存在（属于build plugin)

2) packaging types/tools

* ear: 从当前工程产生一个EAR包(属于build plugin)

* ejb: 从当前工程产生一个ejb包(属于build plugin)

* jar: 将当前工程构建为一个jar包(属于build plugin)

* rar: 将当前工程构建为一个rar包（属于build plugin)

* war: 将当前工程构建为一个war包(属于build plugin)

* app-client/acr: 将当前工程构建为一个应用程序客户端（属于build plugin)

* shade: 将当前工程构建为一个Uber-Jar，这包括相应的依赖(属于build plugin)

* source: 将当前工程构建为一个source-Jar（属于build plugin)

* jlink: 构建为一个Java Run Time Image

* jmod: 构建Java JMod文件

3） Reporting plugings

* changelog: 从SCM中产生最近修改列表；

* changes: 从issue tracker或者change document中产生相应报告

* checkstyle: 产生一个checkstyle报告

* doap: 从POM文件中产生项目描述(DOAP: Description Of A Project)报告

* docck: 为checker plugin生成相关文档信息

* javadoc: 为工程生成javadoc

* jdeps: 在工程中运行JDK的JDeps工具

* jxr: Generate a source cross reference

* linkcheck: 为工程文档产生一个linkcheck报告

* pmd: 产生一个PMD报告

* project-info-reports: 产生标准的工程报告

* surefire-report: 根据单元测试结果产生相应报告

3) tools

* antrun: 在build的阶段执行一系列ant任务

* artifact: 像buildinfo那样管理artifacts任务

* archetype: 从archetype中产生项目结构框架

* assembly: 从源代码和(或)binaries中构建一个发布集合

* dependency: 依赖操作(copy、unpack)和分析

* enforcer: 环境限制检查（Maven Version、JDK等），用户自定义规则执行

* gpg: 从artifacts以及poms中产生signature

* help: 为工程生成相应工作环境信息

* invoker: 运行一系列Maven工程，并校验输出结果

* jarsigner: Signs or verifies project artifacts.

* jdeprscan: 在工程中运行JDK的JDeprScan工具

* patch: 使用GNU的patch工具来向源代码打补丁

* pdf: 为工程文档产生一个PDF版本

* plugin: 为源代码中的所有mojo产生一个Maven plugin描述，并且会将相应的信息包含在JAR包中

* release: 	Release the current project - updating the POM and tagging in the SCM

* remote-resources: 将远程资源拷贝到输出目录

* scm: 为当前工程执行SCM命令

* scm-publish: 将Maven website发布到一个scm位置

* scripting: The Maven Scripting Plugin wraps the Scripting API according to JSR223

* stage: Assists with release staging and promotion

* toolchains: 允许跨plugin共享配置

* wrapper: Download and unpack the maven wrapper distribution (works only with Maven 4)


## 6. Maven Extensions
Maven本质上其实是一个插件执行架构，大部分工作都是通过plugins来完成的。然而，通过extensions，我们可以将相关的hooks添加进Maven当中，比如通过extensions来操作lifecycle。

### 6.1 Configure Extensions

Extensions是将libraries添加到Core ClassLoader的一种方法。

Extensions的一个典型应用场景是启用Wagon providers，用于操作artifacts在repositories之间的传输；而plugins典型应用场景是用于Maven lifecycle的增强。

1） Wagon providers
{% highlight string %}
<project>
  ...
  <build>
    <extensions>
      <extension>
        <groupId>org.apache.maven.wagon</groupId>
         <artifactId>wagon-ftp</artifactId>
         <version>2.10</version>
      </extension>
    </extensions>
  </build>
  ...
</project>
{% endhighlight %}


2) 用于Maven lifecycle增强的plugin
{% highlight string %}
<project>
...
<build>
<plugins>
  <plugin>
    <groupId>org.apache.felix</groupId>
    <artifactId>maven-bundle-plugin</artifactId>
    <extensions>true</extensions>
    <configuration>
      ...
    </configuration>
  </plugin>
</plugins>
</build>
{% endhighlight %}


### 6.2 如何编写一个Extension

这里我们通过一个示例来讲解如何编写一个lifecycle extension。

1） Lifecyle Participation

取决于我们的需要，我们可以通过继承多个类来介入Maven的lifecycle:

* org.apache.maven.execution.AbstractExecutionListener

* org.apache.maven.AbstractMavenLifecycleParticipant


* org.apache.maven.eventspy.AbstractEventSpy


2) 构建一个extension

这里我们基于org.apache.maven:maven-core:3.8.2及其他一些依赖来创建一个Maven工程：
{% highlight string %}
<groupId>org.apache.maven.extensions</groupId>
<artifactId>beer-maven-lifecycle</artifactId>
<version>1.0-SNAPSHOT</version>

<dependency>
  <groupId>org.apache.maven</groupId>
  <artifactId>maven-core</artifactId>
  <version>3.8.2</version>
</dependency>

<!-- dependency for plexus annotation -->
<dependency>
  <groupId>org.codehaus.plexus</groupId>
  <artifactId>plexus-component-annotations</artifactId>
  <version>1.7.1</version>
</dependency>
{% endhighlight %}

然后，创建一个实现类：
{% highlight string %}
// your extension must be a "Plexus" component so mark it with the annotation
@Component( role = AbstractMavenLifecycleParticipant.class, hint = "beer")
public class BeerMavenLifecycleParticipant extends AbstractMavenLifecycleParticipant
{
 
    @Override
    public void afterSessionStart( MavenSession session )
        throws MavenExecutionException
    {
      // start the beer machine
    }
 
 
    @Override
    public void afterProjectsRead( MavenSession session )
        throws MavenExecutionException
    {
      // ask a beer to the machine
    }
 
}
{% endhighlight %}

在构建为extension生成相应元数据：
{% highlight string %}
<build>
...
<plugins>
  ...
  <plugin>
    <groupId>org.codehaus.plexus</groupId>
    <artifactId>plexus-component-metadata</artifactId>
    <version>1.7.1</version>
    <executions>
      <execution>
        <goals>
          <goal>generate-metadata</goal>
        </goals>
      </execution>
    </executions>
  </plugin>
  ...
</plugins>
...
</build>
{% endhighlight %}

3) 使用extension

上面步骤2)我们编写了一个名为beer-maven-lifecycle的extension，下面我们来看看如何使用。我们可以通过3种方法来使用：

* 将相应的extension添加到```${maven.home}/lib/ext```文件夹

* 将其添加到对应project的pom文件的build扩展中

* 配置到```.mvn/extensions.xml```文件中（Maven 3.3.1之后才行）

>注： 假如你使用build extension机制，上面的afterSessionStart()方法将不会被调用，因为通过这种方式扩展是在build之后被才被加载的

下面是采用build extension的方式使用我们的扩展，将其声明到POM文件中：
{% highlight string %}
<build>
 ...
 <extensions>
   ...
   <extension>
     <groupId>org.apache.maven.extensions</groupId>
     <artifactId>beer-maven-lifecycle</artifactId>
     <version>1.0-SNAPSHOT</version>
   </extension>
   ...
 </extensions>
...
</build>
{% endhighlight %}






<br />
<br />
**[参看]：**

1. [Centos7.3下部署Java开发环境](https://ivanzz1001.github.io/records/post/linux/2017/09/19/linux-java-install)

2. [maven官网](https://maven.apache.org/)

<br />
<br />
<br />

