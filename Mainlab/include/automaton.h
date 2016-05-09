#ifndef AUTOMATON_H
#define AUTOMATON_H
#include "list.h"


class Triad
{
public:
	int current_state;
	char symbol;
	int next_state;
};


class automaton
{
public:
	automaton(char* filename);
	~automaton();
	Triad* find_triad(char letter, int current_state);
	int create_new_triads(char* word, int letter_index, int current_state, int max_state);
	void generate_triads();
	void print_triads();
private:

	char* filename;
	List triads_list;
};


#endif