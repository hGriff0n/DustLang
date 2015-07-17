#include "Calculator.h"
//#include "test.h"

#include <pegtl/analyze.hh>

template <typename T> void print(std::shared_ptr<T>&);
template <typename T> void print(std::shared_ptr<T>&, std::string);

int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `calculator::grammar`....." << std::endl;
	pegtl::analyze<calculator::grammar>();		// Analyzes the grammar
	std::cout << "\n\n";

	calculator::AST parse_tree;
	EvalState state;

	state.reg_func("_op+", [](int l, int r) { return l + r; });
	state.reg_func("_op*", [](int l, int r) { return l * r; });
	state.reg_func("_op-", [](int l, int r) { return l - r; });
	state.reg_func("_op/", [](int l, int r) { return l / r; });
	state.reg_func("_op=", [](int l, int r) { return l == r; });
	state.reg_func("_op<", [](int l, int r) { return l < r; });
	state.reg_func("_op>", [](int l, int r) { return l > r; });
	state.reg_func("_op^", [](int l, int r) { return (int)pow((float)l, (float)r); });
	state.reg_func("_op%", [](int l, int r) { return l % r; });

	// Switch statement inside eval (quicker to implement, each case does the call)				// Modifying storage is going to be difficult for both (different signatures)
	// or autoLua-type metaprogramming (this will take longer but is more extensible)
	// or have lua-style calling convention (I can append to autoLua metaprogramming wrapper easily on top of this)

	//state.reg_func("_op+", 2, [](int l, int r) { return l + r; });							// Redefine state to store the num_args
	//state.reg_func("_ou-", 1, [](int r) { return -r; });
	//state.reg_func("_ou!", 1, [](int r) { return !r; });

	// What's the purpose of the return statement then??
	//state.reg_func("_op+", [](EvalState& s) { s.push(s.pop() + s.pop()); return 1; });		// How many values were pushed on the stack
	//state.reg_func("_ou-", [](EvalState& s) { s.push(-s.pop()); return 1; });

	std::string input;

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {									// see pegtl/sum.cc for "string" parse ???
		try {
			pegtl::parse<calculator::grammar, calculator::action>(input, input, parse_tree);	// Still don't know the entire point of the second argument

			//while (!parse_tree.empty())
			//	print(node(parse_tree));

			print(parse_tree.top());
			evaluate(node(parse_tree), state);
			std::cout << ":: " << state.pop() << std::endl;
		} catch (pegtl::parse_error& e) {
			std::cout << e.what() << std::endl;

			clear(parse_tree);
		}

		std::cout << "> ";
	}

}

template <typename T>
void print(std::shared_ptr<T>& ast) {
	print(ast, "|");
	//std::cout << std::endl;			// slightly odd
}

template <typename T>
void print(std::shared_ptr<T>& ast, std::string buffer) {
	std::cout << buffer << "+- " << _typename(*ast) << " " << ast->to_string() << std::endl;
	buffer += " ";

	for (auto& n : *ast)
		print(n, buffer);
}