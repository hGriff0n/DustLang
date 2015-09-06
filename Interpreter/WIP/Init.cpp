#include "Init.h"
#include "TypeSystem.h"

#include "TypeTraits.h"

using namespace dust;
using namespace dust::type;

void initTypeSystem(TypeSystem& ts) {
	auto Object = ts.getType("Object");
	auto Number = ts.newType("Number");
	auto Int = ts.newType("Int", Number);
	auto Float = ts.newType("Float", Number);
	auto String = ts.newType("String");
	auto Bool = ts.newType("Bool");
	auto Table = ts.newType("Table");
	auto Function = ts.newType("Function");

	// Initialize type::Traits id's
	Traits<int>::id = Int;
	Traits<double>::id = Float;
	Traits<std::string>::id = String;
	Traits<bool>::id = Bool;
}

