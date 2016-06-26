#include "Interpreter\Control.h"
#include "Interpreter\Testing\Testing.h"

#include <iostream>
#include <pegtl/analyze.hh>


// Other Stuff and Pipe Dreams
	// Consider changing name of _op() due to semantical differences
	// Way of formatting float -> string conversion ???

// Do I need to protect other TypeSystem methods from indexing with NIL ???

using namespace dust;

// Wrapper around std::getline that waits for [ENTER] to be hit twice before accepting input (to simplify multiline repl testing)
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

// Find the path to the given dust file
	// Currently only ensures that the file has the '.dst' extension
std::string&& findDustFile(const std::string& file) {
	return std::move("Dust\\" + file + ((file.size() < 5 || file.compare(file.length() - 4, 4, ".dst")) ? ".dst" : ""));
}

int main(int argc, const char* argv[]) {
	/*
	 * Analyze the grammar for errors using pegtl's provided functionality
	 *   I'm unsure if this works currently
	 */
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << std::endl;


	/*
	 * Run CATCH automated testing (change to use enums)
	 *  0 = Default testing
	 *  1 = Debug when an exception is thrown
	 *  2 = Output to junit
	 *  3 = Show all tests
	 *  4 = Test backing structures
	 *  5 = No testing
	 */
	auto res = test::runTests(0);
	std::cout << "\nAutomated testing complete....\n\t" << res << " errors were found\n\n";


	/*
	 * Setup and start the main evaluation loop
	 */
	EvalState e;
	parse::AST parse_tree;
	std::string input;

	initState(e);
	std::cout << "\n> ";
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
			auto file = findDustFile(input.substr(3));

			std::cout << " Running file \"" << file << "\"\n";

			try {
				parse::ScopeTracker scp{};
				pegtl::file_parser{ file }.parse<grammar, action, parse::control>(parse_tree, scp);

				if (!parse_tree.empty())
					parse_tree.pop()->eval(e).stream(std::cout << ":: ") << "\n";

			} catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}

		// Show file (basic implementation)
		} else if (input.substr(0, 2) == ":l") {
			auto file = findDustFile(input.substr(3));

			try {
				std::cout << " Reading file \"" << file << "\"\n\n" << pegtl::internal::file_reader{ file }.read() << "\n";
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

					parse_tree.pop()->eval(e);
					e.stream(std::cout << ":: ") << "\n";
				}
				//e.eval(parse_tree.pop()).stream(std::cout << ":: ") << "\n";

			} catch (pegtl::parse_error& e) {
				std::cout << e.what() << std::endl;

			} catch (error::base& e) {
				std::cout << e.what() << std::endl;
			}
		}

		// Reset for the next line
		parse_tree.clear();
		std::cout << "\n> ";
	}

	return 0;
}
