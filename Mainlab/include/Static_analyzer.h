#ifndef STATIC_ANALYZER_H
#define STATIC_ANALYZER_H

#include "heap.h"
#include "list.h"
#include "automaton.h"

class StaticAnalyzer
{
public:
	StaticAnalyzer(char* filename) :_lexems(sizeof(Lexem)), _global_variables(NULL, 0)
	{
		_filename = filename;
		is_program_correct = true;
	}
	void form_lexem_list();
	void print_lexem_list();
	int check_parentheses();
	int check_variables_existence(Variable_List* current_variable_list = NULL, int starting_index = 0, int deleted_lexems = 0);
	int check_flow();
	int check_syntax(int lexem_index = 0);
	void print_variables();
	Variable_List _global_variables;
	List _lexems;
private:

	void check_unary_op(int index);
	void check_binary_op(int index);
	void check_unknown_op(int index);

	bool find_pair_parentheses(int closing_parenthesis_code, int* opening_brace_position);
	void print_variable_list_recursive(Variable_List* list, int indent);
	char* _filename;
	bool is_program_correct;
};


#endif
