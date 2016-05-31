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

int _tmain(int argc, _TCHAR* argv[])
{
	try {
		StaticAnalyzer sa("test_program.txt");
		sa.form_lexem_list();
		sa.check_parentheses();
		sa.check_variables_existence();
		sa.print_variables();
		TreeNode tn;
		tn.form_expression_tree(&sa._lexems, 3, 12);
		tn.solve(&sa._global_variables);
		sa.print_variables();
	}
	catch (int errNo)
	{
		std::cout << "Error code " << errNo << std::endl;
	}
	catch (char* msg)
	{
		std::cout << "Error message: " << msg << std::endl;
	}
	return 0;
}

