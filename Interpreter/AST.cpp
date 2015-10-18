#include "AST.h"
#include <regex>
#include <cctype>

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

		std::string trim(std::string s) {
			auto front = std::find_if_not(s.begin(), s.end(), std::isspace);
			auto back = std::find_if_not(s.rbegin(), s.rend(), std::isspace).base();
			return back <= front ? "" : std::string{front, back};
		}

		// ASTNode methods
		void ASTNode::addChild(std::shared_ptr<ASTNode>& c) {
			throw error::operands_error{ "Attempt to add child to node" };
		}
		std::string ASTNode::print_string(std::string buf) {
			return buf + "+- " + node_type + "\n";
		}

		// Debug methods
		Debug::Debug(std::string _msg) : msg{ _msg } {}
		EvalState& Debug::eval(EvalState& e) { return e; }
		std::string Debug::to_string() { return msg; }
		std::string Debug::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + msg + "\n";
		}

		// Literal methods
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

			else if (id == type::Traits<Nil>::id)
				e.pushNil();

			else
				throw error::dust_error{ "No literal can be constructed of the given type" };

			return e;
		}
		std::string Literal::to_string() {
			return (id == type::Traits<int>::id ? " Int " :
					id == type::Traits<double>::id ? " Float " :
					id == type::Traits<bool>::id ? " Bool " :
					id == type::Traits<std::string>::id ? " String \"" : " Nil ")
				+ val + (id == type::Traits<std::string>::id ? "\"" : "");
		}
		std::string Literal::print_string(std::string buf) {
			return buf + "+- " + node_type + to_string() + "\n";
		}

		// Operator methods
		Operator::Operator(std::string o) : l{ nullptr }, r{ nullptr }, op{ o } {}
		EvalState& Operator::eval(EvalState& e) {
			l->eval(e);

			if (r) {
				r->eval(e);
				e.swap();				// Current Binary operator evalutation expects stack: ..., rhs, lh
			}

			return e.callOp(op);
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

		// VarName methods
		VarName::VarName(std::string var) : name{ var } {}
		EvalState& VarName::eval(EvalState& e) {
			e.get(name);
			return e;
		}
		std::string VarName::to_string() { return name; }
		std::string VarName::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + name + "\n";
		}

		// NewType methods
		NewType::NewType() : name{}, inherit { "Object" } {}
		EvalState& NewType::eval(EvalState& e) {
			auto& ts = e.getTS();
			auto nType = ts.newType(name, ts.get(inherit));

			//definition->eval(e);
			e.pushNil();

			return e;
		}
		std::string NewType::to_string() { return name + " extends " + inherit; }
		std::string NewType::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + to_string() + "\n";
		}
		void NewType::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<Debug>(c))
				(name == "" ? name : inherit) = c->to_string();

			else {
				definition.swap(std::dynamic_pointer_cast<Block>(c));

				if (!definition)
					throw error::invalid_ast_construction{ "Attempt to construct NewType node with a non Block node" };
			}
		}

		// TypeCheck methods
		TypeCheck::TypeCheck() {}
		EvalState& TypeCheck::eval(EvalState& e) {
			auto x = e.size();
			auto res = l->eval(e).pop();						// Possible issue with multiple returns
			
			e.settop(x);

			auto& ts = e.getTS();
			e.push(ts.isChildType(res.type_id, ts.getId(type)));

			return e;
		}
		std::string TypeCheck::to_string() { return type; }
		std::string TypeCheck::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + to_string() + "\n" + l->print_string(buf + " ");
		}
		void TypeCheck::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<Debug>(c))
				type = c->to_string();

			else if (l)
				throw error::invalid_ast_construction{ "Attempt to construct TypeCheck node with more than one expression" };

			else
				l.swap(c);
		}

		// Assign methods
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
			//auto last_var = r_var;
			while (r_var != l_var) {
				if (op.size()) (*r_var)->eval(e).callOp(op);
				e.set((*r_var++)->to_string(), setConst, setStatic);
			}

			//return last_var->eval(e);
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

		// BinaryKeyword methods
		BinaryKeyword::BinaryKeyword(std::string key) : l{ nullptr }, r{ nullptr }, isAnd{ key == "and" } {}
		EvalState& BinaryKeyword::eval(EvalState& e) {
			if (!l || !r) throw error::bad_node_eval{ "Attempt to use BinaryKeyword node with less than two operands" };

			l->eval(e).copy();						// Copy does perform reference incrementing

													// Short circuit evaluation
			if (isAnd) {
				if (!e.pop<bool>())	return e;		// and: if lhs == false return immediately

			} else if (e.pop<bool>()) {
				return e;							// or: if lhs == true return immediately
			}

			// Otherwise leave the right argument on the stack
			e.pop();
			return r->eval(e);
		}
		std::string BinaryKeyword::to_string() { return isAnd ? "and" : "or"; }
		std::string BinaryKeyword::print_string(std::string buf) {
			return buf + "+- " + node_type + " " + to_string() + "\n" + l->print_string(buf + " ") + (r ? r->print_string(buf + " ") : "");
		}
		void BinaryKeyword::addChild(std::shared_ptr<ASTNode>& c) {
			if (!l) l.swap(c);
			else if (!r) r.swap(c);
			else throw error::operands_error{ "Attempt to add more than three operands to BinaryKeyword node" };
		}

		// Control methods
		Control::Control() : Control{ -1 } {}
		Control::Control(char typ) : type{ typ } {}
		EvalState& Control::eval(EvalState& e) {
			throw error::bad_node_eval{ "Attempt to evaluate a Control node" };
		}
		bool Control::iterate(EvalState& e) {
			switch (type) {
				case 0:				// for
				case 1:				// while
					return (bool)expr->eval(e);
				case 2:				// do-while
					if (!next)
						return (bool)expr->eval(e);
				default:
					next = !next;
					return !next;
			}
			// Needs a "reset" function

			// Settop is handled by Block::eval
		}
		void Control::addChild(std::shared_ptr<ASTNode>& c) {
			if (expr) throw error::invalid_ast_construction{ "Attempt to construct control node from more than two expresions" };
			expr = c;
		}
		std::string Control::to_string() { return ""; }
		std::string Control::print_string(std::string buf) {
			return "";
		}

		// Block methods
		Block::Block() : save_scope{ false }, table{ false }, excep_if_empty{ true } {}
		auto Block::begin() {
			return expr.rbegin();
		}
		auto Block::end() {
			return expr.rend();
		}
		size_t Block::size() {
			return expr.size();
		}
		EvalState& Block::eval(EvalState& e) {
			if (expr.empty() && !table) {
				if (!excep_if_empty) {
					e.pushNil();
					return e;
				}

				throw error::bad_node_eval{ "Attempt to evaluate an empty block" };
			}

			size_t x = e.size(), n_key = 1;
			e.newScope();

			//while (control->iterate(e)) {
			for (const auto& i : *this) {
				e.settop(x);							// Pops the results of the last expression (Not executed for the last expression)

				try {
					i->eval(e);							// Dust Exceptions are handled by a surrounding TryCatch node (Block just needs to reset scoping)

				} catch (...) {
					e.endScope();
					throw;
				}

				if (table && !std::dynamic_pointer_cast<Assign>(i)) {
					// perform default assignment
					++n_key;
				}
			}
			//}

			if (save_scope) {
				e.settop(x);
				e.pushScope();
			} else
				e.endScope();

			return e;
		}
		std::string Block::to_string() {
			return table ? " []" : "";
		}
		std::string Block::print_string(std::string buf) {
			std::string ret = buf + "+- " + node_type + to_string() + "\n";
			buf += " ";

			for (auto i : *this)
				ret += i->print_string(buf);

			return ret;
		}
		void Block::addChild(std::shared_ptr<ASTNode>& c) {
			expr.push_back(c);
		}

		// TryCatch methods
		TryCatch::TryCatch() {}
		EvalState& TryCatch::eval(EvalState& e) {
			if (!try_code || !catch_code) throw error::bad_node_eval{ "Attempt to evaluate an incomplete TryCatch node" };

			// Should I mirror block evaluation
				// Or add a flag to block to prevent it from creating/destroying scope
			// This code follows the second option (although the flag(s) are currently unimplemented)

			try {
				try_code->eval(e);

			} catch (error::dust_error& err) {
				e.push(err.what());
				catch_code->eval(e);
			}

			return e;
		}
		std::string TryCatch::to_string() { return ""; }
		std::string TryCatch::print_string(std::string buf) {
			return buf + "+- " + node_type + "::try\n" + try_code->print_string(buf + " ") +
				   buf + "+- " + node_type + "::catch\n" + catch_code->print_string(buf + " ");
		}
		void TryCatch::addChild(std::shared_ptr<ASTNode>& c) {
			auto& code_block = catch_code ? try_code : catch_code;
			if (code_block) throw error::operands_error{ "Attempt to add more than two blocks to TryCatch node" };

			code_block.swap(std::dynamic_pointer_cast<Block>(c));
			if (!code_block) throw error::invalid_ast_construction{ "Attempt to construct TryCatch node with a non-Block node" };
		}


		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		std::string Literal::node_type = "Literal";
		std::string Operator::node_type = "Operator";
		std::string VarName::node_type = "Variable";
		std::string NewType::node_type = "Type";
		std::string TypeCheck::node_type = "TypeCheck";
		std::string Assign::node_type = "Assignment";
		std::string BinaryKeyword::node_type = "Boolean";
		std::string Control::node_type = "Control";
		std::string Block::node_type = "Block";
		std::string TryCatch::node_type = "Try-Catch";
	}
}