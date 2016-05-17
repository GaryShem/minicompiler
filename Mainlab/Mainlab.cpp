// Mainlab.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "include/heap.h"
#include "include/list.h"
#include "include/list_adv.h"
#include "include/hash.h"
#include "include/automaton.h"
#include <iostream>
#include <string>
#include <fstream>

static Heap heap;

int compar(const void* e1, const void* e2)
{
	int* n1 = (int*)e1;
	int* n2 = (int*)e2;
	return *n1 - *n2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// проверка первой лабы
	/*List list(sizeof(int));

	int a;
	int limit = 22;
	for (int i = 0; i < limit; i++)
	{
		a = i;
		list.add(&a);
	}

	list.sort(false, compar);

	for (int j = 0; j < limit; j++)
	{
		list.take_first(&a);
		std::cout << j << "..." << a << std::endl;
	}


	return 0;*/ 

	//проверка второй лабы

	//для русского языка сделаем setlocale

	/*setlocale(LC_ALL, "Russian");
	Diction diction;
	Article* record;
	record = diction.auto_create("mimimi");
	record = diction.auto_create("Баба-Яга");
	record = diction.auto_create("Розовый слоник");

	record = diction.find("Розовый слоник");

	if (record)
	{
		std::cout << "Word: " << record->word << std::endl;
		std::cout << "Description: " << record->description << std::endl;
	}
	else
	{
		std::cout << "Record not found" << std::endl;
	}

	return 0;*/

	// проверка автомата
//	automaton tomaton("keywords.txt");
//	tomaton.generate_triads();
//	tomaton.print_triads();

//	automaton tomaton2("keywords_triads.txt");
//	tomaton2.read_triads();
//	tomaton2.print_triads();

	automaton automatons[]{
		automaton("keywords_triads.txt"),
		automaton("numeric_constant_triads.txt", '?'),
		automaton("parentheses_triads.txt"),
		automaton("variable_name_triads.txt", '?'),
		automaton("operation_triads.txt"),
		automaton("operation_delimiter.txt")
	};
	//	automaton keyword_tomaton("keywords_triads.txt");
	//	automaton var_name_tomaton("variable_name_triads.txt");
	//	automaton numeric_constant_tomaton("numeric_constant_triads.txt");
	//	automaton operation_tomaton("operation_triads.txt");


	char program[] = "test_program.txt";
	std::ifstream file(program);
	char* program_line = (char*)heap.get_mem(10000);
//	char meaningless_symbols[] = " \t\r\n";

	List lexems(sizeof(Lexem));
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
						lex->type = LexemType::NUMERIC_CONST;
						break;
					case 2:
						lex->type = LexemType::PARENTHESES;
						break;
					case 3:
						lex->type = LexemType::VARIABLE_NAME;
						break;
					case 4:
						lex->type = LexemType::OPERATION_BINARY;
						break;
					case 5:
						lex->type = LexemType::OPERATOR_DELIMITER;
						break;
					}

					lexems.add(lex);
					index = lex->last_position-1;
					break;
				}
			}
			index++;
		}
	}

	for (int i = 0; i < lexems.count(); i++)
	{
		Lexem* lex = (Lexem*)lexems.get(i);
		std::cout << lex->starting_position << " " << lex->last_position << " " << lex->type << " ";
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
		case LexemType::OPERATION_BINARY:
			std::cout << "Operator";
			break;
		case LexemType::OPERATOR_DELIMITER:
			std::cout << "Delimiter (;)";
			break;
		}
		std::cout << std::endl;
	}


	return 0;
}

