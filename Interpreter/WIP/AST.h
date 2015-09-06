#pragma once

#include "EvalState.h"

#include <memory>

namespace dust {
	namespace parse {

		class ASTNode {
			private:
			public:
				virtual EvalState& eval(EvalState&) =0;
				virtual std::string to_string() =0;
				virtual void addChild(std::shared_ptr<ASTNode>& c);

				static std::string node_type;

				virtual std::string print_string(std::string buf);
		};

		template <class Node>
		class List : public ASTNode {
			private:
				std::vector<std::shared_ptr<Node>> elems;

			public:
				List() {}

				EvalState& eval(EvalState& e) {
					// throw std::string{ "Attempt to evaluate a List node" };

					for (auto i = begin(); i != end(); ++i)
						(*i)->eval(e);

					return e;
				}

				// Assuming sub-nodes are stored left->right
					// List(a, b, c) => List.elems = { a, b, c }
				auto rbegin() { return elems.begin(); }
				auto rend() { return elems.end(); }
				auto begin() { return elems.rbegin(); }
				auto end() { return elems.rend(); }

				size_t size() { return elems.size(); }
				
				
				List& add(std::shared_ptr<Node>& n) {
					if (n) elems.push_back(n);
					return *this;
				}

				void addChild(std::shared_ptr<ASTNode>& c) {
					add(std::dynamic_pointer_cast<Node>(c));
				}

				std::string to_string() { return ""; }

				virtual std::string print_string(std::string buf) {
					std::string ret = buf + "+- " + node_type + "\n";
					buf += " ";

					for (auto i = begin(); i != end(); ++i)
						ret += (*i)->print_string(buf);

					return ret;
				}

				static std::string node_type;
		};

		class Debug : public ASTNode {
			private:
				std::string msg;

			public:
				Debug(std::string _msg);

				EvalState& eval(EvalState& e);
				std::string to_string();

				// Do I need to have this here
				virtual std::string print_string(std::string buf);

				static std::string node_type;
		};

		class Literal : public ASTNode {
			private:
				std::string val;
				size_t id;
			
			public:
				Literal(std::string _val, size_t t);

				// Based off of the old ast implementation
				EvalState& eval(EvalState& e);

				// Possibly temporary implementation
				std::string to_string();

				virtual std::string print_string(std::string buf);

				static std::string node_type;
		};

		class Operator : public ASTNode {
			private:
				std::shared_ptr<ASTNode> l, r;
				std::string op;

			public:
				Operator(std::string o);

				EvalState& eval(EvalState& e);

				std::string to_string();

				// Access violation reading memory address 0
					// Unary operators have r == nullptr
				virtual std::string print_string(std::string buf);

				static std::string node_type;

				void addChild(std::shared_ptr<ASTNode>& c);
		};

		class VarName : public ASTNode {
			private:
				std::string name;

			public:
				VarName(std::string var);

				EvalState& eval(EvalState& e);

				std::string to_string();

				virtual std::string print_string(std::string buf);

				static std::string node_type;
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

				EvalState& eval(EvalState& e);

				std::string to_string();

				virtual std::string print_string(std::string buf);

				static std::string node_type;

				void addChild(std::shared_ptr<ASTNode>& c);
		};

		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
	}

	// shared_ptr<List<VarName>> cannot be used to initialize a Assign node using makeNode (for some reason)
	// makeNode has to return a shared_ptr<ASTNode> that can then be cast to the shared_ptr<List<VarName>>
		// Note: std::dynamic_cast returns a nullptr if the shared_ptr cannot be cast to the desired type
	template <class T, typename... Args>
	std::shared_ptr<parse::ASTNode> makeNode(Args&... args) {
		return std::make_shared<T>(args...);
	}
}
