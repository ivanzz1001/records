---
layout: post
title: Google test的使用
tags:
- cplusplus
categories: cplusplus
description: google test的使用
---


Google test是一种比较方便的C++测试框架，它能够帮助我们比较方便的进行测试代码的编写，以及输出尽可能详细的失败信息。能够大大缩短我们测试代码的编写效率，而且该框架的使用方法也比较简单，能够降低我们学习新框架的负担。



<!-- more -->


## 1. googletest的编译安装

1） **安装cmake**
<pre>
# yum install cmake
# cmake --version
cmake version 2.8.12.2
</pre>


2) **编译并安装gtest**

执行如下命令下载、编译并安装googletest:
<pre>
# git clone https://github.com/google/googletest.git
# ls
appveyor.yml  BUILD.bazel  ci  CMakeLists.txt  CONTRIBUTING.md  googlemock  googletest  library.json  LICENSE  platformio.ini  README.md  WORKSPACE
# mkdir mybuild && cd mybuild
# cmake ..
-- The C compiler identification is GNU 4.8.5
-- The CXX compiler identification is GNU 4.8.5
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Found PythonInterp: /usr/bin/python (found version "2.7.5") 
-- Looking for include file pthread.h
-- Looking for include file pthread.h - found
-- Looking for pthread_create
-- Looking for pthread_create - not found
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - found
-- Found Threads: TRUE  
-- Configuring done
-- Generating done
-- Build files have been written to: /root/googletest/inst/googletest/mybuild
# ls
bin  CMakeCache.txt  CMakeFiles  cmake_install.cmake  CTestTestfile.cmake  googlemock  googletest  lib  Makefile
</pre>
执行完上面的命令后，可以看到生成了相应的编译脚本，接着再执行如下命令完成编译与安装：
<pre>
# make 
# make install
[ 25%] Built target gtest
[ 50%] Built target gmock
[ 75%] Built target gmock_main
[100%] Built target gtest_main
Install the project...
-- Install configuration: ""
-- Installing: /usr/local/include
-- Installing: /usr/local/include/gmock
-- Installing: /usr/local/include/gmock/gmock-actions.h
-- Installing: /usr/local/include/gmock/gmock-cardinalities.h
-- Installing: /usr/local/include/gmock/gmock-function-mocker.h
-- Installing: /usr/local/include/gmock/gmock-generated-actions.h
-- Installing: /usr/local/include/gmock/gmock-generated-actions.h.pump
-- Installing: /usr/local/include/gmock/gmock-generated-function-mockers.h
-- Installing: /usr/local/include/gmock/gmock-generated-function-mockers.h.pump
-- Installing: /usr/local/include/gmock/gmock-generated-matchers.h
-- Installing: /usr/local/include/gmock/gmock-generated-matchers.h.pump
-- Installing: /usr/local/include/gmock/gmock-matchers.h
-- Installing: /usr/local/include/gmock/gmock-more-actions.h
-- Installing: /usr/local/include/gmock/gmock-more-matchers.h
-- Installing: /usr/local/include/gmock/gmock-nice-strict.h
-- Installing: /usr/local/include/gmock/gmock-spec-builders.h
-- Installing: /usr/local/include/gmock/gmock.h
-- Installing: /usr/local/include/gmock/internal
-- Installing: /usr/local/include/gmock/internal/custom
-- Installing: /usr/local/include/gmock/internal/custom/README.md
-- Installing: /usr/local/include/gmock/internal/custom/gmock-generated-actions.h
-- Installing: /usr/local/include/gmock/internal/custom/gmock-generated-actions.h.pump
-- Installing: /usr/local/include/gmock/internal/custom/gmock-matchers.h
-- Installing: /usr/local/include/gmock/internal/custom/gmock-port.h
-- Installing: /usr/local/include/gmock/internal/gmock-internal-utils.h
-- Installing: /usr/local/include/gmock/internal/gmock-port.h
-- Installing: /usr/local/include/gmock/internal/gmock-pp.h
-- Installing: /usr/local/lib64/libgmock.a
-- Installing: /usr/local/lib64/libgmock_main.a
-- Installing: /usr/local/lib64/pkgconfig/gmock.pc
-- Installing: /usr/local/lib64/pkgconfig/gmock_main.pc
-- Installing: /usr/local/lib64/cmake/GTest/GTestTargets.cmake
-- Installing: /usr/local/lib64/cmake/GTest/GTestTargets-noconfig.cmake
-- Installing: /usr/local/lib64/cmake/GTest/GTestConfigVersion.cmake
-- Installing: /usr/local/lib64/cmake/GTest/GTestConfig.cmake
-- Installing: /usr/local/include
-- Installing: /usr/local/include/gtest
-- Installing: /usr/local/include/gtest/gtest-death-test.h
-- Installing: /usr/local/include/gtest/gtest-matchers.h
-- Installing: /usr/local/include/gtest/gtest-message.h
-- Installing: /usr/local/include/gtest/gtest-param-test.h
-- Installing: /usr/local/include/gtest/gtest-printers.h
-- Installing: /usr/local/include/gtest/gtest-spi.h
-- Installing: /usr/local/include/gtest/gtest-test-part.h
-- Installing: /usr/local/include/gtest/gtest-typed-test.h
-- Installing: /usr/local/include/gtest/gtest.h
-- Installing: /usr/local/include/gtest/gtest_pred_impl.h
-- Installing: /usr/local/include/gtest/gtest_prod.h
-- Installing: /usr/local/include/gtest/internal
-- Installing: /usr/local/include/gtest/internal/custom
-- Installing: /usr/local/include/gtest/internal/custom/README.md
-- Installing: /usr/local/include/gtest/internal/custom/gtest-port.h
-- Installing: /usr/local/include/gtest/internal/custom/gtest-printers.h
-- Installing: /usr/local/include/gtest/internal/custom/gtest.h
-- Installing: /usr/local/include/gtest/internal/gtest-death-test-internal.h
-- Installing: /usr/local/include/gtest/internal/gtest-filepath.h
-- Installing: /usr/local/include/gtest/internal/gtest-internal.h
-- Installing: /usr/local/include/gtest/internal/gtest-param-util.h
-- Installing: /usr/local/include/gtest/internal/gtest-port-arch.h
-- Installing: /usr/local/include/gtest/internal/gtest-port.h
-- Installing: /usr/local/include/gtest/internal/gtest-string.h
-- Installing: /usr/local/include/gtest/internal/gtest-type-util.h
-- Installing: /usr/local/include/gtest/internal/gtest-type-util.h.pump
-- Installing: /usr/local/lib64/libgtest.a
-- Installing: /usr/local/lib64/libgtest_main.a
-- Installing: /usr/local/lib64/pkgconfig/gtest.pc
-- Installing: /usr/local/lib64/pkgconfig/gtest_main.pc
</pre>
之后将/usr/local/lib64/pkgconfig添加到PKG_CONFIG_PATH中：

<pre>
# vi /etc/profile
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig/:/usr/local/lib64/pkgconfig/
# source /etc/profile
# echo $PKG_CONFIG_PATH
:/usr/local/lib/pkgconfig/:/usr/local/lib64/pkgconfig/
</pre>

## 2. 测试google test
下面我们编写一个Hello Google Test来进行测试（hello_test.cpp)：
{% highlight string %}
#include <gtest/gtest.h>
#include <iostream>

//待测函数
int func(int a){
	return a + 1;
}

//单元测试
TEST(FunTest, HandlesZeroInput){
	EXPECT_EQ(1, func(0));
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
{% endhighlight %}

执行如下命令进行编译：
<pre>
# gcc -o hello_test hello_test.cpp -lstdc++ -std=c++11 `pkg-config --cflags --libs gtest`
# ./hello_test 
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from FunTest
[ RUN      ] FunTest.HandlesZeroInput
[       OK ] FunTest.HandlesZeroInput (0 ms)
[----------] 1 test from FunTest (0 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (0 ms total)
[  PASSED  ] 1 test.
</pre>
上面可以看到程序运行成功。

## 3. gtest运行原理
关于gtest的运行原理，我们先来googletest源码中```TEST```宏定义的实现：
{% highlight string %}
// Defines a test.
//
// The first parameter is the name of the test suite, and the second
// parameter is the name of the test within the test suite.
//
// The convention is to end the test suite name with "Test".  For
// example, a test suite for the Foo class can be named FooTest.
//
// Test code should appear between braces after an invocation of
// this macro.  Example:
//
//   TEST(FooTest, InitializesCorrectly) {
//     Foo foo;
//     EXPECT_TRUE(foo.StatusIsOK());
//   }

// Note that we call GetTestTypeId() instead of GetTypeId<
// ::testing::Test>() here to get the type ID of testing::Test.  This
// is to work around a suspected linker bug when using Google Test as
// a framework on Mac OS X.  The bug causes GetTypeId<
// ::testing::Test>() to return different values depending on whether
// the call is from the Google Test framework itself or from user test
// code.  GetTestTypeId() is guaranteed to always return the same
// value, as it always calls GetTypeId<>() from the Google Test
// framework.
#define GTEST_TEST(test_suite_name, test_name)             \
  GTEST_TEST_(test_suite_name, test_name, ::testing::Test, \
              ::testing::internal::GetTestTypeId())

// Define this macro to 1 to omit the definition of TEST(), which
// is a generic name and clashes with some other libraries.
#if !GTEST_DONT_DEFINE_TEST
#define TEST(test_suite_name, test_name) GTEST_TEST(test_suite_name, test_name)
#endif

// Defines a test that uses a test fixture.
//
// The first parameter is the name of the test fixture class, which
// also doubles as the test suite name.  The second parameter is the
// name of the test within the test suite.
//
// A test fixture class must be declared earlier.  The user should put
// the test code between braces after using this macro.  Example:
//
//   class FooTest : public testing::Test {
//    protected:
//     void SetUp() override { b_.AddElement(3); }
//
//     Foo a_;
//     Foo b_;
//   };
//
//   TEST_F(FooTest, InitializesCorrectly) {
//     EXPECT_TRUE(a_.StatusIsOK());
//   }
//
//   TEST_F(FooTest, ReturnsElementCountCorrectly) {
//     EXPECT_EQ(a_.size(), 0);
//     EXPECT_EQ(b_.size(), 1);
//   }
//
// GOOGLETEST_CM0011 DO NOT DELETE
#define TEST_F(test_fixture, test_name)\
  GTEST_TEST_(test_fixture, test_name, test_fixture, \
              ::testing::internal::GetTypeId<test_fixture>())
{% endhighlight %}

TEST是一个宏定义，而```GTEST_TEST_```也是一个宏，其会根据```test_fixture```以及```test_name```字符串拼接出一个class name，然后会采用该class生成相应的对象登记到google测试框架中。从而在调用RUN_ALL_TESTS()时就会一个个的执行相应的测试用例。例如:
{% highlight string %}
TEST_F(FooTest, InitializesCorrectly) {
     EXPECT_TRUE(a_.StatusIsOK());
}

//展开后会变成：
class FooTest_InitializesCorrectly_Test : public FooTest{
public:
	FooTest_InitializesCorrectly_Test(FooTest, InitializesCorrectly){}

private:                                                                 \
    void TestBody() override; 
	
	static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_;
	GTEST_DISALLOW_COPY_AND_ASSIGN_(GTEST_TEST_CLASS_NAME_(test_suite_name,test_name));
};

//然后再构造FooTest_InitializesCorrectly_Test对象，然后调用TestBody()方法。其中TestBody()方法其实就是通过宏拼凑出一个类似
//如下的函数
void FooTest_InitializesCorrectly_Test::TestBody(){
	EXPECT_TRUE(a_.StatusIsOK());
}
{% endhighlight %}



因为上面每个拼接生成的类都会继承自testing::Test，因此这里提供了另外一个宏定义```TEST_F```允许我们对testing::Testing做一些相应的修改，即我们可以通过继承testing::Test来对相应的成员函数进行重载，从而满足一些特定的需求。

## 4. 断言/宏测试
Google Test采用一系列的断言(assert)来进行代码测试，这些宏有点类似于函数调用。当断言失败时，Google Test将会打印出assertion时的源文件和出错行的位置，以及附加的失败信息，用户可以直接通过```<<```在这些断言宏后面跟上自己希望在断言命中时的输出信息。

测试宏可以分为两大类： ```ASSERT_*```和```EXPECT_*```，这些成对的断言功能相同，但效果不同。其中```ASSERT_*```将会在失败时产生致命错误并中止当前调用它的函数的执行。```EXPECT_*```版本的会生成非致命错误，不会终止当前函数，而是继续执行当前函数。通常情况，应该首选使用```EXPECT_*```，因为```ASSERT_*```在报告完错误后不会进行清理工作，可能导致内存泄露问题。

* **基本断言**
{% highlight string %}
   Fatal assertion                 Nonfatal assertion                         verifies
------------------------------------------------------------------------------------------------------
ASSERT_TRUE(condition)          EXPECT_TRUE(condition)                      condition is true
ASSERT_FALSE(condition)         EXPECT_FALSE(condition)                     condition is false
{% endhighlight %}

* **二值比较**
{% highlight string %}
   Fatal assertion                 Nonfatal assertion                         verifies
------------------------------------------------------------------------------------------------------
ASSERT_EQ(val1, val2)          EXPECT_EQ(val1,val2)                          val1 == val2
ASSERT_NE(val1, val2)          EXPECT_NE(val1,val2)                          val1 != val2
ASSERT_LT(val1, val2)          EXPECT_LT(val1, val2)                         val1 < val2
ASSERT_LE(val1, val2)          EXPECT_LE(val1, val2)                         val1 <= val2
ASSERT_GT(val1, val2)          EXPECT_GT(val1, val2)                         val1 > val2
ASSERT_GE(val1, val2)          EXPECT_GE(val1, val2)                         val1 >= val2
{% endhighlight %}

* **字符串比较**
{% highlight string %}
   Fatal assertion                 Nonfatal assertion                         verifies
-------------------------------------------------------------------------------------------------------------------------
ASSERT_STREQ(str1, str2)         EXPECT_STREQ(str1, str2)          the two C strings have the same content
ASSERT_STRNE(str1, str2)         EXPECT_STRNE(str1, str2)          the two C strings have different content
ASSERT_STRCASEEQ(str1, str2)     EXPECT_STRCASEEQ(str1, str2)      the two C strings have the same content, ignoring case 
ASSERT_STRCASENE(str1, str2)     EXPECT_STRCASENE(str1, str2)      the two C strings have different content, ignoring case
{% endhighlight %}


## 5. 事件机制
1) **全局事件**

要实现全局事件，必须写一个类，继承testing::Environment类，实现里面的SetUp()和TearDown()方法。

* Setup()方法在所有案例执行前执行

* TearDown()方法在所有案例执行后执行

还需要告诉gtest添加这个全局事件，我们需要在main函数中通过testing::AddGlobalTestEnvironment()方法将事件挂进来，也就是说，我们可以写很多个这样的类，然后把它们的事件都挂上去。

2） **TestSuit事件**

我们需要写一个类，继承testing::Test，然后实现两个静态方法：

* SetupTestCase()该方法在第一个TestCase之前执行

* TearDownTestCase()该方法在最后一个TestCase之后执行

在编写测试案例时，我们需要使用```TEST_F```这个宏，第一个参数必须是我们上面类的名字，代表一个TestSuit。


3） **TestCase事件**

TestCase事件是挂在每个案例执行前后的，实现方式和上面的几乎一样，不过需要实现的是Setup()方法和TearDown()方法：

* Setup()方法在每个TestCase之前执行

* TearDown()方法在每个TestCase之后执行


以下案例说明上述三个事件的使用：
{% highlight string %}
#include<gtest/gtest.h>
#include<map>
#include<iostream>
using namespace std;


class Student{
public:
	Student(){
		age=0;
	}
	Student(int a){
		age=a;
	}
	void print(){
		cout<<"*********** "<<age<<" **********"<<endl;;
	}  
	
private:
    int age;
};

class FooEnvironment : public testing::Environment{
public:
	virtual void SetUp()
	{
		std::cout << "Foo FooEnvironment SetUP" << std::endl;
	}
	virtual void TearDown()
	{
		std::cout << "Foo FooEnvironment TearDown" << std::endl;
	}
};

static Student *s = NULL;

//在第一个test之前，最后一个test之后调用SetUpTestCase()和TearDownTestCase()
class TestMap:public testing::Test
{
public:
	static void SetUpTestCase()
	{
		cout<<"SetUpTestCase()"<<endl;
		s=new Student(23);
	}
 
	static void TearDownTestCase()
	{
		delete s;
		cout<<"TearDownTestCase()"<<endl;
	}
	void SetUp()
	{
		cout<<"SetUp() is running"<<endl;         
	}
	void TearDown()
	{
		cout<<"TearDown()"<<endl;
	} 
	
};

TEST_F(TestMap, test1){
	cout<<"this is test1"<<endl;
	s->print();
	cout<<"end test1"<<endl;
}
TEST_F(TestMap, test2){
	cout<<"this is test2"<<endl;
	s->print();
	cout<<"end test2"<<endl;
}

int main(int argc, char *argv[]){
	testing::AddGlobalTestEnvironment(new FooEnvironment);
	testing::InitGoogleTest(&argc, argv);
	
	return RUN_ALL_TESTS();
}

{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11 `pkg-config --cflags --libs gtest`
# ./test 
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
Foo FooEnvironment SetUP
[----------] 2 tests from TestMap
SetUpTestCase()
[ RUN      ] TestMap.test1
SetUp() is running
this is test1
*********** 23 **********
end test1
TearDown()
[       OK ] TestMap.test1 (0 ms)
[ RUN      ] TestMap.test2
SetUp() is running
this is test2
*********** 23 **********
end test2
TearDown()
[       OK ] TestMap.test2 (1 ms)
TearDownTestCase()
[----------] 2 tests from TestMap (1 ms total)

[----------] Global test environment tear-down
Foo FooEnvironment TearDown
[==========] 2 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 2 tests.
</pre>

## 6. 参数化
当考虑多次要为被测函数传入不同的值的情况时，可以按下面的方法去测试。必须添加一个类，继承自testing::TestWithParam<T>。其中T就是你需要参数化的参数模型。下面的案例是int参数（官方文档上的案例）:
{% highlight string %}
#include<gtest/gtest.h>


// Returns true iff n is a prime number.
bool IsPrime(int n)
{
	// Trivial case 1: small numbers
	if (n <= 1) return false;

	// Trivial case 2: even numbers
	if (n % 2 == 0) return n == 2;


	// Now, we have that n is odd and n >= 3.
	// Try to divide n by every odd number i, starting from 3
	for (int i = 3; ; i += 2) {
		// We only have to try i up to the squre root of n
		if (i > n/i) break;

		// Now, we have i <= n/i < n.
		// If n is divisible by i, n is not prime.
		if (n % i == 0) return false;
	}
	
	// n has no integer factor in the range (1, n), and thus is prime.
	return true;
}

bool isEven(int n)
{
	return n % 2 == 0 ? true : false;
}


class MyParamTest : public::testing::TestWithParam<int>{};
TEST_P(MyParamTest, CheckPrimer)
{
	int n =  GetParam();
	EXPECT_TRUE(IsPrime(n));
}

TEST_P(MyParamTest, CheckEven)
{
	int n = GetParam();
	EXPECT_TRUE(isEven(n));
}

//被测函数须传入多个相关的值
INSTANTIATE_TEST_CASE_P(NumberTest, MyParamTest, testing::Values(3, 5, 11, 23, 17));
int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11 `pkg-config --cflags --libs gtest`
# ./test
[==========] Running 5 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 5 tests from TrueReturn/IsPrimeParamTest
[ RUN      ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/0
[       OK ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/0 (0 ms)
[ RUN      ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/1
[       OK ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/1 (0 ms)
[ RUN      ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/2
[       OK ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/2 (0 ms)
[ RUN      ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/3
[       OK ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/3 (0 ms)
[ RUN      ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/4
[       OK ] TrueReturn/IsPrimeParamTest.HandleTrueReturn/4 (0 ms)
[----------] 5 tests from TrueReturn/IsPrimeParamTest (0 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 5 tests.
</pre>

<br />
<br />

**[参看]**

1. [googletest （gtest）的使用方法](https://blog.csdn.net/baijiwei/article/details/81265491)

2. [Linux install googletest](https://www.jianshu.com/p/65eb22508441)

3. [CMake初步](https://www.cnblogs.com/lidabo/p/3976947.html)

4. [Google Test源码浅析(三) -------- RUN_ALL_TESTS](https://blog.csdn.net/zhangye3017/article/details/81206059)

5. [Gtest的安装与使用](https://www.cnblogs.com/helloworldcode/p/9606838.html)

<br />
<br />
<br />


