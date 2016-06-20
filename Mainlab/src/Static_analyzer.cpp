#include "../include/Static_analyzer.h"
#include "../include/list.h"
#include "../include/variable.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static Heap heap;

void StaticAnalyzer::form_lexem_list()
{
	automaton automatons[]{
		automaton("triads/keywords.txt"), //0
		automaton("triads/functions.txt"), //1
		automaton("triads/numeric_constants.txt", '?'), //2
		automaton("triads/braces.txt", '?', false), //3
		automaton("triads/variable_names.txt", '?'), //4
		automaton("triads/operations.txt"), //5
		automaton("triads/operation_delimiter.txt"), //6
	};


	std::ifstream file(_filename);
	char* program_line = (char*)heap.get_mem(10000);
	//	char meaningless_symbols[] = " \t\r\n";
	int line_number = 0;
	while (file.getline(program_line, 10000))
	{
		line_number++;
		Lexem* lex;
		int index = 0;
		while (program_line[index] != 0)
		{
			for (int i = 0; i < 7; i++)
			{
				lex = automatons[i].check(program_line, index);
				if (lex != NULL)
				{
					lex->line = line_number;
					switch (i)
					{
						case 0:
							if (lex->code >= 30000)
							{
								lex->type = LexemType::FLOW_CONTROL;
							}
							else if (lex->code >= 10300)
							{
								lex->type = LexemType::VARIABLE_DECLARATION;
							}
							else
							{
								lex->type = LexemType::KEYWORD;
							}
							break;
						case 1:
							lex->type = FUNCTION;
							break;
						case 2:
						{
							lex->type = LexemType::NUMERIC_CONST; //1.38e-3 015 0x15
							lex->const_value = 0;
							for (int zzz = lex->starting_position; zzz < lex->last_position; zzz++)
							{
								int base = 10;
								lex->const_value = lex->const_value * base + (program_line[zzz] - '0');
							}
						}
						break;
						case 3:
							lex->type = LexemType::PARENTHESES;
							break;
						case 4:
						{
							lex->type = LexemType::VARIABLE_NAME;
							int name_length = lex->last_position - lex->starting_position;
							lex->word_string = new char[name_length + 1];
							strncpy(lex->word_string, program_line + lex->starting_position, name_length);
							lex->word_string[name_length] = 0;
						}
						break;
						case 5:
							lex->type = LexemType::OPERATION;
							if (lex->code >= 20000)
							{
								lex->op_type = UNARY_OR_BINARY;
							}
							else if (lex->code >= 19000)
							{
								// унарна€ операци€, про которую нельз€ сразу сказать, постфиксна€ она или префиксна€ - например ++
								// потом уточн€етс€ (в check_syntax в этом же классе)
								lex->op_type = UNARY;
							}
							else 
							{
								lex->op_type = BINARY;
							}
						break;
					case 6:
						lex->type = LexemType::OPERATOR_DELIMITER;
						break;
					default:
						throw std::runtime_error(std::string("Syntax error on line ") + std::to_string(line_number));
					}

					_lexems.add(lex);
					index = lex->last_position - 1;
					break;
				}
			}
			index++;
		}
	}

}

void StaticAnalyzer::print_lexem_list()
{
	for (int i = 0; i < _lexems.count(); i++)
	{
		Lexem* lex = (Lexem*)_lexems.get(i);
		std::cout << i << ": (" << lex->code << ") " << lex->starting_position << " " << lex->last_position << " " << lex->type << " ";
		switch (lex->type)
		{
		case LexemType::KEYWORD:
			std::cout << "Keyword (Unspecified)";
			break;
		case LexemType::VARIABLE_DECLARATION:
			std::cout << "Keyword (Variable Declaration)";
			break;
		case LexemType::FLOW_CONTROL:
			std::cout << "Keyword (Flow Control)";
			break;
		case LexemType::NUMERIC_CONST:
			std::cout << "Constant";
			break;
		case LexemType::PARENTHESES:
			std::cout << "Parentheses";
			break;
		case LexemType::VARIABLE_NAME:
			std::cout << "Variable " << lex->word_string;
			break;
		case LexemType::OPERATION:
			std::cout << "Operation";
			break;
		case LexemType::OPERATOR_DELIMITER:
			std::cout << "Delimiter (;)";
			break;
		case FUNCTION: 
			std::cout << "Function";
			break;
		default: break;
		}
		std::cout << std::endl;
	}
}

int StaticAnalyzer::check_parentheses()
{
	bool result = true;
	int limit = _lexems.count();
	int i = 0;
	while (i < limit)
	{
		Lexem* current_lexem = (Lexem*)_lexems.get(i);
		if (current_lexem->type == LexemType::PARENTHESES)
		{
			int code = current_lexem->code;
			//у закрывающихс€ скобок нечЄтный код
			if (code%2==1)
			{
				result = false;
				break;
			}
			if (code%2==0)
			{
				result = find_pair_parentheses(code + 1, &i);
				if (result == false)
					break;
			}
		}
		i++;
	}
	if (result == false)
	{
		std::ostringstream oss;
		oss << "Error on lexem " << i << " (line " << ((Lexem*)_lexems.get(i))->line << ")" << ": unpaired braces" << std::endl;
		throw std::runtime_error(oss.str());
	}
	return 0;
}

int StaticAnalyzer::check_variables_existence(Variable_List* current_variable_list, int starting_index, int deleted_lexems)
{
	int list_number;
	if (current_variable_list == NULL)
	{
		std::cout << "Starting variable check" << std::endl;
		current_variable_list = &_global_variables;

	}
	list_number = current_variable_list->_list_number + 1;
	for (int i = starting_index; i < _lexems.count(); i++)
	{
		Lexem* lexem = (Lexem*)_lexems.get(i);
		if (lexem->type != LexemType::VARIABLE_NAME && lexem->type != LexemType::PARENTHESES)
		{
			continue;
		}

		// если нашли открывающуюс€ фигурную скобку, то рекурсивно запускаем программу,
		// создава€ при этом новый список переменных - под новую область видимости
		if (lexem->type == LexemType::PARENTHESES)
		{
			if (lexem->code == CURLY_OPEN)
			{
				list_number++;
				Variable_List* new_variable_list = new Variable_List(current_variable_list, list_number);
				current_variable_list->_children_list.add(&new_variable_list);
				check_variables_existence(new_variable_list, i+1, deleted_lexems);
				i = lexem->pair_brace_position - deleted_lexems;
			}
			else if (lexem->code == CURLY_CLOSE)
			{
				return 0;
			}
			continue;
		}

		// если нашли переменную, то провер€ем, существует ли она
		// при этом поиск идЄт с текущей области видимости "вверх" до глобальной
		Variable_Record* variable = current_variable_list->get_variable(lexem->word_string);

		// если переменна€ не объ€влена
		if (variable == NULL)
		{
			if (i == 0 || ((Lexem*)_lexems.get(i - 1))->type != LexemType::VARIABLE_DECLARATION)
			{
				std::ostringstream oss;
				oss << "Use of undeclared identifier in lexem " << i << " on line " << lexem->line;
				throw std::runtime_error(oss.str());
			}
			// объ€вл€ем переменную, убираем весь кусок из списка лексем
			else
			{
				variable = new Variable_Record(lexem->word_string);
				Lexem* previous_lexem = (Lexem*)_lexems.get(i - 1);
				variable->value = new Value((VARIABLE_TYPE)(previous_lexem->code));

				current_variable_list->add(variable);
				Variable_Record* vr = current_variable_list->get_variable(lexem->word_string);
				lexem->variable = vr;
				Lexem temp;
				// удал€ем лексему с ключевым словом из списка
				_lexems.take(i - 1, &temp);
				deleted_lexems++;
			}
		}
		// если переменна€ объ€вл€етс€ второй раз
		else if (variable != NULL && ((Lexem*)_lexems.get(i - 1))->type == LexemType::VARIABLE_DECLARATION)
		{
			std::ostringstream oss;
			oss << "Second declaration of variable " << variable->name << " in lexem " << i << " on line " << lexem->line  << " (variable name: " << lexem->word_string << ")" << std::endl;
			throw std::runtime_error(oss.str());
		}
		else if (variable != NULL)
		{
			lexem->variable = variable;
		}
	}
	return 0;
}

int StaticAnalyzer::check_flow()
{
	int limit = _global_variables.count();
	for (int i = 0; i < limit; i++)
	{
		Variable_Record* var = (Variable_Record*)_global_variables.get(i);
		std::cout << var->name << "   ";
		switch (var->value->type)
		{
			case INT:
				std::cout << "int " << (int)var->GetValue() << std::endl;
				break;
			case DOUBLE:
				std::cout << "double " << (double)var->GetValue() << std::endl;
				break;
			default:
				std::cout << "UNKNOWN! ACHTUNG! (check_flow)" << std::endl;
				break;
		}
	}
	return NULL;
}

int StaticAnalyzer::check_syntax(int lexem_index)
{
	int limit = _lexems.count();
	for (int i = lexem_index; i < limit; i++)
	{
			Lexem* lex = (Lexem*)_lexems.get(i);
			switch (lex->type)
			{
			case KEYWORD: 
				throw std::runtime_error(std::string("Something went wrong"));
				break;
			case VARIABLE_DECLARATION:
			{
				// провер€ем предыдущую лексему
				// объ€вление переменной может идти после точки с зап€той или после фигурной скобки
				if (i != 0)
				{
					Lexem* previous_lex = (Lexem*)_lexems.get(i - 1);
					if (previous_lex->type == OPERATOR_DELIMITER)
					{
					}
					else if (previous_lex->type == PARENTHESES)
					{
						if (previous_lex->code != CURLY_OPEN && previous_lex->code != CURLY_CLOSE)
						{
							std::ostringstream oss;
							oss << "Incorrect variable declaration on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
					}
					else
					{
						std::ostringstream oss;
						oss << "Incorrect variable declaration on line " << lex->line << std::endl;
						throw std::runtime_error(oss.str());
					}
				}
				// теперь провер€ем, что следующа€ лексема - им€ переменной
				Lexem* next_lex = (Lexem*)_lexems.get(i + 1);
				if (next_lex->type != VARIABLE_NAME)
				{
					std::ostringstream oss;
					oss << "Missing identifier after declaration on line " << lex->line << std::endl;
					throw std::runtime_error(oss.str());
				}
			}
				break;
			case FLOW_CONTROL:
			{
				if (i == 0)
				{
					break;
				}
				Lexem* previous_lexem = (Lexem*)_lexems.get(i - 1);
				if (previous_lexem->type != OPERATOR_DELIMITER && (previous_lexem->type != PARENTHESES || (previous_lexem->code != CURLY_OPEN && previous_lexem->code != CURLY_CLOSE)))
				{
					std::ostringstream oss;
					oss << "missing semicolon/brace before flow control keyword";
					throw std::runtime_error(oss.str());
				}
			}
				break;
			case VARIABLE_NAME: 
			case NUMERIC_CONST: 
			{
				int surroundings = 0;
				if (i == 0)
				{
					surroundings++;
				}
				else
				{
					Lexem* previous_lexem = (Lexem*)_lexems.get(i - 1);
					if (previous_lexem->type == PARENTHESES || previous_lexem->type == OPERATION || previous_lexem->type == OPERATOR_DELIMITER)
					{
						surroundings++;
					}
				}
				if (i == _lexems.count()-1)
				{
//					surroundings += 2;
				}
				else
				{
					Lexem* next_lexem = (Lexem*)_lexems.get(i + 1);
					if (next_lexem->type == PARENTHESES || next_lexem->type == OPERATION || next_lexem->type == OPERATOR_DELIMITER)
					{
						surroundings += 2;
					}
				}
				switch(surroundings)
				{
					
					case 3:
						break;
					default:
					{
						std::ostringstream oss;
						oss << "invalid formatting, are you missing a semicolon near line " << lex->line << " ?";
						throw std::runtime_error(oss.str());
					}
						break;
				}
			}
				break;
			case OPERATION: 
			{
				switch (lex->op_type)
				{
				case UNARY:
				{
					check_unary_op(i);
				}
					break;
				case BINARY:
				{
					check_binary_op(i);
				}
					break;
				case UNARY_OR_BINARY: 
				{
					check_unknown_op(i);
					if (lex->op_type == UNARY)
					{
						check_unary_op(i);
					}
				}
				break;
				default: break;
				}
			}
				break;
			case FUNCTION:
			{
				if (i == 0)
				{
					break;
				}
				Lexem* previous_lexem = (Lexem*)_lexems.get(i - 1);
				if (previous_lexem->type != OPERATOR_DELIMITER && (previous_lexem->type != PARENTHESES || (previous_lexem->code != CURLY_OPEN && previous_lexem->code != CURLY_CLOSE)))
				{
					std::ostringstream oss;
					oss << "missing semicolon/brace before function call";
					throw std::runtime_error(oss.str());
				}
				if (lex->code == INPUT)
				{
					Lexem* opening_brace = (Lexem*)_lexems.get(i + 1);
					Lexem* input_variable = (Lexem*)_lexems.get(i + 2);
					if (opening_brace->type != PARENTHESES || opening_brace->code != NORMAL_OPEN || opening_brace->pair_brace_position - (i+1) != 2 || input_variable->type != VARIABLE_NAME)
					{
						std::ostringstream oss;
						oss << "function format error on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
				}
				else if (lex->code == OUTPUT)
				{
					Lexem* opening_brace = (Lexem*)_lexems.get(i + 1);
					if (opening_brace->type != PARENTHESES || opening_brace->code != NORMAL_OPEN)
					{
						std::ostringstream oss;
						oss << "function format error on line " << lex->line;
						throw std::runtime_error(oss.str());
					}
				}
			}
			break;
			case OPERATOR_DELIMITER: break;
			case PARENTHESES: break;
			default: 
			{
				std::ostringstream oss;
				oss << "unknown lexem on line " << lex->line;
				throw std::runtime_error(oss.str());
			}
				break;
			}
	}
}

void StaticAnalyzer::check_unary_op(int index)
{
	Lexem* lex = (Lexem*)_lexems.get(index);
	Lexem* previous_lex;
	Lexem* next_lex;
		if (_lexems.count() == 1)
		{
			std::ostringstream oss;
			oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
	if (index == 0)
	{

		next_lex = (Lexem*)_lexems.get(index + 1);
		if (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST ||
			(next_lex->type == PARENTHESES && next_lex->code == NORMAL_CLOSE))
		{
			lex->op_type = UNARY_PREFIX;
			lex->code = lex->code - 9000 + 1;
			return;
		}
		else
		{
			std::ostringstream oss;
			oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
	}
	if (index == _lexems.count()-1)
	{
		previous_lex = (Lexem*)_lexems.get(index - 1);
		if (previous_lex->type == VARIABLE_NAME || previous_lex->type == NUMERIC_CONST || 
			(previous_lex->type == PARENTHESES && previous_lex->code == NORMAL_CLOSE))
		{
			lex->op_type = UNARY_POSTFIX;
			lex->code = lex->code - 9000;
			return;
		}
		else
		{
			std::ostringstream oss;
			oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
	}
	next_lex = (Lexem*)_lexems.get(index + 1);
	previous_lex = (Lexem*)_lexems.get(index - 1);
	int surroundings = 0;
	if (previous_lex->type == VARIABLE_NAME || previous_lex->type == NUMERIC_CONST ||
		(previous_lex->type == PARENTHESES && previous_lex->code == NORMAL_CLOSE))
	{
		surroundings++;
	}
	if (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST ||
		(next_lex->type == PARENTHESES && next_lex->code == NORMAL_OPEN))
	{
		surroundings += 2;
	}
	switch (surroundings)
	{
		case 0: 
		{
			std::ostringstream oss;
			oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
			break;
		case 1:
			lex->op_type = UNARY_POSTFIX;
			lex->code = lex->code - 9000;
			break;
		case 2:
			lex->op_type = UNARY_PREFIX;
			lex->code = lex->code - 9000 + 1;
			break;
		case 3:
		{
			std::ostringstream oss;
			oss << "unary operation error - two operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
			break;
	}
}

void StaticAnalyzer::check_binary_op(int index)
{
	Lexem* lex = (Lexem*)_lexems.get(index);
	// провер€ем предыдущую лексему
	// перед бинарной операцией могут лежать: константа, переменна€, закрывающа€с€ кругла€
	// за ней - то же самое, но кругла€ открывающа€с€
	if (index == 0 || index == _lexems.count() - 1)
	{
		std::ostringstream oss;
		oss << "Missing left operand on line " << lex->line << std::endl;
		throw std::runtime_error(oss.str());
	}
	Lexem* next_lex = (Lexem*)_lexems.get(index + 1);
	Lexem* previous_lex = (Lexem*)_lexems.get(index - 1);
	int surroundings = 0;
	if (previous_lex->type == VARIABLE_NAME || previous_lex->type == NUMERIC_CONST ||
		(previous_lex->type == PARENTHESES && previous_lex->code == NORMAL_CLOSE))
	{
		surroundings++;
	}
	if (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST ||
		(next_lex->type == PARENTHESES && next_lex->code == NORMAL_OPEN))
	{
		surroundings += 2;
	}
	switch (surroundings)
	{
		case 3:
			break;
		default:
		{
			std::ostringstream oss;
			oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
			throw std::runtime_error(oss.str());
		}
		break;
	}
}

void StaticAnalyzer::check_unknown_op(int index)
{
	Lexem* lex = (Lexem*)_lexems.get(index);
	Lexem* previous_lex;
	Lexem* next_lex;
	if (_lexems.count() < 2)
	{
		std::ostringstream oss;
		oss << "not enough lexems to perform operation!" << std::endl;
		throw std::runtime_error(oss.str());
	}
	if (index == 0)
	{
		next_lex = (Lexem*)_lexems.get(index + 1);
		if (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST ||
			(next_lex->type == PARENTHESES && next_lex->code == NORMAL_OPEN))
		{
			lex->op_type = UNARY_PREFIX;
			lex->code -= 10000;
			return;
		}
		else 
		{
			std::ostringstream oss;
			oss << "Missing right unary operand on line " << lex->line << std::endl;
			throw std::runtime_error(oss.str());
		}
	}
	else if (index == _lexems.count()-1)
	{
		previous_lex = (Lexem*)_lexems.get(index - 1);
		if (previous_lex->type == VARIABLE_NAME || previous_lex->type == NUMERIC_CONST ||
			(previous_lex->type == PARENTHESES && previous_lex->code == NORMAL_CLOSE))
		{
			lex->op_type = UNARY_POSTFIX;
			lex->code -= 10001;
			return;
		}
		else
		{
			std::ostringstream oss;
			oss << "Missing right unary operand on line " << lex->line << std::endl;
			throw std::runtime_error(oss.str());
		}
	}
	else
	{
		next_lex = (Lexem*)_lexems.get(index + 1);
		previous_lex = (Lexem*)_lexems.get(index - 1);
		int surroundings = 0;

		if (previous_lex->type == VARIABLE_NAME || previous_lex->type == NUMERIC_CONST ||
			(previous_lex->type == PARENTHESES && previous_lex->code == NORMAL_CLOSE))
		{
			surroundings++;
		}

		if (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST ||
			(next_lex->type == PARENTHESES && next_lex->code == NORMAL_OPEN))
		{
			surroundings += 2;
		}
		switch (surroundings)
		{
			default:
			{
				std::ostringstream oss;
				oss << "unary operation error - no operands!(check_unary_op, line " << lex->line << ")";
				throw std::runtime_error(oss.str());
			}
			break;
			case 1:
				lex->op_type = UNARY_POSTFIX;
				lex->code -= 10000;
				break;
			case 2:
				lex->op_type = UNARY_PREFIX;
				lex->code -= 10000;
				break;
			case 3:
				lex->op_type = BINARY;
				lex->code -= 5000;
				break;
			break;
		}
	}
}

void StaticAnalyzer::print_variables()
{
	Variable_List *current_list = &_global_variables;
	int limit = current_list->count();
	for (int i = 0; i < limit; i++)
	{
		Variable_Record* var = (Variable_Record*)current_list->get(i);
		std::cout << var->name << "   ";
		switch (var->value->type)
		{
		case INT:
			std::cout << "int " << (int)var->GetValue() << std::endl;
			break;
		case DOUBLE:
			std::cout << "double " << (double)var->GetValue()<< std::endl;
			break;
		case BOOL: 
			std::cout << "bool " << (bool)var->GetValue() << std::endl;
			break;
		default:
		{
			std::ostringstream oss;
			oss << "unknown variable type on var named " << var->name;
			throw std::runtime_error(oss.str());
		}
			break;

		}
	}
	for (int i = 0; i < current_list->_children_list.count(); i++)
	{
		Variable_List *next_list = (*(Variable_List**)current_list->_children_list.get(i));
		print_variable_list_recursive(next_list, 2);
		std::cout << std::endl;
	}
}

void StaticAnalyzer::print_variable_list_recursive(Variable_List* list, int indent)
{
	int limit = list->count();
	for (int i = 0; i < limit; i++)
	{
		Variable_Record* var = (Variable_Record*)list->get(i);
		std::cout << std::string(indent, ' ');
		std::cout << var->name << "   ";
		switch (var->value->type)
		{
			case INT:
				std::cout << "int " << (int)var->GetValue() << std::endl;
				break;
			case DOUBLE:
				std::cout << "double " << (double)var->GetValue() << std::endl;
				break;
			case BOOL:
				std::cout << "bool " << (bool)var->GetValue() << std::endl;
				break;
			default:
			{
				std::ostringstream oss;
				oss << "unknown variable type on var named " << var->name;
				throw std::runtime_error(oss.str());
			}
			break;

		}
	}
	for (int i = 0; i < list->_children_list.count(); i++)
	{
		Variable_List *next_list = (Variable_List*)list->_children_list.get(i);
		print_variable_list_recursive(next_list, indent+2);
		std::cout << std::endl;
	}
}


bool StaticAnalyzer::find_pair_parentheses(int closing_parenthesis_code, int* brace_position)
{
	int limit = _lexems.count();
	int i = *brace_position + 1;
	int opening_brace = *brace_position;
	bool result;
	while (i < limit)
	{
		result = false;
		Lexem* current_lexem = (Lexem*)_lexems.get(i);
		if (current_lexem->type == LexemType::PARENTHESES)
		{
			int code = current_lexem->code;
			*brace_position = i;
			//у закрывающихс€ скобок нечЄтный код
			if (code % 2 == 1)
			{
				if (code == closing_parenthesis_code)
				{
					result = true;
					current_lexem->pair_brace_position = opening_brace;
					((Lexem*)_lexems.get(opening_brace))->pair_brace_position = i;
					break;
				}
				else
				{
					result = false;
					break;
				}
			}
			if (code % 2 == 0)
			{
				result = find_pair_parentheses(code + 1, &i);
				if (result == false)
				{
					break;
				}
			}
		}
		i++;
	}
	return result;
}
