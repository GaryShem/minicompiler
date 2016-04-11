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
	{ // ���� ��������� ���, ��� ����� 
		new_segment();
		memcpy(last->data, data, element_size);
	}
	else
	{ // ���� ����� ������� ���������� � ������������ ������� last
		int segment_index = last_index % element_count;
		memcpy((void*)((size_t)last->data + segment_index*element_size), data, element_size);
	}
	last_index++;
}

void List::take_first(void* store)
{
	// ������� �������� ������� � store
	memcpy(store, (void*)((size_t)first->data + first_index*element_size), element_size);
	// ����������� ������ ������� ��������
	first_index++;

	if (first_index == element_count)
	{ // ���� ������ ������� ����� �� ������� ������� ��������
		Segment* new_first = first->next;
		// ������� ������ ������ �������
		delete_segment(first);
		// ����������� ������ ������� �������� �� ����� ������
		first = new_first;
		// �������� ������ � ��������� �������
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
	// �������� ������ ��� ���� ��������� �������� (�.�. ��� ��� ���������)
	Segment* new_segment = (Segment*)heap.get_mem(sizeof(Segment));
	// ����� �������� ������ ��� LISTSIZE ��������� � ����������� ��������� �� ��� ������� � data
	new_segment->data = heap.get_mem(element_count*element_size);
	if (first == 0)
	{ // ���� ��� ������ �������, �� �� �� � ���������
		first = last = new_segment;
		new_segment->next = new_segment->prev = 0;
		first_index = last_index = 0;
	}
	else
	{ // ���� �� ������, �� �� ����� ���������
		new_segment->prev = last;
		last->next = new_segment;
		last = new_segment;
	}
}

void List::delete_segment(Segment* seg)
{
	// ���� ������� ������, �� ������ ������� - �� ������
	if (seg == 0)
		return;

	// ������� ������ ������� �� ������� (������)
	if (seg->prev == 0)
	{ // ���� ������ ������� - ������
		first = seg->next;
	}
	else
	{
		seg->prev->next = seg->next;
	}

	if (seg->next == 0)
	{ // ���� ������ ������� - ���������, �� ��������������� ��������� �������
		last = seg->prev;
	}
	else
	{
		seg->next->prev = seg->prev;
	}
	heap.free_mem(seg->data);
	heap.free_mem(seg);
}
