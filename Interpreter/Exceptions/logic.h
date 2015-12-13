#pragma once

#include "exceptions.h"

// Logic error exception classes and facilities

// For errors regarding C++ program logic

namespace dust {

	namespace error {

		class logic_error : public base {
			public:
				logic_error(const std::string& _m) : base{ _m } {}
				~logic_error() throw() {}
		};

		// For indexing arrays/vector/stacks out of bounds
		class out_of_bounds : public logic_error {
			public:
				out_of_bounds(const std::string& _fn, int idx, size_t siz) : out_of_bounds{ _fn, (size_t)idx, siz } {}
				out_of_bounds(const std::string& _fn, size_t idx, size_t siz)
					: logic_error{ "Attempt to index container of size " + std::to_string(siz) + " with index " + std::to_string(idx) + " in " + _fn } {}
				~out_of_bounds() throw() {}
		};

		// Attempt to use an illegal template definition
		class illegal_template : public logic_error {
			public:
				illegal_template(const std::string& _m, const std::type_info& _t) : logic_error{ _m + _t.name() } {}
				~illegal_template() throw() {}
		};

		// Attempt to use invalid dust syntax in C++ API calls
		class syntax_error : public logic_error {
			public:
				syntax_error(const std::string& _m) : logic_error{ _m } {}
				~syntax_error() throw() {}
		};

	}

}