#include "../include/hash.h"
#include "../include/list.h"

#include <cstring>


static Heap heap;

Hash::Hash(int _n1, int _n2, int _n3, int _n4, int _n5)
{
	// если параметр больше или равен 0, то присваиваем его
	// иначе присваиваем 0
	n1 = _n1 > 1 ? _n1 : 1;
	n2 = _n2 > 1 ? _n2 : 1;
	n3 = _n3 > 1 ? _n3 : 1;
	n4 = _n4 > 1 ? _n4 : 1;
	n5 = _n5 > 1 ? _n5 : 1;
	table_count = n1*n2*n3*n4*n5;
	// СПРОСИТЬ
	// сейчас создание самих листов перенесено в наследника, потому что листы могут быть разных типов
	// количество таблиц фиксированное - 12, потому что непонятно что именно делать
	table = (List**)heap.get_mem(table_count*sizeof(List*));
}

Hash::~Hash()
{
	heap.free_mem(table);
}

List* Hash::find_list(char* key_word)
{
	int k1, k2, k3, k4, k5;
	// находим все 5 ключей
	k1 = key1(key_word);
	k2 = key2(key_word);
	k3 = key3(key_word);
	k4 = key4(key_word);
	k5 = key5(key_word);
	// комбинируем их и находим номер нужной таблицы
	int table_index = combine_keys(k1, k2, k3, k4, k5) % table_count;
	// возвращаем на неё указатель
	return table[table_index];
}

Diction_list::~Diction_list() // purge all Article data
{
	Article* current = (Article*)heap.get_mem(sizeof(Article));
	while (true)
	{
		// берём элементы по очереди
		take_first(current);
		// если не смогли прочитать элемент (т.е. они кончились), то выходим из цикла
		if (error() == true)
		{
			break;
		}
		// освобождаем память, занятую полями - саму структуру буфера чистим один раз в конце
		heap.free_mem(current->word);
		heap.free_mem(current->description);
	}
	heap.free_mem(current);
}

void Diction_list::put(Article* article) // allocate memory for word & description
{
	Article* article_copy = (Article*)heap.get_mem(sizeof(Article));
	// если нельзя использовать strlen, то будем считать через цикл
	//int i = 0; while (article->word[i] != 0) { i++; }
	// добавляем один для терминирующего нуля
	size_t word_length = strlen(article->word) + 1;
	size_t description_length = strlen(article->description) + 1;

	
	// теперь выделим память в артикле-копии, которую мы запишем в дикшн_лист
	article_copy->word = (char*)heap.get_mem(word_length);
	article_copy->description = (char*)heap.get_mem(description_length);

	// и теперь копируем строки, которые содержались в артикле-параметре, в артикль-копию
	memcpy(article_copy->word, article->word, word_length);
	memcpy(article_copy->description, article->description, description_length);

	// добавим копию в лист
	add(article_copy);

	// так как this->add копирует структуру с указателями (но не копирует строки)
	// мы можешь освободить article_copy
	heap.free_mem(article_copy);
}

Article* Diction_list::find(char* word)
{
	for (int i = 0; i < count(); i++)
	{
		Article* current = (Article*)get(i);
		if (strcmp(word, current->word) == 0)
		{
			// СПРОСИТЬ, хотим ли мы делать копию элемента
			// сейчас копию не делаем
			return current;
		}
	}
	// если не нашли нужный элемент, то возвращаем NULL (nullptr?)
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
			// иначе же берём медленный и грузный take
			// уже не такой грузный - take сам проверяет первый и последний элементы
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
	// через цикл создаём нужное количество таблиц
	for (int i = 0; i < get_table_count(); i++)
	{
		table[i] = new Diction_list();
	}
}
Diction::~Diction()
{
	// через цикл удаляем все созданные таблицы
	for (int i = 0; i < get_table_count(); i++)
	{
		delete(table[i]);
	}
}

Article* Diction::find(char* word)
{
	// сначала по хэшу определяем, в каком именно листе (возможно) лежит наша запись
	Diction_list* list = (Diction_list*)find_list(word);
	// потом в этом листе ищем её
	return list->find(word);
}

Article* Diction::auto_create(char* word)
{
	// сначала по хэшу определяем, в какой именно лист должна быть записана новая запись
	Diction_list* list = (Diction_list*)find_list(word);
	
	// создаём для неё артикль, и запихиваем в него копию слова и описание к нему
	Article* article = (Article*)heap.get_mem(sizeof(Article));

	// строка с шаблоном описания
	char generic_description[] = "Generic description";

	// добавляем слово и шаблонное описание в артикль-буфер
	// копия с них снимется в методе put нашего Diction_list'а
	article->word = word;
	article->description = generic_description;

	// добавляем артикль в список
	// используем put (а не add), потому что он делает полную копию артикля - как раз этого мы и хотим
	list->put(article);
	// так как добавление элемента в список копирует его (мы скопировали сами указатели на word и description)
	// мы можем удалить структуру артикля
	heap.free_mem(article);

	// так как в лист добавилась копия структуры, нам придётся её найти - грустьпичаль
	// в качестве альтернативы можно поменять метод put от Diction_list и add от List, чтобы оба возвращали ссылку на добавленный элемент
	//return list->find(word);

	// альтернатива нашлась - берём последний элемент по его индексу
	// так как индексация с 0, берём count()-1
	// наверное, так лучше
	return (Article*)list->get(list->count() - 1);
}

int Diction::key1(char* key_word) {
	// спросить о проверках на null для key_word
	int f = key_word[0];
	return f > 0 ? f%get_n1() : 0;
}
int Diction::key2(char* key_word)
{
	// если у нас вдруг пустая строка - малоличо
	if (key_word[0] == 0)
		return 0;

	int f = key_word[1];
	return f > 0 ? f%get_n2() : 0;
}