#include "TypeSystem.h"

#define INT_STACK

#include "GC.h"
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

	// Garbage collection / Encapsulate Storage in a class
		// Possibly change from passing around str_record* to size_t (the index where the record is stored)
			// Slightly easier error handling (internal details can't be accessed anyways right now)
			// That'll have to be done in a seperate file (after the move to stack<size_t> is complete)
		// Possibly have the "open" slots sorted (low->high)
			// Compact the string storage
		// Generalize and improve the interface for future additions
			// Maybe change RuntimeStorage to a templated structure that encapsulates these precedings
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
void debugPrint(dust::impl::GC&);

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
	GC gc;

	/*
	Testing declarations
	*/

	auto s1 = gc.loadRef("Hello");
	auto s2 = gc.loadRef("Hello");
	auto s3 = gc.loadRef("need");
	auto s4 = gc.loadRef("records");
	auto s5 = gc.loadRef("allocator");
	auto s6 = gc.loadRef("garbage");
	auto s7 = gc.loadRef("with");
	decltype(s7) s8 = nullptr;

	auto t1 = gc.tempRef("Hello");
	auto t2 = gc.tempRef(" :: ");


	nl();

	/*
	Testing
	//*/

	// What do I need to test
		// That the garbage collector can mark unreferenced records (no "delRef")
		// The unreferenced records are used by the allocater to initialize new records
		// The garbage collector does not interfere with execution

	debugPrint(gc);
	// Proof-of-concept testing

	// Testing that the garbage collector collects no records when there are no records to collect
	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s2, s3, s4, s5, s6, s7
	debugPrint(gc);


	// Testing that the garbage collector still collects no records when there are none to collect
	s2 = gc.setRef(s2, "Nothing here");

	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s2, s3, s4, s5, s6, s7
	debugPrint(gc);


	// Testing that the garbage collector will collect records when there are some to collect
	decRef(s4);
	
	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s2, s3, s5, s6, s7
	debugPrint(gc);


	// Testing that new allocations take advantage of the freed space
	s8 = gc.loadRef("Equality Check");														// s1, s2, s3, s5, s6, s7, s8

	debugPrint(gc);

	// Testing behavior of incrParse collector (currently takes 4 elements)

	s4 = gc.loadRef("Hello");

	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s2, s3, s4, s5, s6, s7, s8
	debugPrint(gc);

	decRef(s3);
	decRef(s4);
	decRef(s5);

	std::printf("Running incrParse collector... Collected %d records\n", gc.run(true));		// s1, s2, s3, s4, s5, s6, s7, s8
	debugPrint(gc);

	incRef(s4);

	std::printf("Running incrParse collector... Collected %d records\n", gc.run(true));		// s1, s2, s5, s6, s7, s8
	debugPrint(gc);


	// Shakedown tests
	s4 = gc.combine(s4, t1);
	s5 = gc.combine(s5, t2);			// s5 is deleted here (only if I use the size_t isCollectableRecord)

	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s2, s4, s5, s6, s7, s8
	debugPrint(gc);

	decRef(s2);
	decRef(s4);
	
	std::printf("Running garbage collector... Collected %d records\n", gc.run());			// s1, s5, s6, s7, s8
	debugPrint(gc);
	gc.delTemps();

	
	s2 = gc.loadRef("Allocation");
	debugPrint(gc);

	//std::cout << "Finished tests";
	std::cin.get();
}

size_t dispatch(size_t t, std::string op, dust::impl::TypeSystem& ts, dust::EvalState& e) {
	auto d_type = ts.findDef(t, op);

	if (d_type == ts.NIL) throw std::string{ "Dispatch Error: " + ts.get(t).name + "." + op + " is not defined" };

	ps(ts.get(d_type).name + "." + op);
	
	return ts.get(d_type).ops[op](e);
}

void debugPrint(dust::impl::GC& gc) {
	nl();
	gc.printAll();
	p("Available Records: ");
	pl(gc.collected());
	nl();
}