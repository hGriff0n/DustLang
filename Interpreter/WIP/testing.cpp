#include "Type.h"
#include <vector>
#include <iostream>

// Current testing devoted to
	// Formalizing the Type System and Inheritance/Common Type Frameworks
	// Systems for selecting the correct types and functions for program semantics

// Nail the semantics and syntax
	// Tracking and maintaining inheritance trees
	// Tracking and maintaining conversion functions (I might not be able to do this without expanding the api a bit)
	// Selecting conversion functions based on precedence
	// Selecting correct type based on function in inheritance situations
	// Selecting correct type based on function in common type situations as well

// Work on formulating the API

class dust::EvalState {
	private:
		std::map<std::string, int> type_id;
		std::vector<impl::Type> types;

	public:
		struct Type {
			unsigned int id;
			EvalState* e;

			Type(unsigned int i, EvalState* s) : id{ i }, e{ s } {}

			Type& addOp(std::string s, const Function& f) {
				e->types[id].operations[s] = f;
				return *this;
			}

			Type& addParent(Type& t) {
				return *this;
			}
		};

		Type newType() {
			types.emplace_back(types.size());

			return { types.size() - 1, this };
		}

		impl::Type getType(std::string t) {
			return types[type_id[t]];
		}
};

int main(int argc, const char* argv[]) {
	using namespace dust::impl;
	using namespace dust;
	using namespace std;

	EvalState e;

	/*
	"Global" structs that will eventually be collected within EvalState
	*/


	/*
	Type declarations
	*/
	Type Object{ "Object", 0 };

	Type Number{ "Number", 1 };

	Type Int{ "Int", 2 };
	Int.operations["_op+"] = [](EvalState& e) { return 1; };
	
	Type Float{ "Float", 3 };
	Float.operations["_op+"] = [](EvalState& e) { return 2; };

	Type String{ "String", 4 };
	String.operations["_op+"] = [](EvalState& e) { return 3; };

	/*
	Conversion declarations
	*/


	/*
	Testing
	*/
	cout << String.operations["_op+"](e);

	cin.get();
}

/*
std::map<std::string, int> type_id;
std::vector<impl::Type> types;
std::map<impl::Type, impl::Type> converts;
std::map<int, std::vector<int>> inheritance;

impl::Type type(int id) { return types[id]; }
impl::Type type(EvalState::Type t) { return types[t.id]; }
impl::Type type(std::string t) { return types[getId(t)]; }
EvalState::Type getType(int id) { return impl::StateType{ id, this }; }
EvalState::Type getType(std::string t) { return impl::StateType{ getId(t), this }; }

EvalState::Type newType(std::string t) {
type_id[t] = types.emplace_back(t, types.size()).id;
	return Type{ getId(t), this };
}
int getId(std::string t) { return type_id[t]; }

namespace std {
	template<> struct less<impl::Type> {
		bool operator() (const impl::Type& lhs,	const impl::Type& rhs) {
			return lhs.id < rhs.id;
		}
	};
}
*/