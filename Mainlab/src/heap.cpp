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
	Segment* active_segment = current;
	while (active_segment)
	{
		Segment_def* serviceRecord = &(active_segment->descriptor[active_segment->descriptor_count]);
		size_t segment_start = (size_t)active_segment->data;

		if (active_segment->descriptor_count >= SEGMENTCOUNT)
		{
			active_segment = active_segment->prev;
			continue;
		}
		// на данном этапе мы знаем, что у нас есть место как минимум под один новый дескриптор
		//if (activeSegment->descriptor[activeSegment->descriptor_count].size >= region_count)
		if (serviceRecord->size >= memory_to_allocate && active_segment->descriptor_count < SEGMENTCOUNT)
		{ // если влезает в конец
			serviceRecord->size -= memory_to_allocate;
			serviceRecord->offset += memory_to_allocate;

			memmove(serviceRecord + sizeof(Segment_def), // сместить на один собственный размер
				serviceRecord,
				sizeof(Segment_def));

			serviceRecord->size = memory_to_allocate;
			serviceRecord->used = true;

			active_segment->descriptor_count++;

			return (void*)(segment_start + serviceRecord->offset);
		}
		else // таки если не влезло в конец
		{
			// проходим по массиву дескрипторов и ищем свободну область, подходящую по размеру
			for (int i = 0; i < active_segment->descriptor_count; i++)
			{
				// сначала проверим занятость куска
				if (active_segment->descriptor[i].used)
				{
					continue;
				}
				// теперь мы точно знаем, что текущий кусок не используется
				// теперь проверим размер куска
				if (active_segment->descriptor[i].size < memory_to_allocate)
				{
					continue;
				}
				// теперь мы знаем, что у нас есть свободный кусок достаточного размера
				// хотим в него воткнуться
				if (active_segment->descriptor_count == SEGMENTCOUNT || active_segment->descriptor[i].size == memory_to_allocate) 
				{ // если нет места под новые дескрипторы в массиве, то отдаём весь кусок вне зависимости от размера
					active_segment->descriptor[i].used = true;
					// возвращаем абсолютный адрес этого куска (адрес сегмента + смещение относительно начала сегмента)
					return (void*)(segment_start + active_segment->descriptor[i].offset);
				}
				else
				{
					// если место есть, то делим наш дескриптор на две части
					int move_count = active_segment->descriptor_count - i + 1;
					size_t move_size = move_count * sizeof(Segment_def);
					memmove(&(active_segment->descriptor[i+1]), &(active_segment->descriptor[i]), move_size);

					// настраиваем дескриптор, отвечающий занятому куску
					active_segment->descriptor[i].size = memory_to_allocate;
					active_segment->descriptor[i].used = true;

					// настраиваем дескриптор, отвечающий оставшемуся свободному куску
					active_segment->descriptor[i+1].size -= memory_to_allocate;
					active_segment->descriptor[i+1].offset += memory_to_allocate;

					// возвращаем адрес нового занятого куска
					return (void*)(segment_start + active_segment->descriptor[i].offset);
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
