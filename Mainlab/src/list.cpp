#include "../include/list.h"
#include <cstring>

static Heap heap;

List::List(int _element_size, int _element_count)
{
	first = 0;
	last = 0;
	first_index = last_index = 0;
	element_size = _element_size;
	element_count = _element_count;
	_error = false;
}

List::~List()
{

}

void* List::get(int pos)
{
	return nullptr;
}

void List::add(void* data)
{
	if (first == 0 ||
		(last_index % element_count == 0 && last->next == 0))
	{ // если сегментов нет, или новый 
		new_segment();
		memcpy(last->data, data, element_size);
	}
	else
	{ // если новый элемент помещается в существующий сегмент last
		int segment_index = last_index % element_count;
		memcpy((void*)((size_t)last->data + segment_index*element_size), data, element_size);
	}
	last_index++;
}

void List::take_first(void* store)
{
	// сначала копируем элемент в store
	memcpy(store, (void*)((size_t)first->data + first_index*element_size), element_size);
	// увеличиваем индекс первого элемента
	first_index++;

	if (first_index == element_count)
	{ // если первый элемент уполз за пределы первого сегмента
		Segment* new_first = first->next;
		// удаляем старый первый сегмент
		delete_segment(first);
		// настраиваем ссылку первого сегмента на новый первый
		first = new_first;
		// сдвигаем первый и последний индексы
		first_index -= element_count;
		last_index -= element_count;
	}
}

void List::take_last(void* store)
{

}

void List::take(int pos, void* store)
{
	
}

void List::sort(bool dir, int method)
{
}

int List::count()
{
	return last_index - first_index;
}

void List::new_segment()
{
	// выделяем память под саму структуру сегмента (т.е. под три указателя)
	Segment* new_segment = (Segment*)heap.get_mem(sizeof(Segment));
	// потом выделяем память под LISTSIZE элементов и присваиваем указатель на эту область в data
	new_segment->data = heap.get_mem(element_count*element_size);
	if (first == 0)
	{ // если это первый сегмент, то он же и последний
		first = last = new_segment;
		new_segment->next = new_segment->prev = 0;
		first_index = last_index = 0;
	}
	else
	{ // если не первый, то он новый последний
		new_segment->prev = last;
		last->next = new_segment;
		last = new_segment;
	}
}

void List::delete_segment(Segment* seg)
{
	// если нулевая ссылка, то просто выходим - на всякий
	if (seg == 0)
		return;

	// убираем данный сегмент из цепочки (списка)
	if (seg->prev == 0)
	{ // если данный сегмент - первый
		first = seg->next;
	}
	else
	{
		seg->prev->next = seg->next;
	}

	if (seg->next == 0)
	{ // если данный сегмент - последний, то переприсваиваем последний сегмент
		last = seg->prev;
	}
	else
	{
		seg->next->prev = seg->prev;
	}
	heap.free_mem(seg->data);
	heap.free_mem(seg);
}
