#pragma once

#include <map>
#include <string>
#include <functional>

namespace dust {
	class EvalState;
	typedef std::function<int(EvalState&)> Function;

	// namespace impl { struct Value; }

	namespace type {
		// struct Value;
		class TypeSystem;

		struct Type {
			std::string name;
			size_t id, parent;
			std::map<std::string, Function> ops;
			//std::map<std::string, Value> fields;		// Replaces ops (Do I need to keep track of fields ???)
				
			// Default values and typed/const variables ???

			Type(std::string t, size_t s) : Type(t, s, -1) {}
			Type(std::string t, size_t s, size_t p) : name{ t }, id{ s }, parent{ p } {}
			Type(std::string t, size_t s, Type p) : Type(t, s, p.id) {}
		};

		// std::less<T> uses operator< by default
		struct convPair : std::pair<size_t, size_t> {
			convPair(size_t t1, size_t t2) : pair{ t1, t2 } {}
			convPair(Type& t1, Type& t2) : pair{ t1.id, t2.id } {}

			bool operator<(convPair& rhs) {
				return first < rhs.first && second < rhs.second;
			}
		};
	}
}
