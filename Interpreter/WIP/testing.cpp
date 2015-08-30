#include "TypeSystem.h"

#include "GC.h"

#include "Init.h"
#include "TypeTraits.h"

#include "Stack.h"

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
	// GC interaction
		// Pop values from the stack, modify them, push them onto the stack
		// Generate a generic framework for popping values from the stack

	// TypeSystem interaction
		// Have conversions use converters to convert
		// Ensure com and dispatch work on Values
		// Maybe even add invokable functions
			// Be able to run "Hello " + 3;

	// Possibly wrap the current CallStack
		// Could provide a more feature-rich API
		// Automatically handle interactions with the GC, etc.

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

// Convert Value to the type
void to_int(impl::Value&, impl::GC&);
void to_float(impl::Value&, impl::GC&);
void to_bool(impl::Value&, impl::GC&);
void to_string(impl::Value&, impl::GC&);

// Make a Value
template <typename T>
impl::Value make_value(T, impl::GC&);
impl::Value make_value(const char* s, impl::GC&);

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
template <typename T>
void push(impl::Stack&, T, impl::GC&);
void push(impl::Stack&, impl::Value&, impl::GC&);

template <typename T>
T pop(impl::Stack&, impl::GC&, int = -1);
impl::Value pop(impl::Stack&, impl::GC&, int = -1);

// In-place pop
template <typename T>
void pop(impl::Stack&, T&, impl::GC&, int = -1);

size_t pop_ref(impl::Stack&, impl::GC&, int = -1);

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

	Stack c;
	auto v1 = make_value("World!", gc);

	/*
	Testing worksheet
	*/

	push(c, 3, gc);
	push(c, 3.2, gc);
	push(c, "Hello", gc);
	c.push(make_value("3", gc));
	push(c, v1, gc);								// World! | Hello | Hello | 3.2 | 3

	c.swap();										// 3 | World! | Hello | 3.2 | 3

	//assign_value(c.at(), "World!", gc);
	//assign_value(c.at(-2), c.at(-1), gc);

	int s = pop<int>(c, gc);						// World! | Hello | 3.2 | 3

	c.swap();										// Hello | World! | 3.2 | 3
	std::string t;
	pop(c, t, gc);

	printValue(c.pop(-3), gc, ts);
	//printValue(c.pop(), gc, ts);			// These should decrement the references (They don't)
	pl(pop<std::string>(c, gc));			// This decrements the reference however
	pl(pop<double>(c, gc, 0));
	
	//printValue(c.pop(0), gc, ts);			// These should decrement the references
	//printValue(pop(c, gc, 0), gc, ts);

	try {
		c.pop();
	} catch (std::out_of_range& e) {
		std::cout << e.what();
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

void assign_value(impl::Value& v, impl::Value& a, impl::GC& gc) {
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);
	if (a.type_id == TypeTraits<std::string>::id) gc.incRef(a.val.i);

	v = a;
}



template <typename T>
void push(impl::Stack& s, T val, impl::GC& gc) {
	s.push(make_value(val, gc));
}

void push(impl::Stack& s, impl::Value& val, impl::GC& gc) {
	if (val.type_id == TypeTraits<std::string>::id) gc.incRef(val.val.i);
	
	s.push(val);
}


template <typename T>
T pop(impl::Stack& s, impl::GC& gc, int idx) {
	auto v = s.pop(idx);
	
	if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

	return TypeTraits<T>::get(v, gc);
}

impl::Value pop(impl::Stack& s, impl::GC& gc, int idx) {
	return s.pop(idx);
}

template <typename T>
void pop(impl::Stack& s, T& val, impl::GC& gc, int idx) {
	val = pop<T>(s, gc, idx);
}

size_t pop_ref(impl::Stack& s, impl::GC& gc, int idx) {
	if (s.at(idx).type_id != TypeTraits<std::string>::id) throw std::logic_error{ "No reference to Object" };

	return s.pop(idx).val.i;
}