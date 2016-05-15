// Mainlab.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "include/heap.h"
#include "include/list.h"
#include "include/list_adv.h"
#include "include/hash.h"
#include "include/automaton.h"
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
	// �������� ������ ����
	/*List list(sizeof(int));

	int a;
	int limit = 22;
	for (int i = 0; i < limit; i++)
	{
		a = i;
		list.add(&a);
	}

	list.sort(false, compar);

	for (int j = 0; j < limit; j++)
	{
		list.take_first(&a);
		std::cout << j << "..." << a << std::endl;
	}


	return 0;*/ 

	//�������� ������ ����

	//��� �������� ����� ������� setlocale

	/*setlocale(LC_ALL, "Russian");
	Diction diction;
	Article* record;
	record = diction.auto_create("mimimi");
	record = diction.auto_create("����-���");
	record = diction.auto_create("������� ������");

	record = diction.find("������� ������");

	if (record)
	{
		std::cout << "Word: " << record->word << std::endl;
		std::cout << "Description: " << record->description << std::endl;
	}
	else
	{
		std::cout << "Record not found" << std::endl;
	}

	return 0;*/

	// �������� ��������
	//automaton tomaton("keywords.txt");
	//tomaton.generate_triads();
	//tomaton.print_triads();

	automaton tomaton2("keywords_triads.txt");
	tomaton2.read_triads();
	tomaton2.print_triads();

	return 0;
}

