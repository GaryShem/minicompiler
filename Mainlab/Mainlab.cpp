// Mainlab.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "include/heap.h"
#include <iostream>
#include <string>

static Heap heap;

int _tmain(int argc, _TCHAR* argv[])
{
	int b = 3;

	int* a = (int*)heap.get_mem(sizeof(int));

	*a = 3;

	std::string miimimi;

	std::cout << *a << std::endl;

	heap.free_mem(a);

	getline(std::cin, miimimi);
	return 0;
}

