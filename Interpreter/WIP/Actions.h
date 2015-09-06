#pragma once

#include "Grammar.h"

namespace dust {
	namespace interpreter {
		/*
		*  Parser Actions
		*/

		template <typename Rule>
		struct action : nothing<Rule> {};

		// Workspace
		template <> struct action<body> {
			static void apply(const input& in, AST& ast) {
				// Should I perform escaping here or at eval?
				ast.push(makeNode<Literal>(in.string(), TypeTraits<std::string>::id));
			}
		};

		// Literal Actions
		template <> struct action<decimal> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), TypeTraits<double>::id));
			}
		};

		template <> struct action<integer> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), TypeTraits<int>::id));
			}
		};

		template <> struct action<boolean> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>("1", TypeTraits<bool>::id));
			}
		};


		// Identifier Actions
		template <> struct action<var_id> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<VarName>(in.string()));
			}
		};


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
		template <> struct action<op_5> {
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
						throw std::string{ "Parsing error: Attempt to construct Operator node without an operator" };
				} else
					throw std::string{ "Parsing error: Attempt to construct Operator node with less than 2 nodes on the stack" };
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
																		// Guaranteed to be an operator

						op->addChild(ast.pop());						// Current ordering expected by operators (lhs, rhs)
						op->addChild(rhs);
						
						ast.push(op);

						// stack: ..., {op}
					} else
						throw std::string{ "Parsing error: Attempt to construct " + Node::node_type + " node without an operator" };
				} else
					throw std::string{ "Parsing error: Attempt to construct " + Node::node_type + " node with less than 3 nodes on the stack" };
			}
		};

		template <> struct action<ee_1> : ee_actions<Operator> {};
		template <> struct action<ee_2> : ee_actions<Operator> {};
		template <> struct action<ee_3> : ee_actions<Operator> {};
		template <> struct action<ee_4> : ee_actions<Operator> {};
		template <> struct action<ee_5> : ee_actions<Assign> {};				// ee_5 is Assignmnet, which needs and uses Assignment nodes. ee_acctions requires Operator nodes


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
						throw std::string{ "Parsing error: Attempt to construct heterogenous list" };		// or should i just quit execution here ???
				}

//				if (typed) ast.push(makeNode<Debug>(","));

				ast.push(list);
			}
		};

		template <> struct action<var_list> : list_actions<VarName> {};
		template <> struct action<expr_list> : list_actions<ASTNode> {};
		//template <> struct action<arg_list> : list_actions<TokenType::Arg, true> {};
		//template <> struct action<tbl_list> : list_actions<TokenType::Field> {};			// true?


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
	}

	template <typename Rule>
	struct action : interpreter::action<Rule> {};
}