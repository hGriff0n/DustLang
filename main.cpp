#include "Interpreter\Control.h"
#include "Interpreter\Testing\Testing.h"

#include <iostream>
#include <pegtl/analyze.hh>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << "\n"
#define nl() pl("")


// Other Stuff and Pipe Dreams
	// Consider changing name of _op() due to semantical differences
	// Way of formatting float -> string conversion ???

// Do I need to protect other TypeSystem methods from indexing with NIL ???

using namespace dust;

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input (allows multiline repl testing)
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

constexpr bool show_all_tests = 4 % 2;

int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << std::endl;

	parse::AST parse_tree;
	std::string input;

	EvalState e;

	initState(e);
	test::runTests(e, show_all_tests);

	std::cout << "\n> ";

	while (getmultiline(std::cin, input) && input != "exit") {
		if (input == "gc") {

		// Type checking (ala. Haskell)
		} else if (input.substr(0, 2) == ":t") {
			parse::ScopeTracker scp{};
			pegtl::parse<grammar, action, parse::control>(input.substr(3), input, parse_tree, scp);
			parse_tree.pop()->eval(e);

			std::cout << e.getTS().getName(e.pop().type_id) << "\n";

		// Parse checking
		} else {
			try {
				parse::ScopeTracker scp{};
				pegtl::parse<grammar, action, parse::control>(input, input, parse_tree, scp);

				if (!parse_tree.empty()) {
					printAST(std::cout, parse_tree.at());

					parse_tree.pop()->eval(e).stream(std::cout << ":: ") << "\n";
				}
				//e.eval(parse_tree.pop()).stream(std::cout << ":: ") << "\n";

				// Need to make a generic 'pop' here
				// Or I can insist that printable equates to String convertible
				// e >> input;		Define operator<< and operator>> for EvalState/Stack ???

			} catch (pegtl::parse_error& e) {
				std::cout << e.what() << std::endl;

			} catch (error::base& e) {
				std::cout << e.what() << std::endl;
			}
		}

		parse_tree.clear();
		std::cout << "\n> ";
	}

	return 0;
}
