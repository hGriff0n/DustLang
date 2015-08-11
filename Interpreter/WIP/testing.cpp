#include "Type.h"
#include <vector>
#include <array>
#include <iostream>

#define p(x) std::cout << x
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << std::endl

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
			size_t id;
			EvalState* e;

			Type(size_t i, EvalState* s) : id{ i }, e{ s } {}

			Type& addOp(std::string s, const Function& f) {
				e->types[id].ops[s] = f;
				return *this;
			}

			impl::Type operator*() { return e->types[id]; }
		};

		Type newType(std::string t) {
			return newType(t, types.front());
		}

		Type newType(std::string t, impl::Type p) {
			types.emplace_back(t, types.size(), p);

			return{ types.size() - 1, this };
		}

		Type newType(std::string t, Type p) {
			types.emplace_back(t, types.size(), p.id);

			return{ types.size() - 1, this };
		}

		impl::Type getType(std::string t) {
			return types[type_id[t]];
		}

		int dispatch(impl::Type t, std::string op) {
			while (t.ops.count(op) == 0)
				t = t.id > 0 ? types[t.parent] : throw std::string{ "Dispatch error" };

			return 0;
			//return t.ops[op](*this);					// I use this (similar) code in the current production !!!!!
		}

		impl::Type dispatch_(impl::Type t, std::string op) {
			while (t.ops.count(op) == 0)
				t = t.id > 0 ? types[t.parent] : throw std::string{ "Dispatch error" };

			return t;
		}
};

dust::impl::Type lub(dust::impl::Type, dust::impl::Type /*, CONVER */);
int dispatch(dust::impl::Type, std::string, std::vector<dust::impl::Type>&, dust::EvalState&);
void setConvert(dust::impl::Type, dust::impl::Type /*, CONVER */);

int main(int argc, const char* argv[]) {
	using namespace dust::impl;
	using namespace dust;

	EvalState e;

	/*
	"Global" structs that will eventually be collected within EvalState
	*/
	std::vector<Type> types;
	dust::impl::ConvTracker convs;

	/*
	Type declarations
	*/
	types.emplace_back("Object", types.size());
	auto Object = types.front();

	Type Number{ "Number", types.size(), Object };
	Number.ops["_op*"] = [](EvalState& e) { return 1; };
	types.push_back(Number);

	Type Int{ "Int", types.size(), Number };
	Int.ops["_op+"] = [](EvalState& e) { return 2; };
	types.push_back(Int);

	Type Float{ "Float", types.size(), Number };
	Float.ops["_op+"] = [](EvalState& e) { return 3; };
	types.push_back(Float);

	Type String{ "String", types.size(), Object };
	String.ops["_op+"] = [](EvalState& e) { return 4; };
	types.push_back(String);

	// Ideal api (Type names act as aliases)
	//auto Number = e.newType("Number");
	//Number.addOp("_op*", [](EvalState& e) { return 1; });

	//auto Int = e.newType("Int", Number);			// Have the API functions prevent a type from being a parent to itself
	//Int.addOp("_op+", [](EvalState& e) { return 2; });
	
	//auto Float = e.newType("Float", Number);
	//Float.addOp("_op+", [](EvalState& e) { return 3; });

	//auto String = e.newType("String");
	//String.addOp("_op+", [](EvalState& e) { return 4; });

	/*
	Conversion declarations
	*/

	convs.add(Int, Float);
	convs.add(Int, String);
	convs.add(Float, Int);
	// convs.add(Number, String);			// Replace conversion of Int -> String (Will lub(Int, String) = String ???)

	/*
	Testing
	*/
	try {
		// Testing common type
		//ps("lub(String, Int)._op+");
		//pl(dispatch(lub(String, Int), "_op+", types, e));		// Should call String._op+
		//ps("lub(Int, String)._op+");
		//pl(dispatch(lub(Int, String), "_op+", types, e));		// Should call String._op+

		// Testing Inheritance
		ps("Int._op*             ");
		pl(dispatch(Int, "_op*", types, e));					// Should call Number._op*
		ps("String._op*          ");
		pl(dispatch(String, "_op*", types, e));					// Dispatch error




	} catch (std::string& e) {
		pl("\n" + e);
	}


	std::cin.get();
}

void setConvert(dust::impl::Type from, dust::impl::Type to /*, CONVER */) {

}

dust::impl::Type lub(dust::impl::Type l, dust::impl::Type r /*, CONVER */) {
	return l;
}

// Very basic inheritance relation
int dispatch(dust::impl::Type t, std::string op, std::vector<dust::impl::Type>& types, dust::EvalState& e) {
	while (t.ops.count(op) == 0)
		t = t.id > 0 ? types[t.parent] : throw std::string{ "Dispatch error" };

	ps(t.name + "." + op);

	return t.ops[op](e);
}

/*
std::map<impl::Type, impl::Type> converts;

impl::Type type(int id) { return types[id]; }
impl::Type type(EvalState::Type t) { return types[t.id]; }
impl::Type type(std::string t) { return types[getId(t)]; }
EvalState::Type getType(int id) { return impl::StateType{ id, this }; }
EvalState::Type getType(std::string t) { return impl::StateType{ getId(t), this }; }

int getId(std::string t) { return type_id[t]; }

namespace std {
	template<> struct less<impl::Type> {
		bool operator() (const impl::Type& lhs,	const impl::Type& rhs) {
			return lhs.id < rhs.id;
		}
	};
}
*/