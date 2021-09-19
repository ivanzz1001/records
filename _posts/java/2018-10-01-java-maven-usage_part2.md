---
layout: post
title: maven的使用
tags:
- java-language
categories: java-language
description: maven的使用
---

本章我们首先介绍以下POM文件，然后再讲述Maven的使用。

<!-- more -->


## 1 POM介绍

### 1.1 什么是POM?

POM是工程对象模型(Project Object Model)的缩写。其是一个Maven工程的XML表示形式，该XML的名称为pom.xml。通常一个project包含配置文件、所涉及到的开发者以及他们所扮演的角色，跟踪系统，组织及license文件，project依赖等。事实上，在Maven中，一个project可以不包含任何源代码，而仅仅含有一个pom.xml文件。

### 1.2 Quick Overview
如下是一个POM工程文件所含有的元素列表。这里注意```modelVersion```为```4.0.0```。这是当前唯一支持的POM版本，必须要拥有该字段。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <!-- The Basics -->
  <groupId>...</groupId>
  <artifactId>...</artifactId>
  <version>...</version>
  <packaging>...</packaging>
  <dependencies>...</dependencies>
  <parent>...</parent>
  <dependencyManagement>...</dependencyManagement>
  <modules>...</modules>
  <properties>...</properties>
 
  <!-- Build Settings -->
  <build>...</build>
  <reporting>...</reporting>
 
  <!-- More Project Information -->
  <name>...</name>
  <description>...</description>
  <url>...</url>
  <inceptionYear>...</inceptionYear>
  <licenses>...</licenses>
  <organization>...</organization>
  <developers>...</developers>
  <contributors>...</contributors>
 
  <!-- Environment Settings -->
  <issueManagement>...</issueManagement>
  <ciManagement>...</ciManagement>
  <mailingLists>...</mailingLists>
  <scm>...</scm>
  <prerequisites>...</prerequisites>
  <repositories>...</repositories>
  <pluginRepositories>...</pluginRepositories>
  <distributionManagement>...</distributionManagement>
  <profiles>...</profiles>
</project>
{% endhighlight %}

从上面我们看到，基本可以分为如下几个部分：

* The Basics

* Build Settings

* More Project Information

* Environment Settings

下面我们就对这些部分分别进行讲解。

## 2 The Basics

POM包含了一个工程所需要的所有必要信息，这也包括在构建过程中所需要的一些配置plugin。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>org.codehaus.mojo</groupId>
  <artifactId>my-project</artifactId>
  <version>1.0</version>
</project>
{% endhighlight %}

## 2.1 Maven Coordinates
上面所定义的POM是Maven所允许的最简形式。```groupId:artifactId:version```这3个字段是必须的(尽管groupId和version这两个字段不一定要显式的定义，其可以从parent处继承，关于```继承```我们后面会讲述到）。这3个字段有点类似于地址和时间戳，其标识了仓库中的一个特定位置，就像是Maven工程的一个坐标系统一样：

* groupId: 在一个组织(organization)或工程中，其通常是唯一的。例如，所有核心的Maven artifacts都处于```org.apache.maven```这个groupId之下。值得指出的是，groupId并不一定需要使用"."来分割。例如，junit就不是使用"."来进行分割的。此外，即使是"."分割的groupId也并不一定要与该工程中包的结构一致。但通常的做法，我们会起名为一致。当该project存放与仓库中的时候，group扮演着一个类似于操作系统中Java包的功能。在上面的例子中，```org.codehaus.mojo```会存放于仓库中的```$M2_REPO/org/codehaus/mojo```目录下。

* artifactId: 本字段通常是所对应工程的名称。尽管groupId十分重要，但是对于一个group内部的人员来说，则很少提及groupId(他们都属于同一个groupId，比如 MojoHaus的groupId为: org.codehaus.mojo)。其与groupId一道，创建了一个唯一标识符以区分其他的project。同时，还与groupId一起确定了对应project在仓库中的位置。比如对于上面的例子来说，my-project粗放与```$M2_REPO/org/codehaus/mojo/my-project```目录下。


* version: 这是整个project名称的最后一个部分。```groupId:artifactId```唯一确定了一个project，但是无法确定到底是该项目的哪一个版本。在上面的例子中，```my-project``` 1.0版本的文件存放于```$M2_REPO/org/codehaus/mojo/my-project/1.0```目录下。

通过上面的3个元素，可以指向一个特定工程的版本。这样Maven就能够准确的处理相应的工程。

###### packaging 
通过```groupId:artifactId:version```我们就获得了相应的地址结构。此外，还有一个更完整的标签来告诉我们这到底要打包成什么，这就是```<packaging>```标签。在上面的例子中，```org.codehaus.mojo:my-project:1.0```这个工程将会打包一个```jar```，然后我们可以通过如下将其打包成一个```war```包：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <packaging>war</packaging>
  ...
</project>
{% endhighlight %}
当并没有指定```packaging```时，默认是打包成```jar```包。我们可以在org.apache.maven.lifecycle.mapping.LifecycleMapping中找到所支持的包类型。当前，核心的packaging值有：pom, jar, maven-plugin, ejb, war, ear, rar。



### 2.2 POM RelationShips
Maven的一个强大之处在于其能够处理project之间的关系：这包括依赖（和传递依赖）、继承、集合(multi-module projects)。

对于大多数工程来说，依赖的管理都是一项十分复杂的工作。


#### 2.2.1 Dependencies

POM的基石其实就是其```依赖列表```(dependencies list)。大多数工程都会依赖于其他来构建和运行。假如所有Maven都可以帮助你管理此列表，那么你将可以从中得到解放。在编译(或实现其他goal)时，Maven会下载并链接这些依赖。此外，Maven还可自动处理传递依赖的问题，这样就可以只关注项目的直接依赖就行。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <dependencies>
    <dependency>
      <groupId>junit</groupId>
      <artifactId>junit</artifactId>
      <version>4.12</version>
      <type>jar</type>
      <scope>test</scope>
      <optional>true</optional>
    </dependency>
    ...
  </dependencies>
  ...
</project>
{% endhighlight %}

* groupId, artifactId, version: 在Maven中我们经常会看到这三个字段。maven是采用这3个字段来计算一个project的坐标。

有的时候，我们可能不能从Maven中央仓库下载对应的工程。例如，某个project依赖于一个jar包，但是该jar包可能是闭源的，并不能从中央仓库下载。有3种方法来处理此种场景：

  * 方法1： 使用install plugin在本地安装依赖。这是最简单且推荐的方法。例如：
<pre>
# mvn install:install-file -Dfile=non-maven-proj.jar -DgroupId=some.group -DartifactId=non-maven-proj -Dversion=1 -Dpackaging=jar
</pre>
  需要指出的是，这仍然需要一个地址，只是此时你可以使用命令行，且install plugin会根据你给定的地址创建POM。

  * 方法2： 创建你自己的私有仓库，然后将相应的依赖包到那里。这在公司只能连上局域网的情况下，此种方式是最有效的

  * 方法3： 将依赖的scope设置为system，然后定义一个systemPatch。这是不推荐的做法，但是这使得我们需要对如下一些字段进行解释。

* classifier： 用于区分来自于相同的POM，但是内容不同的artifacts。其可能是一个添加在版本号之后的随机字符串。

考虑如下一种场景，假设一个project其设计的目标是提供一个Java 11版本的artifact，但同时又可能需要支持JDK1.8。因此，我们可以通过此字段来选择到底使用哪一个。

classifier的另一个使用场景是，将另一个次artifact附加到主artifact上。假如你浏览Maven中央仓库的话，你就会发现有```source```和```javadoc```这两个classifiers分别来部署工程源代码和API文档，然后跟打包的class文件一起被发布。

* type: 对应于所选择的依赖类型。默认值为```jar```。通常代表了所依赖的文件名的后缀。

* scope: 本元素指示在哪些task下需要该classpath，并且限制依赖的传递性。有如下5个范围可用：

  * compile: 这是默认范围。编译依赖在所有的classpaths种都可用。此种依赖会传递到其他project

  * provided: 与compile依赖类似。但通常是指示你需要指定的JDK版本。一般是在编译或者test classpath下会用到，不会传递

  * runtime: 指示对应的依赖不会在编译时用到，但是会在执行时用到。它是处于runtime及test classpath下，但不在compile classpath下

  * test：test编译及执行时才需要的依赖，一般应用程序的执行并不会对其产生依赖

  * system: 此scope与provided类似，但是你需要在本地拥有指定的jar。maven并不会在仓库种去查找

* systemPath： 当scope为system时，才会用到本字段。否则，如果设置本字段，maven将会构建失败。所设定的path必须是一个绝对路径。因此，我们建议使用property来指定路径，比如```${java.home}/lib```。

1) **Exclusions**

Exclusions用于告诉Maven不要将依赖的依赖包含进来（换句话说，就是不要把传递依赖包含进来）。例如，```maven-embedder```需要```maven-core```，但是我们并不想把maven-core也包含进来：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <dependencies>
    <dependency>
      <groupId>org.apache.maven</groupId>
      <artifactId>maven-embedder</artifactId>
      <version>2.0</version>
      <exclusions>
        <exclusion>
          <groupId>org.apache.maven</groupId>
          <artifactId>maven-core</artifactId>
        </exclusion>
      </exclusions>
    </dependency>
    ...
  </dependencies>
  ...
</project>
{% endhighlight %}


#### 2.2.2 Inheritance

Maven的另一项强大之处在于其针对构建系统引入了project inheritance：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <groupId>org.codehaus.mojo</groupId>
  <artifactId>my-parent</artifactId>
  <version>2.0</version>
  <packaging>pom</packaging>
</project>
{% endhighlight %}
上面的```packaging```必须为pom。在Maven构建的整个生命周期种，其会根据对应目标(goal)的类型执行不同的命令。比如，当```packaging```设置为```jar```时，则在package阶段会执行```jar:jar```。之后，我们可以在parent POM种再添加其他的值。parent POM种的如下元素可以被child pom继承：
<pre>
groupId
version
description
url
inceptionYear
organization
licenses
developers
contributors
mailingLists
scm
issueManagement
ciManagement
properties
dependencyManagement
dependencies
repositories
pluginRepositories
build
  plugin executions with matching ids
  plugin configuration
  etc.
reporting
profiles
</pre>

而下面3个元素将不会被继承：
<pre>
artifactId
name
prerequisites
</pre>
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <modelVersion>4.0.0</modelVersion>
 
  <parent>
    <groupId>org.codehaus.mojo</groupId>
    <artifactId>my-parent</artifactId>
    <version>2.0</version>
    <relativePath>../my-parent</relativePath>
  </parent>
 
  <artifactId>my-project</artifactId>
</project>
{% endhighlight %}
上面注意```relativePath```元素。该字段为非必须的，但是其会作为一个标识来告诉Maven优先查找指定的路径的parent，然后才查找本地仓库和远程仓库。


1) **The Super POM**

正像是Java中的所有对象均继承自```java.lang.Object```，所有的POM也都继承自一个Super POM，下面我们来看看该Super POM:
{% highlight string %}
<project>
  <modelVersion>4.0.0</modelVersion>
 
  <repositories>
    <repository>
      <id>central</id>
      <name>Central Repository</name>
      <url>https://repo.maven.apache.org/maven2</url>
      <layout>default</layout>
      <snapshots>
        <enabled>false</enabled>
      </snapshots>
    </repository>
  </repositories>
 
  <pluginRepositories>
    <pluginRepository>
      <id>central</id>
      <name>Central Repository</name>
      <url>https://repo.maven.apache.org/maven2</url>
      <layout>default</layout>
      <snapshots>
        <enabled>false</enabled>
      </snapshots>
      <releases>
        <updatePolicy>never</updatePolicy>
      </releases>
    </pluginRepository>
  </pluginRepositories>
 
  <build>
    <directory>${project.basedir}/target</directory>
    <outputDirectory>${project.build.directory}/classes</outputDirectory>
    <finalName>${project.artifactId}-${project.version}</finalName>
    <testOutputDirectory>${project.build.directory}/test-classes</testOutputDirectory>
    <sourceDirectory>${project.basedir}/src/main/java</sourceDirectory>
    <scriptSourceDirectory>${project.basedir}/src/main/scripts</scriptSourceDirectory>
    <testSourceDirectory>${project.basedir}/src/test/java</testSourceDirectory>
    <resources>
      <resource>
        <directory>${project.basedir}/src/main/resources</directory>
      </resource>
    </resources>
    <testResources>
      <testResource>
        <directory>${project.basedir}/src/test/resources</directory>
      </testResource>
    </testResources>
    <pluginManagement>
      <!-- NOTE: These plugins will be removed from future versions of the super POM -->
      <!-- They are kept for the moment as they are very unlikely to conflict with lifecycle mappings (MNG-4453) -->
      <plugins>
        <plugin>
          <artifactId>maven-antrun-plugin</artifactId>
          <version>1.3</version>
        </plugin>
        <plugin>
          <artifactId>maven-assembly-plugin</artifactId>
          <version>2.2-beta-5</version>
        </plugin>
        <plugin>
          <artifactId>maven-dependency-plugin</artifactId>
          <version>2.8</version>
        </plugin>
        <plugin>
          <artifactId>maven-release-plugin</artifactId>
          <version>2.5.3</version>
        </plugin>
      </plugins>
    </pluginManagement>
  </build>
 
  <reporting>
    <outputDirectory>${project.build.directory}/site</outputDirectory>
  </reporting>
 
  <profiles>
    <!-- NOTE: The release profile will be removed from future versions of the super POM -->
    <profile>
      <id>release-profile</id>
 
      <activation>
        <property>
          <name>performRelease</name>
          <value>true</value>
        </property>
      </activation>
 
      <build>
        <plugins>
          <plugin>
            <inherited>true</inherited>
            <artifactId>maven-source-plugin</artifactId>
            <executions>
              <execution>
                <id>attach-sources</id>
                <goals>
                  <goal>jar-no-fork</goal>
                </goals>
              </execution>
            </executions>
          </plugin>
          <plugin>
            <inherited>true</inherited>
            <artifactId>maven-javadoc-plugin</artifactId>
            <executions>
              <execution>
                <id>attach-javadocs</id>
                <goals>
                  <goal>jar</goal>
                </goals>
              </execution>
            </executions>
          </plugin>
          <plugin>
            <inherited>true</inherited>
            <artifactId>maven-deploy-plugin</artifactId>
            <configuration>
              <updateReleaseInfo>true</updateReleaseInfo>
            </configuration>
          </plugin>
        </plugins>
      </build>
    </profile>
  </profiles>
 
</project>
{% endhighlight %}

#### 2.2.3 Properties

Properties是POM基础种我们需要了解的最后一项内容。Maven properties其实是一个值占位符，类似于Ant中的properties。它们的值可以再POM中的任何位置被访问，访问方式为```${X}```。
{% highlight string %}
<project>
  ...
  <properties>
    <maven.compiler.source>1.7</maven.compiler.source>
    <maven.compiler.target>1.7</maven.compiler.target>
    <!-- Following project.-properties are reserved for Maven in will become elements in a future POM definition. -->
    <!-- Don't start your own properties properties with project. -->
    <project.build.sourceEncoding>UTF-8</project.build.sourceEncoding> 
    <project.reporting.outputEncoding>UTF-8</project.reporting.outputEncoding>
  </properties>
  ...
</project>
{% endhighlight %}




## 3. Build Settings

除了上面给出的POM基本知识意外，在使用POM前我们需要了解另外两个元素。分别为```build```元素和```report```元素。其中```build```元素用来声明工程的目录结构和管理plugin；而```report```元素则用于构建相应的报告信息。

### 3.1 Build
根据POM 4.0.0 XSD规范，```build```元素从概念上分为两个部分：```BaseBuild```、```Build```。其中```BaseBuild```表示那些在两个build(顶级节点project下的build，profiles下的build)间都可用的元素；而```Build```类型不仅仅包含```BaseBuild```元素，而且还包含其他的一些顶级元素（即：Build类型不但公共部分，而且还包含一些非公共部分）。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <!-- "Project Build" contains more elements than just the BaseBuild set -->
  <build>...</build>
 
  <profiles>
    <profile>
      <!-- "Profile Build" contains a subset of "Project Build"s elements -->
      <build>...</build>
    </profile>
  </profiles>
</project>
{% endhighlight %}
>提示：这些不同的build元素可以表示为project build和profile build

* ProjectBuild即为POM文件中的build节点，对应Maven项目的构建配置管理

* ProfileBuild即针对不同环境设置的项目构建配置管

* BaseBuild即build节点与profiles/profile/build节点的一系列共有属性

此外，在阅读下面的内容之前，我们再回顾以下Build Lifecycle、Phase与Goal等概念：

* Build Lifecycle即Maven中项目管理的生命周期，Maven内置了三种生命周期：default，clean，site。

* Phase即生命周期中的各个阶段；以default为例，其生命周期分为validate，compile，test，package，verify，install，deploy等几个阶段，如package即表示生命周期中的打包阶段。

* Goal即各个Phase所定义的执行步骤；这些具体的执行步骤通常由Maven插件提供，使用时的命令格式通常为plugin:goal。

综上，一个完整的生命周期Build Lifecycle由一个或多个阶段Phase组成，一个阶段Phase由零个或多个步骤Goal组成；同一个步骤Goal可以从属于零个或多个阶段Phase；一个插件可以同时提供多个Goal，可以将其理解为一个插件根据不同参数以提供不同的能力。




#### 3.1.1 BaseBuild元素集
BaseBuild如其名字所言，就是POM文件中两个build元素集中的那些公共部分。
{% highlight string %}
<build>
  <defaultGoal>install</defaultGoal>
  <directory>${basedir}/target</directory>
  <finalName>${artifactId}-${version}</finalName>
  <filters>
    <filter>filters/filter1.properties</filter>
  </filters>
  ...
</build>
{% endhighlight %}

* defaultGoal: 默认构建目标，这里的目标可以时```goal```，也可以是```phase```。假如指定的是```goal```的话，那么其应该与命令行的指定方式一致(例如：```jar:jar```)；假如指定的是```phase```的话，也要符合相应的规范（例如：```install```)

* directory: build的输出文件目录；默认为${basedir}/target，若使用相对路径，则当前目录为POM文件所在目录。

* finalName: 这一系列工程构建完成之后的最后输出文件名（注：不带文件扩展名，如my-project-1.0.jar)。默认为```${artifactId}-${version}```。这里的```finalName```可能有一点使用不当，因为plugins在构建工程时都有能力去修改该名称（通常不会修改）。比如，若maven-jar-plugin的classfier被配置为了```test```，那么最后生成的jar包的名称为```my-project-1.0-test.jar```。

* filter: 定义属性文件，该属性文件中的内容将会应用于Resource中。


#### 1. Resources

Build元素的另一个特征是可以指定工程中资源存在的位置。Resources通常并不是源代码，它们通常并不需要进行编译，但是一般由于某些用途会放在工程中。

例如，一个Plexus工程需要一个```configuration.xml```配置文件放于```META-INF/plexus```目录下。尽管我们可以将资源直接放于src/main/resources/META-INF/plexus目录中(注：Maven默认会把src/main/resources目录下的文件打包到Jar)，但是有时候我们可能想要一个自己定义的目录src/main/plexus。那么此种情况下，如果我们想要正确的将资源打包到Jar中，类似如下方式指定资源即可：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <build>
    ...
    <resources>
      <resource>
        <targetPath>META-INF/plexus</targetPath>
        <filtering>false</filtering>
        <directory>${basedir}/src/main/plexus</directory>
        <includes>
          <include>configuration.xml</include>
        </includes>
        <excludes>
          <exclude>**/*.properties</exclude>
        </excludes>
      </resource>
    </resources>
    <testResources>
      ...
    </testResources>
    ...
  </build>
</project>
{% endhighlight %}

* resources: 包含一系列的resource节点，每个resource节点指明本工程所关联的资源

* targetPath: 指明资源构建后的目录结构。target path的默认值为base directory。对于Goal为Jar来说，通常会将此字段设置为```META-INF```

* filtering: 可设置为```true```或者```false```，表示当前资源文件配置节点下，是否启用build/filters下的环境变量。注意```*.properties```中的属性，以及通过命令行```-D```选项可以在resource中直接使用，而不需要通过此字段来启用。

* directory: 指定资源的存放位置。默认为${basedir}/src/main/resources

* includes: 模式匹配，用于指定包含directory目录下的哪些文件。可以使用```*```通配符。

* excludes: 与includes类似。假如includes与excludes有冲突，则以excludes为准

* testResources: testResources节点下包含testResource元素，其结构类似于resource，但一般只用于test phase。

###### 2. Plugins

{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  <build>
    ...
    <plugins>
      <plugin>
        <groupId>org.apache.maven.plugins</groupId>
        <artifactId>maven-jar-plugin</artifactId>
        <version>2.6</version>
        <extensions>false</extensions>
        <inherited>true</inherited>
        <configuration>
          <classifier>test</classifier>
        </configuration>
        <dependencies>...</dependencies>
        <executions>...</executions>
      </plugin>
    </plugins>
  </build>
</project>
{% endhighlight %} 

除了拥有标准的```groupId:artifactId:version```坐标元素之外，还有一些另外的元素用于配置plugin。

* extensions: 可设置为```true```或```false```，表示是否要加载此plugin的extensions。默认值为false。关于```extensions```我们后面会讲述到。

* inherited: 可设置为```true```或```false```，表示本plugin的配置是否可以被继承。默认值为```true```

* configuration: 该plugin的配置信息。我们并不需要深入了解该plugin的运作机理，只需要配置相关属性即可。

假如你的POM声明了一个parent的话，那么其可以从parent的build/plugins或者pluginManagement处继承相关的配置信息。下面我们简单的介绍一些。

  * default configuration inheritance
  
为了演示此类继承，我们假定parent POM中如下片段：
{% highlight string %}
<plugin>
  <groupId>my.group</groupId>
  <artifactId>my-plugin</artifactId>
  <configuration>
    <items>
      <item>parent-1</item>
      <item>parent-2</item>
    </items>
    <properties>
      <parentKey>parent</parentKey>
    </properties>
  </configuration>
</plugin>
{% endhighlight %}
考虑如下的plugin配置，其会从其parent处继承相关配置信息：
{% highlight string %}
<plugin>
  <groupId>my.group</groupId>
  <artifactId>my-plugin</artifactId>
  <configuration>
    <items>
      <item>child-1</item>
    </items>
    <properties>
      <childKey>child</childKey>
    </properties>
  </configuration>
</plugin>
{% endhighlight %}
默认行为是根据元素名来合并相关配置。假如child POM有一个特定的元素，那么就采用该元素的值。假如child POM中并没有对某个元素进行配置，但是parent POM中对该元素进行了配置，那么就采用parent的值。值得注意的是，这其实仅仅指示XML默认的操作行为，与plugin其实无关。

采用上述所讲的规则之后，最终Maven看起来如下：
{% highlight string %}
<plugin>
  <groupId>my.group</groupId>
  <artifactId>my-plugin</artifactId>
  <configuration>
    <items>
      <item>child-1</item>
    </items>
    <properties>
      <childKey>child</childKey>
      <parentKey>parent</parentKey>
    </properties>
  </configuration>
</plugin>
{% endhighlight %}




  * advanced configuration inheritance： combine.children和combine.self

你可以控制child POMs采用何种方式从parent POMs处继承配置信息。在child POMs中通过设置继承属性为```combine.children```或```combine.self```，就能控制其如何合并来自parent的配置信息。

如下是child POM的相关配置，我们继承属性的使用：
{% highlight string %}
<configuration>
  <items combine.children="append">
    <!-- combine.children="merge" is the default -->
    <item>child-1</item>
  </items>
  <properties combine.self="override">
    <!-- combine.self="merge" is the default -->
    <childKey>child</childKey>
  </properties>
</configuration>
{% endhighlight %}

合并后的最终结果如下：
{% highlight string %}
<configuration>
  <items combine.children="append">
    <item>parent-1</item>
    <item>parent-2</item>
    <item>child-1</item>
  </items>
  <properties combine.self="override">
    <childKey>child</childKey>
  </properties>
</configuration>
{% endhighlight %}
当合并属性设置为```combine.children="append"```时，就会将child与parent相关元素按顺序连接起来；当合并属性设置为```combine.self="override"```时，则结果与combine.children="append"完全相反，其会完全抑制parent的配置。针对一个元素，不能同时使用combine.self="override"和combine.children="append"。

值得注意的是，这些继承属性只用于那些声明了的元素上，并不会传递到内部的子元素。即假如child POM中某一个元素是一个复杂的结构，而不是一个简单的文本字串，那么其子元素仍然遵循默认的继承策略。


* dependencies：在POM中，我们会看到很多dependencies，其是plugins节点下的一个元素。plugin下的dependencies与Basic下的dependencies结构和功能都一致。主要的不同在于，Basic下的dependencies是整个工程的依赖，而plugin下的dependencies是对应plugin的依赖。这里通过dependencies节点，可以实现plugin依赖列表的修改，比如通过```exclusions```移除未使用的runtime dependency，或者修改相应依赖的版本。


* executions：我们需要记住的一个很重要的点是一个plugin可能有多个goal。每一个不同的goal都可能有一个单独的配置，甚至将一个plugin goal绑定到不同的build phase阶段。```executions```用于配置一个plugin goal的```execution```

看如下例子，假如你想要将```antrun:run```这个goal绑定到```verify```这个build phase中。我们想要在verify过程中打印处构建目录(build directory)，并且通过设置```inherited```为```false```来避免传递其配置到children。此时，```execution```看起来如下：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <build>
    <plugins>
      <plugin>
        <artifactId>maven-antrun-plugin</artifactId>
        <version>1.1</version>
        <executions>
          <execution>
            <id>echodir</id>
            <goals>
              <goal>run</goal>
            </goals>
            <phase>verify</phase>
            <inherited>false</inherited>
            <configuration>
              <tasks>
                <echo>Build Dir: ${project.build.directory}</echo>
              </tasks>
            </configuration>
          </execution>
        </executions>
 
      </plugin>
    </plugins>
  </build>
</project>
{% endhighlight %}

* id: 用于指定本execution块的标识。当对应的phase运行时，其显示的样式为: [plugin:goal execution: id]。在本例子中，显示为[antrun:run execution: echodir].

* goals: 像其他多元化的POM元素一样，其会包含goals列表中的单个goal。在这种情况下，plugin的goals列表是是通过execution块来指定的。

* phase: 用于指定goals列表中goal的执行phase。这是一个很强大的选项，允许在build lifecycle中将goal绑定到任何phase

* inherited: 与上面所讲述的inherited元素一致，设置为false将会抑制将该execution传递给children。该字段仅仅只对parent POMs有效

* configuration： 与上述所讲述的configuration元素一致，但是仅仅只影响当前plugin所对应的goal。




###### 3. Plugin Management

* pluginManagement： 该元素出现在plugins元素旁边。Plugin Management也包含plugin元素，但其对应的plugin配置信息并不仅仅针对特定的project构建，还希望能够其配置能够被其他POM所继承（注：可以在childen POMs中对所继承的pluginManagement进行覆盖）。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <build>
    ...
    <pluginManagement>
      <plugins>
        <plugin>
          <groupId>org.apache.maven.plugins</groupId>
          <artifactId>maven-jar-plugin</artifactId>
          <version>2.6</version>
          <executions>
            <execution>
              <id>pre-process-classes</id>
              <phase>compile</phase>
              <goals>
                <goal>jar</goal>
              </goals>
              <configuration>
                <classifier>pre-process</classifier>
              </configuration>
            </execution>
          </executions>
        </plugin>
      </plugins>
    </pluginManagement>
    ...
  </build>
</project>
{% endhighlight %}

假如我们将这些设置加入到plugins元素中，那么其仅仅只会作用于单个POM。然而，如果我们将其放在```pluginManagement```节点下，那么本POM以及children POM在添加maven-jar-plugin来构建的时候，均会获得```pre-process-classess```执行块。

有了上述配置之后，在children中我们就可以简化写成：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <build>
    ...
    <plugins>
      <plugin>
        <groupId>org.apache.maven.plugins</groupId>
        <artifactId>maven-jar-plugin</artifactId>
      </plugin>
    </plugins>
    ...
  </build>
</project>
{% endhighlight %}


#### 3.1.2 Base元素集

在XSD的定义中，```Build```类型是指那些仅在```project build```中生效的元素。不管有多少其他的子元素，真正只存在于```project build```而```profile build```中不存在的元素只有两个：directories和extensions

>关于project build与profile build，请参看本节开头的说明

###### 1. Directories
如下一系列的directory节点存在于build元素下，用于设置各种目录结构。由于其不会存在于profile builds中，因此其不能被profiles所修改：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <build>
    <sourceDirectory>${basedir}/src/main/java</sourceDirectory>
    <scriptSourceDirectory>${basedir}/src/main/scripts</scriptSourceDirectory>
    <testSourceDirectory>${basedir}/src/test/java</testSourceDirectory>
    <outputDirectory>${basedir}/target/classes</outputDirectory>
    <testOutputDirectory>${basedir}/target/test-classes</testOutputDirectory>
    ...
  </build>
</project>
{% endhighlight %}
假如上面这些```Directory```元素所设置的是一个绝对路径，则相应的目录被使用；假如设置的是相对路径，则其是相对于```${basedir}```的。

###### 2. Extensions

Extensions是在构建中所用到的artifacts列表。它们会被包含在构建时的classpath中。可以向build process启用相应的extensions，也可以在build lifecycle中激活相应的plugin。换句话说，extensions就是在build过程中被激活的artifacts。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <build>
    ...
    <extensions>
      <extension>
        <groupId>org.apache.maven.wagon</groupId>
        <artifactId>wagon-ftp</artifactId>
        <version>1.0-alpha-3</version>
      </extension>
    </extensions>
    ...
  </build>
</project>
{% endhighlight %}

### 3.2 Reporting
Reporting包含了site generation阶段相关的元素。许多Maven插件都可以配置在Reporting节点下以生成报告信息，例如产生javadoc报告。与上文介绍的Build元素可以配置plugins类似，reporting也有相似的能力。两者之间明显的不同在于： build节点下是在```execution```块下控制plugin goals， 而reporting是在```reportSet```下配置plugin goals。一个微妙的区别在于：reporting元素下的plugin配置与build元素下的plugin配置工作方式一致，但事实并非如此（build plugin配置并不会影响reporting plugin）。

假如你对```build```节点下的元素了解清楚的话，那么对```reporting```节点下的元素应该也不会陌生，在```reporting```节点下，可能唯一的一个新元素就是Boolean类型的```excludeDefaults```元素了。该元素标识site generator不产生默认情况下生成的报告。当通过build的site阶段产生报告信息时，则通常还会包含一个```Project Info```段，从而产生一个完整的报告。这些报告的目标都是由```maven-project-info-reports-plugin```产生的。
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <reporting>
    <outputDirectory>${basedir}/target/site</outputDirectory>
    <plugins>
      <plugin>
        <artifactId>maven-project-info-reports-plugin</artifactId>
        <version>2.0.1</version>
        <reportSets>
          <reportSet></reportSet>
        </reportSets>
      </plugin>
    </plugins>
  </reporting>
  ...
</project>
{% endhighlight %}

#### 3.2.1 Report Sets
需要记住的很重要的一点是：一个plugin可能有多个goals。而每一个不同的goal都可能有不同的配置。在Report节点下可以设置相关report插件的goal配置。

举个例子，假设你想要配置```javadoc:javadoc```这样一个goal来链接到[http://java.sun.com/j2se/1.5.0/docs/api/](http://java.sun.com/j2se/1.5.0/docs/api/)，并让其可以被children继承，那么可以配置如下：
{% highlight string %}
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 https://maven.apache.org/xsd/maven-4.0.0.xsd">
  ...
  <reporting>
    <plugins>
      <plugin>
        ...
        <reportSets>
          <reportSet>
            <id>sunlink</id>
            <reports>
              <report>javadoc</report>
            </reports>
            <inherited>true</inherited>
            <configuration>
              <links>
                <link>http://java.sun.com/j2se/1.5.0/docs/api/</link>
              </links>
            </configuration>
          </reportSet>
        </reportSets>
      </plugin>
    </plugins>
  </reporting>
  ...
</project>
{% endhighlight %}







<br />
<br />
**[参看]：**

1. [Centos7.3下部署Java开发环境](https://ivanzz1001.github.io/records/post/linux/2017/09/19/linux-java-install)

2. [maven官网](https://maven.apache.org/)

3. [Maven 编译及打包](https://www.jianshu.com/p/c91002c43258?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)

4. [maven教程](https://www.runoob.com/maven/maven-intellij.html)

5. [maven中央仓库](https://mvnrepository.com/)

<br />
<br />
<br />

