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
			throw error::operands_error{ "Attempt to add child to ASTNode" };
		}
		std::string ASTNode::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}

		// Debug methods
		Debug::Debug(std::string _msg) : msg{ _msg } {}
		EvalState& Debug::eval(EvalState& e) { return e; }
		std::string Debug::toString() { return msg; }

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
				throw error::dust_error{ "No literal can be constructed from " + val };

			return e;
		}
		std::string Literal::toString() {
			return (id == type::Traits<int>::id ? " Int " :
					id == type::Traits<double>::id ? " Float " :
					id == type::Traits<bool>::id ? " Bool " :
					id == type::Traits<std::string>::id ? " String \"" : " Nil ")
				+ val + (id == type::Traits<std::string>::id ? "\"" : "");
		}
		std::string Literal::printString(std::string buf) {
			return buf + "+- " + node_type + toString() + "\n";
		}

		// Operator methods
		Operator::Operator(std::string o) : l{ nullptr }, r{ nullptr }, op{ o } {}
		EvalState& Operator::eval(EvalState& e) {
			l->eval(e);

			if (r) {
				r->eval(e);
				e.swap();						// Operators expect stack = ..., {rhs}, {lhs}
			}

			return e.callOp(op);
		}
		std::string Operator::toString() { return op; }
		std::string Operator::printString(std::string buf) {
			return buf + "+- " + node_type + " " + op + "\n" + l->printString(buf + " ") + (r ? r->printString(buf + " ") : "");
		}
		void Operator::addChild(std::shared_ptr<ASTNode>& c) {
			if (!l) l.swap(c);
			else if (!r) r.swap(c);
			else throw error::unimplemented_operation{ "Dust does not currently support ternary operators using the Operator node" };
		}

		// VarName methods
		VarName::VarName(std::string var) : name{ var } {}
		EvalState& VarName::eval(EvalState& e) {
			if (sub_var) return e.push(name), e;

			e.push(name);
			e.getScoped(lvl);

			for (auto k : sub_fields)
				k->eval(e).get();

			return e;
		}
		void VarName::addChild(std::shared_ptr<ASTNode>& c) {
			sub_fields.emplace_back(c);
		}
		std::string VarName::toString() { return name; }
		std::string VarName::printString(std::string buf) {
			std::string ret = buf + "+- " + node_type + " " + name + "\n";
			buf += " ";

			for (auto i : sub_fields)
				ret += i->printString(buf);

			return ret;
		}
		void VarName::setSubStatus() {
			sub_var = !sub_var;
		}
		void VarName::addLevel(const std::string& dots) {
			lvl = dots.size();
		}
		EvalState& VarName::set(EvalState& e, bool is_const, bool is_static) {
			if (sub_fields.empty()) {
				e.push(name);
				e.swap();
				e.setScoped(lvl, is_const, is_static);

			} else {
				e.push(name);
				e.getScoped(lvl);

				for (int i = 0; i != sub_fields.size() - 1; ++i) {
					sub_fields[i]->eval(e).get();
					//if (e.is<Nil>()) throw error::dust_error{ "Attempt to assign to a Nil value" };
				}

				e.swap();
				sub_fields.back()->eval(e).swap();
				e.set();
			}

			return e;
		}

		// TypeName methods
		TypeName::TypeName(std::string n) : name{ n } {}
		EvalState& TypeName::eval(EvalState& e) {
			//e.getTS().getType(name);
			return e;
		}
		std::string TypeName::toString() { return name; }
		std::string TypeName::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}

		// TypeCast methods
		TypeCast::TypeCast() {}
		EvalState& TypeCast::eval(EvalState& e) {
			return expr->eval(e).callMethod(convert->toString());
		}
		void TypeCast::addChild(std::shared_ptr<ASTNode>& c) {
			if (!convert && std::dynamic_pointer_cast<TypeName>(c))
				convert.swap(std::dynamic_pointer_cast<TypeName>(c));

			else if (!expr)
				expr.swap(c);

			else
				throw error::invalid_ast_construction{ "Attempt to construct TypeCast Node with multiple expressions" };
		}
		std::string TypeCast::toString() { return convert->toString(); }
		std::string TypeCast::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n" + expr->printString(buf + " ");
		}

		// NewType methods
		NewType::NewType() : name{}, inherit { "Object" } {}
		EvalState& NewType::eval(EvalState& e) {
			auto& ts = e.getTS();
			auto nType = ts.newType(name, ts.get(inherit));

			definition->eval(e);

			//e.pushScope();
			// associate table to type
			//e.pop();

			return e;
		}
		std::string NewType::toString() { return name + " extends " + inherit; }
		std::string NewType::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}
		void NewType::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<TypeName>(c))
				(name == "" ? name : inherit) = c->toString();

			else {
				definition.swap(std::dynamic_pointer_cast<Block>(c));

				if (!definition)
					throw error::invalid_ast_construction{ "Attempt to construct NewType node without a definition (Block) node" };
			}
		}

		// TypeCheck methods
		TypeCheck::TypeCheck() {}
		EvalState& TypeCheck::eval(EvalState& e) {
			auto x = e.size();
			auto res = l->eval(e).pop();						// Possible issue with multiple returns (might pick up the last return, hopefully picks up the first)
			
			e.settop(x);

			if (res.type_id == type::Traits<Nil>::id)			// Nil isn't part of the current type hierarchr
				e.push(type == "Nil");

			else {
				auto& ts = e.getTS();
				e.push(ts.isChildType(res.type_id, ts.getId(type)));
			}

			return e;
		}
		std::string TypeCheck::toString() { return type; }
		std::string TypeCheck::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n" + l->printString(buf + " ");
		}
		void TypeCheck::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<TypeName>(c))
				type = c->toString();

			else if (!l)
				l.swap(c);

			else
				throw error::invalid_ast_construction{ "Attempt to construct TypeCheck node with more than one expression" };
		}

		// Assign methods
		Assign::Assign(std::string _op, bool _const, bool _static) : setConst{ _const }, setStatic{ _static }, vars{ nullptr }, vals{ nullptr } {
			op = _op.size() ? "_op" + _op : _op;
		}
		EvalState& Assign::eval(EvalState& e) {
			if (!vars) throw error::bad_node_eval{ "Attempt to use Assign node without a linked var_list" };
			if (!vals) throw error::bad_node_eval{ "Attempt to use Assign node without a linked expr_list" };

			auto r_var = vars->rbegin(), l_var = vars->rend();
			auto l_val = vals->begin(), r_val = vals->end();
			auto var_s = vars->size(), val_s = vals->size();

			// This code is currently not suited to multiple returns and the splat operator
				// For multiple returns, combining the next two loops should work

			// More values than variables. Readjust val
			while (val_s > var_s) {
				--r_val; --val_s;
			}

			// Evaluate expression list (left -> right)
			while (l_val != r_val)
				(*l_val++)->eval(e);

			// More variables than values. Push nils
				// Might change to copy() depending on compound assignment semantics (extend the last value to match)
			while (var_s > val_s) {
				e.pushNil(); --var_s;
			}

			// Perform assignments. Compound if necessary
			//auto last_var = r_var;
			while (r_var != l_var) {
				if (op.size()) (*r_var)->eval(e).callOp(op);
				
				(*r_var++)->set(e, setConst, setStatic);
			}

			return (*vars->rbegin())->eval(e);				//return last_var->eval(e);
		}
		std::string Assign::toString() { return op; }
		std::string Assign::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n" + vars->printString(buf + " ") + vals->printString(buf + " ");
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

		// BooleanOperator methods
		BooleanOperator::BooleanOperator(std::string key) : l{ nullptr }, r{ nullptr }, isAnd{ key == "and" } {}
		EvalState& BooleanOperator::eval(EvalState& e) {
			if (!l || !r) throw error::bad_node_eval{ "Attempt to use BinaryKeyword node with less than two operands" };

			l->eval(e).copy();						// Copy does perform reference incrementing

			// Short circuit evaluation
			if (isAnd) {
				if (!e.pop<bool>())	return e;		// and: if lhs == false return immediately

			} else if (e.pop<bool>())
				return e;							// or: if lhs == true return immediately
			

			// Otherwise leave the right argument on the stack
			e.pop();
			return r->eval(e);
		}
		std::string BooleanOperator::toString() { return isAnd ? "and" : "or"; }
		std::string BooleanOperator::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n" + l->printString(buf + " ") + (r ? r->printString(buf + " ") : "");
		}
		void BooleanOperator::addChild(std::shared_ptr<ASTNode>& c) {
			if (!l) l.swap(c);
			else if (!r) r.swap(c);
			else throw error::operands_error{ "Attempt to add more than three operands to BinaryKeyword node" };
		}

		// Control methods
		Control::Control() : Control{ -1 } {}
		Control::Control(int typ) : type{ typ }, next{ true } {}
		EvalState& Control::eval(EvalState& e) {
			switch (type) {
				case Type::FOR:
					break;
				case Type::WHILE:
					break;
				default:
					next = true;
					break;										// Removing this line causes C2059: syntax error: '}'
			}

			return e;
		}
		void Control::addChild(std::shared_ptr<ASTNode>& c) {
			if (expr) throw error::operands_error{ "Attempt to construct control node with more than two expresions" };
			expr = c;
		}
		std::string Control::toString() { return ""; }
		std::string Control::printString(std::string buf) {
			return "";
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

			// Settop is handled by Block::eval
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

			size_t x = e.size(), next = 1;

			e.newScope();
			control->eval(e);								// Perform loop setup (if needed)

			while (control->iterate(e)) {
				for (const auto& i : *this) {
					e.settop(x);							// Pops the results of the last expression (Not executed for the last expression)

					try {
						i->eval(e);							// Dust Exceptions are handled by a surrounding TryCatch node (Block just needs to reset scoping)

					} catch (...) {
						e.endScope();						// Ensure scoping gets destroyed
						throw;
					}

					if (table && !std::dynamic_pointer_cast<Assign>(i)) {
						e.push<int>(next++);
						e.swap();
						e.setScoped();					// WARNING! Might give "wrong" answers
					}
				}
			}

			if (save_scope) {
				e.settop(x);
				e.pushScope(next);

			} else
				e.endScope();

			return e;
		}
		std::string Block::toString() {
			return table ? " []" : "";
		}
		std::string Block::printString(std::string buf) {
			std::string ret = buf + "+- " + node_type + toString() + "\n";
			buf += " ";

			for (auto i : *this)
				ret += i->printString(buf);

			return ret;
		}
		void Block::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<Control>(c)) {
				if (control) throw error::operands_error{ "Attempt to construct Block with multiple Control nodes" };
				control.swap(std::dynamic_pointer_cast<Control>(c));

			} else
				expr.push_back(c);
		}

		// TryCatch methods
		TryCatch::TryCatch() {}
		EvalState& TryCatch::eval(EvalState& e) {
			if (!try_code || !catch_code) throw error::bad_node_eval{ "Attempt to evaluate an incomplete TryCatch node" };

			// Should I mirror block or table evaluation
				// The question is, should variables in the try block be local

			try {
				try_code->eval(e);

			} catch (error::dust_error& err) {
				e.push(err.what());
				catch_code->eval(e);
			}

			return e;
		}
		std::string TryCatch::toString() { return ""; }
		std::string TryCatch::printString(std::string buf) {
			return buf + "+- " + node_type + "::try\n" + try_code->printString(buf + " ") +
				   buf + "+- " + node_type + "::catch\n" + catch_code->printString(buf + " ");
		}
		void TryCatch::addChild(std::shared_ptr<ASTNode>& c) {
			auto& code_block = catch_code ? try_code : catch_code;
			if (code_block) throw error::operands_error{ "Attempt to add more than two blocks to TryCatch node" };

			code_block.swap(c);
			if (!code_block) throw error::invalid_ast_construction{ "Attempt to construct TryCatch node with a non-Block node" };
		}


		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		std::string Literal::node_type = "Literal";
		std::string Operator::node_type = "Operator";
		std::string VarName::node_type = "Variable";
		std::string TypeName::node_type = "Type";
		std::string TypeCast::node_type = "TypeCast";
		std::string NewType::node_type = "NewType";
		std::string TypeCheck::node_type = "TypeCheck";
		std::string Assign::node_type = "Assignment";
		std::string BooleanOperator::node_type = "Boolean";
		std::string Control::node_type = "Control";
		std::string Block::node_type = "Block";
		std::string TryCatch::node_type = "Try-Catch";
	}
}