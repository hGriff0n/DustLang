#pragma once

#include <pegtl.hh>
#include "AST.h"

// For quick testing of grammar, pegjs.org/online

#define pstring(x) pegtl_string_t(x)
#define key_string(x) key<pstring(x)> {}

namespace dust {
	namespace parse {
		using namespace pegtl;
		using AST = Stack<std::shared_ptr<ASTNode>>;

		// Rule Templates
		template <typename Rule, typename Sep, typename Pad>							// Covers pegtl::list (apes list_tail)
		struct list : seq<pegtl::list<Rule, Sep, Pad>, opt<star<Pad>, disable<Sep>>> {};	// Prevents the comma action from being called multiple times (is this necessary?)
		template <typename Rule>
		struct unless : if_then_else<at<Rule>, failure, any> {};


		// Forward declarations
		struct expr;	struct expr_0;			struct expr_x;
		struct block;	struct inline_block;
		struct comma;	struct sep;


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


		// Identifier Tokens
		struct id_end : identifier_other {};
		struct type_id : seq<range<'A', 'Z'>, star<id_end>> {};							// [A-Z]{id_end}*
		struct var_id : seq<range<'a', 'z'>, star<id_end>> {};							// [a-z]{id_end}*


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
		struct k_inherit : pstring("<-") {};											// Can't start an indentifier


		// Literal Tokens
		struct integer : plus<digit> {};												// [0-9]+
		struct decimal : seq<plus<digit>, one<'.'>, star<digit>> {};					// [0-9]+\.[0-9]*
		struct boolean : sor<k_true, k_false> {};
		struct body : star<sor<seq<esc, quote>, unless<quote>>> {};						// ((\\\")|[^"])*
		struct str : seq<quote, body, quote> {};										// \"{body}\"

		//struct table : seq<o_brack, opt<expr_list>, seps, c_brack> {};
		struct table : seq<o_brack, opt<block>, seps, c_brack> {};					// \[{block}? *\]
			// This doesn't allow [ 4 ] syntax (block relies on scoping changes to end parsing)
				// Then tables can't be ended with a ']' unless it's on a newline and "de-scoped" (not that that's not good style)
			// table_block struct ???
		struct literals : sor<decimal, integer, boolean, str, k_nil> {};


		// Operator Tokens
		struct op_0 : one<'!', '-'> {};													// unary operators
		struct op_1 : one<'^'> {};
		struct op_2 : one<'*', '/'> {};
		struct op_3 : one<'+', '-', '%'> {};
		struct op_4 : sor<pstring("<="), pstring(">="), pstring("!="), one<'<', '=', '>'>> {};							// A tad "crude"
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
		struct var_list : s_list<var_id> {};											// AST and lookahead? (seq<var_id, seps, sor<one<','>, op_5>>)  // this could technically match an expression list
		struct expr_list : s_list<expr_x> {};											// {expr_5} *, *

		struct assign : seq<var_list, seps, op_x> {};									// assignments are right associative
		struct ee_x : seq<assign, seps, expr_list> {};									// ensure that expr_6 doesn't trigger the expression reduction
		struct expr_x : if_then_else<at<assign>, ee_x, expr_6> {};						// {var_list} *{op_5} * {expr_list}


		// Scoping Rules
		struct block {
			using analyze_t = analysis::counted<analysis::rule_type::STAR, 0, block, expr>;			// I have no idea how to do this

			template <apply_mode A, template<typename...> class Action, template<typename...> class Control>
			static bool match(input& in, AST& ast, const int exit) {
				int depth;
				ast.push(makeNode<Debug>("NEW_SCOPE"));														// Push sentinel node

				// Parse until the scope depth decreases or the file ends
				while (in.size() > 0 && (depth = block::depth(in)) >= exit) {
					// If the scope depth increases, parse a new block
					if (depth > exit)
						Control<must<block>>::template match<A, Action, Control>(in, ast, exit + 1);		// Increment to depth in order to handle deep scoping (Needs optimizing for empty Blocks)

					// Otherwise, parse an expression
					else {
						block::eat(in, depth);
						Control<must<expr>>::template match<A, Action, Control>(in, ast, exit);
					}
				}

				return true;
			}

			// Determines the depth of the current line. Doesn't consume			Also assumes that scoping == '\t'
			static int depth(const input& in) {
				return block::depth(in.string());
			}
			static int depth(const std::string& in) {
				int ret = 0;
				while (ret < in.size() && in.at(ret) == '\t') ++ret;
				return ret;
			}

			// Consumes x scope layers from the input
			static void eat(input& in, const int depth) {
				in.bump(depth);					// Might need to rework depending on how scoping is syntax'd
			}
		};

		// Allows the first line of a block to be in-lined
			// Supposed to only allow inlining of single expression loops
		struct inline_block {
			using analyze_t = block::analyze_t;

			template <apply_mode A, template<typename...> class Action, template<typename...> class Control>
			static bool match(input& in, AST& ast, const int scope) {
				return Control<block>::template match<A, Action, Control>(in, ast, scope + 1);
			}
		};


		// Organization Tokens
		struct expr : seq<seps, expr_x, seps> {};
		//struct prog : plus<seps, expr, seps> {};										// Have to modify with scoping ???
		struct file : block {};
		//struct file : seq<block, eof> {};

	}

	// Grammar Token
	struct grammar : pegtl::must<parse::file, pegtl::eolf> {};
	//struct grammar : pegtl::must<parse::file> {};
}
