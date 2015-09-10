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

		// Attempt to evaluate a node with the node in an incomplete state
		class incomplete_node : public bad_api_call {
			public:
			incomplete_node(const std::string& _m) : bad_api_call{ _m } {}
			~incomplete_node() throw() {}
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
			storage_access_error(const std::string& _m) : runtime_error{ _m } {}
			~storage_access_error() throw() {}
		};

		// Attempt to use a nullptr str_record*
		class null_exception : public storage_access_error {
			public:
			null_exception(const std::string& _m) : storage_access_error{ _m } {}
			~null_exception() throw() {}
		};

		// For errors regarding value conversions
		class conversion_error : public runtime_error {
			public:
			conversion_error(const std::string& _m) : runtime_error{ _m } {}
			~conversion_error() throw() {}
		};

	}
}