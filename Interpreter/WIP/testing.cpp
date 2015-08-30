#include "TypeSystem.h"

#include "GC.h"

#include "Init.h"
#include "TypeTraits.h"

//#include "Stack.h"
#include "CallStack.h"

#include "stack.h"
#include <iostream>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << std::endl
#define nl() pl("")

// Current testing devoted to
	// Systems for implementing dynamic variable storage and recall
	// Possibly also for testing the development of type_traits style classes

// TODO:
	// Extend the current Stack with CallStack
		// Some other API functions

	// TypeSystem interaction
		// Have conversions use converters to convert
		// Ensure com and dispatch work on Values
		// Maybe even add invokable functions
			// Be able to run "Hello " + 3;

	// Consider merging CallStack and Stack
		// CallStack would remain an extension of Stack but would have a TypeSystem& member
		// Would only convert objects automatically if a converter is defined, etc.
		// Do I want this (especially in regards to defining converters)
		// But at the same time I'll need it for dispatch and calling converters

	// Variables
		// Maintain different typing from Value
		// Ensure Values can be assigned to Variables
			// Ensure Variable's typing remains independent from Value's
		// Constant checking
			// Throw errors if the constant flag is set

	// TypeSystem 
		// Type checking
			// Be able to store sub-types as-is
			// Call converters if necessary
			// Throw errors otherwise

	// Encapsulation (Start moving all the functions into EvalState)


	// Expression evaluation
	// AST Construction Framework
	// Grammar integration (AST)

// Things to work on
	// Improving and consolidating the API
	// Namespace naming for best code organization
	// After the rewrite is finished, move the dust documents into this project and update the documentation

// Other Stuff and Pipe Dreams
	// Generalize RuntimeStorage and move the Garbage Collecter to "targeting" Storage (instead of inheriting)
		// Strings would have a different RuntimeStorage instance than tables, userdata, etc. (though most of the functions can be reused)
		// Perform these changes at the same time if I perform them at all
			// Generalizing the Garbage Collecter to "target" storage does not exactly require generalizing Storage however
	// Contrive of a better way of getting dust type from c++ type

// I also need to merge my current work on dust semantics and syntax with the documents in DustParser (keed documentation intact)

class dust::EvalState {
	private:
		std::map<std::string, int> type_id;
		impl::TypeSystem ts;
		impl::GC gc;

	public:

		impl::TypeSystem& getTS() {
			return ts;
		}

		impl::GC& getGC() {
			return gc;
		}

		int dispatch(impl::Type& t, std::string op) {
			auto ty = ts.findDef(t.id, op);

			return 0;
			//return ts.get(ty).ops[op](*this);					// I use this (similar) code in the current production !!!!!
		}

		impl::Type dispatch_(impl::Type& t, std::string op) {
			return ts.get(ts.findDef(t.id, op));
		}
};

using namespace dust;

size_t dispatch(size_t, std::string, impl::TypeSystem&, EvalState&);
void debugPrint(impl::GC&);
void printValue(impl::Value&, impl::GC&, impl::TypeSystem&);

void initConversions(impl::TypeSystem&);
void convert(impl::Value&, size_t, impl::TypeSystem&);


// Assign a Value
template <typename T>
void assign_value(impl::Value&, T, impl::GC&);
void assign_value(impl::Value&, impl::Value&, impl::GC&);


// Type Traits specializations
template<> int TypeTraits<int>::get(const impl::Value& v, impl::GC& gc) {
	try {
		if (v.type_id == TypeTraits<double>::id)
			return v.val.d;

		else if (v.type_id == TypeTraits<int>::id || v.type_id == TypeTraits<bool>::id)
			return v.val.i;

		else if (v.type_id == TypeTraits<std::string>::id)
			return std::stoi(gc.deref(v.val.i));
	} catch (...) {}

	throw std::string{ "Not convertible to Int" };
}

template<> double TypeTraits<double>::get(const impl::Value& v, impl::GC& gc) {
	try {
		if (v.type_id == TypeTraits<double>::id)
			return v.val.d;

		else if (v.type_id == TypeTraits<int>::id || v.type_id == TypeTraits<bool>::id)
			return v.val.i;

		else if (v.type_id == TypeTraits<std::string>::id) {
			return std::stod(gc.deref(v.val.i));
		}
	} catch (...) {}

	throw std::string{ "Not convertible to Float" };
}

template<> std::string TypeTraits<std::string>::get(const impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id)
		return gc.deref(v.val.i);

	else if (v.type_id == TypeTraits<bool>::id)
		return v.val.i ? "true" : "false";

	else if (v.type_id == TypeTraits<int>::id)
		return std::to_string(v.val.i);

	else if (v.type_id == TypeTraits<double>::id)
		return std::to_string(v.val.d);

	throw std::string{ "Not convertible to String" };
}

template<> impl::Value TypeTraits<std::string>::make(std::string s, impl::GC& gc) {
	return{ gc.loadRef(s), TypeTraits<std::string>::id };
}


// Stack specializations
// void push_ref(impl::Stack&, size_t, int = -1);			// ???

int main(int argc, const char* argv[]) {
	using namespace impl;

	EvalState e;

	// The big question is how converters are going to be implemented (particularly automatic conversions)
		// Raw conversions are easy enough to access that I can implement converters for basic types
		// But difficult enough that converters should be the default method of converting
		
		// What if I take advantage of the Stack/CallStack relationship?
			// Move the current CallStack API to Stack
			// Move the templates to 

		// Define a "Transfer type"
			// Create a type that is returned by CallStack::pop

		// Declare basic type methods as constant
			// Int -> String cannot be modified, etc.
			// In this formulation the converter and the raw conversion are one in the same
				// Converters are also really for the benefit of dust code (not for the C++ API)
				// C++ API only needs to know (and can really only store) a certain number of types
					// I can add functions/structures to allow a more "dust" style of interacting in C++

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	TypeSystem ts;
	GC gc;

	initTypeSystem(ts);
	initConversions(ts);

	/*
	Testing declarations
	*/

	CallStack c(gc);
	auto v1 = TypeTraits<std::string>::make("World!", gc);
	
	/*
	Testing worksheet
	*/

	c.push(3);
	c.push(3.2);
	c.push("Hello");
	c.push("3");
	c.swap();
	c.push(v1);												// "World!" | "Hello" | "3" | 3.2 | 3

	c.copy(-3);												// "3" | "World!" | "Hello" | "3" | 3.2 | 3

	auto s = (std::string)c;
	pl(s);
	c.push(s);												// "3" | "World!" | "Hello" | "3" | 3.2 | 3

	ps("Is idx 1 a String");
	pl(c.is<std::string>(1));

	c.reserve(15);

	c.replace(-4);											// "World!" | "Hello" | "3" | 3.2 | 3
	c.push(33);
	c.copy();												// 33 | 33 | "World!" | "Hello" | "3" | 3.2 | 3
	c.insert(-4);											// 33 | "World!" | "Hello" | 33 | "3" | 3.2 | 3
	c.pop(-4);

	c.settop(5);											// "World!" | "Hello" | "3" | 3.2 | 3
	c.push((std::string)c + " " + c.pop<std::string>());	// "Hello World!" | "3" | 3.2 | 3
	
	pl(c.pop<std::string>());
	auto i = c.pop<int>();
	c.pop();
	c.pop();

	try {
		c.pop();
	} catch (std::out_of_range& e) {
		pl(e.what());
	}

	//std::cout << "Finished tests";
	std::cin.get();
}


size_t dispatch(size_t t, std::string op, impl::TypeSystem& ts, EvalState& e) {
	auto d_type = ts.findDef(t, op);

	if (d_type == ts.NIL) throw std::string{ "Dispatch Error: " + ts.get(t).name + "." + op + " is not defined" };

	ps(ts.get(d_type).name + "." + op);
	
	return ts.get(d_type).ops[op](e);
}


void printValue(impl::Value& v, impl::GC& gc, impl::TypeSystem& ts) {
	ps(ts.getName(v.type_id));
	pl(TypeTraits<std::string>::get(v, gc));
}


void initConversions(impl::TypeSystem& ts) {
	using namespace dust;

	ts.getType("Int").addOp("String", [](EvalState& e) { return 3; });
	ts.getType("Int").addOp("Float", [](EvalState& e) { return 3; });

	//ts.getType("Int").addOp("String", [](CallStack& c) { c.push((std::string)c); return 1; });
	//ts.getType("Int").addOp("Float", [](CallStack& c) { c.push((int)c); return 1; });
	//ts.getType("String").addOp("_op=", [](CallStack& c) { c.push(c.pop_ref() == c.pop_ref()); return 1; });
}


void convert(impl::Value& v, size_t id, impl::TypeSystem& ts) {
	ps(ts.getName(v.type_id) + " -> " + ts.getName(id));
	pl(ts.convertible(v.type_id, id));
}


template <typename T>
void assign_value(impl::Value& v, T val, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	v = make_value(val, gc);
}

void assign_value(impl::Value& v, impl::Value& a, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);
	if (a.type_id == TypeTraits<std::string>::id) gc.incRef(a.val.i);

	v = a;
}
