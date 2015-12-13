#pragma once

#include <map>
#include <string>
#include <functional>

#include "Table.h"

namespace dust {
	class EvalState;
	typedef std::function<int(EvalState&)> Function;

	namespace type {

		/*
		 * Basic struct to hold information of dust types
		 */
		struct Type {
			std::string name;
			size_t id, parent;

			// Internal storage of type methods
			std::map<std::string, Function> ops;				// Temporary fix
			Table _ops_;										// Future implementation
				
			// Default values and typed/const variables ???

			Type(std::string t, size_t s) : Type(t, s, -1) {}
			Type(std::string t, size_t s, size_t p) : name{ t }, id{ s }, parent{ p }, _ops_{ nullptr } {}
			Type(std::string t, size_t s, Type p) : Type(t, s, p.id) {}
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
