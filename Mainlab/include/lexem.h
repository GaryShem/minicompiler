#ifndef LEXEM_H
#define LEXEM_H
#include "variable.h"

enum LexemType
{
	KEYWORD,
	VARIABLE_NAME,
	NUMERIC_CONST,
	OPERATION,
	OPERATOR_DELIMITER,
	PARENTHESES
};

enum OPERATION_TYPE
{
	UNARY,
	BINARY,
	UNARY_OR_BINARY
};

enum OPERATIONS
{
	ASSIGNMENT = 11111,
	BINARY_PLUS = 10100
};

enum KEYWORD_TYPE
{
	VARIABLE_DECLARATION,
	TYPE_CASTING
};

enum PARENTHESIS_TYPE
{
	NORMAL_OPEN = 10000,
	NORMAL_CLOSE,
	CURLY_OPEN,
	CURLY_CLOSE,
	SQUARE_OPEN,
	SQUARE_CLOSE,
};

struct Lexem
{
	LexemType type;
	VARIABLE_TYPE var_type;
	OPERATION_TYPE op_type;
	int const_value;
	int code;
	int starting_position;
	int last_position;

	char* word_string;
};


#endif
