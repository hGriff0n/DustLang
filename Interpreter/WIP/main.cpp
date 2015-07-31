#include "Calculator.h"

#include <pegtl/analyze.hh>

template <typename T> void print(std::shared_ptr<T>&);
template <typename T> void print(std::shared_ptr<T>&, std::string);

//int main(int argc, const char* argv[]) {
int t_main(int argc, const char* argv[]) {
	using namespace std;
	// Test generic typing

	vector<DustObj> generics;
	generics.push_back(makeObj(3));
	generics.push_back(makeObj(3.3));
	generics.push_back(makeObj(true));

	cout << "::Test makeObj correctness::\n";
	cout << _type(generics[0].type) << " " << generics[0] << endl;
	cout << _type(generics[1].type) << " " << generics[1] << endl;
	cout << _type(generics[2].type) << " " << generics[2] << endl;

	cout << "\n::Test DustObj conversions::\n";
	double g0 = (double)generics[0];
	int g1 = (int)generics[1];
	double g2 = (double)generics[2];
	cout << _type(generics[1].type) << " " << generics[1] << " -> INT " << g1 << endl;
	cout << _type(generics[0].type) << " " << generics[0] << " -> FLOAT " << g0 << endl;
	cout << _type(generics[2].type) << " " << generics[2] << " -> FLOAT " << g2 << endl;

	cout << "\n::Test DustObj reassignment::\n";
	generics[0] = 5.6;
	generics[1] = false;
	generics[2] = -2;
	cout << _type(generics[0].type) << " " << generics[0] << endl;
	cout << _type(generics[1].type) << " " << generics[1] << endl;
	cout << _type(generics[2].type) << " " << generics[2] << endl;

	cout << "\n::Test DustObj recasting::\n";
	recast<int>(generics[0]);
	recast<double>(generics[1]);
	recast<bool>(generics[2]);
	cout << _type(generics[0].type) << " " << generics[0] << endl;
	cout << _type(generics[1].type) << " " << generics[1] << endl;
	cout << _type(generics[2].type) << " " << generics[2] << endl;

	cout << "\n::Test DustObj constants::\n";
	generics[1] = 5.5;
	generics[1].let = true;
	generics[1] = 3.3;
	cout << _type(generics[1].type) << " " << generics[1] << endl;

	cout << "\n::Test DustObj let typing::\n";
	generics[0] = 5;
	generics[0].typed = true;
	generics[0] = 3.3;
	cout << _type(generics[0].type) << " " << generics[0] << endl;

	cin.get();
	return 0;
}

//int old_main(int argc, const char* argv[]) {
int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `calculator::grammar`....." << std::endl;
	pegtl::analyze<calculator::grammar>();		// Analyzes the grammar
	std::cout << "\n\n";

	calculator::AST parse_tree;
	EvalState state;

	// Switch statement inside eval (quicker to implement, each case does the call)				// Modifying storage is going to be difficult for both (different signatures)
	// or autoLua-type metaprogramming (this will take longer but is more extensible)
	// or have lua-style calling convention (I can append to autoLua metaprogramming wrapper easily on top of this)

	addOperators(state);
	std::string input;

	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {									// see pegtl/sum.cc for "string" parse ???
		try {
			if (input == "clear")
				system("cls");
			else if (input == "pop")
				std::cout << ":: " << state.pop() << std::endl;
			else if (input == "size")
				std::cout << ":: " << str_size() << std::endl;
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