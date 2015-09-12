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
	namespace type {

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

						TypeVisitor& addOp(std::string op, Function f);

						operator size_t();
				};

				static const size_t NIL = -1;

			private:
				std::vector<Type> types;								// Maintains type records, Indices are type_id (Could I maintain this as a tree, reduces O-cost of ancestor and isParentOf?)
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
				size_t findDef(size_t t, std::string fn);

				// Find op definition in type without considering inheritance relationships
				size_t isDefd(size_t t, std::string fn);
				//size_t findLoc(size_t, std::string);


				// Common Type Resolution (Find a type that defines op and that both l and r can be cast to)
				size_t com(size_t l, size_t r, std::string op);

				size_t com(Type& l, Type& r, std::string op);


				// Type Definition methods
				// Create a type visitor to a new type with an optional parent
				TypeVisitor newType(std::string t);

				TypeVisitor newType(std::string t, Type& p);

				TypeVisitor newType(std::string t, TypeVisitor& p);

				// Gets a type visitor to a predefined type
				TypeVisitor getType(size_t t);

				TypeVisitor getType(std::string t);

				// Get the implementation type (for eval, etc.)
				Type get(size_t t);

				Type get(std::string t);


				// Temporary/Ungrouped Methods (may be expanded/grouped later)
				// Checks if there is a converter between t1 and t2
				bool convertible(size_t t1, size_t t2);

				// Checks if t1 is a child of t2
				bool isChildType(size_t t1, size_t t2);

				std::string getName(size_t t);
				size_t getId(std::string t);
		};
	}
}