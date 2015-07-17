#pragma once

#include <pegtl.hh>
#include <iostream>

#include "ast.h"
#include "state.h"			// Include the stack in here ???

// doesn't have operator precedence (how to avoid left recursion)
// also need to change the calling convention (for unary operators)

// For dust I should probably split the grammar and actions into seperate files
// For quick testing of grammar, pegjs.org/online

namespace calculator {
	using namespace pegtl;

	using AST = std::stack<std::shared_ptr<ASTNode>>;		// Would STATE be a better name (might have to adjust to use pointers)
	using input = pegtl::input;
	using eps = pegtl::success;

	struct expr;
	struct unary;

	struct sep : star<one<' '>> {};

	struct comment : if_must<two<'#'>, until<eolf>> {};										// ##.*
	struct integer : plus<digit> {};														// [0-9]+
	struct decimal : seq<plus<digit>, one<'.'>, star<digit>> {};							// [0-9]+\.[0-9]*
	struct number : sor<decimal, integer> {};												// {decimal}|{integer}
																							//struct literal : sor<number> {};														// {number}?

																							// no operators, etc. (variables would also be included here)
	struct parens : if_must<one<'('>, sep, expr, sep, one<')'>> {};							//\( *{expr} *\)					possible problems with function calls
	struct atomic : sor<number, parens, unary> {};											//{number}|{parens}|{unary}			highest precedence


	struct unary : seq<one<'-', '!'>, atomic> {};											// [\-!]{atomic}
																							// stuff to use to build infix

																							// Need to rework to give operator precedence (Probably'll invalidate the actions)		// {op} *{expr}
	struct p_1 : seq<one<'*', '/'>, sep, expr> {};											// [/\*] *{expr}
	struct p_2 : seq<one<'+', '-'>, sep, expr> {};											// [\+\-] *{expr}
	struct p_3 : seq<one<'<', '=', '>'>, sep, expr> {};										// [<=>] *{expr}
	struct p_4 : seq<one<'^', '%'>, sep, expr> {};											// A catch-all definition (I'm going to be redefining this system anyways)
	struct infix : sor<p_1, p_2, p_3, p_4> {};												// {p_1}|{p_2}|{p_3}				pegtl_calc and lua53_parse work a bit differently (ala above)
	struct m_expr : seq<atomic, opt<sep, infix>> {};										// {atomic}( *{infix})?				relies on a lot of recursion

																							//struct infix : success {};

																							//struct m_expr : list<atomic, infix, one<' '>> {};										// {atomic} *{infix}*

																							// Expr is more of a guarantee that something exists on the stack
	struct expr : sor<m_expr> {};															// {m_expr}
	struct statement : sor<expr, eps> {};													// ({expr})?

	struct grammar : must<sep, statement, sep, opt<comment>, eof> {};						//  *{statement} *{comment}?{eof}


																							// Which ones need to define actions ???
																							// number, unary, p_1/2/3

	template <typename Rule>
	struct action : nothing<Rule> {};

	template <> struct action<integer> {
		static void apply(const input& in, AST& ast) {
			ast.push(makeNode<Literal>(in.string(), ValType::INT));		// What do I want to store in the nodes?  (Node Data, Location, ...)
		}
	};

	template <> struct action<decimal> {
		static void apply(const input& in, AST& ast) {
			ast.push(makeNode<Literal>(in.string(), ValType::FLOAT));
		}
	};

	template <> struct action<unary> {
		static void apply(const input& in, AST& ast) {
			auto op = makeNode<UnOp>(in.string().substr(0, 1));
			op->addChild(node(ast));
			ast.push(op);
		}
	};

	struct infix_action {
		static void apply(const input& in, AST& ast) {
			auto op = makeNode<BinOp>(in.string().substr(0, 1));				// std::string{ in.string().front() });
			op->addChild(node(ast)).addChild(node(ast));
			ast.push(op);
		}
	};

	template <> struct action<p_1> : infix_action{};
	template <> struct action<p_2> : infix_action{};
	template <> struct action<p_3> : infix_action{};
	template <> struct action<p_4> : infix_action{};
}