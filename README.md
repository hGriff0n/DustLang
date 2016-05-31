# The Dust Programming Language (v 1.3.1)
	
Dust is the working title for a multi-paradigm, expression-based, interpreted programming language. The development and design of Dust is largely intended as a hobby project that would explore how
programming languages are designed and implemented by growing and nurturing the small seed that has sat in my mind for a couple of years into a fully-featured language. It is not intended to be a
"research" endeavor that seeks to try out new paradigms and implementations. Indeed, many of dust's features are cobbled together from many different languages (ie. C++, Lua, Haskell, Python).

#### [Implementation](https://github.com/hGriff0n/DustLang/tree/master/Interpreter)
Dust is built using the [PEGTL](https://github.com/ColinH/PEGTL) library to implement its parsing algorithm. Many thanks to ColinH for both creating an intuitive and efficient parsing library, and for providing excellent documentation.

Dust is built and designed using Visual Studio 2015 and remains untested on other compilers. However, provided the compiler supports C++14, compilation should not be an issue.
Currently the Dust interpreter and runtime, along with the included testing framework and repl loop, consist of just under 5000 SLOC, split across header and source files

Do note that with Issue #18, compilation is not currently supported on non-Windows devices as command line color support is not implemented


# Dust

Now with the introduction and technical details out of the way, allow me to show you a quick run-through of dust syntax, semantics and features.

	print("Welcome to the Dust Language")			## Note: Functions are not yet implemented in the interpreter

## Semantical Design

Dust is designed to be an expression-based programming language. This means that every, or nearly every, construction in dust is reducible to a single value,
which can then be used by other expressions or ignored. 
[BENEFITS OF THE EXPRESSION-BASED MODEL].

Dust also natively supports many programming paradigms, including OOP, imperative, and functional styles (Though currently the functional support is lacking).

## Basic Syntax

Scoping in dust is performed similar to python, being dependent on explicit indentation (currently a '\t' is expected, see Issues #19).

Two #'s are used to start a single-line comment. There is currently no syntax for multi-line comments.

#### Assignments

An assignment in dust is performed through the ':' symbol, expecting a list of variables on the left and an expression list on the right.
An assignment expression evaluates to the value of the last assigned variable and has the lowest priority of all dust expressions.

    b: 3 ^ 2
    (a: 1) = 1                             ## true
    c: b = 8 + a                           ## true

Compound assignments can be performed by appending a binary operator after the assignment operator (eg. ":+").
The expression is then evaluated as if the variable being assigned was spliced in between the assignment and the compound operators.
The original expression is treated as if it was surrounded by parentheses.

	a: b: 1
	(a:* 2 + 2) = (b: b * (2 + 2))         ## true

Dust also supports multiple assignment, hence the expectation of a list on both sides. A multiple assignment expression is basically a
compaction of many lines into one statement. The number of variables is rather straightforward, but the number of expressions is a bit
more involved due to multiple returns (variables will also gain the "splat" symbol in the future). A function returning multiple values
can be viewed as increasing the number of "expressions" be the number of extra values.

When there are more variables to be assigned than values to give them, the extra variables are assigned 'nil'. When there are more values
than variables to take them, any leftover expressions are not evaluated. In the case of functions, however, this guarantee doesn't hold
for multiple returns. The expression list is evaluated before any assignments are made and a swap in dust is rather trivial as a result.

	a, b, d: b, a, 4					   ## simple swap
    a, b, c: 4, 3, 2, (d: 1)               ## 'd' is still equal to 4 as the assignment is never evaluated
    a, b, c, d: 1, 2, 3                    ## d is now equal to nil

#### Tables

The prime organizational tool that dust provides to programmers is the table, an associative container that can have both keys and fields of any type.
Tables are found everywhere in dust; scoping is even handled using tables (though scope tables aren't tracked by the garbage collector while they are scopes).

Tables are defined and created by placing a series of expressions within two brackets (to the parser and evaluator, tables are blocks internally). The only difference
between the two is that intermediate results in table evaluation, ie. non-assignments, are saved using a default key, integers from [1..] by default. An empty table
is specified by having no expressions between the brackets. Empty tables are especially important to dust as they are another way of specifying "false" (Note: A table
with only nil members is currently not considered to be empty).

    a: [ 1 2 3 b: 4 ]
    b: [ 1 3 ]                             ## Create an array[2] with elements 1 and 3
    c: [] and 3 or 5                       ## Assign 5 to c

To access table elements, dust offers two syntaxes, dot('.') and bracket('[]'). Brackets are the default method of indexing dust tables, since they allow any dust
expression to be used as a key value. Dot syntax is primarily a form of syntactical sugar for string and integer literals (Note: a["1"] != a.1 and a[1] = a.1).
There are (currently) no special considerations or semantics that need to be considered when assigning table fields.

    a[1] = a.1                             ## true
    b.3: 5                                 ## Set the 3rd element of b to 5
    a[b[2]]                                ## Access a with key = b[2]

Naturally, the Table type also defines a set of operators (+ (addition), - (subtraction), ^ (intersection), * (union), = (equality)). The Table type occupies a
unique position for dust types as Tables are chosen (in operator resolution) over any other type, regardless of converters and operator definitions, though this
may be changed in later versions (Note this means that "[3] = 3"). It should also be noted, that these operators treat their tables more as sets and arrays than
tables and are currently implemented using linear search and traversal (ie. not efficient).

    a - b                                                   ## [ 1 2 4 ]
    a ^ c and "C is in A" or "C is not in A"                ## C is not in A
    b * 4                                                   ## [ 1 3 5 4 ]
    a + c                                                   ## [ 1 2 3 4 1 3 5 ]

Lastly, tables are also used to implement the user-defined type system, with some modifications. This will be explored more in the Type System section.

### Functions

What to mention:
	Standard call syntax
	Argument handling (any number without issue)
	Multiple return statements
	OOP resolution
	Future metamethod work

Functions are very important to dust, both in implementation and in spirit.

Function calling is fairly simple and largely unexciting. The only interesting thing is that dust functions naturally handle any number of arguments with
no issues of stack corruption (excepting stack overflow of course)

	Int.abs(3) = -3.abs()					## Demonstrating OOP syntax. Both sides call the same function Int.abs
	max(1, 2, 3, 5)							## Max takes >1 arguments
	give5(3)								## Gives 5 as expected. The 3 is evaluated, but unused

OOP in dust is performed using the special argument, "self".


## Type System

What to mention:
	Type Declaration
		member/static split
		default methods
	Type Creation
		Table backing structure
		can't assign static fields from instance
		can't create static fields from instance

Naturally, dust provides the ability to define any number of custom types 