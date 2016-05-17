#include "../include/automaton.h"
#include "../include/heap.h"
#include <fstream>
#include <iostream>

#define STRING_LENGTH 1000

static Heap heap;

automaton::automaton(char* filename, char wildcard) : triads_list(sizeof(Triad))
{
	this->filename = filename;
	this->wildcard = wildcard;
	alphabet = (char*)heap.get_mem(STRING_LENGTH);
	wildcard_match_string = (char*)heap.get_mem(STRING_LENGTH);
	read_triads();
}

automaton::~automaton()
{
	heap.free_mem(alphabet);
	heap.free_mem(wildcard_match_string);
}

Triad* automaton::find_triad(int current_state, char letter)
{
	int i;
	int limit = triads_list.count();
	for (i = 0; i < limit; i++)
	{
		Triad* result = (Triad*)triads_list.get(i);
		if (result->current_state == current_state && result->symbol == letter)
			return result;
	}
	return NULL;
}

/*int automaton::create_new_triads(char* word, int letter_index, int current_state, int max_state)
{
	Triad* buffer_triad = (Triad*)heap.get_mem(sizeof(Triad));

	// идём по циклу до ПРЕДпоследней буквы
	// у последней буквы после цикла будет переход в финальное состояние
	while (word[letter_index+1] != 0)
	{
		// вводим информацию в новую триаду
		buffer_triad->current_state = current_state;
		buffer_triad->symbol = word[letter_index];
		buffer_triad->next_state = max_state;

		// добавляем новую триаду в лист
		triads_list.add(buffer_triad);

		// обновляем номера состояний и индекс буквы
		current_state = max_state;
		max_state++;
		letter_index++;
	}
	// добавляем переход в "финальное" состояние
	buffer_triad->current_state = current_state;
	buffer_triad->symbol = word[letter_index];
	// состояние -1 означает, что следующий символ должен быть разделителем - в общем, не являться частью слова
	// например, charrrr не будет ключевым, потому что после первого R будет не пробел, а ещё одна буква
	buffer_triad->next_state = -1;
	triads_list.add(buffer_triad);

	// освобождаем память буфферной триады
	heap.free_mem(buffer_triad);

	// возвращаем новый max_state
	return max_state;
}*/

/*void automaton::generate_triads()
{
	std::ifstream file(filename);
	int word_length;
	word_length = 100;
	char* word = (char*)heap.get_mem(word_length);
	int current_state;
	int max_state;
	int letter_index;
	Triad* current_triad;

	max_state = 1;
	// читаем файл по строкам, т.е. по одному слову
	while (file.getline(word, word_length))
	{
		// в начале мы всегда в состоянии 0
		current_state = 0;
		// и начинаем с первой буквы
		letter_index = 0;
		char alphabet[] = "qwertyuioopasdfghjklzxcvbnm";
		while (strchr(alphabet, word[letter_index])) // пока строка не кончится
		{
			current_triad = find_triad(current_state, word[letter_index]);
			if (current_triad == NULL || current_triad->next_state == -1)
			{ 
				// если мы не смогли найти триаду, которая соответствует текущему состоянию,
				// то просто по порядку генерируем новые, т.к. там уже не будет повторений
				max_state = create_new_triads(word, letter_index, current_state, max_state);
				break;
			}
			else
			{
				current_state = current_triad->next_state;
			}
			letter_index++;
		}
	}


	file.close();
	heap.free_mem(word);
}*/

void automaton::read_triads(char delimiter)
{
	std::ifstream file(filename);
//	int word_length;
//	word_length = 1000;
	char* triad_line = (char*)heap.get_mem(STRING_LENGTH);
	int current_state;
	int next_state;
	int letter_index;
	char symbol;
	bool is_state_negative;
	Triad* current_triad = (Triad*)heap.get_mem(sizeof(Triad));
	char* numbers = "0123456789";
	file.getline(alphabet, STRING_LENGTH);
	file.getline(wildcard_match_string, STRING_LENGTH);
	// читаем файл по строкам, т.е. по одному слову
	while (file.getline(triad_line, STRING_LENGTH))
	{
		// и начинаем с первой буквы
		letter_index = 0;
		if (triad_line[letter_index] == '-')
		{
			is_state_negative = true;
			letter_index++;
		}
		else
		{
			is_state_negative = false;
		}
		current_state = 0;
		next_state = 0;
		while (strchr(numbers, triad_line[letter_index]))
		{
			current_state = current_state * 10 + (triad_line[letter_index] - '0');
			letter_index++;
		}
		if (is_state_negative)
		{
			current_state *= -1;
		}
		while (triad_line[letter_index] == delimiter)
		{
			letter_index++;
		}
		symbol = triad_line[letter_index];
		letter_index++;

		while (triad_line[letter_index] == delimiter)
		{
			letter_index++;
		}

		if (triad_line[letter_index] == '-')
		{
			is_state_negative = true;
			letter_index++;
		}
		else
		{
			is_state_negative = false;
		}

		while (triad_line[letter_index] != 0 && strchr(numbers, triad_line[letter_index]))
		{
			next_state = next_state * 10 + (triad_line[letter_index] - '0');
			letter_index++;
		}
		if (is_state_negative)
		{
			next_state *= -1;
		}

		current_triad->current_state = current_state;
		current_triad->symbol = symbol;
		current_triad->next_state = next_state;
		triads_list.add(current_triad);
		
	}

	file.close();
	heap.free_mem(current_triad);
	heap.free_mem(triad_line);
}

void automaton::print_triads()
{
	int i;
	int limit = triads_list.count();
	std::cout << "alphabet" << std::endl;
	std::cout << alphabet << std::endl;
	std::cout << "wildcard string" << std::endl;
	std::cout << wildcard_match_string << std::endl;
	std::cout << "wildcard" << std::endl;
	std::cout << wildcard << std::endl;
	for (i = 0; i < limit; i++)
	{
		Triad* current = (Triad*)triads_list.get(i);
		std::cout << current->current_state << " " << current->symbol << " " << current->next_state << std::endl;
	}
}

Lexem* automaton::check(char* str, int index, int current_state)
{
	int triad_count = triads_list.count();
	Lexem* lex = NULL;
	if (current_state >= 10000 && (str[index] == 0 || strchr(alphabet, str[index]) == NULL))
	{
		lex = (Lexem*)heap.get_mem(sizeof(Lexem));
		lex->starting_position = index;
		lex->code = current_state;
		lex->last_position = index;
		return lex;
	}
	if (current_state < 10000 && str[index] == 0)
	{
		return NULL;
	}
	int triad_index = 0;
	Triad* current_triad;

	while (triad_index < triad_count)
	{
		current_triad = (Triad*)triads_list.get(triad_index);
		// если же текущее состояние - то, которое мы искали, то начинаем по очереди проверять и вызывать функцию рекурсивно для следующих состояний
		if (current_triad->current_state > current_state)
		{
			return NULL;
		}
		if (current_triad->current_state == current_state && (current_triad->symbol == wildcard || current_triad->symbol == str[index]))
		{
			if (current_triad->symbol == wildcard && strchr(wildcard_match_string, str[index]))
			{
				lex = check(str, index + 1, current_triad->next_state);
			}
			else if (current_triad->symbol == str[index])
			{
				lex = check(str, index + 1, current_triad->next_state);
			}
			if (lex != NULL)
			{
				lex->starting_position = index;
				return lex;
			}
		}
		triad_index++;
	}

	return NULL;
}

