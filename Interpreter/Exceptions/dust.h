#pragma once

#include "exceptions.h"

// Dust error classes and facilities

// For errors regarding dust behavior semantics

namespace dust {
	namespace error {

		class dust_error : public base {
			public:
			dust_error(const std::string& _m) : base{ _m } {}
			~dust_error() throw() {}
		};

		// For errors in dispatch/indexing (particularly method not found)
		class dispatch_error : public dust_error {
			public:
			dispatch_error(const std::string& _m) : dust_error{ _m } {}
			~dispatch_error() throw() {}
		};

		// Special case for converter methods
		class converter_not_found : public dispatch_error {
			public:
			converter_not_found(const std::string& _m) : dispatch_error{ _m } {}
			~converter_not_found() throw() {}
		};

		// For illegal / undefined dust operations
		class illegal_operation : public dust_error {
			public:
			illegal_operation(const std::string& _m) : dust_error{ _m } {}
			~illegal_operation() throw() {}
		};

		// For currently unimplemented operations
		class unimplemented_operation : public illegal_operation {
			public:
			unimplemented_operation(const std::string& _m) : illegal_operation{ _m } {}
			~unimplemented_operation() throw() {}
		};

	}
}