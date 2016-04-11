//#include "heap.h"
#include "stdlib.h"
#include <string.h>
#include "../include/heap.h"

void* Heap::get_mem(int size)
{
	size_t memory_to_allocate = size/sizeof(int);//������� 8-�������� ������ �� ������� ��� ����������� �������
	if (size%sizeof(int))
		memory_to_allocate++;
	memory_to_allocate *= sizeof(int); //������������� � �����
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
		// �� ������ ����� �� �����, ��� � ��� ���� ����� ��� ������� ��� ���� ����� ����������
		//if (activeSegment->descriptor[activeSegment->descriptor_count].size >= region_count)
		if (serviceRecord->size >= memory_to_allocate && active_segment->descriptor_count < SEGMENTCOUNT)
		{ // ���� ������� � �����
			serviceRecord->size -= memory_to_allocate;
			serviceRecord->offset += memory_to_allocate;

			memmove(serviceRecord + sizeof(Segment_def), // �������� �� ���� ����������� ������
				serviceRecord,
				sizeof(Segment_def));

			serviceRecord->size = memory_to_allocate;
			serviceRecord->used = true;

			active_segment->descriptor_count++;

			return (void*)(segment_start + serviceRecord->offset);
		}
		else // ���� ���� �� ������ � �����
		{
			// �������� �� ������� ������������ � ���� �������� �������, ���������� �� �������
			for (int i = 0; i < active_segment->descriptor_count; i++)
			{
				// ������� �������� ��������� �����
				if (active_segment->descriptor[i].used)
				{
					continue;
				}
				// ������ �� ����� �����, ��� ������� ����� �� ������������
				// ������ �������� ������ �����
				if (active_segment->descriptor[i].size < memory_to_allocate)
				{
					continue;
				}
				// ������ �� �����, ��� � ��� ���� ��������� ����� ������������ �������
				// ����� � ���� ����������
				if (active_segment->descriptor_count == SEGMENTCOUNT || active_segment->descriptor[i].size == memory_to_allocate) 
				{ // ���� ��� ����� ��� ����� ����������� � �������, �� ����� ���� ����� ��� ����������� �� �������
					active_segment->descriptor[i].used = true;
					// ���������� ���������� ����� ����� ����� (����� �������� + �������� ������������ ������ ��������)
					return (void*)(segment_start + active_segment->descriptor[i].offset);
				}
				else
				{
					// ���� ����� ����, �� ����� ��� ���������� �� ��� �����
					int move_count = active_segment->descriptor_count - i + 1;
					size_t move_size = move_count * sizeof(Segment_def);
					memmove(&(active_segment->descriptor[i+1]), &(active_segment->descriptor[i]), move_size);

					// ����������� ����������, ���������� �������� �����
					active_segment->descriptor[i].size = memory_to_allocate;
					active_segment->descriptor[i].used = true;

					// ����������� ����������, ���������� ����������� ���������� �����
					active_segment->descriptor[i+1].size -= memory_to_allocate;
					active_segment->descriptor[i+1].offset += memory_to_allocate;

					// ���������� ����� ������ �������� �����
					return (void*)(segment_start + active_segment->descriptor[i].offset);
				}
				// TODO:
			}
		}
	}
	// �������� ������ ��������, ���������� � ����
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
