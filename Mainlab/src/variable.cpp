#include "../include/variable.h"

static Heap heap;

Value::Value(VARIABLE_TYPE type)
{
	this->type = type;
	switch (type)
	{
	case INT:
		value = heap.get_mem(sizeof(int));
		break;
	case DOUBLE:
		value = heap.get_mem(sizeof(double));
		break;
	case CHAR:
		value = heap.get_mem(sizeof(char));
		break;
	default:
		throw 654647;
	}
}

Variable_List::Variable_List() : List(sizeof(Variable_Record))
{

}

Variable_Record* Variable_List::get_variable(char* var_name)
{
	int limit = count();
	for (int i = 0; i < limit; i++)
	{
		Variable_Record* result = (Variable_Record*)get(i);
		if (strcmp(var_name, result->name) == 0)
		{
			return result;
		}
	}
	return NULL;
}