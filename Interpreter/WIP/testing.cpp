//#include "Type.h"
//#include <vector>
#include "TypeSystem.h"

#include <iostream>

#define p(x) std::cout << x
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << std::endl

// Current testing devoted to
	// Systems for selecting the correct types and functions for program semantics

// Things to work on
	// Possibly handling multiple inheritance
	// Add Inheritance considerations for lub operations (searches inheritance tree for conversion function)?
	// Improving and consolidating the API
	// Move function definitions into .cpp files

class dust::EvalState {
	private:
		std::map<std::string, int> type_id;
		impl::TypeSystem ts;

	public:

		impl::Type getType(std::string t) {
			return ts.get(type_id[t]);
		}

		int dispatch(impl::Type& t, std::string op) {
			auto ty = ts.findDef(t.id, op);

			return 0;
			//return ts._get(ty).ops[op](*this);					// I use this (similar) code in the current production !!!!!
		}

		impl::Type dispatch_(impl::Type t, std::string op) {
			return ts.get(ts.findDef(t.id, op));
		}
};

size_t dispatch(dust::impl::TypeSystem::Type&, std::string, dust::impl::TypeSystem&, dust::EvalState&);
size_t dispatch(size_t, std::string, dust::impl::TypeSystem&, dust::EvalState&);

int main(int argc, const char* argv[]) {
	using namespace dust::impl;
	using namespace dust;

	EvalState e;

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	TypeSystem ts;

	/*
	Type declarations
	*/
	//auto Object = ts.getType(0);

	auto Number = ts.newType("Number");
	Number.addOp("_op*", [](EvalState& e) { return 1; });

	auto Int = ts.newType("Int", Number);
	Int.addOp("_op+", [](EvalState& e) { return 2; });

	auto Float = ts.newType("Float", Number);
	Float.addOp("_op+", [](EvalState& e) { return 3; });
	Float.addOp("_op*", [](EvalState& e) { return 3; });
	
	auto String = ts.newType("String");
	String.addOp("_op+", [](EvalState& e) { return 4; });
	String.addOp("_op/", [](EvalState& e) { return 4; });

	/*
	Conversion declarations
	*/
	Int.addConv(String);
	String.addConv(Int);
	Int.addConv(Float);
	// Conversions will be declared in dust by defining specific functions
		// Change the conversion process to handle this case (Need a way to associate name -> id)
	//Int.addOp("String", [](EvalState& e) { return 2; });
	//String.addOp("Int", [](EvalState& e) { return 4; });
	//Int.addOp("Float", [](EvalState& e) { return 2; });
	// Number.addConv(String);			// Replace conversion of Int -> String (Will lub(Int, String) = String ???)

	/*
	Testing
	*/

	// Testing common type
	try {
		ps("String + Int ");
		pl(dispatch(ts.lub(String, Int, "_op+"), "_op+", ts, e));						// String._op+ (4)
		ps("Int + String ");
		pl(dispatch(ts.lub(Int, String, "_op+"), "_op+", ts, e));						// String._op+ (4)
		ps("String * Int ");
		pl(dispatch(ts.lub(String, Int, "_op*"), "_op*", ts, e));						// Number._op* (1)
		ps("Float / Int  ");
		pl(dispatch(ts.lub(Float, Int, "_op/"), "_op/", ts, e));						// Exception
	} catch (std::string& e) {
		pl(e);
	}

	// Testing Inheritance
	try {
		ps("Int._op*     ");
		pl(dispatch(Int, "_op*", ts, e));								// Number._op* (1)
		ps("Float._op*   ");
		pl(dispatch(Float, "_op*", ts, e));								// Float._op* (3)
		ps("String._op*  ");
		pl(dispatch(String, "_op*", ts, e));							// Exception

	} catch (std::string& e) {
		pl(e);
	}


	std::cin.get();
}

size_t dispatch(size_t t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	t = ts.findDef(t, op);

	ps(ts.get(t).name + "." + op);
	
	return ts.get(t).ops[op](e);
}

size_t dispatch(dust::impl::TypeSystem::Type& t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	return dispatch(t.id, op, ts, e);
}

/*
impl::Type type(int id) { return types[id]; }
impl::Type type(EvalState::Type t) { return types[t.id]; }
impl::Type type(std::string t) { return types[getId(t)]; }
EvalState::Type getType(int id) { return impl::StateType{ id, this }; }
EvalState::Type getType(std::string t) { return impl::StateType{ getId(t), this }; }

int getId(std::string t) { return type_id[t]; }
*/