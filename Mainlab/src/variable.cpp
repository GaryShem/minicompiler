#include "../include/variable.h"

static Heap heap;

Value::Value(VARIABLE_TYPE type)
{
	this->type = type;
}

Value::~Value()
{
}

Variable_List::Variable_List(Variable_List* parent_list, int list_number) : List(sizeof(Variable_Record)), _children_list(sizeof(Variable_List))
{
	_parent_list = parent_list;
	_list_number = list_number;
}

Variable_List::~Variable_List()
{
}

Variable_Record* Variable_List::get_variable(char* var_name)
{

	Variable_List* current = this;
	while (current != NULL)
	{
		int limit = current->count();
		for (int i = 0; i < limit; i++)
		{
			Variable_Record* result = (Variable_Record*)current->get(i);
			if (strcmp(var_name, result->name) == 0)
			{
				return result;
			}
		}
		current = current->_parent_list;
	}
	return NULL;
}
