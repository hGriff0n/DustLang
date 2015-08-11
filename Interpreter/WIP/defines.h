#pragma once

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