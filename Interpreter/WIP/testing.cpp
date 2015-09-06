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