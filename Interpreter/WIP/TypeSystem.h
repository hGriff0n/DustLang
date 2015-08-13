#pragma once

#include "Type.h"
#include <vector>

#include <iostream>

namespace dust {
	namespace impl {
		class TypeSystem {
			public:
				// A visitor interface to the type records that allows modifications to be performed on individual types in-place (modifications are maintained)
				struct Type {
					size_t id;
					TypeSystem* ts;

					Type(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

					Type& addOp(std::string op, Function f) {
						if (ts->type_id.count(op) > 0)
							ts->addConv(id, ts->type_id[op]);

						ts->types[id].ops[op] = f;

						return *this;
					}

					operator size_t() {
						return id;
					}
				};

				static const size_t NIL = -1;

			private:
				std::vector<impl::Type> types;							// Maintains type records, Indices are type_id
				std::map<convPair, std::array<size_t, 2>> conv;			// Tracks conversion precedence
				std::map<convPair, size_t> siblings;					// Memoize the ancestor of two types
				std::map<std::string, size_t> type_id;					// Associates name to type id

				// Generate a convPair key so that key(a, b) == key(b, a)
				convPair key(size_t t1, size_t t2) {
					return convPair{ std::min(t1, t2), std::max(t1, t2) };
				}

				// Determines if p is a parent of t (ie. p `isParentOf` t)
				bool isParentOf(size_t p, size_t t) {
					while (t != p && t != NIL)
						t = t > 0 ? types[t].parent : NIL;

					return t != NIL;
				}

				// Find the common ancestor of l and r
				size_t ancestor(size_t l, size_t r) {
					while (l != 0) {
						if (isParentOf(l, r)) return l;
						l = types[l].parent;
					}

					return 0;				// Object is always an ancestor
				}

				// Really only useful if I'm storing the conversion function in conv (I'm not though)
				size_t convert(size_t from, size_t to) {
					auto idx = key(from, to);

					if (conv.count(idx) == 0) return NIL;
					int id = conv[idx][conv[idx][0] == to ? 0 : 1];
					if (id == 0) return NIL;

					return id;
				}

				// Add a conversion to the registry while maintaining precedence levels (currently uses first definition)
				void addConv(size_t from, size_t to) {
					auto idx = key(from, to);
					int sidx = 0;

					// Assign a default value to the array ???
					// if (conv.count(idx) == 0) conv[idx][1] = NIL;

					if (conv[idx][1] == from) {
						conv[idx][0] = conv[idx][1];
						sidx = 1;
					} else if (conv[idx][0] == from)
						sidx = 1;

					conv[idx][sidx] = to;
				}

				// Create a Type visitor with the given parent
				Type newType(std::string t, size_t p) {
					if (type_id.count(t) == 0) {							// If the type is new
						types.push_back({ t, types.size(), p });
						//types.emplace_back(t, types.size(), p);

						type_id[t] = types.size() - 1;						// Add a new name association

					} else {}												// This potentially allows for forward declarations, etc. with minimal code

					return Type{ type_id[t], this };
				}

			public:
				TypeSystem() {
					types.emplace_back("Object", 0);
				}

				// Type Creation Methods
				Type newType(std::string t) {
					return newType(t, 0);
				}

				Type newType(std::string t, impl::Type& p) {
					return newType(t, p.id);
				}

				Type newType(std::string t, Type& p) {
					return newType(t, p.id);
				}


				// Inheritance Resolution (Find definition of op in the inheritance tree from t upwards)
				size_t findDef(size_t t, std::string op) {
					while (t != NIL && types[t].ops.count(op) == 0)
						t = t > 0 ? types[t].parent : NIL;

					return t;
				}


				// Common Type Resolution (Find a type that defines op and that both l and r can be cast to)
					// Need a way to signify whether a converter was selected (maybe move this logic out of TypeSystem, then I have to create duplicate indices)
				size_t com(size_t l, size_t r, std::string op) {
					// Try for same type (not strictly needed as ancestor(l, l) = l, however this isn't something that needs to be memoized)
					if (l == r) return l;

					// Try for direct conversion
					auto idx = key(l, r);
					if (conv.count(idx) > 0) {							// If there is a defined conversion
						auto convs = conv[idx];

						if (findDef(convs[0], op) != NIL)				// Test the first precedence
							return convs[0];

						if (findDef(convs[1], op) != NIL)				// Test the second precedence
							return convs[1];							// convs[1] defaults to 0 == Object.id
					}

					// Give common ancestor
					return siblings.count(idx) == 0 ? siblings[idx] = ancestor(l, r) : siblings[idx];
				}

				size_t com(dust::impl::Type& l, dust::impl::Type& r, std::string op) {
					return com(l.id, r.id, op);
				}


				// Temporary methods (may be expanded/grouped later)
				Type getType(size_t idx) {
					return{ idx, this };
				}

				Type getType(std::string t) {
					return getType(type_id[t]);
				}

				impl::Type get(size_t idx) {
					return types[idx];
				}

				impl::Type get(std::string t) {
					return get(type_id[t]);
				}

				bool immDef(size_t idx, std::string op) {
					return types[idx].ops.count(op);
				}
				
				// Testing methods

		};
	}
}
