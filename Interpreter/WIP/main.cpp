#include "Calculator.h"

#include <pegtl/analyze.hh>

template <typename T> void print(std::shared_ptr<T>&);
template <typename T> void print(std::shared_ptr<T>&, std::string);

int main(int argc, const char* argv[]) {
	pegtl::analyze<calculator::grammar>();		// Analyzes the grammar

	// Move this stuff (condensed to the current working subset and some "todo" comments) onto github
	EvalState state;
	calculator::AST parse_tree;

	state.reg_func("_op+", [](int l, int r) { return l + r; });
	state.reg_func("_op*", [](int l, int r) { return l * r; });
	state.reg_func("_op-", [](int l, int r) { return l - r; });
	state.reg_func("_op/", [](int l, int r) { return l / r; });
	state.reg_func("_op=", [](int l, int r) { return l == r; });
	state.reg_func("_op<", [](int l, int r) { return l < r; });
	state.reg_func("_op>", [](int l, int r) { return l > r; });

	std::string input;

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {									// see pegtl/sum.cc for "string" parse ???
		try {
			pegtl::parse<calculator::grammar, calculator::action>(input, input, parse_tree);	// Still don't know the entire point of the second argument

			print(parse_tree.top());
			evaluate(node(parse_tree), state);

			// Could use some type inference here
			std::cout << ":: " << state.pop() << std::endl;
		} catch (pegtl::parse_error& e) {
			std::cout << e.what() << std::endl;				// Can use some work in "pretty"
			// <code>:<line>:<character>	parse error matching struct <grammar>

			clear(parse_tree);
		}

		std::cout << "> ";
	}

}


// Pretty-print the construct AST
template <typename T>
void print(std::shared_ptr<T>& ast) {
	print(ast, "|");
	std::cout << std::endl;			// slightly odd
}


// std::string lister = "+- ";
template <typename T>
void print(std::shared_ptr<T>& ast, std::string buffer) {
	std::cout << buffer << "+- " << _typename(*ast) << " " << ast->to_string() << std::endl;
	buffer += " ";

	for (auto& n : *ast)
		print(n, buffer);
}