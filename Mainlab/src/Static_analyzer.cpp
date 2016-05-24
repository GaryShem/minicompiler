#include "../include/Static_analyzer.h"
#include "../include/list.h"
#include "../include/variable.h"
#include <fstream>
#include <iostream>

static Heap heap;

void StaticAnalyzer::form_lexem_list()
{
	automaton automatons[]{
		automaton("keywords_triads.txt"),
		automaton("numeric_constant_triads.txt", '?'),
		automaton("parentheses_triads.txt", '?', false),
		automaton("variable_name_triads.txt", '?'),
		automaton("operation_triads.txt"),
		automaton("operation_delimiter.txt")
	};
	//	automaton keyword_tomaton("keywords_triads.txt");
	//	automaton var_name_tomaton("variable_name_triads.txt");
	//	automaton numeric_constant_tomaton("numeric_constant_triads.txt");
	//	automaton operation_tomaton("operation_triads.txt");


	std::ifstream file(_filename);
	char* program_line = (char*)heap.get_mem(10000);
	//	char meaningless_symbols[] = " \t\r\n";
	int line_number = 0;
	while (file.getline(program_line, 10000))
	{
		Lexem* lex;
		int index = 0;
		while (program_line[index] != 0)
		{
			for (int i = 0; i < 6; i++)
			{
				lex = automatons[i].check(program_line, index);
				if (lex != NULL)
				{
					switch (i)
					{
					case 0:
						lex->type = LexemType::KEYWORD;
						break;
					case 1:
					{
						lex->type = LexemType::NUMERIC_CONST;
						lex->const_value = 0;
						for (int zzz = lex->starting_position; zzz < lex->last_position; zzz++)
						{
							lex->const_value = lex->const_value * 10 + (program_line[zzz] - '0');
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
						lex->word_string = new char[name_length+1];
						strncpy(lex->word_string, program_line + lex->starting_position, name_length);
						lex->word_string[name_length] = 0;
					}
						break;
					case 4:
						lex->type = LexemType::OPERATION;
						switch (lex->code)
						{
						case 11111: // присваивание всегда бинарное
						case 10100:
							lex->op_type = BINARY;
							break;
						default: 
							lex->op_type = UNARY_OR_BINARY;
						}
						break;
					case 5:
						lex->type = LexemType::OPERATOR_DELIMITER;
						break;
					default:
						throw 101548;
					}

					_lexems.add(lex);
					index = lex->last_position - 1;
					break;
				}
			}
			index++;
		}
	}

	for (int i = 0; i < _lexems.count(); i++)
	{
		Lexem* lex = (Lexem*)_lexems.get(i);
		std::cout << i << ": " << lex->starting_position << " " << lex->last_position << " " << lex->type << " ";
		switch (lex->type)
		{
		case LexemType::KEYWORD:
			std::cout << "Keyword";
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
		std::cout << "Error on lexem " << i << std::endl;
		return i;
	}
	return 0;
}

int StaticAnalyzer::check_variables_existence()
{
	int limit = _lexems.count();
	for (int i = 0; i < limit; i++)
	{
		Lexem* lexem = (Lexem*)_lexems.get(i);
		if (lexem->type != LexemType::VARIABLE_NAME)
		{
			continue;
		}
		// если нашли переменную, то провер€ем, существует ли она
		Variable_Record* variable = _variables.get_variable(lexem->word_string);

		// если переменна€ не объ€влена
		if (variable == NULL )
		{
			if (i == 0 || ((Lexem*)_lexems.get(i - 1))->type != LexemType::KEYWORD)
			{
				std::cout << "Use of undeclared identifier in lexem " << i << std::endl;
			}
			else
			{
				variable = new Variable_Record(lexem->word_string);
				Lexem* previous_lexem = (Lexem*)_lexems.get(i - 1);
				variable->value = new Value((VARIABLE_TYPE)(previous_lexem->code));

				_variables.add(variable);
			}
		}
		// если переменна€ объ€вл€етс€ второй раз
		else if (variable != NULL && ((Lexem*)_lexems.get(i - 1))->type == LexemType::KEYWORD)
		{
			std::cout << "Second declaration of variable " << variable->name << " in lexem " << i << std::endl;
		}
	}
	return 0;
}

void StaticAnalyzer::print_variables()
{
	int limit = _variables.count();
	for (int i = 0; i < limit; i++)
	{
		Variable_Record* var = (Variable_Record*)_variables.get(i);
		std::cout << var->name << "   ";
		switch (var->value->type)
		{
		case INT:
			std::cout << "int " << *(int*)var->value->value << std::endl;
			break;
		case DOUBLE:
			std::cout << "double " << *(int*)var->value->value << std::endl;
			break;
		case CHAR:
			std::cout << "char " << *(int*)var->value->value << std::endl;
			break;
		default:
			std::cout << "UNKNOWN! ACHTUNG!";
			break;
		} 
	}
}

bool StaticAnalyzer::find_pair_parentheses(int closing_parenthesis_code, int* brace_position)
{
	int limit = _lexems.count();
	int i = *brace_position + 1;
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
