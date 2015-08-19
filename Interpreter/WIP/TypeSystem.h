#pragma once

#include "Type.h"
#include <vector>
#include <array>

#include <iostream>

// TODO/Considerations
	// Can I get inheritable (implicit) converters to work in the case of com operations (Way of establishing precedence)
		// They can work easily in the case of function arguments and typed assignments
	// How can I define a general converter (specifically for Tables)
		// I could just have every type define a converter to Table (wraps the value in a table) and have this definition automatically be added to new types
		// Though if you think about it, this isn't all that different from hard-coding the selection in the evaluation (There's trade-offs of course)
		// Moreover the converter can also be overwritten/rewritten to have lower precedence
	// Consider moving converter precedence resolution to "first declaration" (currently "first definition")
	// Currently NIL type is an error code, but the idea is for it to have some meaning (ie. operations and values)
	// Define entry, throw, and catch points for exceptions and error handling (The next chunk of dust is to add exceptions so I won't handle this now)
		// Add in checks for assigning nil in the future (for some Type methods/members)

namespace dust {
	namespace impl {

		class TypeSystem {
			public:
				// A visitor interface to the type records that allows modifications on individual types to be maintained after scope exit
					// Possible security issues
				struct TypeVisitor {
					private:
						size_t id;
						TypeSystem* ts;

					public:
						TypeVisitor(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

						TypeVisitor& addOp(std::string, Function);
					
						operator size_t();
				};

				static const size_t NIL = -1;

			private:
				std::vector<impl::Type> types;							// Maintains type records, Indices are type_id (Could I maintain this as a tree, reduces O-cost of ancestor and isParentOf?)
				std::map<convPair, std::array<size_t, 2>> conv;			// Tracks conversion precedence
				std::map<convPair, size_t> siblings;					// Memoize the ancestor of two types
				std::map<std::string, size_t> type_id;					// Associates name to type id

				// Generate a convPair key so that key(a, b) == key(b, a)
				convPair key(size_t, size_t);

				// Really only useful if I'm storing the conversion function in conv (I'm not though)
				//size_t convert(size_t, size_t);

				// Add a conversion to the registry while maintaining precedence levels (Only callable from the TypeVisitor interface)
				void addConv(size_t, size_t);

				// Determines if p is a parent of t (ie. p `isParentOf` t)
				bool isParentOf(size_t, size_t);

				// Find the common ancestor of l and r
				size_t ancestor(size_t, size_t);

				// Create a Type visitor with the given parent
				TypeVisitor newType(std::string, size_t);

			public:
				TypeSystem();

				// Inheritance Resolution (returns NIL if no definition is found
				// Find op definition in type or in parent(type)
				size_t findDef(size_t, std::string);

				// Find op definition in type without considering inheritance relationships
				size_t isDefd(size_t, std::string);
			  //size_t findLoc(size_t, std::string);


				// Common Type Resolution (Find a type that defines op and that both l and r can be cast to)
				size_t com(size_t, size_t, std::string);

				/* Example 'dispatch' that performs converter resolution using the current API
				dispatch(size_t l, size_t r, std::string op)
					auto com_t = ts.com(l, r, op)
					auto dis_t = ts.findDef(com_t, op)
					
					if (dis_t == NIL)
						throw SomeExceptionClassThatIveNotYetGottenAroundToDefining()

					// Determine whether com selected a converter and perform conversions
					if ((com_t == l ^ com_t == r) && ts.convertible(l, r))
						forceType(-1, com_t);					// The name is a placeholder (Maybe move to be a TypeSystem method ???)
						forceType(-2, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary

					return ts.get(dis_t).ops[op](e);
				*/

				size_t com(Type&, Type&, std::string);


				// Type Definition methods
				// Create a type visitor to a new type with an optional parent
				TypeVisitor newType(std::string);

				TypeVisitor newType(std::string, Type&);

				TypeVisitor newType(std::string, TypeVisitor&);

				// Gets a type visitor to a predefined type
				TypeVisitor getType(size_t);

				TypeVisitor getType(std::string);
				
				// Get the implementation type (for eval, etc.)
				Type get(size_t);

				Type get(std::string);


				// Temporary/Ungrouped Methods (may be expanded/grouped later)
				bool convertible(size_t, size_t);		// Check for converter

				// void forceType(int, size_t);			// Would also need a stack to be passed
		};
	}
}
