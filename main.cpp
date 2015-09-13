#include "Interpreter\Actions.h"

#include "Interpreter\Testing\Testing.h"
#include <iostream>

#include <pegtl/analyze.hh>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << "\n"
#define nl() pl("")

// TODO:		
// Update the PEGTL library if possible

// Things to work on
	// Improving and consolidating the API
	// Improving and updating documentation

// Other Stuff and Pipe Dreams
	// Generalize Storage and move the Garbage Collecter to "targeting" Storage (instead of inheriting)
		// Strings would have a different RuntimeStorage instance than tables, userdata, etc. (though most of the functions can be reused)
		// Perform these changes at the same time if I perform them at all
			// Generalizing the Garbage Collecter to "target" storage does not exactly require generalizing Storage however
	// Consider changing name of _op() due to semantical differences
	// Consider adding a push_ref method to CallStack (roughly mirrors pop_ref)
	// Consider specializing the control template argument (see PEGTL for more)
		// This would give me greater control over error messages and throwing from the parser stage
	// Way of formatting float -> string conversion ???

// Do I need to protect other TypeSystem methods from indexing with NIL

using namespace dust;

// Assign a Value
template <typename T>
void assign_value(impl::Value&, T, impl::GC&);
void assign_value(impl::Value&, impl::Value&, impl::GC&);


template <class ostream>
void print(ostream& s, std::shared_ptr<parse::ASTNode>& ast) {
	(s << ast->print_string("|")).flush();
}


int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << std::endl;

	parse::AST parse_tree;
	std::string input;
	bool isResString;

	EvalState e;
	initState(e);

	test::run_tests(e);

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {
		if (input == "gc") {

		} else {
			try {
				pegtl::parse<grammar, action>(input, input, parse_tree);

				print(std::cout, parse_tree.at());
				isResString = parse_tree.pop()->eval(e).is<std::string>();
				//isResString = e.eval(parse_tree.pop()).is<std::string>();

				// Need to make a generic 'pop' here
				// Or I can insist that printable equates to String convertible
				std::cout << ":: "
					<< (isResString ? "\"" : "")
					<< (std::string)e
					<< (isResString ? "\"" : "")
					<< "\n";

			} catch (pegtl::parse_error& e) {
				std::cout << e.what() << std::endl;
			} catch (error::base& e) {
				std::cout << e.what() << std::endl;
			}

			parse_tree.clear();
		}
		std::cout << "\n> ";
	}
}

template <typename T>
void assign_value(impl::Value& v, T val, impl::GC& gc) {
	if (v.type_id == type::Traits<std::string>::id) gc.decRef(v.val.i);

	v = make_value(val, gc);
}

void assign_value(impl::Value& v, impl::Value& a, impl::GC& gc) {
	if (v.type_id == type::Traits<std::string>::id) gc.decRef(v.val.i);
	if (a.type_id == type::Traits<std::string>::id) gc.incRef(a.val.i);

	v = a;
}

void dust::test::run_tests(EvalState& e) {
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
		t.require_true("a = 1");
		t.require_true("b = 2");
		//t.require_true("a + b = 3");
		//t.require_true("a = 1 and b = 2");

		t.require_eval("a, b: b, a", 1);
		t.require_true("a = 2");
		t.require_true("b = 1");

		t.require_eval("a, b: 1, 2, 3", 2);
		t.require_true("a = 1");
		t.require_true("b = 2");

		t.require_eval("a, b: 3", 0);
		t.require_true("a = 3");
		t.require_true("b = 0");

	// Compound Operations
		t.init_sub_test("Compound Assignment");
			t.require_eval("a, b:+ 2, 2", 2);
			t.require_true("a = 5");
			t.require_true("b = 2");

			t.require_eval("a, b:* 2", 0);
			t.require_true("a = 10");
			t.require_true("b = 0");

			t.require_eval("a:= (b: 3) * 2 + 2 ^ 2", true);
			t.require_true("a");
			t.require_true("b = 3");
		t.close_sub_test();
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

	std::cout << "Passed: " << t.num_pass << " | Failed: " << (t.num_tests - t.num_pass) << " | " << t.num_pass << " / " << t.num_tests << " Tests (" << std::setprecision(4) << ((float)t.num_pass / t.num_tests * 100) << "%)\n\n";

}