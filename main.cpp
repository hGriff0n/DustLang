#include "Interpreter\Control.h"
#include "Interpreter\Testing\Testing.h"

#include <iostream>
#include <pegtl/analyze.hh>

#include <fstream>

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

	e.push("error");
	e.push([](EvalState& e) {
		e.push(e.size());				// Since I don't take the 4 off the stack, loc = 1 and ret_idx = 2
		return 1;
	});
	e.set(EvalState::SCOPE);

	e.push("test");
	e.push([](EvalState& e) {
		e.push(3);
		e.push(4);
		e.push("error");
		e.get(EvalState::SCOPE);
		e.call(1);
		return 1;
	});
	e.set(EvalState::SCOPE);

	// Main repl loop
	std::cout << "\n> ";

	// repl loop
	while (getmultiline(std::cin, input) && input != "exit") {

		// Run the garbage collector
		if (input == ":gc") {
			e.getGC().run();

		// Type checking (ala. GHCI)
		} else if (input.substr(0, 2) == ":t") {
			parse::ScopeTracker scp{};
			pegtl::parse<grammar, action, parse::control>(input.substr(3), input, parse_tree, scp);
			parse_tree.pop()->eval(e);

			std::cout << e.getTS().getName(e.pop().type_id) << "\n";

		// Run file (basic implementation)
		} else if (input.substr(0, 2) == ":r") {
			std::string file = input.substr(3);
			if (file.size() < 5 || file.compare(file.length() - 4, 4, ".dst")) file += ".dst";				// Append .dst if not provided

			std::cout << " Running file \"" << file << "\"\n";

			try {
				parse::ScopeTracker scp{};

				std::string input{ std::istreambuf_iterator<char>(std::ifstream{ file }), std::istreambuf_iterator<char>() };
				pegtl::parse<grammar, action, parse::control>(input, file, parse_tree, scp);
				//pegtl::file_parser{ file }.parse<grammar, action, parse::control>(parse_tree, scp);

				if (!parse_tree.empty())
					parse_tree.pop()->eval(e).stream(std::cout << ":: ") << "\n";

			} catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}

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
