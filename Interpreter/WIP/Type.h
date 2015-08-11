#pragma once

#include <map>
#include <array>
#include <string>
#include <functional>

namespace dust {
	class EvalState;
	typedef std::function<int(EvalState&)> Function;

	namespace impl {
		class TypeSystem;

		struct Type {
			std::string name;
			size_t id, parent;									//std::vector<int> parents for multiple inheritance
			std::map<std::string, Function> ops;
			//std::map<std::string, impl::Value> fields;		// Replaces ops (Do I need to keep track of fields ???)

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

		class ConvTracker {
			using key_type = convPair;
			std::map<key_type, std::array<size_t, 2>> conv;

			key_type key(Type& t1, Type& t2) {
				return t1.id < t2.id ? convPair{ t1.id, t2.id } : convPair{ t2.id, t1.id };
			}

			public:
				// Common Type conversion
				size_t lub(Type& l, Type& r) {
					auto idx = key(l, r);

					if (conv.count(idx) == 0) throw std::string{ "No Conversion between " + l.name + " and " + r.name };
					//if (conv.count(idx) == 0) return 0;		// Common Type is Object

					return conv[idx][0];
				}

				// From -> To conversion (This function is really only useful if ConvTracker stores the conversion function)
				size_t convert(Type& from, Type& to) {
					auto idx = key(from, to);

					if (conv.count(idx) == 0) goto noConv;

					int id = conv[idx][conv[idx][0] == to.id ? 0 : 1];
					if (id == 0) goto noConv;

					return id;

				noConv:
					throw std::string{ "No Conversion from " + from.name + " to " + to.name };
				}

				void add(Type& from, Type& to) {
					auto idx = key(from, to);
					int sidx = 0;										// No conversion || to -> from not defined

					if (conv[idx][1] == from.id) {						// to -> from defined at index 1
						conv[idx][0] = conv[idx][1];					// new conversion precedence
						sidx = 1;

					} else if (conv[idx][0] == from.id)					// to -> from defined at index 0
						sidx = 1;

					conv[idx][sidx] = to.id;
				}
		};
	}
}
