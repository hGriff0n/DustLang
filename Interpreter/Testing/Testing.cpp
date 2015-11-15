#include "Testing.h"
#include <sstream>
#include <iostream>

namespace dust {
	namespace test {
		void run_tests(EvalState& e) {
			auto t = makeTester(e, std::cout);

			// Basics and Type System tests
			t.init_sub_test("Basics and Type System");
				t.require_eval("3", 3);										// 1
				t.require_type("3", "Int");									// 2
				t.require_noerror("## Hello");								// 3
				t.require_eval("3## Hello", 3);								// 4
				t.require_noerror(" ");										// 5
				t.require_type("", "Nil");									// 6

				t.require_eval("\"3\"", "3");								// 7
				t.require_type("\"3\"", "String");							// 8

				t.require_error(".3");										// 9
				t.require_eval("3.3", 3.3);									// 10
				t.require_type("3.3", "Float");								// 11

				t.require_eval("true", true);								// 12
				t.require_type("true", "Bool");								// 13
			t.close_sub_test();

			// Operator resolution tests
			t.init_sub_test("Operator Resolution");							// 1
				t.require_eval("3 + 3", 6);									// 2
				t.require_eval("\"The answer is \" + (6.3 ^ 2)",
					"The answer is 39.690000");								// 3
				t.require_error("3 + true");								// 4
				t.require_excep<error::dispatch_error>("3 + true");			// 5
				t.require_eval("\"4\" + 3", "43");							// 6
				t.require_eval("\"4\" - 3", 1);								// 7
			t.close_sub_test();

			// Test assignments
			t.init_sub_test("Assignment");
				t.require_eval("a: 2", 2);									// 1
				t.require_true("a = 2");									// 2

				t.require_eval("a, b: 1, 2", 2);							// 3
				t.require_true("a = 1 and b = 2");							// 4

				t.require_eval("a, b: b, a", 1);							// 5
				t.require_true("a = 2 and b = 1");							// 6

				t.require_eval("a, b: 1, 2, 3", 2);							// 7
				t.require_true("a = 1 and b = 2");							// 8

				//t.require_eval("a, b: 3", Nil{});							// 9
				t.require_true("(a, b: 3) = nil");
				t.require_true("a = 3 and !b");								// 10
				t.eval("b: 0");

				// Compound Operations
				t.init_sub_test("Compound Assignment");
					t.require_eval("a, b:+ 2, 2", 2);						// 1
					t.require_true("a = 5 and b = 2");						// 2

					t.require_excep<error::dispatch_error>("a, b:* 2");
					t.require_true("a != 10");
					t.require_eval("a, b:* 2, 0", 0);						// 3
					t.require_true("a = 10 and b = 0");						// 4

					t.require_eval("a:= (b: 3) * 2 + 2 ^ 2", true);			// 5
					t.require_true("a and b = 3");							// 6
				t.close_sub_test();
			t.close_sub_test();

			// Test boolean ternary statement
			t.init_sub_test("Boolean Ternary");
				t.require_eval("true and false or true", "true");			// 1		# Counter-intuitive. But this is how lua's ternary operator works
				t.require_eval("!c and 4 or 5", 4);							// 2

				t.require_excep<pegtl::parse_error>("false and a: 5");		// 3
				t.require_eval("false and (a: 5)", false);					// 4
				t.require_true("a = true");									// 5		# Checking that a: 5 is not evaluated

				t.require_eval("a: b or 5", 3);								// 6
				t.require_true("a = 3");									// 7

				t.require_eval("b: c and 4 or (c: 5)", 5);					// 8
				t.require_true("b = 5 and c = b");							// 9

				t.require_eval("b: c and 4 or (c: 5)", 4);					// 10
				t.require_true("b = 4");									// 11
			t.close_sub_test();

			t.init_sub_test("Tricky Operations");
				t.require_eval("3 + (a: 4)", 7);							// 1
				t.require_true("a = 4");									// 2

				t.require_excep<pegtl::parse_error>("3 + a: 3");			// 3
				t.require_excep<error::missing_node_x>("a, b: 3, c: 4");	// 4

				t.require_eval("a, b: 3, (c: 4)", 4);						// 5
				t.require_true("a = 3");									// 6

				t.require_eval("3 +-3", 0);									// 7

				t.eval("a: 2");
				t.require_eval("(a: 3) + 3 * a", 12);						// 8
				t.require_true("a = 3");									// 9
			t.close_sub_test();

			t.init_sub_test("Type System");
				t.require_true("3 <- Int");									// 1
				t.require_true("3 + 0.3 <- Float");							// 2
			t.close_sub_test();

			t.init_sub_test("Multiline Parsing");				// Need to escape output
				t.require_eval("a\n+ b", 7);								// 1
				t.require_excep<pegtl::parse_error>("3 + a: 3\n - 4");		// 2
				t.require_eval("3 + a 3\n - 4", -1);						// 3
				t.require_eval("3 + 3\n  \n4 + 4", 8);						// 4
				t.require_eval("## Hello\n3", 3);							// 5

				t.init_sub_test("Scoped Assignment");
					t.require_eval("a: 2\n\ta: 5\n\ta", 5);					// 1
					t.require_true("a = 2");								// 2
					t.require_eval("a", 2);									// 3
					t.require_eval("a: 3\n\ta + 2", 5);						// 4
					t.require_eval("a: 4\n"
								   "\ta: 3\n"
								   "\tb: a + .a", 7);						// 5
					t.require_eval(".a", 4);								// 6
				t.close_sub_test();

				t.init_sub_test("Scopes and Comments");
					t.require_eval("a: 2 ## Assign 2 to a\n"
								   "\tb: .a + 3 ## Assign b to a + 3\n"
								   "\tb + a", 7);							// 1
					t.require_eval("a: 2\n"
								   "\tb: 3 + .a\n"
								   "## Assign b to 3 + a\n"
								   "\tb + a", 7);							// 2
				t.close_sub_test();
			t.close_sub_test();

			t.init_sub_test("Table Testing");
				t.eval("a: [ 1 ]\nb: 1");
				t.require_eval("a.1", 1);
				t.require_true("a[1] = a[b]");

				t.eval("a.a: [ a: 3 ]");
				t.require_eval("a", "[ 1, a: [ a: 3 ] ]");
				t.require_eval("a.a", "[ a: 3 ]");
				t.require_eval("a.a.b: 2", 2);
				t.require_eval("a.a", "[ a: 3, b: 2 ]");

				t.init_sub_test("Table Operators");
					t.eval("a: [ 1 2 3 2 5 5 4 ]");
					t.eval("b: [ 1 3 ]");
					t.eval("c: 5");
					t.require_true("(a ^ b) = b");
					t.require_eval("a ^ c", "[ 5, 5 ]");
					t.require_eval("b + c", "[ 1, 3, 5 ]");
					t.require_eval("a - b", "[ 2, 2, 5, 5, 4 ]");
					t.require_eval("b * c", "[ 1, 3, 5 ]");
				t.close_sub_test();

				t.require_eval("a[b[2]]", 3);
				t.require_eval("a[b[2] * 2]", 5);

			t.close_sub_test();
			//t.init_sub_test("API Testing");
			//t.close_sub_test();

			//t.end_tests();

			t.print_review(std::cout);
		}

		std::string makeReview(const std::string& buf, const std::string& test, int np, int nt) {
			std::stringstream ss;

			ss << buf << ":: " << test << " | Passed: " << np
									   << " | Failed: " << (nt - np)
									   << " | " << np << " / " << nt
									   << " Tests (" << std::setprecision(4) << ((float)np / nt * 100) << "%)\n";

			return ss.str();
		}

	}
}