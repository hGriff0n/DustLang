#pragma once

enum class ValType : int {
	INT = 1,
	FLOAT = 2,
};

enum class TokenType : int {
	Comment = 0,
	Literal = 1,
	Operator = 2,
	Expr = 99,
};