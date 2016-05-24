#ifndef STATIC_ANALYZER_H
#define STATIC_ANALYZER_H

#include "heap.h"
#include "list.h"
#include "automaton.h"




class StaticAnalyzer
{
public:
	StaticAnalyzer(char* filename) :_lexems(sizeof(Lexem))
	{
		_filename = filename;
		is_program_correct = true;
	}
	void form_lexem_list();
	int check_parentheses();
	int check_variables_existence();
	void print_variables();
	Variable_List _variables;
	List _lexems;
private:

	bool find_pair_parentheses(int closing_parenthesis_code, int* opening_brace_position);
	char* _filename;
	bool is_program_correct;
};


#endif
