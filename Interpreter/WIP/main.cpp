#include "Calculator.h"

#include <pegtl/analyze.hh>

template <typename T> void print(std::shared_ptr<T>&);
template <typename T> void print(std::shared_ptr<T>&, std::string);

int main(int argc, const char* argv[]) {
	using namespace std;
	// Test generic typing

	vector<DustObj> generics;
	generics.push_back(makeObj(3));
	generics.push_back(makeObj(3.3));
	generics.push_back(makeObj(true));

	cout << "::Test makeObj correctness::\n";
	cout << _type(generics[0].type) << " " << (int)generics[0] << endl;
	cout << _type(generics[1].type) << " " << (double)generics[1] << endl;
	cout << _type(generics[2].type) << " " << (bool)generics[2] << endl;

	cout << "\n::Test DustObj conversions::\n";
	double g0 = generics[0];
	int g1 = generics[1];
	double g2 = generics[2];
	cout << _type(generics[1].type) << " " << (double)generics[1] << " -> INT " << g1 << endl;
	cout << _type(generics[0].type) << " " << (int)generics[0] << " -> FLOAT " << g0 << endl;
	cout << _type(generics[2].type) << " " << (bool)generics[2] << " -> FLOAT " << g2 << endl;

	cout << "\n::Test DustObj reassignment::\n";
	generics[0] = 5.6;
	generics[1] = false;
	generics[2] = -2;
	cout << _type(generics[0].type) << " " << (double)generics[0] << endl;
	cout << _type(generics[1].type) << " " << (bool)generics[1] << endl;
	cout << _type(generics[2].type) << " " << (int)generics[2] << endl;

	cout << "\n::Test DustObj recasting::\n";
	recast<int>(generics[0]);
	recast<double>(generics[1]);
	recast<bool>(generics[2]);
	cout << _type(generics[0].type) << " " << (int)generics[0] << endl;
	cout << _type(generics[1].type) << " " << (double)generics[1] << endl;
	cout << _type(generics[2].type) << " " << (bool)generics[2] << endl;

	cin.get();
}

int tmp_old_main(int argc, const char* argv[]) {
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
	state.reg_func("_op%", [](EvalState& s) { s.push(s.pop() % s.pop()); return 1; });			// Not an "official" operator
	state.reg_func("_ou-", [](EvalState& s) { s.push(-s.pop()); return 1; });
	state.reg_func("_ou!", [](EvalState& s) { s.push(!s.pop()); return 1; });
	state.reg_func("print", [](EvalState& s) { std::cout << s.pop() << std::endl; return 0; });			// These functions need more "features" (print("H","e", "l","l","o" ) => H\ne\nl\nl\no\n

	std::string input;

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {									// see pegtl/sum.cc for "string" parse ???
		try {
			if (input == "clear")
				system("cls");
			else if (input == "pop")
				std::cout << "::" << state.pop() << std::endl;
			else {
				pegtl::parse<calculator::grammar, calculator::action>(input, input, parse_tree);	// Still don't know the entire point of the second argument

				//while (!parse_tree.empty())
				//	print(parse_tree.pop());

				//*/
				print(parse_tree.top());
				evaluate(parse_tree.pop(), state);
				std::cout << ":: " << state.pop() << std::endl;
				//*/
			}
		} catch (pegtl::parse_error& e) {
			std::cout << e.what() << std::endl;

			clear(parse_tree);
		} // catch stack exceptions

		std::cout << "> ";
	}

	return 0;
}

template <typename T>
void print(std::shared_ptr<T>& ast) {
	print(ast, "|");
}

template <typename T>
void print(std::shared_ptr<T>& ast, std::string buffer) {
	std::cout << buffer << "+- " << _typename(*ast) << " " << ast->to_string() << std::endl;
	buffer += " ";

	for (auto& n : *ast)
		print(n, buffer);
}