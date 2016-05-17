#include "../include/hash.h"
#include "../include/list.h"

#include <cstring>


static Heap heap;

Hash::Hash(int _n1, int _n2, int _n3, int _n4, int _n5)
{
	// ���� �������� ������ ��� ����� 0, �� ����������� ���
	// ����� ����������� 0
	n1 = _n1 > 1 ? _n1 : 1;
	n2 = _n2 > 1 ? _n2 : 1;
	n3 = _n3 > 1 ? _n3 : 1;
	n4 = _n4 > 1 ? _n4 : 1;
	n5 = _n5 > 1 ? _n5 : 1;
	table_count = n1*n2*n3*n4*n5;
	// ��������
	// ������ �������� ����� ������ ���������� � ����������, ������ ��� ����� ����� ���� ������ �����
	// ���������� ������ ������������� - 12, ������ ��� ��������� ��� ������ ������
	table = (List**)heap.get_mem(table_count*sizeof(List*));
}

Hash::~Hash()
{
	heap.free_mem(table);
}

List* Hash::find_list(char* key_word)
{
	int k1, k2, k3, k4, k5;
	// ������� ��� 5 ������
	k1 = key1(key_word);
	k2 = key2(key_word);
	k3 = key3(key_word);
	k4 = key4(key_word);
	k5 = key5(key_word);
	// ����������� �� � ������� ����� ������ �������
	int table_index = combine_keys(k1, k2, k3, k4, k5) % table_count;
	// ���������� �� �� ���������
	return table[table_index];
}

Diction_list::~Diction_list() // purge all Article data
{
	Article* current = (Article*)heap.get_mem(sizeof(Article));
	while (true)
	{
		// ���� �������� �� �������
		take_first(current);
		// ���� �� ������ ��������� ������� (�.�. ��� ���������), �� ������� �� �����
		if (error() == true)
		{
			break;
		}
		// ����������� ������, ������� ������ - ���� ��������� ������ ������ ���� ��� � �����
		heap.free_mem(current->word);
		heap.free_mem(current->description);
	}
	heap.free_mem(current);
}

void Diction_list::put(Article* article) // allocate memory for word & description
{
	Article* article_copy = (Article*)heap.get_mem(sizeof(Article));
	// ���� ������ ������������ strlen, �� ����� ������� ����� ����
	//int i = 0; while (article->word[i] != 0) { i++; }
	// ��������� ���� ��� �������������� ����
	size_t word_length = strlen(article->word) + 1;
	size_t description_length = strlen(article->description) + 1;

	
	// ������ ������� ������ � �������-�����, ������� �� ������� � �����_����
	article_copy->word = (char*)heap.get_mem(word_length);
	article_copy->description = (char*)heap.get_mem(description_length);

	// � ������ �������� ������, ������� ����������� � �������-���������, � �������-�����
	memcpy(article_copy->word, article->word, word_length);
	memcpy(article_copy->description, article->description, description_length);

	// ������� ����� � ����
	add(article_copy);

	// ��� ��� this->add �������� ��������� � ����������� (�� �� �������� ������)
	// �� ������ ���������� article_copy
	heap.free_mem(article_copy);
}

Article* Diction_list::find(char* word)
{
	for (int i = 0; i < count(); i++)
	{
		Article* current = (Article*)get(i);
		if (strcmp(word, current->word) == 0)
		{
			// ��������, ����� �� �� ������ ����� ��������
			// ������ ����� �� ������
			return current;
		}
	}
	// ���� �� ����� ������ �������, �� ���������� NULL (nullptr?)
	return NULL;
}

void Diction_list::del(char* word)
{
	for (int i = 0; i < count(); i++)
	{
		Article* current = (Article*)get(i);
		if (strcmp(word, current->word) == 0)
		{
			Article buffer;
			// ����� �� ���� ��������� � ������� take
			// ��� �� ����� ������� - take ��� ��������� ������ � ��������� ��������
			{
				take(i, &buffer);
			}
			heap.free_mem(buffer.word);
			heap.free_mem(buffer.description);
			return;
		}
	}
}

Diction::Diction() : Hash(33, 33, 0, 0, 0)
{
	// ����� ���� ������ ������ ���������� ������
	for (int i = 0; i < get_table_count(); i++)
	{
		table[i] = new Diction_list();
	}
}
Diction::~Diction()
{
	// ����� ���� ������� ��� ��������� �������
	for (int i = 0; i < get_table_count(); i++)
	{
		delete(table[i]);
	}
}

Article* Diction::find(char* word)
{
	// ������� �� ���� ����������, � ����� ������ ����� (��������) ����� ���� ������
	Diction_list* list = (Diction_list*)find_list(word);
	// ����� � ���� ����� ���� �
	return list->find(word);
}

Article* Diction::auto_create(char* word)
{
	// ������� �� ���� ����������, � ����� ������ ���� ������ ���� �������� ����� ������
	Diction_list* list = (Diction_list*)find_list(word);
	
	// ������ ��� �� �������, � ���������� � ���� ����� ����� � �������� � ����
	Article* article = (Article*)heap.get_mem(sizeof(Article));

	// ������ � �������� ��������
	char generic_description[] = "Generic description";

	// ��������� ����� � ��������� �������� � �������-�����
	// ����� � ��� �������� � ������ put ������ Diction_list'�
	article->word = word;
	article->description = generic_description;

	// ��������� ������� � ������
	// ���������� put (� �� add), ������ ��� �� ������ ������ ����� ������� - ��� ��� ����� �� � �����
	list->put(article);
	// ��� ��� ���������� �������� � ������ �������� ��� (�� ����������� ���� ��������� �� word � description)
	// �� ����� ������� ��������� �������
	heap.free_mem(article);

	// ��� ��� � ���� ���������� ����� ���������, ��� ������� � ����� - ������������
	// � �������� ������������ ����� �������� ����� put �� Diction_list � add �� List, ����� ��� ���������� ������ �� ����������� �������
	//return list->find(word);

	// ������������ ������� - ���� ��������� ������� �� ��� �������
	// ��� ��� ���������� � 0, ���� count()-1
	// ��������, ��� �����
	return (Article*)list->get(list->count() - 1);
}

int Diction::key1(char* key_word) {
	// �������� � ��������� �� null ��� key_word
	int f = key_word[0];
	return f > 0 ? f%get_n1() : 0;
}
int Diction::key2(char* key_word)
{
	// ���� � ��� ����� ������ ������ - ��������
	if (key_word[0] == 0)
		return 0;

	int f = key_word[1];
	return f > 0 ? f%get_n2() : 0;
}