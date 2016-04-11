//#include "heap.h"
#include "stdlib.h"
#include <string.h>
#include "../include/heap.h"

void* Heap::get_mem(int size)
{
	size_t memory_to_allocate = size/sizeof(int);//сколько 8-байтовых кусков мы выделим для запрошенной области
	if (size%sizeof(int))
		memory_to_allocate++;
	memory_to_allocate *= sizeof(int); //преобразовали в байты
	Segment* activeSegment = current;
	while (activeSegment)
	{
		Segment_def* serviceRecord = &(activeSegment->descriptor[activeSegment->descriptor_count]);


		if (activeSegment->descriptor_count >= SEGMENTCOUNT)
		{
			activeSegment = activeSegment->prev;
			continue;
		}
		// на данном этапе мы знаем, что у нас есть место как минимум под один новый дескриптор
		//if (activeSegment->descriptor[activeSegment->descriptor_count].size >= region_count)
		if (serviceRecord->size >= memory_to_allocate && activeSegment->descriptor_count < SEGMENTCOUNT)
		{ // если влезает в конец
			serviceRecord->size -= memory_to_allocate;
			serviceRecord->offset += memory_to_allocate;

			memmove(serviceRecord + sizeof(Segment_def), // сместить на один собственный размер
				serviceRecord,
				sizeof(Segment_def));

			serviceRecord->size = memory_to_allocate;
			serviceRecord->used = true;

			activeSegment->descriptor_count++;

			return (void*)((size_t)activeSegment->data + serviceRecord->offset);
		}
		else // таки если не влезло в конец
		{
			for (int i = 0; i < activeSegment->descriptor_count; i++)
			{
				// сначала проверим занятость куска
				if (activeSegment->descriptor[i].used)
				{
					continue;
				}
				// теперь мы точно знаем, что текущий кусок не используется
				// теперь проверим размер куска
				if (activeSegment->descriptor[i].size < memory_to_allocate)
				{
					continue;
				}
				// теперь мы знаем, что у нас есть свободный кусок достаточного размера
				// хотим в него воткнуться
				if (activeSegment->descriptor_count == SEGMENTCOUNT || activeSegment->descriptor[i].size == memory_to_allocate) 
				{ // если нет места под новые дескрипторы в массиве, то отдаём весь кусок вне зависимости от размера
					activeSegment->descriptor[i].used = true;
					// возвращаем абсолютный адрес этого куска (адрес сегмента + смещение относительно начала сегмента)
					return (void*)((size_t)activeSegment->data + activeSegment->descriptor[i].offset);
				}
				else
				{
					// если место есть, то делим наш дескриптор на две части
				}
				// TODO:
			}
		}
	}
	// создание нового сегмента, добавление в него
}

void Heap::free_mem(void*)
{
	throw 1;
}

int Heap::make_segment()
{
	struct Segment* s = new Segment;
	s->data = malloc(segment_size);
	if (s == NULL || s->data == NULL)
		return -1;
	s->prev = current;
	current = s;
	s->descriptor_count = 0;
	s->descriptor[0].offset = 0;
	s->descriptor[0].used = false;
	s->descriptor[0].size = SEGMENTSIZE;
	return 0;
}

void Heap::delete_segments()
{
	while (current)
	{
		Segment* next = current->prev;
		free(current->data);
		free(current);
		current = next;
	}
	throw 1;
}
