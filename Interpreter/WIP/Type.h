#pragma once

#include <map>
#include <string>
#include <functional>

namespace dust {
	class EvalState;
	typedef std::function<int(EvalState&)> Function;

	namespace impl {
		struct Type {
			int id;
			std::string name;
			std::map<std::string, Function> operations;
			//std::map<std::string, impl::Value> fields;		// Replaces operations
			//std::vector<int> parents;

			Type(std::string t, int s) : name{ t }, id { s } {}
		};
	}
}
