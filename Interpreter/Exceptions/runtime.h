#pragma once

#include "exceptions.h"

// Runtime error classes and facilities

// For errors regarding C++ API runtime behavior

namespace dust {
	namespace error {

		class runtime_error : public base {
			public:
				runtime_error(const std::string& _m) : base{ _m } {}
				~runtime_error() throw() {}
		};

		// For errors regarding api_calls, etc.
		class bad_api_call : public runtime_error {
			public:
				bad_api_call(const std::string& _m) : runtime_error{ _m } {}
				~bad_api_call() throw() {}
		};
		
		// Faulty attempt to evaluate an ast node
		class bad_node_eval : public bad_api_call {
			public:
				bad_node_eval(const std::string& _m) : bad_api_call{ _m } {}
				~bad_node_eval() throw() {}
		};

		// For errors regarding the state of the stack
		class stack_state_error : public runtime_error {
			public:
				stack_state_error(const std::string& _m) : runtime_error{ _m } {}
				~stack_state_error() throw() {}
		};

		// Object on the stack is not of an expected type
		class stack_type_error : public stack_state_error {
			public:
				stack_type_error(const std::string& _m) : stack_state_error{ _m } {}
				~stack_type_error() throw() {}
		};
		
		// For errors regarding getting and setting garbage collected data
		class storage_access_error : public runtime_error {
			public:
				storage_access_error(const std::string& _fn, size_t ref) :
					runtime_error{ "Attempt to access invalid record " + std::to_string(ref) + " in " + _fn } {}
				~storage_access_error() throw() {}
		};

		// Attempt to use a nullptr str_record*
		class null_exception : public runtime_error {
			public:
				null_exception(const std::string& _m) : runtime_error{ _m } {}
				~null_exception() throw() {}
		};

		// For errors regarding value conversions
		class conversion_error : public runtime_error {
			public:
				conversion_error(const std::string& _fn, const std::string& _t) :
					runtime_error{ "Couldn't convert to " + _t + " in " + _fn } {}
				~conversion_error() throw() {}
		};

	}
}