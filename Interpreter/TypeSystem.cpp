#include "TypeSystem.h"
#include "TypeTraits.h"
#include "Exceptions\logic.h"
#include <cctype>					// std::isupper in TypeSystem::newType

namespace dust {
	namespace type {
		using TypeVisitor = TypeSystem::TypeVisitor;

		TypeVisitor::TypeVisitor(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

		TypeVisitor& TypeVisitor::addOp(std::string op, Function f) {
			// Check if the function is a converter
			if (ts->type_id.count(op) > 0)
				ts->addConv(id, ts->type_id[op]);

			ts->types[id].ops[op] = f;
			return *this;
		}

		TypeVisitor::operator size_t() {
			return id;
		}


		convPair TypeSystem::key(size_t t1, size_t t2) {
			return convPair{ std::min(t1, t2), std::max(t1, t2) };
		}

		bool TypeSystem::isParentOf(size_t p, size_t t) {
			while (t != p && t > 0) t = types[t].parent;

			return t > Traits<Nil>::id;
		}

		size_t TypeSystem::ancestor(size_t l, size_t r) {
			while (l != 0) {
				if (isParentOf(l, r)) return l;
				l = types[l].parent;
			}

			return 1;				// Object is always an ancestor
		}

		/*/    See comment in TypeSystem.h
		size_t TypeSystem::convert(size_t from, size_t to) {
			auto idx = key(from, to);

			if (conv.count(idx) == 0) return type::Traits<Nil>::id;
			int id = conv[idx][conv[idx][0] == to ? 0 : 1];
			if (id == 0) return type::Traits<Nil>::id;

			return id;
		}
		//*/

		void TypeSystem::addConv(size_t from, size_t to) {
			auto idx = key(from, to);
			int sidx = 0;

			// Assign a default value to the array
			if (conv.count(idx) == 0) conv[idx][1] = NO_DEF;

			// Test for cases where the converter will have low precedence
			if (conv[idx][1] == from) {
				conv[idx][0] = conv[idx][1];
				sidx = 1;

			} else if (conv[idx][0] == from)
				sidx = 1;

			conv[idx][sidx] = to;
		}

		TypeVisitor TypeSystem::newType(std::string t, size_t p) {
			// Test that t is a type identifier
			if (!t.empty() && !std::isupper(t[0]))
				throw error::syntax_error{ t + " is not a valid dust type identifier" };

			// Test for new type definition
			if (type_id.count(t) == 0) {
				types.push_back({ t, types.size(), p });

				type_id[t] = types.size() - 1;						// Add a new name association

			} else {}												// This potentially allows for forward declarations, etc. with minimal code

			return TypeVisitor{ type_id[t], this };
		}

		TypeSystem::TypeSystem() {
			types.emplace_back("Nil", 0);
			types.emplace_back("Object", 1, 0);		// Should parent(Object) == Nil
			type_id["Nil"] = 0;
			type_id["Object"] = 1;
		}

		TypeVisitor TypeSystem::newType(std::string t) {
			return newType(t, 1);
		}

		TypeVisitor TypeSystem::newType(std::string t, Type& p) {
			return newType(t, p.id);
		}

		TypeVisitor TypeSystem::newType(std::string t, TypeVisitor& p) {
			return newType(t, (size_t)p);
		}

		size_t TypeSystem::findDef(size_t t, std::string fn) {
			t = (t == Traits<Nil>::id ? Traits<bool>::id : t);

			while (t != NO_DEF && isDefd(t, fn) == NO_DEF)
				t = types[t].parent;

			return t;
		}

		size_t TypeSystem::isDefd(size_t t, std::string fn) {
			return types[t].ops.count(fn) > 0 ? t : NO_DEF;
		}

		size_t TypeSystem::com(size_t l, size_t r, std::string op) {
			if (l == r) return l;																// Avoid memoizing ancestor(l, l) == l

			if (l == Traits<Nil>::id || r == Traits<Nil>::id) return Traits<Nil>::id;			// Handle Nil and Table cases
			if (l == getId("Table") || r == getId("Table")) return getId("Table");
			
			// Try for direct conversion
			auto idx = key(l, r);
			if (conv.count(idx) > 0) {							// If there is a defined conversion
				auto convs = conv[idx];

				if (findDef(convs[0], op) != NO_DEF)				// Test the highest precedence
					return convs[0];

				if (findDef(convs[1], op) != NO_DEF)				// Test the lowest precedence
					return convs[1];
			}

			// Give common ancestor
			return siblings.count(idx) == 0 ? siblings[idx] = ancestor(l, r) : siblings[idx];
		}

		size_t TypeSystem::com(Type& l, Type& r, std::string op) {
			return com(l.id, r.id, op);
		}

		TypeVisitor TypeSystem::getType(size_t t) {
			return{ t, this };
		}

		TypeVisitor TypeSystem::getType(std::string t) {
			return{ type_id[t], this };
		}

		Type TypeSystem::get(size_t t) {
			return types[t];
		}

		Type TypeSystem::get(std::string t) {
			return types[type_id[t]];
		}

		bool TypeSystem::convertible(size_t t1, size_t t2) {
			return conv.count(key(t1, t2));
		}

		bool TypeSystem::isChildType(size_t t1, size_t t2) {
			return isParentOf(t2, t1);
		}

		std::string TypeSystem::getName(size_t t) {
			return types[t].name;
		}

		size_t TypeSystem::getId(std::string t) {
			return type_id[t];
		}
	}
}
