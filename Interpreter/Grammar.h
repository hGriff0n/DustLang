#pragma once

#include "ast.h"

#define pstring(x) pegtl_string_t(x)
#define key_string(x) key<pstring(x)> {}

namespace dust {
	namespace parse {
		using namespace pegtl;
		using AST = Stack<std::shared_ptr<ASTNode>>;
		using eps = success;

		// Rule Templates
		template <typename Rule, typename Sep, typename Pad>							// Covers pegtl::list (apes list_tail)
		struct list : seq<pegtl::list<Rule, Sep, Pad>, opt<star<Pad>, disable<Sep>>> {};	// Prevents the comma action from being called multiple times (is this necessary?)
		template <typename Rule>
		struct unless : if_then_else<at<Rule>, failure, any> {};
		template <typename Cond, typename Then>
		struct _if : if_then_else<Cond, Then, eps> {};


		// Forward Declarations
		struct expr;		struct expr_0;		struct expr_x;
		struct keywords;	struct expr_7;


		// Whitespace 
		//struct space : one<' ', '\n', '\r', \t', '\v', '\f'> {};
		struct spaces : plus<one<' '>> {};
		struct seps : star<space> {};
		struct white : plus<space> {};
		struct scope : star<one<'\t'>> {};
		struct tail : one<' ', '\r', '\t', '\v', '\f'> {};
		struct endline : until<eolf, space> {};


		// Readability Tokens
		struct comma : one<','> {};

		template <typename Rule>
		struct w_list : list<Rule, comma, space> {};									// Weak list, allows trailing comma		// I could add the Sep template and pass in comma everywhere (arguably more extensible, don't need the forward declaration) or I could just move these after "Readability"
		template <typename Rule>
		struct s_list : pegtl::list<Rule, comma, space> {};								// Strict list, no trailing comma

		struct o_paren : one<'('> {};
		struct c_paren : one<')'> {};
		struct o_brack : one<'['> {};
		struct c_brack : one<']'> {};
		struct quote : one<'"'> {};
		struct esc : one<'\\'> {};		// % ???
		struct comment : if_must<two<'#'>, until<eolf>> {};								// ##.*
		struct var_list : s_list<expr_0> {};											// AST and lookahead? (seq<var_name, seps, sor<one<','>, op_5>>)  // this could technically match an expression list
		struct expr_list : s_list<expr_x> {};											// {expr_5} *, *


		// Identifier Tokens
		struct id_end : identifier_other {};
		struct type_id : seq<range<'A', 'Z'>, star<id_end>> {};							// [A-Z]{id_end}*
		struct var_id : seq<not_at<keywords>, range<'a', 'z'>, star<id_end>> {};		// [a-z]{id_end}*
		struct var_lookup : seq<star<one<'.'>>, at<var_id>> {};
		struct var_name : seq<var_lookup, var_id> {};									// \.*{var_id}


		// Keyword Tokens
		template <class Str>
		struct key : seq<Str, not_at<id_end>> {};

		struct k_true : key_string("true");
		struct k_false : key_string("false");
		struct k_nil : key_string("nil");
		struct k_or : key_string("or");
		struct k_and : key_string("and");
		struct k_type : key_string("type");
		struct k_try : key_string("try");
		struct k_catch : key_string("catch");
		struct k_do : key_string("do");
		struct k_while : key_string("while");
		struct k_repeat : key_string("repeat");
		struct k_for : key_string("for");
		struct k_in : key_string("in");
		struct k_if : key_string("if");
		struct k_else : key_string("else");
		struct k_elseif : key_string("elseif");

		struct keywords : sor<k_and, k_true, k_false, k_or, k_nil, k_do, k_in, k_if, k_else, k_elseif, k_while, k_for, k_type, k_try, k_catch, k_repeat> {};


		// Literal Tokens
		struct integer : plus<digit> {};												// [0-9]+
		struct decimal : seq<plus<digit>, one<'.'>, not_at<var_id>, star<digit>> {};	// [0-9]+\.[0-9]*
		struct bad_decimal : seq<one<'.'>, not_at<var_id>, star<digit>> {};				// \.[0-9]*				Rule to catch invalid decimals at "parse-time"
		struct boolean : sor<k_true, k_false> {};
		struct body : star<sor<seq<esc, quote>, unless<quote>>> {};						// ((\\\")|[^"])*
		struct str : seq<quote, body, quote> {};										// \"{body}\"

		struct table_inner : until<c_brack, sor<white, expr>> {};
		struct table : seq<o_brack, seps, table_inner> {};								// \[ *{expr}* *\]

		struct literals : sor<decimal, bad_decimal, integer, boolean, table, str, k_nil> {};


		// Operators
		struct op_0 : one<'!', '-'> {};																	// unary operators
		struct op_1 : one<'^'> {};
		struct op_2 : one<'*', '/'> {};
		struct op_3 : one<'+', '-', '%'> {};
		struct op_4 : sor<pstring("<="), pstring(">="), pstring("!="), one<'<', '=', '>'>> {};			// A tad "crude"
		struct op_7 : seq<one<':'>, opt<one<'^', '*', '/', '+', '-', '%', '<', '=', '>'>>> {};			// Assignment operators (should :>=, :<=, and :!= be valid???)
		struct op_inherit : pstring("<-") {};


		// Atomic Tokens
		struct unary : seq<op_0, expr_0> {};
		struct cast : seq<one<'('>, type_id, one<')'>> {};								// to avoid matching parens (error?)
		struct parens : if_must<o_paren, seps, expr, seps, c_paren> {};					// \( *{expr} *\)
		struct lvalue : sor<literals, var_name, unary, cast, parens> {};				// {literal}|{var_name}|{unary}|{parens}

		
		// Expression Tokens
		// Indexable/Callable variables/values
		struct dot_index : seq<one<'.'>, sor<var_id, integer>> {};						// \.({var_id}|{integer})
		struct brac_index : seq<one<'['>, seps, expr_7, seps, one<']'>> {};				// \[ *{expr_7} *\]
		struct fn_call : seq<one<'('>, seps, expr_list, one<')'>> {};					// \( *{expr_list}\)
		struct expr_0 : seq<lvalue, star<sor<dot_index, brac_index, fn_call>>> {}; 		// {lvalue}({dot_index}|{brac_index}|{fn_call})


		// Type Casts
		//struct type_cast : seq<type_id, parens> {};									// {type_id}{parens}			Int("4")
		struct type_cast : seq<cast, expr_0> {};										// ({type_id}){expr_0}			(Int)"4"
		struct expr_tcast : sor<type_cast, expr_0> {};

		// Operator '^'
		struct ee_1 : if_must<op_1, seps, expr_tcast> {};								// ee_#'s structure the parsing to allow ast to be constructed left->right
		struct expr_1 : seq<expr_tcast, star<seps, ee_1>> {};							// {expr_0}( *{op_1} *{expr_0})*

		// Operator '*', '/'
		struct ee_2 : if_must<op_2, seps, expr_1> {};									// change name to left_assoc_# (or something similar) ???
		struct expr_2 : seq<expr_1, star<seps, ee_2>> {};								// {expr_1}( *{op_2} *{expr_1})*

		// Operator '+', '-'
		struct ee_3 : if_must<op_3, seps, expr_2> {};
		struct expr_3 : seq<expr_2, star<seps, ee_3>> {};								// {expr_2}( *{op_3} *{expr_2})*

		// Type Check and Boolean operators
		struct ee_4 : if_must<op_4, seps, expr_3> {};
		struct ee_tc : if_must<op_inherit, seps, type_id> {};
		//struct expr_4 : seq<expr_3, opt<seps, sor<ee_tc, ee_4>>> {};					// {expr_3}( *({<- *{type_id})|({op_4} *{expr_3})?
		struct expr_4 : seq<expr_3, star<seps, sor<ee_tc, ee_4>>> {};					// {expr_3}( *({<- *{type_id})|({op_4} *{expr_3})*

		// Boolean and
		struct ee_5 : if_must<k_and, seps, expr_4> {};
		struct expr_5 : seq<expr_4, star<seps, ee_5>> {};								// {expr_4}( *and *{expr_4})?

		// Boolean or
		struct ee_6 : if_must<k_or, seps, expr_5> {};
		struct expr_6 : seq<expr_5, star<seps, ee_6>> {};								// {expr_5}( *or *{expr_5})?

		// Assignment (Right-associative)
		struct assign : seq<var_list, seps, op_7> {};									// assignments are right associative
		struct ee_7 : seq<assign, seps, expr_list> {};									// ensure that expr_6 doesn't trigger the expression reduction
		struct expr_7 : if_then_else<at<assign>, ee_7, expr_6> {};						// {var_list} *{op_5} * {expr_list}

		// Type Creation
		struct ee_inherit : seq<seps, op_inherit, seps, type_id> {};
		struct ee_type : seq<k_type, spaces, type_id, white, table, opt<ee_inherit>> {};
		struct expr_type : sor<ee_type, expr_7> {};										// type[ ]*{type_id} *{table}( *<- *{type_id})?

		// Try-Catch
		struct inline_expr : seq<plus<tail>, expr> {};

		struct ee_try : if_must<k_try, opt<inline_expr>> {};
		struct ee_catch : if_must<k_catch, seps, one<'('>, var_id, one<')'>> {};
		struct expr_trycatch : sor<ee_try, seq<ee_catch, opt<inline_expr>>, expr_type> {};

		// Loops
		struct ee_while : if_must<k_while, seps, expr> {};								// while *{expr}
		struct ee_repeat : if_must<k_repeat, seps, expr> {};							// repeat *{expr}
		struct ee_do : seq<k_do> {};

		struct expr_for : seq<var_list, seps, k_in, seps, expr_0> {};
		struct ee_for : if_must<k_for, seps, expr_for> {};								// for *{var_list} *in *{expr_0}

		struct ee_loop : seq<sor<ee_while, ee_repeat, ee_for>, opt<seps, k_do>, opt<inline_expr>> {};
		struct expr_loop : sor<ee_loop, expr_trycatch> {};

		// Branching
		struct ee_if : if_must<k_if, seps, expr> {};
		struct ee_elseif : if_must<k_elseif, seps, expr> {};
		struct ee_cond : seq<sor<ee_if, ee_elseif, k_else>, opt<seps, k_do>, opt<inline_expr>> {};
		struct expr_cond : sor<ee_cond, expr_loop> {};

		// Collector Tags
		struct expr_x : expr_cond {};
		struct expr : expr_x {};


		// Organization Tokens
		// line = \t*[ ]*{expr}? *{comment}?\n

		// This is slow (Has to parse over eval_line twice on match)
		struct eval_line : seq<scope, pad<expr, tail>, sor<comment, must<eolf>>> {};				// Analysis says...
		struct line : if_then_else<at<eval_line>, eval_line, until<eolf>> {};						// No progress (but it works)
		//struct line : seq<scope, pad<expr, tail>, sor<comment, must<eolf>>> {};					// Progress	   (but doesn't work)

		struct file : star<line> {};
		//struct file : star<if_then_else<at<line>, line, until<eolf>>> {};
	}

	struct grammar : pegtl::must<parse::file> {};
}