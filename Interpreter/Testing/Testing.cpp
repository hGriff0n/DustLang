#include "Testing.h"

namespace dust {
	namespace test {
		void run_tests(EvalState& e) {
			TestOrganizer<decltype(std::cout)> t{ e, std::cout };

			// Literals and Typing tests
			t.init_sub_test("Literals and Type System");
			t.require_eval("3", 3);
			t.require_type("3", "Int");

			t.require_eval("\"3\"", "3");
			t.require_type("\"3\"", "String");

			t.require_eval("3.3", 3.3);
			t.require_type("3.3", "Float");

			t.require_eval("true", true);
			t.require_type("true", "Bool");
			t.close_sub_test();

			// Operator resolution tests
			t.init_sub_test("Operator Resolution");
			t.require_eval("3 + 3", 6);
			t.require_eval("\"The answer is \" + (6.3 ^ 2)", "The answer is 39.690000");
			t.require_error("3 + true");
			t.require_excep<error::dispatch_error>("3 + true");
			t.require_eval("\"4\" + 3", "43");
			t.require_eval("\"4\" - 3", 1);
			t.close_sub_test();

			// Test assignments
			t.init_sub_test("Assignment");
			t.require_eval("a: 2", 2);
			t.require_true("a = 2");

			t.require_eval("a, b: 1, 2", 2);
			t.require_true("a = 1 and b = 2");

			t.require_eval("a, b: b, a", 1);
			t.require_true("a = 2 and b = 1");

			t.require_eval("a, b: 1, 2, 3", 2);
			t.require_true("a = 1 and b = 2");

			t.require_eval("a, b: 3", 0);
			t.require_true("a = 3 and b = 0");

			// Compound Operations
			t.init_sub_test("Compound Assignment");
			t.require_eval("a, b:+ 2, 2", 2);
			t.require_true("a = 5 and b = 2");

			t.require_eval("a, b:* 2", 0);
			t.require_true("a = 10 and b = 0");

			t.require_eval("a:= (b: 3) * 2 + 2 ^ 2", true);
			t.require_true("a and b = 3");
			t.close_sub_test();
			t.close_sub_test();

			// Test boolean keywords
			t.init_sub_test("Boolean Keywords");
			t.require_eval("c = 0 and 4 or 5", 4);

			t.require_eval("false and a: 5", false);
			t.require_true("a = true");

			t.require_eval("a: b or 5", 3);
			t.require_true("a = 3");

			// 0 => true. Should I keep this?
			t.require_eval("b: c != 0 and 4 or (c: 5)", 5);
			t.require_true("b = 5 and c = b");

			t.require_eval("b: c and 4 or (c: 5)", 4);
			t.require_true("b = 4");
			t.close_sub_test();

			t.init_sub_test("Tricky Operations");
			t.require_eval("3 + (a: 4)", 7);
			t.require_true("a = 4");

			t.require_eval("3 + a: 3", 7);

			t.require_excep<error::missing_node_x>("a, b: 3, c: 4");
			t.require_eval("a, b: 3, (c: 4)", 4);
			t.require_true("a = 3");

			t.require_eval("3 +-3", 0);

			t.require_eval("(a: 3) + 3 * a", 12);
			t.require_true("a = 3");
			t.close_sub_test();

			t.init_sub_test("Multiline Parsing");
			t.require_eval("a\n+ b", 7);				// Need to escape output
			t.close_sub_test();

			std::cout << "Passed: " << t.num_pass << " | Failed: " << (t.num_tests - t.num_pass) << " | " << t.num_pass << " / " << t.num_tests << " Tests (" << std::setprecision(4) << ((float)t.num_pass / t.num_tests * 100) << "%)\n\n";


		}

	}
}