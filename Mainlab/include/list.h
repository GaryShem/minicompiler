#ifndef LAB_LIST_H
#define LAB_LIST_H

#include "heap.h"

#define LISTSIZE 10

class List
{
public:
	List(int _element_size, int _element_count = LISTSIZE);
	~List();

	void*      get(int pos); // получает ссылку на элемент в позиции pos (элемент не удаляет)
	void       add(void* data); // добавляет элемент, на который ссылается data (при этом делает копию)

	// returns and deletes elements
	void       take_first(void* store); // берёт из листа первый элемент и удаляет из листа
	void       take_last(void* store); // берёт из листа последний элемент и удаляет из листа
	void       take(int pos, void* store); // берёт из листа элемент в позиции pos и удаляет из листа
	void       sort(bool dir = true, int(*method)(const void*, const void*) = 0); // сортирует лист где method - функция сравнения элементов листа

	int        count(); // возвращает количество элементов в листе
    bool       error() { return _error; } // true if error in last operation
private:
	struct Segment
	{
		void*    data;
		Segment* prev;
		Segment* next;
	};
	struct Segment*  first; // ссылка на первый сегмент списка
	struct Segment*  last; // ссылка на последний сегмент списка
	int              first_index; // индекс первого элемента (будет изменяться при take_first)
	int              last_index; // индекс последнего элемента (будет изменяться при take и take_last)

	int              element_size; // размер элемента в байтах
	int              element_count; // количество элементов в одном сегменте листа
    bool             _error; // ашыпка
	
	void new_segment(); // функция для создания нового сегмента (добавляет в конец)
	void delete_segment(Segment* seg); // функция удаления сегмента по ссылке
};

#endif