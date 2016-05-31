#ifndef TREE_H
#define TREE_H
#include "lexem.h"

class TreeNode
{
public:
	TreeNode();
	TreeNode* form_expression_tree(List* lexems, int first_index, int last_index);
	Value* solve(Variable_List* variables);
	Lexem* lexem;
	TreeNode* left_node;
	TreeNode* right_node;

private:
	Value* solve_addition(Value* lvalue, Value* rvalue);
	Value* solve_assignment(Value* lvalue, Value* rvalue);
	Value* solve_multiplication(Value* lvalue, Value* rvalue);
	Value* solve_is_equal(Value* lvalue, Value* rvalue);
};

#endif