#pragma once

#include "Grammar.h"
#include "Exceptions\parsing.h"

namespace dust {
	namespace parse {
		using ScopeTracker = Stack<size_t>;

		template <typename Rule>
		struct action : pegtl::nothing<Rule> {};

		// Organization Action
		template <> struct action<scope> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				if (lvl.empty()) {
					push(ast, 1, in);
					lvl.push(0);
				}

				auto depth = in.size();

				// Push control nodes on the stack
				if (lvl.at() < depth) {
					push(ast, depth - lvl.at(), in);
					lvl.push(depth);

				// Create blocks
				} else if (lvl.at() > depth) {
					reduce(ast, lvl.at() - depth, in);

					while (!lvl.empty() && lvl.at() >= depth)
						lvl.pop();

					lvl.push(depth);
				}

			}

			static void push(AST& ast, int n, input& in) {
				while (n--> 0)
					ast.push(makeNode<Control>(in));
			}

			// Also handles try-catch reduction
			static void reduce(AST& ast, int n, input& in) {
				while (n--> 0) {
					auto block = makeNode<Block>(in);

					do block->addChild(ast.at());
					while (!isNode<Control>(ast.pop()));

					// Try-Catch reduction (need to generalize for If-Elseif-Else and functions)
					if (!ast.empty() && isNode<TryCatch>(ast.at()) && !std::dynamic_pointer_cast<TryCatch>(ast.at())->isFull())
						ast.at()->addChild(block);
					else
						ast.push(block);
				}
			}
		};

		template <> struct action<file> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				// stack: ... | ..., {Control}, ....

				// Finish assembly of AST
				if (ast.empty()) {
					auto b = makeNode<Block>(in);
					b->addChild(makeNode<Control>(in));
					ast.push(b);

				} else {
					// Possibly wrong when requiring files (clears the scopetracker)
					action<scope>::reduce(ast, lvl.at() + 1, in);
					lvl.clear();			// I could always use a unique ScopeTracker for each file

				}

				setFields(ast);
			}

			static void setFields(AST& ast) {
				// stack: ..., {Block}

				auto b = std::dynamic_pointer_cast<Block>(ast.at());

				b->excep_if_empty = false;
				// require code
			}
		};

		template <> struct action<o_brack> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				action<scope>::push(ast, 1, in);
			}
		};

		template <> struct action<table> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				action<scope>::reduce(ast, 1, in);

				// stack: ..., {Block}

				auto b = std::dynamic_pointer_cast<Block>(ast.at());
				b->table = true;
				b->save_scope = true;

				// stack: ..., {Block}
			}
		};


		// Control Flow
		template <> struct action<ee_while> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				// stack: ..., {condition}

				auto c = makeNode<Control>(in, Control::WHILE);
				c->addChild(ast.pop());

				ast.push(c);
				lvl.push(lvl.at() + 1);

				// stack: ..., {Control}
			}
		};

		template <> struct action<ee_do> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				// stack: ..., {condition}
				
				auto c = makeNode<Control>(in, Control::DO_WHILE);
				c->addChild(ast.pop());

				ast.push(c);
				lvl.push(lvl.at() + 1);

				// stack: ..., {Control}
			}
		};

		template <> struct action<ee_efirst> : action<ee_do> {};


		// Try-Catch Actions
		template <> struct action<k_try> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				// stack: ...

				ast.push(makeNode<TryCatch>(in));
				action<scope>::push(ast, 1, in);
				lvl.push(lvl.at() + 1);

				// stack: ..., {TryCatch}, {Control}
			}
		};

		template <> struct action<k_catch> {
			static void apply(input& in, AST& ast, ScopeTracker& lvl) {
				// stack : ..., {TryCatch}

				if (!isNode<TryCatch>(ast.at()))
					throw error::missing_node_x{ "Catch", "TryCatch" };

				action<scope>::push(ast, 1, in);
				lvl.push(lvl.at() + 1);

				// stack: ..., {TryCatch}, {Control}
			}
		};


		// Expression Actions
		template <> struct action<unary> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				if (ast.size() >= 2) {
					if (isNode<Operator>(ast.at(-2)))
						// stack: ..., {op}, {operand}

						ast.at(-2)->addChild(ast.pop());

						// stack: ..., {op}
					else
						throw error::missing_node_x{ "Operator" };
				} else
					throw error::missing_nodes{ "Operator", 2 };
			}
		};

		template <typename Node>
		struct ee_actions {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				if (ast.size() >= 3) {
					if (isNode<Node>(ast.at(-2))) {
						// stack: ..., {lhs}, {op}, {rhs}

						auto rhs = ast.pop();							// Doesn't check that rhs or lhs is valid
						auto op = ast.pop();							// However, rhs and lhs are guaranteed to be ASTNode's, which operator accepts

						op->addChild(ast.pop());						// Current ordering expected by operators (lhs, rhs)
						op->addChild(rhs);								// Assignment::addChild handles the Assignment specific node checks

						ast.push(op);

						// stack: ..., {op}
					} else
						throw error::missing_node_x{ Node::node_type };
				} else
					throw error::missing_nodes{ Node::node_type, 3 };
			}
		};

		template <> struct action<ee_1> : ee_actions<Operator> {};
		template <> struct action<ee_2> : ee_actions<Operator> {};
		template <> struct action<ee_3> : ee_actions<Operator> {};
		template <> struct action<ee_4> : ee_actions<Operator> {};
		template <> struct action<ee_5> : ee_actions<BooleanOperator> {};
		template <> struct action<ee_6> : ee_actions<BooleanOperator> {};
		template <> struct action<ee_7> : ee_actions<Assign> {};				// ee_x is Assignmnet, which needs and uses Assignment nodes. ee_acctions requires Operator nodes (???)

																				// List Actions (there has to be a way to simplify this code)
		template <class type, bool force_type = false>
		struct list_actions {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				auto list = makeNode<List<type>>(in);

				if (ast.at()->toString() != ",")
					ast.push(makeNode<Debug>(in, ","));

				while (!ast.empty() && ast.at()->toString() == ",") {
					ast.pop();

					if (isNode<type>(ast.at()))
						list->addChild(ast.pop());
					else
						break;
				}

				ast.push(list);
			}
		};

		template <class type>
		struct list_actions<type, true> {					// Specialization of list_actions to force conversions to type (for VarName, could just specialize on VarName ???)
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				auto list = makeNode<List<type>>(in);

				if (ast.at()->toString() != ", ")
					ast.push(makeNode<Debug>(in, ","));

				while (!ast.empty() && ast.at()->toString() == ",") {
					ast.pop();

					if (isNode<type>(ast.at()))
						list->addChild(ast.pop());
					else
						list->addChild(makeNode<type>(ast.pop()));
				}

				ast.push(list);
			}
		};

		template <> struct action<var_list> : list_actions<VarName, true> {};
		template <> struct action<expr_list> : list_actions<ASTNode> {};


		// Operator Actions
		struct OpAction {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Operator>(in, "_op" + in.string()));
			}
		};

		template <> struct action<op_0> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Operator>(in, "_ou" + in.string()));
			}
		};

		template <> struct action<op_1> : OpAction {};
		template <> struct action<op_2> : OpAction {};
		template <> struct action<op_3> : OpAction {};
		template <> struct action<op_4> : OpAction {};


		// Assignment
		template <> struct action<op_7> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Assign>(in, in.string().substr(1)));
			}
		};


		// Variable Actions
		template <> struct action<dot_index> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				std::shared_ptr<VarName> t = std::dynamic_pointer_cast<VarName>(ast.at());			// Works ???
				if (t) t->setSubStatus();

				ast.at(-2)->addChild(ast.pop());
			}
		};

		template <> struct action<brac_index> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.at(-2)->addChild(ast.pop());
			}
		};

		template <> struct action<lvalue> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {

			}
		};

		template <> struct action<var_id> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<VarName>(in, in.string()));
			}
		};

		template <> struct action<var_lookup> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Debug>(in, in.string()));
			}
		};

		template <> struct action<var_name> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				std::dynamic_pointer_cast<VarName>(ast.at())->addLevel(ast.pop(-2)->toString());
			}
		};


		// Type Actions
		template <> struct action<type_id> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				// stack: ...

				ast.push(makeNode<TypeName>(in, in.string()));

				// stack: ..., {TypeName}
			}
		};

		template <> struct action<ee_type> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				// stack: ..., {NewType}, ...

				int i = -1;
				while (!std::dynamic_pointer_cast<NewType>(ast.at(i))) --i;

				auto typ = ast.pop(i++);

				for (; i; ++i) typ->addChild(ast.pop(i));

				ast.push(typ);

				// stack: ..., {NewType}
			}
		};

		template <> struct action<ee_tc> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				// stack: ..., {type}, {ASTNode}

				auto tc = makeNode<TypeCheck>(in);
				tc->addChild(ast.pop());
				tc->addChild(ast.pop());

				ast.push(tc);

				// stack: ..., {type_check}
			}
		};

		template <> struct action<type_cast> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				// stack: ..., {type}, {ASTNode}

				auto t = makeNode<TypeCast>(in);
				t->addChild(ast.pop());
				t->addChild(ast.pop());

				ast.push(t);

				// stack: ..., {type_cast}
			}
		};


		// Keyword Actions
		template <> struct action<k_and> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<BooleanOperator>(in, in.string()));
			}
		};

		template <> struct action<k_or> : action<k_and> {};

		template <> struct action<k_type> {
			static void apply(input& in, AST& ast, const ScopeTracker& _) {
				//stack: ...

				ast.push(makeNode<NewType>(in));

				// stack: ..., {NewType}
			}
		};


		// Literal Actions
		template <> struct action<decimal> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Literal>(in, in.string(), type::Traits<double>::id));
			}
		};

		template <> struct action<boolean> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Literal>(in, in.string(), type::Traits<bool>::id));
			}
		};

		template <> struct action<integer> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Literal>(in, in.string(), type::Traits<int>::id));
			}
		};

		template <> struct action<body> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Literal>(in, unescape(in.string()), type::Traits<std::string>::id));
			}
		};

		template <> struct action<k_nil> {
			static void apply(const input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Literal>(in, "", type::Traits<Nil>::id));
			}
		};


		// Debug Actions
		template <> struct action<o_paren> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Debug>(in, "("));
			}
		};

		template <> struct action<c_paren> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				ast.swap(); ast.pop();			// Needs some testing (empty parens)
			}
		};

		template <> struct action<comma> {
			static void apply(input& in, AST& ast, ScopeTracker& _) {
				ast.push(makeNode<Debug>(in, in.string()));
			}
		};
	}

	template <typename Rule>
	struct action : parse::action<Rule> {};
}
