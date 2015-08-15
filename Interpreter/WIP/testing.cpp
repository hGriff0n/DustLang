//#include "Type.h"
//#include <vector>
#include "TypeSystem.h"

#include <iostream>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << std::endl

// Current testing devoted to
	// Systems for selecting the correct types and functions for program semantics

// TODO:
	// Is there anything else that TypeSystem should handle
		// I'll need to add in checks for assigning nil in the future (for Type methods/members)
	// Improve/Expand the TypeSystem temporary methods
	// Determine whether a boolean flag should be used if a converter is selected in TypeSystem::Com
		// It's not necessary to provide complete information. However it's nicer to work with.
		// It's also possible to return the boolean and have the common type as the reference "flag"

// Things to work on
	// How can I define a general converter (specifically for Tables)
	// Can I get inheritable (implicit) converters to work in the case of com operations (Way of establishing precedence)
		// They can work easily in the case of function arguments and typed assignments
	// Improving and consolidating the API
	// Currently NIL type is an error code, but the idea is for it to have some meaning (ie. operations and values)
	// Define entry, throw, and catch points for exceptions and error handling (The next chunk of dust is to add exceptions so I won't handle this now)
	// After the rewrite is finished, move the dust documents into this project and update the documentation

// I also need to merge my current work on dust semantics and syntax with the documents in DustParser (keed documentation intact)

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
	auto Object = ts.getType("Object");

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

	auto Bool = ts.newType("Bool");				// 5
	auto Table = ts.newType("Table");			// 6
	auto Function = ts.newType("Function");		// 7
	
	/*
	Conversion declarations
	*/
	Int.addOp("String", [](EvalState& e) { return 2; });
	String.addOp("Int", [](EvalState& e) { return 4; });
	Int.addOp("Float", [](EvalState& e) { return 2; });
	// Number.addOp("String", [](EvalState& e) { return 1; });		// Replace conversion Int -> String. Adds conversion Float -> String (Iff converters can be inherited, precedence issues)

	pl("");
	/*
	Testing
	*/

	//* Testing common type
	try {
		ps("String + Int   ");
		pl(dispatch(ts.com(String, Int, "_op+"), "_op+", ts, e));						// String._op+ (4)
		ps("Int + String   ");
		pl(dispatch(ts.com(Int, String, "_op+"), "_op+", ts, e));						// String._op+ (4)
		ps("String * Int   ");
		pl(dispatch(ts.com(String, Int, "_op*"), "_op*", ts, e));						// Number._op* (1)
		ps("Int + Int      ");
		pl(dispatch(ts.com(Int, Int, "_op+"), "_op+", ts, e));							// Int._op+ (2)
		//ps("Float / Int    ");
		//pl(dispatch(ts.com(Float, Int, "_op/"), "_op/", ts, e));						// Exception
		ps("Float + String ");
		pl(dispatch(ts.com(Float, String, "_op+"), "_op+", ts, e));						// Exception if Float -> String not defined
	} catch (std::string& e) {
		pl(e);
	}

	pl("");
	pl("");

	// Testing Inheritance
	try {
		ps("Int._op*       ");
		pl(dispatch(Int, "_op*", ts, e));								// Number._op* (1)
		ps("Float._op*     ");
		pl(dispatch(Float, "_op*", ts, e));								// Float._op* (3)
		ps("String._op*    ");
		pl(dispatch(String, "_op*", ts, e));							// Exception

	} catch (std::string& e) {
		pl(e);
	}
	//*/

	std::cin.get();
}

size_t dispatch(size_t t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	auto d_type = ts.findDef(t, op);

	if (d_type == ts.NIL) throw std::string{ "Dispatch Error: " + ts.get(t).name + "." + op + " is not defined" };

	ps(ts.get(d_type).name + "." + op);
	
	return ts.get(d_type).ops[op](e);
}
