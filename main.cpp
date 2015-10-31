
#include "Interpreter\Actions.h"
#include "Interpreter\Testing\Testing.h"

#include <iostream>
#include <pegtl/analyze.hh>

#include "Interpreter\DualGC.h"

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

template <class ostream>
void print(ostream& s, std::shared_ptr<parse::ASTNode>& ast) {
	(s << ast->print_string("|")).flush();
}

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input
template <class istream>
istream& getmultiline(istream& in, std::string& s) {
	std::getline(in, s);
	if (s == "exit") return in;

	std::string tmp{};
	while (std::getline(in, tmp)) {
		if (tmp == "") return in;

		s += "\n" + tmp;
	}

	return in;
}

int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << std::endl;

	parse::AST parse_tree;
	std::string input;
	EvalState e;

	initState(e);
	test::run_tests(e);

	std::cout << "> ";

	// a: [ b : 3 ] => Table ## That table has 1 member, b = 3
	// a.b => Nil ???
	while (getmultiline(std::cin, input) && input != "exit") {
		if (input == "gc") {

		} else {
			try {
				pegtl::parse<grammar, action>(parse::trim(input), input, parse_tree, 0);

				print(std::cout, parse_tree.at());

				parse_tree.pop()->eval(e).stream(std::cout << ":: ") << "\n";
				//e.eval(parse_tree.pop()).stream(std::cout << ":: ") << "\n";

				// Need to make a generic 'pop' here
				// Or I can insist that printable equates to String convertible
				// e >> input;		Define operator<< and operator>> for EvalState/Stack ???

			} catch (pegtl::parse_error& e) {
				std::cout << e.what() << std::endl;

			} catch (error::base& e) {
				std::cout << e.what() << std::endl;
			}

			parse_tree.clear();
		}
		std::cout << "\n> ";
	}

	return 0;
}
