# Dust Lang
	
Dust is the working title for a multi-paradigm, expression-based, interpreted programming language. The development and design of Dust is largely intended as a hobby project that would explore how programming languages are designed and implemented. 

	print("Welcome to the Dust Language")

## [Implementation](https://github.com/hGriff0n/DustLang/tree/master/Interpreter/WIP)
Dust uses the [PEGTL](https://github.com/ColinH/PEGTL) library to implement its parsing algorithm. Many thanks to ColinH for the excellent documentation and implementation of the library.

## Syntax

Dust is designed to be an expression-based, multi-paradigm programming language. This means that every, or nearly every, construction in dust is reducible to a single value, which can then be used by other expressions or dropped as needed.

Two #'s are used to start a single-line comment. There is currently no syntax for multi-line comments.

#### Operators

The operators that can be overloaded are (Currently this is also the set of all operators):

	Unary Operators: _ou!, _ou-
	Binary Operators: _op*, _op-, _op/, _op+, _op^
	Relational Operators: _op=, _op<, _op>, _op!=, _op<=, _op>=

Discuss Common Type and the process of operator resolution and invocation

#### Assignments

Assignment in dust is performed through the ':' operator.
An assignment expression reduces to the value of the assigned variable.
Assignments have the lowest priority of all dust expressions.

    b: 3 ^ 2
	(a: 1) = 1                             ## true
    c: b = 8 + a                           ## true

Compound assignments can be performed by appending a binary (or relational) operator onto the assignment operator.
The expression is then evaluated as if the variable being assigned was spliced in between the assignment and the compound operators.
The original expression is treated as if it was surrounded by parentheses.

	a: b: 1
	(a:* 2 + 2) = (b: b * (2 + 2))         ## true

Dust also supports multiple assignment.
Multiple assignment can be intuitively viewed as a compaction of a series of assignments into one line.
Thusly, a multiple assignment reduces to the value of the last assigned variable.
However this intuitive view breaks down slightly when the number of variables and expressions don't match.
Current behavior when the # of variables is greater than the # of expressions is to assign nil to the leftover variables.
When the # of expressions is greater than the number of variables, the extra expressions are "dropped" from evaluation.

    (a, b, c, d: 1, 2, 3, 4) = 4           ## true
    a, b, c: 4, 3, 2, (d: 1)               ## 'd' is still equal to 4 as the assignment is never evaluated
    a, b, c, d: 1, 2, 3                    ## d is now equal to nil (Currently implemented as 0)