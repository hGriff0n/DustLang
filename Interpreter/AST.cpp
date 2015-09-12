#include "AST.h"
#include <regex>

#include "Exceptions\parsing.h"
#include "Exceptions\dust.h"

namespace dust {
	namespace parse {
		std::string escape(std::string s) {
			std::string ret = s;

			return ret;
		}

		std::string unescape(std::string s) {
			static std::regex escape{ "\\\\\"" };		// Escape \"

			return std::regex_replace(s, escape, "\"");
		}

		void ASTNode::addChild(std::shared_ptr<ASTNode>& c) {
			throw error::operands_error{ "Attempt to add child to node" };
		}

		std::string ASTNode::print_string(std::string buf) {
			return buf + "+- " + node_type + "\n";
		}

		Debug::Debug(std::string _msg) : msg{ _msg } {}
		EvalState& Debug::eval(EvalState& e) { return e; }
		std::string Debug::to_string() { return msg; }
		std::string Debug::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + msg + "\n";
		}

		Literal::Literal(std::string _val, size_t t) : val{ _val }, id{ t } {}
		EvalState& Literal::eval(EvalState& e) {
			if (id == type::Traits<int>::id)
				e.push(std::stoi(val));

			else if (id == type::Traits<double>::id)
				e.push(std::stod(val));

			else if (id == type::Traits<bool>::id)
				e.push<bool>(std::stoi(val));

			else if (id == type::Traits<std::string>::id)
				e.push(val);
			else
				throw error::dust_error{ "No literal can be constructed of the given type" };

			return e;
		}
		std::string Literal::to_string() {
			return (id == type::Traits<int>::id ? " Int " :
				id == type::Traits<double>::id ? " Float " :
				id == type::Traits<bool>::id ? " Bool " :
				id == type::Traits<std::string>::id ? " String \"" :
				" Nil ") + val + (id == type::Traits<std::string>::id ? "\"" : "");
		}
		std::string Literal::print_string(std::string buf) {
			return buf + "+- " + node_type + to_string() + "\n";
		}

		Operator::Operator(std::string o) : l{ nullptr }, r{ nullptr }, op{ o } {}
		EvalState& Operator::eval(EvalState& e) {
			l->eval(e);

			// Binary operator
			if (r ? r->eval(e), true : false)
				e.swap();					// Current Binary operator evalutation expects stack: ..., rhs, lhs

			e.callOp(op);
			return e;
		}
		std::string Operator::to_string() { return op; }
		std::string Operator::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + op + "\n" + l->print_string(buf + " ") + (r ? r->print_string(buf + " ") : "");
		}
		void Operator::addChild(std::shared_ptr<ASTNode>& c) {
			if (!l) l.swap(c);
			else if (!r) r.swap(c);
			else throw error::unimplemented_operation{ "Dust does not currently support ternary operators" };
		}

		VarName::VarName(std::string var) : name{ var } {}
		EvalState& VarName::eval(EvalState& e) {
			e.getVar(name);
			return e;
		}
		std::string VarName::to_string() { return name; }
		std::string VarName::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + name + "\n";
		}

		Assign::Assign(std::string _op, bool _const, bool _static) : setConst{ _const }, setStatic{ _static }, vars{ nullptr }, vals{ nullptr } {
			op = _op.size() ? "_op" + _op : _op;
		}
		EvalState& Assign::eval(EvalState& e) {
			if (!vars) throw error::incomplete_node{ "Attempt to use Assign node without a linked var_list" };
			if (!vals) throw error::incomplete_node{ "Attempt to use Assign node without a linked expr_list" };

			auto r_var = vars->rbegin(), l_var = vars->rend();
			auto l_val = vals->begin(), r_val = vals->end();
			auto var_s = vars->size(), val_s = vals->size();

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
				if (op.size()) (*r_var)->eval(e).callOp(op);
				e.setVar((*r_var++)->to_string(), setConst, setStatic);
			}

			return (*vars->rbegin())->eval(e);

		}
		std::string Assign::to_string() { return op; }
		std::string Assign::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + op + "\n" + vars->print_string(buf + " ") + vals->print_string(buf + " ");
		}
		void Assign::addChild(std::shared_ptr<ASTNode>& c) {
			if (!vars) {
				vars.swap(std::dynamic_pointer_cast<var_type>(c));
				if (vars) return;
			}

			if (!vals) {
				vals.swap(std::dynamic_pointer_cast<val_type>(c));
				if (vals) return;
			}

			if (vars && vals) throw error::operands_error{ "Assignment is a binary operation" };
		}


		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		std::string Literal::node_type = "Literal";
		std::string Operator::node_type = "Operator";
		std::string VarName::node_type = "Variable";
		std::string Assign::node_type = "Assignment";
	}
}