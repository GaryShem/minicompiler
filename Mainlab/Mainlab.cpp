// Mainlab.cpp : Defines the entry point for the console application.
//

#include "include/stdafx.h"
#include "include/heap.h"
#include "include/list.h"
#include "include/list_adv.h"
#include "include/hash.h"
#include "include/automaton.h"
#include "include/tree.h"
#include "include/Static_analyzer.h"
#include <iostream>
#include <string>
#include <fstream>

static Heap heap;

int compar(const void* e1, const void* e2)
{
	int* n1 = (int*)e1;
	int* n2 = (int*)e2;
	return *n1 - *n2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	StaticAnalyzer sa("test_program.txt");
	sa.form_lexem_list();
	sa.check_parentheses();
	sa.check_variables_existence();
	sa.print_variables();
	TreeNode tn;
	tn.form_tree(&sa._lexems, 3, 8);
	tn.solve(&sa._variables);
	sa.print_variables();

	return 0;
}

