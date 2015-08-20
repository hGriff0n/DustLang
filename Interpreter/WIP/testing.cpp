#include "TypeSystem.h"

#include "Value.h"
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

	// Garbage collection
		
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

	EvalState e;

	/*
	"Global" structures that will eventually be collected within EvalState
	*/
	TypeSystem ts;
	//stack<Value> s{};

	/*
	Testing declarations
	*/

	str_record* s1 = makeRecord("Hello");
	str_record* s2 = makeRecord("Hello");
	str_record* tr = set(nullptr, ", ");				// temporary register

	//test();
	nl();

	/*
	Testing
	//*/

	// "Hello" + ", " + "World!" + "Hello"
	// s2 = makeRecord("Hello")

	// tr = set(nullptr, ", ")
	// s2 = combine(s2, tr)
	// tr = delRef(tr)

	// tr = set(tr, "World!")					// Is explicitly deleting tr absolutely necessary?
	// s2 = combine(s2, tr)						// set will reuse memory if the passed record is the only reference (but not if the new string already has a record)
	// tr = delRef(tr)							// delRef immediately adds the record to GC stack for allocation purposes (set relies on the garbage collector to collect the old records)

	// tr = set(tr, "Hello")
	// s2 = combine(s2, tr)
	// tr = delRef(tr)

	// s2 = "Hello"
	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));
	ps(tr);
	pl(deref(tr));

	nl();
	printAll();
	nl();

	// s2 + ", "
	s2 = combine(s2, tr);
	tr = delRef(tr);

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	printAll();
	nl();

	// s2 + "World!"
	tr = set(tr, "World!");
	s2 = combine(s2, tr);
	tr = delRef(tr);				// However, s3 is not "actually" deleted

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	printAll();
	nl();

	// s2 + "Hello"
	tr = set(tr, "Hello");
	s2 = combine(s2, tr);
	tr = delRef(tr);				// Doesn't invalidate s1

	ps(s1);
	pl(deref(s1));
	ps(s2);
	pl(deref(s2));

	nl();
	printAll();

	std::cin.get();
}

size_t dispatch(size_t t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	auto d_type = ts.findDef(t, op);

	if (d_type == ts.NIL) throw std::string{ "Dispatch Error: " + ts.get(t).name + "." + op + " is not defined" };

	ps(ts.get(d_type).name + "." + op);
	
	return ts.get(d_type).ops[op](e);
}
