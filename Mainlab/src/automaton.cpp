#include "../include/automaton.h"
#include "../include/heap.h"
#include <fstream>
#include <iostream>

static Heap heap;

automaton::automaton(char* filename, char wildcard) : triads_list(sizeof(Triad))
{
	this->filename = filename;
	this->wildcard = wildcard;
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

	// ��� �� ����� �� ������������� �����
	// � ��������� ����� ����� ����� ����� ������� � ��������� ���������
	while (word[letter_index+1] != 0)
	{
		// ������ ���������� � ����� ������
		buffer_triad->current_state = current_state;
		buffer_triad->symbol = word[letter_index];
		buffer_triad->next_state = max_state;

		// ��������� ����� ������ � ����
		triads_list.add(buffer_triad);

		// ��������� ������ ��������� � ������ �����
		current_state = max_state;
		max_state++;
		letter_index++;
	}
	// ��������� ������� � "���������" ���������
	buffer_triad->current_state = current_state;
	buffer_triad->symbol = word[letter_index];
	// ��������� -1 ��������, ��� ��������� ������ ������ ���� ������������ - � �����, �� �������� ������ �����
	// ��������, charrrr �� ����� ��������, ������ ��� ����� ������� R ����� �� ������, � ��� ���� �����
	buffer_triad->next_state = -1;
	triads_list.add(buffer_triad);

	// ����������� ������ ��������� ������
	heap.free_mem(buffer_triad);

	// ���������� ����� max_state
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
	// ������ ���� �� �������, �.�. �� ������ �����
	while (file.getline(word, word_length))
	{
		// � ������ �� ������ � ��������� 0
		current_state = 0;
		// � �������� � ������ �����
		letter_index = 0;
		char alphabet[] = "qwertyuioopasdfghjklzxcvbnm";
		while (strchr(alphabet, word[letter_index])) // ���� ������ �� ��������
		{
			current_triad = find_triad(word[letter_index], current_state);
			if (current_triad == NULL)
			{ 
				// ���� �� �� ������ ����� ������, ������� ������������� �������� ���������,
				// �� ������ �� ������� ���������� �����, �.�. ��� ��� �� ����� ����������
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
}

void automaton::read_triads(char delimiter)
{
	std::ifstream file(filename);
	int word_length;
	word_length = 100;
	char* triad_line = (char*)heap.get_mem(word_length);
	char* buffer = (char*)heap.get_mem(word_length);
	int current_state;
	int next_state;
	int letter_index;
	char symbol;
	bool is_state_negative;
	Triad* current_triad;
	char* numbers = "0123456789";

	// ������ ���� �� �������, �.�. �� ������ �����
	while (file.getline(triad_line, word_length))
	{
		// � �������� � ������ �����
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
		char alphabet[] = "qwertyuioopasdfghjklzxcvbnm";
		int buffer_index = 0;
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

		current_triad = new Triad();
		current_triad->current_state = current_state;
		current_triad->symbol = symbol;
		current_triad->next_state = next_state;
		triads_list.add(current_triad);
	}


	file.close();
	heap.free_mem(triad_line);
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
