#include "../include/automaton.h"
#include "../include/heap.h"
#include <fstream>
#include <iostream>

static Heap heap;

automaton::automaton(char* filename) : triads_list(sizeof(Triad))
{
	this->filename = filename;
}

automaton::~automaton()
{
}

Triad* automaton::find_triad(char letter, int current_state)
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

int automaton::create_new_triads(char* word, int letter_index, int current_state, int max_state)
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
}

void automaton::generate_triads()
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
			current_triad = find_triad(word[letter_index], current_state);
			if (current_triad == NULL)
			{ 
				// если мы не смогли найти триаду, которая соответствует текущему состоянию,
				// то просто по порядку генерируем новые, т.к. там уже не будет повторений

				// вообще говоря, разные ветви в теории могли бы сойтись обратно (например, ClaSS и CheSS)
				// но это потребовало бы очень сильного усложнения алгоритма и множества лишних проверок,
				// поэтому после первого расхождения мы считаем, что каждое слово "идёт своим путём"
				max_state = create_new_triads(word, letter_index, current_state, max_state);
				break;
			}
			else
			{
				current_state = current_triad->current_state;
			}
			letter_index++;
		}
	}


	file.close();
	heap.free_mem(word);
}

void automaton::print_triads()
{
	int i;
	int limit = triads_list.count();
	for (i = 0; i < limit; i++)
	{
		Triad* current = (Triad*)triads_list.get(i);
		std::cout << current->current_state << " " << current->symbol << " " << current->next_state << std::endl;
	}
}
