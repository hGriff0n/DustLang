#pragma once

enum class ValType : int {
	BOOL = 0,
	INT = 1,
	FLOAT = 2,
	STRING = 3,
	FUNCTION = 4,
	TABLE = 6,
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