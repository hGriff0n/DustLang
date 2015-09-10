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

// 

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
	std::cout << "\n" << std::endl;

	parse::AST parse_tree;
	std::string input;
	bool isResString;
	
	EvalState e;
	initState(e);

	test::Tester t{ e };

	t.require_eval("a: 2.2", 2.2);


	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {
		if (input == "gc") {
		
		} else {
			try {
				pl(input);
				pegtl::parse<grammar, action>(input, input, parse_tree);

				print(std::cout, parse_tree.at());
				isResString = parse_tree.pop()->eval(e).is<std::string>();

				// Need to make a generic 'pop' here
					// Or I can insist that printable equates to String convertible
				std::cout << ":: "
					<< (isResString ? "\"" : "")
					<< (std::string)e
					<< (isResString ? "\"" : "")
					<< "\n" << std::endl;

			} catch (pegtl::parse_error& e) {
				std::cout << e.what() << std::endl;
			} catch (error::base& e) {
				std::cout << e.what() << std::endl;
			}

			parse_tree.clear();
		}
		std::cout << "> ";
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
	Tester t{ e };

	t.require_eval("3", "3");
}