# The Dust Programming Language (v 1.3.1)
	
Dust is the working title for a multi-paradigm, expression-based, interpreted programming language. The development and design of Dust is largely intended as a hobby project that would explore how
programming languages are designed and implemented by growing and nurturing the small seed that has sat in my mind for a couple of years into a fully-featured language. It is not intended to be a
"research" endeavor that seeks to try out new paradigms and implementations. Indeed, many of dust's features are cobbled together from many different languages (ie. C++, Lua, Haskell, Python).

#### [Implementation](https://github.com/hGriff0n/DustLang/tree/master/Interpreter)
Dust is built using the [PEGTL](https://github.com/ColinH/PEGTL) library to implement its parsing algorithm. Many thanks to ColinH for both creating an intuitive and efficient parsing library, and for providing excellent documentation.

Dust is built and designed using Visual Studio 2015 and remains untested on other compilers. However, provided the compiler supports C++14, compilation should not be an issue.
Currently the Dust interpreter and runtime (including repl loop and test suite) consist of just over 5000 SLOC, split across header and source files


# Dust

Now with the introduction and technical details out of the way, allow me to show you a quick run-through of dust syntax, semantics and features.

	print("Welcome to the Dust Language")			## Note: Functions are not yet implemented in the interpreter

## Semantical Design

Dust is designed to be an expression-based programming language. This means that every, or nearly every, construction in dust is reducible to a single value,
which can then be used by other expressions or ignored. 
[BENEFITS OF THE EXPRESSION-BASED MODEL].

Dust also natively supports many programming paradigms, including OOP, imperative, and functional styles (Though currently the functional support is lacking).

## Basic Syntax (needs work)

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

Naturally, dust also provides a way to define and call functions, albeit with a few neat features. Calling a function uses rather standard language syntax, the function
name is followed by a pair of parentheses that surround the arguments to the function. A dust function can take any number of arguments to its call without any issue,
to the calling code or the runtime, making some patterns really easy to express. To complement the multiple arguments, dust functions can also return multiple values
without errors, albeit currently through an explicit "return" statement due to parsing problems (multiple return values from custom class methods do not maintain this
error-free guarantee currently). Dust also supports a form of "unified-call syntax" for OOP method calling. A type method can be called by indexing the type table and
passing the instance as the first argument or by calling the method through "indexing" the instance directly.

	give5(3)								## Gives 5 as expected. The 3 is evaluated, but unused
	max(1, 2, 3, 5)							## Max takes >1 arguments
	Int.abs(3) = -3.abs()					## Demonstrating OOP syntax. Both sides call the same function Int.abs

Function definition uses "def" syntax, similar to Python. To take advantage of the unified-call syntax for a type function, simply add the "self" parameter as the first
argument to the function. Resolution of this parameter gets handled at the call site to ensure that self is always the expected value.

	def Float.ceil(self)
		(Int)self + 1

Operators in dust are also handled through dust functions, or "metamethods" due to their nature. So "3 + 3" translates to a call "Int._op+(3, 3)" due to the common type
of 3 and 3 being an Int. Currently, dust only allows for the behavior of standard operators to be definable this way. However, the plan is to extend this behavior to many
areas of the language, such as table indexing, etc. Indeed, some of this vision can be seen in the new, copy, and drop class methods that get automatically defined for
custom types but retain the ability to be defined by the programmer later on.


## Type System

Dust is a strong, dynamically typed language, though the type system is a rather simple construction. Dust performs it's dynamic typing through the Object supertype,
which all types inherit from, except for Nil. This type system can also be extended with custom classes. At an implementation level, classes are a pair of tables, one
for the static type fields and another for the instance members. The way this split gets handled takes advantage of the required type creation syntax, the member table.
All fields declared within the table are automatically classified as "instance-level" while any fields declared outside are "static". This distinction also prevents
the addition of custom fields to instances at runtime.

	type NewType [ member: 3 ]				## member is an instance variable
	
	NewType.count: 0						## count is a static field

	def NewType.new(self, member)			## New is a class metamethod (The language version sets up the structure and passes self to the function)
		NewType.count:+ 1
		self.member: member					## self.member is initialized to 3 in the default new method

Dust supports a single inheritance object model, though there is no way to express/add "interfaces" (though due to dust's duck typing, this would just be sugar).

## MORE TO COME
