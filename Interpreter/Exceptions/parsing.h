#pragma once

#include "exceptions.h"

// Parsing error classes and facilities

// Parser errors are for errors in parsing and/or ast construction

namespace dust {
	namespace error {

		// This doesn't quite capture pegtl::parse_error however
		class parse_error : public base {
			public:
				parse_error(const std::string& _m) : base{ _m } {}
				~parse_error() throw() {}
		};

		// When an ast node cannot be constructed due to lack of nodes
		class missing_nodes : public parse_error {
			public:
				missing_nodes(const std::string& _n, int num)
					: parse_error{ "Attempt to construct " + _n + " node with less than " + std::to_string(num) + " nodes on the stack" } {}
				~missing_nodes() throw() {}
		};

		// When an ast node cannot be constructed due to a lack of a specific node
		class missing_node_x : public parse_error {
			public:
				missing_node_x(const std::string& _n)
					: missing_node_x{ _n, _n } {}
				missing_node_x(const std::string& _n, const std::string& _nd)
					: parse_error{ "Attempt to construct " + _n + " node without a " + _nd + " node" } {}
				~missing_node_x() throw() {}
		};

		// Attempt to call addChild when the node cannot have children
		class operands_error : public parse_error {
			public:
				operands_error(const std::string& _m) : parse_error{ _m } {}
				~operands_error() throw() {}
		};

		// Attempt to construct an ast using nodes it cannot takes
		class invalid_ast_construction : public parse_error {
			public:
				invalid_ast_construction(const std::string& _m) : parse_error{ _m } {}
				~invalid_ast_construction() throw() {}
		};
		
	}
}