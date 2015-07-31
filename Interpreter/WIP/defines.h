#pragma once

// Ensure that the common type of two types is always the biggest (for the current version)
	// ie. lub(INT, FLOAT) == FLOAT, lub(STRING, FUNCTION) == FUNCTION
enum class ValType : int {
	BOOL = 0,
	INT = 1,
	FLOAT = 2,
	STRING = 3,
	TABLE = 6,
	FUNCTION = 7,
};

enum class TokenType : int {
	Comment = 0,
	Literal = 1,
	Operator = 2,
	Variable = 3,
	Expr = 98,
	Debug = 99,
	Assignment = 100,			// Temporary
	List = 101,					// Temporary
};