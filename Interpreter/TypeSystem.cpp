#include "TypeSystem.h"
#include "Exceptions\logic.h"
#include <cctype>

namespace dust {
	namespace type {
		using TypeVisitor = TypeSystem::TypeVisitor;

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
			while (t != p && t != NIL)
				t = t > 0 ? types[t].parent : NIL;

			return t != NIL;
		}

		size_t TypeSystem::ancestor(size_t l, size_t r) {
			while (l != 0) {
				if (isParentOf(l, r)) return l;
				l = types[l].parent;
			}

			return 0;				// Object is always an ancestor
		}

		/*/    See comment in TypeSystem.h
		size_t TypeSystem::convert(size_t from, size_t to) {
			auto idx = key(from, to);

			if (conv.count(idx) == 0) return NIL;
			int id = conv[idx][conv[idx][0] == to ? 0 : 1];
			if (id == 0) return NIL;

			return id;
		}
		//*/

		void TypeSystem::addConv(size_t from, size_t to) {
			auto idx = key(from, to);
			int sidx = 0;

			// Assign a default value to the array
			if (conv.count(idx) == 0) conv[idx][1] = NIL;

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
			types.emplace_back("Object", 0);
		}

		TypeVisitor TypeSystem::newType(std::string t) {
			return newType(t, 0);
		}

		TypeVisitor TypeSystem::newType(std::string t, Type& p) {
			return newType(t, p.id);
		}

		TypeVisitor TypeSystem::newType(std::string t, TypeVisitor& p) {
			return newType(t, (size_t)p);
		}

		size_t TypeSystem::findDef(size_t t, std::string fn) {
			while (t != NIL && isDefd(t, fn) == NIL)
				t = t > 0 ? types[t].parent : NIL;

			return t;
		}

		size_t TypeSystem::isDefd(size_t t, std::string fn) {
			return types[t].ops.count(fn) > 0 ? t : NIL;
		}

		size_t TypeSystem::com(size_t l, size_t r, std::string op) {
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