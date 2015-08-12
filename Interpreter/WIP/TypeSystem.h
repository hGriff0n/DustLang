#pragma once

#include "Type.h"
#include <vector>

#include <iostream>

namespace dust {
	namespace impl {
		class TypeSystem {
			public:
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

					operator impl::Type() {
						return ts->types[id];
					}
				};

			private:
				static const size_t NO_CONVERSION = -1;		// or 0

				std::vector<impl::Type> types;
				std::map<convPair, std::array<size_t, 2>> conv;
				std::map<std::string, size_t> type_id;

				convPair key(size_t t1, size_t t2) {
					return t1 < t2 ? convPair{ t1, t2 } : convPair{ t2, t1 };

					//return convPair{ std::min(t1, t2), std::max(t1, t2) };
				}

				size_t commonType(size_t l, size_t r) {
					if (l == r) return l;

					auto idx = key(l, r);

					if (conv.count(idx) == 0) return NO_CONVERSION;

					return conv[idx][0];
				}

				// Really only useful if I'm storing the conversion function in conv
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

				Type newType(std::string t, size_t p) {
					if (type_id.count(t) == 0) {							// If the type is new
						types.push_back({ t, types.size(), p });
						//types.emplace_back(t, types.size(), p);

						type_id[t] = types.size() - 1;

					} else {}												// Possible way of allowing forward declarations ???

					return Type{ type_id[t], this };
				}

			public:
				TypeSystem() {
					types.emplace_back("Object", 0);
				}

				// Create new types
				Type newType(std::string t) {
					return newType(t, 0);
				}

				Type newType(std::string t, impl::Type& p) {
					return newType(t, p.id);
				}

				Type newType(std::string t, Type& p) {
					return newType(t, p.id);
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
				size_t com(size_t l, size_t r, std::string op) {
					auto c_type = commonType(l, r);

					if (c_type == NO_CONVERSION) throw std::string{ "No conversion between " + types[l].name + " and " + types[r].name };

					// if (t == 0) (conv.lub currently throws an error if there is no direct conversion)
					// Should this take into account inheritance conversions (ie. Int + String would translate to String(Number(Int)) + String if Number.String and !Int.String)

					try {
						findDef(c_type, op);				// Check if the common type has the operator declared
						return c_type;						// Uses try-check since findDef throws an exception when the inheritance tree is used (unfortunately)
					} catch (std::string& e) {}

					return l + r - c_type;					// Assume the other type has the operator declared (This will raise an error in dispatch)
				}

				size_t com(dust::impl::Type& l, dust::impl::Type& r, std::string op) {
					return com(l.id, r.id, op);
				}

				size_t com(Type& l, Type& r, std::string op) {
					return com(l.id, r.id, op);
				}


				// Temporary methods (may be expanded later)
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
		};
	}
}
