#ifndef TREE_H
#define TREE_H
#include "lexem.h"
#include <list>

class TreeNode
{
public:
	TreeNode();
	static TreeNode* form_expression_tree(List* lexems, int first_index, int last_index);
	Value* solve();
	Lexem* lexem;
	TreeNode* left_node;
	TreeNode* right_node;
	TreeNode* flow_control_specials;

private:
	Value* solve_addition(Value* lvalue, Value* rvalue);
	Value* solve_subtraction(Value* lvalue, Value* rvalue);
	Value* solve_assignment(Value* lvalue, Value* rvalue);
	Value* solve_multiplication(Value* lvalue, Value* rvalue);
	Value* solve_is_equal(Value* lvalue, Value* rvalue);

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