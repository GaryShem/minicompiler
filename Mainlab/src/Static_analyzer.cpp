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
		automaton("triads/keywords.txt"),
		automaton("triads/numeric_constants.txt", '?'),
		automaton("triads/braces.txt", '?', false),
		automaton("triads/variable_names.txt", '?'),
		automaton("triads/operations.txt"),
		automaton("triads/operation_delimiter.txt")
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
			for (int i = 0; i < 6; i++)
			{
				lex = automatons[i].check(program_line, index);
				if (lex != NULL)
				{
					lex->line = line_number;
					switch (i)
					{
						case 0:
							if (lex->code >= 11000)
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
						{
							lex->type = LexemType::NUMERIC_CONST;
							lex->const_value = 0;
							for (int zzz = lex->starting_position; zzz < lex->last_position; zzz++)
							{
								int base = 10;
								lex->const_value = lex->const_value * base + (program_line[zzz] - '0');
							}
						}
						break;
						case 2:
							lex->type = LexemType::PARENTHESES;
							break;
						case 3:
						{
							lex->type = LexemType::VARIABLE_NAME;
							int name_length = lex->last_position - lex->starting_position;
							lex->word_string = new char[name_length + 1];
							strncpy(lex->word_string, program_line + lex->starting_position, name_length);
							lex->word_string[name_length] = 0;
						}
						break;
						case 4:
							lex->type = LexemType::OPERATION;
							if (lex->code >= 20000)
							{
								lex->op_type = UNARY_OR_BINARY;
							}
							else if (lex->code >= 19000)
							{
								lex->code = UNARY;
							}
							else 
							{
								lex->code = BINARY;
							}
						break;
					case 5:
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
			std::cout << "Variable";
			break;
		case LexemType::OPERATION:
			std::cout << "Operation";
			break;
		case LexemType::OPERATOR_DELIMITER:
			std::cout << "Delimiter (;)";
			break;
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
	int limit = _lexems.count();
	int list_number;
	if (current_variable_list == NULL)
	{
		std::cout << "Starting variable check" << std::endl;
		current_variable_list = &_global_variables;

	}
	list_number = current_variable_list->_list_number + 1;
	for (int i = starting_index; i < limit; i++)
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
				current_variable_list->_children_list.add(new_variable_list);
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
			if (i == 0 || ((Lexem*)_lexems.get(i - 1))->type != LexemType::KEYWORD)
			{
				std::cout << "Use of undeclared identifier in lexem " << i << " on line " << lexem->line << std::endl;
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
		else if (variable != NULL && ((Lexem*)_lexems.get(i - 1))->type == LexemType::KEYWORD)
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
				std::cout << "UNKNOWN! ACHTUNG!" << std::endl;
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
		try
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
			case FLOW_CONTROL: break;
			case VARIABLE_NAME: break;
			case NUMERIC_CONST: break;
			case OPERATION: 
			{
				switch (lex->op_type)
				{
				case UNARY:
				{

				}
					break;
				case BINARY:
				{
					// провер€ем предыдущую лексему
					// перед бинарной операцией могут лежать: константа, переменна€, закрывающа€с€ кругла€ или квадратна€ скобка
					if (i == 0)
					{
						std::ostringstream oss;
						oss << "Missing left operand on line " << lex->line << std::endl;
						throw std::runtime_error(oss.str());
					}
					else
					{
						Lexem* previous_lex = (Lexem*)_lexems.get(i - 1);
						if (previous_lex->type != NUMERIC_CONST && previous_lex->type != VARIABLE_NAME && previous_lex->type != PARENTHESES)
						{
							std::ostringstream oss;
							oss << "Missing left operand on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
						if (previous_lex->type == PARENTHESES && (previous_lex->code != SQUARE_CLOSE && previous_lex->code != NORMAL_CLOSE))
						{
							std::ostringstream oss;
							oss << "Missing left operand on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
					}
					// теперь смотрим справа от оператора
					Lexem* next_lexem = (Lexem*)_lexems.get(i + 1);
					if (next_lexem->type != NUMERIC_CONST && next_lexem->type != VARIABLE_NAME && next_lexem->type != PARENTHESES)
					{
						std::ostringstream oss;
						oss << "Missing left operand on line " << lex->line << std::endl;
						throw std::runtime_error(oss.str());
					}
					if (next_lexem->type == PARENTHESES && next_lexem->code != NORMAL_OPEN)
					{
						std::ostringstream oss;
						oss << "Missing left operand on line " << lex->line << std::endl;
						throw std::runtime_error(oss.str());
					}
				}
					break;
				case UNARY_OR_BINARY: 
				{
					Lexem* previous_lex;
					Lexem* next_lex = (Lexem*)_lexems.get(i + 1);
					if (i == 0)
					{
						if (next_lex->type != VARIABLE_NAME && next_lex->type != NUMERIC_CONST && next_lex->type != PARENTHESES )
						{
							std::ostringstream oss;
							oss << "Missing right unary operand on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
						if (next_lex->type == PARENTHESES && next_lex->code != NORMAL_OPEN)
						{
							std::ostringstream oss;
							oss << "Missing right unary operand on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
						lex->op_type = UNARY;
						lex->code -= 5000;
					}
					else
					{
						previous_lex = (Lexem*)_lexems.get(i - 1);
						// если слева точка с зап€той, то это вс€ко унарный оператор, и справа должен быть операнд
						if (previous_lex->type == OPERATOR_DELIMITER && (next_lex->type == VARIABLE_NAME || next_lex->type == NUMERIC_CONST || (next_lex->type == PARENTHESES && next_lex->code == NORMAL_OPEN)))
						{
							lex->op_type = UNARY;
							lex->code -= 5000;
							break;
						}
						// если слева и справа переменные или константы, то бинарна€ операци€
						if ((previous_lex->type == NUMERIC_CONST || previous_lex->type == VARIABLE_NAME) && (next_lex->type == NUMERIC_CONST || next_lex->type == VARIABLE_NAME))
						{
							lex->op_type = BINARY;
							lex->code -= 10000;
							break;
						}
						// если справа закрывающа€с€ скобка, то всЄ плохо
						if (next_lex->type == PARENTHESES && next_lex->code != NORMAL_OPEN)
						{
							std::ostringstream oss;
							oss << "Missing right operand on line " << lex->line << std::endl;
							throw std::runtime_error(oss.str());
						}
						// если слева скобка
						if (previous_lex->type == PARENTHESES)
						{
							// если она открывающа€с€ - оператор унарный
							if (previous_lex->code == NORMAL_OPEN)
							{
								lex->op_type = UNARY;
								lex->code -= 5000;
								break;
							}
							// если закрывающа€с€ - бинарный
							else if (previous_lex->code == NORMAL_CLOSE)
							{
								lex->op_type = BINARY;
								lex->code -= 10000;
								break;
							}
							// а иначе всЄ тлен
							else
							{
								std::ostringstream oss;
								oss << "Missing left operand on line " << lex->line << std::endl;
								throw std::runtime_error(oss.str());
							}
						}
					}
					previous_lex = (Lexem*)_lexems.get(i - 1);
				}
				break;
				default: break;
				}
			}
				break;
			case OPERATOR_DELIMITER: break;
			case PARENTHESES: break;
			case COMMA: break;
			default: break;
			}
		}
		catch (std::runtime_error err)
		{
			std::cout << err.what() << std::endl;
		}
	}
}

void StaticAnalyzer::print_variables(int indent)
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
			std::cout << "double " << (double)var->GetValue()<< std::endl;
			break;
		default:
			std::cout << "UNKNOWN! ACHTUNG!" << std::endl;
			break;
		}
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
