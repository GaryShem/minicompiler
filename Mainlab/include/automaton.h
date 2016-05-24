#ifndef AUTOMATON_H
#define AUTOMATON_H

#include "list.h"
#include "lexem.h"

struct Triad
{
	int current_state;
	char symbol;
	int next_state;
};

class automaton
{
public:
	automaton(char* filename, char wildcard = '\a', bool greedy = true);
	~automaton();
	Triad* find_triad(int current_state, char letter);
	void read_triads(char delimiter = ' ');
	void print_triads();
	Lexem* check(char* str, int index, int current_state = 0);
private:

	bool _greedy;
	char wildcard;
	char* filename;
	List triads_list;
	char* alphabet;
	char* wildcard_match_string;
};


#endif