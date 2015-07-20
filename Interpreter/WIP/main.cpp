#include "Calculator.h"

#include <pegtl/analyze.hh>

template <typename T> void print(std::shared_ptr<T>&);
template <typename T> void print(std::shared_ptr<T>&, std::string);

int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `calculator::grammar`....." << std::endl;
	pegtl::analyze<calculator::grammar>();		// Analyzes the grammar
	std::cout << "\n\n";

	calculator::AST parse_tree;
	EvalState state;

	// Switch statement inside eval (quicker to implement, each case does the call)				// Modifying storage is going to be difficult for both (different signatures)
	// or autoLua-type metaprogramming (this will take longer but is more extensible)
	// or have lua-style calling convention (I can append to autoLua metaprogramming wrapper easily on top of this)

	// What's the purpose of the return statement then?? (Currently how many values were pushed on the stack)
	state.reg_func("_op+", [](EvalState& s) { s.push(s.pop() + s.pop()); return 1; });											// Relies on arguments being evaluated right->left
	state.reg_func("_op*", [](EvalState& s) { s.push(s.pop() * s.pop()); return 1; });
	state.reg_func("_op-", [](EvalState& s) { s.push(s.pop() - s.pop()); return 1; });
	state.reg_func("_op/", [](EvalState& s) { s.push(s.pop()/s.pop()); return 1; });
	state.reg_func("_op=", [](EvalState& s) { s.push(s.pop() == s.pop()); return 1; });
	state.reg_func("_op<", [](EvalState& s) { s.push(s.pop() < s.pop()); return 1; });
	state.reg_func("_op>", [](EvalState& s) { s.push(s.pop() > s.pop()); return 1; });
	state.reg_func("_op^", [](EvalState& s) { float base = s.pop(); s.push((int)pow(base, (float)s.pop())); return 1; });
	state.reg_func("_op<=", [](EvalState& s) { s.push(s.pop() <= s.pop()); return 1; });
	state.reg_func("_op>=", [](EvalState& s) { s.push(s.pop() >= s.pop()); return 1; });
	state.reg_func("_op!=", [](EvalState& s) { s.push(s.pop() != s.pop()); return 1; });
	state.reg_func("_op%", [](EvalState& s) { s.push(s.pop() % s.pop()); return 1; });
	state.reg_func("_ou-", [](EvalState& s) { s.push(-s.pop()); return 1; });
	state.reg_func("_ou!", [](EvalState& s) { s.push(!s.pop()); return 1; });

	std::string input;

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {									// see pegtl/sum.cc for "string" parse ???
		try {
			pegtl::parse<calculator::grammar, calculator::action>(input, input, parse_tree);	// Still don't know the entire point of the second argument

			print(parse_tree.top());
			evaluate(parse_tree.pop(), state);
			std::cout << ":: " << state.pop() << std::endl;
		} catch (pegtl::parse_error& e) {
			std::cout << e.what() << std::endl;

			clear(parse_tree);
		}

		std::cout << "> ";
	}

	return 0;
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