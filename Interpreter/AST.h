#pragma once

#include "EvalState.h"

#include <pegtl.hh>
#include <memory>

namespace dust {
	namespace parse {
		// Possibly create grammars and actions for escaping/unescaping strings
		std::string escape(std::string);
		std::string unescape(std::string);
		std::string trim(std::string);

		/*
		 * Creation statistics for a node
		 */
		struct ParseData {
			size_t col, line;

			ParseData(const pegtl::input& in);
		};

		/*
		 * Base class for the AST structure
		 */
		class ASTNode {
			private:
			public:
				ParseData p;

				ASTNode(const ParseData& in);
				static std::string node_type;

				virtual EvalState& eval(EvalState&) = 0;
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString() = 0;
				virtual std::string printString(std::string buf);
		};

		//class Nothing {
		//	public:
		//		EvalState& eval(EvalState& e);
		//		std::string toString();
		//};

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
				List(const ParseData& in) : ASTNode{ in } {}
				static std::string node_type;

				EvalState& eval(EvalState& e) {
					for (auto n = rbegin(); n != rend(); ++n) (*n)->eval(e);

					return e;
				}
				void addChild(std::shared_ptr<ASTNode>& c) {
					add(std::dynamic_pointer_cast<Node>(c));
				}

				std::string toString() { return ""; }
				virtual std::string printString(std::string buf) {
					std::string ret = buf + "+- " + node_type + "\n";
					buf += " ";

					//for (auto i = begin(); i != end(); ++i)	ret += (*i)->printString(buf);
					for (auto i : *this) ret += i->printString(buf);

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
				Debug(const ParseData& in, std::string _msg);
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
				Literal(const ParseData& in, std::string _val, size_t t);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();		// Possibly temporary implementation
		};

		/*
		 * Represents unary and binary operations using infix/prefix operators
		 */
		class Operator : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				std::string op;

			public:
				Operator(const ParseData& in, std::string o);
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
				std::vector<std::shared_ptr<ASTNode>> fields;
				int lvl = 0; bool sub_var = false, get_first = true;

			public:
				VarName(const ParseData& p, std::string var);
				VarName(std::shared_ptr<ASTNode>&& var);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);

				void setSubStatus();
				void setLevel(const std::string& dots);
				EvalState& set(EvalState& e, bool is_const, bool is_static);
		};

		/*
		 * Represents a type
		 */
		class TypeName : public ASTNode {
			private:
				std::string name;

			public:
				TypeName(const ParseData& in, std::string n);
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
				TypeCast(const ParseData& in);
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
			using val_type = List<ASTNode>;
			using arg_type = std::shared_ptr<ASTNode>;

			private:
				std::shared_ptr<var_type> vars;
				std::shared_ptr<val_type> vals;
				std::string op;
				bool set_const, set_static;

			public:
				Assign(const ParseData& in, std::string _op, bool _const = false, bool _static = false);
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
				BooleanOperator(const ParseData& in, std::string key);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for specifying number of evaluations of a Block
		 */
		class Control : public ASTNode {
			private:
				std::shared_ptr<ASTNode> expr;
				std::shared_ptr<List<VarName>> vars;

				bool next;

			public:
				enum Type {
					NONE = -1,
					FOR,
					WHILE,
					DO_WHILE,
					TRY_CATCH,
					FUNCTION
				};

				Control(const ParseData& in, Type typ = NONE);

				static std::string node_type;
				Type type;
				
				virtual EvalState& eval(EvalState& e);
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString();
				virtual std::string printString(std::string buf);

				virtual bool iterate(EvalState& e, size_t loc);
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
				Block(const ParseData& in);
				static std::string node_type;
				bool save_scope, table, excep_if_empty;

				virtual EvalState& eval(EvalState& e);
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				virtual std::string toString();
				virtual std::string printString(std::string buf);

				auto begin();
				auto end();
				size_t size();
		};

		/*
		 * Node for creating new types
		 */
		class NewType : public ASTNode {
			private:
				std::string name, inherit;
				std::shared_ptr<Block> definition;

			public:
				NewType(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
		};

		/*
		 * Node for type checking
		 */
		class TypeCheck : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l;
				std::string type;

			public:
				TypeCheck(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for exception handling
		 */
		class TryCatch : public ASTNode {
			private:
				std::shared_ptr<Block> try_code, catch_code;

			public:
				TryCatch(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
				virtual std::string printString(std::string buf);

				virtual void addChild(std::shared_ptr<ASTNode>& c);

				bool isFull();
		};

		/*
		 * Node for branching statements
		 */
		class If : public ASTNode {
			using BlockType = std::shared_ptr<Block>;
			using ExprType = std::shared_ptr<ASTNode>;

			private:
				std::vector<std::pair<ExprType, BlockType>>	statements;
				bool accepting = true;

			public:
				If(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
				virtual std::string printString(std::string buf);

				void addBlock(ExprType& expr, BlockType& block);

				void setFull();
				bool isFull();
		};

		/*
		 * Node for a function call
		 */
		class FunctionCall : public ASTNode {
			private:
				std::shared_ptr<VarName> fn;
				std::shared_ptr<List<ASTNode>> args;

			public:
				FunctionCall(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);

				std::string toString();
				virtual std::string printString(std::string buf);
		};

		/*
		 * Node for function arguments
		 */
		class Argument : public ASTNode {
			private:
				std::shared_ptr<VarName> var;
				
			public:
				Argument(std::shared_ptr<VarName>& v, bool is_self);
				static std::string node_type;
				const bool self;

				EvalState& eval(EvalState& e);

				std::string toString();
				std::string printString(std::string buf);
		};

		/*
		 * Node for a function definition
		 */
		class FunctionDef : public Control {
			private:
				std::shared_ptr<List<Argument>> args;

			public:
				FunctionDef(const ParseData& in);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				void addChild(std::shared_ptr<ASTNode>& c);
				
				std::string toString();
				virtual std::string printString(std::string buf);

				bool iterate(EvalState& e, size_t loc);
		};

		/*
		 * Node for putting dust functions on the stack
		 */
		class FunctionLiteral : public ASTNode {
			private:
				std::shared_ptr<ASTNode> fn;

			public:
				FunctionLiteral(std::shared_ptr<ASTNode>& fn);
				static std::string node_type;

				EvalState& eval(EvalState& e);

				std::string toString();
				std::string printString(std::string buf);
		};

		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
	}

	template <class T, typename... Args>
	std::shared_ptr<parse::ASTNode> makeNode(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <class R, class T>
	std::shared_ptr<parse::ASTNode> makeNode(std::shared_ptr<T>&& ptr) {
		return std::make_shared<R>(std::forward<std::shared_ptr<T>>(ptr));
	}

	template <class T>
	bool isNode(std::shared_ptr<parse::ASTNode>& p) {
		return std::dynamic_pointer_cast<T>(p) != nullptr;
	}

	template <class ostream>
	void printAST(ostream& s, std::shared_ptr<parse::ASTNode>& ast) {
		(s << ast->printString("|")).flush();
	}
}

