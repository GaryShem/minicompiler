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
	Segment* current;
	Segment* next;
	current = first;
	while (current)
	{
		next = current->next;
		delete_segment(current);
		current = next;
	}
}

void* List::get(int pos)
{
	void* ourEl;
	if (count() < pos || pos < 0)
	{
		_error = true;
		return 0;
	}


	if (pos + first_index < element_count) // если в первом сегменте
		ourEl = (void*)((size_t)(first->data) + (first_index + pos)*element_size);
	else
	{ // если не в первом сегменте 
		struct Segment* ourSegm = first->next;
		pos = pos - (element_count - first_index);
		// тут ищем индекс элемента в текущем сегменте ourSegm
		while (pos >= element_count)
		{
			ourSegm = ourSegm->next;
			pos -= element_count;
		}
		ourEl = (void*)((size_t)(ourSegm->data) + pos*element_size);
	}
	return ourEl;
}

void List::add(void* data)
{
	_error = false;
	if (first == 0 ||
		(last_index % element_count == 0 && last->next == 0))
	{ // если сегментов нет, или новый элемент не помещается в последний сегмент
		new_segment(); // new_segment() переприсваивает last
		memmove(last->data, data, element_size);
	}
	else
	{ // если новый элемент помещается в существующий сегмент last
		int segment_index = last_index % element_count;
		memmove((void*)((size_t)last->data + segment_index*element_size), data, element_size);
	}
	last_index++;
}

void List::take_first(void* store)
{
	// если элементов нет, то дадим ошибку
	if (count() < 1)
	{
		_error = true;
		return;
	}
	_error = false;
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
	// если элементов нет, то дадим ошибку
	if (count() < 1)
	{
		_error = true;
		return;
	}
	_error = false;
	// сначала выясним индекс элемента только внутри последнего сегмента
	// так как last_index указывает на индекс элемента, следующего за последним, берём -1
	int segment_index = (last_index - 1) % element_count;
	// копируем элемент с этим индексом в store
	memcpy(store, (void*)((size_t)last->data + segment_index*element_size), element_size);
	// уменьшаем индекс последнего элемента
	last_index--;
	//int n = *((int*)store); // дебуг
	if (segment_index == 0)
	{ // если индекс за последним элементом стал в начале сегмента
	  // то этот сегмент пуст и его надо удалить
		// удаляем старый последний сегмент - delete_segment() сам перенастраивает ссылки
		delete_segment(last);
	}
}

void List::take(int pos, void* store)
{
	// на всякий случай рассмотрим случай, где pos указывает на первый или последний элемента
	// тогда вызываем соответствующий "быстрый" take_****
	if (pos == 0)
	{
		take_first(store);
		return;
	}
	else if (pos == count()-1)
	{
		take_last(store);
		return;
	}
	
	// сначала копируем нужный элемент в store
	// адрес элемента получаем с помощью this->get()
	memcpy(store, get(pos), element_size);

	// так как мы уговорились о том, что take будет медленным, не будем морочиться над оптимизацией
	// смещаем по одному элементу через цикл с помощью memcpy
	// это позволяет не морочить себе мозги заполненностью сегментов
	for (int i = pos + 1; i < count(); i++)
	{
		memcpy(get(i-1), get(i), element_size);
	}

	last_index--;

	// опять же - если последний сегмент оказался пуст, то удаляем его

	if (last_index % element_count == 0)
	{ // если индекс за последним элементом стал в начале сегмента
		// то этот сегмент пуст и его надо удалить
		// удаляем старый последний сегмент - delete_segment() сам перенастраивает ссылки
		delete_segment(last);
	}
}

void List::sort(bool dir, int (*method)(const void*, const void*))
{ // считаем, что dir == true при возрастающем порядке сортировки
	if (method == 0)
		return;
	int count = this->count();
	void* buffer = heap.get_mem(element_size); // буффер под элемент для перестановок
	for (int i = 0; i < count - 1; i++)
	{
		void* starting = get(i);
		void* max_or_min = starting;
		for (int j = i+1; j < count; j++)
		{
			// method должен быть функцией сравнения, принимающей указатели на два элемента
			// возвращает 1 если первый элемент больше второго, 0 если они равны, -1 если первый меньше второго
			void* current = get(j);
			//int n = *((int*)current); //дебуг
			int comparison_result = method(max_or_min, current);
			if (dir)
			{
				// так как dir == true для возрастающего порядка, для убывания достаточно
				// умножать результат на -1 - это будет равносильно перемене мест параметров в вызове method
				comparison_result *= -1;
			}
			// если максимальный элемент меньше текущего
			// (если минимальный элемент больше текущего)
			// --- В ЗАВИСИМОСТИ ОТ dir ---
			// то переприсваиваем указатель максимального (минимального) элемента
			if (comparison_result < 0)
			{
				max_or_min = current;
			}
		}
		// теперь мы нашли номер максимального (или минимального) элемента
		// если этот элемент - не "текущий", т.е. не i-ый, то меняем их местами
		if (starting != max_or_min)
		{
			memcpy(buffer, starting, element_size);
			memcpy(starting, max_or_min, element_size);
			memcpy(max_or_min, buffer, element_size);
		}
	}
}

int List::count()
{
	_error = false;
	return last_index - first_index;
}

void List::new_segment()
{
	// выделяем память под саму структуру сегмента (т.е. под три указателя: data, next, prev)
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
		new_segment->next = 0;
		last = new_segment;
	}
}

void List::delete_segment(Segment* seg)
{
	// если нулевая ссылка, то просто выходим - на всякий
	if (seg == 0)
		return;

	// убираем данный сегмент из цепочки (списка)
	// существует ли предыдущий?
	if (seg->prev == 0)
	{ 
		// если данный сегмент - первый, то просто смещаем указатель первого сегмента
		first = seg->next;
	}
	else
	{
		// иначе переносим ссылку next предыдущего сегмента на следующим за данным
		seg->prev->next = seg->next;
	}

	// существует ли следующий?
	if (seg->next == 0)
	{ 
		// если данный сегмент - последний, то переприсваиваем последний сегмент
		last = seg->prev;
	}
	else
	{
		// иначе переносим ссылку prev следующего сегмента на предыдущий перед данным
		seg->next->prev = seg->prev;
	}
	// и освобождаем память - сначала данные сегмента, потом саму структуру с указателями
	heap.free_mem(seg->data);
	heap.free_mem(seg);
}
