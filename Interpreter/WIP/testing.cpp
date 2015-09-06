#include "Init.h"

//#include "EvalState.h"
#include "AST.h"

#include <iostream>
#include <sstream>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << std::endl
#define nl() pl("")

// Current testing devoted to
	// Systems for implementing dynamic variable storage and recall
	// Possibly also for testing the development of type_traits style classes

// TODO:
	// AST Construction Framework
		// Make Stack into a generic structure ??? (current AST construction (in Grammar.h) expects this)
			// CallStack would change to public impl::Stack<impl::Value>
			// Would have to move "is<T>" into CallStack

	// Grammar integration (AST)
		// Need to adjust the typedef "AST" struct in Calculator.h
		// Adjust the parsing actions to account for the changes in AST structure
		// Be able to pass all tests using the loop in old_main

	// Cleaning
		// Organize files into sub-folders
		// Merge Backend-Rewrite into master (delete TypeSystem branch)
		// Organize namespaces (Use better namespace divisions)
		// Reduce TypeTraits specialization errors
			// include "GC.h" in TypeTraits.h and move the specializations to TypeTraits.h ???
		// Improve method declarations to improve developability
		// Add a push_ref method ???
		// Simplify and reduce the process of decrementing references
		// Focus the API (reduce unecessary functions)
		// Error and Exception throwing
		// Comments and Code Organization

	// Step Back and Determine Where I Am
		// Comment all of the current code
		// Go over and update documentation
		// Update the "ToDo" list to prioritize important/simple improvements and updates
		// Consider changing name of _op() due to semantical differences
		// Consider adding a push_ref method to CallStack (roughly mirrors pop_ref)
		// What if a type inherits from String ???

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

void convert(impl::Value&, size_t, impl::TypeSystem&);

// Assign a Value
template <typename T>
void assign_value(impl::Value&, T, impl::GC&);
void assign_value(impl::Value&, impl::Value&, impl::GC&);


// Type Traits specializations
	// Moving these definitions to CallStack.h causes a LNK2005 error
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

template<> bool TypeTraits<bool>::get(const impl::Value& v, impl::GC& gc) {
	if (v.type_id == TypeTraits<bool>::id)
		return v.val.i;

	throw std::string{ "Not convertible to Bool" };
}

template <class ostream>
void print(ostream& s, std::shared_ptr<interpreter::ASTNode>& ast) {
	(s << ast->print_string("|") << "\n").flush();
}

int main(int argc, const char* argv[]) {
	using namespace impl;
	using namespace interpreter;

	EvalState e;
	initState(e);

	/*
	Testing declarations
	*/

	auto hello = makeNode<Literal>("Hello", TypeTraits<std::string>::id);
	auto world = makeNode<Literal>(" World!", TypeTraits<std::string>::id);
	auto add = makeNode<Operator>("_op+", hello, world);

	// The old code uses addChild to set the sub-nodes
	// But the old code still stores the nodes in a std::vector<std::shared_ptr<ASTNode>>

	add->eval(e);
	pl((std::string)e);

	nl();
	print(std::cout, add);

	auto var_a = makeNode<VarName>("a");
	auto var_b = makeNode<VarName>("b");
	auto list = makeNode<List<VarName>>(var_a, hello, var_b);			// "Ignores" hello (as it's not a VarName)
	auto vals = makeNode<List<ASTNode>>(var_a, hello, var_b);			// Doesn't ignore hello (as it is a ASTNode)

	auto as = makeNode<Assign>(list, vals, "");

	try {
		auto sa = makeNode<Assign>(vals, list, "");						// Throws an exception as vals is not a List<VarName>
	} catch (std::string& e) {
		pl(e);
	}

	/*
	Testing worksheet
	*/

	int a, b;
	std::string input, op;
	nl();

	while (std::getline(std::cin, input)) {
		if (input == "exit") break;

		std::istringstream{ input } >> a >> op >> b;
		e.push(b);
		e.push(a);

		p("> ");
		pl((int)e.callOp("_op" + op));
		//pl(e.callOp("_op" + op).pop<int>());
		nl();
	}

	//std::cout << "Finished tests";
	//std::cin.get();
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
	Int.addOp("String", [](EvalState& e) { e.push((std::string)e); return 1; });
	Int.addOp("Float", [](EvalState& e) { e.push((double)e); return 1; });

	Float.addOp("String", [](EvalState& e) { e.push((std::string)e); return 1; });
	Float.addOp("Int", [](EvalState& e) { e.push((int)e); return 1; });
	
	String.addOp("Int", [](EvalState& e) { e.push((int)e); return 1; });
	String.addOp("Float", [](EvalState& e) { e.push((float)e); return 1; });
}

void initOperations(impl::TypeSystem& ts) {
	auto Object = ts.getType("Object");
	auto Int = ts.getType("Int");
	auto Float = ts.getType("Float");
	auto String = ts.getType("String");
	auto Bool = ts.getType("Bool");
	
	//! -
	//^ * / + - % < = > <= != >=

	Object.addOp("_op<=", [](EvalState& e) {
		e.copy(-2);
		e.copy(-2);
		e.callOp("_op=");
		
		auto eq = (bool)e;
		if (!eq) {
			e.callOp("_op<");
			return 1;
		}
		
		e.pop();
		e.pop();
		e.push(eq);
		
		return 1;
	});
	Object.addOp("_op>=", [](EvalState& e) {
		e.copy(-2);
		e.copy(-2);
		e.callOp("_op=");

		auto eq = (bool)e;
		if (!eq) {
			e.callOp("_op>");
			return 1;
		}

		e.pop();
		e.pop();
		e.push(eq);

		return 1;
	});
	Object.addOp("_op!=", [](EvalState& e) {
		e.callOp("_op=");
		e.callOp("_ou!");
		return 1;
	});

	Int.addOp("_op+", [](EvalState& e) { e.push((int)e + (int)e); return 1; });
	Int.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });		// Could be moved to Number
	Int.addOp("_op-", [](EvalState& e) { e.push((int)e - (int)e); return 1; });
	Int.addOp("_op*", [](EvalState& e) { e.push((int)e * (int)e); return 1; });
	Int.addOp("_op^", [](EvalState& e) {
		auto base = (double)e;
		e.push(pow(base, (double)e));
		return 1;
	});											// Could be moved to Number
	Int.addOp("_op%", [](EvalState& e) { e.push((int)e % (int)e); return 1; });
	Int.addOp("_op<", [](EvalState& e) { e.push((int)e < (int)e); return 1; });		// 
	Int.addOp("_op=", [](EvalState& e) { e.push((int)e == (int)e); return 1; });
	Int.addOp("_op>", [](EvalState& e) { e.push((int)e > (int)e); return 1; });
	Int.addOp("_ou-", [](EvalState& e) { e.push(-(int)e); return 1; });


	String.addOp("_op+", [](EvalState& e) { e.push((std::string)e + e.pop<std::string>(-2)); return 1; });			// Why is Int._op- correct then???
	String.addOp("_op=", [](EvalState& e) { e.push(e.pop_ref(true) == e.pop_ref(true)); return 1; });


	Float.addOp("_op+", [](EvalState& e) { e.push((double)e + (double)e); return 1; });
	Float.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });
	Float.addOp("_op-", [](EvalState& e) { e.push((double)e - (double)e); return 1; });
	Float.addOp("_op*", [](EvalState& e) { e.push((double)e * (double)e); return 1; });
	Float.addOp("_op^", [](EvalState& e) {
		auto base = (double)e;
		e.push(pow(base, (double)e));
		return 1;
	});
	Float.addOp("_op<", [](EvalState& e) { e.push((double)e < (double)e); return 1; });		// 
	Float.addOp("_op=", [](EvalState& e) { e.push((double)e == (double)e); return 1; });
	Float.addOp("_op>", [](EvalState& e) { e.push((double)e >(double)e); return 1; });
	Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });
	Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });

	Bool.addOp("_op=", [](EvalState& e) { return 1; });
	Bool.addOp("_ou!", [](EvalState& e) { e.push(!(bool)e);  return 1; });
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
