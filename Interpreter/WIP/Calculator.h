#pragma once

#include <pegtl.hh>
#include <iostream>

#include "ast.h"
#include "state.h"

namespace calculator {
	using namespace pegtl;		// I think I can "redefine" elements of the PEGTL library by "overloading" them here (see identifier)
								// Scope lookup checks here first I think.

	using AST = std::stack<std::shared_ptr<ASTNode>>;		// Would STATE be a better name (might have to adjust to use pointers)
	using input = pegtl::input;				// For some reason, Intellisense was complaining input was "ambiguous"

	struct expr;
	struct sign : one<'+', '-'> {};

	// Comments (slightly hackish currently)
	struct comment : if_must<two<'#'>, until<eolf>> {};								// ##[^{eolf}]*

	// Specifying literals
	struct integer : seq<opt<sign>, plus<digit>> {};								// (+|-)?[0-9]+
	struct decimal : seq<opt<sign>, plus<digit>, one<'.'>, opt<plus<digit>>> {};	// (+|-)?[0-9]+\.[0-9]+
	struct number : sor<decimal, integer> {};										// {decimal}|{integer}

	// Specifying syntax
	struct op : one<'+', '*', '-', '/', '=', '<', '>'> {};
	struct un_op : seq<op, expr> {};										// {op}{expr}
	struct bin_op : seq<number, pad<op, one<' '>>, number> {};				// {number} *{op} *{number}

	// Specifying organization
	struct expr : sor<bin_op, seq<star<space>, comment>> {};						// need to be able to recognize number and bin_op

	struct grammar : must<plus<expr>, star<space>, eof> {};							// accepts multiple lines and then quits (allows trailing whitespace)



	// Specifying parsing actions
	template <typename Rule>
	struct action : nothing<Rule> {};

	template<> struct action<comment> {
		static void apply(const input& in, AST& ast) {
			std::cout << "Comment:" << in.string().substr(2) << "\n";				// Could still use trimming
		}
	};

	template<> struct action<integer> {
		static void apply(const input& in, AST& ast) {
			ast.push(std::make_shared<Literal>(in.string() + " ", ValType::INT));		// What do I want to store in the nodes?  (Node Data, Location, ...)
		}
	};

	template <> struct action<decimal> {
		static void apply(const input& in, AST& ast) {
			ast.push(std::make_shared<Literal>(in.string() + " ", ValType::FLOAT));
		}
	};

	template <> struct action<op> {
		static void apply(const input& in, AST& ast) {
			ast.push(std::make_shared<BinOp>(in.string()));			// Leave the "raw" Node on the stack for later processing
		}
	};

	//*
	// Also look at the shunting yard algorithm

	template <> struct action<bin_op> {
		static void apply(const input& in, AST& ast) {
			// stack: ..., {lhs}, {op}, {rhs}

			auto rhs = node(ast);
			auto op = node(ast);
			//op->addChild(node(ast)).addChild(rhs);			// Depends on how the argument "passing" is handled (this line or the next line)
			op->addChild(rhs).addChild(node(ast));
			ast.push(op);										// Don't replace op with the above line. push expects a shared_ptr<ASTNode> not an ASTNode

			// stack: ..., {op}
		}
	}; 
	//*/

	template <> struct action<expr> {
		static void apply(const input& in, AST& ast) {
			auto n_expr = std::make_shared<Expression>();

			// Will need to correct the implementation (too greedy currently)
				// Perhaps use "shift-reduce" to construct the object
				// Currently also produces slightly "non-optimal" trees (can be compacted further)

			while (ast.size() > 0)
				n_expr->addChild(node(ast));

			ast.push(n_expr);
		}
	};
}
