#ifndef TREE_H
#define TREE_H
#include "lexem.h"
#include <vector>

class Forest;

class TreeNode
{
public:
	TreeNode();
	static TreeNode* form_expression_tree(List* lexems, int first_index, int last_index);
	Value* solve();
	Lexem* lexem;

	int expression_start;
	int expression_end;
	
	std::vector<TreeNode*> children_nodes;
	std::vector<Forest*> children_forests;

private:
	Value* solve_addition(Value* lvalue, Value* rvalue);
	Value* solve_subtraction(Value* lvalue, Value* rvalue);
	Value* solve_assignment(Value* lvalue, Value* rvalue);
	Value* solve_multiplication(Value* lvalue, Value* rvalue);
	Value* solve_assignment_multiplication(Value* lvalue, Value* rvalue);

	Value* solve_is_equal(Value* lvalue, Value* rvalue);
	Value* solve_less_than(Value* lvalue, Value* rvalue);
	Value* solve_less_or_equal(Value* lvalue, Value* rvalue);
	Value* solve_greater_than(Value* lvalue, Value* rvalue);
	Value* solve_greater_or_equal(Value* lvalue, Value* rvalue);
	
	Value* solve_unary_minus(Value* value);
	Value* solve_postfix_increment(Value* value);
	Value* solve_prefix_increment(Value* value);

	Value* generate_value(Value* value1, Value* value2);
};

class Forest
{
public:
	Forest();
	Value* solve();
	List TreeList;
	void form_forest(List* lexems, int first_index, int last_index);
};

#endif