#pragma once

#include <pegtl.hh>
#include <iostream>

#include "AST.h"

// also need to change the calling convention (for unary operators) ???
// Problem: virtually impossible to create a parsing error (ie. 3a -> Literal INT 3)

// For quick testing of grammar, pegjs.org/online

#define key_string(x) key<pegtl_string_t(x)> {}

namespace dust {
	namespace parse {
		using namespace pegtl;
		using eps = pegtl::success;
		template <typename Cond, typename Then, typename Else>
		using if_else = pegtl::if_then_else<Cond, Then, Else>;
		using AST = Stack<std::shared_ptr<ASTNode>>;			// Would STATE be a better name


		// Rule Templates
		template <typename Rule, typename Sep, typename Pad>							// Covers pegtl::list (apes list_tail)
		struct list : seq<pegtl::list<Rule, Sep, Pad>, opt<star<Pad>, disable<Sep>>> {};	// Prevents the comma action from being called multiple times (is this necessary?)
		template <typename Rule>
		struct unless : if_then_else<at<Rule>, failure, any> {};


		// Scoping Rules
		// General scoping class. Catches when the scoping doesn't change
		struct scp {
			template <apply_mode A, template<typename...> class Action, template<typename...> class Control>
			static bool match(input& in, AST& ast) {	//, int& counter) {
				//bool matches = scp::scope(in) == counter;
				//if (matches) "eat input"
				//return matches
			}

			// Determines the depth of the current line. Doesn't consume
			static int scope(input& in) {
				// Determine the number of continuous scope characters
			}
		};

		// Catches when the scope depth changess
		struct exit_scp {
			template <apply_mode A, template<typename...> class Action, template<typename...> class Control>
			static bool match(input& in, AST& ast) {
				//int count = scp::scope(in);

				//if (count != counter) {
				//	if (count < counter) {
				//		while (counter != count)								// Construct the necessary Block nodes (this struct only matches once)
				//			Action<Block>::apply(in, ast, counter--);
				
				//	} else if (count > counter) {
				//		while (counter != count) {
				//			++counter;
				//			push nodes on the stack								// Setup up the AST (this struct only matches once)
				//		}
				//	}
				
				//	"eat" input
				//	return true;
				//}

				//return false;
			}
		};


		// Forward declarations
		struct expr;
		struct expr_0;
		struct expr_x;
		struct block;
		struct id_end;
		struct k_nil;
		struct comma;
		struct sep;


		// "Readability" Tokens
		template <typename Rule>
		struct w_list : list<Rule, comma, sep> {};										// Weak list, allows trailing comma		// I could add the Sep template and pass in comma everywhere (arguably more extensible, don't need the forward declaration) or I could just move these after "Readability"
		template <typename Rule>
		struct s_list : pegtl::list<Rule, comma, sep> {};								// Strict list, no trailing comma

		struct sep : one<' ', '\n'> {};
		//struct sep : one<' ', '\n', '\t'> {};
		struct seps : star<sep> {};
		struct o_paren : one<'('> {};
		struct c_paren : one<')'> {};
		struct o_brack : one<'['> {};
		struct c_brack : one<']'> {};
		struct comma : one<','> {};
		struct quote : one<'"'> {};
		struct esc : one<'\\'> {};		// % ???
		struct comment : if_must<two<'#'>, until<eolf>> {};								// ##.*

		
		// Keyword Tokens
		template <class Str>
		struct key : seq<Str, not_at<id_end>> {};

		struct k_and : key_string("and");
		struct k_true : key_string("true");
		struct k_false : key_string("false");
		struct k_or : key_string("or");
		struct k_nil : key_string("nil");
		struct k_do : key_string("do");
		struct k_if : key_string("if");
		struct k_type : key_string("type");
		struct k_inherit : pegtl_string_t("<-") {};										// Can't start an indentifier


		// Literal Tokens
		struct integer : plus<digit> {};												// [0-9]+
		struct decimal : seq<plus<digit>, one<'.'>, star<digit>> {};					// [0-9]+\.[0-9]*
		struct boolean : sor<k_true, k_false> {};
		struct body : plus<sor<seq<esc, quote>, unless<quote>>> {};
		struct str : seq<quote, opt<body>, quote> {};
		struct literals : sor<decimal, integer, boolean, str, k_nil> {};

		//struct table : seq<o_brack, opt<block>, seps, c_brack> {};
		struct table : seq<one<'['>, opt<block>, seps, one<']'>> {};					// \[{block}? *\]
		//struct literals : sor<decimal, integer, boolean, str, table, k_nil> {};


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
		struct op_x : seq<one<':'>, opt<one<'^', '*', '/', '+', '-', '%', '<', '=', '>'>>> {};							// Assignment operators (should :>=, :<=, and :!= be valid???)


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

		struct ee_5 : seq<pad<k_and, sep>, expr_4> {};
		struct expr_5 : seq<expr_4, opt<seps, ee_5>, seps> {};							// {expr_4}( *and *{expr_4})? *

		struct ee_6 : seq<pad<k_or, sep>, expr_5> {};
		struct expr_6 : seq<expr_5, opt<seps, ee_6>, seps> {};							// {expr_5}( *or *{expr_5})? *


		// Multiple Assignment
			// expr_x is going to be a fairly high level expression
			// Optimize and integrate multiple assignment
			// Can I utilize the lookahead phase to push var_id's onto the stack ???
		struct var_list : s_list<var_id> {};											// AST and lookahead? (seq<var_id, seps, sor<one<','>, op_5>>)  // this could technically match an expression list
		struct expr_list : s_list<expr_x> {};											// {expr_5} *, *

		struct assign : seq<var_list, seps, op_x> {};									// assignments are right associative
		struct ee_x : seq<assign, seps, expr_list> {};									// ensure that expr_6 doesn't trigger the expression reduction
		struct expr_x : if_then_else<at<assign>, ee_x, expr_6> {};						// {var_list} *{op_5} * {expr_list}


		// Organization Tokens
		struct expr : expr_x {};
		struct block : plus<seps, expr, seps> {};										// Have to modify with scoping ???
		//struct block : plus<sor<scp, diff_scp>, seps, expr, seps> {};

	}

	// Grammar Token
	struct grammar : pegtl::must<parse::block, pegtl::eolf> {};
}
