#include "../include/tree.h"
#include "../include/list.h"
#include <sstream>
#include <string>
#include <iostream>

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
	int delim = first_index;
	for (delim = first_index; delim < last_index; delim++)
	{
		Lexem* lex = (Lexem*)lexems->get(delim);
		if (lex->type == OPERATOR_DELIMITER)
		{
			TreeNode* expr = TreeNode::form_expression_tree(lexems, first_index, delim);
			if (expr != NULL)
			{
				TreeList.add(&expr);
			}
			first_index = delim + 1;
		}
		if (lex->type == FLOW_CONTROL)
		{
			TreeNode* expr = TreeNode::form_expression_tree(lexems, delim, delim+1);
			TreeList.add(&expr);
			first_index = expr->expression_end;
			delim = first_index - 1;
		}
		if (lex->type == FUNCTION)
		{
			TreeNode* expr = TreeNode::form_expression_tree(lexems, delim, delim + 1);
			TreeList.add(&expr);
			first_index = expr->expression_end;
			delim = first_index - 1;
		}
	}
}

TreeNode::TreeNode()
{
	lexem = NULL;
}

TreeNode* TreeNode::form_expression_tree(List* lexems, int first_index, int last_index)
{
	TreeNode* res = new TreeNode();
	int max_code = -1;
	int priority_index = -1;
	if (first_index == last_index)
	{
		return NULL;
	}
	// если осталась одна лексема, то просто пишем её в нод - это лист дерева
	if (last_index - first_index == 1)
	{
		Lexem* lex = (Lexem*)lexems->get(first_index);
		if (lex->type == FUNCTION)
		{
			res->lexem = lex;
			switch ((FLOW_CONTROLS)lex->code)
			{
				case INPUT:
				{
					Lexem* variable_lex = (Lexem*)lexems->get(first_index + 2);
					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index + 2, first_index + 3));
					res->expression_end = first_index + 4;
				}
					break;
				case OUTPUT:
				{
					Lexem* opening_brace = (Lexem*)lexems->get(first_index + 1);
					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index + 2, opening_brace->pair_brace_position));
					res->expression_end = opening_brace->pair_brace_position + 1;
				}
					break;
				default:
				{
					std::ostringstream oss;
					oss << "invalid function on line " << lex->line;
					throw std::runtime_error(oss.str());
				}
					break;
				
					
			}
			return res;

		}
		if (lex->type == FLOW_CONTROL)
		{
			res->lexem = lex;
			switch ((FLOW_CONTROLS)lex->code)
			{
				case IF:
				{
					Lexem* condition_start = (Lexem*)lexems->get(first_index + 1);
					if (condition_start->type != PARENTHESES || condition_start->code != (int)NORMAL_OPEN)
					{
						std::ostringstream oss;
						oss << "missing condition statement braces for if operator on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index + 2, condition_start->pair_brace_position));

					int index_if_operator_start = condition_start->pair_brace_position + 1;
					Lexem* if_operator_start = (Lexem*)lexems->get(index_if_operator_start);
					if (if_operator_start->type != PARENTHESES || if_operator_start->code != (int)CURLY_OPEN)
					{
						std::ostringstream oss;
						oss << "missing operator block braces for if operator on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
					int index_if_operator_end = if_operator_start->pair_brace_position;
					Forest *if_operator_forest = new Forest;
					if_operator_forest->form_forest(lexems, index_if_operator_start + 1, index_if_operator_end);
					res->children_forests.push_back(if_operator_forest);
					res->expression_end = index_if_operator_end + 1;
					if (index_if_operator_end < lexems->count() - 1)
					{
						Lexem* potential_else = (Lexem*)lexems->get(index_if_operator_end + 1);
						if (potential_else->type == FLOW_CONTROL && potential_else->code == FLOW_CONTROLS::ELSE)
						{
							Lexem* else_start = (Lexem*)lexems->get(index_if_operator_end + 2);
							Forest* else_forest = new Forest;
							else_forest->form_forest(lexems, index_if_operator_end + 3, else_start->pair_brace_position);
							res->children_forests.push_back(else_forest);
							res->expression_end = else_start->pair_brace_position + 1;
						}
					}
					return res;
				}
					break;
				case FOR:
				{
					Lexem* cycle_specials = (Lexem*)lexems->get(first_index + 1);
					if (cycle_specials->type != PARENTHESES || cycle_specials->code != (int)NORMAL_OPEN)
					{
						std::ostringstream oss;
						oss << "missing cycle statement braces for FOR operator on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
					int index_delim1 = first_index + 1;
					Lexem* delim1 = (Lexem*)lexems->get(index_delim1);
					while (delim1->type != OPERATOR_DELIMITER)
					{
						index_delim1++;
						delim1 = (Lexem*)lexems->get(index_delim1);
					}

					int index_delim2 = index_delim1 + 1;
					Lexem* delim2 = (Lexem*)lexems->get(index_delim2);
					while (delim2->type != OPERATOR_DELIMITER)
					{
						index_delim2++;
						delim2 = (Lexem*)lexems->get(index_delim2);
					}

					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index + 2, index_delim1));
					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, index_delim1 + 1, index_delim2));
					res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, index_delim2 + 1, cycle_specials->pair_brace_position));
					
					int index_operatock_block_start = cycle_specials->pair_brace_position + 1;
					Lexem* operator_start = (Lexem*)lexems->get(index_operatock_block_start);
					if (operator_start->type != PARENTHESES || operator_start->code != (int)CURLY_OPEN)
					{
						std::ostringstream oss;
						oss << "missing operator block braces for FOR operator on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
					int index_if_operator_end = operator_start->pair_brace_position;
					Forest *operator_forest = new Forest;
					operator_forest->form_forest(lexems, index_operatock_block_start + 1, index_if_operator_end);
					res->children_forests.push_back(operator_forest);
					res->expression_end = index_if_operator_end + 1;
					return res;
				}
					break;
			}
		}
		if (lex->type != VARIABLE_NAME && lex->type != NUMERIC_CONST)
		{
			throw std::runtime_error("tree formation error");
		}
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
	switch (res->lexem->op_type)
	{
	case UNARY_PREFIX:
		res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, priority_index + 1, last_index));
		break;
	case UNARY_POSTFIX:
		res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index, priority_index)); 
		break;
	case BINARY:
		res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, first_index, priority_index));
		res->children_nodes.push_back(TreeNode::form_expression_tree(lexems, priority_index + 1, last_index));
		break;
	default: 
	{
		std::ostringstream oss;
		oss << "unknown operation (form tree) on line " << res->lexem->line;
		throw std::runtime_error(oss.str());
	}
		break;
	}
	return res;
}

Value* TreeNode::solve_assignment_multiplication(Value* lvalue, Value* rvalue)
{
	lvalue->SetValue(lvalue->GetValue()*rvalue->GetValue());
	return lvalue;
}

Value* TreeNode::solve()
{
	Value* lvalue = NULL;
	Value* rvalue = NULL;
	switch (lexem->type)
	{
	case LexemType::OPERATION:
		
		switch ((OPERATIONS)lexem->code)
		{
		case ASSIGNMENT:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_assignment(lvalue, rvalue);
			break;
		case ASSIGNMENT_MULTIPLICATION:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_assignment_multiplication(lvalue, rvalue);
			break;
		case BINARY_PLUS:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_addition(lvalue, rvalue);
			break;
		case BINARY_MINUS:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_subtraction(lvalue, rvalue);
			break;
		case MULTIPLICATION:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_multiplication(lvalue, rvalue);
			break;
		case IS_EQUAL:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_is_equal(lvalue, rvalue);
			break;
		case LESS_THAN:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_less_than(lvalue, rvalue);
			break;
		default:
			throw std::runtime_error("unknown operation");
			break;
		case LESS_OR_EQUAL:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_less_or_equal(lvalue, rvalue);
			break;
		case GREATER_THAN:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_greater_than(lvalue, rvalue);
			break;
		case GREATER_OR_EQUAL:
			lvalue = children_nodes[0]->solve();
			rvalue = children_nodes[1]->solve();
			return solve_greater_or_equal(lvalue, rvalue);
			break;
		case UNARY_MINUS: 
			return solve_unary_minus(children_nodes[0]->solve());
			break;
		case POSTFIX_INCREMENT: 
			return solve_postfix_increment(children_nodes[0]->solve());
			break;
		case PREFIX_INCREMENT: 
			return solve_prefix_increment(children_nodes[0]->solve());
			break;

		}
		break;
	case VARIABLE_NAME: 
	{
		return lexem->variable->value;
//		Variable_Record* res = variables->get_variable(lexem->word_string);
//		return res->value;
	}
		break;
	case FLOW_CONTROL:
		switch (lexem->code)
		{
			case IF:
			{
				double res = children_nodes[0]->solve()->GetValue();
				if (res)
				{
					children_forests[0]->solve();
				}
				else
				{
					if (children_forests.size() == 2)
					{
						children_forests[1]->solve();
					}
				}
			}
				break;
			case FOR:
			{
				Forest* operator_block = children_forests[0];
				TreeNode* initial = children_nodes[0];
				if (initial != NULL)
				{
					initial->solve();
				}
				TreeNode* condition = children_nodes[1];
				TreeNode* cyclic = children_nodes[2];
				while (condition == NULL || condition->solve()->GetValue())
				{
					operator_block->solve();

					if (cyclic != NULL)
					{
						cyclic->solve();
					}
				}
			}
				break;
			default:
			{
				std::ostringstream oss;
				oss << "control flow error on line " << lexem->line;
				throw std::runtime_error(oss.str());
			}
				break;
		}
		break;
	case FUNCTION:
	{
		switch(lexem->code)
		{
			case INPUT:
			{
				double temp;
				std::cin >> temp;
				Value *value = children_nodes[0]->solve();
				value->SetValue(temp);
			}
			break;
			case OUTPUT:
			{
				Value *value = children_nodes[0]->solve();
				switch (value->GetType())
				{
				case INT: 
					std::cout << (int)value->GetValue() << std::endl;
					break;
				case DOUBLE: 
					std::cout << (double)value->GetValue() << std::endl;
					break;
				case BOOL: 
					std::cout << (bool)value->GetValue() << std::endl;
					break;
				default: break;
				}
			}
			break;
			default:
			{
				std::ostringstream oss;
				oss << "function flow error on line " << lexem->line;
				throw std::runtime_error(oss.str());
			}
				break;
		}
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

Value* TreeNode::solve_less_than(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(BOOL);
	res->SetValue(lvalue->GetValue() < rvalue->GetValue());
	return res;
}

Value* TreeNode::solve_less_or_equal(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(BOOL);
	res->SetValue(lvalue->GetValue() <= rvalue->GetValue());
	return res;
}

Value* TreeNode::solve_greater_than(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(BOOL);
	res->SetValue(lvalue->GetValue() > rvalue->GetValue());
	return res;
}

Value* TreeNode::solve_greater_or_equal(Value* lvalue, Value* rvalue)
{
	Value* res = new Value(BOOL);
	res->SetValue(lvalue->GetValue() >= rvalue->GetValue());
	return res;
}

Value* TreeNode::solve_unary_minus(Value* value)
{
	Value* res = new Value(value->GetType());
	res->SetValue((-1)*value->GetValue());
	return res;
}

Value* TreeNode::solve_postfix_increment(Value* value)
{
	Value* res = new Value(value->GetType());
	res->SetValue(value->GetValue());
	value->SetValue(value->GetValue() + 1);
	return res;
}

Value* TreeNode::solve_prefix_increment(Value* value)
{
	Value* res = new Value(value->GetType());
	value->SetValue(value->GetValue() + 1);
	res->SetValue(value->GetValue());
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
