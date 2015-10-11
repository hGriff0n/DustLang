#pragma once

#include <stdexcept>

// File for basic exception classes and facilities for dealing with exceptions

// logic_error
	// out_of_bounds
	// illegal_template
	// syntax_error
// parse_error
	// missing_nodes
	// missing_node_x
	// operands_error
	// invalid_ast_construction
// runtime_error
	// bad_api_call
	// bad_node_eval
	// incomplete_node
	// stack_state_error
	// stack_type_error
	// storage_access_error
	// null_exception
	// conversion_error
// dust_error
	// dispatch_error
	// converter_not_found
	// illegal_operation
	// unimplemented_operation

namespace dust {
	namespace error {

		class base : public std::exception {
			private:
				const std::string msg;

			public:
				base(const std::string& _m) : msg{ _m } {}
				~base() throw() {}
				virtual const char* what() const throw() { return msg.c_str(); }
		};

	}
}