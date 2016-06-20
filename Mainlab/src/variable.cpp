#include "../include/variable.h"
#include <stdexcept>

static Heap heap;

Value::Value(VARIABLE_TYPE type)
{
	this->type = type;
}

VARIABLE_TYPE Value::GetType()
{
	return type;
}

double Value::GetValue()
{
	switch (type)
	{
		case INT:
			return (int)value;
			break;
		case DOUBLE:
			return value;
			break;
		case BOOL:
			return value == 0 ? 0 : 1;
			break;
		default:
			throw std::runtime_error("Unknown variable type");
	}
}

VARIABLE_TYPE Variable_Record::GetType()
{
	return value->GetType();
}

double Variable_Record::GetValue()
{
	return value->GetValue();
}

void Variable_Record::SetValue(double new_value)
{
	value->SetValue(new_value);
}

void Value::SetValue(double new_value)
{
	switch (type)
	{
		case INT:
			value = (int)new_value;
			break;
		case DOUBLE:
			value = (double)new_value;
			break;
		case BOOL:
			value = new_value == 0 ? 0 : 1;
			break;
	}
}

Value::~Value()
{
}

Variable_List::Variable_List(Variable_List* parent_list, int list_number) : List(sizeof(Variable_Record)), _children_list(sizeof(Variable_List*))
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
