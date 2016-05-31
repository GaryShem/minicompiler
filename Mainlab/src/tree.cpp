#include "../include/tree.h"
#include "../include/list.h"

TreeNode::TreeNode()
{
	left_node = right_node = NULL;
	lexem = NULL;
}

TreeNode* TreeNode::form_expression_tree(List* lexems, int first_index, int last_index)
{
	int max_code = -1;
	int priority_index = -1;
	// если осталась одна лексема, то просто пишем еЄ в нод - это лист дерева
	if (last_index - first_index == 1)
	{
		this->lexem = (Lexem*)lexems->get(first_index);
		return this;
	}
	// убираем окаймл€ющие скобки - если перва€ и последн€€ лексема €вл€ютс€ парными скобками,
	// в качестве выражени€ берЄм то, что было внутри скобок,
	// при этом сами скобки нам уже неинтересны
	Lexem* first_lexem = (Lexem*)lexems->get(first_index);
	if (first_lexem->type == LexemType::PARENTHESES && first_lexem->pair_brace_position == last_index-1)
	{
		return form_expression_tree(lexems, first_index + 1, last_index-1);
	}
	for (int i = first_index; i < last_index; i++)
	{
		Lexem* lex = (Lexem*)lexems->get(i);
		if (lex->type == PARENTHESES)
		{
			if (lex->code % 2 == 0)
			{
				i = lex->pair_brace_position;
			}
			else
			{
				throw 46843;
			}
		}
		else if (lex->type == LexemType::OPERATION && lex->code > max_code)
		{
			max_code = lex->code;
			priority_index = i;
		}
	}
	lexem = (Lexem*)lexems->get(priority_index);
	left_node = new TreeNode;
	left_node->form_expression_tree(lexems, first_index, priority_index);
	right_node = new TreeNode;
	right_node->form_expression_tree(lexems, priority_index + 1, last_index);
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
			return solve_assignment(lvalue, rvalue);
			break;
		case BINARY_PLUS:
			lvalue = left_node->solve(variables);
			rvalue = right_node->solve(variables);
			return solve_addition(lvalue, rvalue);
			break;
		case MULTIPLICATION:
			lvalue = left_node->solve(variables);
			rvalue = right_node->solve(variables);
			return solve_multiplication(lvalue, rvalue);
			break;
		case IS_EQUAL:
			lvalue = left_node->solve(variables);
			rvalue = right_node->solve(variables);
			return solve_multiplication(lvalue, rvalue);
			break;
		default:
			throw 285307;
		}
		break;
	case VARIABLE_NAME: 
	{
		return lexem->variable->value;
//		Variable_Record* res = variables->get_variable(lexem->word_string);
//		return res->value;
	}
		break;
	case NUMERIC_CONST: 
	{
		Value* res = new Value(VARIABLE_TYPE::INT);
		res->value = lexem->const_value;
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
	Value* res = new Value(INT);
	switch (lvalue->type)
	{
		case INT:
		{
			res->type = INT;
			int lnum = (int)lvalue->value;
			switch (rvalue->type)
			{
				case INT:
				{
					int rnum = (int)rvalue->value;
					res->value = lnum + rnum;
				}
				break;
				case DOUBLE:
				{
					res->type = DOUBLE;
					int rnum = (double)rvalue->value;
					res->value = lnum + rnum;
				}
				break;
				default:
					throw 12648;
					break;
			}
		}
		break;
		case DOUBLE:
		{
			res->type = DOUBLE;
			double lnum = (double)lvalue->value;
			switch (rvalue->type)
			{
				case INT:
				{
					int rnum = (int)rvalue->value;
					res->value = lnum + rnum;
				}
				break;
				case DOUBLE:
				{
					int rnum = (double)rvalue->value;
					res->value = lnum + rnum;
				}
				break;
				default:
					throw 12648;
					break;
			}
		}
		break;
		default:
			throw 546546351;
	}
	return res;
}

Value* TreeNode::solve_assignment(Value* lvalue, Value* rvalue)
{
	switch (lvalue->type)
	{
	case INT:
		lvalue->value = (int)rvalue->value;
		break;
	case DOUBLE:
		lvalue->value = (int)rvalue->value;
		break;
	default: 
		throw 18260148;
		break;
	}
	return lvalue;
}

Value* TreeNode::solve_multiplication(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(INT);
	switch (lvalue->type)
	{
		case INT:
		{
			res->type = INT;
			int lnum = (int)lvalue->value;
			switch (rvalue->type)
			{
				case INT:
				{
					int rnum = (int)rvalue->value;
					res->value = lnum * rnum;
				}
				break;
				case DOUBLE:
				{
					res->type = DOUBLE;
					int rnum = (double)rvalue->value;
					res->value = lnum * rnum;
				}
				break;
				default:
					throw 12648;
					break;
			}
		}
		break;
		case DOUBLE:
		{
			res->type = DOUBLE;
			double lnum = (double)lvalue->value;
			switch (rvalue->type)
			{
				case INT:
				{
					int rnum = (int)rvalue->value;
					res->value = lnum * rnum;
				}
				break;
				case DOUBLE:
				{
					int rnum = (double)rvalue->value;
					res->value = lnum * rnum;
				}
				break;
				default:
					throw 12648;
					break;
			}
		}
		break;
		default:
			throw 546546351;
	}
	return res;
}

Value* TreeNode::solve_is_equal(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(INT);
	res->value = (lvalue->value == rvalue->value);
	return res;
}
