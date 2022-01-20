---
layout: post
title: STL源码分析学习笔记(1)
tags:
- cplusplus
categories: cplusplus
description: STL源码分析学习笔记
---


本文是阅读侯捷老师《STL源码剖析》书籍的一个笔记，在此做个记录，以便后续查阅。本文以```SGI STL v3.3```进行讲述。 



<!-- more -->

## 1. STL六大组件 - 功能与运用
STL提供六大组件，彼此可以组合套用：

1）容器(containers): 各种数据结构，如vector，list，deque，set，map，用来存放数据。从实现的角度来看，STL容器是一种class template。

2）算法(algorithms): 各种常用算法如sort，search，copy，erase...从实现的角度来看，STL算法是一种function template。

3）迭代器(iterators): 扮演容器与算法之间的胶合剂，是所谓的“泛型指针”。共有五种类型，以及其他衍生变化。从实现的角度来看，迭代器是一种将```operator *```、```operator->```、```operator++```、```operator--```等指针相关操作予以重载class templete。所有STL容器都附带有自己专属的迭代器————是的，只有容器设计者才直到如何遍历自己的元素。原生指针(native pointer)也是一种迭代器。

4）仿函数(functors): 行为类似于函数，可作为算法的某种策略(policy)。从实现的角度来看，仿函数是一种重载了operator()的class或class template。一般函数指针可视为侠义的仿函数。

5）配接器(adapters): 一种用来修饰容器(containers)或仿函数(functors)或迭代器(iterators)接口的东西。例如，STL提供的queue和stack，虽然看似容器，其实只能算一种容器适配器，因为它们的底部完全借助deque，所有操作都由底层的deque供应。改变functor接口者，称为functor adapter；改变container接口者，称为container adapter；改变iterator接口者，称为iterator adapter。

6）配置器(allocators): 负责空间配置和管理。从实现的角度来看，配置器是一个实现了动态空间配置、空间管理、空间释放的class template。

下图显式了STL六大组件的交互关系图：

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part1_1.png)

>注：《STL源代码分析》中是以SGI STL来进行讲解的，我们可以到[Download STL source code](http://labmaster.mi.infn.it/Laboratorio2/serale/www.sgi.com/tech/stl/download.html)进行下载

当前最新版SGI STL v3.3头文件如下：
<pre>
algo.h                hash_map.h     numeric               stdexcept              stl_heap.h               stl_slist.h
algobase.h            hash_set       pair.h	               stl_algo.h	          stl_iterator.h           stl_stack.h
algorithm             hash_set.h     pthread_alloc         stl_algobase.h         stl_iterator_base.h      stl_string_fwd.h
alloc.h               hashtable.h    pthread_alloc.h       stl_alloc.h            stl_list.h               stl_tempbuf.h
bitset                heap.h         queue                 stl_bvector.h          stl_map.h                stl_threads.h
bvector.h             iterator       rope                  stl_config.h	          stl_multimap.h           stl_tree.h
char_traits.h         iterator.h     rope.h                stl_construct.h        stl_multiset.h           stl_uninitialized.h
concept_checks.h      limits         ropeimpl.h            stl_ctraits_fns.h      stl_numeric.h            stl_vector.h
container_concepts.h  list           sequence_concepts.h   stl_deque.h            stl_pair.h               string
defalloc.h            list.h         set                   stl_exception.h        stl_queue.h              tempbuf.h
deque                 map            set.h                 stl_function.h         stl_range_errors.h       tree.h
deque.h	              map.h          slist                 stl_hash_fun.h         stl_raw_storage_iter.h   type_traits.h
function.h            memory         slist.h	           stl_hash_map.h         stl_relops.h	           utility
functional            multimap.h     stack                 stl_hash_set.h         stl_rope.h	           valarray
hash_map              multiset.h     stack.h	           stl_hashtable.h        stl_set.h                vector
</pre>

如何在当前自己的Linux操作系统上找到stl头文件的位置呢？ 我们可以简单编写一个测试代码```test.cpp```:
{% highlight string %}
#include <stdio.h>
#include <vector>

int main(int argc, char *argv[])
{
        std::vector<int> vec;

        vec.push_back(2);

        return 0x0;
}
{% endhighlight %}
执行如下编译命令：
{% highlight string %}
# gcc -v -o test test.cpp -lstdc++
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/libexec/gcc/x86_64-redhat-linux/4.8.5/lto-wrapper
Target: x86_64-redhat-linux
Configured with: ../configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=http://bugzilla.redhat.com/bugzilla --enable-bootstrap --enable-shared --enable-threads=posix --enable-checking=release --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-gnu-unique-object --enable-linker-build-id --with-linker-hash-style=gnu --enable-languages=c,c++,objc,obj-c++,java,fortran,ada,go,lto --enable-plugin --enable-initfini-array --disable-libgcj --with-isl=/builddir/build/BUILD/gcc-4.8.5-20150702/obj-x86_64-redhat-linux/isl-install --with-cloog=/builddir/build/BUILD/gcc-4.8.5-20150702/obj-x86_64-redhat-linux/cloog-install --enable-gnu-indirect-function --with-tune=generic --with-arch_32=x86-64 --build=x86_64-redhat-linux
Thread model: posix
gcc version 4.8.5 20150623 (Red Hat 4.8.5-44) (GCC) 
COLLECT_GCC_OPTIONS='-v' '-o' 'test' '-mtune=generic' '-march=x86-64'
 /usr/libexec/gcc/x86_64-redhat-linux/4.8.5/cc1plus -quiet -v -D_GNU_SOURCE test.cpp -quiet -dumpbase test.cpp -mtune=generic -march=x86-64 -auxbase test -version -o /tmp/ccF9VG8N.s
GNU C++ (GCC) version 4.8.5 20150623 (Red Hat 4.8.5-44) (x86_64-redhat-linux)
        compiled by GNU C version 4.8.5 20150623 (Red Hat 4.8.5-44), GMP version 6.0.0, MPFR version 3.1.1, MPC version 1.0.1
GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
ignoring nonexistent directory "/usr/lib/gcc/x86_64-redhat-linux/4.8.5/include-fixed"
ignoring nonexistent directory "/usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../x86_64-redhat-linux/include"
#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../include/c++/4.8.5
 /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../include/c++/4.8.5/x86_64-redhat-linux
 /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../include/c++/4.8.5/backward
 /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include
 /usr/local/include
 /usr/include
End of search list.
GNU C++ (GCC) version 4.8.5 20150623 (Red Hat 4.8.5-44) (x86_64-redhat-linux)
        compiled by GNU C version 4.8.5 20150623 (Red Hat 4.8.5-44), GMP version 6.0.0, MPFR version 3.1.1, MPC version 1.0.1
GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
Compiler executable checksum: 51b2dcccf6085e5bfbbf3932e5685252
COLLECT_GCC_OPTIONS='-v' '-o' 'test' '-mtune=generic' '-march=x86-64'
 as -v --64 -o /tmp/ccaJ99rj.o /tmp/ccF9VG8N.s
GNU assembler version 2.27 (x86_64-redhat-linux) using BFD version version 2.27-43.base.el7_8.1
COMPILER_PATH=/usr/libexec/gcc/x86_64-redhat-linux/4.8.5/:/usr/libexec/gcc/x86_64-redhat-linux/4.8.5/:/usr/libexec/gcc/x86_64-redhat-linux/:/usr/lib/gcc/x86_64-redhat-linux/4.8.5/:/usr/lib/gcc/x86_64-redhat-linux/
LIBRARY_PATH=/usr/lib/gcc/x86_64-redhat-linux/4.8.5/:/usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../lib64/:/lib/../lib64/:/usr/lib/../lib64/:/usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../:/lib/:/usr/lib/
COLLECT_GCC_OPTIONS='-v' '-o' 'test' '-mtune=generic' '-march=x86-64'
 /usr/libexec/gcc/x86_64-redhat-linux/4.8.5/collect2 --build-id --no-add-needed --eh-frame-hdr --hash-style=gnu -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../lib64/crt1.o /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../lib64/crti.o /usr/lib/gcc/x86_64-redhat-linux/4.8.5/crtbegin.o -L/usr/lib/gcc/x86_64-redhat-linux/4.8.5 -L/usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../.. /tmp/ccaJ99rj.o -lstdc++ -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-redhat-linux/4.8.5/crtend.o /usr/lib/gcc/x86_64-redhat-linux/4.8.5/../../../../lib64/crtn.o
{% endhighlight %}
查看如下目录就可以找到stl相关头文件了：
<pre>
# ls /usr/include/c++/4.8.5/ 
algorithm  cerrno     complex             cstdint   debug         future            list      profile           stdexcept     typeindex
array      cfenv      complex.h           cstdio    decimal       initializer_list  locale    queue             streambuf     typeinfo
atomic     cfloat     condition_variable  cstdlib   deque         iomanip           map       random            string        type_traits
backward   chrono     csetjmp             cstring   exception     ios               memory    ratio             system_error  unordered_map
bits       cinttypes  csignal             ctgmath   ext           iosfwd            mutex     regex             tgmath.h      unordered_set
bitset     ciso646    cstdalign           ctime     fenv.h        iostream          new       scoped_allocator  thread        utility
cassert    climits    cstdarg             cwchar    forward_list  istream           numeric   set               tr1           valarray
ccomplex   clocale    cstdbool            cwctype   fstream       iterator          ostream   sstream           tr2           vector
cctype     cmath      cstddef             cxxabi.h  functional    limits            parallel  stack             tuple         x86_64-redhat-linux

# ls /usr/include/c++/4.8.5/bits/
algorithmfwd.h             exception_defines.h  locale_classes.tcc       regex_compiler.h        stl_deque.h                stl_tempbuf.h
allocator.h                exception_ptr.h      locale_facets.h          regex_constants.h       stl_function.h             stl_tree.h
alloc_traits.h             forward_list.h       locale_facets_nonio.h    regex_cursor.h          stl_heap.h                 stl_uninitialized.h
atomic_base.h              forward_list.tcc     locale_facets_nonio.tcc  regex_error.h           stl_iterator_base_funcs.h  stl_vector.h
atomic_lockfree_defines.h  fstream.tcc          locale_facets.tcc        regex_grep_matcher.h    stl_iterator_base_types.h  streambuf_iterator.h
basic_ios.h                functexcept.h        localefwd.h              regex_grep_matcher.tcc  stl_iterator.h             streambuf.tcc
basic_ios.tcc              functional_hash.h    mask_array.h             regex.h                 stl_list.h                 stream_iterator.h
basic_string.h             gslice_array.h       memoryfwd.h              regex_nfa.h             stl_map.h                  stringfwd.h
basic_string.tcc           gslice.h             move.h                   regex_nfa.tcc           stl_multimap.h             unique_ptr.h
boost_concept_check.h      hash_bytes.h         nested_exception.h       shared_ptr_base.h       stl_multiset.h             unordered_map.h
c++0x_warning.h            hashtable.h          ostream_insert.h         shared_ptr.h            stl_numeric.h              unordered_set.h
char_traits.h              hashtable_policy.h   ostream.tcc              slice_array.h           stl_pair.h                 uses_allocator.h
codecvt.h                  indirect_array.h     postypes.h               sstream.tcc             stl_queue.h                valarray_after.h
concept_check.h            ios_base.h           ptr_traits.h             stl_algobase.h          stl_raw_storage_iter.h     valarray_array.h
cpp_type_traits.h          istream.tcc          random.h                 stl_algo.h              stl_relops.h               valarray_array.tcc
cxxabi_forced.h            list.tcc             random.tcc               stl_bvector.h           stl_set.h                  valarray_before.h
deque.tcc                  locale_classes.h     range_access.h           stl_construct.h         stl_stack.h                vector.tcc
</pre>

## 2. SGI STL的编译器组态设置(configuration)
不同的编译器对C++语言的支持程度不尽相同。作为一个希望具备广泛移植能力的程序库，SGI STL准备了一个环境组态文件```<stl_config.h>```，其中定义了许多常量，标示某些组态的成立与否。所有STL头文件都会直接或间接包含这个组态文件，并以条件式写法，让预处理器(pre-processor)根据各个常量决定取舍哪一段程序代码。例如：

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part1_2.png)

```<stl_config.h>```文件起始处有一份常量定义说明，然后即针对各家不同的编译器以及可能的不同版本，给予常量定义。从这里我们可以一窥各家编译器对标准C++的支持成都。当然，随着版本的演进，这些组态都有可能改变。如下我们列出一些在SGI STL v3.3版本中```<stl_config.h>```的组态：
{% highlight string %}
#ifndef __STL_CONFIG_H
# define __STL_CONFIG_H

// Flags:
// * __STL_NO_BOOL: defined if the compiler doesn't have bool as a builtin
//   type.
// * __STL_HAS_WCHAR_T: defined if the compier has wchar_t as a builtin type.
// * __STL_NO_DRAND48: defined if the compiler doesn't have the drand48 
//   function.
// * __STL_STATIC_TEMPLATE_MEMBER_BUG: defined if the compiler can't handle
//   static members of template classes.
// * __STL_STATIC_CONST_INIT_BUG: defined if the compiler can't handle a
//   constant-initializer in the declaration of a static const data member
//   of integer type.  (See section 9.4.2, paragraph 4, of the C++ standard.)
// * __STL_CLASS_PARTIAL_SPECIALIZATION: defined if the compiler supports
//   partial specialization of template classes.
// * __STL_PARTIAL_SPECIALIZATION_SYNTAX: defined if the compiler 
//   supports partial specialization syntax for full specialization of
//   class templates.  (Even if it doesn't actually support partial 
//   specialization itself.)
// * __STL_FUNCTION_TMPL_PARTIAL_ORDER: defined if the compiler supports
//   partial ordering of function templates.  (a.k.a partial specialization
//   of function templates.)
// * __STL_MEMBER_TEMPLATES: defined if the compiler supports template
//   member functions of classes.
// * __STL_MEMBER_TEMPLATE_CLASSES: defined if the compiler supports 
//   nested classes that are member templates of other classes.
// * __STL_TEMPLATE_FRIENDS: defined if the compiler supports templatized
//   friend declarations.
// * __STL_EXPLICIT_FUNCTION_TMPL_ARGS: defined if the compiler 
//   supports calling a function template by providing its template
//   arguments explicitly.
// * __STL_LIMITED_DEFAULT_TEMPLATES: defined if the compiler is unable
//   to handle default template parameters that depend on previous template
//   parameters.
// * __STL_NON_TYPE_TMPL_PARAM_BUG: defined if the compiler has trouble with
//   function template argument deduction for non-type template parameters.
// * __SGI_STL_NO_ARROW_OPERATOR: defined if the compiler is unable
//   to support the -> operator for iterators.
// * __STL_DEFAULT_CONSTRUCTOR_BUG: defined if T() does not work properly
//   when T is a builtin type.
// * __STL_USE_EXCEPTIONS: defined if the compiler (in the current compilation
//   mode) supports exceptions.
// * __STL_USE_NAMESPACES: defined if the compiler has the necessary
//   support for namespaces.
// * __STL_NO_EXCEPTION_HEADER: defined if the compiler does not have a
//   standard-conforming header <exception>.
// * __STL_NO_BAD_ALLOC: defined if the compiler does not have a <new>
//   header, or if <new> does not contain a bad_alloc class.  If a bad_alloc
//   class exists, it is assumed to be in namespace std.
// * __STL_SGI_THREADS: defined if this is being compiled for an SGI IRIX
//   system in multithreaded mode, using native SGI threads instead of 
//   pthreads.
// * __STL_WIN32THREADS: defined if this is being compiled on a WIN32
//   compiler in multithreaded mode.
// * __STL_PTHREADS: defined if we should use portable pthreads
//   synchronization.
// * __STL_UITHREADS: defined if we should use UI / solaris / UnixWare threads
//   synchronization.  UIthreads are similar to pthreads, but are based 
//   on an earlier version of the Posix threads standard.
// * __STL_LONG_LONG if the compiler has long long and unsigned long long
//   types.  (They're not in the C++ standard, but they are expected to be 
//   included in the forthcoming C9X standard.)
// * __STL_THREADS is defined if thread safety is needed.
// * __STL_VOLATILE is defined to be "volatile" if threads are being
//   used, and the empty string otherwise.
// * __STL_USE_CONCEPT_CHECKS enables some extra compile-time error
//   checking to make sure that user-defined template arguments satisfy
//   all of the appropriate requirements.  This may result in more
//   comprehensible error messages.  It incurs no runtime overhead.  This 
//   feature requires member templates and partial specialization.
// * __STL_NO_USING_CLAUSE_IN_CLASS: The compiler does not handle "using"
//   clauses inside of class definitions.
// * __STL_NO_FRIEND_TEMPLATE_CLASS: The compiler does not handle friend
//   declaractions where the friend is a template class.
// * __STL_NO_FUNCTION_PTR_IN_CLASS_TEMPLATE: The compiler does not
//   support the use of a function pointer type as the argument
//   for a template.
// * __STL_MEMBER_TEMPLATE_KEYWORD: standard C++ requires the template
//   keyword in a few new places (14.2.4).  This flag is set for
//   compilers that support (and require) this usage.


// User-settable macros that control compilation:
// * __STL_USE_SGI_ALLOCATORS: if defined, then the STL will use older
//   SGI-style allocators, instead of standard-conforming allocators,
//   even if the compiler supports all of the language features needed
//   for standard-conforming allocators.
// * __STL_NO_NAMESPACES: if defined, don't put the library in namespace
//   std, even if the compiler supports namespaces.
// * __STL_NO_RELOPS_NAMESPACE: if defined, don't put the relational
//   operator templates (>, <=. >=, !=) in namespace std::rel_ops, even
//   if the compiler supports namespaces and partial ordering of
//   function templates.
// * __STL_ASSERTIONS: if defined, then enable runtime checking through the
//   __stl_assert macro.
// * _PTHREADS: if defined, use Posix threads for multithreading support.
// * _UITHREADS:if defined, use SCO/Solaris/UI threads for multithreading 
//   support
// * _NOTHREADS: if defined, don't use any multithreading support.  
// * _STL_NO_CONCEPT_CHECKS: if defined, disables the error checking that
//   we get from __STL_USE_CONCEPT_CHECKS.
// * __STL_USE_NEW_IOSTREAMS: if defined, then the STL will use new,
//   standard-conforming iostreams (e.g. the <iosfwd> header).  If not
//   defined, the STL will use old cfront-style iostreams (e.g. the
//   <iostream.h> header).

// Other macros defined by this file:

// * bool, true, and false, if __STL_NO_BOOL is defined.
// * typename, as a null macro if it's not already a keyword.
// * explicit, as a null macro if it's not already a keyword.
// * namespace-related macros (__STD, __STL_BEGIN_NAMESPACE, etc.)
// * exception-related macros (__STL_TRY, __STL_UNWIND, etc.)
// * __stl_assert, either as a test or as a null macro, depending on
//   whether or not __STL_ASSERTIONS is defined.


#endif /* __STL_CONFIG_H */

// Local Variables:
// mode:C++
// End:
{% endhighlight %}



>注：在我当前Linux主机上的GCC 4.8.5，并未看到有stl_config.h头文件，其应该是移植过后的，当前新版GCC应该对大部分特性都支持


## 3. 可能令你困惑的C++语法
如下我们给出一些例子来测试一下gcc对templete参数推导(argument deduction)、偏特化(partial specification)等的支持情况。

1）**组态：__STL_STATIC_TEMPLATE_MEMBER_BUG**

如下我们测试一下GCC 4.8.5编译器对```__STL_STATIC_TEMPLATE_MEMBER_BUG```的支持：
{% highlight string %}
#include <iostream>
using namespace std;

template <typename T>
class testClass{
public:                    //纯粹为了方便测试，使用public
	static int _data;
};

//为static data members 进行定义（配置内存），并设置初值
int testClass<int>::_data = 1;
int testClass<char>::_data = 2;


int main(int argc, char *argv[])
{
	cout<<testClass<int>::_data<<endl;
	cout<<testClass<char>::_data<<endl;
	
	testClass<int> obji1, obji2;
	testClass<char> objc1, objc2;
	
	cout<<obji1._data<<endl;
	cout<<obji2._data<<endl;
	cout<<objc1._data<<endl;
	cout<<objc2._data<<endl;
	
	obji1._data = 3;
	objc2._data = 4;
	
	cout<<obji1._data<<endl;
	cout<<obji2._data<<endl;
	cout<<objc1._data<<endl;
	cout<<objc2._data<<endl;
	
	return 0x0;
}
{% endhighlight %}
编译：
{% highlight string %}
# gcc -o test test.cpp -lstdc++
test.cpp:11:5: error: specializing member ‘testClass<int>::_data’ requires ‘template<>’ syntax
 int testClass<int>::_data = 1;
     ^
test.cpp:12:5: error: specializing member ‘testClass<char>::_data’ requires ‘template<>’ syntax
 int testClass<char>::_data = 2;
     ^
{% endhighlight %}

从上面我们可以看到，GCC 4.8.5已经不支持这种写法了，需要将初始化改为如下：
{% highlight string %}
//为static data members 进行定义（配置内存），并设置初值
template <> int testClass<int>::_data = 1;
template <> int testClass<char>::_data = 2;
{% endhighlight %}

然后再执行编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
1
2
1
1
2
2
3
3
4
4
</pre>
从上面测试结果可以看到，对于GCC 4.8.5，是支持```__STL_STATIC_TEMPLATE_MEMBER_BUG```的，只是与原来老的初始化方法有些区别。

2）**组态：__STL_CLASS_PARTIAL_SPECIALIZATION**

如下我们测试一下GCC 4.8.5编译器对```__STL_CLASS_PARTIAL_SPECIALIZATION```的支持：
{% highlight string %}
#include <iostream>
using namespace std;


//一般化设计
template <class I, class O>
struct testClass
{
  testClass() {cout<<"I, O"<<endl; }
};

//特殊化设计
template <class T>
struct testClass<T *, T*>
{
  testClass() {cout<<"T *, T*"<<endl; }
};

//特殊化设计
template <class T>
struct testClass<const T *, T*>
{
  testClass() {cout<<"const T *, T*"<<endl; }
};


int main(int argc, char *argv[])
{
	testClass<int, char> obj1;
	testClass<int *, int *> obj2;
	testClass<const int *, int *> obj3;
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
I, O
T *, T*
const T *, T*
</pre>
可以看到，GCC 4.8.5对于```__STL_CLASS_PARTIAL_SPECIALIZATION```的支持还是良好的。

3）**组态：__STL_FUNCTION_TMPL_PARTIAL_ORDER**

如下我们测试一下GCC 4.8.5编译器对```__STL_FUNCTION_TMPL_PARTIAL_ORDER```的支持。请注意，虽然```<stl_config.h>```文件中声明，这个常量的意义就是partial specialization of function templates，但其实两者并不相同。前者意义如下所示，后者的实际意义请参考C++语法书籍。

>参看:
> [Partial Ordering of Function Templates (C++)](http://msdn.microsoft.com/zh-cn/library/zaycz069.aspx)
>
> [C++ Template](https://www.linuxprobe.com/c-templates-edition.html)

{% highlight string %}
#include <iostream>
using namespace std;

class alloc{
};

template <class T, class Alloc = alloc>
class vector{
public:
	void swap(vector<T, Alloc> &){
		cout<<"swap()"<<endl;
	}
};

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER                     //只为说明，非本程序内容
template<class T, class Alloc>
inline void swap(vector<T, Alloc> &x, vector<T, Alloc> &y){
	x.swap(y);
}
#endif                                                       //只为说明，非本程序内容


//以上节录自stl_vector.h，灰色部分系源代码中的条件编译，非本测试程序内容
int main(int argc, char *argv[])
{
	vector<int> x, y;
	
	swap(x, y);
	
	return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
</pre>
看到没有任何输出结果。

4) **组态：__STL_EXPLICIT_FUNCTION_TMPL_ARGS**

如下我们测试一下GCC 4.8.5编译器对```__STL_EXPLICIT_FUNCTION_TMPL_ARGS```的支持。关于该字段，注释中对其的解释为：defined if the compiler supports calling a function template by providing its template arguments explicitly. 

>注：整个SGI STL内都每用到这一常量定义

{% highlight string %}
#include <iostream>
using namespace std;

template <class T>
class vector{
public:
	template <class TT> 
	void test(TT a, TT b) {
		cout << a << ' ' << b << endl; 
	}
};


int main(int argc, char *argv[])
{
	vector<int> vec;
	
	vec.test<int>(3, 4);
	
	return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
3 4
</pre>

5) **组态：__STL_MEMBER_TEMPLATES**

关于```__STL_MEMBER_TEMPLATES```对其相关解释为：defined if the compiler supports template member functions of classes。
{% highlight string %}
//file: test.cpp
//测试 class template之内是否可再有template (members)

#include <iostream>
using namespace std;

class alloc{
};

template <class T, class Alloc = alloc>
class vector{
public:
	typedef T value_type;
	typedef value_type * iterator;
	
	template <class I>
	void insert(iterator position, I first, I last){
		cout<<"insert()"<<endl;
	}
};

int main(int argc, char *argv[]){
	int ia[5] = {0,1,2,3,4};
	
	vector<int> x;
	vector<int>::iterator ite;
	
	x.insert(ite, ia, ia + 5);
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
insert()
</pre>


6) **组态：__STL_LIMITED_DEFAULT_TEMPLATES**

关于```__STL_LIMITED_DEFAULT_TEMPLATES```对其相关解释为：defined if the compiler is unable to handle default template parameters that depend on previous template
{% highlight string %}
//file: test.cpp
//测试template 参数可否根据前一个template参数而设定默认值
//ref. C++ Primer 3/e, p.816

#include <iostream>
using namespace std;

class alloc{
};

template <class T, class Alloc = alloc, size_t BufSize = 0>
class deque{
public:
	deque(){cout<<"deque"<<endl;}
};

//根据前一个参数值T，设定下一个参数Sequence的默认值为deque<t>
template <class T, class Sequence = deque<T> >
class stack{
public:
	stack(){cout<<"stack"<<endl;}
	
private:
	Sequence c;
};


int main(int argc, char *argv[]){

	stack<int> x;
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
deque
stack
</pre>


7) **组态：__STL_NON_TYPE_TMPL_PARAM_BUG**

关于```__STL_NON_TYPE_TMPL_PARAM_BUG```对其相关解释为：defined if the compiler has trouble with function template argument deduction for non-type template parameters.
{% highlight string %}
//file: test.cpp
//测试class template可否拥有non-type template参数
//ref. C++ Primer 3/e, p.825

#include <iostream>
#include <cstddef>             //for size_t 
using namespace std;

class alloc{
};

inline size_t __deque_buf_size(size_t n, size_t sz)
{
	return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
}

template <class T, class Ref, class Ptr, size_t BufSiz>
struct __deque_iterator{
	typedef __deque_iterator<T, T&, T*, BufSiz> iterator;
	typedef __deque_iterator<T, const T&, const T*, Bufsiz> const_iterator;
	
	static size_t buffer_size(){
		return __deque_buf_size(BufSiz, sizeof(T));
	}
};


template <class T, class Alloc = alloc, size_t BufSiz = 0>
class deque{
public:
	//Iterators
	typedef __deque_iterator<T, T&, T*, Bufsiz> iterator;
	
	
};

int main(int argc, char *argv[]){

	cout<<deque<int>::iterator::buffer_size() <<endl;
	cout<<deque<int, alloc, 64>::iterator::buffer_size()<<endl;
	
	return 0x0;
}
{% endhighlight %}
在gcc 4.8.5中执行编译：
{% highlight string %}
# gcc -o test test.cpp -lstdc++
test.cpp:20:50: error: ‘Bufsiz’ was not declared in this scope
  typedef __deque_iterator<T, const T&, const T*, Bufsiz> const_iterator;
                                                  ^
test.cpp:20:56: error: template argument 4 is invalid
  typedef __deque_iterator<T, const T&, const T*, Bufsiz> const_iterator;
                                                        ^
test.cpp:32:38: error: ‘Bufsiz’ was not declared in this scope
  typedef __deque_iterator<T, T&, T*, Bufsiz> iterator;
                                      ^
test.cpp:32:44: error: template argument 4 is invalid
  typedef __deque_iterator<T, T&, T*, Bufsiz> iterator;
                                            ^
test.cpp: In function ‘int main(int, char**)’:
test.cpp:39:20: error: ‘deque<int, alloc, 0ul>::iterator’ is not a class or namespace
  cout<<deque<int>::iterator::buffer_size() <<endl;
                    ^
test.cpp:40:31: error: ‘deque<int, alloc, 64ul>::iterator’ is not a class or namespace
  cout<<deque<int, alloc, 64>::iterator::buffer_size()<<endl;
{% endhighlight %}
我们看到gcc 4.8.5似乎并不支持non-type template参数


8） **组态：__STL_NULL_TMPL_ARGS**

在<stl_config.h>中关于```__STL_NULL_TMPL_ARGS```的定义如下：
{% highlight string %}
# ifdef __STL_EXPLICIT_FUNCTION_TMPL_ARGS
#   define __STL_NULL_TMPL_ARGS <>
# else
#   define __STL_NULL_TMPL_ARGS
# endif
{% endhighlight %}

这个组态常量常常出现在类似这样的场合(class template的friend函数声明):
{% highlight string %}
// in <stl_stack.h>

template <class T, class Sequence = deque<T> >
class stack{
	friend bool operator== __STL_NULL_TMPL_ARGS(const stack&, const stack&);
	friend bool operator< __STL_NULL_TMPL_ARGS(const stack&, cont stack&);
	
	...
};
{% endhighlight %}

展开后就变成了：
{% highlight string %}
template <class T, class Sequence = deque<T> >
class stack{
	friend bool operator== <> (const stack&, const stack&);
	friend bool operator< <> (const stack&, cont stack&);
	
	...
};
{% endhighlight %}

这种奇特的语法是为了实现所谓的bound friend templates，也就是说class template的某个具现体(instantiation)与其friend function template的某个具现体有一对一的关系。下面是一个测试程序：
{% highlight string %}
//file: test.cpp
//测试__STL_NULL_TMPL_ARGS in <stl_config.h>
//ref. C++ Primer 3/e, p.834: bound friend function template

#include <iostream>
#include <cstddef>             //for size_t
using namespact std;

class alloc{
};

template <class T, class Alloc = alloc, size_t BufSiz = 0>
class deque{
public:
	deque(){
		cout<<"deque"<<endl;
	}
};

//以下声明如果不出现，GCC也可以通过；如果出现，GCC也可以通过。这一点和
//C++ Primer 3/4 p834的说法有出入。书上说一定要有这些前置声明
/*
template <class T, class Sequence>
class stack;

template <class T, class Sequence>
bool operator==(const stack<T, Sequence>& x, const stack<T, Sequence> &y);

template <class T, class Sequence>
bool operator<const stack<T, Sequence>& x, const stack<T, Sequence> &y);
*/


template <class T, class Sequence = deque<T> >
class stack{
	//写成这样是可以的
	friend bool operator== <T> (const stack<T> &, const stack<T> &);
	friend bool operator< <T> (const stack<T> &, const stack<T> &);
	
	//写成这样也是可以的
	friend bool operator== <T> (const stack &, const stack &);
	friend bool operator< <T> (const stack &, const stack &);
	
	//写成这样也是可以的
	friend bool operator== <> (const stack &, const stack &);
	friend bool operator< <> (const stack &, const stack &);
	
	//写成这样就不可以
	friend bool operator== (const stack &, const stack &);
	friend bool operator< (const stack &, const stack &);
	
public:
	stack(){
		cout<<"stack"<<endl;
	}
	
private:
	Sequence c;
};

template <class T, class Sequence>
bool operator==(const stack<T, Sequence>& x, const stack<T, Sequence> &y)
{
	return cout<<"operator=="<<'\t';
}

template <class T, class Sequence>
bool operator<const stack<T, Sequence>& x, const stack<T, Sequence> &y)
{
	{
	return cout<<"operator<"<<'\t';
}


int main(int argc, char *argv[])
{
	stack<int> x;                //deque stack
	stack<int> y;                //deque stack 
	
	cout<<(x == y)<<endl;        //operator ==
	cout<<(x < y) <<endl;        //operator <
	
	stack<char> y1;              //deque stack 
	//cout<<(x == y1) <<endl;    //error: no match for ...
	//cout<<(x < y1) <<endl;     //error: no match for ...
	
	return 0x0;
}
{% endhighlight %}

9) **组态：__STL_TEMPLATE_NULL (class template explicit specialization)**

<stl_config.h>定义了一个```__STL_TEMPLATE_NULL```如下：
{% highlight string %}
# if defined(__STL_CLASS_PARTIAL_SPECIALIZATION) \
     || defined (__STL_PARTIAL_SPECIALIZATION_SYNTAX)
#   define __STL_TEMPLATE_NULL template<>
# else
#   define __STL_TEMPLATE_NULL
# endif
{% endhighlight %}

这个组态常量常常出现在类似这样的场合：
{% highlight string %}
//in <type_traits.h>
template <class type> struct __type_traits{ ... };
__STL_TEMPLATE_NULL struct __type_traits<char> { ... };

//in <stl_hash_fun.h>
template <class Key> struct hash{};
__STL_TEMPLATE_NULL struct hash<char> { ... };
__STL_TEMPLATE_NULL struct hash<unsigned char> { ... };
{% endhighlight %}
展开后就变成了：
{% highlight string %}
//in <type_traits.h>
template <class type> struct __type_traits{ ... };
template<> struct __type_traits<char> { ... };

//in <stl_hash_fun.h>
template <class Key> struct hash{};
template<> struct hash<char> { ... };
template<> struct hash<unsigned char> { ... };
{% endhighlight %}

这是所谓的class template explicit specialization。下面这个例子适用于GCC和VC6，允许使用者不指定```template<>```就完成explicit specialization。C++ Builder则非常严格地要求必须完全遵照C++标准规格，也就是必须明白写出```template<>```。

{% highlight string %}
//file: test.cpp
//以下测试class template explicit specialization
//test __STL_TEMPLATE_NULL in <stl_config.h>
//ref. C++ Primer 3/e, p.858

#include <iostream>
using namespace std;

//将__STL_TEMPLATE_NULL定义为template<>，可以
//若定义为blank，如下，则只适用于GCC(注：GCC 4.8.5也不支持为空了)
#define __STL_TEMPLATE_NULL  template<>

template <class Key> struct hash{

	void operator()() { cout << "hash<T>"<<endl; }
};


//explicit specialization
__STL_TEMPLATE_NULL struct hash<char>{

	void operator()() { cout<<"hash<char>"<<endl; }
};

__STL_TEMPLATE_NULL struct hash<unsigned char>{

	void operator()() { cout<<"hash<unsigned char>"<<endl; }
};

int main(int argc, char *argv[])
{
	hash<long> t1;
	hash<char> t2;
	hash<unsigned char> t3;
	
	t1();
	t2();
	t3();
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
hash<T>
hash<char>
hash<unsigned char>
</pre>

## 3. 临时对象地产生与运用
所谓临时对象，就是一种无名对象(unnamed objects)。它的出现如果不在程序员的预期之下（例如任何pass by value操作都会引发copy操作，于是形成一个临时对象），往往造成效率上的负担。但有时候可以制造一些临时对象，却又是使程序干净清爽的技巧。刻意制造临时对象的方法是，在型别名称之后直接加一对小括号，并可指定初值，例如Shape(3,5)或int(8)，其意义相当于调用相应的constructor且不指定对象名称。STL最常将此技巧应用于仿函数(functor)与算法的搭配上。例如：
{% highlight string %}
//file: test.cpp
//本例测试仿函数用于for_each()的情形

#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

template <typename T>
class print{
public:
	void operator()(const T& elem){
		cout<<elem<<' ';
	}
};

int main(int argc, char *argv[])
{
	int ia[] = {0,1,2,3,4,5};
	
	vector<int> iv(ia, ia+6);
	
	//print<int>()是一个临时对象，不是一个函数调用
	for_each(iv.begin(), iv.end(), print<int>());
	
	return 0x0;
}
{% endhighlight %}
上面```for_each```一行便是产生“class template具现体” print<int>的一个临时对象。这个对象被传入for_each()之中起作用。当for_each()结束时，这个临时对象也就结束了它的生命。

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
0 1 2 3 4 5
</pre>









<br />
<br />

**[参看]:**



<br />
<br />
<br />





