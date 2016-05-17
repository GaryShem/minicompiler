#ifndef AUTOMATON_H
#define AUTOMATON_H
#include "list.h"

struct Triad
{
	int current_state;
	char symbol;
	int next_state;
};

enum LexemType
{
	VARIABLE_DECLARATION,
	KEYWORD,
	VARIABLE_NAME,
	NUMERIC_CONST,
	OPERATION_BINARY,
	OPERATION_UNARY,
	ASSIGNMENT,
	OPERATOR_DELIMITER,
	PARENTHESES
};

enum VARIABLE_TYPE
{
	INT,
	DOUBLE,
	CHAR
};

struct Lexem
{
	LexemType type;
	VARIABLE_TYPE var_type;
	int code;
	int starting_position;
	int last_position;

	char* word_string;
};

class automaton
{
public:
	automaton(char* filename, char wildcard = '\a');
	~automaton();
	Triad* find_triad(int current_state, char letter);
	void read_triads(char delimiter = ' ');
	/*void generate_triads();*/
	void print_triads();
	Lexem* check(char* str, int index, int current_state = 0);
private:
	/*int create_new_triads(char* word, int letter_index, int current_state, int max_state);*/

	char wildcard;
	char* filename;
	List triads_list;
	char* alphabet;
	char* wildcard_match_string;
};


#endif