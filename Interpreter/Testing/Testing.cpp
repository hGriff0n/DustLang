#include "Testing.h"

namespace dust {
	namespace test {
		void run_tests(EvalState& e) {
			TestOrganizer<decltype(std::cout)> t{ e, std::cout };

			// Literals and Typing tests
			t.init_sub_test("Literals and Type System");
				t.require_eval("3", 3);										// 1
				t.require_type("3", "Int");									// 2

				t.require_eval("\"3\"", "3");								// 3
				t.require_type("\"3\"", "String");							// 4

				t.require_eval("3.3", 3.3);									// 5
				t.require_type("3.3", "Float");								// 6

				t.require_eval("true", true);								// 7
				t.require_type("true", "Bool");								// 8
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

				t.require_eval("a, b: 3", 0);								// 9
				t.require_true("a = 3 and b = 0");							// 10

				// Compound Operations
				t.init_sub_test("Compound Assignment");
					t.require_eval("a, b:+ 2, 2", 2);						// 1
					t.require_true("a = 5 and b = 2");						// 2

					t.require_eval("a, b:* 2", 0);							// 3
					t.require_true("a = 10 and b = 0");						// 4

					t.require_eval("a:= (b: 3) * 2 + 2 ^ 2", true);			// 5
					t.require_true("a and b = 3");							// 6
				t.close_sub_test();
			t.close_sub_test();

			// Test boolean ternary statement
			t.init_sub_test("Boolean Ternary");
				t.require_eval("true and false or true", "true");			// 1		# Counter-intuitive. But this is how lua's ternary works
				t.require_eval("c = 0 and 4 or 5", 4);						// 2

				t.require_excep<pegtl::parse_error>("false and a: 5");		// 3
				t.require_eval("false and (a: 5)", false);					// 4
				t.require_true("a = true");									// 5		# a: 5 is not evaluated

				t.require_eval("a: b or 5", 3);								// 6
				t.require_true("a = 3");									// 7

				// 0 => true. Should I keep this?
				t.require_eval("b: c != 0 and 4 or (c: 5)", 5);				// 8
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

				t.eval("a:2");
				t.require_eval("(a: 3) + 3 * a", 12);						// 8
				t.require_true("a = 3");									// 9
			t.close_sub_test();

			t.init_sub_test("Multiline Parsing");				// Need to escape output
				t.require_eval("a\n+ b", 7);								// 1
				t.require_excep<pegtl::parse_error>("3 + a: 3\n - 4");
				t.require_eval("3 + a 3\n - 4", -1);
			t.close_sub_test();

			std::cout << "Passed: " << t.num_pass << " | Failed: " << (t.num_tests - t.num_pass) << " | " << t.num_pass << " / " << t.num_tests << " Tests (" << std::setprecision(4) << ((float)t.num_pass / t.num_tests * 100) << "%)\n\n";


		}

	}
}