#include "../include/tree.h"
#include "../include/list.h"

TreeNode::TreeNode()
{
	left_node = right_node = NULL;
	lexem = NULL;
}

TreeNode* TreeNode::form_tree(List* lexems, int first_index, int last_index)
{
	int brace_count = 0;
	int max_code = -1;
	int priority_index = -1;
	if (last_index - first_index == 1) // если осталась одна лексема, то просто пишем её в нод - это лист дерева
	{
		this->lexem = (Lexem*)lexems->get(first_index);
		return this;
	}
	for (int i = first_index; i < last_index; i++)
	{
		Lexem* lex = (Lexem*)lexems->get(i);
		if (lex->type == LexemType::OPERATION && lex->code > max_code)
		{
			max_code = lex->code;
			priority_index = i;
		}
	}
	lexem = (Lexem*)lexems->get(priority_index);
	left_node = new TreeNode;
	left_node->form_tree(lexems, first_index, priority_index);
	right_node = new TreeNode;
	right_node->form_tree(lexems, priority_index + 1, last_index);
	return this;
}

Value* TreeNode::solve(Variable_List* variables)
{
	Value* lvalue = NULL;
	Value* rvalue = NULL;
	switch (lexem->type)
	{
	case LexemType::OPERATION:
		
		switch (lexem->code)
		{
		case ASSIGNMENT:
			lvalue = left_node->solve(variables);
			rvalue = right_node->solve(variables);
			if (lvalue->type != rvalue->type)
				throw 6545642;
			return solve_assignment(lvalue, rvalue);
			break;
		case BINARY_PLUS:
			lvalue = left_node->solve(variables);
			rvalue = right_node->solve(variables);
			if (lvalue->type != rvalue->type)
				throw 6545642;
			return solve_addition(lvalue, rvalue);
			break;
		default:
			throw 285307;
		}
		break;
	case VARIABLE_NAME: 
	{
		Variable_Record* res = variables->get_variable(lexem->word_string);
		return res->value;
	}
		break;
	case NUMERIC_CONST: 
	{
		Value* res = new Value(VARIABLE_TYPE::INT);
		memcpy(res->value, &(lexem->const_value), sizeof(int));
		return res;
	}
		break;
	case OPERATOR_DELIMITER: break;
	case PARENTHESES: break;
	case KEYWORD: break;
	default: break;
	}
	return NULL;
}

Value* TreeNode::solve_addition(Value* lvalue, Value* rvalue)
{
	switch (lvalue->type)
	{
	case INT:
	{	
		Value* res = new Value(INT);
		int *resnum = (int*)res->value;
		int *lnum = (int*)lvalue->value;
		int *rnum = (int*)rvalue->value;
		*resnum = *lnum + *rnum;
		return res;
	}
		break;
	case DOUBLE:
	{
		Value* res = new Value(DOUBLE);
		double *resnum = (double*)res->value;
		double *lnum = (double*)lvalue->value;
		double *rnum = (double*)rvalue->value;
		*resnum = *lnum + *rnum;
		return res;
	}
		break;
	default:
		throw 546546351;
	}
}

Value* TreeNode::solve_assignment(Value* lvalue, Value* rvalue)
{
	memcpy(lvalue->value, rvalue->value, get_op_size(lvalue));
	return lvalue;
}

int TreeNode::get_op_size(Value* value)
{
	int result;
	switch (value->type)
	{
	case INT:
		result = sizeof(int);
		break;
	case DOUBLE:
		result = sizeof(double);
		break;
	case CHAR:
		result = sizeof(char);
		break;
	default:
		throw 1238912;
	}
	return result;
}
