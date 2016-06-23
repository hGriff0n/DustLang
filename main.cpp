#include "Interpreter\Control.h"

#include <iostream>
#include <pegtl/analyze.hh>

// Automated Testing Framework
#define CATCH_CONFIG_RUNNER
#ifdef CATCH_CONFIG_RUNNER
#include "Interpreter\libs\catch.hpp"
#include "Interpreter\Testing\Testing.h"
#endif


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

int main(int argc, const char* argv[]) {
	/*
	 * Analyze the grammar for errors using pegtl's provided functionality
	 *   I'm unsure if this works currently
	 */
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << std::endl;


#ifdef CATCH_CONFIG_RUNNER
	/*
	 * Run Catch testing in declared order
	 *  Outputs to junit format (Use junit-viewer to "view" the results)
	 */

	// Uncomment to start debugging when an exception is thrown
	//auto res = Catch::Session().run(9, new char*[9]{ "DustTests", "--order", "decl", "-r", "junit", "-e", "-b", "--use-colour", "yes" });

	// Uncomment to ouput to a junit xml file
		// Is this only printing because I'm not specifying a place to print to?
	//auto res = Catch::Session().run(5, new char*[5]{ "DustTests", "--order", "decl", "-r", "junit" });

	// Uncomment to show all tests
	//auto res = Catch::Session().run(6, new char*[6]{ "DustTests", "--order", "decl", "-s", "--use-colour", "yes" });

	// Uncomment if not using one of the other testing runs
	auto res = Catch::Session().run(5, new char*[5]{ "DustTests", "--order", "decl", "--use-colour", "yes" });

	std::cout << "\nAutomated testing found " << res << " errors\n\n";
#endif


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
			std::string file = "Dust\\" + input.substr(3);
			if (file.size() < 5 || file.compare(file.length() - 4, 4, ".dst")) file += ".dst";				// Append .dst if not provided

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
			std::string file = "Dust\\" + input.substr(3);
			if (file.size() < 5 || file.compare(file.length() - 4, 4, ".dst")) file += ".dst";				// Append .dst if not provided

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
