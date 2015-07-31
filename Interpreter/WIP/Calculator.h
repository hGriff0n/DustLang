#pragma once

#include <pegtl.hh>
#include <iostream>

#include "ast.h"
#include "state.h"

// also need to change the calling convention (for unary operators) ???
// Problem: virtually impossible to create a parsing error (ie. 3a -> Literal INT 3)

// For dust I should probably split the grammar and actions into seperate files
// For quick testing of grammar, pegjs.org/online

namespace calculator {
	using namespace pegtl;
	using eps = pegtl::success;
	template <typename Cond, typename Then, typename Else>
	using if_else = pegtl::if_then_else<Cond, Then, Else>;
	using AST = stack<std::shared_ptr<ASTNode>>;		// Would STATE be a better name


	// Rule Templates
	struct comma; struct sep;
	template <typename Rule, typename Sep, typename Pad>							// Covers pegtl::list (apes list_tail)
	struct list : seq<pegtl::list<Rule, Sep, Pad>, opt<star<Pad>, disable<Sep>>> {};	// Prevents the comma action from being called multiple times (is this necessary?)
	//template <typename If, typename Then, typename Else = failure>
	//struct if_at : if_then_else<at<If>, Then, Else> {};
	template <typename Rule>
	struct unless : if_then_else<at<Rule>, failure, any> {};
	//struct unless : if_at<Rule, failure, any> {};

	// Can I move these under "Readability"
	template <typename Rule>
	struct w_list : list<Rule, comma, sep> {};										// Weak list, allows trailing comma		// I could add the Sep template and pass in comma everywhere (arguably more extensible, don't need the forward declaration) or I could just move these after "Readability"
	template <typename Rule>
	struct s_list : pegtl::list<Rule, comma, sep> {};								// Strict list, no trailing comma

	// Forward declarations
	struct expr;
	struct expr_0;
	struct expr_5;
	struct str;

	// "Readability" Tokens
	struct sep : one<' '> {};
	struct seps : star<sep> {};
	struct o_paren : one<'('> {};
	struct c_paren : one<')'> {};
	struct comma : one<','> {};
	struct quote : one<'"'> {};
	struct esc : one<'\\'> {};		// % ???
	struct comment : if_must<two<'#'>, until<eolf>> {};								// ##.*

	// Literal Tokens
	struct integer : plus<digit> {};												// [0-9]+
	struct decimal : seq<plus<digit>, one<'.'>, star<digit>> {};					// [0-9]+\.[0-9]*
	struct boolean : sor<string<'t','r','u','e'>, string<'f','a','l','s','e'>> {};
	struct body : plus<sor<seq<esc, quote>, unless<quote>>> {};
	struct str : seq<quote, opt<body>, quote> {};
	struct literals : sor<decimal, integer, boolean, str> {};

	// Keyword Tokens


	// Identifier Tokens
	struct id_end : identifier_other {};
	struct type_id : seq<range<'A', 'Z'>, star<id_end>> {};							// [A-Z]{id_end}*
	struct var_id : seq<range<'a', 'z'>, star<id_end>> {};							// [a-z]{id_end}*

	// Operator Tokens
	struct op_0 : one<'!', '-'> {};													// unary operators
	struct op_1 : one<'^'> {};
	struct op_2 : one<'*', '/'> {};
	struct op_3 : one<'+', '-', '%'> {};
	struct op_4 : sor<string<'<', '='>, string<'>', '='>, string<'!', '='>, one<'<', '=', '>'>> {};					// A tad "crude"
	struct op_5 : seq<one<':'>, opt<one<'^', '*', '/', '+', '-', '%', '<', '=', '>'>>> {};							// Assignment operators (should :>=, :<=, and :!= be valid???)

	// Atomic Tokens
	struct unary : seq<op_0, expr_0> {};
	struct parens : if_must<o_paren, seps, expr, seps, c_paren> {};

	// Expression Tokens
	struct expr_0 : sor<literals, var_id, unary, parens> {};						// {number}|{var_id}|{unary}|{parens}
	struct ee_1 : if_must<op_1, seps, expr_0> {};									// Structures the parsing to allow the ast to be constructed left->right
	struct expr_1 : seq<expr_0, star<seps, ee_1>, seps> {};							// {expr_0}( *{op_1} *{expr_0})* *
	struct ee_2 : if_must<op_2, seps, expr_1> {};									// change name to left_assoc_# (or something similar)
	struct expr_2 : seq<expr_1, star<seps, ee_2>, seps> {};							// {expr_1}( *{op_2} *{expr_1})* *
	struct ee_3 : if_must<op_3, seps, expr_2> {};
	struct expr_3 : seq<expr_2, star<seps, ee_3>, seps> {};							// {expr_2}( *{op_3} *{expr_2})* *
	struct ee_4 : if_must<op_4, seps, expr_3> {};
	struct expr_4 : seq<expr_3, star<seps, ee_4>, seps> {};							// {expr_3}( *{op_4} *{expr_3})* *


	// Optimize and integrate multiple assignment
		// Can I utilize the lookahead phase to push var_id's onto the stack ???
	struct var_list : s_list<var_id> {};											// AST and lookahead? (seq<var_id, seps, sor<one<','>, op_5>>)  // this could technically match an expression list
	struct expr_list : s_list<expr_5> {};											// {expr_5} *, *

	struct assign : seq<var_list, seps, op_5> {};									// assignments are right associative
	struct ee_5 : seq<assign, seps, expr_list> {};									// ensure that expr_4 doesn't trigger the expression reduction
	struct expr_5 : if_then_else<at<assign>, ee_5, expr_4> {};						// {var_list} *{op_5} * {expr_list}




	// Organization Tokens
	struct expr : seq<expr_5> {};

	// Grammar Token
	struct grammar : must<expr> {};


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