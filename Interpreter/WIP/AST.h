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
		};

		template <class Node>
		class List : public ASTNode {
			private:
				std::vector<Node*> elems;

			public:
				EvalState& eval(EvalState& e) {
					for (auto i = begin(); i != end(); ++i)
						(*i)->eval(e);

					return e;
				}

				// Assuming elems is constructed left->right
				auto rbegin() { return elems.begin(); }
				auto rend() { return elems.end(); }
				auto begin() { return elems.rbegin(); }
				auto end() { return elems.rend(); }
				size_t size() { return elems.size(); }

				std::string to_string() { return ""; }

				static std::string node_type;
		};

		class Debug : public ASTNode {
			private:
				std::string msg;

			public:
				EvalState& eval(EvalState& e) { return e; }
				std::string to_string() { return msg; }

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
					return (id == TypeTraits<int>::id ? "Int " :
							id == TypeTraits<double>::id ? "Float " :
							id == TypeTraits<bool>::id ? "Bool " :
							id == TypeTraits<std::string>::id ? "String " :
							"Nil ") + val;
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
						if (compound) (*r_var)->eval(e).call( op);
						e.setVar((*r_var++)->to_string());
					}

					return (*vars->begin())->eval(e);
				}

				std::string to_string() { return op; }

				static std::string node_type;
		};


		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		template<class T> std::string List<T>::node_type = "<" + T::node_Type + ">";
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

/*/
Old AST pretty print

template <class T>
void print(T* ast) {
	print(ast, "|");
}

template <class T>
void print(T* ast, std::string buffer) {
	std::cout << buffer << "+- " << T::node_type << " " << ast->to_string() << std::endl;
	buffer += " ";

	for (auto& n : *ast)
		print(n, buffer);
}

//*/