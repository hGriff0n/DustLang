#include "TypeSystem.h"

#define USE_GC
//#define USE_TEST_GC

#ifdef USE_GC
	#include "GC.h"
#else
	#ifdef USE_TEST_GC
		#include "_GC.h"
	#else
		#include "Value.h"
	#endif
#endif

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
	// Define what I'm expecting from this phase of the project and what each part should accomplish

	// Garbage collection / Encapsulate Storage in a class
		// Have a system for speciying "open" slots
			// Decide which GC model to use as the baseline (both can perform the same)
				// May even include both with a preprocessor define to switch between the two
			// Generate the basic structures and methods that'd allow the garbage collector to work
		// Implement basic garbage collection
			// Stop-the-world processing, gather all unused references in one go
			// Possible to only run if there's less than x number of open slots on the stack
		// Implement a incremental collecter
			// Possibly even add a tag to switch between the two (or split into seperate functions)
		// Generalize and improve the interface for future additions
			// Maybe move the main GC class to a templated storage structure that encapsulates these precedings
			// Then create a new GC class that combines the storage structures into one interface
		
	// Atoms
	// Values
	// Stack interaction
		// Modify the stack interface so that it is indexable (allowing the forceType function)
	// Variables
	// Encapsulation
	// TypeSystem integration
	// EvalState and expression evaluation
	// AST Construction
	// Integrate with the grammar

// Things to work on
	// Improving and consolidating the API
	// Namespace naming for best code organization
	// After the rewrite is finished, move the dust documents into this project and update the documentation

// Other Stuff and Pipe Dreams

// I also need to merge my current work on dust semantics and syntax with the documents in DustParser (keed documentation intact)

class dust::EvalState {
	private:
		std::map<std::string, int> type_id;
		impl::TypeSystem ts;

	public:

		impl::TypeSystem& getTS() {
			return ts;
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

size_t dispatch(size_t, std::string, dust::impl::TypeSystem&, dust::EvalState&);

int main(int argc, const char* argv[]) {
	using namespace dust::impl;
	using namespace dust;
#ifdef USE_TEST_GC
	using namespace dust::test;
#endif

	EvalState e;

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	TypeSystem ts;

	// impl::GC is less reliant on pointers
	// test::GC has a simpler implementation

	GC gc;

	/*
	Testing declarations
	*/

	auto s1 = gc.loadRef("Hello");
	auto s2 = gc.loadRef("Hello");
	
	//test();
	nl();

	/*
	Testing
	//*/
#ifndef USE_EXP_TEMPS
	auto tr = gc.setRef(nullptr, ", ");
	
	// s2 = "Hello"
	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));
	ps(tr);
	pl(deref(tr));

	nl();
	gc.printAll();
	nl();

	// s2 + ", "
	s2 = gc.combine(s2, tr);
	tr = gc.delRef(tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	gc.printAll();
	nl();

	// s2 + "World!"
	tr = gc.setRef(tr, "World!");
	s2 = gc.combine(s2, tr);
	tr = gc.delRef(tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	gc.printAll();
	nl();

	// s2 + "Hello"
	tr = gc.setRef(tr, "Hello");
	s2 = gc.combine(s2, tr);
	tr = gc.delRef(tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));
#else
	auto tr = gc.tempRef(", ");

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));
	ps(tr);
	pl(deref(tr));

	s2 = gc.combine(s2, tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	gc.printAll();
	nl();

	gc.setTemp(tr, "World!");
	s2 = gc.combine(s2, tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	gc.printAll();
	nl();

	gc.setTemp(tr, "Hello");
	s2 = gc.combine(s2, tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	gc.flushTemporaries();
	tr = nullptr;

#endif

	nl();
	gc.printAll();
	pl(gc.collected());

	std::cin.get();
}

size_t dispatch(size_t t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	auto d_type = ts.findDef(t, op);

	if (d_type == ts.NIL) throw std::string{ "Dispatch Error: " + ts.get(t).name + "." + op + " is not defined" };

	ps(ts.get(d_type).name + "." + op);
	
	return ts.get(d_type).ops[op](e);
}
