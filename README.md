# Dust Lang
	
Dust is the working title for a multi-paradigm, expression-based, interpreted programming language. The development and design of Dust is largely intended as a hobby project that would explore how programming languages are designed and implemented. 

	print("Welcome to the Dust Language")

## [Implementation](https://github.com/hGriff0n/DustLang/tree/master/Interpreter/WIP)
Details on how dust is implemented (include the author of the PEGTL library).

## Syntax
-- I might end up moving this to another document though

#### Multiple Assignments

    a, b, c, d: 1, 2, 3, 4

However the way that multiple assignments are designed and implemented results in a couple of possibly ambiguous situations, especially when chaining assignments. Thusly, it is advisable to explicitly wrap any chained assignments in parentheses so as to clearly deliminate, both to the parser and to future you, so as to avoid the confusion (Note: This semantical meaning is a current sticking point of the language's design).

    a, b: c, d: 3, 4            ## Equivalent to a, b: (c, d: 3, 4) and not a, b: c, (d: 3, 4)