---
layout: post
title: 其他内部排序
tags:
- data-structure
categories: data-structure
description: 内部排序
---


本节我们介绍一下“桶排序”。

<!-- more -->

## 1. 桶排序
桶排序(英文： Bucket Sort)是排序算法的一种，适用于待排序数据值域较大但分布比较均匀的情况。

### 1.1 工作原理
桶排序按下列步骤进行：

1） 设置一个定量的数组当做空桶；

2） 遍历序列，并将元素一个个放到对应的桶中；

3） 对每个不是空的桶进行排序；

4） 从不是空的桶里把元素再放回原来的序列中；

### 1.2 性质
1） **稳定性**

如果使用稳定的内层排序，并且将元素插入桶中时不改变元素间的相对顺序，那么桶排序就是一种稳定的排序算法。由于每块元素不多，一般使用插入排序。此时桶排序是一种稳定的排序算法。

2）**时间复杂度**

桶排序的平均时间复杂度为```O(n+n^2/k+k)```(将值域平均分成n块 + 排序 + 重新合并元素），当```k≈n```时为O(n)。

桶排序的最坏时间复杂度为O(n^2)。

### 1.3 算法实现
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>



 void insertion_sort(std::vector<int> &v)
 {
 	int j;
 	for (int i = 1; i<v.size(); i++){
		if(v[i] < v[i-1]){
			int t = v[i];
			v[i] = v[i-1];

			for(j = i - 2; j >= 0 && v[j] > t; j--){
				v[j+1] = v[j];
			}

			v[j+1] = t;
		}
 	}
 }


 void bucket_sort(int *a, int length)
 {
 	int i, p = 0;
	int min, max;


	if(!a || length <= 0)
		return;

	min = max = a[0];

	for(i = 1; i < length; i++){
		if(a[i] > max)
			max = a[i];
		else if(a[i] < min)
			min = a[i];
	}


	int bucket_size = (max - min) / length + 1;
	std::vector<int> *bucket = new std::vector<int>[bucket_size]();

	for(i = 0; i < bucket_size; i++)
		bucket[i].clear();

	for(i = 0; i < length; i++)
		bucket[(a[i] - min) / length].push_back(a[i]);

	for(i = 0; i < bucket_size; i++){
		if(bucket[i].size() > 0){
			insertion_sort(bucket[i]);

			for(int j = 0; j<bucket[i].size(); j++)
				a[p++] = bucket[i][j];
		}
	}

 }


 int main(int argc, char *argv[])
 {
	int MAX_VAL = 10000;
	const int N = 100;

	int a[N];

	srand((unsigned int)time(NULL));
	for(int i = 0; i < N; i++){
		a[i] = rand() % MAX_VAL;
	}

	bucket_sort(a, N);

	for(int i = 0; i< N; i++)
		printf("%d ", a[i]);

	return 0x0;
 }
{% endhighlight %}


  


**[参看]:**

1. [OI WIKI-排序](https://oi-wiki.org/basic/bucket-sort/)

2. [基数(Radix)排序](https://www.yiibai.com/data_structure/radix-sort.html)
<br />
<br />


