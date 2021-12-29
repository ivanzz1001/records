---
layout: post
title: new vs operator new
tags:
- cplusplus
categories: cplusplus
description: new vs operator new
---


本章介绍一下new operator与operator new，在此做一个记录以便后续查阅。


<!-- more -->

## 1. new vs operator new in C++

When you create a new object, memory is allocated using operator new function and then the constructor is invoked to initialize the memory. Here, The new operator does both the allocation and the initialization, where as the operator new only does the allocation.

Let us see how these both work individually.

### 1. new keyword
The new operator is an **operator** which denotes a request for memory allocation on the Heap. If sufficient memory is available, new operator initializes the memory and returns the address of the newly allocated and initialized memory to the pointer variable. When you create an object of class using new keyword(normal new).

* The memory for the object is allocated using **operator new** from heap. 

* The constructor of the class is invoked to properly initialize this memory.

{% highlight string %}
// CPP program to illustrate
// use of new keyword
#include<iostream>
using namespace std;
class car
{
	string name;
	int num;

	public:
		car(string a, int n)
		{
			cout << "Constructor called" << endl;
			this ->name = a;
			this ->num = n;
		}

		void enter()
		{
			cin>>name;
			cin>>num;
		}

		void display()
		{
			cout << "Name: " << name << endl;
			cout << "Num: " << num << endl;
		}
};

int main()
{
	// Using new keyword
	car *p = new car("Honda", 2017);
	p->display();
}
{% endhighlight %}

Output:
<pre>
Constructor called
Name: Honda
Num: 2017
</pre>

### 1.2 Operator new
Operator new is a **function** that allocates raw memory and conceptually a bit similar to malloc().

* It is the mechanism of overriding the default heap allocation logic.

* It doesn’t initializes the memory i.e constructor is not called. However, after our overloaded new returns, the compiler then automatically calls the constructor also as applicable.

* It’s also possible to [overload operator new](https://www.geeksforgeeks.org/overloading-new-delete-operator-c/) either globally, or for a specific class

{% highlight string %}
// CPP program to illustrate
// use of operator new
#include<iostream>
#include<stdlib.h>

using namespace std;

class car
{
	string name;
	int num;

	public:

		car(string a, int n)
		{
			cout << "Constructor called" << endl;
			this->name = a;
			this->num = n;
		}

		void display()
		{
			cout << "Name: " << name << endl;
			cout << "Num: " << num << endl;
		}

		void *operator new(size_t size)
		{
			cout << "new operator overloaded" << endl;
			void *p = malloc(size);
			return p;
		}

		void operator delete(void *ptr)
		{
			cout << "delete operator overloaded" << endl;
			free(ptr);
		}
};

int main()
{
	car *p = new car("HYUNDAI", 2012);
	p->display();
	delete p;
}
{% endhighlight %}

Output:
<pre>
new operator overloaded
Constructor called
Name:HYUNDAI
Num:2012
delete operator overloaded
</pre>

### 1.3 New operator vs operator new

1) **Operator vs function**: new is an operator as well as a keyword whereas operator new is only a function.

2) **New calls “Operator new”**: “new operator” calls “operator new()” , like the way + operator calls operator +()

3) **“Operator new” can be Overloaded**: Operator new can be overloaded just like functions allowing us to do customized tasks.


4) **Memory allocation**: ‘new expression’ call ‘operator new’ to allocate raw memory, then call constructor.

## 2. Overloading New and Delete operator in c++

The new and delete operators can also be overloaded like other operators in C++. New and Delete operators can be overloaded globally or they can be overloaded for specific classes. 

* If these operators are overloaded using member function for a class, it means that these operators are overloaded **only for that specific class**.

* If overloading is done outside a class (i.e. it is not a member function of a class), the overloaded ‘new’ and ‘delete’ will be called anytime you make use of these operators (within classes or outside classes). This is **global overloading**.

### 2.1 Syntax for overloading the new operator 

In ```C++ 11```, there's three different forms with **operator new**:
{% highlight string %}
throwing (1)    void* operator new (std::size_t size);
nothrow (2)	    void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
placement (3)   void* operator new (std::size_t size, void* ptr) noexcept;
{% endhighlight %}
Allocate storage space.

Default allocation functions (*single-object form*).

**(1) throwing allocation**

Allocates size bytes of storage, suitably aligned to represent any object of that size, and returns a non-null pointer to the first byte of this block.

On failure, it throws a bad_alloc exception.

**(2) nothrow allocation**

Same as above (1), except that on failure it returns a null pointer instead of throwing an exception.

**(3) placement**

Simply returns ptr (no storage is allocated).

Notice though that, if the function is called by a new-expression, the proper initialization will be performed (for class objects, this includes calling its default constructor).

The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:

* **Global**: All three versions of operator new are declared in the global namespace, not within the std namespace.

* **Implicit**: The allocating versions ((1) and (2)) are implicitly declared in every translation unit of a C++ program, no matter whether header <new> is included or not.

* **Replaceable**: The allocating versions ((1) and (2)) are also replaceable: A program may provide its own definition that replaces the one provided by default to produce the result described above, or can overload it for specific types.

###### 2.1.1 Example

The following example shows how to overload the new operator:
{% highlight string %}
// CPP program to demonstrate
// Overloading new and delete operator
// for a specific class
#include<iostream>
#include<stdlib.h>

using namespace std;
class student
{
	string name;
	int age;
public:
	student()
	{
		cout<< "Constructor is called\n" ;
	}
	student(string name, int age)
	{
		this->name = name;
		this->age = age;
	}
	void display()
	{
		cout<< "Name:" << name << endl;
		cout<< "Age:" << age << endl;
	}
	void * operator new(size_t size)
	{
		cout<< "Overloading new operator with size: " << size << endl;
		void * p = ::operator new(size);
		//void * p = malloc(size); will also work fine
	
		return p;
	}

	void operator delete(void * p)
	{
		cout<< "Overloading delete operator " << endl;
		free(p);
	}
};

int main()
{
	student * p = new student("Yash", 24);

	p->display();
	delete p;
}
{% endhighlight %}
Output:
<pre>
Overloading new operator with size: 16
Name:Yash
Age:24
Overloading delete operator 
</pre>

### 2.2 Syntax for overloading the new[] operator 

In ```C++ 11```, there's three different forms with **operator new[]**:
{% highlight string %}
throwing (1)    void* operator new[] (std::size_t size);
nothrow (2)     void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
placement (3)   void* operator new[] (std::size_t size, void* ptr) noexcept;
{% endhighlight %}
Allocate storage space for array

Default allocation functions (array form).

**(1) throwing allocation**

Allocates size bytes of storage, suitably aligned to represent any object of that size, and returns a non-null pointer to the first byte of this block.

On failure, it throws a bad_alloc exception.

The default definition allocates memory by calling operator new: ```::operator new (size)```.

If replaced, both operator new and operator new[] shall return pointers with identical properties.

**(2) nothrow allocation**

Same as above (1), except that on failure it returns a null pointer instead of throwing an exception.

**(3) placement**

Simply returns ptr (no storage is allocated).

Notice though that, if the function is called by the new expression, the proper initialization will be performed (for class objects, this includes calling its default constructor).

### 2.3 Syntax for overloading the delete operator 
In ```C++ 11```, there's three different forms with **operator delete**:

{% highlight string %}
ordinary (1)    void operator delete (void* ptr) noexcept;
nothrow (2)     void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
placement (3)   void operator delete (void* ptr, void* voidptr2) noexcept;
{% endhighlight %}
Deallocate storage space

Default deallocation functions (single-object form).

### 2.4 Syntax for overloading the delete[] operator 
In ```C++ 11```, there's three different forms with **operator delete[]**:
{% highlight string %}
ordinary (1)   void operator delete[] (void* ptr) noexcept;
nothrow (2)	   void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
placement (3)  void operator delete[] (void* ptr, void* voidptr2) noexcept
{% endhighlight %}









<br />
<br />

**[参看]:**

1. [new vs operator new in C++](https://www.geeksforgeeks.org/new-vs-operator-new-in-cpp/)

2. [operator overloading](https://en.cppreference.com/w/cpp/language/operators)

3. [cpp reference-new](http://www.cplusplus.com/reference/new/)

4. [All about “new” operator in C++](https://aticleworld.com/dynamic-memory-and-new-operator-c/)

5. [C++中使用placement new](https://blog.csdn.net/linuxheik/article/details/80449059)

<br />
<br />
<br />





