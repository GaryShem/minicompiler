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
	automaton(char* filename, char wildcard = '\a');
	~automaton();
	Triad* find_triad(char letter, int current_state);
	void read_triads(char delimiter = ' ');
	void generate_triads();
	void print_triads();
private:
	int create_new_triads(char* word, int letter_index, int current_state, int max_state);

	char wildcard;
	char* filename;
	List triads_list;
};


#endif