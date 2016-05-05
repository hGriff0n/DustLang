#pragma once

#include <map>
#include <string>
#include <functional>

#include "Table.h"

namespace dust {
	class EvalState;
	using NativeFn = std::function<int(EvalState&)>;

	namespace type {

		/*
		 * Basic struct to hold information of dust types
		 */
		struct Type {
			std::string name;
			size_t id, parent;

			// Internal storage of type methods
			Table fields;
			//impl::Value ref;
				
			// Default values and typed/const variables ???

			Type(std::string t, size_t s) : Type(t, s, -1) {}
			Type(std::string t, size_t s, size_t p) : name{ t }, id{ s }, parent{ p }, fields{ new impl::Table{} } {}
			Type(std::string t, size_t s, Type p) : Type(t, s, p.id) {}
			Type(Type&& t) : name{ std::move(t.name) }, id{ t.id }, parent{ t.parent }, fields{ t.fields } {
				t.fields = nullptr;
			}

			~Type() {
				delete fields;
			}
		};

		/*
		 * Special class used to store two types as keys in a map
		 * overrides operator< for use by std::less<T>
		 */
		struct ConvPair : std::pair<size_t, size_t> {
			ConvPair(size_t t1, size_t t2) : pair{ t1, t2 } {}
			ConvPair(Type& t1, Type& t2) : pair{ t1.id, t2.id } {}

			bool operator<(ConvPair& rhs) {
				return first < rhs.first && second < rhs.second;
			}
		};
	}
}
