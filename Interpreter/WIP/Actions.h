#pragma once

#include "Grammar.h"

namespace dust {
	namespace impl {
		/*
		*  Parser Actions
		*/

		template <typename Rule>
		struct action : nothing<Rule> {};

		// Workspace
		template <> struct action<body> {
			static void apply(const input& in, AST& ast) {
				// Should I perform escaping here or at eval?
				ast.push(makeNode<Literal>(in.string(), ValType::STRING));
			}
		};

		// Literal Actions
		template <> struct action<decimal> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), ValType::FLOAT));
			}
		};

		template <> struct action<integer> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>(in.string(), ValType::INT));
			}
		};

		template <> struct action<boolean> {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<Literal>("1", ValType::BOOL));
			}
		};


		// Identifier Actions
		template <> struct action<var_id> {
			static void apply(input& in, AST& ast) {
				ast.push(makeNode<Variable>(in.string()));
			}
		};


		// Operator Actions
		template <typename T>
		struct OpAction {
			static void apply(const input& in, AST& ast) {
				ast.push(makeNode<T>(in.string()));
			}
		};

		template <> struct action<op_0> : OpAction<UnOp> {};
		template <> struct action<op_1> : OpAction<BinOp> {};
		template <> struct action<op_2> : OpAction<BinOp> {};
		template <> struct action<op_3> : OpAction<BinOp> {};
		template <> struct action<op_4> : OpAction<BinOp> {};
		template <> struct action<op_5> : OpAction<Assignment> {};

		// Atomic Actions
		template <> struct action<unary> {
			// See ee_actions for reasoning?
			static void apply(input& in, AST& ast) {
				if (ast.size() >= 2) {
					auto operand = ast.pop();

					if (ast.top()->token_type() == TokenType::Operator)
						ast.top()->addChild(operand);
					else
						ast.push(operand);
				}
			}
		};


		// Expression Actions
		// Consider formalizing a shift-reduce framework here
		struct ee_actions {
			static void apply(input& in, AST& ast) {
				// stack: ..., {lhs}, {op}, {rhs}
				if (ast.size() >= 3) {
					auto rhs = ast.pop();							// Don't check that the rhs is valid
					auto op = ast.pop();							// Don't check that the operator is valid
																	// Don't check that the lhs is valid

					op->addChild(rhs).addChild(ast.pop());				// Current ordering expected by operators
																		//op->addChild(ast.pop()).addChild(rhs);			// If I change the ordering
					ast.push(op);
				}
				// stack: ..., {op}
			}
		};

		template <> struct action<ee_1> : ee_actions {};
		template <> struct action<ee_2> : ee_actions {};
		template <> struct action<ee_3> : ee_actions {};
		template <> struct action<ee_4> : ee_actions {};
		template <> struct action<ee_5> : ee_actions {};


		// List Actions
		template <TokenType t, bool only = false> struct list_actions {
			static void apply(input& in, AST& ast) {
				auto list = makeNode<List>(t);
				bool typed = false;

				//if (ast.top()->to_string() == ",") ast.pop();				// Old code to handle two commas on stack top

				if (ast.top()->to_string() != ",")
					ast.push(makeNode<Debug>(","));

				while (!ast.empty() && ast.top()->to_string() == ",") {
					ast.pop();
					if (!only || ast.top()->token_type() == t) list->addChild(ast.pop());
					else typed = true;
				}

				if (typed) ast.push(makeNode<Debug>(","));

				ast.push(list);
			}
		};

		template <> struct action<var_list> : list_actions<TokenType::Variable, true> {};
		template <> struct action<expr_list> : list_actions<TokenType::Expr> {};
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
}