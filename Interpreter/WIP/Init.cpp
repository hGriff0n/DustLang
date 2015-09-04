#include "Init.h"
#include "TypeSystem.h"

#include "TypeTraits.h"

using namespace dust;
using namespace dust::impl;

void initTypeSystem(TypeSystem& ts) {
	auto Object = ts.getType("Object");
	auto Number = ts.newType("Number");
	auto Int = ts.newType("Int", Number);
	auto Float = ts.newType("Float", Number);
	auto String = ts.newType("String");
	auto Bool = ts.newType("Bool");
	auto Table = ts.newType("Table");
	auto Function = ts.newType("Function");

	// Initialize TypeTraits id's
	TypeTraits<int>::id = Int;
	TypeTraits<double>::id = Float;
	TypeTraits<std::string>::id = String;
	TypeTraits<bool>::id = Bool;
}

