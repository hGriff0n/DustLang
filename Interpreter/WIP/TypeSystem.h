#pragma once

#include "Type.h"
#include <vector>

namespace dust {
	namespace impl {
		class TypeSystem {
			private:
				static const size_t NO_CONVERSION = -1;		// or 0

				std::vector<impl::Type> types;
				std::map<convPair, std::array<size_t, 2>> conv;

				convPair key(size_t t1, size_t t2) {
					return t1 < t2 ? convPair{ t1, t2 } : convPair{ t2, t1 };
				}

				size_t commonType(size_t l, size_t r) {
					auto idx = key(l, r);

					if (conv.count(idx) == 0) return NO_CONVERSION;

					return conv[idx][0];
				}

				// How is this useful???
				size_t convert(size_t from, size_t to) {
					auto idx = key(from, to);

					if (conv.count(idx) == 0) return NO_CONVERSION;
					int id = conv[idx][conv[idx][0] == to ? 0 : 1];
					if (id == 0) return NO_CONVERSION;

					return id;
				}

				void addConv(size_t from, size_t to) {
					auto idx = key(from, to);
					int sidx = 0;

					if (conv[idx][1] == from) {
						conv[idx][0] = conv[idx][1];
						sidx = 1;
					} else if (conv[idx][0] == from)
						sidx = 1;

					conv[idx][sidx] = to;
				}

			public:
				TypeSystem() {
					types.emplace_back("Object", 0);
				}

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


				// Find a definition for op in the inheritance tree from t upwards
				size_t findDef(size_t t, std::string op) {
					auto err_t = t;

					while (types[t].ops.count(op) == 0)
						t = t > 0 ? types[t].parent : throw std::string{ "Dispatch error: " + types[err_t].name + "." + op + " not defined" };

					return t;
				}

				size_t findDef(Type& t, std::string op) {
					return findDef(t.id, op);
				}


				// Find the common type of l and r
				size_t lub(size_t l, size_t r, std::string op) {
					auto c_type = commonType(l, r);

					// if (t == 0) (conv.lub currently throws an error if there is no direct conversion)
					// Should this take into account inheritance conversions (ie. Int + String would translate to String(Number(Int)) + String if Number.String and !Int.String)

					try {
						findDef(c_type, op);				// Check if the common type has the operator declared
						return c_type;						// Uses try-check since findDef throws an exception when the inheritance tree is used (unfortunately)
					} catch (std::string& e) {}

					return l + r - c_type;					// Assume the other type has the operator declared (This will raise an error in dispatch)
				}

				size_t lub(dust::impl::Type& l, dust::impl::Type& r, std::string op) {
					return lub(l.id, r.id, op);
				}

				size_t lub(Type& l, Type& r, std::string op) {
					return lub(l.id, r.id, op);
				}


				// Temporary methods (may be expanded later)
				Type getType(size_t idx) {
					return{ idx, this };
				}

				impl::Type get(size_t idx) {
					return types[idx];
				}
		};
	}
}
