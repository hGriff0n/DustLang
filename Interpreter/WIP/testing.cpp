#include "Init.h"

#include "TypeSystem.h"
#include "CallStack.h"
//#include "EvalState.h"

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
	// Consider adding basic evaluation capabilities
		// Test invokable functions
			// ie. "Hello " + 3 = "Hello 3"
		// add a "push_ref" method to CallStack ???

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
	_EvalState _e;
	initState(_e);

	// The big question is how converters are going to be implemented (particularly automatic conversions)
		// Raw conversions are easy enough to access that I can implement converters for basic types
		// But difficult enough that converters should be the default method of converting
		
		// Modify the Stack/CallStack relationship
			// Move the current CallStack API to Stack

		// Define a "Transfer type"
			// Create a type that is returned by CallStack::pop

		// Declare basic type methods as constant
			// Int -> String cannot be modified, etc.
			// In this formulation the converter and the raw conversion are one in the same
				// Converters are really only for the benefit of dust code (not for the C++ API)
					// Basically a converter is a special function that ensures that the data on the stack is a value of the correct type
					// The normal C++ API does not need this capability as is (It is perfectly valid, and actually better, for the programmer to work with the basic types)
						// But since a converter is a normal function, the capability still exists for a programmer to convert between non-basic types
						// Basic types being int, bool, double, std::string, void*
				// C++ API only needs to know (and can really only store) a certain number of types
					// I can add functions/structures to allow a more "dust" style of interacting in C++

		// Have the current method be the standard and converters are optional (for API development)
			// This is explained more above
			// This still leaves the question of how best to implement functions (calling won't be part of CallStack)

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	GC gc;
	TypeSystem ts;
	CallStack c{ gc };

	initTypeSystem(ts);
	initConversions(ts);

	/*
	Testing declarations
	*/

	auto v1 = TypeTraits<std::string>::make("World!", gc);
	
	/*
	Testing worksheet
	*/

	_e.push("Hello");

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

	c.push("Hello");
	c.push("Hello");										// "Hello" | "Hello"
	c.push(c.pop_ref(true) == c.pop_ref(true));				// true
	pl(c.pop<std::string>());

	c.clear();
	c.push(3);				// 0 -> Int
	c.push(3.2);			// 1 -> Float
	c.push("Hello");		// 2 -> String

	auto Number = ts.getType("Number");
	auto Int = ts.getType("Int");
	auto Float = ts.getType("Float");
	auto String = ts.getType("String");


	nl();
	// Test com works properly
	pl(ts.getName(ts.com(c.at(0), c.at(1), "_op*")));			// Float: Int -> Float and Float._op*
	pl(ts.getName(ts.com(c.at(0), c.at(1), "_op/")));			// Number: Common ancestor and Number._op/
	pl(ts.getName(ts.com(c.at(0), c.at(0), "_op*")));			// Int: Same type
	pl(ts.getName(ts.com(c.at(0), c.at(), "_op+")));			// String: Int -> String and String._op+
	pl(ts.getName(ts.com(c.at(0), c.at(), "_op/")));			// Int: String -> Int and Int._op/


	nl();
	// Test dispatch works properly
	pl(dispatch(c.at(0), "_op+", ts, e));							// Int._op+
	pl(dispatch(c.at(0), "_op*", ts, e));							// Number._op*
	try {
		pl(dispatch(c.at(), "_op*", ts, e));
	} catch (std::string& e) {
		pl(e);
	}
	pl(dispatch(ts.com(c.at(0), c.at(1), "_op*"), "_op*", ts, e));		// Float._op*
	pl(dispatch(ts.com(c.at(0), c.at(1), "_op%"), "_op%", ts, e));		// Number._op%


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
	auto Int = ts.getType("Int");
	auto Float = ts.getType("Float");
	auto String = ts.getType("String");


	// Initialize Conversions
	Int.addOp("String", [](EvalState& e) { return 2; });
	Int.addOp("Float", [](EvalState& e) { return 2; });

	String.addOp("Int", [](EvalState& e) { return 2; });
	//Float.addOp("Int", [](EvalState& e) { return 3; });
	
	//ts.getType("Int").addOp("String", [](CallStack& c) { c.push((std::string)c); return 1; });
	//ts.getType("Int").addOp("Float", [](CallStack& c) { c.push((int)c); return 1; });
	//ts.getType("String").addOp("_op=", [](CallStack& c) { c.push(c.pop_ref(true) == c.pop_ref(true)); return 1; });
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
