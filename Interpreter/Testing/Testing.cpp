#include "Testing.h"
#include <sstream>
#include <iostream>

namespace dust {
	namespace test {
		void runTests(EvalState& e, bool print_all) {
			auto t = makeTester(e, std::cout, print_all);

			// Basics and Type System tests
			t.initSubTest("Basics and Type System");
				t.requireEval("3", 3);											// 1
				t.requireType("3", "Int");										// 2
				t.requireNoError("## Hello");									// 3
				t.requireEval("3## Hello", 3);									// 4
				t.requireNoError(" ");											// 5
				t.requireType("", "Nil");										// 6

				t.requireEval("\"3\"", "3");									// 7
				t.requireType("\"3\"", "String");								// 8

				t.requireError(".3");											// 9
				t.requireEval("3.3", 3.3);										// 10
				t.requireType("3.3", "Float");									// 11

				t.requireEval("true", true);									// 12
				t.requireType("true", "Bool");									// 13
			t.closeSubTest();

			// Operator resolution tests
			t.initSubTest("Operator Resolution");								// 1
				t.requireEval("3 + 3", 6);										// 2
				t.requireEval("\"The answer is \" + (6.3 ^ 2)",
					"The answer is 39.690000");									// 3
				t.requireError("3 + true");										// 4
				t.requireException<error::dispatch_error>("3 + true");			// 5
				t.requireEval("\"4\" + 3", "43");								// 6
				t.requireEval("\"4\" - 3", 1);									// 7
			t.closeSubTest();

			// Test assignments
			t.initSubTest("Assignment");
				t.requireEval("a: 2", 2);										// 1
				t.requireTrue("a = 2");											// 2

				t.requireEval("a, b: 1, 2", 2);									// 3
				t.requireTrue("a = 1 and b = 2");								// 4

				t.requireEval("a, b: b, a", 1);									// 5
				t.requireTrue("a = 2 and b = 1");								// 6

				t.requireEval("a, b: 1, 2, 3", 2);								// 7
				t.requireTrue("a = 1 and b = 2");								// 8

				//t.requireEval("a, b: 3", Nil{});								// 9		## Can't print Nil
				t.requireTrue("(a, b: 3) = nil");
				t.requireTrue("a = 3 and !b");									// 10
				t.eval("b: 0");

				// Compound Operations
				t.initSubTest("Compound Assignment");
					t.requireEval("a, b:+ 2, 2", 2);							// 1
					t.requireTrue("a = 5 and b = 2");							// 2

					t.requireException<error::dispatch_error>("a, b:* 2");		// 3
					t.requireTrue("a = 10");									// 4

					t.requireException<error::dispatch_error>("a, b:* nil, 2");	// 5
					t.requireTrue("a = 10");									// 6

					t.eval("a, b: 5, 2");

					t.requireEval("a, b:* 2, 0", 0);							// 7
					t.requireTrue("a = 10 and b = 0");							// 8

					t.requireEval("a:= (b: 3) * 2 + 2 ^ 2", true);				// 9
					t.requireTrue("a and b = 3");								// 10
				t.closeSubTest();
			t.closeSubTest();

			// Test boolean ternary statement
			t.initSubTest("Boolean Ternary");
				t.requireEval("true and false or true", "true");				// 1		# Counter-intuitive. But this is how lua's ternary operator works
				t.requireEval("!c and 4 or 5", 4);								// 2

				t.requireException<pegtl::parse_error>("false and a: 5");		// 3
				t.requireEval("false and (a: 5)", false);						// 4
				t.requireTrue("a = true");										// 5		# Checking that a: 5 is not evaluated

				t.requireEval("a: b or 5", 3);									// 6
				t.requireTrue("a = 3");											// 7

				t.requireEval("b: c and 4 or (c: 5)", 5);						// 8
				t.requireTrue("b = 5 and c = b");								// 9

				t.requireEval("b: c and 4 or (c: 5)", 4);						// 10
				t.requireTrue("b = 4");											// 11
			t.closeSubTest();

			t.initSubTest("Tricky Operations");
				t.requireEval("3 + (a: 4)", 7);									// 1
				t.requireTrue("a = 4");											// 2

				t.requireException<pegtl::parse_error>("3 + a: 3");				// 3
				//t.requireException<error::missing_node_x>("a, b: 3, c: 4");		// 4

				t.requireEval("a, b: 3, (c: 4)", 4);							// 5
				t.requireTrue("a = 3");											// 6

				t.requireEval("3 +-3", 0);										// 7

				t.eval("a: 2");
				t.requireEval("(a: 3) + 3 * a", 12);							// 8
				t.requireTrue("a = 3");											// 9
			t.closeSubTest();

			t.initSubTest("Type System");
				t.requireTrue("3 <- Int");										// 1
				t.requireTrue("3 + 0.3 <- Float");								// 2
			t.closeSubTest();

			t.initSubTest("Multiline Parsing");				// Need to escape output
				t.requireEval("a\n+ b", 7);										// 1
				t.requireException<pegtl::parse_error>("3 + a: 3\n - 4");		// 2
				t.requireEval("3 - \n 3", 0);									// 3
				t.requireEval("3 + 3\n  \n4 + 4", 8);							// 4
				t.requireEval("## Hello\n3", 3);								// 5

				t.initSubTest("Scoped Assignment");
					t.requireEval("a: 2\n	a: 5\n	a", 5);						// 1
					t.requireTrue("a = 2");										// 2
					t.requireEval("a", 2);										// 3
					t.requireEval("a: 3\n	a + 2", 5);							// 4
					t.requireEval("a: 4\n"
								  "	a: 3\n"
								  "	b: a + .a", 7);							// 5
					t.requireEval(".a", 4);										// 6
				t.closeSubTest();

				t.initSubTest("Scopes and Comments");
					t.requireEval("a: 2 ## Assign 2 to a\n"
								  "	b: .a + 3 ## Assign b to a + 3\n"
								  "	b + a", 7);								// 1
					t.requireEval("a: 2\n"
								   "	b: 3 + .a\n"
								   "## Assign b to 3 + a\n"
								   "	b + a", 7);								// 2
					t.requireEval("3\n"
								  "		a: 5\n"
								  "	## Comment\n"
								  "		a + 1", 6);
					t.requireEval("af: 3\n"
								  "		5\n"
						          "	af", 3);									// 3
					t.requireEval("3\n"
								  "## Comment\n"
								  "4", 4);										// 4
				t.closeSubTest();
			t.closeSubTest();

			t.initSubTest("Table Testing");
				t.eval("a: [ 1 ]\nb: 1");
				t.requireEval("a.1", 1);
				t.requireTrue("a[1] = a[b]");

				t.eval("a.a: [ a: 3 ]");
				t.requireEval("a", "[ 1, a: [ a: 3 ] ]");
				t.requireEval("a.a", "[ a: 3 ]");
				t.requireEval("a.a.b: 2", 2);
				t.requireEval("a.a", "[ a: 3, b: 2 ]");

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

				t.requireEval("a[b[2]]", 3);
				t.requireEval("a[b[2] * 2]", 5);

			t.closeSubTest();

			t.initSubTest("Try-Catch Statement");
				t.requireEval("try\n"
							  "	3 + true\n"
							  "catch (e) 4\n", 4);

				t.requireEval("try 3 + true\n"
							  "catch (e) 4", 4);

				t.requireEval("try 3 + 3\n"
							  "catch (e) 4", 6);

				t.requireEval("a: 3\n"
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

				t.requireEval("try\n"
							  "	.a: 3 + 3\n"
							  "	3 + a\n"
							  "catch(e)\n", 9);

				t.requireType("try 3 + true\n"
							  "catch (e) e", "String");

				t.requireException<error::missing_node_x>("catch (e) 4");

			t.closeSubTest();

			t.initSubTest("Control Flow Testing");
				t.initSubTest("While Loop");
					// Test that the while loop works
					t.requireEval("i: 0\n"
								  "while i < 5\n"
								  "	.i:+ 1\n"
								  "i", 5);

					// Test inline while loop
					t.requireEval("i: 0\n"
								  "while i < 5 .i:+ 1\n"
								  "i", 5);

					// Test optional do statement
					t.requireEval("i: 0\n"
								  "while i < 5 do\n"
								  "	.i:+ 1\n"
								  "i", 5);

					// Test order of evaluation in while loop
					t.requireEval("i: 5\n"
								  "while i < 5 .i:+ 1\n"
								  "i", 5);

					// Test the value of the while loop
				t.closeSubTest();

				t.initSubTest("Do-While loop");
					// Test that the do-while loop works
					t.requireEval("i: 0\n"
								  "repeat i < 5\n"
								  "	.i:+ 1\n"
								  "i", 5);

					// Test inline do-while loops
					t.requireEval("i: 0\n"
								  "repeat i < 5 .i:+ 1\n"
								  "i", 5);

					// Test order of evaluationi in do-while loop
					t.requireEval("i: 5\n"
								  "repeat i < 5 .i:+ 1\n"
								  "i", 6);

					// test the value of the do-while loop
				t.closeSubTest();

				t.initSubTest("If statements");
					t.requireEval("if i 3", 3);
					t.requireEval("if i = 3 6", false);				// Doesn't execute
					t.requireEval("if i = 3 6\n"
								  "else 7", 7);
					t.requireEval("if i = 3 6\n"
								  "elseif i = 4 7\n"
								  "else 8", 8);
					t.requireException<error::missing_node_x>("else \"Hello\"");
					t.requireException<error::missing_node_x>("if i 3\n"
																		"i: 3 + 3\n"
																		"else 5");
					t.requireException<error::invalid_ast_construction>("if i 3\n"
																		"else 5\n"
																		"elseif a = 3 .i: 6");

				t.closeSubTest();
			t.closeSubTest();

			//t.initSubTest("API Testing");
			//t.closeSubTest();

			//t.end_tests();

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