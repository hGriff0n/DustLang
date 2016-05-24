#include "Testing.h"
#include <sstream>
#include <iostream>

namespace dust {
	namespace test {
		void runAllTests(EvalState& e, bool print_all) {
			using dispatch_error = error::dispatch_error;

			auto t = makeTester(e, std::cout, print_all);

			// Testing basic literals and type resolution
			t.initSubTest("Basics and Type System");
				t.requireEval("3", 3);
				t.requireType("3", "Int");

				t.requireNoError("## Hello");									// Testing that comments parse correctly
				t.requireEval("3## Hello", 3);
				t.requireNoError(" ");											// Testing that empty files won't break the interpreter
				t.requireType("", "Nil");

				t.requireEval("\"3\"", "3");
				t.requireType("\"3\"", "String");

				t.requireError(".3");											// Testing that decimals must be well formed
				t.requireEval("3.3", 3.3);
				t.requireType("3.3", "Float");

				t.requireEval("true", true);
				t.requireType("true", "Bool");
			t.closeSubTest();

			// Testing Operator Type Resolution
			t.initSubTest("Operator Resolution");
				t.requireEval("3 + 3", 6);										// Testing that operators work
				t.requireEval("\"The answer is \" + (6.3 ^ 2)",					// Testing that converters get selected
					"The answer is 39.690000");

				t.requireException<dispatch_error>("\"4\" - 3");				// Testing that converter selection is consistant
				t.requireException<dispatch_error>("3 + true");					// Testing that correct exceptions are produced
			t.closeSubTest();

			// Testing basic variable assignment
			t.initSubTest("Assignment");
				t.requireEval("a: 2", 2);										// Testing basic assignment semantics
				t.requireTrue("a = 2");

				t.requireEval("a, b: 1, 2", 2);									// Testing multiple assignment works properly
				t.requireTrue("a = 1 and b = 2");

				t.requireEval("a, b: b, a", 1);									// Swapping the variables
				t.requireTrue("a = 2 and b = 1");

				t.requireEval("a, b: 1, 2, 3", 2);								// Multiple assignment with more values than variables
				t.requireTrue("a = 1 and b = 2");

				t.requireTrue("(a, b: 3) = nil");								// Multiple assignment with more variables than values
				t.requireTrue("a = 3 and !b");
				t.eval("b: 0");

				// Testing compound assignment operations
				t.initSubTest("Compound Assignment");
					t.requireEval("a, b:+ 2, 2", 2);							// Testing basic compound assignment semantics
					t.requireTrue("a = 5 and b = 2");

					t.requireException<dispatch_error>("a, b:* 2");				// Throws errors when expected
					t.requireTrue("a = 10");									// Testing l->r evaluation

					t.requireException<dispatch_error>("a, b:* nil, 2");
					t.requireTrue("b = 2");

					t.eval("a, b: 5, 2");

					t.requireEval("a, b:* 2, 0", 0);							// Further testing semantics
					t.requireTrue("a = 10 and b = 0");

					t.requireEval("a:= (b: 3) * 2 + 2 ^ 2", true);				// Testing AST construction with large value
					t.requireTrue("a and b = 3");
				t.closeSubTest();
			t.closeSubTest();

			// Test boolean ternary statement
			t.initSubTest("Boolean Ternary");
				t.requireEval("true and false or true", "true");				// Testing basic ternary semantics
				t.requireEval("!c and 4 or 5", 4);								// Counter-intuitive. But this is how lua's ternary operator works

				t.requireException<pegtl::parse_error>("false and a: 5");		// Testing AST construction
				t.requireEval("false and (a: 5)", false);
				t.requireTrue("a = true");										// Testing short-circuit evaluation

				t.requireEval("a: b or 5", 3);
				t.requireTrue("a = 3");

				t.requireEval("b: c and 4 or (c: 5)", 5);						// Testing ternary semantics
				t.requireTrue("b = 5 and c = b");

				t.requireEval("b: c and 4 or (c: 5)", 4);						// Testing short-circuit evaluation
				t.requireTrue("b = 4");
			t.closeSubTest();

			// Testing precise parsing and evaluation
			t.initSubTest("Tricky Operations");
				t.requireEval("3 + (a: 4)", 7);									// Testing AST construction of Assignment
				t.requireTrue("a = 4");

				t.requireException<pegtl::parse_error>("3 + a: 3");				// Testing parser error production
				//t.requireException<error::missing_node_x>("a, b: 3, c: 4");

				t.requireEval("a, b: 3, (c: 4)", 4);							// Testing nested assignments
				t.requireTrue("a = 3");

				t.requireEval("3 +-3", 0);										// Testing AST construction with unary operators

				t.eval("a: 2");
				t.requireEval("(a: 3) + 3 * a", 12);							// Testing l->r evaluation
				t.requireTrue("a = 3");
			t.closeSubTest();

			// Testing type interactions
			t.initSubTest("Type System");
				t.requireTrue("3 <- Int");										// Test that the type-check operator works
				t.requireTrue("3 + 0.3 <- Float");

				t.requireNoError("type NewType [ a: 0 ]");						// Test that custom types work properly
				t.requireNoError("foo: NewType.new()");
				t.requireTrue("foo <- NewType");

				t.requireEval("foo.class", "NewType");							// Test that the type is created properly
				t.requireTrue("foo._type = NewType");
					// Fails with pegtl::error <Can't find eolf>

				t.requireTrue("foo.a = 0");										// Testing instance semantics
				t.requireEval("foo.a: 3", 3);
				t.requireEval("foo.a - NewType.a", 3);
				t.requireTrue("foo._type.a != foo.a");
				t.requireException<error::dust_error>("foo._max: 3");

				t.requireNoError("type TestType [ a: 0 ]\n"
								 "def TestType.new(_a)\n"
								 "	.a = _a or 0\n");
				t.requireType("bar: TestType.new(5)", "TestType");
				t.requireEval("bar.a", 5);

				//t.eval("bar = foo.type.new()");
				//t.requireTrue("bar <- NewType");								// Test typeof operations
			t.closeSubTest();

			// Testing parser with multilined input
			t.initSubTest("Multiline Parsing");
				t.requireEval("a\n+ b", 7);										// Test that parser handles newlines where correct
				t.requireException<pegtl::parse_error>("3 + a: 3\n - 4");
				t.requireEval("3 - \n 3", 0);
				t.requireEval("3 + 3\n  \n4 + 4", 8);
				t.requireEval("## Hello\n3", 3);

				// Testing parser on scopes and comments
				t.initSubTest("Scopes and Comments");
					t.requireEval("a: 2 ## Assign 2 to a\n"						// Test that scoping works
								"	b: .a + 3 ## Assign b to a + 3\n"
								"	b + a", 7);

					t.requireEval("a: 2\n"										// Test that comments don't break scope parsing
								"	b: 3 + .a\n"
								"## Assign b to 3 + a\n"
								"	b + a", 7);

					t.requireEval("af: 3\n"										// Test that scopes are constructed correctly
								"		5\n"
								"	af", 3);

					t.requireEval("3\n"											// Test that comments don't break newline parsing
								"## Comment\n"
								"4", 4);
				t.closeSubTest();

				// Testing scoped variables
				t.initSubTest("Scoped Assignment");
					t.requireEval("a: 2\n	a: 5\n	a", 5);						// Test scope lookup in get
					t.requireTrue("a = 2");										// Test scope lookup set
					t.requireEval("a: 3\n	a + 2", 5);

					t.requireEval("a: 4\n"										// Test forced scope lookup
								  "	a: 3\n"
								  "	b: a + .a", 7);
					t.requireEval(".a", 4);
				t.closeSubTest();
			t.closeSubTest();

			// Testing table literals and operations
			t.initSubTest("Table Testing");
				t.eval("a: [ 1 ]\nb: 1");
				t.requireEval("a.1", 1);										// Test table construction and indexing
				t.requireTrue("a[1] = a[b]");

				t.eval("a.a: [ a: 3 ]");
				t.requireEval("a", "[ 1, a: [ a: 3 ] ]");						// Test table printing and indexing
				t.requireEval("a.a", "[ a: 3 ]");
				t.requireEval("a.a.b: 2", 2);
				t.requireEval("a.a", "[ a: 3, b: 2 ]");

				// Test all table specific operators
				t.initSubTest("Table Operators");
					t.eval("a: [ 1 2 3 2 5 5 4 ]");
					t.eval("b: [ 1 3 ]");
					t.eval("c: 5");

					t.requireTrue("(a ^ b) = b");
					t.requireEval("a ^ c", "[ 5, 5 ]");
					t.requireEval("b + c", "[ 1, 3, 5 ]");
					t.requireEval("a - b", "[ 2, 2, 5, 5, 4 ]");
					t.requireEval("b * c", "[ 1, 3, 5 ]");
				t.closeSubTest();

				t.requireEval("a[b[2]]", 3);									// Test bracket indexing with sub-expressions
				t.requireEval("a[b[2] * 2]", 5);

			t.closeSubTest();

			// Testing dust exception handling
			t.initSubTest("Try-Catch Statement");
				t.requireEval("try\n"											// Test basic semantics
							  "	3 + true\n"
							  "catch (e) 4\n", 4);

				t.requireEval("try 3 + true\n"									// Test in-line declaration
							  "catch (e) 4", 4);

				t.requireEval("try 3 + 3\n"										// Test behavior on no-excep
							  "catch (e) 4", 6);

				t.requireEval("a: 3\n"											// Test behavior with mutliline blocks
							  "try\n"
							  "	3 + true\n"
							  "	.a: 2\n"
							  "catch (e)\n"
							  "a", 3);

				t.requireEval("a: 3\n"
							  "try\n"
							  "	3 + 3\n"
							  "	.a: 2\n"
							  "catch (e)\n"
							  "a", 2);

				t.requireEval("try\n"											// Ensure no catch block needed
							  "	.a: 3 + 3\n"
							  "	3 + a\n"
							  "catch(e)\n", 9);

				t.requireType("try 3 + true\n"									// Ensure type of exception
							  "catch (e) e", "String");

				t.requireException<error::missing_node_x>("catch (e) 4");		// Test parsing with no try statement

			t.closeSubTest();

			// Testing control flow constructions
			t.initSubTest("Control Flow Testing");

				// Testing the while loop
				t.initSubTest("While Loop");
					t.requireEval("i: 0\n"										// Testing basic semantics
								  "while i < 5\n"
								  "	.i:+ 1\n"
								  "i", 5);

					t.requireEval("i: 0\n"										// Test inline loop
								  "while i < 5 .i:+ 1\n", 5);

					t.requireEval("i: 0\n"										// Test parsing of optional "do"
								  "while i < 5 do\n"
								  "	.i:+ 1\n"
								  "i", 5);
					
					t.requireEval("i: 5\n"										// Test order of evaluation
						"while i < 5 .i:+ 1\n"
						"i", 5);

					t.requireType("i: 5\n"										// Test value of unevaluated loop
						"while i < 5 .i:+ 1\n", "Nil");

				t.closeSubTest();

				// Testing the do-while loop
				t.initSubTest("Do-While loop");
					t.requireEval("i: 0\n"										// Test basic semantics
								  "repeat i < 5\n"
								  "	.i:+ 1\n"
								  "i", 5);

					t.requireEval("i: 0\n"										// Test inline construction
								  "repeat i < 5 .i:+ 1", 5);
					
					t.requireEval("i: 5\n"										// Test order of evaluation
								  "repeat i < 5 .i:+ 1\n"
								  "i", 6);
				t.closeSubTest();

				// Testing if-elseif-else
				t.initSubTest("If statements");
					t.requireEval("if i 3", 3);									// Test basic semantics
					t.requireEval("if i = 3 6", false);
					t.requireEval("if i = 3 6\n"
								  "else 7", 7);

					t.requireEval("if i = 3 6\n"								// Test elseif chaining
								  "elseif i = 4 7\n"
								  "else 8", 8);

					t.requireException<error::missing_node_x>("else \"Hello\"");
					t.requireException<error::missing_node_x>("if i 3\n"		// Test parsing correctness
															  "i: 3 + 3\n"
															  "else 5");
					t.requireException<error::invalid_ast_construction>("if i 3\n"
																		"else 5\n"
																		"elseif a = 3 .i: 6");

				t.closeSubTest();

				// Testing the for loop
				t.initSubTest("For loop");
					t.requireEval("sum: 0\n"									// Test basic semantics
								  "for i in [ 1 2 3 4 5 ]\n"
								  "	.sum:+ i\n"
								  "sum", 15);

					t.requireType("for i in [1]", "Nil");						// Test value of loop with no body

					t.requireEval("msg: \"\"\n"
								  "for w in [ \"Hello,\" \"World!\" \"I'm\" \"Margaret\" ]\n"
								  "	.msg:+ w + \" \"\n"
								  "msg", "Hello, World! I'm Margaret ");

					t.eval("a: [ 1 2 b: [ 3 4 5 ] ]");
					t.requireEval("f: 1\n"
								  "for i in a.b .f:* i\n", 60);					// Test in-line construction

					t.eval("a: [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ]");
					t.requireTrue("sum_a, sum_b: 0, 0\n"
								  "for i in (a ^ [ 1 2 3 5 7 11 13 17 ])\n"
								  "	.sum_a:+ i\n"
								  "for i in [ 1 2 3 5 7 11 13 17 ]\n"
								  "	.sum_b:+ i\n"
								  "sum_a = sum_b");
				t.closeSubTest();
			t.closeSubTest();

			// Testing function calling and definition
			t.initSubTest("Functions");
				e.push("abs");
				e.push([](EvalState& e) {
					auto x = (int)e;
					e.push(x > 0 ? x : -x);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.addMember(e.getTS().getType("Int"), "abs", [](EvalState& e) {
					e.enableObjectSyntax().get(EvalState::SELF);

					auto x = (int)e;
					e.push(x > 0 ? x : -x);
					return 1;
				});

				e.push("add");
				e.push([](EvalState& e) {
					e.push((int)e + (int)e);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("give5");
				e.push([](EvalState& e) {
					e.push(5);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("bound");
				e.push([](EvalState& e) {
					auto d = (double)e;
					e.push((int)std::floor(d));
					e.push((int)std::ceil(d));
					return 2;
				});
				e.set(EvalState::SCOPE);

				e.push("max");
				e.push([](EvalState& e) {
					Optional max{ e }, opt;							// Max may have a value, opt guaranteed to be nil

					while (opt.set(e)) {							// For each optional argument, try to beat max
						e.push(max);
						e.push(opt);
						e.callOp("_op>");							// Run opt > max

						if ((bool)e) max.set(opt);
					}

					e.push(max);
					return 1;
				});
				e.set(EvalState::SCOPE);

				e.push("sum");
				e.push([](EvalState& e) {
					Optional sum{ e }, nxt;							// But I want to demonstrate Optional

					while (nxt.copy(e)) {
						e.push(sum);
						e.callOp("_op+");

						sum.set(e);
					}

					e.push(sum);
					return 1;
				});
				e.set(EvalState::SCOPE);

				t.requireType("abs", "Function");

				// Need to add tests for having more values than expected or less

				// Testing function calling
				t.initSubTest("Calling Free Functions");
					t.requireEval("abs(1)", 1);								// Test basic semantics
					t.requireEval("abs(-1)", 1);

					t.requireEval("add(2, 3)", 5);
					t.requireEval("add( 1 + 1,3)", 5);

					t.requireEval("give5()", 5);
					
					t.requireEval("give5(3)", 5);							// Test more arguments than expected
					t.requireSize("give5(3)", 1);
					t.requireException<error::runtime_error>("add(2)");		// Test fewer arguments than expected

					t.requireEval("max(3, 5)", 5);							// Testing optional arguments
					t.requireEval("max(3)", 3);

					t.requireException<dispatch_error>("foo(3)");			// Test dispatch error
				t.closeSubTest();
				
				// Testing OOP semantics
				t.initSubTest("Calling Member Functions");
					t.eval("a: -1");

					t.requireEval("a.abs()", 1);							// Test basic OOP semantics
					t.requireEval("-1.abs()", -1);
					t.requireEval("(-1).abs()", 1);
					t.requireEval("(5 * -2).abs()", 10);
					t.requireEval("Int.abs(a - 3)", 4);						// Test calling through type

					// Error message: Attempt to call a non-function ???
					t.requireException<dispatch_error>("\"Hello\".abs()");

				t.closeSubTest();

				// Testing function passing
				t.initSubTest("Functions as Values");
					t.eval("sba: abs");
					t.requireType("sba", "Function");						// Test assignment works
					t.requireTrue("sba(-3) = abs(3)");

					t.eval("a: [ abs: abs ]");
					t.requireTrue("a.abs(3) = 3.abs()");

					t.eval("abs: 5");
					t.requireType("abs", "Int");
				t.closeSubTest();

				// Testing multiple return values
				t.initSubTest("Multiple Returns");
					t.requireEval("a, b: bound(3.3)", 4);					// Test basic semantics
					t.requireEval("a + b", 7);

					t.requireEval("add(bound(3.3))", 7);					// Test argument passing
				t.closeSubTest();

				// Testing defining functions
				t.initSubTest("Function Definitions");
					t.requireType("def min(x, y)\n"							// Test basic function definition
									"	if x < y x\n"
									"	else y", "Function");
					t.requireType("min", "Function");

					t.requireEval("min(3, 5)", 3);							// Test function calling of dust functions
					t.requireEval("min(3, 5, 7)", 3);						// With more arguments
					t.requireSize("min(3, 5, 7)", 1);

					t.requireException<dispatch_error>("min(3)");			// With fewer arguments

					t.requireType("def Float.abs(self)\n"					// Test OOP function definition
								"	self < 0 and -self or self", "Function");
					t.requireType("Float.abs", "Function");

					t.requireEval("Float.abs(-3.3)", 3.3);					// Test direct calling semantics
					t.requireEval("(-5.5).abs()", 5.5);						// Test OOP calling semantics
					t.requireEval("5.5.abs()", 5.5);						// Testing parser construction

					// Test Operator Overloading
					t.requireType("def Bool._op+(self, o)\n"				// Testing metamethod definition
								"	self or o", "Function");
					
					t.requireEval("true + false", true);					// Testing operator lookup

					t.requireType("def Int.diverge(a)\n"					// Testing function construction
								"	return a + 1, a - 1", "Function");

					t.requireEval("a, b: Int.diverge(3)", 4);				// Testing basic semantics
					t.requireEval("a + b", 6);

					t.requireException<dispatch_error>("3.diverge()");		// Testing OOP interaction

				t.closeSubTest();

				t.initSubTest("Recursion");
					t.requireType("def factorial(n)\n"
								  "	if n = 0 return 1\n"
								  "	n * factorial(n-1)", "Function");

					t.requireEval("factorial(0)", 1);
					t.requireEval("factorial(3)", 6);
				t.closeSubTest();
			t.closeSubTest();

			//t.initSubTest("Metamethods");
			//t.closeSubTest();

			t.printReview(std::cout);
		}

		std::pair<std::string, bool> makeReview(const std::string& buf, const std::string& test, int np, int nt) {
			std::stringstream ss;

			ss << buf << ":: " << test << " | Passed: " << np
									   << " | Failed: " << (nt - np)
									   << " | " << np << " / " << nt
									   << " Tests (" << std::setprecision(4) << ((float)np / nt * 100) << "%)\n";

			return std::make_pair(ss.str(), !(nt - np));
		}

	}
}