#include "Init.h"
#include "TypeSystem.h"
#include "GC.h"

#include "TypeTraits.h"

using namespace dust;
using namespace dust::impl;

void initTypeSystem(TypeSystem& ts) {
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

	auto Bool = ts.newType("Bool");
	auto Table = ts.newType("Table");
	auto Function = ts.newType("Function");

	// Initialize TypeTraits id's
	TypeTraits<int>::id = Int;
	TypeTraits<double>::id = Float;
	TypeTraits<std::string>::id = String;
	TypeTraits<bool>::id = Bool;
}

