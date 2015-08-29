#include "TypeSystem.h"

#include "GC.h"

#include "Init.h"
#include "TypeTraits.h"

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
	// Stack
		// Stack for storing values to be used in calculations
		// Have the stack be indexable (from the front and the back, necessary for the forceType function
		// Various other stuff (focus on robustness and usability)

	// Stack interaction
		// Store Values on the stack
		// Modify Values on the stack ?

	// GC interaction
		// Have pushing and popping call correct tags
			// inc- and decRef if the Value is a String (I'm not sure how this'll work though)

	// TypeSystem interaction
		// Have conversions use converters to convert
		// Ensure com and dispatch work on Values
		// Maybe even add invokable functions
			// Be able to run "Hello " + 3;

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

void to_int(impl::Value&, impl::GC&);
void to_float(impl::Value&, impl::GC&);
void to_bool(impl::Value&, impl::GC&);
void to_string(impl::Value&, impl::GC&);

template <typename T>
impl::Value make_value(T, impl::GC&);
impl::Value make_value(const char* s, impl::GC&);

template <typename T>
void assign_value(impl::Value&, T, impl::GC&);


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


int main(int argc, const char* argv[]) {
	using namespace impl;

	EvalState e;
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

	auto v1 = make_value(3.2, gc);
	auto v2 = make_value(3, gc);
	auto v3 = make_value("Hello", gc);

	/*
	Testing worksheet
	*/

	printValue(v1, gc, ts);						// FLOAT :: 3.200000

	//TypeTraits<int>::to(v1, gc);
	to_int(v1, gc);
	printValue(v1, gc, ts);						// INT :: 3

	// convert to float
	to_float(v1, gc);
	printValue(v1, gc, ts);						// FLOAT :: 3.000000

	// convert to bool
	to_bool(v1, gc);
	printValue(v1, gc, ts);						// BOOL :: true

	to_int(v1, gc);

	// convert to string
	to_string(v1, gc);							// STRING :: 3
	printValue(v1, gc, ts);

	// convert to int
	to_int(v1, gc);
	printValue(v1, gc, ts);						// INT :: 3 (Currently gives 0 as the gc is not hooked up correctly yet)

	// assign "World" to v3
	assign_value(v3, "World", gc);
	std::printf("Running Garbage Collecter. Collected %d references\n", gc.run());

	try {
		to_int(v3, gc);
	} catch (std::string& e) {
		pl(e);
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

	//ts.getType("Int").addOp("String", [](CallStack& c) { c.push(pop_string(c)); return 1; });
	//ts.getType("Int").addOp("Float", [](CallStack& c) { c.push(pop_int(c)); return 1; });
}


void to_int(impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	v.val.i = TypeTraits<int>::get(v, gc);
	v.type_id = TypeTraits<int>::id;
}

void to_float(impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	v.val.d = TypeTraits<double>::get(v, gc);
	v.type_id = TypeTraits<double>::id;
}

void to_bool(impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);
	
	v.val.i = TypeTraits<int>::get(v, gc);
	v.type_id = TypeTraits<bool>::id;
}

void to_string(impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	v.val.i = gc.loadRef(TypeTraits<std::string>::get(v, gc));
	v.type_id = TypeTraits<std::string>::id;
}


void convert(impl::Value& v, size_t id, impl::TypeSystem& ts) {
	ps(ts.getName(v.type_id) + " -> " + ts.getName(id));
	pl(ts.convertible(v.type_id, id));
}


template <typename T>
impl::Value make_value(T val, impl::GC& gc) {
	return TypeTraits<T>::make(val, gc);
}

impl::Value make_value(const char* s, impl::GC& gc) {
	return TypeTraits<std::string>::make(s, gc);
}


template <typename T>
void assign_value(impl::Value& v, T val, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	v = make_value(val, gc);
}