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

				// ������ ������, ������ ����� � ������ ����� �� ������� ������� (��������, ClaSS � CheSS)
				// �� ��� ����������� �� ����� �������� ���������� ��������� � ��������� ������ ��������,
				// ������� ����� ������� ����������� �� �������, ��� ������ ����� "��� ����� ����"
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
