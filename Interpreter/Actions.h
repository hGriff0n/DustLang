#pragma once

#include "Grammar.h"

#include "Exceptions\parsing.h"

namespace dust {
	namespace parse {
		/*
		*  Parser Actions
		*/

		template <typename Rule>
		struct action : nothing<Rule> {};

		// Workspace

		// Literal Actions
		template <> struct action<decimal> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), type::Traits<double>::id));
			}
		};

		template <> struct action<boolean> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(std::string{ in.string() == "true" ? "1" : "0" }, type::Traits<bool>::id));
			}
		};

		template <> struct action<integer> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), type::Traits<int>::id));
			}
		};

		template <> struct action<body> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(unescape(in.string()), type::Traits<std::string>::id));
			}
		};

		template <> struct action<k_nil> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>("0", type::Traits<bool>::id));
				//ast.push(makeNode<Literal>("", -1));
			}
		};


		// Identifier Actions
		template <> struct action<var_id> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<VarName>(in.string()));
			}
		};

		// Keyword Actions
		template <> struct action<k_and> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<BinaryKeyword>(in.string()));					// and/or are not becoming methods of Bool
			}
		};

		template <> struct action<k_or> : action<k_and> {};


		// Operator Actions
		struct OpAction {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Operator>("_op" + in.string()));
			}
		};

		template <> struct action<op_0> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Operator>("_ou" + in.string()));
			}
		};

		template <> struct action<op_1> : OpAction {};
		template <> struct action<op_2> : OpAction {};
		template <> struct action<op_3> : OpAction {};
		template <> struct action<op_4> : OpAction {};


		// Assignment Operators don't follow the same structure as Operators
		template <> struct action<op_x> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Assign>(in.string().substr(1)));
			}
		};


		// Atomic Actions
		template <> struct action<unary> {
			static void apply(input& in, AST& ast) {
				if (ast.size() >= 2) {
					if (std::dynamic_pointer_cast<Operator>(ast.at(-2)))
						// stack: ..., {op}, {operand}

						ast.at(-2)->addChild(ast.pop());

						// stack: ..., {op}
					else
						throw error::missing_node_x{ "Attempt to construct Operator node without an operator" };
				} else
					throw error::missing_nodes{ "Attempt to construct Operator node with less than 2 nodes on the stack" };
			}
		};

		// Expression Actions
		template <typename Node>
		struct ee_actions {
			static void apply(input& in, AST& ast) {
				if (ast.size() >= 3) {
					if (std::dynamic_pointer_cast<Node>(ast.at(-2))) {
						// stack: ..., {lhs}, {op}, {rhs}

						auto rhs = ast.pop();							// Doesn't check that rhs or lhs is valid
						auto op = ast.pop();							// However, rhs and lhs are guaranteed to be ASTNode's, which operator accepts

						op->addChild(ast.pop());						// Current ordering expected by operators (lhs, rhs)
						op->addChild(rhs);								// Assignment::addChild handles the Assignment specific node checks

						ast.push(op);

						// stack: ..., {op}
					} else
						throw error::missing_node_x{ "Attempt to construct " + Node::node_type + " node without a empty " + Node::node_type + " node" };
				} else
					throw error::missing_nodes{ "Attempt to construct " + Node::node_type + " node with less than 3 nodes on the stack" };
			}
		};

		template <> struct action<ee_1> : ee_actions<Operator> {};
		template <> struct action<ee_2> : ee_actions<Operator> {};
		template <> struct action<ee_3> : ee_actions<Operator> {};
		template <> struct action<ee_4> : ee_actions<Operator> {};
		template <> struct action<ee_5> : ee_actions<BinaryKeyword> {};
		template <> struct action<ee_6> : ee_actions<BinaryKeyword> {};
		template <> struct action<ee_x> : ee_actions<Assign> {};				// ee_x is Assignmnet, which needs and uses Assignment nodes. ee_acctions requires Operator nodes

		// List Actions
		template <class type>
		struct list_actions {
			static void apply(input& in, AST& ast) {
				auto list = makeNode<List<type>>();

				if (ast.at()->to_string() != ",")
					ast.push(makeNode<Debug>(","));

				while (!ast.empty() && ast.at()->to_string() == ",") {
					ast.pop();
					if (std::dynamic_pointer_cast<type>(ast.at()))
						list->addChild(ast.pop());
					else
						break;
						//throw std::string{ "Parsing error: Attempt to construct heterogenous list" };		// or should i just quit execution here ???
				}

				// if (typed) ast.push(makeNode<Debug>(","));

				ast.push(list);
			}
		};

		template <> struct action<var_list> : list_actions<VarName> {};
		template <> struct action<expr_list> : list_actions<ASTNode> {};

		// Block and Scoping Actions
			// A block is constructed "after" it's nodes
			// If an operation needs a block, it first needs to push a NEW_SCOPE debug node on the stack
		template <> struct action<block> {
			static void apply(input& in, AST& ast) {
				auto b = makeNode<Block>();
				bool block_node = false;

				while (!ast.empty() && !(block_node = ast.at()->to_string() == "NEW_SCOPE"))
					b->addChild(ast.pop());

				if (block_node) ast.pop();
				ast.push(b);
			}
		};

		// Other Actions
		template <> struct action<o_paren> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<Debug>("("));
			}
		};

		template <> struct action<c_paren> {
			static void apply(input& in, AST& ast) {
				ast.swap(); ast.pop();			// Might use some testing (empty parens)
			}
		};

		template <> struct action<comma> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<Debug>(in.string()));
			}
		};

		// Creating Tables ???
		template <> struct action<o_brack> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<Table>());
			}
		};

		template <> struct action<table> {
			static void apply(input& in, AST& ast) {
				if (std::dynamic_pointer_cast<Block>(ast.at()) && std::dynamic_pointer_cast<Table>(ast.at(-2)))
					ast.at(-2)->addChild(ast.pop());

				else if (!std::dynamic_pointer_cast<Table>(ast.at()))			// If not an empty table
					throw error::missing_node_x{ "Attempt to construct a Table node without a Table or Block node" };
			}
		};
	}

	template <typename Rule>
	struct action : parse::action<Rule> {};
}