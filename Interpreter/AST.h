#pragma once

#include "EvalState.h"

#include <memory>

namespace dust {
	namespace parse {
		// Possibly create grammars and actions for escaping/unescaping strings
		std::string escape(std::string);
		std::string unescape(std::string);
		std::string trim(std::string);					// Needed to prevent errors when evaluating " "

		/*
		 * Base class for the AST structure
		 */
		class ASTNode {
			private:
			public:
				static std::string node_type;

				virtual EvalState& eval(EvalState&) = 0;
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString() = 0;
				virtual std::string printString(std::string buf);
		};

		/*
		 * An iterable collection of nodes of a desired type
		 */
		template <class Node>
		class List : public ASTNode {
			private:
				std::vector<std::shared_ptr<Node>> elems;

				List& add(std::shared_ptr<Node>& n) {
					if (n) elems.push_back(n);
					return *this;
				}

			public:
				List() {}
				static std::string node_type;

				EvalState& eval(EvalState& e) {
					throw error::bad_node_eval{ "Attempt to evaluate a List node" };
				}
				void addChild(std::shared_ptr<ASTNode>& c) {
					add(std::dynamic_pointer_cast<Node>(c));
				}

				std::string toString() { return ""; }
				virtual std::string printString(std::string buf) {
					std::string ret = buf + "+- " + node_type + "\n";
					buf += " ";

					for (auto i = begin(); i != end(); ++i)		// for (auto i : *this)
						ret += (*i)->printString(buf);

					return ret;
				}

				// Assuming sub-nodes are stored left->right
					// List(a, b, c) => List.elems = { a, b, c }
				auto rbegin() { return elems.begin(); }
				auto rend() { return elems.end(); }
				auto begin() { return elems.rbegin(); }
				auto end() { return elems.rend(); }

				size_t size() { return elems.size(); }
		};

		/*
		 * Non-evaluable node used to simplify AST assembly (Seperate nodes)
		 */
		class Debug : public ASTNode {
			private:
				std::string msg;

			public:
				Debug(std::string _msg);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
		};

		/*
		 * Represents a non-table literal
		 */
		class Literal : public ASTNode {
			private:
				std::string val;
				size_t id;

			public:
				Literal(std::string _val, size_t t);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();		// Possibly temporary implementation
				std::string printString(std::string buf);
		};

		/*
		 * Represents unary and binary operations using infix/prefix operators
		 */
		class Operator : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				std::string op;

			public:
				Operator(std::string o);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Represents a variable
		 */
		class VarName : public ASTNode {
			private:
				//std::shared_ptr<ASTNode> name;
				//std::vector<std::shared_ptr<ASTNode>> sub_fields;
				std::vector<std::shared_ptr<ASTNode>> fields;
				int lvl = 0; bool sub_var = false;

			public:
				VarName(std::string var);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);

				void setSubStatus();
				void addLevel(const std::string& dots);
				EvalState& set(EvalState& e, bool is_const, bool is_static);
		};

		/*
		 * Represents a type
		 */
		class TypeName : public ASTNode {
			private:
				std::string name;

			public:
				TypeName(std::string n);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Represents a type cast operation
		 * May be combined with other nodes in the future (current grammar is a type constructor)
		 */
		class TypeCast : public ASTNode {
			private:
				std::shared_ptr<TypeName> convert;
				std::shared_ptr<ASTNode> expr;

			public:
				TypeCast();
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Represents an assignment operation
		 */
		class Assign : public ASTNode {
			using var_type = List<VarName>;
			//using var_type = List<ASTNode>;
			using val_type = List<ASTNode>;
			using arg_type = std::shared_ptr<ASTNode>;

			private:
				std::shared_ptr<var_type> vars;
				std::shared_ptr<val_type> vals;
				std::string op;
				bool set_const, set_static;

			public:
				Assign(std::string _op, bool _const = false, bool _static = false);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		//class Keyword : public ASTNode { };

		/*
		 * Temporary representation of the and/or keywords
		 */
		class BooleanOperator : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				bool isAnd;

			public:
				BooleanOperator(std::string key);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for specifiying number of evaluations of a Block
		 */
		class Control : public ASTNode {
			private:
				std::shared_ptr<ASTNode> expr;
				bool next;
				int type;

			public:
				Control();
				Control(int typ);
				static std::string node_type;
				enum Type {
					FOR,
					WHILE,
					DO_WHILE
				};
				
				virtual EvalState& eval(EvalState& e);
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString();
				virtual std::string printString(std::string buf);

				bool iterate(EvalState& e);
		};

		/*
		 * Node for a collection of expressions with a common scope
		 */
		class Block : public ASTNode {
			private:
				std::shared_ptr<Control> control;
				std::vector<std::shared_ptr<ASTNode>> expr;

			protected:
			public:
				auto begin();
				auto end();
				size_t size();

				Block();
				static std::string node_type;
				bool save_scope, table, excep_if_empty;

				virtual EvalState& eval(EvalState& e);
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for creating new types
		 */
		class NewType : public ASTNode {
			private:
				std::string name, inherit;
				std::shared_ptr<Block> definition;

			public:
				NewType();
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for type checking
		 */
		class TypeCheck : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l;
				std::string type;

			public:
				TypeCheck();
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Exception handling for sub-nodes
		 */
		class TryCatch : public ASTNode {
			private:
				//std::shared_ptr<Block> try_code, catch_code;
				std::shared_ptr<ASTNode> try_code, catch_code;

			public:
				TryCatch();
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
				virtual std::string printString(std::string buf);

				virtual void addChild(std::shared_ptr<ASTNode>& c);
		};

		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
	}

	template <class T, typename... Args>
	std::shared_ptr<parse::ASTNode> makeNode(Args&... args) {
		return std::make_shared<T>(args...);
	}

	template <class ostream>
	void printAST(ostream& s, std::shared_ptr<parse::ASTNode>& ast) {
		(s << ast->printString("|")).flush();
	}
}

