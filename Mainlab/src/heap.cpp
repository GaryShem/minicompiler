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
	Segment_def* service_record;
	size_t segment_start;
	for (active_segment = current; active_segment; active_segment = active_segment->prev)
	{
		service_record = &(active_segment->descriptor[active_segment->descriptor_count]);
		segment_start = (size_t)active_segment->data;

		// �� ������ ����� �� �����, ��� � ��� ���� ����� ��� ������� ��� ���� ����� ����������
		//if (activeSegment->descriptor[activeSegment->descriptor_count].size >= region_count)
		if (service_record->size >= memory_to_allocate && active_segment->descriptor_count < SEGMENTCOUNT)
		{ // ���� ������� � �����
			service_record->size -= memory_to_allocate;
			service_record->offset += memory_to_allocate;
			Segment_def* new_service_record = service_record + 1;

			memmove(service_record + 1, // �������� �� ���� ����������� ������ (+1 �������� ������� �� ������ ������)
				service_record,
				sizeof(Segment_def));

			// ��� ��� service_record �� ��� ��������� �� ������� ������, �� ������ ������ ��, ��� �������� �� ����� ������
			service_record->size = memory_to_allocate;
			service_record->used = true;
			// � ����� ��������� ������ ���� ������ �������� � ������
			new_service_record->size -= memory_to_allocate;
			new_service_record->offset += memory_to_allocate;

			active_segment->descriptor_count++;

			return (void*)(segment_start + service_record->offset);
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
					// ���� ������ ����� ����� ��������������, �� ����� ���� ���� ����� ����
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

					// ����������� ���������� ��������������� ������������
					active_segment->descriptor_count++;

					// ���������� ����� ������ �������� �����
					return (void*)(segment_start + active_segment->descriptor[i].offset);
				}
			}
		}
	}
	// �������� ������ ��������, ���������� � ����
	make_segment();

	// ��-��������, ��� ����� ������� � �������, �� ����������� �� ��������� header-a - ��������

	// ��������������� ���������
	service_record = &current->descriptor[0];
	active_segment = current;
	segment_start = (size_t)active_segment->data;

	// ���� ����� ����� ������� � �������
	Segment_def* new_service_record = service_record + 1;
	size_t bytes_to_move = sizeof(Segment_def);
	memmove(new_service_record, // �������� �� ���� ����������� ������
		service_record,
		bytes_to_move);

	// ��� ��� service_record �� ��� ��������� �� ������� ������, �� ������ ������ ��, ��� �������� �� ����� ������
	service_record->size = memory_to_allocate;
	service_record->used = true;
	// � ����� ��������� ������ ���� ������ �������� � ������
	new_service_record->size -= memory_to_allocate;
	new_service_record->offset += memory_to_allocate;

	active_segment->descriptor_count++;

	return (void*)(segment_start + service_record->offset);
}

void Heap::free_mem(void* address_void)
{
	// ������� ������ ������� ������������
	int shift_count = 0;
	size_t address = (size_t)address_void;

	Segment* active_segment = current;
	size_t segment_start;

	// ������� ���� ������ �������
	while (true)
	{
		segment_start = (size_t)active_segment->data;
		if (address >= segment_start && address < segment_start + SEGMENTSIZE)
			break;
		active_segment = active_segment->prev;
	}
	
	// ����� ������ �������, ������ �������� ������ ���� � �������������
	// ������� ��� �������� ������� �������� ������������ ������ ����� �������� � ��������� ����������
	size_t offset = address - segment_start;

	int i = 0;
	while (offset < active_segment->descriptor[i].offset)
	{
		i++;
	}

	// ���� ����� ������ �� ������, �� ������ offset � active_segment->descriptor[i].offset ����� ����� �����
	// ���� ���������, ��� ������������ - �����, ������� ��������� ���������, � �� �� ����� ���������
	if (offset != active_segment->descriptor[i].offset)
		return;

	bool is_next_free = false;
	if (active_segment->descriptor[i+1].used == false)
	{
		is_next_free = true;
		shift_count++;
		active_segment->descriptor[i].size += active_segment->descriptor[i + 1].size;
	}
	if (i > 0 && active_segment->descriptor[i - 1].used == false)
	{
		shift_count++;
		active_segment->descriptor[i - 1].size += active_segment->descriptor[i].size;
	}

	// ������ � ����� ������� �� 1/2/3 ��������� ������������ ���������� ����� ������ ���������� �����
	// �������� �������� ������ ����� ������� ������������ �� ���������� �������, ������������ � shift_count - �������� ������

	// ���� shift_count == 0, �� ������ �� ������ � ������������ �� �������
	if (shift_count == 0)
		return;
	
	// ������ �� �����, ��� ����� ����� �����

	int move_count;
	size_t move_size;
	if (is_next_free)
	{ // ���� ��������� ���������� ��������, �� ����� ����� ������� � ������� i+2
		move_count = active_segment->descriptor_count - i - 1;
		move_size = move_count * sizeof(Segment_def);
		memmove(&active_segment->descriptor[i + 2 - shift_count],
			&active_segment->descriptor[i + 2],
			move_size);
	}
	else // ���� �� ��������� ���������� �����, �� ����� ����� ������� � ������� i+1
	{ // ��������������, ���������� ��������� ����� �� 1 ������, ��� � ���������� ������
		// � ���� ����� shift_count ����� ����� 1
		move_count = active_segment->descriptor_count - i;
		move_size = move_count * sizeof(Segment_def);
		memmove(&active_segment->descriptor[i + 1 - shift_count],
			&active_segment->descriptor[i + 1],
			move_size);
	}
	active_segment->descriptor_count -= shift_count;
}

int Heap::make_segment()
{
	struct Segment* s = new Segment;
	s->data = malloc(segment_size);
	if (s == NULL || s->data == NULL)
		throw "heap exception";
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
}
