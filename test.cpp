// 内存池.cpp : Defines the entry point for the console application.
//

#include <iostream>  
#include <cassert>  
#include <time.h>  
#include <vector>  
#include <stack>  
 
#include "MemoryPool.h"  
 
#include "StackAlloc.h"
 
 
using namespace std;  
 
 
/* Adjust these values depending on how much you trust your computer */  
#define ELEMS 1000000  
#define REPS 5  
 int main2();
 
int main()  
{  
	typedef double ElemType; 
//	typedef size_t ElemType; 
 
	clock_t start;  
 
	MemoryPool<ElemType> pool;  
	start = clock();  
	for(int i = 0;i < REPS;++i)  
	{  
		for(int j = 0;j< ELEMS;++j)  
		{  
			// 创建元素  
			ElemType* x = pool.newElement(i * ELEMS + j);  
 
			// 释放元素  
			pool.deleteElement(x);  
		}  
	}  
 
	std::cout << "MemoryPool Time: ";  
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";  
 
 
	start = clock();  
	for(int i = 0;i < REPS;++i)  
	{  
		for(int j = 0;j< ELEMS;++j)  
		{  
			ElemType* x = new ElemType;  
			delete x;  
		}  
	}  
	std::cout << "new/delete Time: ";  
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";  
	
	main2();
	return 0;  
 
}  
 
int main2()
{
	clock_t start;
 
	std::cout << "Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com\n";
	std::cout << "Provided to compare the default allocator to MemoryPool.\n\n";
 
	/* Use the default allocator */
	StackAlloc<int, std::allocator<int> > stackDefault;
	start = clock();
	for (int j = 0; j < REPS; j++)
	{
		assert(stackDefault.empty());
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
			stackDefault.push(i);
		}
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
			stackDefault.pop();
		}
	}
	std::cout << "Default Allocator Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";
 
	/* Use MemoryPool */
	StackAlloc<int, MemoryPool<int> > stackPool;
	start = clock();
	for (int j = 0; j < REPS; j++)
	{
		assert(stackPool.empty());
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
			stackPool.push(i);
		}
 
 
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
			stackPool.pop();
		}
	}
 
	std::cout << "MemoryPool Allocator Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";
 
 
	std::cout << "Here is a secret: the best way of implementing a stack"
		" is a dynamic array.\n";
 
	/* Compare MemoryPool to std::vector */
	//std::stack<int> stackVector;
	std::stack<int> stackVector;
 
	//   说明：当前内存池不支持vector [10/30/2016 ZOSH];
	//std::vector<int, MemoryPool<int> > stackVector;
 
	start = clock();
	for (int j = 0; j < REPS; j++)
	{
		assert(stackVector.empty());
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackVector.push(i);
			stackVector.push(i);
			stackVector.push(i);
			stackVector.push(i);
		}
		for (int i = 0; i < ELEMS / 4; i++) {
			// Unroll to time the actual code and not the loop
			stackVector.pop();
			stackVector.pop();
			stackVector.pop();
			stackVector.pop();
		}
	}
	std::cout << "Stack Time: ";
	std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";
 
	std::cout << "The vector implementation will probably be faster.\n\n";
	std::cout << "MemoryPool still has a lot of uses though. Any type of tree"
		" and when you have multiple linked lists are some examples (they"
		" can all share the same memory pool).\n";
 
	return 0;
}
