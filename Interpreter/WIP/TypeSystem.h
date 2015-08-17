#pragma once

#include "Type.h"
#include <vector>

#include <iostream>

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


				// Common Type Resolution (Find a type that defines op and that both l and r can be cast to)
				size_t com(size_t, size_t, std::string);

				/* Example 'dispatch' that performs converter resolution using the current API
				dispatch(size_t l, size_t r, std::string op)
					auto com_t = ts.com(l, r, op)
					auto dis_t = ts.findDef(com_t, op)
					
					if (dis_t == NIL)
						throw SomeExceptionClassThatIveNotYetGottenAroundToDefining()

					// Using convertible check (Doesn't distinguish between the two sides)
						// This is the one I'm going to be using (Simpler and doesn't rely on a specific organization of values)
						// I could possibly remove the "convertibile" check depending on how forceType handles inheritance
						// Min: 2 bools, 0 func:							O(1), O(1)
						// Max: 3 bools, 2 func:							O(1), O(1), O(...), O(?), O(?)
					if ((com_t == l ^ com_t == r) && ts.convertible(l, r))
						forceType(-1, com_t);					// The name is a placeholder (Maybe move to the typesystem ???)
						forceType(-2, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary

					// Using isDefd check
						// Min: 1 bools, 0 func:							O(1)
						// Max: 5 bools, 1 func:							O(1), O(1), O(1), O(...), O(...), O(?)
					if (l != r)
						if (com_t == r && ts.isDefd(l, ts.get(r).name) != NIL)
							forceType(LHS, com_t);											// LHS and RHS are constants that refer to specific stack indices
						else if (com_t == l && ts.isDefd(r, ts.get(l).name) != NIL)
							forceType(RHS, com_t);

					// Algorithmic cost of convertible and isDefd is the same
						// Convertible size is O(n!), n = number of types
						// isDefd size is O(n), n = number of methods
						// But they both only use map::count

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
