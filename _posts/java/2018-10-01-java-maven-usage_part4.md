---
layout: post
title: maven使用手册
tags:
- java-language
categories: java-language
description: maven使用案例
---


本文通过一个更详细的例子来演示Maven的使用。


<!-- more -->

## 1. What is Maven?

初始见到Maven你会觉得有特别多的东西，但简而言之Maven为我们提供一个模式化工程构建基础设施，并以此便于我们理解和管理项目。Maven本质上是一个工程管理和理解的工具，提供了相应的方法来协助管理工程：

* Builds

* Documentation

* Reporting

* Dependencies

* SCMs

* Releases

* Distribution

Maven可以协助我们以一个标准的方式更高效的管理整个开发周期。

## 2. 创建第一个Maven工程

这里我们介绍如何创建第一个Maven工程。这里为了创建第一个Maven工程，我们会使用Maven的```archetype```机制。```archetype```可以看作是一个预先定义好的模型，所有同类型的工程都可以使用该模型来管理。如下我们演示一下archetype机制的使用，想要了解更多archetype内容请参看[Introduction to Archetypes](https://maven.apache.org/guides/introduction/introduction-to-archetypes.html).

这里作为第一个工程，我们创建一个最简单的Maven工程：
{% highlight string %}
# mkdir MavenSpace
# cd MavenSpace
# mvn -B archetype:generate -DgroupId=com.mycompany.app -DartifactId=my-app -DarchetypeArtifactId=maven-archetype-quickstart -DarchetypeVersion=1.4
[INFO] Scanning for projects...
[INFO] 
[INFO] ------------------< org.apache.maven:standalone-pom >-------------------
[INFO] Building Maven Stub Project (No POM) 1
[INFO] --------------------------------[ pom ]---------------------------------
[INFO] 
[INFO] >>> maven-archetype-plugin:3.2.0:generate (default-cli) > generate-sources @ standalone-pom >>>
[INFO] 
[INFO] <<< maven-archetype-plugin:3.2.0:generate (default-cli) < generate-sources @ standalone-pom <<<
[INFO] 
[INFO] 
[INFO] --- maven-archetype-plugin:3.2.0:generate (default-cli) @ standalone-pom ---
[INFO] Generating project in Batch mode
[INFO] ----------------------------------------------------------------------------
[INFO] Using following parameters for creating project from Archetype: maven-archetype-quickstart:1.4
[INFO] ----------------------------------------------------------------------------
[INFO] Parameter: groupId, Value: com.mycompany.app
[INFO] Parameter: artifactId, Value: my-app
[INFO] Parameter: version, Value: 1.0-SNAPSHOT
[INFO] Parameter: package, Value: com.mycompany.app
[INFO] Parameter: packageInPathFormat, Value: com/mycompany/app
[INFO] Parameter: version, Value: 1.0-SNAPSHOT
[INFO] Parameter: package, Value: com.mycompany.app
[INFO] Parameter: groupId, Value: com.mycompany.app
[INFO] Parameter: artifactId, Value: my-app
[INFO] Project created from Archetype in dir: /root/MavenSpace/my-app
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  5.938 s
[INFO] Finished at: 2021-09-20T01:36:53-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}

执行完上面的命令，你会看到在目录下会生成一个```my-app```文件夹，并且在该文件夹中会有一个pom.xml文件，看起来类似于如下：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
 
  <name>my-app</name>
  <!-- FIXME change it to the project's website -->
  <url>http://www.example.com</url>
 
  <properties>
    <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding>
    <maven.compiler.source>1.7</maven.compiler.source>
    <maven.compiler.target>1.7</maven.compiler.target>
  </properties>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
 
  <build>
    <pluginManagement><!-- lock down plugins versions to avoid using Maven defaults (may be moved to parent pom) -->
       ... lots of helpful plugins
    </pluginManagement>
  </build>
</project>
{% endhighlight %}

pom.xml包含有本项目的POM(Project Object Model)。在Maven中，POM是最基本的工作单元。简而言之，POM包含了工程相关的所有重要信息。

上面所生成的POM十分简单，但还是展示了一些POM的关键元素，下面我们简单介绍一下：

* project: Maven pom.xml的顶级元素

* modelVersion: 该元素用于指示对象模型的版本号。本字段很少改变，通常不建议开发人员对该字段做修改

* groupId: 该字段工程所属organization或group的唯一标识符。groupId是一个工程的关键标识符，典型的写法为使用公司域名。例如：org.apache.maven.plugins就是所有Maven plugins的groupId

* artifactId: 本字段表示工程所生成的primary artifact的基准名称。一个工程的primary artifact通常为一个```JAR```文件。而Secondary artifacts也会使用该```artifactId```作为其最终名称的一部分。Maven所产生的一个典型的artifact的形式为：```<artifactId>-<version>.<extension>```(例如： myapp-1.0.jar)

* version: 本字段用于指示工程所产生的artifact的版本号。Maven花费了很大的精力来帮助你进行版本管理，并且在版本中你经常会看到```SNAPSHOT```这样的设计，其表示该工程处于一个开发状态。在后面我们会继续讲解snapshots的用法

* name： 该字段表示工程的展示名称。通常被用于Maven所生成的一些工程文档当中

* url: 用于指示工程官方网站的位置。通常被用于Maven所生成的一些工程文档当中

* properties: 该字段其实是一些值占位符(value placeholders)，在POM的任何地方都可以访问这些值。

* dependencies: 该元素的子节点用于列出工程的依赖。这是POM的核心。

* build: 该元素主要是声明(declare)工程的目录结构，以及plugin的管理

现在我们来看一下执行完上面的命令之后所生成的文件的目录结构：
<pre>
# pwd
/root/MavenSpace
# ls
my-app

# tree
.
└── my-app
    ├── pom.xml
    └── src
        ├── main
        │   └── java
        │       └── com
        │           └── mycompany
        │               └── app
        │                   └── App.java
        └── test
            └── java
                └── com
                    └── mycompany
                        └── app
                            └── AppTest.java

12 directories, 3 files
</pre>
正如你所看到的，通过archetype所创建的工程有一个POM，一个应用程序源代码目录，还有一个测试源代码目录。这是Maven工程标准的目录结构(应用程序源代码放在```${basedir}/src/main/java```目录下，测试源代码放在```${basedir}/src/test/java```目录下，```${basedir}```表示含有pom.xml文件的目录）。

假如你想要手工创建一个Maven工程的话，我们建议你使用上面的目录结构。这是Maven的一个常用准则，如果想要了解更多Maven标准布局，请参看[Introduction to the Standard Directory Layout](Introduction to the Standard Directory Layout)。


## 3. 如何编译应用程序？

进入到```archetype:generate```所生成的含有```pom.xml```文件的文件夹下，然后执行如下命令来编译应用程序源代码：
{% highlight string %}
# mvn compile
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/main/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:compile (default-compile) @ my-app ---
[INFO] Changes detected - recompiling the module!
[INFO] Compiling 1 source file to /root/MavenSpace/my-app/target/classes
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  1.768 s
[INFO] Finished at: 2021-09-20T01:51:57-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %} 
当我们第一次执行该命令（或其他命令）时，Maven需要下载所有的plugins和相关的依赖。对于一个全新安装的Maven，下载这些plugins和依赖可能会花费一定的时间。当你再一次执行该命令时，由于本地已经缓存了这些plugins和依赖，则不需要重新下载，因此编译过程就会十分快捷。

从编译的输出我们可以看到，被编译的类会放在```${basedir}/target/classes```目录下，这也是Maven一个基本准则。因此，假如你是一个热切的旁观者，你将会注意到这些准则。上述生成的POM很小，你并不需要显式的告诉Maven应用程序源代码在哪，也不需要显式的指明输出存放在哪。通过遵循Maven的标准准则，我们不需要费太多劲就能够完成很多东西。


## 4. 如何编译测试源代码及运行单元测试？

上面我们已经成功的编译了应用程序源代码，现在我们打算编译单元测试源代码然后再进行一些测试：
{% highlight string %}
# mvn test
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/main/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:compile (default-compile) @ my-app ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:testResources (default-testResources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/test/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:testCompile (default-testCompile) @ my-app ---
[INFO] Changes detected - recompiling the module!
[INFO] Compiling 1 source file to /root/MavenSpace/my-app/target/test-classes
[INFO] 
[INFO] --- maven-surefire-plugin:2.22.1:test (default-test) @ my-app ---
[INFO] 
[INFO] -------------------------------------------------------
[INFO]  T E S T S
[INFO] -------------------------------------------------------
[INFO] Running com.mycompany.app.AppTest
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.088 s - in com.mycompany.app.AppTest
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  3.124 s
[INFO] Finished at: 2021-09-20T02:05:02-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %} 
通过上面的输出，我们注意到：

* 本次Maven会下载更多的依赖。这些依赖和插件对于执行单元测试来说是必需的（对于编译时所需的依赖已经下载过了，不必重新下载）

* 在编译和执行单元测试之前，Maven会编译主代码（注：这里因为我们并没有对应用程序源代码做任何修改，因此不必再重新编译）

假如你仅仅只想编译单元测试源代码，而并不想执行这些单元测试，你可以执行如下命令：
<pre>
# mvn test-compile
</pre> 

## 5. 如何创建一个Jar包并将其安装到本地仓库？

要生成一个Jar包的话，直接执行如下命令即可：
{% highlight string %}
# mvn package
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/main/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:compile (default-compile) @ my-app ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:testResources (default-testResources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/test/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:testCompile (default-testCompile) @ my-app ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- maven-surefire-plugin:2.22.1:test (default-test) @ my-app ---
[INFO] 
[INFO] -------------------------------------------------------
[INFO]  T E S T S
[INFO] -------------------------------------------------------
[INFO] Running com.mycompany.app.AppTest
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.114 s - in com.mycompany.app.AppTest
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] 
[INFO] --- maven-jar-plugin:3.0.2:jar (default-jar) @ my-app ---
[INFO] Building jar: /root/MavenSpace/my-app/target/my-app-1.0-SNAPSHOT.jar
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  2.976 s
[INFO] Finished at: 2021-09-20T02:37:33-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}
现在我们来看一下```${basedir}/target```目录，我们会发现已经生成了Jar包：
<pre>
# ls target/
classes  generated-sources  generated-test-sources  maven-archiver  maven-status  my-app-1.0-SNAPSHOT.jar  surefire-reports  test-classes
</pre>

现在我们将所生成的artifact(JAR文件）安装到本地仓库（默认位置为：```${user.home}/.m2/repository```)，执行如下命令：
{% highlight string %}
# mvn install
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.5.2/maven-install-plugin-2.5.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.5.2/maven-install-plugin-2.5.2.jar (33 kB at 22 kB/s)
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/main/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:compile (default-compile) @ my-app ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:testResources (default-testResources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/MavenSpace/my-app/src/test/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:testCompile (default-testCompile) @ my-app ---
[INFO] Nothing to compile - all classes are up to date
[INFO] 
[INFO] --- maven-surefire-plugin:2.22.1:test (default-test) @ my-app ---
[INFO] 
[INFO] -------------------------------------------------------
[INFO]  T E S T S
[INFO] -------------------------------------------------------
[INFO] Running com.mycompany.app.AppTest
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.108 s - in com.mycompany.app.AppTest
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] 
[INFO] --- maven-jar-plugin:3.0.2:jar (default-jar) @ my-app ---
[INFO] 
[INFO] --- maven-install-plugin:2.5.2:install (default-install) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.4/maven-shared-utils-0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.4/maven-shared-utils-0.4.pom (4.0 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.15/plexus-utils-3.0.15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.15/plexus-utils-3.0.15.pom (3.1 kB at 7.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.4/maven-shared-utils-0.4.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.15/plexus-utils-3.0.15.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.4/maven-shared-utils-0.4.jar (155 kB at 194 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.15/plexus-utils-3.0.15.jar (239 kB at 161 kB/s)
[INFO] Installing /root/MavenSpace/my-app/target/my-app-1.0-SNAPSHOT.jar to /root/.m2/repository/com/mycompany/app/my-app/1.0-SNAPSHOT/my-app-1.0-SNAPSHOT.jar
[INFO] Installing /root/MavenSpace/my-app/pom.xml to /root/.m2/repository/com/mycompany/app/my-app/1.0-SNAPSHOT/my-app-1.0-SNAPSHOT.pom
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  6.958 s
[INFO] Finished at: 2021-09-20T02:42:05-07:00
[INFO] ------------------------------------------------------------------------

# tree ~/.m2/repository/com/mycompany/
/root/.m2/repository/com/mycompany/
└── app
    └── my-app
        ├── 1.0-SNAPSHOT
        │   ├── maven-metadata-local.xml
        │   ├── my-app-1.0-SNAPSHOT.jar
        │   ├── my-app-1.0-SNAPSHOT.pom
        │   └── _remote.repositories
        └── maven-metadata-local.xml

3 directories, 5 files

{% endhighlight %}
在上面的输出结果中，surefile plugin(用于执行test)会查找特定命名规范文件中的test。默认情况下这些Test包括：

* ```**/*Test.java```

* ```**/Test*.java```

* ```**/*TestCase.java```

但默认情况下不包括：

* ```**/Abstract*Test.java```

* ```**/Abstract*TestCase.java```

经过前面，我们已经一步一步走过了Maven工程的setting up、building、testing、packaging、和installing流程。这是一个Maven工程所会执行的主要流程。

接下来简单介绍一下如何通过Maven产生site信息：
{% highlight string %}
# mvn site
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-site-plugin:3.7.1:site (default-site) @ my-app ---
[INFO] configuring report plugin org.apache.maven.plugins:maven-project-info-reports-plugin:3.0.0
[INFO] 15 reports detected for maven-project-info-reports-plugin:3.0.0: ci-management, dependencies, dependency-info, dependency-management, distribution-management, index, issue-management, licenses, mailing-lists, modules, plugin-management, plugins, scm, summary, team
[INFO] Rendering site with default locale English (en)
[INFO] Relativizing decoration links with respect to localized project URL: http://www.example.com
[INFO] Rendering content with org.apache.maven.skins:maven-default-skin:jar:1.2 skin.
[INFO] Generating "Dependencies" report  --- maven-project-info-reports-plugin:3.0.0:dependencies
[INFO] Generating "Dependency Information" report --- maven-project-info-reports-plugin:3.0.0:dependency-info
[INFO] Generating "About" report         --- maven-project-info-reports-plugin:3.0.0:index
[INFO] Generating "Plugin Management" report --- maven-project-info-reports-plugin:3.0.0:plugin-management
[INFO] Generating "Plugins" report       --- maven-project-info-reports-plugin:3.0.0:plugins
[INFO] Generating "Summary" report       --- maven-project-info-reports-plugin:3.0.0:summary
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  3.906 s
[INFO] Finished at: 2021-09-20T02:56:42-07:00
{% endhighlight %}


另外，还有一些其他的单独的goals可以执行，例如：
{% highlight string %}
# mvn clean
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/3.1.0/maven-clean-plugin-3.1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/3.1.0/maven-clean-plugin-3.1.0.jar (30 kB at 19 kB/s)
[INFO] 
[INFO] --- maven-clean-plugin:3.1.0:clean (default-clean) @ my-app ---
[INFO] Deleting /root/MavenSpace/my-app/target
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  2.489 s
[INFO] Finished at: 2021-09-20T02:57:59-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}
执行上述命令会把构建所生成的```target```目录下的文件全部删除。注意，并不会删除本地仓库中的内容。



## 6. What is a SNAPSHOT version?

在pom.xml文件中，我们注意到了version元素，其含有一个```-SNAPSHOT```后缀：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0"
  ...
  <groupId>...</groupId>
  <artifactId>my-app</artifactId>
  ...
  <version>1.0-SNAPSHOT</version>
  <name>Maven Quick Start Archetype</name>
  ...
{% endhighlight %}
```SNAPSHOT```指的是开发分支上最新的源代码，并不能保证源代码的稳定，相应的代码可能随时会修改。相反，release版本（任何不是以```SNAPSHOT```结尾的的版本）的源代码将不会再进行修改。

换句话说，```SNAPSHOT```版本是最终release版本之前的开发版本。

## 7. 如何使用plugins?

假如你想要定制Maven工程的build过程，你可以通过添加或重新配置plugin来实现。

在如下的例子中，我们会重新配置Java编译器以支持JDK5.0版本源代码，最简单的方式就是添加如下配置到POM中：
{% highlight string %}
...
<build>
  <plugins>
    <plugin>
      <groupId>org.apache.maven.plugins</groupId>
      <artifactId>maven-compiler-plugin</artifactId>
      <version>3.3</version>
      <configuration>
        <source>1.5</source>
        <target>1.5</target>
      </configuration>
    </plugin>
  </plugins>
</build>
...
{% endhighlight %}
通过上面我们注意到，Maven中的所有plugin看起来都类似于一个dependency，事实上在某些方面它们确实类似。plugin会自动的下载然后被使用（默认是下载最新版本，假如指定了版本号的话，则会下载指定版本的plugin)

configuration元素所指定的参数会被应用到maven-compiler-plugin的每一个goal中。在上面的例子中，compiler plugin已经作为编译进程的一部分，因此相应的配置参数会影响到编译过程。此外，我们也可以添加新的goals到build lifecycle流程中。

如果我们想知道一个plugin支持哪些配置的话，我们可以查询[Plugin List](https://maven.apache.org/plugins/)，然后找到你正在使用的plugin及所对应的goal。就通常来说，我们可以参阅[Guide to Configuring Plugins](https://maven.apache.org/guides/mini/guide-configuring-plugins.html)来了解如何配置plugin的可用参数。


## 8. 如何添加resource到Jar包中？
另一个常见的用例是：在不修改POM的情况下将资源文件打包进Jar包。针对这一常见功能，Maven的实现依然是依赖其标准目录结构（Standard Directory Layout)，这意味着通过遵循Maven标准准则，Maven就会自动的将相应目录中的资源打包进JAR中。

在下面的例子中，我们添加了一个```${basedir}/src/main/resources```目录，然后在其中放入我们想要打包进JAR包中的资源文件。Maven的准则也很简单：任何放入```${basedir}/src/main/resources```文件夹中的目录或文件都会被原模原样的打包进JAR。
{% highlight string %}
my-app
|-- pom.xml
`-- src
    |-- main
    |   |-- java
    |   |   `-- com
    |   |       `-- mycompany
    |   |           `-- app
    |   |               `-- App.java
    |   `-- resources
    |       `-- META-INF
    |           `-- application.properties
    `-- test
        `-- java
            `-- com
                `-- mycompany
                    `-- app
                        `-- AppTest.java
{% endhighlight %}
在我们的例子中，我们看到有一个```META-INF```目录，然后在该目录中有一个```application.properties```文件。假如你解压Maven所打包的JAR文件的话，你将会看到如下：
{% highlight string %}
|-- META-INF
|   |-- MANIFEST.MF
|   |-- application.properties
|   `-- maven
|       `-- com.mycompany.app
|           `-- my-app
|               |-- pom.properties
|               `-- pom.xml
`-- com
    `-- mycompany
        `-- app
            `-- App.class
{% endhighlight %}

如你所看到的，```${basedir}/src/main/resources```中的可以在JAR包的根目录中找到，并且```application.properties```文件就在```META-INF```目录中。同时，我们也看到有一些其他文件，比如```META-INF/MANIFEST.MF```和pom.xml、pom.properties。这是Maven所产生的Jar包中的一些标准文件。我们也可以选择创建自己的MANIFEST，但假如不自己创建的话，Maven默认也会帮我们生成一个。另外，pom.xml与pom.properties也会被打包进JAR中，这样使得每一个由Maven生成的artifact都具有自描述能力，并且假如有需要的话在应用程序中你也可以充分利用这些元数据。其中一个简单的使用场景是获取应用程序的版本号。对POM文件的操作可能需要一些Maven工具的支持，但是对于pom.properties文件我们可以直接使用标准JAVA API就可以读取：
{% highlight string %}
#Generated by Maven
#Tue Oct 04 15:43:21 GMT-05:00 2005
version=1.0-SNAPSHOT
groupId=com.mycompany.app
artifactId=my-app
{% endhighlight %}


如果要将resources添加到单元测试的classpath中，可以遵循将resource添加到JAR包中类似的方式，不过此时我们需要将resource放到```${basedir}/src/test/resources```目录。此时我们的工程目录结构类似于如下：
{% highlight string %}
my-app
|-- pom.xml
`-- src
    |-- main
    |   |-- java
    |   |   `-- com
    |   |       `-- mycompany
    |   |           `-- app
    |   |               `-- App.java
    |   `-- resources
    |       `-- META-INF
    |           |-- application.properties
    `-- test
        |-- java
        |   `-- com
        |       `-- mycompany
        |           `-- app
        |               `-- AppTest.java
        `-- resources
            `-- test.properties
{% endhighlight %}
在单元测试中，我们可以简单的使用如下的方式来访问资源：
{% highlight string %}
...
 
// Retrieve resource
InputStream is = getClass().getResourceAsStream( "/test.properties" );
 
// Do something with the resource
 
...
{% endhighlight %}


## 9. 如何过滤资源文件？

有时候，资源文件需要包含一个只用于构建(build)时期使用的值。在Maven中要想实现此目的，可以通过```${<property name>}```方式将相应的值以引用的方式添加进资源文件中。其中property可以是pom.xml文件中定义的某个值，也可以是用户的settings.xml中定义的某个值，还可以是外部properties文件中定义的某个值，或者是系统的某个propertity值。

为了使Maven能够在拷贝时过滤相应的资源，可以简单的在pom.xml将相应的资源目录设置为```true```:
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
 
  <build>
    <resources>
      <resource>
        <directory>src/main/resources</directory>
        <filtering>true</filtering>
      </resource>
    </resources>
  </build>
</project>
{% endhighlight %}

上面我们注意到，我们新添加了```build```、```resources```、```resource```元素。另外，我们必须明确的指出相应的资源所存放的目录```src/main/resources```。所有的这些信息其实在前面都已经默认提供了，只是```filtering```的默认值为false，因此这里我们必须将此添加到pom.xml文件中以覆盖默认值，并且将filtering设置为true。

如果要引用pom.xml中定义的某个值，可以使用其在XML中的元素名来实现。例如，```${project.name}```则可以访问到工程名，```${project.version}```则可以访问到工程的版本，```${project.build.finalName}```可以访问到工程构建后打包后的最终文件名。值得注意的是，POM中的有些元素具有默认值，这些元素可能并不需要定义在```pom.xml```文件中，但是我们仍然可以访问。类似的，定义在用户settings.xml中的值也可以使用以```settings```开头的属性名来访问（例如：```${settings.localRepository}```可以访问到用户的本地仓库)。

这里继续我们的示例，我们添加一对属性到```application.properties```文件（其放置于src/main/resources目录下）中，当对应的resource被filter时，相应的属性值将会被提供：
<pre>
# tree
.
├── pom.xml
└── src
    ├── main
    │   ├── java
    │   │   └── com
    │   │       └── mycompany
    │   │           └── app
    │   │               └── App.java
    │   └── resources
    │       └── application.properties
    └── test
        └── java
            └── com
                └── mycompany
                    └── app
                        └── AppTest.java

12 directories, 4 files

# cat src/main/resources/application.properties
# application.properties
application.name=${project.name}
application.version=${project.version}
</pre>

如此设置之后，我们可以执行下面的命令（注：process-resources是build lifecycle中的一个phase，在该phase中相应的资源会被copy和filter)。
{% highlight string %}
# mvn process-resources
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] Copying 1 resource
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  0.888 s
[INFO] Finished at: 2021-09-20T19:58:44-07:00
[INFO] ------------------------------------------------------------------------

{% endhighlight %}

执行完成上述命令之后，可以在```target/classes```目录下找到```application.properties```文件，其看起来类似如下：
{% highlight string %}
# tree
.
├── pom.xml
├── src
│   ├── main
│   │   ├── java
│   │   │   └── com
│   │   │       └── mycompany
│   │   │           └── app
│   │   │               └── App.java
│   │   └── resources
│   │       └── application.properties
│   └── test
│       └── java
│           └── com
│               └── mycompany
│                   └── app
│                       └── AppTest.java
└── target
    └── classes
        └── application.properties

14 directories, 5 files

# cat target/classes/application.properties
# application.properties
application.name=my-app
application.version=1.0-SNAPSHOT
{% endhighlight %}

可以看到相应的值已经被替换了（注：filter默认为false，此时相应的值将不会被替换）。


###### 过滤外部资源文件
如果要引用一个外部文件的属性，我们需要做的就是将该外部文件添加到pom.xml中。下面我们首先创建一个外部资源文件，将其命名为```src/main/filters/filter.properties```:
{% highlight string %}
# filter.properties
my.filter.value=hello!
{% endhighlight %}

接着我们将其添加到```pom.xml```中：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
 
  <build>
    <filters>
      <filter>src/main/filters/filter.properties</filter>
    </filters>
    <resources>
      <resource>
        <directory>src/main/resources</directory>
        <filtering>true</filtering>
      </resource>
    </resources>
  </build>
</project>
{% endhighlight %}

然后，我们在```applications.properties```中引用该属性：
{% highlight string %}
# application.properties
application.name=${project.name}
application.version=${project.version}
message=${my.filter.value}
{% endhighlight %}

此时，我们再执行```mvn process-resources```命令：
{% highlight string %}
# mvn process-resources
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] Copying 1 resource
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  0.653 s
[INFO] Finished at: 2021-09-20T20:39:11-07:00
[INFO] ------------------------------------------------------------------------

# tree
.
├── pom.xml
├── src
│   ├── main
│   │   ├── filters
│   │   │   └── filter.properties
│   │   ├── java
│   │   │   └── com
│   │   │       └── mycompany
│   │   │           └── app
│   │   │               └── App.java
│   │   └── resources
│   │       └── application.properties
│   └── test
│       └── java
│           └── com
│               └── mycompany
│                   └── app
│                       └── AppTest.java
└── target
    └── classes
        └── application.properties

15 directories, 6 files

# cat target/classes/application.properties 
# application.properties
application.name=my-app
application.version=1.0-SNAPSHOT
message=hello!
{% endhighlight %}

我们看到新的值已经被写入到application.properties中了。除了将```my.filter.value```写到一个外部文件外，我们也可以将其定义在pom.xml文件的```properties```段，你将会获得一样的效果(注：此时就不需要再引用src/main/filters/filter.properties文件了）：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
 
  <build>
    <resources>
      <resource>
        <directory>src/main/resources</directory>
        <filtering>true</filtering>
      </resource>
    </resources>
  </build>
 
  <properties>
    <my.filter.value>hello</my.filter.value>
  </properties>
</project>
{% endhighlight %}


###### 获取系统properties
此外，也可以在resources中获取系统properties，不管这些properties是来源于Java(如： java.version或者user.home)还是通过标准的命令行形式传递的properties(```-D```传递参数)。例如，我们可以通过如下方式来改变```application.properties```:
{% highlight string %}
# application.properties
java.version=${java.version}
command.line.prop=${command.line.prop}
{% endhighlight %}

现在，我们执行如下命令（注：```command.line.prop```是通过命令行传递），```application.properties```将会包含系统属性值：
{% highlight string %}
# mvn process-resources "-Dcommand.line.prop=hello again"
{% endhighlight %}

## 10. 如何使用外部dependencies?
你可能已经注意到，在我们上面的例子中对应的POM有一个```dependencies```元素。实际上，我们一直在使用外部依赖，现在我们来较为详细的介绍一下其工作流程。欲了解更多信息，请参看[Introduction to Dependency Mechanism](https://maven.apache.org/guides/introduction/introduction-to-dependency-mechanism.html).

在pom.xml文件中的```dependencies```段列出了工程在构建时(compile time、test time、run time等)所需要的所有外部依赖。到目前为止，我们的工程只依赖```JUnit```:
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
</project>
{% endhighlight %}
对于每一个外部依赖，我们都至少需要定义四项内容：groupId、artifactId、version、scope。其中```groupId```、```artifactId```和```version```就是该依赖该构建时所指定的。而```scope```元素用于表示我们的工程如何使用该依赖，其值可以为：```compile```、```test```或者```runtime```。要了解更多关于dependency的内容，请参看[Project Descriptor Reference](https://maven.apache.org/ref/current/maven-model/maven.html)


一个dependency拥有这些信息之后，Maven在构建工程时就可以引用这些dependency。那么Maven可以从什么地方引用这些dependency呢？Maven首先会从本地仓库（默认位置为```${user.home}/.m2/repository```)来查找所有依赖。在上一节中，我们将我们工程的artifact（my-app-1.0-SNAPSHOT)安装到了本地仓库。一旦安装到了那里，其他工程就可以将其作为一个dependency来引用它，如下所示：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-other-app</artifactId>
  ...
  <dependencies>
    ...
    <dependency>
      <groupId>com.mycompany.app</groupId>
      <artifactId>my-app</artifactId>
      <version>1.0-SNAPSHOT</version>
      <scope>compile</scope>
    </dependency>
  </dependencies>
</project>
{% endhighlight %}

但假如在其他地方构建的dependencies呢？它们是如何下载到本地仓库的呢？当一个工程引用到一个dependency，但是其并不在本地仓库时，Maven将会从一个远程仓库将其下载到本地。你可能会注意到，当你第一次构建工程时，Maven会下载很多东西（注：构建工程时相应plugin的dependencies)。默认情况下，默认情况Maven可以从远程仓库[ https://repo.maven.apache.org/maven2/]( https://repo.maven.apache.org/maven2/)中找到。我们也可以建立自己的远程仓库（可能是公司内部的中心仓库）以替换默认的远程仓库。要了解更多关于repository的信息，请参看[Introduction to Repositories](https://maven.apache.org/guides/introduction/introduction-to-repositories.html)。

接下来我们添加另外一个dependency到工程中。假如我们添加了一些日志打印到代码中，我们可能需要```log4j```这样一个dependency。首先我们需要知道log4j的groupId、artifactId以及version。我们可以在Maven中央仓库的[/maven2/log4j/log4j](https://repo.maven.apache.org/maven2/log4j/log4j/)找到。在该目录下，有一个名为```maven-metadata.xml```文件。如下是log4j下该文件的大概内容：
{% highlight string %}
<metadata>
  <groupId>log4j</groupId>
  <artifactId>log4j</artifactId>
  <version>1.1.3</version>
  <versioning>
    <versions>
      <version>1.1.3</version>
      <version>1.2.4</version>
      <version>1.2.5</version>
      <version>1.2.6</version>
      <version>1.2.7</version>
      <version>1.2.8</version>
      <version>1.2.11</version>
      <version>1.2.9</version>
      <version>1.2.12</version>
    </versions>
  </versioning>
</metadata>
{% endhighlight %}
从上面我们看到，对应的groupId为```log4j```，artfactId为```log4j```。接下来我们可以选择不同的版本，这里我们可以选择最新版本1.2.12(有一些maven-metadata.xml也会指出哪一个版本是当前的发布版本，参看[repository metadata reference](https://maven.apache.org/ref/current/maven-repository-metadata/repository-metadata.html))。另外，与maven-metadata.xml一起，我们可以看到不同版本的log4j库的目录。进入这些目录可以看到有：

* 实际的Jar文件（例如：log4j-1.2.12.jar)

* 相应的POM文件（pom.xml用于指出log4j相应的依赖）

* 另一个maven-metadata.xml文件

* md5文件

通过这些我们就可以直接定位到相应的library。

了解上述内容后，我们就可以将该依赖添加到我们的工程pom.xml中了：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
    <dependency>
      <groupId>log4j</groupId>
      <artifactId>log4j</artifactId>
      <version>1.2.12</version>
      <scope>compile</scope>
    </dependency>
  </dependencies>
</project>
{% endhighlight %}
接下来我们就可以使用```mvn compile```命令来编译工程，此时Maven就会自动为我们下载log4j依赖。

## 11. 如何部署JAR包到远程仓库？

如果要部署jar包到外部仓库，你必须要在pom.xml中配置远程仓库的地址，并且要在settings.xml中配置该远程仓库的授权信息。

如下是使用SCP和username/password授权的一个例子：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>jar</packaging>
 
  <name>Maven Quick Start Archetype</name>
  <url>http://maven.apache.org</url>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
    <dependency>
      <groupId>org.apache.codehaus.plexus</groupId>
      <artifactId>plexus-utils</artifactId>
      <version>1.0.4</version>
    </dependency>
  </dependencies>
 
  <build>
    <filters>
      <filter>src/main/filters/filters.properties</filter>
    </filters>
    <resources>
      <resource>
        <directory>src/main/resources</directory>
        <filtering>true</filtering>
      </resource>
    </resources>
  </build>
  <!--
   |
   |
   |
   -->
  <distributionManagement>
    <repository>
      <id>mycompany-repository</id>
      <name>MyCompany Repository</name>
      <url>scp://repository.mycompany.com/repository/maven2</url>
    </repository>
  </distributionManagement>
</project>
{% endhighlight %}

{% highlight string %}
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  ...
  <servers>
    <server>
      <id>mycompany-repository</id>
      <username>jvanzyl</username>
      <!-- Default value is ~/.ssh/id_dsa -->
      <privateKey>/path/to/identity</privateKey> (default is ~/.ssh/id_dsa)
      <passphrase>my_key_passphrase</passphrase>
    </server>
  </servers>
  ...
</settings>
{% endhighlight %}
需要注意的是，假如你连接到的是一个openssh服务器，并且该服务器在```sshd_config```配置文件中将```PasswordAuthentication```设置为了```no```，则每一次进行username/password权限验证时，都需要重新输入密码。此种情况下，你可能需要使用public key的方式来进行鉴权。

假如你将密码设置在```settings.xml```文件时，需要注意密码泄露风险。请参看[ Password Encryption](https://maven.apache.org/guides/mini/guide-encryption.html)。

## 12. 如何创建documentation?

我们可以使用archetype机制来为已存在的project创建documentation，执行如下命令：
{% highlight string %}
# mvn archetype:generate \
  -DarchetypeGroupId=org.apache.maven.archetypes \
  -DarchetypeArtifactId=maven-archetype-site \
  -DgroupId=com.mycompany.app \
  -DartifactId=my-app-site
{% endhighlight %}

参看[Guide to creating a site](https://maven.apache.org/guides/mini/guide-site.html)以了解更多关于documentation的信息。

## 13. 如何创建其他类型的projects?

这里需要注意的是，Maven lifecycle适用于任何类型的project。例如，我们可以通过如下的命令来创建一个Web应用程序：
{% highlight string %}
# mvn archetype:generate \
    -DarchetypeGroupId=org.apache.maven.archetypes \
    -DarchetypeArtifactId=maven-archetype-webapp \
    -DgroupId=com.mycompany.app \
    -DartifactId=my-webapp
{% endhighlight %}
执行后，会创建一个```my-webapp```目录，包含如下工程描述：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>my-webapp</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>war</packaging>
 
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.11</version>
      <scope>test</scope>
    </dependency>
  </dependencies>
 
  <build>
    <finalName>my-webapp</finalName>
  </build>
</project>
{% endhighlight %}

注意上面的```packaging```元素，其告诉Maven构建输出为```WAR```包。进入webapp目录，然后执行如下命令：
<pre>
# mvn package
</pre>

此时，我们会看到```target/my-webapp.war```被构建，并能看到构建的步骤。

## 14. 如何一次构建多个project?

Maven在设计时也是考虑到了如何一次构建多个module。在本节，我们会演示如何一次构建上述的```WAR```以及```JAR```包。

首先我们需要添加一个parent ```pom.xml```文件到这两个工程的父目录中：
{% highlight string %}
# tree 
+- pom.xml
+- my-app
| +- pom.xml
| +- src
|   +- main
|     +- java
+- my-webapp
| +- pom.xml
| +- src
|   +- main
|     +- webapp
{% endhighlight %}

然后在该parent pom.xml文件中添加如下内容：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>com.mycompany.app</groupId>
  <artifactId>app</artifactId>
  <version>1.0-SNAPSHOT</version>
  <packaging>pom</packaging>
 
  <modules>
    <module>my-app</module>
    <module>my-webapp</module>
  </modules>
</project>
{% endhighlight %}

对于webapp工程，我们需要依赖于my-app所生成的```JAR```包，因此将如下添加到```my-webapp/pom.xml```:
{% highlight string %}
  ...
  <dependencies>
    <dependency>
      <groupId>com.mycompany.app</groupId>
      <artifactId>my-app</artifactId>
      <version>1.0-SNAPSHOT</version>
    </dependency>
    ...
  </dependencies>
{% endhighlight %}

之后，将如下```<parent>```部分的内容添加到其他两个子目录中的pom.xml文件（my-app以及my-webapp目录下的pom.xml)中：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <parent>
    <groupId>com.mycompany.app</groupId>
    <artifactId>app</artifactId>
    <version>1.0-SNAPSHOT</version>
  </parent>
  ...
{% endhighlight %}


然后，在顶级目录执行如下命令：
<pre>
# mvn verify
</pre>

此时，```my-webapp/target/my-webapp.war```就会被创建，并且对应的```JAR```也包含在其中：
{% highlight string %}
$ jar tvf my-webapp/target/my-webapp-1.0-SNAPSHOT.war
   0 Fri Jun 24 10:59:56 EST 2005 META-INF/
 222 Fri Jun 24 10:59:54 EST 2005 META-INF/MANIFEST.MF
   0 Fri Jun 24 10:59:56 EST 2005 META-INF/maven/
   0 Fri Jun 24 10:59:56 EST 2005 META-INF/maven/com.mycompany.app/
   0 Fri Jun 24 10:59:56 EST 2005 META-INF/maven/com.mycompany.app/my-webapp/
3239 Fri Jun 24 10:59:56 EST 2005 META-INF/maven/com.mycompany.app/my-webapp/pom.xml
   0 Fri Jun 24 10:59:56 EST 2005 WEB-INF/
 215 Fri Jun 24 10:59:56 EST 2005 WEB-INF/web.xml
 123 Fri Jun 24 10:59:56 EST 2005 META-INF/maven/com.mycompany.app/my-webapp/pom.properties
  52 Fri Jun 24 10:59:56 EST 2005 index.jsp
   0 Fri Jun 24 10:59:56 EST 2005 WEB-INF/lib/
2713 Fri Jun 24 10:59:56 EST 2005 WEB-INF/lib/my-app-1.0-SNAPSHOT.jar
{% endhighlight %}
我们来看一下这是如何工作的呢？ 

1） 首先parent POM会被创建(称为```app```)，其```packaging```被设置为了```pom```，并且其定义了两个```modules```。这就告诉了Maven，需要在这些projects上运行所有的操作，而不仅仅只是在本工程上运行（我们可以通过在命令行添加```--non-recursive```来改变此行为）。

2） 接下来告诉```WAR```其需要```my-app```这个```JAR```。这会做如下一些事情： 

* 将jar添加到WAR中相关代码的classpath中；

* JAR永远会在WAR之前被构建；

* 通知WAR插件在构建时将JAR包含在其lib目录中

这里，你可能会注意到```junit-4.11.jar```这样一个dependency并没有出现在WAR包中。原因是```junit-4.11.jar```这样一个dependency其scope为```test```，这就说明junit-4.11.jar只是在testing时有需要，因此其可以不用包含在web应用程序当中。

3）最后一步就是包含parent definition。这可以确保即使该web application工程被单独发布时，也可以在仓库中找到其parent POM。











<br />
<br />
**[参看]：**

1. [maven官网](https://maven.apache.org/)

2. [Maven in 5 Minutes](https://maven.apache.org/guides/getting-started/maven-in-five-minutes.html)

3. [Maven Getting Started Guide](https://maven.apache.org/guides/getting-started/index.html)

<br />
<br />
<br />

