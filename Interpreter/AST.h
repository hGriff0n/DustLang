#pragma once

#include "EvalState.h"

#include <memory>

namespace dust {
	namespace parse {
		// Possibly create grammars and actions for escaping/unescaping strings
		std::string escape(std::string);
		std::string unescape(std::string);

		class ASTNode {
			private:
			public:
				static std::string node_type;

				virtual EvalState& eval(EvalState&) =0;
				virtual std::string to_string() =0;

				virtual void addChild(std::shared_ptr<ASTNode>& c);
				virtual std::string print_string(std::string buf);
		};

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

				std::string to_string() { return ""; }


				void addChild(std::shared_ptr<ASTNode>& c) {
					add(std::dynamic_pointer_cast<Node>(c));
				}

				virtual std::string print_string(std::string buf) {
					std::string ret = buf + "+- " + node_type + "\n";
					buf += " ";

					for (auto i = begin(); i != end(); ++i)		// for (auto i : *this)
						ret += (*i)->print_string(buf);

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

		class Debug : public ASTNode {
			private:
				std::string msg;

			public:
				Debug(std::string _msg);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();

				virtual std::string print_string(std::string buf);
		};

		class Literal : public ASTNode {
			private:
				std::string val;
				size_t id;

			public:
				Literal(std::string _val, size_t t);
				static std::string node_type;

				EvalState& eval(EvalState& e);	// Based off of the old ast implementation
				std::string to_string();		// Possibly temporary implementation

				virtual std::string print_string(std::string buf);
		};

		class Operator : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				std::string op;

			public:
				Operator(std::string o);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();

				virtual std::string print_string(std::string buf);
				void addChild(std::shared_ptr<ASTNode>& c);
		};

		class VarName : public ASTNode {
			private:
				std::string name;

			public:
				VarName(std::string var);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();

				virtual std::string print_string(std::string buf);
		};

		class Assign : public ASTNode {
			typedef List<VarName> var_type;
			typedef List<ASTNode> val_type;
			typedef std::shared_ptr<ASTNode> arg_type;

			private:
				std::shared_ptr<var_type> vars;
				std::shared_ptr<val_type> vals;
				std::string op;
				bool setConst, setStatic;

			public:
				Assign(std::string _op, bool _const = false, bool _static = false);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();

				virtual std::string print_string(std::string buf);
				void addChild(std::shared_ptr<ASTNode>& c);
		};

		// class Keyword : public ASTNode {};

		class BinaryKeyword : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				bool isAnd;

			public:
				BinaryKeyword(std::string key);
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();
	
				virtual std::string print_string(std::string buf);
				void addChild(std::shared_ptr<ASTNode>& c);
		};

		class Block : public ASTNode {
			private:
				std::vector<std::shared_ptr<ASTNode>> expr;

			public:
				Block();
				static std::string node_type;

				EvalState& eval(EvalState& e);
				std::string to_string();

				virtual std::string print_string(std::string buf);
				void addChild(std::shared_ptr<ASTNode>& c);

				auto begin();
				auto end();
				size_t size();
		};

		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
	}

	template <class T, typename... Args>
	std::shared_ptr<parse::ASTNode> makeNode(Args&... args) {
		return std::make_shared<T>(args...);
	}
}
