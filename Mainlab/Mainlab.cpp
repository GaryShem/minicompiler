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
		sa.print_lexem_list();
		sa.check_parentheses();
		sa.check_variables_existence();
		sa.check_parentheses();
		sa.print_lexem_list();
		sa.check_syntax();
		sa.print_variables();
		sa.print_lexem_list();

		Forest f;
		f.form_forest(&sa._lexems, 0, sa._lexems.count());
		f.solve();
		sa.print_variables();
	}
	catch (int errNo)
	{
		std::cout << "Error code " << errNo << std::endl;
	}
	catch (std::runtime_error re)
	{
		std::cout << re.what() << std::endl;
	}
	return 0;
}

