---
layout: post
title: C++中的友元函数和友元类
tags:
- cplusplus
categories: cplusplus
description: C/C++基础
---


本文主要介绍一下C++中的友元函数和友元类的使用。

<!-- more -->


## 1. 友元函数

其实友元函数又可以分为：

* 普通友元函数

* 友成员函数

### 1.1 普通友元函数
代码中有一个全局函数，该函数想要去访问某个类的成员变量（该类的成员变量是```private```的，且该类并未提供任何获取其私有成员变量的```public```方法），这时候可以在这个类中把该全局函数声明为```友元函数```，这样这个```全局函数```就具备了能够获取那个类的私有成员变量的资格。


>注：
>
>1）普通友元函数不是类成员函数，而是一个类外的函数，但是可以访问类所有成员。
>
>2) 可以将普通友元函数声明在类的任何一个section(public/protected/private)

参看如下示例，通过友元函数获取Person对象的各字段信息(test.cpp)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


class Person{
public:
        Person(char *name, int age, int sex);
        ~Person(){}

private:
        std::string name;
        int age;
        int sex;

private:
        friend void showPersonInfo(Person *p);

};

Person::Person(char *name, int age, int sex)
  :name(name),
   age(age),
   sex(sex){
}

void showPersonInfo(Person *p)
{
        std::cout<<"name: "<<p->name<<" age: "<<p->age<<" sex: "<<(p->sex==0?"male":"female")<<std::endl;
}

int main(int argc, char *argv[])
{
        Person p((char *)"ivan1001", 20, 0);

        showPersonInfo(&p);

        return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
name: ivan1001 age: 20 sex: male
</pre>

### 1.2 友成员函数
假设有两个类，分别为```class A```和```class B```。class A的一个成员函数想访问class B的私有成员变量，此时我们可以在class B中将class A的成员函数声明为友元函数。

参看如下示例，打印Person类的详细信息(包含姓名,年龄，性别，省份，城市，街道)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


class Address;

class Person{
public:
        Person(char *name, int age, int sex);
        ~Person(){}

private:
        std::string name;
        int age;
        int sex;

public:
        void showAddressInfo(Address *paddr);

};



class Address{
public:
        Address(char *province, char *city, char *district);

        ~Address(){}

private:
        std::string province;
        std::string city;
        std::string district;

private:
        friend void Person::showAddressInfo(Address *paddr);
};


Person::Person(char *name, int age, int sex)
  :name(name),
   age(age),
   sex(sex){
}

Address::Address(char *province, char *city, char *district)
  :province(std::string(province)),
   city(city),
   district(district){

}

void Person::showAddressInfo(Address *paddr)
{
        std::cout<<"province: "<<paddr->province<<" city: "<<paddr->city<<" district: "<<paddr->district<<std::endl;
}


int main(int argc, char *argv[])
{
        Person p((char *)"ivan1001", 20, 0);
        Address addr((char *)"Guangdong", (char *)"Shenzhen", (char *)"Nanshan");

        p.showAddressInfo(&addr);

        return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
province: Guangdong city: Shenzhen district: Nanshan
</pre>


## 2. 友元类
友元类则更为简单，可以将class A声明为class B的友元类，那么class A就拥有了能够访问class B所有成员的资格，包括private、protected、public的。

参看如下示例：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>


class Address;

class Person{
public:
        Person(char *name, int age, int sex);
        ~Person(){}

private:
        std::string name;
        int age;
        int sex;

public:
        void showAddressInfo(Address *paddr);

};



class Address{
public:
        Address(char *province, char *city, char *district);

        ~Address(){}

private:
        std::string province;
        std::string city;
        std::string district;

private:
        friend class Person;
};


Person::Person(char *name, int age, int sex)
  :name(name),
   age(age),
   sex(sex){
}

Address::Address(char *province, char *city, char *district)
  :province(std::string(province)),
   city(city),
   district(district){

}

void Person::showAddressInfo(Address *paddr)
{
        std::cout<<"province: "<<paddr->province<<" city: "<<paddr->city<<" district: "<<paddr->district<<std::endl;
}


int main(int argc, char *argv[])
{
        Person p((char *)"ivan1001", 20, 0);
        Address addr((char *)"Guangdong", (char *)"Shenzhen", (char *)"Nanshan");

        p.showAddressInfo(&addr);

        return 0x0;
}
{% endhighlight %}

上述，我们称class Person是class Address的友元类。

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
province: Guangdong city: Shenzhen district: Nanshan
</pre>

需要注意的是：

* 友元的关系是单向的而不是双向的。如果声明了```class B``` 是```class A```的友元类，不等于```class A``` 是```class B```的友元类，```class A```中的成员函数不能访问```class B```中的```private```成员。

* 友元的关系不能传递。如果类```class B```是```class A```的友元类，```class C```是```class B```的友元类，不等于```class C```是```class A```的友元类。



<br />
<br />

**[参看]:**


<br />
<br />
<br />





