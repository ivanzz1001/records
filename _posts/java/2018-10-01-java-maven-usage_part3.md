---
layout: post
title: maven使用案例
tags:
- java-language
categories: java-language
description: maven使用案例
---


本章我们通过一个简单的案例来介绍Maven的使用。


<!-- more -->

## 1. Maven简单使用案例
下面我们给出一个简单的案例来演示如何使用Maven。

### 1.1 安装Maven
对于Maven的安装，我们在前面已经介绍过，这里不再细述。

### 1.2 创建Project
我们需要在某个地方创建工程，因此首先创建一个文件夹```workspace```，然后在该文件夹下执行如下命令生成一个简单的java工程：
{% highlight string %}
# mkdir -p workspace
# cd workspace
# pwd
/root/workspace
# mvn archetype:generate -DgroupId=com.mycompany.app -DartifactId=my-app -DarchetypeArtifactId=maven-archetype-quickstart -DarchetypeVersion=1.4 -DinteractiveMode=false
[INFO] Scanning for projects...
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/2.5/maven-clean-plugin-2.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/2.5/maven-clean-plugin-2.5.pom (3.9 kB at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/22/maven-plugins-22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/22/maven-plugins-22.pom (13 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/21/maven-parent-21.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/21/maven-parent-21.pom (26 kB at 33 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/10/apache-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/10/apache-10.pom (15 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/2.5/maven-clean-plugin-2.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/2.5/maven-clean-plugin-2.5.jar (25 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.4/maven-install-plugin-2.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.4/maven-install-plugin-2.4.pom (6.4 kB at 9.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/23/maven-plugins-23.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/23/maven-plugins-23.pom (9.2 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/22/maven-parent-22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/22/maven-parent-22.pom (30 kB at 9.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/11/apache-11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/11/apache-11.pom (15 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.4/maven-install-plugin-2.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.4/maven-install-plugin-2.4.jar (27 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.7/maven-deploy-plugin-2.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.7/maven-deploy-plugin-2.7.pom (5.6 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.7/maven-deploy-plugin-2.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.7/maven-deploy-plugin-2.7.jar (27 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.3/maven-site-plugin-3.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.3/maven-site-plugin-3.3.pom (21 kB at 7.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/24/maven-plugins-24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/24/maven-plugins-24.pom (11 kB at 9.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/23/maven-parent-23.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/23/maven-parent-23.pom (33 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/13/apache-13.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/13/apache-13.pom (14 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.3/maven-site-plugin-3.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.3/maven-site-plugin-3.3.jar (124 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-antrun-plugin/1.3/maven-antrun-plugin-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-antrun-plugin/1.3/maven-antrun-plugin-1.3.pom (4.7 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/12/maven-plugins-12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/12/maven-plugins-12.pom (12 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/9/maven-parent-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/9/maven-parent-9.pom (33 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/4/apache-4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/4/apache-4.pom (4.5 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-antrun-plugin/1.3/maven-antrun-plugin-1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-antrun-plugin/1.3/maven-antrun-plugin-1.3.jar (24 kB at 9.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-assembly-plugin/2.2-beta-5/maven-assembly-plugin-2.2-beta-5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-assembly-plugin/2.2-beta-5/maven-assembly-plugin-2.2-beta-5.pom (15 kB at 6.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/16/maven-plugins-16.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/16/maven-plugins-16.pom (13 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/15/maven-parent-15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/15/maven-parent-15.pom (24 kB at 10.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/6/apache-6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/6/apache-6.pom (13 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-assembly-plugin/2.2-beta-5/maven-assembly-plugin-2.2-beta-5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-assembly-plugin/2.2-beta-5/maven-assembly-plugin-2.2-beta-5.jar (209 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-dependency-plugin/2.8/maven-dependency-plugin-2.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-dependency-plugin/2.8/maven-dependency-plugin-2.8.pom (11 kB at 8.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-dependency-plugin/2.8/maven-dependency-plugin-2.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-dependency-plugin/2.8/maven-dependency-plugin-2.8.jar (153 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-release-plugin/2.5.3/maven-release-plugin-2.5.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-release-plugin/2.5.3/maven-release-plugin-2.5.3.pom (11 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/release/maven-release/2.5.3/maven-release-2.5.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/release/maven-release/2.5.3/maven-release-2.5.3.pom (5.0 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/27/maven-parent-27.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/27/maven-parent-27.pom (41 kB at 6.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/17/apache-17.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/17/apache-17.pom (16 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-release-plugin/2.5.3/maven-release-plugin-2.5.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-release-plugin/2.5.3/maven-release-plugin-2.5.3.jar (53 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-metadata.xml
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/maven-metadata.xml
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/mojo/maven-metadata.xml (20 kB at 16 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-metadata.xml (14 kB at 7.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/maven-metadata.xml
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/maven-metadata.xml (949 B at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/3.2.0/maven-archetype-plugin-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/3.2.0/maven-archetype-plugin-3.2.0.pom (11 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/maven-archetype/3.2.0/maven-archetype-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/maven-archetype/3.2.0/maven-archetype-3.2.0.pom (12 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/34/maven-parent-34.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/34/maven-parent-34.pom (43 kB at 8.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/23/apache-23.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/23/apache-23.pom (18 kB at 8.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/3.2.0/maven-archetype-plugin-3.2.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-archetype-plugin/3.2.0/maven-archetype-plugin-3.2.0.jar (97 kB at 8.6 kB/s)
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
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-catalog/3.2.0/archetype-catalog-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-catalog/3.2.0/archetype-catalog-3.2.0.pom (1.9 kB at 1.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-models/3.2.0/archetype-models-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-models/3.2.0/archetype-models-3.2.0.pom (2.7 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.3.0/plexus-utils-3.3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.3.0/plexus-utils-3.3.0.pom (5.2 kB at 5.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/5.1/plexus-5.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/5.1/plexus-5.1.pom (23 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-descriptor/3.2.0/archetype-descriptor-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-descriptor/3.2.0/archetype-descriptor-3.2.0.pom (2.0 kB at 3.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-common/3.2.0/archetype-common-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-common/3.2.0/archetype-common-3.2.0.pom (17 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/2.4.16/groovy-2.4.16.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/2.4.16/groovy-2.4.16.pom (19 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/ivy/ivy/2.5.0/ivy-2.5.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/ivy/ivy/2.5.0/ivy-2.5.0.pom (6.8 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/7/apache-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/7/apache-7.pom (14 kB at 8.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sourceforge/jchardet/jchardet/1.0/jchardet-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/net/sourceforge/jchardet/jchardet/1.0/jchardet-1.0.pom (1.3 kB at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/2.0.0/plexus-component-annotations-2.0.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/2.0.0/plexus-component-annotations-2.0.0.pom (750 B at 825 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/2.0.0/plexus-containers-2.0.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/2.0.0/plexus-containers-2.0.0.pom (4.8 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jdom/jdom/1.1.3/jdom-1.1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/jdom/jdom/1.1.3/jdom-1.1.3.pom (1.8 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/3.0/maven-model-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/3.0/maven-model-3.0.pom (3.9 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven/3.0/maven-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven/3.0/maven-3.0.pom (22 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/3.0/maven-core-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/3.0/maven-core-3.0.pom (6.6 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/3.0/maven-settings-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/3.0/maven-settings-3.0.pom (1.9 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings-builder/3.0/maven-settings-builder-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings-builder/3.0/maven-settings-builder-3.0.pom (2.2 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.14/plexus-interpolation-1.14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.14/plexus-interpolation-1.14.pom (910 B at 1.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.18/plexus-components-1.1.18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.18/plexus-components-1.1.18.pom (5.4 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.7/plexus-2.0.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.7/plexus-2.0.7.pom (17 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-sec-dispatcher/1.3/plexus-sec-dispatcher-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-sec-dispatcher/1.3/plexus-sec-dispatcher-1.3.pom (3.0 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/12/spice-parent-12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/12/spice-parent-12.pom (6.8 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/4/forge-parent-4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/4/forge-parent-4.pom (8.4 kB at 5.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-cipher/1.4/plexus-cipher-1.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-cipher/1.4/plexus-cipher-1.4.pom (2.1 kB at 4.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/3.0/maven-repository-metadata-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/3.0/maven-repository-metadata-3.0.pom (1.9 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/3.0/maven-artifact-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/3.0/maven-artifact-3.0.pom (1.9 kB at 1.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/3.0/maven-plugin-api-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/3.0/maven-plugin-api-3.0.pom (2.3 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-plexus/1.4.2/sisu-inject-plexus-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-plexus/1.4.2/sisu-inject-plexus-1.4.2.pom (5.4 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/inject/guice-plexus/1.4.2/guice-plexus-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/inject/guice-plexus/1.4.2/guice-plexus-1.4.2.pom (3.1 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/inject/guice-bean/1.4.2/guice-bean-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/inject/guice-bean/1.4.2/guice-bean-1.4.2.pom (2.6 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject/1.4.2/sisu-inject-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject/1.4.2/sisu-inject-1.4.2.pom (1.2 kB at 2.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-parent/1.4.2/sisu-parent-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-parent/1.4.2/sisu-parent-1.4.2.pom (7.8 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/6/forge-parent-6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/6/forge-parent-6.pom (11 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.2.3/plexus-classworlds-2.2.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.2.3/plexus-classworlds-2.2.3.pom (4.0 kB at 2.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.6/plexus-2.0.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.6/plexus-2.0.6.pom (17 kB at 8.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-bean/1.4.2/sisu-inject-bean-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-bean/1.4.2/sisu-inject-bean-1.4.2.pom (5.5 kB at 6.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-guice/2.1.7/sisu-guice-2.1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-guice/2.1.7/sisu-guice-2.1.7.pom (11 kB at 5.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model-builder/3.0/maven-model-builder-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model-builder/3.0/maven-model-builder-3.0.pom (2.2 kB at 4.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-aether-provider/3.0/maven-aether-provider-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-aether-provider/3.0/maven-aether-provider-3.0.pom (2.5 kB at 2.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-api/1.7/aether-api-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-api/1.7/aether-api-1.7.pom (1.7 kB at 1.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-parent/1.7/aether-parent-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-parent/1.7/aether-parent-1.7.pom (7.7 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-util/1.7/aether-util-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-util/1.7/aether-util-1.7.pom (2.1 kB at 4.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-impl/1.7/aether-impl-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-impl/1.7/aether-impl-1.7.pom (3.7 kB at 4.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-spi/1.7/aether-spi-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-spi/1.7/aether-spi-1.7.pom (1.7 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-invoker/3.0.1/maven-invoker-3.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-invoker/3.0.1/maven-invoker-3.0.1.pom (4.9 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/31/maven-shared-components-31.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/31/maven-shared-components-31.pom (5.1 kB at 7.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/31/maven-parent-31.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/31/maven-parent-31.pom (43 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/19/apache-19.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/19/apache-19.pom (15 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.pom (5.6 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/30/maven-shared-components-30.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/30/maven-shared-components-30.pom (4.6 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/30/maven-parent-30.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/30/maven-parent-30.pom (41 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/18/apache-18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/18/apache-18.pom (16 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.pom (14 kB at 6.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/42/commons-parent-42.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/42/commons-parent-42.pom (68 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.12.0/maven-artifact-transfer-0.12.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.12.0/maven-artifact-transfer-0.12.0.pom (11 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/33/maven-shared-components-33.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/33/maven-shared-components-33.pom (5.1 kB at 5.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/33/maven-parent-33.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/33/maven-parent-33.pom (44 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/21/apache-21.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/21/apache-21.pom (17 kB at 3.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-common-artifact-filters/3.0.1/maven-common-artifact-filters-3.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-common-artifact-filters/3.0.1/maven-common-artifact-filters-3.0.1.pom (4.8 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.1.0/maven-shared-utils-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.1.0/maven-shared-utils-3.1.0.pom (5.0 kB at 8.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.11/commons-codec-1.11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.11/commons-codec-1.11.pom (14 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.5/slf4j-api-1.7.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.5/slf4j-api-1.7.5.pom (2.7 kB at 6.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.5/slf4j-parent-1.7.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.5/slf4j-parent-1.7.5.pom (12 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-velocity/1.2/plexus-velocity-1.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-velocity/1.2/plexus-velocity-1.2.pom (2.8 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/4.0/plexus-components-4.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/4.0/plexus-components-4.0.pom (2.7 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/4.0/plexus-4.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/4.0/plexus-4.0.pom (22 kB at 27 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/10/forge-parent-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/10/forge-parent-10.pom (14 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.1/commons-collections-3.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.1/commons-collections-3.2.1.pom (13 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/9/commons-parent-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/9/commons-parent-9.pom (22 kB at 33 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.7/velocity-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.7/velocity-1.7.pom (11 kB at 23 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.4/commons-lang-2.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.4/commons-lang-2.4.pom (14 kB at 25 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.3.3/wagon-provider-api-3.3.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.3.3/wagon-provider-api-3.3.3.pom (1.9 kB at 5.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/3.3.3/wagon-3.3.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/3.3.3/wagon-3.3.3.pom (21 kB at 34 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.5.0/maven-archiver-3.5.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.5.0/maven-archiver-3.5.0.pom (4.5 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.0/plexus-archiver-4.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.0/plexus-archiver-4.2.0.pom (4.8 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/3.2.0/plexus-io-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/3.2.0/plexus-io-3.2.0.pom (4.5 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.19/commons-compress-1.19.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.19/commons-compress-1.19.pom (18 kB at 36 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/48/commons-parent-48.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/48/commons-parent-48.pom (72 kB at 73 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/iq80/snappy/snappy/0.4/snappy-0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/iq80/snappy/snappy/0.4/snappy-0.4.pom (15 kB at 29 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.8/xz-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.8/xz-1.8.pom (1.9 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.25/plexus-interpolation-1.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.25/plexus-interpolation-1.25.pom (2.6 kB at 6.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.1/plexus-archiver-4.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.1/plexus-archiver-4.2.1.pom (4.8 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity-api/1.0/plexus-interactivity-api-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity-api/1.0/plexus-interactivity-api-1.0.pom (823 B at 1.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity/1.0/plexus-interactivity-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity/1.0/plexus-interactivity-1.0.pom (1.6 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9-stable-1/plexus-container-default-1.0-alpha-9-stable-1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9-stable-1/plexus-container-default-1.0-alpha-9-stable-1.pom (3.9 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.0.3/plexus-containers-1.0.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.0.3/plexus-containers-1.0.3.pom (492 B at 1.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.4/plexus-1.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.4/plexus-1.0.4.pom (5.7 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.13/junit-4.13.pom
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.13/junit-4.13.pom (25 kB at 47 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.pom (766 B at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-parent/1.3/hamcrest-parent-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-parent/1.3/hamcrest-parent-1.3.pom (2.0 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1-alpha-2/classworlds-1.1-alpha-2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1-alpha-2/classworlds-1.1-alpha-2.pom (3.1 kB at 8.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-script-interpreter/1.0/maven-script-interpreter-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-script-interpreter/1.0/maven-script-interpreter-1.0.pom (3.8 kB at 9.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/17/maven-shared-components-17.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/17/maven-shared-components-17.pom (8.7 kB at 21 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/1.8.3/groovy-1.8.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/1.8.3/groovy-1.8.3.pom (32 kB at 56 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.7/antlr-2.7.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.7/antlr-2.7.7.pom (632 B at 1.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm/3.2/asm-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm/3.2/asm-3.2.pom (264 B at 685 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm-parent/3.2/asm-parent-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm-parent/3.2/asm-parent-3.2.pom (4.4 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm-commons/3.2/asm-commons-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm-commons/3.2/asm-commons-3.2.pom (415 B at 1.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm-tree/3.2/asm-tree-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm-tree/3.2/asm-tree-3.2.pom (404 B at 1.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm-util/3.2/asm-util-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm-util/3.2/asm-util-3.2.pom (409 B at 1.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/asm/asm-analysis/3.2/asm-analysis-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/asm/asm-analysis/3.2/asm-analysis-3.2.pom (417 B at 1.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/beanshell/bsh/2.0b4/bsh-2.0b4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/beanshell/bsh/2.0b4/bsh-2.0b4.pom (1.2 kB at 3.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/beanshell/beanshell/2.0b4/beanshell-2.0b4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/beanshell/beanshell/2.0b4/beanshell-2.0b4.pom (1.4 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant/1.8.1/ant-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant/1.8.1/ant-1.8.1.pom (8.8 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant-parent/1.8.1/ant-parent-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant-parent/1.8.1/ant-parent-1.8.1.pom (4.3 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-catalog/3.2.0/archetype-catalog-3.2.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-descriptor/3.2.0/archetype-descriptor-3.2.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-common/3.2.0/archetype-common-3.2.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/2.4.16/groovy-2.4.16.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/ivy/ivy/2.5.0/ivy-2.5.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-descriptor/3.2.0/archetype-descriptor-3.2.0.jar (24 kB at 41 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/net/sourceforge/jchardet/jchardet/1.0/jchardet-1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-catalog/3.2.0/archetype-catalog-3.2.0.jar (19 kB at 31 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/2.0.0/plexus-component-annotations-2.0.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/2.0.0/plexus-component-annotations-2.0.0.jar (4.2 kB at 4.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jdom/jdom/1.1.3/jdom-1.1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/net/sourceforge/jchardet/jchardet/1.0/jchardet-1.0.jar (27 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/3.0/maven-artifact-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/3.0/maven-artifact-3.0.jar (52 kB at 29 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings-builder/3.0/maven-settings-builder-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings-builder/3.0/maven-settings-builder-3.0.jar (38 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetype/archetype-common/3.2.0/archetype-common-3.2.0.jar (184 kB at 63 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-velocity/1.2/plexus-velocity-1.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.6/commons-io-2.6.jar (215 kB at 54 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.7/velocity-1.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-velocity/1.2/plexus-velocity-1.2.jar (8.1 kB at 1.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.4/commons-lang-2.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.7/velocity-1.7.jar (450 kB at 62 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.3.3/wagon-provider-api-3.3.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.3.3/wagon-provider-api-3.3.3.jar (56 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.5.0/maven-archiver-3.5.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.5.0/maven-archiver-3.5.0.jar (26 kB at 3.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.1/maven-shared-utils-3.2.1.jar (167 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.25/plexus-interpolation-1.25.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.25/plexus-interpolation-1.25.jar (85 kB at 8.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.1/plexus-archiver-4.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/jdom/jdom/1.1.3/jdom-1.1.3.jar (151 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/3.2.0/plexus-io-3.2.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/4.2.1/plexus-archiver-4.2.1.jar (196 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.19/commons-compress-1.19.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.19/commons-compress-1.19.jar (615 kB at 41 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/iq80/snappy/snappy/0.4/snappy-0.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/iq80/snappy/snappy/0.4/snappy-0.4.jar (58 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.8/xz-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.8/xz-1.8.jar (109 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.3.0/plexus-utils-3.3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/3.2.0/plexus-io-3.2.0.jar (76 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity-api/1.0/plexus-interactivity-api-1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interactivity-api/1.0/plexus-interactivity-api-1.0.jar (9.8 kB at 530 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9-stable-1/plexus-container-default-1.0-alpha-9-stable-1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.3.0/plexus-utils-3.3.0.jar (263 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.13/junit-4.13.jar
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.13/junit-4.13.jar (382 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/hamcrest/hamcrest-core/1.3/hamcrest-core-1.3.jar (45 kB at 1.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1-alpha-2/classworlds-1.1-alpha-2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1-alpha-2/classworlds-1.1-alpha-2.jar (38 kB at 1.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/3.0/maven-plugin-api-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/3.0/maven-plugin-api-3.0.jar (49 kB at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-plexus/1.4.2/sisu-inject-plexus-1.4.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-plexus/1.4.2/sisu-inject-plexus-1.4.2.jar (202 kB at 7.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-bean/1.4.2/sisu-inject-bean-1.4.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-inject-bean/1.4.2/sisu-inject-bean-1.4.2.jar (153 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-guice/2.1.7/sisu-guice-2.1.7-noaop.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.4/commons-lang-2.4.jar (262 kB at 8.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/3.0/maven-core-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/sisu/sisu-guice/2.1.7/sisu-guice-2.1.7-noaop.jar (472 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/3.0/maven-repository-metadata-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/3.0/maven-repository-metadata-3.0.jar (30 kB at 987 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model-builder/3.0/maven-model-builder-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model-builder/3.0/maven-model-builder-3.0.jar (148 kB at 4.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-aether-provider/3.0/maven-aether-provider-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-aether-provider/3.0/maven-aether-provider-3.0.jar (51 kB at 1.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-impl/1.7/aether-impl-1.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-impl/1.7/aether-impl-1.7.jar (106 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-spi/1.7/aether-spi-1.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-spi/1.7/aether-spi-1.7.jar (14 kB at 414 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-api/1.7/aether-api-1.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9-stable-1/plexus-container-default-1.0-alpha-9-stable-1.jar (194 kB at 5.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-util/1.7/aether-util-1.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-api/1.7/aether-api-1.7.jar (74 kB at 2.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.2.3/plexus-classworlds-2.2.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.2.3/plexus-classworlds-2.2.3.jar (46 kB at 1.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-sec-dispatcher/1.3/plexus-sec-dispatcher-1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-sec-dispatcher/1.3/plexus-sec-dispatcher-1.3.jar (29 kB at 837 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-cipher/1.4/plexus-cipher-1.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-cipher/1.4/plexus-cipher-1.4.jar (13 kB at 391 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/3.0/maven-model-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/3.0/maven-model-3.0.jar (165 kB at 4.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/3.0/maven-settings-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/3.0/maven-settings-3.0.jar (47 kB at 1.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-invoker/3.0.1/maven-invoker-3.0.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-invoker/3.0.1/maven-invoker-3.0.1.jar (33 kB at 917 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.12.0/maven-artifact-transfer-0.12.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.12.0/maven-artifact-transfer-0.12.0.jar (120 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-common-artifact-filters/3.0.1/maven-common-artifact-filters-3.0.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-common-artifact-filters/3.0.1/maven-common-artifact-filters-3.0.1.jar (61 kB at 1.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.11/commons-codec-1.11.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.11/commons-codec-1.11.jar (335 kB at 8.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.5/slf4j-api-1.7.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.5/slf4j-api-1.7.5.jar (26 kB at 682 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.1/commons-collections-3.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.1/commons-collections-3.2.1.jar (575 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-script-interpreter/1.0/maven-script-interpreter-1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-script-interpreter/1.0/maven-script-interpreter-1.0.jar (21 kB at 523 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/beanshell/bsh/2.0b4/bsh-2.0b4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/beanshell/bsh/2.0b4/bsh-2.0b4.jar (282 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant/1.8.1/ant-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/aether/aether-util/1.7/aether-util-1.7.jar (108 kB at 2.5 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/ant/ant/1.8.1/ant-1.8.1.jar (1.5 MB at 36 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/3.0/maven-core-3.0.jar (527 kB at 8.9 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/ivy/ivy/2.5.0/ivy-2.5.0.jar (1.4 MB at 16 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/groovy/groovy/2.4.16/groovy-2.4.16.jar (4.7 MB at 53 kB/s)
[INFO] Generating project in Batch mode
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-quickstart/1.4/maven-archetype-quickstart-1.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-quickstart/1.4/maven-archetype-quickstart-1.4.pom (1.6 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-bundles/1.4/maven-archetype-bundles-1.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-bundles/1.4/maven-archetype-bundles-1.4.pom (4.5 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-quickstart/1.4/maven-archetype-quickstart-1.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/archetypes/maven-archetype-quickstart/1.4/maven-archetype-quickstart-1.4.jar (7.1 kB at 18 kB/s)
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
[INFO] Project created from Archetype in dir: /root/workspace/my-app
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  05:33 min
[INFO] Finished at: 2021-09-19T08:27:11-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}

执行上面的命令，Maven就会开始下载最新的artifacts到本地仓库。
>注： 上面mvn命令可能要执行多次，因为下载artifacts可能一次不能成功

执行完上面的命令后，你将会看到在workspace目录下产生了一个与```artifactId```名称相同的目录```my-app```，我们进入该目录：
{% highlight string %}
# ls
my-app
# cd my-app
# ls
pom.xml  src
# tree
.
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

11 directories, 3 files

# cat src/main/java/com/mycompany/app/App.java 
package com.mycompany.app;

/**
 * Hello world!
 *
 */
public class App 
{
    public static void main( String[] args )
    {
        System.out.println( "Hello World!" );
    }
}

# cat src/test/java/com/mycompany/app/AppTest.java 
package com.mycompany.app;

import static org.junit.Assert.assertTrue;

import org.junit.Test;

/**
 * Unit test for simple App.
 */
public class AppTest 
{
    /**
     * Rigorous Test :-)
     */
    @Test
    public void shouldAnswerWithTrue()
    {
        assertTrue( true );
    }
}
{% endhighlight %}
上面我们看到的就是一个标准的工程架构。在src/main/java目录包含有工程的源代码，在src/test/java目录包含有测试源代码。而pom.xml就是工程的POM。


#### 1.3.1 POM
pom.xml就是Maven工程的核心配置文件。它是一个单独的配置文件，包含有构建工程所需的大部分信息。整个POM很庞大并且十分复杂，但这里我们并不需要全部了解POM。下面我们来看一下所生成的POM:
{% highlight string %}
<?xml version="1.0" encoding="UTF-8"?>

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
      <plugins>
        <!-- clean lifecycle, see https://maven.apache.org/ref/current/maven-core/lifecycles.html#clean_Lifecycle -->
        <plugin>
          <artifactId>maven-clean-plugin</artifactId>
          <version>3.1.0</version>
        </plugin>
        <!-- default lifecycle, jar packaging: see https://maven.apache.org/ref/current/maven-core/default-bindings.html#Plugin_bindings_for_jar_packaging -->
        <plugin>
          <artifactId>maven-resources-plugin</artifactId>
          <version>3.0.2</version>
        </plugin>
        <plugin>
          <artifactId>maven-compiler-plugin</artifactId>
          <version>3.8.0</version>
        </plugin>
        <plugin>
          <artifactId>maven-surefire-plugin</artifactId>
          <version>2.22.1</version>
        </plugin>
        <plugin>
          <artifactId>maven-jar-plugin</artifactId>
          <version>3.0.2</version>
        </plugin>
        <plugin>
          <artifactId>maven-install-plugin</artifactId>
          <version>2.5.2</version>
        </plugin>
        <plugin>
          <artifactId>maven-deploy-plugin</artifactId>
          <version>2.8.2</version>
        </plugin>
        <!-- site lifecycle, see https://maven.apache.org/ref/current/maven-core/lifecycles.html#site_Lifecycle -->
        <plugin>
          <artifactId>maven-site-plugin</artifactId>
          <version>3.7.1</version>
        </plugin>
        <plugin>
          <artifactId>maven-project-info-reports-plugin</artifactId>
          <version>3.0.0</version>
        </plugin>
      </plugins>
    </pluginManagement>
  </build>
</project>
{% endhighlight %}

#### 1.3.2 What did I just do?

在上面我们执行了Maven的一个目标(goal)： ```archetype:generate```，并且在执行该目标时向其传递了很多参数。前缀```archetype```就是提供该目标的plugin。假如你对Ant比较熟悉的话，你可能会认为这类似于一项任务。该```archetype:generate```目标会基于``` maven-archetype-quickstart```架构类型创建一个简单的工程。现在我们可以说：一个plugin就是goals的集合，用于实现某种通用目标。例如，对于```jboss-maven-plugin```来说，其目的就是为了“deal with various jboss items”.

#### 1.3.3 构建工程
执行如下命令构建工程：
{% highlight string %}
# mvn package
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/3.0.2/maven-resources-plugin-3.0.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/3.0.2/maven-resources-plugin-3.0.2.pom (7.1 kB at 1.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/30/maven-plugins-30.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/30/maven-plugins-30.pom (10 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/3.0.2/maven-resources-plugin-3.0.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-resources-plugin/3.0.2/maven-resources-plugin-3.0.2.jar (32 kB at 8.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.8.0/maven-compiler-plugin-3.8.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.8.0/maven-compiler-plugin-3.8.0.pom (12 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/32/maven-plugins-32.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/32/maven-plugins-32.pom (11 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/32/maven-parent-32.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/32/maven-parent-32.pom (43 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/20/apache-20.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/20/apache-20.pom (16 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.8.0/maven-compiler-plugin-3.8.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-compiler-plugin/3.8.0/maven-compiler-plugin-3.8.0.jar (62 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.22.1/maven-surefire-plugin-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.22.1/maven-surefire-plugin-2.22.1.pom (5.0 kB at 8.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire/2.22.1/surefire-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire/2.22.1/surefire-2.22.1.pom (26 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.22.1/maven-surefire-plugin-2.22.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-surefire-plugin/2.22.1/maven-surefire-plugin-2.22.1.jar (41 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/3.0.2/maven-jar-plugin-3.0.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/3.0.2/maven-jar-plugin-3.0.2.pom (6.2 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/3.0.2/maven-jar-plugin-3.0.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-jar-plugin/3.0.2/maven-jar-plugin-3.0.2.jar (27 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.11/junit-4.11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.11/junit-4.11.pom (2.3 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.11/junit-4.11.jar
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.11/junit-4.11.jar (245 kB at 14 kB/s)
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:resources (default-resources) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.4/plexus-utils-2.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.4/plexus-utils-2.0.4.pom (3.3 kB at 7.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.6/plexus-component-annotations-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.6/plexus-component-annotations-1.6.pom (748 B at 886 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.6/plexus-containers-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.6/plexus-containers-1.6.pom (3.8 kB at 8.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.3.2/plexus-3.3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.3.2/plexus-3.3.2.pom (22 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/17/spice-parent-17.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/17/spice-parent-17.pom (6.8 kB at 6.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.5/plexus-utils-2.0.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.5/plexus-utils-2.0.5.pom (3.3 kB at 4.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.5/plexus-utils-1.5.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.5/plexus-utils-1.5.5.pom (5.1 kB at 6.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.11/plexus-1.0.11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.11/plexus-1.0.11.pom (9.0 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.24/plexus-utils-3.0.24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.24/plexus-utils-3.0.24.pom (4.1 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-filtering/3.1.1/maven-filtering-3.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-filtering/3.1.1/maven-filtering-3.1.1.pom (5.7 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.0/maven-shared-utils-3.0.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.0/maven-shared-utils-3.0.0.pom (5.6 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/21/maven-shared-components-21.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/21/maven-shared-components-21.pom (5.1 kB at 2.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/25/maven-parent-25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/25/maven-parent-25.pom (37 kB at 9.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/15/apache-15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/15/apache-15.pom (15 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.4/commons-io-2.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.4/commons-io-2.4.pom (10 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/25/commons-parent-25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/25/commons-parent-25.pom (48 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/9/apache-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/9/apache-9.pom (15 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/2.0.1/jsr305-2.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/2.0.1/jsr305-2.0.1.pom (965 B at 2.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.22/plexus-interpolation-1.22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.22/plexus-interpolation-1.22.pom (1.5 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.3.1/plexus-components-1.3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.3.1/plexus-components-1.3.1.pom (3.1 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.3.1/plexus-3.3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.3.1/plexus-3.3.1.pom (20 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-build-api/0.0.7/plexus-build-api-0.0.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-build-api/0.0.7/plexus-build-api-0.0.7.pom (3.2 kB at 2.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/15/spice-parent-15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/spice/spice-parent/15/spice-parent-15.pom (8.4 kB at 5.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/5/forge-parent-5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/forge/forge-parent/5/forge-parent-5.pom (8.4 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.8/plexus-utils-1.5.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.8/plexus-utils-1.5.8.pom (8.1 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.2/plexus-2.0.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/2.0.2/plexus-2.0.2.pom (12 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.pom (13 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/39/commons-parent-39.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/39/commons-parent-39.pom (62 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/16/apache-16.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/16/apache-16.pom (15 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.24/plexus-interpolation-1.24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.24/plexus-interpolation-1.24.pom (2.6 kB at 5.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.6/plexus-component-annotations-1.6.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.24/plexus-utils-3.0.24.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-filtering/3.1.1/maven-filtering-3.1.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.0/maven-shared-utils-3.0.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/2.0.1/jsr305-2.0.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.6/plexus-component-annotations-1.6.jar (4.3 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-build-api/0.0.7/plexus-build-api-0.0.7.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/code/findbugs/jsr305/2.0.1/jsr305-2.0.1.jar (32 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/plexus/plexus-build-api/0.0.7/plexus-build-api-0.0.7.jar (8.5 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.24/plexus-interpolation-1.24.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-filtering/3.1.1/maven-filtering-3.1.1.jar (51 kB at 24 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.5/commons-io-2.5.jar (209 kB at 82 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.0/maven-shared-utils-3.0.0.jar (155 kB at 54 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.24/plexus-utils-3.0.24.jar (247 kB at 73 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.24/plexus-interpolation-1.24.jar (79 kB at 12 kB/s)
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/workspace/my-app/src/main/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:compile (default-compile) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.7.1/plexus-component-annotations-1.7.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.7.1/plexus-component-annotations-1.7.1.pom (770 B at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.7.1/plexus-containers-1.7.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.7.1/plexus-containers-1.7.1.pom (5.0 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-incremental/1.1/maven-shared-incremental-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-incremental/1.1/maven-shared-incremental-1.1.pom (4.7 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/19/maven-shared-components-19.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/19/maven-shared-components-19.pom (6.4 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/2.2.1/maven-plugin-api-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/2.2.1/maven-plugin-api-2.2.1.pom (1.5 kB at 1.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven/2.2.1/maven-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven/2.2.1/maven-2.2.1.pom (22 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/11/maven-parent-11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/11/maven-parent-11.pom (32 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/5/apache-5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/5/apache-5.pom (4.1 kB at 2.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/2.2.1/maven-core-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/2.2.1/maven-core-2.2.1.pom (12 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/2.2.1/maven-settings-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/2.2.1/maven-settings-2.2.1.pom (2.2 kB at 5.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/2.2.1/maven-model-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/2.2.1/maven-model-2.2.1.pom (3.2 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.15/plexus-utils-1.5.15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.15/plexus-utils-1.5.15.pom (6.8 kB at 8.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.11/plexus-interpolation-1.11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.11/plexus-interpolation-1.11.pom (889 B at 830 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.14/plexus-components-1.1.14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.14/plexus-components-1.1.14.pom (5.8 kB at 3.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-parameter-documenter/2.2.1/maven-plugin-parameter-documenter-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-parameter-documenter/2.2.1/maven-plugin-parameter-documenter-2.2.1.pom (2.0 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-jdk14/1.5.6/slf4j-jdk14-1.5.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-jdk14/1.5.6/slf4j-jdk14-1.5.6.pom (1.9 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.5.6/slf4j-parent-1.5.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.5.6/slf4j-parent-1.5.6.pom (7.9 kB at 9.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.5.6/slf4j-api-1.5.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.5.6/slf4j-api-1.5.6.pom (3.0 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/jcl-over-slf4j/1.5.6/jcl-over-slf4j-1.5.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/jcl-over-slf4j/1.5.6/jcl-over-slf4j-1.5.6.pom (2.2 kB at 2.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-profile/2.2.1/maven-profile-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-profile/2.2.1/maven-profile-2.2.1.pom (2.2 kB at 2.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/2.2.1/maven-artifact-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/2.2.1/maven-artifact-2.2.1.pom (1.6 kB at 1.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/2.2.1/maven-repository-metadata-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/2.2.1/maven-repository-metadata-2.2.1.pom (1.9 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-error-diagnostics/2.2.1/maven-error-diagnostics-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-error-diagnostics/2.2.1/maven-error-diagnostics-2.2.1.pom (1.7 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-project/2.2.1/maven-project-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-project/2.2.1/maven-project-2.2.1.pom (2.8 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact-manager/2.2.1/maven-artifact-manager-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact-manager/2.2.1/maven-artifact-manager-2.2.1.pom (3.1 kB at 6.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/backport-util-concurrent/backport-util-concurrent/3.1/backport-util-concurrent-3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/backport-util-concurrent/backport-util-concurrent/3.1/backport-util-concurrent-3.1.pom (880 B at 2.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-registry/2.2.1/maven-plugin-registry-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-registry/2.2.1/maven-plugin-registry-2.2.1.pom (1.9 kB at 4.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-descriptor/2.2.1/maven-plugin-descriptor-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-descriptor/2.2.1/maven-plugin-descriptor-2.2.1.pom (2.1 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-monitor/2.2.1/maven-monitor-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-monitor/2.2.1/maven-monitor-2.2.1.pom (1.3 kB at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.1/maven-shared-utils-0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.1/maven-shared-utils-0.1.pom (4.0 kB at 5.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/18/maven-shared-components-18.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/18/maven-shared-components-18.pom (4.9 kB at 8.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-java/0.9.10/plexus-java-0.9.10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-java/0.9.10/plexus-java-0.9.10.pom (5.1 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-languages/0.9.10/plexus-languages-0.9.10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-languages/0.9.10/plexus-languages-0.9.10.pom (4.1 kB at 5.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/ow2/asm/asm/6.2/asm-6.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/ow2/asm/asm/6.2/asm-6.2.pom (2.9 kB at 2.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/ow2/ow2/1.5/ow2-1.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/ow2/ow2/1.5/ow2-1.5.pom (11 kB at 7.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M9/qdox-2.0-M9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M9/qdox-2.0-M9.pom (16 kB at 9.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/sonatype/oss/oss-parent/9/oss-parent-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/sonatype/oss/oss-parent/9/oss-parent-9.pom (6.6 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-api/2.8.4/plexus-compiler-api-2.8.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-api/2.8.4/plexus-compiler-api-2.8.4.pom (867 B at 2.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler/2.8.4/plexus-compiler-2.8.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler/2.8.4/plexus-compiler-2.8.4.pom (6.0 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.22/plexus-utils-3.0.22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.22/plexus-utils-3.0.22.pom (3.8 kB at 8.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-manager/2.8.4/plexus-compiler-manager-2.8.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-manager/2.8.4/plexus-compiler-manager-2.8.4.pom (692 B at 1.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-javac/2.8.4/plexus-compiler-javac-2.8.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-javac/2.8.4/plexus-compiler-javac-2.8.4.pom (771 B at 1.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compilers/2.8.4/plexus-compilers-2.8.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compilers/2.8.4/plexus-compilers-2.8.4.pom (1.3 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.4/plexus-utils-2.0.4.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.14/plexus-interpolation-1.14.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.7.1/plexus-component-annotations-1.7.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-incremental/1.1/maven-shared-incremental-1.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-java/0.9.10/plexus-java-0.9.10.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-incremental/1.1/maven-shared-incremental-1.1.jar (14 kB at 29 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/ow2/asm/asm/6.2/asm-6.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-component-annotations/1.7.1/plexus-component-annotations-1.7.1.jar (4.3 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M9/qdox-2.0-M9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-java/0.9.10/plexus-java-0.9.10.jar (39 kB at 85 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-api/2.8.4/plexus-compiler-api-2.8.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-api/2.8.4/plexus-compiler-api-2.8.4.jar (27 kB at 27 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-manager/2.8.4/plexus-compiler-manager-2.8.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/ow2/asm/asm/6.2/asm-6.2.jar (111 kB at 72 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-javac/2.8.4/plexus-compiler-javac-2.8.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-javac/2.8.4/plexus-compiler-javac-2.8.4.jar (21 kB at 8.6 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-compiler-manager/2.8.4/plexus-compiler-manager-2.8.4.jar (4.7 kB at 1.7 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M9/qdox-2.0-M9.jar (317 kB at 105 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.14/plexus-interpolation-1.14.jar (61 kB at 16 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/2.0.4/plexus-utils-2.0.4.jar (222 kB at 18 kB/s)
[INFO] Changes detected - recompiling the module!
[INFO] Compiling 1 source file to /root/workspace/my-app/target/classes
[INFO] 
[INFO] --- maven-resources-plugin:3.0.2:testResources (default-testResources) @ my-app ---
[INFO] Using 'UTF-8' encoding to copy filtered resources.
[INFO] skip non existing resourceDirectory /root/workspace/my-app/src/test/resources
[INFO] 
[INFO] --- maven-compiler-plugin:3.8.0:testCompile (default-testCompile) @ my-app ---
[INFO] Changes detected - recompiling the module!
[INFO] Compiling 1 source file to /root/workspace/my-app/target/test-classes
[INFO] 
[INFO] --- maven-surefire-plugin:2.22.1:test (default-test) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/maven-surefire-common/2.22.1/maven-surefire-common-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/maven-surefire-common/2.22.1/maven-surefire-common-2.22.1.pom (11 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-annotations/3.5.2/maven-plugin-annotations-3.5.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-annotations/3.5.2/maven-plugin-annotations-3.5.2.pom (1.6 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-tools/3.5.2/maven-plugin-tools-3.5.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-tools/3.5.2/maven-plugin-tools-3.5.2.pom (15 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-api/2.22.1/surefire-api-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-api/2.22.1/surefire-api-2.22.1.pom (3.5 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-logger-api/2.22.1/surefire-logger-api-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-logger-api/2.22.1/surefire-logger-api-2.22.1.pom (2.0 kB at 3.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-booter/2.22.1/surefire-booter-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-booter/2.22.1/surefire-booter-2.22.1.pom (7.5 kB at 8.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.pom (24 kB at 8.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.0.4/plexus-utils-1.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.0.4/plexus-utils-1.0.4.pom (6.9 kB at 7.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-api/3.0/maven-reporting-api-3.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-api/3.0/maven-reporting-api-3.0.pom (2.4 kB at 2.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/15/maven-shared-components-15.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/15/maven-shared-components-15.pom (9.3 kB at 10.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/16/maven-parent-16.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/16/maven-parent-16.pom (23 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1/classworlds-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1/classworlds-1.1.pom (3.3 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-toolchain/2.2.1/maven-toolchain-2.2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-toolchain/2.2.1/maven-toolchain-2.2.1.pom (3.3 kB at 7.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M8/qdox-2.0-M8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M8/qdox-2.0-M8.pom (16 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/maven-surefire-common/2.22.1/maven-surefire-common-2.22.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/2.2.1/maven-plugin-api-2.2.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-annotations/3.5.2/maven-plugin-annotations-3.5.2.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-api/2.22.1/surefire-api-2.22.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-logger-api/2.22.1/surefire-logger-api-2.22.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-logger-api/2.22.1/surefire-logger-api-2.22.1.jar (13 kB at 27 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-booter/2.22.1/surefire-booter-2.22.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugin-tools/maven-plugin-annotations/3.5.2/maven-plugin-annotations-3.5.2.jar (14 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/2.2.1/maven-artifact-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-api/2.22.1/surefire-api-2.22.1.jar (186 kB at 182 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.15/plexus-utils-1.5.15.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-api/2.2.1/maven-plugin-api-2.2.1.jar (12 kB at 9.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-descriptor/2.2.1/maven-plugin-descriptor-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact/2.2.1/maven-artifact-2.2.1.jar (80 kB at 44 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.5.15/plexus-utils-1.5.15.jar (228 kB at 70 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-project/2.2.1/maven-project-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/4.12/junit-4.12.jar (315 kB at 84 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/2.2.1/maven-settings-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-project/2.2.1/maven-project-2.2.1.jar (156 kB at 38 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-profile/2.2.1/maven-profile-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-descriptor/2.2.1/maven-plugin-descriptor-2.2.1.jar (39 kB at 8.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact-manager/2.2.1/maven-artifact-manager-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-profile/2.2.1/maven-profile-2.2.1.jar (35 kB at 7.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/backport-util-concurrent/backport-util-concurrent/3.1/backport-util-concurrent-3.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-settings/2.2.1/maven-settings-2.2.1.jar (49 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-registry/2.2.1/maven-plugin-registry-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/backport-util-concurrent/backport-util-concurrent/3.1/backport-util-concurrent-3.1.jar (332 kB at 53 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.11/plexus-interpolation-1.11.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-registry/2.2.1/maven-plugin-registry-2.2.1.jar (30 kB at 4.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/2.2.1/maven-model-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-interpolation/1.11/plexus-interpolation-1.11.jar (51 kB at 7.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/2.2.1/maven-core-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-model/2.2.1/maven-model-2.2.1.jar (88 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-parameter-documenter/2.2.1/maven-plugin-parameter-documenter-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-core/2.2.1/maven-core-2.2.1.jar (178 kB at 23 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-jdk14/1.5.6/slf4j-jdk14-1.5.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-jdk14/1.5.6/slf4j-jdk14-1.5.6.jar (8.8 kB at 1.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.5.6/slf4j-api-1.5.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-plugin-parameter-documenter/2.2.1/maven-plugin-parameter-documenter-2.2.1.jar (22 kB at 2.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/jcl-over-slf4j/1.5.6/jcl-over-slf4j-1.5.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.5.6/slf4j-api-1.5.6.jar (22 kB at 2.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-api/3.0/maven-reporting-api-3.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-api/3.0/maven-reporting-api-3.0.jar (11 kB at 1.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/2.2.1/maven-repository-metadata-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/jcl-over-slf4j/1.5.6/jcl-over-slf4j-1.5.6.jar (17 kB at 1.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-error-diagnostics/2.2.1/maven-error-diagnostics-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-artifact-manager/2.2.1/maven-artifact-manager-2.2.1.jar (68 kB at 6.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-monitor/2.2.1/maven-monitor-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-error-diagnostics/2.2.1/maven-error-diagnostics-2.2.1.jar (13 kB at 1.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1/classworlds-1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-repository-metadata/2.2.1/maven-repository-metadata-2.2.1.jar (26 kB at 2.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-toolchain/2.2.1/maven-toolchain-2.2.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-monitor/2.2.1/maven-monitor-2.2.1.jar (10 kB at 941 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M8/qdox-2.0-M8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-toolchain/2.2.1/maven-toolchain-2.2.1.jar (38 kB at 3.2 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/classworlds/classworlds/1.1/classworlds-1.1.jar (38 kB at 3.1 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-booter/2.22.1/surefire-booter-2.22.1.jar (274 kB at 14 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/com/thoughtworks/qdox/qdox/2.0-M8/qdox-2.0-M8.jar (316 kB at 11 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/maven-surefire-common/2.22.1/maven-surefire-common-2.22.1.jar (528 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-junit4/2.22.1/surefire-junit4-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-junit4/2.22.1/surefire-junit4-2.22.1.pom (3.1 kB at 4.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-providers/2.22.1/surefire-providers-2.22.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-providers/2.22.1/surefire-providers-2.22.1.pom (2.5 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-junit4/2.22.1/surefire-junit4-2.22.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/surefire/surefire-junit4/2.22.1/surefire-junit4-2.22.1.jar (85 kB at 14 kB/s)
[INFO] 
[INFO] -------------------------------------------------------
[INFO]  T E S T S
[INFO] -------------------------------------------------------
[INFO] Running com.mycompany.app.AppTest
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0, Time elapsed: 0.091 s - in com.mycompany.app.AppTest
[INFO] 
[INFO] Results:
[INFO] 
[INFO] Tests run: 1, Failures: 0, Errors: 0, Skipped: 0
[INFO] 
[INFO] 
[INFO] --- maven-jar-plugin:3.0.2:jar (default-jar) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.1.1/maven-archiver-3.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.1.1/maven-archiver-3.1.1.pom (4.3 kB at 6.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.1/maven-shared-utils-3.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.1/maven-shared-utils-3.0.1.pom (4.6 kB at 7.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.3/plexus-archiver-3.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.3/plexus-archiver-3.3.pom (5.3 kB at 9.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.2/plexus-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/3.2/plexus-3.2.pom (19 kB at 21 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/2.7.1/plexus-io-2.7.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/2.7.1/plexus-io-2.7.1.pom (4.9 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.2/commons-io-2.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.2/commons-io-2.2.pom (11 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/24/commons-parent-24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/24/commons-parent-24.pom (47 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.11/commons-compress-1.11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.11/commons-compress-1.11.pom (13 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.5/xz-1.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.5/xz-1.5.pom (1.9 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.4/plexus-archiver-3.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.4/plexus-archiver-3.4.pom (5.3 kB at 2.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.1.1/maven-archiver-3.1.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.1/maven-shared-utils-3.0.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.4/plexus-archiver-3.4.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/2.7.1/plexus-io-2.7.1.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.11/commons-compress-1.11.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-io/2.7.1/plexus-io-2.7.1.jar (86 kB at 61 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.5/xz-1.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-archiver/3.1.1/maven-archiver-3.1.1.jar (24 kB at 15 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/tukaani/xz/1.5/xz-1.5.jar (100 kB at 33 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-compress/1.11/commons-compress-1.11.jar (426 kB at 79 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.0.1/maven-shared-utils-3.0.1.jar (154 kB at 21 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-archiver/3.4/plexus-archiver-3.4.jar (187 kB at 16 kB/s)
[INFO] Building jar: /root/workspace/my-app/target/my-app-1.0-SNAPSHOT.jar
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  03:40 min
[INFO] Finished at: 2021-09-19T09:00:05-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}
从上面的打印我们可以看出，执行了很多操作。上面执行完成后，我们来看一下生成了哪些东西：
<pre>
# tree
.
├── pom.xml
├── src
│   ├── main
│   │   └── java
│   │       └── com
│   │           └── mycompany
│   │               └── app
│   │                   └── App.java
│   └── test
│       └── java
│           └── com
│               └── mycompany
│                   └── app
│                       └── AppTest.java
└── target
    ├── classes
    │   └── com
    │       └── mycompany
    │           └── app
    │               └── App.class
    ├── generated-sources
    │   └── annotations
    ├── generated-test-sources
    │   └── test-annotations
    ├── maven-archiver
    │   └── pom.properties
    ├── maven-status
    │   └── maven-compiler-plugin
    │       ├── compile
    │       │   └── default-compile
    │       │       ├── createdFiles.lst
    │       │       └── inputFiles.lst
    │       └── testCompile
    │           └── default-testCompile
    │               ├── createdFiles.lst
    │               └── inputFiles.lst
    ├── my-app-1.0-SNAPSHOT.jar
    ├── surefire-reports
    │   ├── com.mycompany.app.AppTest.txt
    │   └── TEST-com.mycompany.app.AppTest.xml
    └── test-classes
        └── com
            └── mycompany
                └── app
                    └── AppTest.class

32 directories, 13 files
</pre>

与前面执行的第一个命令(archetype:generate)不同，这里只是一个简单的单词```package```。这是一个build phase，而不是goal。build lifecycle是一个有序的phase集合。当在命令行指定一个phase的时候，maven就会按顺序从前到后依次执行到指定phase为止。例如，当你执行```compile``` phase时，那么实际会被执行到的phases有：
<pre>
1: validate
2: generate-sources
3: process-sources
4: generate-resources
5: process-resources
6: compile
</pre>

现在我们可以测试一下上面新编译和打包的Jar，执行如下命令：
<pre>
# java -cp target/my-app-1.0-SNAPSHOT.jar com.mycompany.app.App
Hello World!
</pre>


## 2. Java 9 or later
默认情况下，Maven可能会使用一个老版本的```maven-compiler-plugin```，其并不兼容Java9及之后的JDK版本。为了兼容Java9及之后的JDK版本，我们必须至少使用```v3.6.0```版本之后的```maven-compiler-plugin```，并且设置```maven.compiler.release```属性为你想发布到的目标Java版本（eg: 9、10、11、12等）

下面的例子中，我们配置Maven工程使用3.8.1版本的```maven-compiler-plugin```，然后设置目标发布Java版本为Java 11:
{% highlight string %}
    <properties>
        <maven.compiler.release>11</maven.compiler.release>
    </properties>
 
    <build>
        <pluginManagement>
            <plugins>
                <plugin>
                    <groupId>org.apache.maven.plugins</groupId>
                    <artifactId>maven-compiler-plugin</artifactId>
                    <version>3.8.1</version>
                </plugin>
            </plugins>
        </pluginManagement>
    </build>
{% endhighlight %}

## 3. Running Maven Tools

### 3.1 Maven Phases
Maven Phases有很多，如下是通常默认所执行的phase:

* validate: 校验project是否正确，以及其他必要的信息是否可用

* compile: 编译工程源代码

* test: 使用单元测试框架来测试编译的源代码。这些测试并不需要源码被打包或部署

* package: 将编译好的源代码打包成指定的发布格式（例如： Jar）

* integration-test: 进行集成测试。假如有需要的话，会将编译出的源代码进行部署

* verify: 校验编译出的包(package)有效，并且满足质量准则

* install: 将包(package)安装到本地仓库，使得本地的其他工程能够使用其作为依赖

* deploy: 通常在集成或者发布环境中，我们会执行deploy将最后生成的包(package)发布到远程仓库以供他人使用。


另外还有两个其他的Maven lifecycles没有在上述默认列表中给出：

* clean: 清除前面构建所尝试的artifacts

* site: 为工程产生site文档信息

Phases其实最终还是会映射到底层的goals。每个phase所执行的特定的goals取决于工程所打包的类型。例如，当工程类型为JAR时，package执行的是```jar:jar```； 当工程类型是WAR时，package执行的时```war:war```。

另一个有趣的点是phases和goals可以按顺序执行，例如：
<pre>
# mvn clean dependency:copy-dependencies package
</pre>
上面的命令首先会```clean```整个工程，然后拷贝dependencies，之后再pakcage工程。

### 3. Generating the site
{% highlight string %}
# mvn site
[INFO] Scanning for projects...
[INFO] 
[INFO] ----------------------< com.mycompany.app:my-app >----------------------
[INFO] Building my-app 1.0-SNAPSHOT
[INFO] --------------------------------[ jar ]---------------------------------
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.7.1/maven-site-plugin-3.7.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.7.1/maven-site-plugin-3.7.1.pom (19 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/31/maven-plugins-31.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/31/maven-plugins-31.pom (10 kB at 27 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.7.1/maven-site-plugin-3.7.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-site-plugin/3.7.1/maven-site-plugin-3.7.1.jar (135 kB at 181 kB/s)
[INFO] 
[INFO] --- maven-site-plugin:3.7.1:site (default-site) @ my-app ---
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-exec/1.4/maven-reporting-exec-1.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-exec/1.4/maven-reporting-exec-1.4.pom (12 kB at 31 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.0/doxia-sink-api-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.0/doxia-sink-api-1.0.pom (1.4 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.0/doxia-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.0/doxia-1.0.pom (9.6 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/10/maven-parent-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/10/maven-parent-10.pom (32 kB at 82 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.3/maven-shared-utils-0.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.3/maven-shared-utils-0.3.pom (4.0 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether-util/0.9.0.M2/aether-util-0.9.0.M2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether-util/0.9.0.M2/aether-util-0.9.0.M2.pom (2.0 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether/0.9.0.M2/aether-0.9.0.M2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether/0.9.0.M2/aether-0.9.0.M2.pom (28 kB at 74 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-10/plexus-i18n-1.0-beta-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-10/plexus-i18n-1.0-beta-10.pom (2.1 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.12/plexus-components-1.1.12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.12/plexus-components-1.1.12.pom (3.0 kB at 8.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.10/plexus-1.0.10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.10/plexus-1.0.10.pom (8.2 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.5/plexus-utils-1.4.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.5/plexus-utils-1.4.5.pom (2.3 kB at 5.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.2/plexus-classworlds-2.5.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.2/plexus-classworlds-2.5.2.pom (7.3 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.8/doxia-sink-api-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.8/doxia-sink-api-1.8.pom (1.5 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.8/doxia-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.8/doxia-1.8.pom (18 kB at 48 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.8/doxia-logging-api-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.8/doxia-logging-api-1.8.pom (1.5 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.7.1/plexus-container-default-1.7.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.7.1/plexus-container-default-1.7.1.pom (2.8 kB at 7.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.20/plexus-utils-3.0.20.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.0.20/plexus-utils-3.0.20.pom (3.8 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.1/plexus-classworlds-2.5.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.1/plexus-classworlds-2.5.1.pom (5.0 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/xbean/xbean-reflect/3.7/xbean-reflect-3.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/xbean/xbean-reflect/3.7/xbean-reflect-3.7.pom (5.1 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/xbean/xbean/3.7/xbean-3.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/xbean/xbean/3.7/xbean-3.7.pom (15 kB at 41 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis-java5-flava/2.0/genesis-java5-flava-2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis-java5-flava/2.0/genesis-java5-flava-2.0.pom (5.5 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis-default-flava/2.0/genesis-default-flava-2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis-default-flava/2.0/genesis-default-flava-2.0.pom (18 kB at 47 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis/2.0/genesis-2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/geronimo/genesis/genesis/2.0/genesis-2.0.pom (18 kB at 49 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/collections/google-collections/1.0/google-collections-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/collections/google-collections/1.0/google-collections-1.0.pom (2.5 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/google/google/1/google-1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/google/google/1/google-1.pom (1.6 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.8/doxia-module-xhtml-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.8/doxia-module-xhtml-1.8.pom (1.9 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-modules/1.8/doxia-modules-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-modules/1.8/doxia-modules-1.8.pom (2.6 kB at 7.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.1.0/plexus-utils-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.1.0/plexus-utils-3.1.0.pom (4.7 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.8/doxia-core-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.8/doxia-core-1.8.pom (4.2 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.5/commons-lang3-3.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.5/commons-lang3-3.5.pom (23 kB at 61 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/41/commons-parent-41.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/41/commons-parent-41.pom (65 kB at 145 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.0.2/httpclient-4.0.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.0.2/httpclient-4.0.2.pom (7.5 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-client/4.0.2/httpcomponents-client-4.0.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-client/4.0.2/httpcomponents-client-4.0.2.pom (9.0 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/project/4.1/project-4.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/project/4.1/project-4.1.pom (16 kB at 42 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.0.1/httpcore-4.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.0.1/httpcore-4.0.1.pom (4.9 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-core/4.0.1/httpcomponents-core-4.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-core/4.0.1/httpcomponents-core-4.0.1.pom (9.4 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/project/4.0/project-4.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/project/4.0/project-4.0.pom (13 kB at 35 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1.1/commons-logging-1.1.1.pom (18 kB at 47 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/5/commons-parent-5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/5/commons-parent-5.pom (16 kB at 43 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.3/commons-codec-1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.3/commons-codec-1.3.pom (6.1 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-apt/1.8/doxia-module-apt-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-apt/1.8/doxia-module-apt-1.8.pom (2.0 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xdoc/1.8/doxia-module-xdoc-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xdoc/1.8/doxia-module-xdoc-1.8.pom (4.7 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-fml/1.8/doxia-module-fml-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-fml/1.8/doxia-module-fml-1.8.pom (4.6 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-markdown/1.8/doxia-module-markdown-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-markdown/1.8/doxia-module-markdown-1.8.pom (2.4 kB at 6.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-all/0.18.4/flexmark-all-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-all/0.18.4/flexmark-all-0.18.4.pom (7.4 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-java/0.18.4/flexmark-java-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-java/0.18.4/flexmark-java-0.18.4.pom (17 kB at 43 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark/0.18.4/flexmark-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark/0.18.4/flexmark-0.18.4.pom (2.3 kB at 6.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-util/0.18.4/flexmark-util-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-util/0.18.4/flexmark-util-0.18.4.pom (791 B at 2.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-abbreviation/0.18.4/flexmark-ext-abbreviation-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-abbreviation/0.18.4/flexmark-ext-abbreviation-0.18.4.pom (1.8 kB at 4.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-autolink/0.18.4/flexmark-ext-autolink-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-autolink/0.18.4/flexmark-ext-autolink-0.18.4.pom (1.6 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/nibor/autolink/autolink/0.6.0/autolink-0.6.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/nibor/autolink/autolink/0.6.0/autolink-0.6.0.pom (9.2 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-formatter/0.18.4/flexmark-formatter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-formatter/0.18.4/flexmark-formatter-0.18.4.pom (1.1 kB at 3.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-typographic/0.18.4/flexmark-ext-typographic-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-typographic/0.18.4/flexmark-ext-typographic-0.18.4.pom (1.3 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-anchorlink/0.18.4/flexmark-ext-anchorlink-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-anchorlink/0.18.4/flexmark-ext-anchorlink-0.18.4.pom (1.6 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-aside/0.18.4/flexmark-ext-aside-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-aside/0.18.4/flexmark-ext-aside-0.18.4.pom (1.5 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-jira-converter/0.18.4/flexmark-jira-converter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-jira-converter/0.18.4/flexmark-jira-converter-0.18.4.pom (2.1 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-strikethrough/0.18.4/flexmark-ext-gfm-strikethrough-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-strikethrough/0.18.4/flexmark-ext-gfm-strikethrough-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-tables/0.18.4/flexmark-ext-tables-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-tables/0.18.4/flexmark-ext-tables-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-wikilink/0.18.4/flexmark-ext-wikilink-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-wikilink/0.18.4/flexmark-ext-wikilink-0.18.4.pom (1.3 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-ins/0.18.4/flexmark-ext-ins-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-ins/0.18.4/flexmark-ext-ins-0.18.4.pom (1.3 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-superscript/0.18.4/flexmark-ext-superscript-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-superscript/0.18.4/flexmark-ext-superscript-0.18.4.pom (1.3 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-definition/0.18.4/flexmark-ext-definition-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-definition/0.18.4/flexmark-ext-definition-0.18.4.pom (1.3 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-emoji/0.18.4/flexmark-ext-emoji-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-emoji/0.18.4/flexmark-ext-emoji-0.18.4.pom (1.5 kB at 2.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-escaped-character/0.18.4/flexmark-ext-escaped-character-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-escaped-character/0.18.4/flexmark-ext-escaped-character-0.18.4.pom (1.3 kB at 3.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-footnotes/0.18.4/flexmark-ext-footnotes-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-footnotes/0.18.4/flexmark-ext-footnotes-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tables/0.18.4/flexmark-ext-gfm-tables-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tables/0.18.4/flexmark-ext-gfm-tables-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tasklist/0.18.4/flexmark-ext-gfm-tasklist-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tasklist/0.18.4/flexmark-ext-gfm-tasklist-0.18.4.pom (1.4 kB at 3.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-front-matter/0.18.4/flexmark-ext-jekyll-front-matter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-front-matter/0.18.4/flexmark-ext-jekyll-front-matter-0.18.4.pom (1.5 kB at 3.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-yaml-front-matter/0.18.4/flexmark-ext-yaml-front-matter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-yaml-front-matter/0.18.4/flexmark-ext-yaml-front-matter-0.18.4.pom (1.3 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-tag/0.18.4/flexmark-ext-jekyll-tag-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-tag/0.18.4/flexmark-ext-jekyll-tag-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-xwiki-macros/0.18.4/flexmark-ext-xwiki-macros-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-xwiki-macros/0.18.4/flexmark-ext-xwiki-macros-0.18.4.pom (1.3 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-spec-example/0.18.4/flexmark-ext-spec-example-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-spec-example/0.18.4/flexmark-ext-spec-example-0.18.4.pom (1.3 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-test-util/0.18.4/flexmark-test-util-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-test-util/0.18.4/flexmark-test-util-0.18.4.pom (917 B at 2.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-toc/0.18.4/flexmark-ext-toc-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-toc/0.18.4/flexmark-ext-toc-0.18.4.pom (1.3 kB at 3.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-html-parser/0.18.4/flexmark-html-parser-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-html-parser/0.18.4/flexmark-html-parser-0.18.4.pom (1.5 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.10.2/jsoup-1.10.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.10.2/jsoup-1.10.2.pom (7.3 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-pdf-converter/0.18.4/flexmark-pdf-converter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-pdf-converter/0.18.4/flexmark-pdf-converter-0.18.4.pom (3.2 kB at 7.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-core/0.0.1-RC9/openhtmltopdf-core-0.0.1-RC9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-core/0.0.1-RC9/openhtmltopdf-core-0.0.1-RC9.pom (1.7 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-parent/0.0.1-RC9/openhtmltopdf-parent-0.0.1-RC9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-parent/0.0.1-RC9/openhtmltopdf-parent-0.0.1-RC9.pom (4.3 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-pdfbox/0.0.1-RC9/openhtmltopdf-pdfbox-0.0.1-RC9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-pdfbox/0.0.1-RC9/openhtmltopdf-pdfbox-0.0.1-RC9.pom (2.0 kB at 4.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox/2.0.4/pdfbox-2.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox/2.0.4/pdfbox-2.0.4.pom (8.3 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox-parent/2.0.4/pdfbox-parent-2.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox-parent/2.0.4/pdfbox-parent-2.0.4.pom (13 kB at 34 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/fontbox/2.0.4/fontbox-2.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/fontbox/2.0.4/fontbox-2.0.4.pom (2.4 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.2/commons-logging-1.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.2/commons-logging-1.2.pom (19 kB at 48 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/34/commons-parent-34.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/34/commons-parent-34.pom (56 kB at 93 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-rtl-support/0.0.1-RC9/openhtmltopdf-rtl-support-0.0.1-RC9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-rtl-support/0.0.1-RC9/openhtmltopdf-rtl-support-0.0.1-RC9.pom (1.7 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/ibm/icu/icu4j/58.1/icu4j-58.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/ibm/icu/icu4j/58.1/icu4j-58.1.pom (4.9 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-jsoup-dom-converter/0.0.1-RC9/openhtmltopdf-jsoup-dom-converter-0.0.1-RC9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-jsoup-dom-converter/0.0.1-RC9/openhtmltopdf-jsoup-dom-converter-0.0.1-RC9.pom (1.6 kB at 4.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.8.3/jsoup-1.8.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.8.3/jsoup-1.8.3.pom (6.4 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-profile-pegdown/0.18.4/flexmark-profile-pegdown-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-profile-pegdown/0.18.4/flexmark-profile-pegdown-0.18.4.pom (4.2 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-youtrack-converter/0.18.4/flexmark-youtrack-converter-0.18.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-youtrack-converter/0.18.4/flexmark-youtrack-converter-0.18.4.pom (1.7 kB at 4.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.5/servlet-api-2.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.5/servlet-api-2.5.pom (157 B at 425 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.8.1/doxia-decoration-model-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.8.1/doxia-decoration-model-1.8.1.pom (3.3 kB at 8.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sitetools/1.8.1/doxia-sitetools-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sitetools/1.8.1/doxia-sitetools-1.8.1.pom (14 kB at 37 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.8.1/doxia-site-renderer-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.8.1/doxia-site-renderer-1.8.1.pom (6.5 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.8.1/doxia-skin-model-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.8.1/doxia-skin-model-1.8.1.pom (2.9 kB at 8.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-7/plexus-i18n-1.0-beta-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-7/plexus-i18n-1.0-beta-7.pom (1.1 kB at 2.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.1/plexus-utils-1.4.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.1/plexus-utils-1.4.1.pom (1.9 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-30/plexus-container-default-1.0-alpha-30.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-30/plexus-container-default-1.0-alpha-30.pom (3.5 kB at 9.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.0-alpha-30/plexus-containers-1.0-alpha-30.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-containers/1.0-alpha-30/plexus-containers-1.0-alpha-30.pom (1.9 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/1.2-alpha-9/plexus-classworlds-1.2-alpha-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/1.2-alpha-9/plexus-classworlds-1.2-alpha-9.pom (3.2 kB at 8.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/3.8.1/junit-3.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/3.8.1/junit-3.8.1.pom (998 B at 2.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.1/commons-collections-3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.1/commons-collections-3.1.pom (6.1 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity-tools/2.0/velocity-tools-2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity-tools/2.0/velocity-tools-2.0.pom (18 kB at 45 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.7.0/commons-beanutils-1.7.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.7.0/commons-beanutils-1.7.0.pom (357 B at 964 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0.3/commons-logging-1.0.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0.3/commons-logging-1.0.3.pom (866 B at 2.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8/commons-digester-1.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8/commons-digester-1.8.pom (7.0 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1/commons-logging-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1/commons-logging-1.1.pom (6.2 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/log4j/log4j/1.2.12/log4j-1.2.12.pom
Downloaded from central: https://repo.maven.apache.org/maven2/log4j/log4j/1.2.12/log4j-1.2.12.pom (145 B at 392 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/logkit/logkit/1.0.1/logkit-1.0.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/logkit/logkit/1.0.1/logkit-1.0.1.pom (147 B at 390 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/avalon-framework/avalon-framework/4.1.3/avalon-framework-4.1.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/avalon-framework/avalon-framework/4.1.3/avalon-framework-4.1.3.pom (167 B at 429 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.3/servlet-api-2.3.pom
Downloaded from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.3/servlet-api-2.3.pom (156 B at 403 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-chain/commons-chain/1.1/commons-chain-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-chain/commons-chain/1.1/commons-chain-1.1.pom (6.0 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.6/commons-digester-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.6/commons-digester-1.6.pom (974 B at 2.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.6/commons-beanutils-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.6/commons-beanutils-1.6.pom (2.3 kB at 6.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0/commons-logging-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0/commons-logging-1.0.pom (163 B at 445 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/2.0/commons-collections-2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/2.0/commons-collections-2.0.pom (171 B at 464 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/2.1/commons-collections-2.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/2.1/commons-collections-2.1.pom (3.3 kB at 8.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/xml-apis/xml-apis/1.0.b2/xml-apis-1.0.b2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/xml-apis/xml-apis/1.0.b2/xml-apis-1.0.b2.pom (2.2 kB at 5.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2/commons-collections-3.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2/commons-collections-3.2.pom (11 kB at 28 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.3.1/commons-validator-1.3.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.3.1/commons-validator-1.3.1.pom (9.0 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0.4/commons-logging-1.0.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.0.4/commons-logging-1.0.4.pom (5.3 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/dom4j/dom4j/1.1/dom4j-1.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/dom4j/dom4j/1.1/dom4j-1.1.pom (142 B at 384 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/oro/oro/2.0.8/oro-2.0.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/oro/oro/2.0.8/oro-2.0.8.pom (140 B at 380 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/sslext/sslext/1.2-0/sslext-1.2-0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/sslext/sslext/1.2-0/sslext-1.2-0.pom (653 B at 1.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-core/1.3.8/struts-core-1.3.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-core/1.3.8/struts-core-1.3.8.pom (4.3 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-parent/1.3.8/struts-parent-1.3.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-parent/1.3.8/struts-parent-1.3.8.pom (9.8 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-master/4/struts-master-4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-master/4/struts-master-4.pom (12 kB at 28 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/2/apache-2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/2/apache-2.pom (3.4 kB at 9.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.2/antlr-2.7.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.2/antlr-2.7.2.pom (145 B at 381 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-taglib/1.3.8/struts-taglib-1.3.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-taglib/1.3.8/struts-taglib-1.3.8.pom (3.1 kB at 8.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-tiles/1.3.8/struts-tiles-1.3.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-tiles/1.3.8/struts-tiles-1.3.8.pom (2.9 kB at 7.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.6.2/velocity-1.6.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity/1.6.2/velocity-1.6.2.pom (11 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-integration-tools/1.8.1/doxia-integration-tools-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-integration-tools/1.8.1/doxia-integration-tools-1.8.1.pom (5.9 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/1.4/commons-io-1.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/1.4/commons-io-1.4.pom (13 kB at 33 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/7/commons-parent-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/7/commons-parent-7.pom (17 kB at 45 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9/plexus-container-default-1.0-alpha-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-9/plexus-container-default-1.0-alpha-9.pom (1.2 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/1.0/wagon-provider-api-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/1.0/wagon-provider-api-1.0.pom (1.8 kB at 1.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/1.0/wagon-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/1.0/wagon-1.0.pom (9.8 kB at 25 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/20/maven-parent-20.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/20/maven-parent-20.pom (25 kB at 58 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.2/plexus-utils-1.4.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/1.4.2/plexus-utils-1.4.2.pom (2.0 kB at 5.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty/6.1.25/jetty-6.1.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty/6.1.25/jetty-6.1.25.pom (6.3 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/project/6.1.25/project-6.1.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/project/6.1.25/project-6.1.25.pom (9.2 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-parent/10/jetty-parent-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-parent/10/jetty-parent-10.pom (3.3 kB at 8.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/eclipse/jetty/jetty-parent/14/jetty-parent-14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/eclipse/jetty/jetty-parent/14/jetty-parent-14.pom (16 kB at 41 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-util/6.1.25/jetty-util-6.1.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-util/6.1.25/jetty-util-6.1.25.pom (3.9 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/servlet-api/2.5-20081211/servlet-api-2.5-20081211.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/servlet-api/2.5-20081211/servlet-api-2.5-20081211.pom (2.7 kB at 7.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-parent/7/jetty-parent-7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-parent/7/jetty-parent-7.pom (13 kB at 34 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.4/commons-lang3-3.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.4/commons-lang3-3.4.pom (22 kB at 53 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/37/commons-parent-37.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/37/commons-parent-37.pom (63 kB at 116 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-exec/1.4/maven-reporting-exec-1.4.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether-util/0.9.0.M2/aether-util-0.9.0.M2.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.1.0/maven-shared-utils-3.1.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-10/plexus-i18n-1.0-beta-10.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.2/plexus-classworlds-2.5.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-exec/1.4/maven-reporting-exec-1.4.jar (30 kB at 51 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.8/doxia-sink-api-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.8/doxia-sink-api-1.8.jar (12 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.8/doxia-logging-api-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.8/doxia-logging-api-1.8.jar (12 kB at 7.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.8/doxia-module-xhtml-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-i18n/1.0-beta-10/plexus-i18n-1.0-beta-10.jar (12 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.8/doxia-core-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.8/doxia-module-xhtml-1.8.jar (18 kB at 8.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.0.2/httpclient-4.0.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-classworlds/2.5.2/plexus-classworlds-2.5.2.jar (53 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.3/commons-codec-1.3.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/eclipse/aether/aether-util/0.9.0.M2/aether-util-0.9.0.M2.jar (134 kB at 53 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.0.1/httpcore-4.0.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.1.0/maven-shared-utils-3.1.0.jar (164 kB at 63 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-apt/1.8/doxia-module-apt-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.3/commons-codec-1.3.jar (47 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xdoc/1.8/doxia-module-xdoc-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xdoc/1.8/doxia-module-xdoc-1.8.jar (38 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-fml/1.8/doxia-module-fml-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.8/doxia-core-1.8.jar (168 kB at 53 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-markdown/1.8/doxia-module-markdown-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-apt/1.8/doxia-module-apt-1.8.jar (54 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-all/0.18.4/flexmark-all-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.0.1/httpcore-4.0.1.jar (173 kB at 51 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark/0.18.4/flexmark-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.0.2/httpclient-4.0.2.jar (293 kB at 86 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-abbreviation/0.18.4/flexmark-ext-abbreviation-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-fml/1.8/doxia-module-fml-1.8.jar (39 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-anchorlink/0.18.4/flexmark-ext-anchorlink-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-markdown/1.8/doxia-module-markdown-1.8.jar (21 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-aside/0.18.4/flexmark-ext-aside-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-all/0.18.4/flexmark-all-0.18.4.jar (2.1 kB at 552 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-autolink/0.18.4/flexmark-ext-autolink-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-abbreviation/0.18.4/flexmark-ext-abbreviation-0.18.4.jar (33 kB at 8.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/nibor/autolink/autolink/0.6.0/autolink-0.6.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-anchorlink/0.18.4/flexmark-ext-anchorlink-0.18.4.jar (17 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-definition/0.18.4/flexmark-ext-definition-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/nibor/autolink/autolink/0.6.0/autolink-0.6.0.jar (16 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-emoji/0.18.4/flexmark-ext-emoji-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-aside/0.18.4/flexmark-ext-aside-0.18.4.jar (15 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-escaped-character/0.18.4/flexmark-ext-escaped-character-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-autolink/0.18.4/flexmark-ext-autolink-0.18.4.jar (6.4 kB at 1.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-footnotes/0.18.4/flexmark-ext-footnotes-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-definition/0.18.4/flexmark-ext-definition-0.18.4.jar (37 kB at 8.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-strikethrough/0.18.4/flexmark-ext-gfm-strikethrough-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark/0.18.4/flexmark-0.18.4.jar (357 kB at 76 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tables/0.18.4/flexmark-ext-gfm-tables-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-emoji/0.18.4/flexmark-ext-emoji-0.18.4.jar (48 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tasklist/0.18.4/flexmark-ext-gfm-tasklist-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-escaped-character/0.18.4/flexmark-ext-escaped-character-0.18.4.jar (13 kB at 2.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-front-matter/0.18.4/flexmark-ext-jekyll-front-matter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-footnotes/0.18.4/flexmark-ext-footnotes-0.18.4.jar (38 kB at 7.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-tag/0.18.4/flexmark-ext-jekyll-tag-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-strikethrough/0.18.4/flexmark-ext-gfm-strikethrough-0.18.4.jar (33 kB at 6.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-ins/0.18.4/flexmark-ext-ins-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tasklist/0.18.4/flexmark-ext-gfm-tasklist-0.18.4.jar (28 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-xwiki-macros/0.18.4/flexmark-ext-xwiki-macros-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-gfm-tables/0.18.4/flexmark-ext-gfm-tables-0.18.4.jar (34 kB at 6.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-spec-example/0.18.4/flexmark-ext-spec-example-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-ins/0.18.4/flexmark-ext-ins-0.18.4.jar (13 kB at 2.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-test-util/0.18.4/flexmark-test-util-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-front-matter/0.18.4/flexmark-ext-jekyll-front-matter-0.18.4.jar (18 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-superscript/0.18.4/flexmark-ext-superscript-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-jekyll-tag/0.18.4/flexmark-ext-jekyll-tag-0.18.4.jar (21 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-tables/0.18.4/flexmark-ext-tables-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-xwiki-macros/0.18.4/flexmark-ext-xwiki-macros-0.18.4.jar (31 kB at 5.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-toc/0.18.4/flexmark-ext-toc-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-spec-example/0.18.4/flexmark-ext-spec-example-0.18.4.jar (40 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-typographic/0.18.4/flexmark-ext-typographic-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-superscript/0.18.4/flexmark-ext-superscript-0.18.4.jar (13 kB at 2.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-wikilink/0.18.4/flexmark-ext-wikilink-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-toc/0.18.4/flexmark-ext-toc-0.18.4.jar (85 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-yaml-front-matter/0.18.4/flexmark-ext-yaml-front-matter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-tables/0.18.4/flexmark-ext-tables-0.18.4.jar (61 kB at 9.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-formatter/0.18.4/flexmark-formatter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-test-util/0.18.4/flexmark-test-util-0.18.4.jar (238 kB at 37 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-html-parser/0.18.4/flexmark-html-parser-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-typographic/0.18.4/flexmark-ext-typographic-0.18.4.jar (22 kB at 3.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.10.2/jsoup-1.10.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-wikilink/0.18.4/flexmark-ext-wikilink-0.18.4.jar (25 kB at 3.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-jira-converter/0.18.4/flexmark-jira-converter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-ext-yaml-front-matter/0.18.4/flexmark-ext-yaml-front-matter-0.18.4.jar (17 kB at 2.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-pdf-converter/0.18.4/flexmark-pdf-converter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-formatter/0.18.4/flexmark-formatter-0.18.4.jar (73 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-core/0.0.1-RC9/openhtmltopdf-core-0.0.1-RC9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-html-parser/0.18.4/flexmark-html-parser-0.18.4.jar (28 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-pdfbox/0.0.1-RC9/openhtmltopdf-pdfbox-0.0.1-RC9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-pdf-converter/0.18.4/flexmark-pdf-converter-0.18.4.jar (4.4 kB at 618 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox/2.0.4/pdfbox-2.0.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-jira-converter/0.18.4/flexmark-jira-converter-0.18.4.jar (39 kB at 5.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/fontbox/2.0.4/fontbox-2.0.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.10.2/jsoup-1.10.2.jar (351 kB at 48 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-rtl-support/0.0.1-RC9/openhtmltopdf-rtl-support-0.0.1-RC9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-pdfbox/0.0.1-RC9/openhtmltopdf-pdfbox-0.0.1-RC9.jar (122 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/ibm/icu/icu4j/58.1/icu4j-58.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-rtl-support/0.0.1-RC9/openhtmltopdf-rtl-support-0.0.1-RC9.jar (25 kB at 3.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-jsoup-dom-converter/0.0.1-RC9/openhtmltopdf-jsoup-dom-converter-0.0.1-RC9.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-jsoup-dom-converter/0.0.1-RC9/openhtmltopdf-jsoup-dom-converter-0.0.1-RC9.jar (20 kB at 2.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-profile-pegdown/0.18.4/flexmark-profile-pegdown-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/openhtmltopdf/openhtmltopdf-core/0.0.1-RC9/openhtmltopdf-core-0.0.1-RC9.jar (1.2 MB at 145 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-util/0.18.4/flexmark-util-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-profile-pegdown/0.18.4/flexmark-profile-pegdown-0.18.4.jar (6.5 kB at 715 B/s)
Downloading from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-youtrack-converter/0.18.4/flexmark-youtrack-converter-0.18.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-util/0.18.4/flexmark-util-0.18.4.jar (278 kB at 31 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.5/servlet-api-2.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/com/vladsch/flexmark/flexmark-youtrack-converter/0.18.4/flexmark-youtrack-converter-0.18.4.jar (41 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.8.1/doxia-decoration-model-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/javax/servlet/servlet-api/2.5/servlet-api-2.5.jar (105 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.8.1/doxia-site-renderer-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/fontbox/2.0.4/fontbox-2.0.4.jar (1.5 MB at 149 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.8.1/doxia-skin-model-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.8.1/doxia-decoration-model-1.8.1.jar (61 kB at 6.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-30/plexus-container-default-1.0-alpha-30.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.8.1/doxia-site-renderer-1.8.1.jar (65 kB at 6.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/junit/junit/3.8.1/junit-3.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.8.1/doxia-skin-model-1.8.1.jar (16 kB at 1.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity-tools/2.0/velocity-tools-2.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/junit/junit/3.8.1/junit-3.8.1.jar (121 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.7.0/commons-beanutils-1.7.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-30/plexus-container-default-1.0-alpha-30.jar (237 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8/commons-digester-1.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.7.0/commons-beanutils-1.7.0.jar (189 kB at 17 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-chain/commons-chain/1.1/commons-chain-1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8/commons-digester-1.8.jar (144 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1/commons-logging-1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/velocity/velocity-tools/2.0/velocity-tools-2.0.jar (347 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.3.1/commons-validator-1.3.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-chain/commons-chain/1.1/commons-chain-1.1.jar (90 kB at 7.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/dom4j/dom4j/1.1/dom4j-1.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.1/commons-logging-1.1.jar (53 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/oro/oro/2.0.8/oro-2.0.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.3.1/commons-validator-1.3.1.jar (139 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/sslext/sslext/1.2-0/sslext-1.2-0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/dom4j/dom4j/1.1/dom4j-1.1.jar (457 kB at 36 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-core/1.3.8/struts-core-1.3.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/oro/oro/2.0.8/oro-2.0.8.jar (65 kB at 5.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.2/antlr-2.7.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/sslext/sslext/1.2-0/sslext-1.2-0.jar (26 kB at 2.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-taglib/1.3.8/struts-taglib-1.3.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/antlr/antlr/2.7.2/antlr-2.7.2.jar (358 kB at 27 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-tiles/1.3.8/struts-tiles-1.3.8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-core/1.3.8/struts-core-1.3.8.jar (329 kB at 24 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-integration-tools/1.8.1/doxia-integration-tools-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-tiles/1.3.8/struts-tiles-1.3.8.jar (120 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/1.0/wagon-provider-api-1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-integration-tools/1.8.1/doxia-integration-tools-1.8.1.jar (47 kB at 3.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty/6.1.25/jetty-6.1.25.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/struts/struts-taglib/1.3.8/struts-taglib-1.3.8.jar (252 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/servlet-api/2.5-20081211/servlet-api-2.5-20081211.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/1.0/wagon-provider-api-1.0.jar (53 kB at 3.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-util/6.1.25/jetty-util-6.1.25.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/servlet-api/2.5-20081211/servlet-api-2.5-20081211.jar (134 kB at 9.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.4/commons-lang3-3.4.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/pdfbox/pdfbox/2.0.4/pdfbox-2.0.4.jar (2.5 MB at 163 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty/6.1.25/jetty-6.1.25.jar (539 kB at 36 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/mortbay/jetty/jetty-util/6.1.25/jetty-util-6.1.25.jar (177 kB at 12 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.4/commons-lang3-3.4.jar (435 kB at 26 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/com/ibm/icu/icu4j/58.1/icu4j-58.1.jar (12 MB at 207 kB/s)
[INFO] configuring report plugin org.apache.maven.plugins:maven-project-info-reports-plugin:3.0.0
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-project-info-reports-plugin/3.0.0/maven-project-info-reports-plugin-3.0.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-project-info-reports-plugin/3.0.0/maven-project-info-reports-plugin-3.0.0.pom (20 kB at 44 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-project-info-reports-plugin/3.0.0/maven-project-info-reports-plugin-3.0.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-project-info-reports-plugin/3.0.0/maven-project-info-reports-plugin-3.0.0.jar (300 kB at 349 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-impl/3.0.0/maven-reporting-impl-3.0.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-impl/3.0.0/maven-reporting-impl-3.0.0.pom (7.6 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.0/maven-shared-utils-3.2.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.0/maven-shared-utils-3.2.0.pom (4.9 kB at 13 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.7/doxia-sink-api-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sink-api/1.7/doxia-sink-api-1.7.pom (1.5 kB at 4.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.7/doxia-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia/1.7/doxia-1.7.pom (15 kB at 39 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.7/doxia-logging-api-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-logging-api/1.7/doxia-logging-api-1.7.pom (1.5 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.7.4/doxia-decoration-model-1.7.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-decoration-model/1.7.4/doxia-decoration-model-1.7.4.pom (3.4 kB at 9.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sitetools/1.7.4/doxia-sitetools-1.7.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-sitetools/1.7.4/doxia-sitetools-1.7.4.pom (14 kB at 36 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.7/doxia-core-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-core/1.7/doxia-core-1.7.pom (4.1 kB at 11 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/xmlunit/xmlunit/1.5/xmlunit-1.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/xmlunit/xmlunit/1.5/xmlunit-1.5.pom (3.0 kB at 8.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.7.4/doxia-site-renderer-1.7.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-site-renderer/1.7.4/doxia-site-renderer-1.7.4.pom (6.7 kB at 18 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.7.4/doxia-skin-model-1.7.4.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-skin-model/1.7.4/doxia-skin-model-1.7.4.pom (3.0 kB at 8.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.7/doxia-module-xhtml-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-module-xhtml/1.7/doxia-module-xhtml-1.7.pom (1.6 kB at 4.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-modules/1.7/doxia-modules-1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/doxia/doxia-modules/1.7/doxia-modules-1.7.pom (2.6 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-dependency-tree/2.2/maven-dependency-tree-2.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-dependency-tree/2.2/maven-dependency-tree-2.2.pom (7.3 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/20/maven-shared-components-20.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-components/20/maven-shared-components-20.pom (5.1 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/24/maven-parent-24.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/maven-parent/24/maven-parent-24.pom (37 kB at 89 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/apache/14/apache-14.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/apache/14/apache-14.pom (15 kB at 38 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-jar/1.2/maven-shared-jar-1.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-jar/1.2/maven-shared-jar-1.2.pom (4.4 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.9/maven-shared-utils-0.9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/0.9/maven-shared-utils-0.9.pom (6.2 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-digest/1.0/plexus-digest-1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-digest/1.0/plexus-digest-1.0.pom (1.1 kB at 2.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.7/plexus-components-1.1.7.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-components/1.1.7/plexus-components-1.1.7.pom (5.0 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.8/plexus-1.0.8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus/1.0.8/plexus-1.0.8.pom (7.2 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-8/plexus-container-default-1.0-alpha-8.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-8/plexus-container-default-1.0-alpha-8.pom (7.3 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/bcel/bcel/6.2/bcel-6.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/bcel/bcel/6.2/bcel-6.2.pom (20 kB at 50 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.9.1/maven-artifact-transfer-0.9.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.9.1/maven-artifact-transfer-0.9.1.pom (7.6 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.6/commons-codec-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.6/commons-codec-1.6.pom (11 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/22/commons-parent-22.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/22/commons-parent-22.pom (42 kB at 100 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-api/1.10.0/maven-scm-api-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-api/1.10.0/maven-scm-api-1.10.0.pom (1.6 kB at 4.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm/1.10.0/maven-scm-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm/1.10.0/maven-scm-1.10.0.pom (26 kB at 66 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-manager-plexus/1.10.0/maven-scm-manager-plexus-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-manager-plexus/1.10.0/maven-scm-manager-plexus-1.10.0.pom (2.2 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-managers/1.10.0/maven-scm-managers-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-managers/1.10.0/maven-scm-managers-1.10.0.pom (1.5 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-hg/1.10.0/maven-scm-provider-hg-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-hg/1.10.0/maven-scm-provider-hg-1.10.0.pom (2.4 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers/1.10.0/maven-scm-providers-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers/1.10.0/maven-scm-providers-1.10.0.pom (3.6 kB at 9.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-perforce/1.10.0/maven-scm-provider-perforce-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-perforce/1.10.0/maven-scm-provider-perforce-1.10.0.pom (2.6 kB at 6.7 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-starteam/1.10.0/maven-scm-provider-starteam-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-starteam/1.10.0/maven-scm-provider-starteam-1.10.0.pom (2.7 kB at 7.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svn-commons/1.10.0/maven-scm-provider-svn-commons-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svn-commons/1.10.0/maven-scm-provider-svn-commons-1.10.0.pom (2.8 kB at 7.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-svn/1.10.0/maven-scm-providers-svn-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-svn/1.10.0/maven-scm-providers-svn-1.10.0.pom (2.2 kB at 6.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svnexe/1.10.0/maven-scm-provider-svnexe-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svnexe/1.10.0/maven-scm-provider-svnexe-1.10.0.pom (2.7 kB at 7.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.pom (17 kB at 40 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/17/commons-parent-17.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/17/commons-parent-17.pom (31 kB at 78 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvs-commons/1.10.0/maven-scm-provider-cvs-commons-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvs-commons/1.10.0/maven-scm-provider-cvs-commons-1.10.0.pom (2.4 kB at 6.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-cvs/1.10.0/maven-scm-providers-cvs-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-cvs/1.10.0/maven-scm-providers-cvs-1.10.0.pom (1.8 kB at 4.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsexe/1.10.0/maven-scm-provider-cvsexe-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsexe/1.10.0/maven-scm-provider-cvsexe-1.10.0.pom (2.8 kB at 7.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsjava/1.10.0/maven-scm-provider-cvsjava-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsjava/1.10.0/maven-scm-provider-cvsjava-1.10.0.pom (2.7 kB at 7.3 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/netbeans/lib/cvsclient/20060125/cvsclient-20060125.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/netbeans/lib/cvsclient/20060125/cvsclient-20060125.pom (459 B at 1.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/ethz/ganymed/ganymed-ssh2/build210/ganymed-ssh2-build210.pom
Downloaded from central: https://repo.maven.apache.org/maven2/ch/ethz/ganymed/ganymed-ssh2/build210/ganymed-ssh2-build210.pom (710 B at 1.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-git-commons/1.10.0/maven-scm-provider-git-commons-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-git-commons/1.10.0/maven-scm-provider-git-commons-1.10.0.pom (2.7 kB at 7.4 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-git/1.10.0/maven-scm-providers-git-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-providers-git/1.10.0/maven-scm-providers-git-1.10.0.pom (2.2 kB at 6.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-gitexe/1.10.0/maven-scm-provider-gitexe-1.10.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-gitexe/1.10.0/maven-scm-provider-gitexe-1.10.0.pom (2.6 kB at 6.9 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-lightweight/3.1.0/wagon-http-lightweight-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-lightweight/3.1.0/wagon-http-lightweight-3.1.0.pom (2.6 kB at 7.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-providers/3.1.0/wagon-providers-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-providers/3.1.0/wagon-providers-3.1.0.pom (2.8 kB at 5.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/3.1.0/wagon-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon/3.1.0/wagon-3.1.0.pom (21 kB at 53 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-shared/3.1.0/wagon-http-shared-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-shared/3.1.0/wagon-http-shared-3.1.0.pom (2.6 kB at 6.8 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.11.2/jsoup-1.11.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.11.2/jsoup-1.11.2.pom (8.2 kB at 22 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.5.5/httpclient-4.5.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpclient/4.5.5/httpclient-4.5.5.pom (6.2 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-client/4.5.5/httpcomponents-client-4.5.5.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-client/4.5.5/httpcomponents-client-4.5.5.pom (15 kB at 39 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-parent/10/httpcomponents-parent-10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-parent/10/httpcomponents-parent-10.pom (34 kB at 83 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.4.9/httpcore-4.4.9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcore/4.4.9/httpcore-4.4.9.pom (5.1 kB at 14 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-core/4.4.9/httpcomponents-core-4.4.9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-core/4.4.9/httpcomponents-core-4.4.9.pom (13 kB at 35 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-parent/9/httpcomponents-parent-9.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/httpcomponents/httpcomponents-parent/9/httpcomponents-parent-9.pom (34 kB at 85 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.10/commons-codec-1.10.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.10/commons-codec-1.10.pom (12 kB at 31 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/35/commons-parent-35.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/35/commons-parent-35.pom (58 kB at 130 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-api/1.7.25/slf4j-api-1.7.25.pom (3.8 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.25/slf4j-parent-1.7.25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/slf4j/slf4j-parent/1.7.25/slf4j-parent-1.7.25.pom (14 kB at 36 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.1.0/wagon-provider-api-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.1.0/wagon-provider-api-3.1.0.pom (1.9 kB at 4.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.6/commons-validator-1.6.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.6/commons-validator-1.6.pom (12 kB at 20 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.9.2/commons-beanutils-1.9.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.9.2/commons-beanutils-1.9.2.pom (14 kB at 37 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/33/commons-parent-33.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/33/commons-parent-33.pom (53 kB at 118 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8.1/commons-digester-1.8.1.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8.1/commons-digester-1.8.1.pom (10 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/11/commons-parent-11.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-parent/11/commons-parent-11.pom (25 kB at 61 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.2/commons-collections-3.2.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.2.2/commons-collections-3.2.2.pom (12 kB at 32 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-impl/3.0.0/maven-reporting-impl-3.0.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.0/maven-shared-utils-3.2.0.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-dependency-tree/2.2/maven-dependency-tree-2.2.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-jar/1.2/maven-shared-jar-1.2.jar
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-digest/1.0/plexus-digest-1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/reporting/maven-reporting-impl/3.0.0/maven-reporting-impl-3.0.0.jar (18 kB at 31 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-8/plexus-container-default-1.0-alpha-8.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-digest/1.0/plexus-digest-1.0.jar (12 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.1/commons-collections-3.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-jar/1.2/maven-shared-jar-1.2.jar (38 kB at 57 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/bcel/bcel/6.2/bcel-6.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-shared-utils/3.2.0/maven-shared-utils-3.2.0.jar (165 kB at 197 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.9.1/maven-artifact-transfer-0.9.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-dependency-tree/2.2/maven-dependency-tree-2.2.jar (64 kB at 77 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.6/commons-codec-1.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-container-default/1.0-alpha-8/plexus-container-default-1.0-alpha-8.jar (195 kB at 165 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-api/1.10.0/maven-scm-api-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-collections/commons-collections/3.1/commons-collections-3.1.jar (559 kB at 453 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-manager-plexus/1.10.0/maven-scm-manager-plexus-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/shared/maven-artifact-transfer/0.9.1/maven-artifact-transfer-0.9.1.jar (123 kB at 97 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-hg/1.10.0/maven-scm-provider-hg-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-hg/1.10.0/maven-scm-provider-hg-1.10.0.jar (69 kB at 40 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-perforce/1.10.0/maven-scm-provider-perforce-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-manager-plexus/1.10.0/maven-scm-manager-plexus-1.10.0.jar (11 kB at 6.1 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-starteam/1.10.0/maven-scm-provider-starteam-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-codec/commons-codec/1.6/commons-codec-1.6.jar (233 kB at 129 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svn-commons/1.10.0/maven-scm-provider-svn-commons-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-api/1.10.0/maven-scm-api-1.10.0.jar (110 kB at 59 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svnexe/1.10.0/maven-scm-provider-svnexe-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/bcel/bcel/6.2/bcel-6.2.jar (674 kB at 337 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-perforce/1.10.0/maven-scm-provider-perforce-1.10.0.jar (86 kB at 39 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvs-commons/1.10.0/maven-scm-provider-cvs-commons-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svn-commons/1.10.0/maven-scm-provider-svn-commons-1.10.0.jar (38 kB at 16 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsexe/1.10.0/maven-scm-provider-cvsexe-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-starteam/1.10.0/maven-scm-provider-starteam-1.10.0.jar (73 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsjava/1.10.0/maven-scm-provider-cvsjava-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvs-commons/1.10.0/maven-scm-provider-cvs-commons-1.10.0.jar (79 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/netbeans/lib/cvsclient/20060125/cvsclient-20060125.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-svnexe/1.10.0/maven-scm-provider-svnexe-1.10.0.jar (81 kB at 30 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/ch/ethz/ganymed/ganymed-ssh2/build210/ganymed-ssh2-build210.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-lang/commons-lang/2.6/commons-lang-2.6.jar (284 kB at 97 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-git-commons/1.10.0/maven-scm-provider-git-commons-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsjava/1.10.0/maven-scm-provider-cvsjava-1.10.0.jar (45 kB at 15 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-gitexe/1.10.0/maven-scm-provider-gitexe-1.10.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-cvsexe/1.10.0/maven-scm-provider-cvsexe-1.10.0.jar (31 kB at 10 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.2/commons-io-2.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-git-commons/1.10.0/maven-scm-provider-git-commons-1.10.0.jar (35 kB at 10.0 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.5/commons-lang3-3.5.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/scm/maven-scm-provider-gitexe/1.10.0/maven-scm-provider-gitexe-1.10.0.jar (69 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.1.0/plexus-utils-3.1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/ch/ethz/ganymed/ganymed-ssh2/build210/ganymed-ssh2-build210.jar (245 kB at 65 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-lightweight/3.1.0/wagon-http-lightweight-3.1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/netbeans/lib/cvsclient/20060125/cvsclient-20060125.jar (619 kB at 160 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-shared/3.1.0/wagon-http-shared-3.1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-io/commons-io/2.2/commons-io-2.2.jar (174 kB at 44 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.11.2/jsoup-1.11.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-lightweight/3.1.0/wagon-http-lightweight-3.1.0.jar (17 kB at 4.2 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.1.0/wagon-provider-api-3.1.0.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/commons/commons-lang3/3.5/commons-lang3-3.5.jar (480 kB at 115 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.6/commons-validator-1.6.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/codehaus/plexus/plexus-utils/3.1.0/plexus-utils-3.1.0.jar (262 kB at 63 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.9.2/commons-beanutils-1.9.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-http-shared/3.1.0/wagon-http-shared-3.1.0.jar (36 kB at 8.5 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8.1/commons-digester-1.8.1.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/wagon/wagon-provider-api/3.1.0/wagon-provider-api-3.1.0.jar (55 kB at 12 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.2/commons-logging-1.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/commons-validator/commons-validator/1.6/commons-validator-1.6.jar (186 kB at 39 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/commons-digester/commons-digester/1.8.1/commons-digester-1.8.1.jar (146 kB at 31 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/commons-beanutils/commons-beanutils/1.9.2/commons-beanutils-1.9.2.jar (234 kB at 49 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/commons-logging/commons-logging/1.2/commons-logging-1.2.jar (62 kB at 12 kB/s)
Downloaded from central: https://repo.maven.apache.org/maven2/org/jsoup/jsoup/1.11.2/jsoup-1.11.2.jar (392 kB at 72 kB/s)
[INFO] 15 reports detected for maven-project-info-reports-plugin:3.0.0: ci-management, dependencies, dependency-info, dependency-management, distribution-management, index, issue-management, licenses, mailing-lists, modules, plugin-management, plugins, scm, summary, team
[INFO] Rendering site with default locale English (en)
[INFO] Relativizing decoration links with respect to localized project URL: http://www.example.com
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/skins/maven-default-skin/1.2/maven-default-skin-1.2.jar
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/skins/maven-default-skin/1.2/maven-default-skin-1.2.jar (14 kB at 24 kB/s)
[INFO] Rendering content with org.apache.maven.skins:maven-default-skin:jar:1.2 skin.
[INFO] Generating "Dependencies" report  --- maven-project-info-reports-plugin:3.0.0:dependencies
[INFO] Generating "Dependency Information" report --- maven-project-info-reports-plugin:3.0.0:dependency-info
[INFO] Generating "About" report         --- maven-project-info-reports-plugin:3.0.0:index
[INFO] Generating "Plugin Management" report --- maven-project-info-reports-plugin:3.0.0:plugin-management
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/3.1.0/maven-clean-plugin-3.1.0.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-clean-plugin/3.1.0/maven-clean-plugin-3.1.0.pom (5.2 kB at 4.6 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.8.2/maven-deploy-plugin-2.8.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-deploy-plugin/2.8.2/maven-deploy-plugin-2.8.2.pom (7.1 kB at 19 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/25/maven-plugins-25.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-plugins/25/maven-plugins-25.pom (9.6 kB at 26 kB/s)
Downloading from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.5.2/maven-install-plugin-2.5.2.pom
Downloaded from central: https://repo.maven.apache.org/maven2/org/apache/maven/plugins/maven-install-plugin/2.5.2/maven-install-plugin-2.5.2.pom (6.4 kB at 18 kB/s)
[INFO] Generating "Plugins" report       --- maven-project-info-reports-plugin:3.0.0:plugins
[INFO] Generating "Summary" report       --- maven-project-info-reports-plugin:3.0.0:summary
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  02:40 min
[INFO] Finished at: 2021-09-19T09:57:13-07:00
[INFO] ------------------------------------------------------------------------
{% endhighlight %}

执行```site``` phase时，会根据工程的pom.xml生成site信息。我们可以在```target/site```目录下查看到相应的site信息：
<pre>
# tree
.
├── pom.xml
├── src
│   ├── main
│   │   └── java
│   │       └── com
│   │           └── mycompany
│   │               └── app
│   │                   └── App.java
│   └── test
│       └── java
│           └── com
│               └── mycompany
│                   └── app
│                       └── AppTest.java
└── target
    ├── classes
    │   └── com
    │       └── mycompany
    │           └── app
    │               └── App.class
    ├── generated-sources
    │   └── annotations
    ├── generated-test-sources
    │   └── test-annotations
    ├── maven-archiver
    │   └── pom.properties
    ├── maven-status
    │   └── maven-compiler-plugin
    │       ├── compile
    │       │   └── default-compile
    │       │       ├── createdFiles.lst
    │       │       └── inputFiles.lst
    │       └── testCompile
    │           └── default-testCompile
    │               ├── createdFiles.lst
    │               └── inputFiles.lst
    ├── my-app-1.0-SNAPSHOT.jar
    ├── site
    │   ├── css
    │   │   ├── maven-base.css
    │   │   ├── maven-theme.css
    │   │   ├── print.css
    │   │   └── site.css
    │   ├── dependencies.html
    │   ├── dependency-info.html
    │   ├── images
    │   │   ├── close.gif
    │   │   ├── collapsed.gif
    │   │   ├── expanded.gif
    │   │   ├── external.png
    │   │   ├── icon_error_sml.gif
    │   │   ├── icon_info_sml.gif
    │   │   ├── icon_success_sml.gif
    │   │   ├── icon_warning_sml.gif
    │   │   ├── logos
    │   │   │   ├── build-by-maven-black.png
    │   │   │   ├── build-by-maven-white.png
    │   │   │   └── maven-feather.png
    │   │   └── newwindow.png
    │   ├── index.html
    │   ├── plugin-management.html
    │   ├── plugins.html
    │   ├── project-info.html
    │   └── summary.html
    ├── surefire-reports
    │   ├── com.mycompany.app.AppTest.txt
    │   └── TEST-com.mycompany.app.AppTest.xml
    └── test-classes
        └── com
            └── mycompany
                └── app
                    └── AppTest.class

36 directories, 36 files

</pre>




<br />
<br />
**[参看]：**

1. [maven官网](https://maven.apache.org/)

2. [Maven in 5 Minutes](https://maven.apache.org/guides/getting-started/maven-in-five-minutes.html)

<br />
<br />
<br />

