#pragma once

#include "Type.h"
#include <vector>

size_t findDef(dust::impl::Type&, std::string, std::vector<dust::impl::Type>&);
size_t findDef(size_t, std::string, std::vector<dust::impl::Type>&);

namespace dust {
	namespace impl {
		class TypeSystem {
			private:
				std::vector<impl::Type> types;
				ConvTracker conv;

				//bool isConverter(std::string);
				void addConv(size_t from, size_t to) {
					conv.add(types[from], types[to]);
				}
			public:
				TypeSystem() {
					types.emplace_back("Object", 0);
				}

				// Structure to allow procedural loading of type operations
				struct Type {
					size_t id;
					TypeSystem* ts;

					Type(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

					Type& addOp(std::string op, Function f) {
						ts->types[id].ops[op] = f;

						// if (ts->isConverter(op))
							// ts->addConv(id, type_id(op));

						return *this;
					}

					Type& addConv(Type& t) {
						ts->addConv(id, t.id);
						return *this;
					}

					operator impl::Type() {
						return ts->types[id];
					}
				};

				// Create new types
				Type newType(std::string t) {
					return newType(t, types[0]);
				}
				Type newType(std::string t, impl::Type& p) {
					types.push_back({ t, types.size(), p });
					//types.emplace_back(t, types.size(), p);

					return Type{ types.size() - 1, this };
				}
				Type newType(std::string t, Type& p) {
					types.push_back({ t, types.size(), p.id });
					return Type{ types.size() - 1, this };
				}

				// Temporary methods (may be expanded later)
				Type getType(size_t idx) {
					return{ idx, this };
				}
				impl::Type _get(size_t idx) {
					return types[idx];
				}

				// Find a definition for op in the inheritance tree from t upwards
				size_t findDef(Type& t, std::string op) {
					return findDef(t.id, op);
				}
				size_t findDef(size_t t, std::string op) {
					auto err_t = t;

					while (types[t].ops.count(op) == 0)
						t = t > 0 ? types[t].parent : throw std::string{ "Dispatch error: " + types[err_t].name + "." + op + " not defined" };

					return t;
				}

				// Find the common type of l and r
				size_t lub(size_t l, size_t r, std::string op) {
					return lub(types[l], types[r], op);
				}
				size_t lub(dust::impl::Type& l, dust::impl::Type& r, std::string op) {
					auto t = conv.lub(l, r);

					// if (t == 0) (conv.lub currently throws an error if there is no direct conversion)
					// Should this take into account inheritance conversions (ie. Int + String would translate to String(Number(Int)) + String if Number.String and !Int.String)

					try {
						findDef(t, op);						// Check if the common type has the operator declared
						return t;							// Uses try-check since findDef throws an exception when the inheritance tree is used (unfortunately)
					} catch (std::string& e) {}

					return l.id + r.id - t;					// Assume the other type has the operator declared (This will raise an error in dispatch)
				}
				size_t lub(Type& l, Type& r, std::string op) {
					return lub(types[l.id], types[r.id], op);
				}
		};
	}
}


/*/
// Move type system functions into TypeSystem class
Type newType(std::string);
Type newType(std::string, impl::Type& p);
Type newType(std::string, Type& p);
Type newType(std::string, size_t);

size_t lub(size_t, size_t, std::string);
size_t lub(dust::impl::Type&, dust::impl::Type&, std::string);

size_t findDef(size_t, std::string, std::vector<dust::impl::Type>&);
size_t findDef(dust::impl::Type&, std::string, std::vector<dust::impl::Type>&);
};

impl::Type getType(std::string t) {
return types[type_id[t]];
}

int dispatch(impl::Type t, std::string op) {
while (t.ops.count(op) == 0)
t = t.id > 0 ? types[t.parent] : throw std::string{ "Dispatch error" };

return 0;
//return t.ops[op](*this);					// I use this (similar) code in the current production !!!!!
}

impl::Type dispatch_(impl::Type t, std::string op) {
while (t.ops.count(op) == 0)
t = t.id > 0 ? types[t.parent] : throw std::string{ "Dispatch error" };

return t;
}
};
//*/