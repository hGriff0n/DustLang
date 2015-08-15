#pragma once

#include "Type.h"
#include <vector>

#include <iostream>

namespace dust {
	namespace impl {

		class TypeSystem {
			public:
				// A visitor interface to the type records that allows modifications on individual types to be maintained after scope exit
				struct TypeVisitor {
					size_t id;
					TypeSystem* ts;

					TypeVisitor(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

					TypeVisitor& addOp(std::string, Function);
					
					operator size_t();
				};

				static const size_t NIL = -1;

			private:
				std::vector<impl::Type> types;							// Maintains type records, Indices are type_id
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

				// Type Visitor Creation Methods
				TypeVisitor newType(std::string);

				TypeVisitor newType(std::string, Type&);

				TypeVisitor newType(std::string, TypeVisitor&);


				// Inheritance Resolution (Find definition of a field in the inheritance tree given a starting type)
				size_t findDef(size_t, std::string);


				// Common Type Resolution (Find a type that defines op and that both l and r can be cast to)
					// Need a way to signify whether a converter was selected (maybe move this logic out of TypeSystem, then I have to create duplicate indices)
					// I actually don't need to provide this information explicitly (see explanation below). However, it could be beneficial to do so
				size_t com(size_t, size_t, std::string);
				//size_t com(size_t, size_t, std::string, bool&);
				//bool com(size_t, size_t, std::string, size_t&);

				// The key differentiator between a conversion relationship and an inheritance relationship is the presence of a converter
					// Therefore, performing a `immDef` check for a converter (in the unselected type) should be enough to resolve the action
						// Actually only a `convertible` check of (l, r) is needed (if they aren't convertible, then the return is an ancestor)
					// This check would only need to be performed when t == l xor t == r (However, it would be simpler if a boolean flag was added)

				size_t com(Type&, Type&, std::string);


				// Temporary methods (may be expanded/grouped later)
				TypeVisitor getType(size_t);

				TypeVisitor getType(std::string);

				Type get(size_t);

				Type get(std::string);

				bool immDef(size_t, std::string);

				bool convertible(size_t, size_t);
				
				// Testing methods

		};
	}
}
