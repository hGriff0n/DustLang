#pragma once

#include "EvalState.h"

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
				std::vector<Node*> elems;

			public:
				EvalState& eval(EvalState& e) {
					// throw std::string{ "Attempt to evaluate a List node" };

					for (auto i = begin(); i != end(); ++i)
						(*i)->eval(e);

					return e;
				}

				//*
				// Assuming sub-nodes are stored right->left
					// List(a, b, c) => List.elems = { c, b, a }
				auto rbegin() { return elems.begin(); }
				auto rend() { return elems.end(); }
				auto begin() { return elems.rbegin(); }
				auto end() { return elems.rend(); }

				/*/
				// Assuming sub-nodes are stored left->right
					// List(a, b, c) => List.elems = { a, b, c }
				auto rbegin() { return elems.rbegin(); }
				auto rend() { return elems.rend(); }
				auto begin() { return elems.begin(); }
				auto end() { return elems.end(); }
				//*/

				size_t size() { return elems.size(); }
				List& add(Node* n) {
					elems.push_back(n);
					return *this;
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
				ASTNode *l, *r;
				std::string op;

			public:
				Operator(std::string o, ASTNode* lhs, ASTNode* rhs = nullptr) : l{ lhs }, r{ rhs }, op{ o } {}

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
					std::string s = buf + "+- " + node_type + " " + op + "\n";
					s += l->print_string(buf + " ");
					s += r->print_string(buf + " ");
					return s;
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
			private:
				List<VarName>* vars;
				List<ASTNode>* vals;
				std::string op;

			public:
				Assign(List<VarName>* l, List<ASTNode>* r, std::string o) : vars{ l }, vals{ r }, op{ "_op" + o } {}

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
						e.setVar((*r_var++)->to_string());
					}

					return (*vars->begin())->eval(e);
				}

				std::string to_string() { return op; }

				virtual std::string print_string(std::string buf) {
					std::string s = buf + "+- " + node_type + " " + op + "\n";
					s += vars->print_string(buf + " ") + vals->print_string(buf + " ");
					return s;
				}

				static std::string node_type;
		};


		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		template<class T> std::string List<T>::node_type = "List<" + T::node_type + ">";
		std::string Literal::node_type = "Literal";
		std::string Operator::node_type = "Operator";
		std::string VarName::node_type = "Variable";
		std::string Assign::node_type = "Assignment";
	}

	// If I move to smart pointers
	//template <class T, typename... Args>
	//auto makeNode(Args... args) {
	//	return std::make_shared<T>(args...);
	//}
}
