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

// Things to work on
	// Possibly handling multiple inheritance
	// Add Inheritance considerations for lub operations (searches inheritance tree for conversion function)
	// Improving and consolidating the API

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

/*/
// Move type system functions into TypeSystem class
class dust::TypeSystem {
	private:
		std::vector<dust::impl::Type> typs;
		dust::impl::ConvTracker conv;

	public:
		struct Type {};

		Type newType(std::string);
		Type newType(std::string, impl::Type& p);
		Type newType(std::string, Type& p);
		Type newType(std::string, size_t);

		size_t lub(size_t, size_t, std::string);
		size_t lub(dust::impl::Type&, dust::impl::Type&, std::string);

		size_t findDef(size_t, std::string, std::vector<dust::impl::Type>&);
		size_t findDef(dust::impl::Type&, std::string, std::vector<dust::impl::Type>&);
};
//*/

size_t dispatch(dust::impl::Type&, std::string, std::vector<dust::impl::Type>&, dust::EvalState&);
size_t dispatch(size_t, std::string, std::vector<dust::impl::Type>&, dust::EvalState&);

size_t findDef(dust::impl::Type&, std::string, std::vector<dust::impl::Type>&);
size_t findDef(size_t, std::string, std::vector<dust::impl::Type>&);

size_t lub(dust::impl::Type&, dust::impl::Type&, std::string, dust::impl::ConvTracker&, std::vector<dust::impl::Type>&);

int main(int argc, const char* argv[]) {
	using namespace dust::impl;
	using namespace dust;

	EvalState e;

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	//TypeSystem ts;
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
	Float.ops["_op*"] = [](EvalState& e) { return 3; };
	types.push_back(Float);

	Type String{ "String", types.size(), Object };
	String.ops["_op+"] = [](EvalState& e) { return 4; };
	String.ops["_op/"] = [](EvalState& e) { return 4; };
	types.push_back(String);

	// Ideal api (Type names act as aliases)
	//auto Number = ts.newType("Number");
	//Number.addOp("_op*", [](EvalState& e) { return 1; });

	//auto Int = ts.newType("Int", Number);			// Have the API functions prevent a type from being a parent to itself
	//Int.addOp("_op+", [](EvalState& e) { return 2; });
	
	//auto Float = ts.newType("Float", Number);
	//Float.addOp("_op+", [](EvalState& e) { return 3; });

	//auto String = ts.newType("String");
	//String.addOp("_op+", [](EvalState& e) { return 4; });
	//String.addOp("_op/", [](EvalState& e) { return 4; });

	/*
	Conversion declarations
	*/
	//ts.addConv(Int, Float);		// This will probably be automated in the addOp method (Int.String -> addConv(Int, String))
	convs.add(Int, Float);
	convs.add(Int, String);
	convs.add(String, Int);
	// convs.add(Number, String);			// Replace conversion of Int -> String (Will lub(Int, String) = String ???)

	/*
	Testing
	*/

	// Testing common type
	try {
		ps("String + Int         ");
		pl(dispatch(lub(String, Int, "_op+", convs, types), "_op+", types, e));			// String._op+
		ps("Int + String         ");
		pl(dispatch(lub(Int, String, "_op+", convs, types), "_op+", types, e));			// String._op+
		ps("String * Int         ");
		pl(dispatch(lub(String, Int, "_op*", convs, types), "_op*", types, e));			// Number._op*
		ps("Float / Int          ");
		pl(dispatch(lub(Float, Int, "_op/", convs, types), "_op/", types, e));			// Exception
	} catch (std::string& e) {
		pl("\n" + e);
	}

	// Testing Inheritance
	try {
		ps("Int._op*             ");
		pl(dispatch(Int, "_op*", types, e));						// Number._op*
		ps("Float._op*           ");
		pl(dispatch(Float, "_op*", types, e));						// Float._op*
		ps("String._op*          ");
		pl(dispatch(String, "_op*", types, e));						// Exception

	} catch (std::string& e) {
		pl("\n" + e);
	}


	std::cin.get();
}

// Very basic inheritance relation
size_t dispatch(dust::impl::Type& t, std::string op, std::vector<dust::impl::Type>& types, dust::EvalState& e) {
	return dispatch(t.id, op, types, e);
}

// Overload for using the type field index
size_t dispatch(size_t t, std::string op, std::vector<dust::impl::Type>& types, dust::EvalState& e) {
	t = findDef(t, op, types);

	ps(types[t].name + "." + op);
	
	return types[t].ops[op](e);
	// return types[findDef(t, op, types)].ops[op](e);
}

size_t findDef(dust::impl::Type& t, std::string field, std::vector<dust::impl::Type>& types) {
	return findDef(t.id, field, types);
}

size_t findDef(size_t t, std::string field, std::vector<dust::impl::Type>& types) {
	auto err_t = t;

	while (types[t].ops.count(field) == 0)
		t = t > 0 ? types[t].parent : throw std::string{ "Dispatch error: " + types[err_t].name + "." + field + " not defined" };

	return t;
}

size_t lub(dust::impl::Type& l, dust::impl::Type& r, std::string op, dust::impl::ConvTracker& conv, std::vector<dust::impl::Type>& types) {
	auto t = conv.lub(l, r);

	// if (t == 0) (conv.lub currently throws an error if there is no direct conversion)
	// Should this take into account inheritance conversions (ie. Int + String would translate to String(Number(Int)) + String if Number.String and !Int.String)

	try {
		findDef(t, op, types);				// Check if the common type has the operator declared
		return t;							// Uses try-check since findDef throws an exception when the inheritance tree is used (unfortunately)
	} catch (std::string& e) { }

	return l.id + r.id - t;					// Assume the other type has the operator declared (This will raise an error in dispatch)
}

/*
impl::Type type(int id) { return types[id]; }
impl::Type type(EvalState::Type t) { return types[t.id]; }
impl::Type type(std::string t) { return types[getId(t)]; }
EvalState::Type getType(int id) { return impl::StateType{ id, this }; }
EvalState::Type getType(std::string t) { return impl::StateType{ getId(t), this }; }

int getId(std::string t) { return type_id[t]; }
*/