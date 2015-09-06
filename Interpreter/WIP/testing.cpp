#include "Init.h"
#include "Actions.h"

#include <iostream>

#include <pegtl/analyze.hh>

#define p(x) std::cout << (x)
#define ps(x) p(x) << " :: "
#define pl(x) p(x) << "\n"
#define nl() pl("")

// Current testing devoted to
	// Systems for implementing dynamic variable storage and recall
	// Possibly also for testing the development of type_traits style classes

// TODO:
	// Cleaning
		// Simplify and reduce the process of decrementing references
		// Remove Init.h (merge with EvalState.h ???)
		// Add a push_ref method ???
		// Focus the API (reduce unecessary functions)
		// Clean and Organize AST.h
		// Reduce type::Traits specialization errors
			// include "GC.h" in TypeTraits.h and move the specializations to TypeTraits.h ???
		// Error and Exception throwing
		// Comments and Code Organization
		// Merge Backend-Rewrite into master (delete TypeSystem branch)

	// Step Back and Determine Where I Am
		// Comment all of the current code
		// Go over and update documentation
		// Update the "ToDo" list to prioritize important/simple improvements and updates
		// Consider changing name of _op() due to semantical differences
		// Consider adding a push_ref method to CallStack (roughly mirrors pop_ref)
		// What if a type inherits from String ???
		// Organize files into sub-folders

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

void convert(impl::Value&, size_t, type::TypeSystem&);

// Assign a Value
template <typename T>
void assign_value(impl::Value&, T, impl::GC&);
void assign_value(impl::Value&, impl::Value&, impl::GC&);


// Type Traits specializations
// Moving these definitions to CallStack.h causes a LNK2005 error
template<> int type::Traits<int>::get(const impl::Value& v, impl::GC& gc) {
	try {
		if (v.type_id == type::Traits<double>::id)
			return v.val.d;

		else if (v.type_id == type::Traits<int>::id || v.type_id == type::Traits<bool>::id)
			return v.val.i;

		else if (v.type_id == type::Traits<std::string>::id)
			return std::stoi(gc.deref(v.val.i));
	} catch (...) {}

	throw std::string{ "Not convertible to Int" };
}

template<> double type::Traits<double>::get(const impl::Value& v, impl::GC& gc) {
	try {
		if (v.type_id == type::Traits<double>::id)
			return v.val.d;

		else if (v.type_id == type::Traits<int>::id || v.type_id == type::Traits<bool>::id)
			return v.val.i;

		else if (v.type_id == type::Traits<std::string>::id) {
			return std::stod(gc.deref(v.val.i));
		}
	} catch (...) {}

	throw std::string{ "Not convertible to Float" };
}

template<> std::string type::Traits<std::string>::get(const impl::Value& v, impl::GC& gc) {
	if (v.type_id == type::Traits<std::string>::id)
		return gc.deref(v.val.i);

	else if (v.type_id == type::Traits<bool>::id)
		return v.val.i ? "true" : "false";

	else if (v.type_id == type::Traits<int>::id)
		return std::to_string(v.val.i);

	else if (v.type_id == type::Traits<double>::id)
		return std::to_string(v.val.d);

	throw std::string{ "Not convertible to String" };
}

template<> bool type::Traits<bool>::get(const impl::Value& v, impl::GC& gc) {
	if (v.type_id == type::Traits<bool>::id)
		return v.val.i;

	throw std::string{ "Not convertible to Bool" };
}

template <class ostream>
void print(ostream& s, std::shared_ptr<parse::ASTNode>& ast) {
	(s << ast->print_string("|") << "\n").flush();
}


int main(int argc, const char* argv[]) {
	std::cout << "Analyzing `dust::grammar`.....\n";
	pegtl::analyze<grammar>();
	std::cout << "\n" << std::endl;

	parse::AST parse_tree;
	std::string input;
	
	EvalState e;
	initState(e);


	std::cout << "> ";
	while (std::getline(std::cin, input) && input != "exit") {
		try {
			pegtl::parse<grammar, action>(input, input, parse_tree);

			print(std::cout, parse_tree.at());
			parse_tree.pop()->eval(e);

			// Need to make a generic way here
			std::cout << ":: " << (std::string)e << std::endl;
		} catch (pegtl::parse_error& e) {
			std::cout << e.what() << std::endl;
		} catch (std::out_of_range& e) {
			std::cout << e.what() << std::endl;
		} catch (std::string& e) {
			std::cout << e << std::endl;
		}

		parse_tree.clear();
		std::cout << "> ";
	}
}


void initConversions(type::TypeSystem& ts) {
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

void initOperations(type::TypeSystem& ts) {
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
	Int.addOp("_op>", [](EvalState& e) { e.push((int)e >(int)e); return 1; });
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


void convert(impl::Value& v, size_t id, type::TypeSystem& ts) {
	ps(ts.getName(v.type_id) + " -> " + ts.getName(id));
	pl(ts.convertible(v.type_id, id));
}


template <typename T>
void assign_value(impl::Value& v, T val, impl::GC& gc) {
	if (v.type_id == type::Traits<std::string>::id) gc.decRef(v.val.i);

	v = make_value(val, gc);
}

void assign_value(impl::Value& v, impl::Value& a, impl::GC& gc) {
	if (v.type_id == type::Traits<std::string>::id) gc.decRef(v.val.i);
	if (a.type_id == type::Traits<std::string>::id) gc.incRef(a.val.i);

	v = a;
}