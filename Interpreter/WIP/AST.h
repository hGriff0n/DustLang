#pragma once

#include "EvalState.h"

#include <memory>

namespace dust {
	namespace interpreter {

		class ASTNode {
			private:
			public:
				virtual EvalState& eval(EvalState&) =0;
				virtual std::string to_string() =0;

				static std::string node_type;

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + "\n";
				}
		};

		template <class Node>
		class List : public ASTNode {
			private:
				//std::vector<Node*> elems;
				std::vector<std::shared_ptr<Node>> elems;

			public:
				template <class... Ns>
				List(Ns... ns) {
					add(ns...);
				}

				EvalState& eval(EvalState& e) {
					// throw std::string{ "Attempt to evaluate a List node" };

					for (auto i = begin(); i != end(); ++i)
						(*i)->eval(e);

					return e;
				}

				// Assuming sub-nodes are stored right->left
					// List(a, b, c) => List.elems = { c, b, a }
				auto rbegin() { return elems.begin(); }
				auto rend() { return elems.end(); }
				auto begin() { return elems.rbegin(); }
				auto end() { return elems.rend(); }

				size_t size() { return elems.size(); }
				List& add(std::shared_ptr<Node>& n) {
				//List& add(Node* n) {
					if (n) elems.push_back(n);
					return *this;
				}

				List& add() {
					return *this;
				}

				template <class... Ns>
				List& add(std::shared_ptr<ASTNode>& n, Ns... ns) {
				//List& add(Node* n, Ns... ns) {
					//add(ns...);
					//return add(std::dynamic_pointer_cast<Node>(n));			// add(nullptr) if n is not castable to a shared_ptr<Node>
					add(std::dynamic_pointer_cast<Node>(n));
					return add(ns...);
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
				EvalState& eval(EvalState& e) { return e; }
				std::string to_string() { return msg; }

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + " " + msg + "\n";
				}

				static std::string node_type;
		};

		class Literal : public ASTNode {
			private:
				std::string val;
				size_t id;
			
			public:
				Literal(std::string v, size_t t) : val{ v }, id{ t } {}

				// Based off of the old ast implementation
				EvalState& eval(EvalState& e) {
					if (id == TypeTraits<int>::id)
						e.push(std::stoi(val));

					else if (id == TypeTraits<double>::id)
						e.push(std::stod(val));

					else if (id == TypeTraits<bool>::id)
						e.push<bool>(std::stoi(val));

					else if (id == TypeTraits<std::string>::id)
						e.push(val);

					else
						throw std::string{ "No literal" };

					return e;
				}

				// Possibly temporary implementation
				std::string to_string() {
					return (id == TypeTraits<int>::id ? " Int " :
							id == TypeTraits<double>::id ? " Float " :
							id == TypeTraits<bool>::id ? " Bool " :
							id == TypeTraits<std::string>::id ? " String " :
							" Nil ") + val;
				}

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + to_string() + "\n";
				}

				static std::string node_type;
		};

		class Operator : public ASTNode {
			private:
				//ASTNode *l, *r;
				std::shared_ptr<ASTNode> l, r;
				std::string op;

			public:
				//Operator(std::string o, ASTNode* lhs, ASTNode* rhs = nullptr) : l{ lhs }, r{ rhs }, op{ o } {}
				Operator(std::string o, std::shared_ptr<ASTNode>& lhs) : l{ lhs }, r{ nullptr }, op{ o } {}
				Operator(std::string o, std::shared_ptr<ASTNode>& lhs, std::shared_ptr<ASTNode>& rhs) : l{ lhs }, r{ rhs }, op{ o } {}

				EvalState& eval(EvalState& e) {
					l->eval(e);

					// Binary operator
					if (r ? r->eval(e), true : false)
						e.swap();					// Current Binary operator evalutation expects stack: ..., rhs, lhs

					e.callOp(op);
					return e;
				}

				std::string to_string() { return op; }

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + " " + op + "\n" + l->print_string(buf + " ") + r->print_string(buf + " ");
				}

				static std::string node_type;
		};

		class VarName : public ASTNode {
			private:
				std::string name;

			public:
				VarName(std::string var) : name{ var } {}

				EvalState& eval(EvalState& e) {
					e.getVar(name);
					return e;
				}

				std::string to_string() { return name; }

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + " " + name + "\n";
				}

				static std::string node_type;
		};

		class Assign : public ASTNode {
			typedef List<VarName> var_type;
			typedef List<ASTNode> val_type;
			typedef std::shared_ptr<ASTNode> arg_type;

			private:
				//List<VarName>* vars;
				//List<ASTNode>* vals;
				std::shared_ptr<var_type> vars;
				std::shared_ptr<val_type> vals;
				std::string op;
				bool setConst, setStatic;

				Assign(std::shared_ptr<var_type>& l, std::shared_ptr<val_type>& r, std::string o, bool c, bool s) : vars{ l }, vals{ r }, op{ "_op" + o }, setConst{ c }, setStatic{ s } {
					if (!l) throw std::string{ "Attempt to construct Assign node without a var_list" };
					if (!r) throw std::string{ "Attempt to construct Assign node without a val_list" };
				}

			public:
				//Assign(List<VarName>* l, List<ASTNode>* r, std::string o, bool c = false, bool s = false) : vars{ l }, vals{ r }, op{ "_op" + o }, setConst{ c }, setStatic{ s } {}
				Assign(arg_type& l, arg_type& r, std::string o, bool c = false, bool s = false) : Assign{ std::dynamic_pointer_cast<var_type>(l), std::dynamic_pointer_cast<val_type>(r), o, c, s } {}

				EvalState& eval(EvalState& e) {
					auto r_var = vars->rbegin(), l_var = vars->rend();
					auto l_val = vals->begin(), r_val = vals->end();
					auto var_s = vars->size(), val_s = vals->size();
					bool compound = op.size() - 3;

					// This code is currently not suited to multiple returns and the splat operator

					// More values than variables. Readjust val
					while (val_s > var_s) {
						--r_val; --val_s;
					}

					// Evaluate expression list (left -> right)
					while (l_val != r_val)
						(*l_val++)->eval(e);

					// More variables than values. Push nils
						// Might change to copy() depending on compound assignment semantics
					while (var_s > val_s) {
						e.push(0); --var_s;
					}

					// Perform assignments. Compound if necessary
					while (r_var != l_var) {
						if (compound) (*r_var)->eval(e).callOp(op);
						e.setVar((*r_var++)->to_string(), setConst, setStatic);
					}

					return (*vars->begin())->eval(e);
				}

				std::string to_string() { return op; }

				virtual std::string print_string(std::string buf) {
					return buf + "+- " + node_type + " " + op + "\n" + vars->print_string(buf + " ") + vals->print_string(buf + " ");
				}

				static std::string node_type;
		};


		// Have to move into the .cpp file
		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
		std::string Literal::node_type = "Literal";
		std::string Operator::node_type = "Operator";
		std::string VarName::node_type = "Variable";
		std::string Assign::node_type = "Assignment";
	}

	// shared_ptr<List<VarName>> cannot be used to initialize a Assign node using makeNode (for some reason)
	// makeNode has to return a shared_ptr<ASTNode> that can then be cast to the shared_ptr<List<VarName>>
		// Note: std::dynamic_cast returns a nullptr if the shared_ptr cannot be cast to the desired type
	template <class T, typename... Args>
	std::shared_ptr<interpreter::ASTNode> makeNode(Args&... args) {
		return std::make_shared<T>(args...);
	}
}
