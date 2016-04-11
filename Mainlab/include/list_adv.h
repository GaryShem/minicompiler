#ifndef LAB_LIST_ADV_H
#define LAB_LIST_ADV_H

#include "list.h"

class Stack : List
{
public:
	Stack() : List(sizeof(double)) { };
	~Stack() { List::~List(); }

	void   push(double x) { add(&x); };
	double pop() { double res; take_last(&res); return res; };  
};

class Queue : List
{
public:
	Queue() : List(sizeof(double)) { };
	~Queue() { List::~List(); }

	void   put(double x) { add(&x); };
	double get() { double res; take_first(&res); return res; };
};

class Deque : List
{
public:
    Deque() : List(sizeof(double)) { };
    ~Deque() { List::~List(); }

	void   put(double x) { add(&x); };
	double first() { double res; take_first(&res); return res; };
	double last()  { double res; take_last(&res); return res; };

};

#endif