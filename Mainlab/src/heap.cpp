#include "stdlib.h"
#include <string.h>
#include "../include/heap.h"

void* Heap::get_mem(int size)
{
	size_t memory_to_allocate = size/cluster_size; //сколько кластеров мы выделим для запрошенной области
	if (size%cluster_size)
		memory_to_allocate++;
	memory_to_allocate *= cluster_size; //преобразовали в байты
	Segment* active_segment = current;
	Segment_def* service_record;
	size_t segment_start;
	for (active_segment = current; active_segment; active_segment = active_segment->prev)
	{
		service_record = &(active_segment->descriptor[active_segment->descriptor_count]);
		segment_start = (size_t)active_segment->data;

		// на данном этапе мы знаем, что у нас есть место как минимум под один новый дескриптор
		//if (activeSegment->descriptor[activeSegment->descriptor_count].size >= region_count)
		if (service_record->size >= memory_to_allocate && active_segment->descriptor_count < SEGMENTCOUNT)
		{ // если влезает в конец
			return add_to_segment_end(memory_to_allocate, active_segment);
			service_record->size -= memory_to_allocate;
			service_record->offset += memory_to_allocate;
			Segment_def* new_service_record = service_record + 1;

			memmove(service_record + 1, // сместить на один собственный размер (+1 ВНЕЗАПНО смещает на размер записи)
				service_record,
				sizeof(Segment_def));

			// так как service_record всё ещё указывает на нулевую запись, мы просто меняем то, что осталось по этому адресу
			service_record->size = memory_to_allocate;
			service_record->used = true;
			// у новой сервисной записи тоже меняем смещение и размер
			new_service_record->size -= memory_to_allocate;
			new_service_record->offset += memory_to_allocate;

			active_segment->descriptor_count++;

			return (void*)(segment_start + service_record->offset);
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
					// если размер куска равен запрашиваемому, то ясное дело тоже отдаём весь
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

					// увеличиваем количество задействованных дескрипторов
					active_segment->descriptor_count++;

					// возвращаем адрес нового занятого куска
					return (void*)(segment_start + active_segment->descriptor[i].offset);
				}
			}
		}
	}
	// создание нового сегмента, добавление в него
	make_segment();

	// по-хорошему, это стоит вынести в функцию, но ограничения на изменения header-a - СПРОСИТЬ
	return add_to_segment_end(memory_to_allocate, current);

	// переприсваиваем указатели
	service_record = &current->descriptor[0];
	active_segment = current;
	segment_start = (size_t)active_segment->data;

	// этот кусок можно вынести в функцию
	Segment_def* new_service_record = service_record + 1;
	size_t bytes_to_move = sizeof(Segment_def);
	memmove(new_service_record, // сместить на один собственный размер
		service_record,
		bytes_to_move);

	// так как service_record всё ещё указывает на нулевую запись, мы просто меняем то, что осталось по этому адресу
	service_record->size = memory_to_allocate;
	service_record->used = true;
	// у новой сервисной записи тоже меняем смещение и размер
	new_service_record->size -= memory_to_allocate;
	new_service_record->offset += memory_to_allocate;

	active_segment->descriptor_count++;

	return (void*)(segment_start + service_record->offset);
}

void* Heap::add_to_segment_end(int size, Segment* segment)
{
	Segment_def* service_record = &segment->descriptor[segment->descriptor_count];
	size_t segment_start = (size_t)segment->data;

	// этот кусок можно вынести в функцию
	Segment_def* new_service_record = service_record + 1;
	size_t bytes_to_move = sizeof(Segment_def);
	memmove(new_service_record, // сместить на один собственный размер
		service_record,
		bytes_to_move);

	// так как service_record всё ещё указывает на нулевую запись, мы просто меняем то, что осталось по этому адресу
	service_record->size = size;
	service_record->used = true;
	// у новой сервисной записи тоже меняем смещение и размер
	new_service_record->size -= size;
	new_service_record->offset += size;

	segment->descriptor_count++;

	return (void*)(segment_start + service_record->offset);
}

void Heap::free_mem(void* address_void)
{
	// счётчик сдвига таблицы дескрипторов
	int shift_count = 0;
	size_t address = (size_t)address_void;

	Segment* active_segment = current;
	size_t segment_start;

	// сначала ищем нужный сегмент
	while (true)
	{
		segment_start = (size_t)active_segment->data;
		if (address >= segment_start && address < segment_start + SEGMENTSIZE)
			break;
		active_segment = active_segment->prev;
	}
	
	// нашли нужный сегмент, теперь работаем внутри него с дескрипторами
	// сначала для удобства вынесем смещение относительно начала этого сегмента в отдельную переменную
	size_t offset = address - segment_start;

	int i = 0;
	while (offset < active_segment->descriptor[i].offset)
	{
		i++;
	}

	// если никто ничего не сломал, то сейчас offset и active_segment->descriptor[i].offset равны между собой
	// надо учитывать, что пользователь - дебил, поэтому проверяем равенство, а то всё может сломаться
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

	// теперь в самом верхнем из 1/2/3 свободных дескрипторов содержится общий размер свободного куска
	// осталось сместить нижнюю часть массива дескрипторов на количество записей, содержащееся в shift_count - счётчике сдвига

	// если shift_count == 0, то ничего не делаем и возвращаемся из функции
	if (shift_count == 0)
		return;
	
	// теперь мы знаем, что сдвиг точно будет

	int move_count;
	size_t move_size;
	if (is_next_free)
	{ // если следующий дескриптор свободен, то сдвиг будет начиная с индекса i+2
		move_count = active_segment->descriptor_count - i - 1;
		move_size = move_count * sizeof(Segment_def);
		memmove(&active_segment->descriptor[i + 2 - shift_count],
			&active_segment->descriptor[i + 2],
			move_size);
	}
	else // если же следующий дескриптор занят, то сдвиг будет начиная с индекса i+1
	{ // соответственно, сдвигаемых элементов будет на 1 больше, чем в предыдущем случае
		// в этом блоке shift_count будет равен 1
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
