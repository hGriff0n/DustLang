#pragma once

#include <map>
#include <array>
#include <string>
#include <functional>

struct dust::impl::Type;

namespace std {
	template<> struct less<std::pair<dust::impl::Type, dust::impl::Type>> {
		using argType = std::pair<dust::impl::Type, dust::impl::Type>;

		bool operator()(const argType& lhs, const argType& rhs) {
			return lhs.first.id < rhs.first.id && lhs.second.id < rhs.second.id;
		}
	};
}

namespace dust {
	class EvalState;
	typedef std::function<int(EvalState&)> Function;

	namespace impl {
		struct Type {
			std::string name;
			size_t id, parent;									//std::vector<int> parents for multiple inheritance
			std::map<std::string, Function> ops;
			//std::map<std::string, impl::Value> fields;		// Replaces ops

			Type(std::string t, size_t s) : Type(t, s, -1) {}
			Type(std::string t, size_t s, size_t p) : name{ t }, id{ s }, parent{ p } {}
			Type(std::string t, size_t s, Type p) : Type(t, s, p.id) {}

			//operator int() { return id; }
		};

		struct ConvTracker {
			using storage = Type;
			using key_type = std::pair<Type, Type>;					// key_type = std::pair<int, int> (but then I'd be specializing less<std::pair<int, int>>)
			std::map<key_type, std::array<storage, 2>> conv;		// I don't know what I want to store in the array

			key_type key(Type t1, Type t2) {
				return t1.id < t2.id ? std::make_pair(t1, t2) : std::make_pair(t2, t1);
				//return t1.id < t2.id ? std::make_pair(t1.id, t2.id) : std::make_pair(t2.id, t1.id);
			}

			// Common Type conversion
			storage lub(Type t1, Type t2) {
				auto idx = key(t1, t2);

				if (conv.count(idx) == 0) throw std::string{ "No Conversion between " + t1.name + " and " + t2.name };

				return conv[idx][0];											// Select the highest precedence conversion
			}

			// To-From conversion
			storage getConv(Type from, Type to) {
				auto idx = key(from, to);

				if (conv.count(idx) == 0) throw std::string{ "No Conversion from " + from.name + " to " + to.name };

				return conv[idx][(conv[idx][0].id == to.id ? 0 : 1)];
			}

			// Add new conversion (maintains precedence ordering)
			void add(Type from, Type to) {
				auto idx = key(from, to);
				int sidx = 0;															// No conversion or to -> from not defined

				if (conv[idx][1].id == from.id) {										// to -> from defined at index 1
						conv[idx][0] = conv[idx][1];
						sidx = 1;

				} else if (conv[idx][0].id == from.id)									// to -> from defined at index 0
					sidx = 1;

				conv[idx][sidx] = to;
			}
		};
	}
}
