// Mainlab.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "include/heap.h"
#include "include/list.h"
#include "include/list_adv.h"
#include <iostream>
#include <string>

static Heap heap;

int compar(const void* e1, const void* e2)
{
	int* n1 = (int*)e1;
	int* n2 = (int*)e2;
	return *n1 - *n2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	List list(sizeof(int));

	int a;
	int limit = 22;
	for (int i = 0; i < limit; i++)
	{
		a = i;
		list.add(&a);
	}

	//list.sort(false, compar);

	for (int j = 0; j < limit; j++)
	{
		list.take_last(&a);
		std::cout << j << "..." << a << std::endl;
	}


	return 0;
}

