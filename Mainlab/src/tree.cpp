#include "../include/tree.h"
#include "../include/list.h"

Forest::Forest() : TreeList(sizeof(TreeNode*))
{
}

Value* Forest::solve()
{
	Value* res = NULL;
	int limit = TreeList.count();
	for (int i = 0; i < limit; i++)
	{
		TreeNode** expr_ptr = (TreeNode**)TreeList.get(i);
		TreeNode* expr = *expr_ptr;
		res = expr->solve();
	}
	return res;
}

void Forest::form_forest(List* lexems, int first_index, int last_index)
{
	int delim = 0;
	int limit = last_index;
	for (delim = 0; delim < last_index; delim++)
	{
		Lexem* lex = (Lexem*)lexems->get(delim);
		if (lex->type == OPERATOR_DELIMITER)
		{
			TreeNode* expr = TreeNode::form_expression_tree(lexems, first_index, delim);
			TreeList.add(&expr);
			first_index = delim + 1;
		}
	}
}

TreeNode::TreeNode()
{
	left_node = right_node = flow_control_specials = NULL;
	lexem = NULL;
}

TreeNode* TreeNode::form_expression_tree(List* lexems, int first_index, int last_index)
{
	TreeNode* res = new TreeNode();
	int max_code = -1;
	int priority_index = -1;
	// если осталась одна лексема, то просто пишем её в нод - это лист дерева
	if (last_index - first_index == 1)
	{
		res->lexem = (Lexem*)lexems->get(first_index);
		return res;
	}
	// убираем окаймляющие скобки - если первая и последняя лексема являются парными скобками,
	// в качестве выражения берём то, что было внутри скобок,
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
	// TODO: добавить обработку для унарных операций
	// там не будет создаваться right_node, потому что не нужен

	// для бинарных операций - где есть и правый, и левый лепестки
	res->lexem = (Lexem*)lexems->get(priority_index);
	res->left_node = TreeNode::form_expression_tree(lexems, first_index, priority_index);
	res->right_node = TreeNode::form_expression_tree(lexems, priority_index + 1, last_index);
	return res;
}

Value* TreeNode::solve()
{
	Value* lvalue = NULL;
	Value* rvalue = NULL;
	switch (lexem->type)
	{
	case LexemType::OPERATION:
		
		switch (lexem->code)
		{
		case ASSIGNMENT:
			lvalue = left_node->solve();
			rvalue = right_node->solve();
			return solve_assignment(lvalue, rvalue);
			break;
		case BINARY_PLUS:
			lvalue = left_node->solve();
			rvalue = right_node->solve();
			return solve_addition(lvalue, rvalue);
			break;
		case MULTIPLICATION:
			lvalue = left_node->solve();
			rvalue = right_node->solve();
			return solve_multiplication(lvalue, rvalue);
			break;
		case IS_EQUAL:
			lvalue = left_node->solve();
			rvalue = right_node->solve();
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
		res->SetValue(lexem->const_value);
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
	Value* res = generate_value(lvalue, rvalue);
	res->SetValue(lvalue->GetValue() + rvalue->GetValue());

	return res;
}

Value* TreeNode::solve_subtraction(Value* lvalue, Value* rvalue)
{
	Value* res = generate_value(lvalue, rvalue);
	res->SetValue(lvalue->GetValue() - rvalue->GetValue());

	return res;
}

Value* TreeNode::solve_assignment(Value* lvalue, Value* rvalue)
{
	lvalue->SetValue(rvalue->GetValue());
	return lvalue;
}

Value* TreeNode::solve_multiplication(Value* lvalue, Value* rvalue)
{
	Value* res = generate_value(lvalue, rvalue);
	res->SetValue(lvalue->GetValue() * rvalue->GetValue());

	return res;
}

Value* TreeNode::solve_is_equal(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(BOOL);
	res->SetValue(lvalue->GetValue() == rvalue->GetValue());
	return res;
}

Value* TreeNode::generate_value(Value* lvalue, Value* rvalue)
{
	Value* res;
	if (lvalue->type == DOUBLE || rvalue->type == DOUBLE)
	{
		res = new Value(DOUBLE);
	}
	else if (lvalue->type == INT || rvalue->type == INT)
	{
		res = new Value(INT);
	}
	// сюда добавляется CHAR
	else if (lvalue->GetValue() == BOOL && rvalue->GetValue() == BOOL)
	{
		res = new Value(BOOL);
	}
	else
	{
		throw std::runtime_error(std::string("Кривые ручки программиста не сделали обработку такого типа"));
	}
	return res;
}
