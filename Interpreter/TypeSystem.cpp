#include "TypeSystem.h"
#include "TypeTraits.h"
#include "Table.h"
#include "Exceptions\logic.h"
#include <cctype>					// std::isupper in TypeSystem::newType

namespace dust {
	namespace type {
		using TypeVisitor = TypeSystem::TypeVisitor;

		TypeVisitor::TypeVisitor(size_t i, TypeSystem* self) : id{ i }, ts{ self } {}

		TypeVisitor& TypeVisitor::addOp(impl::Value op, impl::Value v) {
			ts->types[id].fields->getVar(op).val = v;
			return *this;
		}

		TypeVisitor& TypeVisitor::addOp(impl::Value op, impl::Value v, const std::string& fn) {
			if (ts->type_id.count(fn)) ts->addConv(id, ts->type_id[fn]);

			return addOp(op, v);
		}

		TypeVisitor::operator size_t() {
			return id;
		}


		ConvPair TypeSystem::key(size_t t1, size_t t2) {
			return ConvPair{ std::min(t1, t2), std::max(t1, t2) };
		}

		bool TypeSystem::isParentOf(size_t p, size_t t) {
			while (t != p && t > 0) t = types[t].parent;

			return t > Traits<Nil>::id;
		}

		size_t TypeSystem::ancestor(size_t l, size_t r) {
			while (l > 0) {
				if (isParentOf(l, r)) return l;
				l = types[l].parent;
			}

			return 1;				// Object is always an ancestor
		}

		void TypeSystem::addConv(size_t from, size_t to) {
			auto idx = key(from, to);
			int sidx = 0;

			// Assign a default value to the array
			if (conv.count(idx) == 0) conv[idx][1] = NO_DEF;

			// Test for cases where the converter will have low precedence
			if (conv[idx][1] == from) {
				return;

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
				types.emplace_back(Type{ t, types.size(), p });

				type_id[t] = types.size() - 1;						// Add a new name association

			} else {}												// This potentially allows for forward declarations, etc. with minimal code

			return TypeVisitor{ type_id[t], this };
		}

		TypeSystem::TypeSystem() {
			types.emplace_back("Nil", 0);
			types.emplace_back("Object", 1, 0);		// parent(Object) == Nil	Keep ???
			type_id["Nil"] = 0;
			type_id["Object"] = 1;
		}

		TypeVisitor TypeSystem::newType(std::string t) {
			return newType(t, 1);
		}

		TypeVisitor TypeSystem::newType(std::string t, const Type& p) {
			return newType(t, p.id);
		}

		TypeVisitor TypeSystem::newType(std::string t, TypeVisitor& p) {
			return newType(t, (size_t)p);
		}

		size_t TypeSystem::findDef(size_t t, const impl::Value& key) {
			t = (t == Traits<Nil>::id ? Traits<bool>::id : t);

			while (t != NO_DEF && isDefd(t, key) == NO_DEF)
				t = types[t].parent;

			return t;
		}

		size_t TypeSystem::isDefd(size_t t, const impl::Value& key) {
			return types[t].fields->hasKey(key) ? t : NO_DEF;
		}

		size_t TypeSystem::com(size_t l, size_t r, const impl::Value& op) {
			if (l == r) return l;																// Avoid memoizing ancestor(l, l) == l

			// Shortcut for table and Nil
			if (l == Traits<Table>::id || r == Traits<Table>::id) return Traits<Table>::id;		// com(Table, x, x) = Table
			if (l == Traits<Nil>::id || r == Traits<Nil>::id) return Traits<Nil>::id;			// com(Nil, x, x) = Nil (?)
			
			// Try for direct conversion
			auto idx = key(l, r);
			if (conv.count(idx) > 0)								// If there is a defined conversion, give the highest precedence
				return conv[idx][0];

			// Otherwise, give a common ancestor
			return siblings.count(idx) == 0 ? siblings[idx] = ancestor(l, r) : siblings[idx];
		}

		size_t TypeSystem::com(Type& l, Type& r, const impl::Value& op) {
			return com(l.id, r.id, op);
		}

		TypeVisitor TypeSystem::getType(size_t t) {
			return{ t, this };
		}

		TypeVisitor TypeSystem::getType(std::string t) {
			return{ type_id[t], this };
		}

		const Type& TypeSystem::get(size_t t) {
			return types[t];
		}

		const Type& TypeSystem::get(std::string t) {
			return types[type_id[t]];
		}

		bool TypeSystem::convertible(size_t f, size_t t) {
			auto conv_key = key(f, t);
			bool has_conversion = conv.count(conv_key);

			if (has_conversion) {
				auto pair = conv[conv_key];
				return pair[0] == t || pair[1] == t;
			}

			return false;
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

		// Temporary TDD method
		void TypeSystem::setMethods(size_t t, Table tbl) {
			types[t].fields = tbl;
			tbl = nullptr;
		}
	}
}
