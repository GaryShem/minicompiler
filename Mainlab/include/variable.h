#ifndef VARIABLE_H
#define VARIABLE_H
#include "list.h"
#include <cstring>

enum VARIABLE_TYPE
{
	INT = 10010,
	DOUBLE,
	CHAR
};

class Value
{
public:
	Value(VARIABLE_TYPE type);
	VARIABLE_TYPE type;
	void* value;
};
class Variable_Record
{
public:
	char* name;
	Value* value;
	Variable_Record(char* name)
	{
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
	}
	~Variable_Record()
	{
		delete[] name;
		delete (value);
	}
	void* GetValue() { return value->value; }
	void SetValue(Value* new_value)
	{
		if (new_value->type != value->type)
			throw "Invalid Variable Type Assignment";
		value->value = new_value->value;
	}
};

class Variable_List : public List
{
public:
	Variable_List();
	Variable_Record* get_variable(char* var_name);

};

#endif