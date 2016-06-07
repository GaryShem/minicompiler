#ifndef VARIABLE_H
#define VARIABLE_H
#include "list.h"
#include <cstring>

enum VARIABLE_TYPE
{
	INT = 10300,
	DOUBLE,
	BOOL
};

class Value
{
public:
	Value(VARIABLE_TYPE type);
	~Value();
	VARIABLE_TYPE type;
	VARIABLE_TYPE GetType();
	double GetValue();
	void SetValue(double new_value);
private:
	double value;
};
class Variable_Record
{
public:
	char* name;
	Value* value;
	VARIABLE_TYPE GetType();
	double GetValue();
	void SetValue(double new_value);
	Variable_Record(char* name, Value* value = NULL)
	{
		this->name = new char[strlen(name) + 1];
		strcpy(this->name, name);
		this->value = value;
	}
	~Variable_Record()
	{
		delete[] name;
		delete (value);
	}
};

class Variable_List : public List
{
public:
	Variable_List(Variable_List* parent_list, int list_number);
	~Variable_List();
	Variable_Record* get_variable(char* var_name);
	Variable_List* _parent_list;
	List _children_list;
	int _list_number;
};

#endif