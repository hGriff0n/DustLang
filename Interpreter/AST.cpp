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

		// ParseData methods
		ParseData::ParseData(const pegtl::input& in) : col{ in.column() }, line{ in.line() } {}

		// ASTNode methods
		ASTNode::ASTNode(const ParseData& in) : p{ in } {}
		void ASTNode::addChild(std::shared_ptr<ASTNode>& c) {
			throw error::operands_error{ "Attempt to add child to ASTNode" };
		}
		std::string ASTNode::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}

		// Debug methods
		Debug::Debug(const ParseData& in, std::string _msg) : ASTNode{ in }, msg{ _msg } {}
		EvalState& Debug::eval(EvalState& e) { return e; }
		std::string Debug::toString() { return msg; }

		// Literal methods
		Literal::Literal(const ParseData& in, std::string _val, size_t t) : ASTNode{ in }, val{ _val }, id{ t } {}
		EvalState& Literal::eval(EvalState& e) {
			if (id == type::Traits<int>::id)
				e.push(std::stoi(val));

			else if (id == type::Traits<double>::id)
				e.push(std::stod(val));

			else if (id == type::Traits<bool>::id)
				e.push(val == "true");

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

		// Value methods
		Value::Value(const ParseData& in, impl::Value v) : ASTNode{ in }, val { v } {}
		EvalState& Value::eval(EvalState& e) {
			e.push(val);
			return e;
		}
		std::string Value::toString() { return ""; }
		std::string Value::printString(std::string buf) { return ""; }

		// Operator methods
		Operator::Operator(const ParseData& in, std::string o) : ASTNode{ in }, l{ nullptr }, r{ nullptr }, op{ o } {}
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
		VarName::VarName(const ParseData& in, std::string var) : VarName{ makeNode<Literal>(in, var, type::Traits<std::string>::id) } {}
		VarName::VarName(std::shared_ptr<ASTNode>&& var) : ASTNode{ var->p } {
			fields.emplace_back(var);
		}
		void VarName::addChild(std::shared_ptr<ASTNode>& c) {
			fields.emplace_back(c);
		}
		std::string VarName::toString() { return fields.front()->toString(); }
		std::string VarName::printString(std::string buf) {
			std::string ret = buf + "+- " + node_type + "\n";
			buf += " ";

			for (auto i : fields)
				ret += i->printString(buf);

			return ret;
		}
		void VarName::setSubStatus() { sub_var = !sub_var; }
		void VarName::addLevel(const std::string& dots) {
			lvl = dots.size();
		}
		EvalState& VarName::eval(EvalState& e) {
			auto field = std::begin(fields);
			(*field)->eval(e);

			if (!sub_var) {
				e.getScoped(lvl);

				while (++field != std::end(fields))
					(*field)->eval(e).get();
			}

			return e;
		}
		EvalState& VarName::set(EvalState& e, bool is_const, bool is_static) {
			auto field = std::begin(fields), end = std::end(fields) - 1;
			(*field)->eval(e);

			if (field == end) {
				e.swap();
				e.setScoped(lvl, is_const, is_static);

			} else {
				e.getScoped(lvl);

				while (++field != end)
					(*field)->eval(e).get();
					//if (e.is<Nil>()) throw error::dust_error{ "Attempt to assign to a Nil value" };

				e.swap();
				(*end)->eval(e).swap();
				e.set();
			}

			return e;
		}

		// TypeName methods
		TypeName::TypeName(const ParseData& in, std::string n) : ASTNode{ in }, name{ n } {}
		EvalState& TypeName::eval(EvalState& e) {
			//e.getTS().getType(name);
			return e;
		}
		std::string TypeName::toString() { return name; }
		std::string TypeName::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}

		// TypeCast methods
		TypeCast::TypeCast(const ParseData& in) : ASTNode{ in } {}
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
		NewType::NewType(const ParseData& in) : ASTNode{ in }, name{}, inherit{ "Object" } {}
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
		TypeCheck::TypeCheck(const ParseData& in) : ASTNode{ in } {}
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
		Assign::Assign(const ParseData& in, std::string _op, bool _const, bool _static) : ASTNode{ in }, set_const{ _const }, set_static{ _static }, vars{ nullptr }, vals{ nullptr } {
			op = _op.size() ? "_op" + _op : _op;
		}
		EvalState& Assign::eval(EvalState& e) {
			if (!vars) throw error::bad_node_eval{ "Attempt to use Assign node without a linked var_list" };
			if (!vals) throw error::bad_node_eval{ "Attempt to use Assign node without a linked expr_list" };

			auto l_var = vars->begin(), r_var = vars->end();
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
				e.pushNil(); ++val_s;
			}

			// Reverse the stack to enable left->right evaluation
			for (int top = -1, bottom = -(int)val_s; top > bottom; --top, ++bottom)
				e.swap(top, bottom);

			// Perform assignments. Compound if necessary
			while (r_var != l_var) {
				if (op.size()) (*l_var)->eval(e).callOp(op);

				(*l_var++)->set(e, set_const, set_static);
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
		BooleanOperator::BooleanOperator(const ParseData& in, std::string key) : ASTNode{ in }, l{ nullptr }, r{ nullptr }, isAnd{ key == "and" } {}
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
		Control::Control(const ParseData& in, Type typ) : ASTNode{ in }, type{ typ }, next{ true } {}
		EvalState& Control::eval(EvalState& e) {
			switch (type) {
				case Type::FOR:
					if (!expr->eval(e).is<Table>())
						throw error::bad_node_eval{ "Attempt to iterate over a non-table" };

					e.pushNil();

					// Generator Abstraction (needs work)
					/*

					// Get generator function into local state (memoize)
					if (???) {

						// Special handling for default generators
						if (e.is<Table>())
							e.copy();
							e.push("_iterator");
							e.get();					// Get the table's iterator

						} // else if (e.is<?>()) {}		// custom data types

						generator = e.pop();
						??? = false;
					}

					// Push initial arguments
					e.pushNil();
					*/

					break;
				case Type::TRY_CATCH:
					std::dynamic_pointer_cast<VarName>(expr)->set(e, true, false);
					break;
				case Type::WHILE:
					e.pushNil();
					break;
				case Type::DO_WHILE:
				default:
					next = true;
					break;										// Removing this line causes C2059: syntax error: '}'
			}

			return e;
		}
		void Control::addChild(std::shared_ptr<ASTNode>& c) {
			if (expr) {
				if (type == Type::FOR && isNode<List<VarName>>(c))
					vars = std::dynamic_pointer_cast<List<VarName>>(c);
				else
					throw error::operands_error{ "Attempt to construct control node with more than two expresions" };
			} else
				expr = c;
		}
		std::string Control::toString() { return ""; }
		std::string Control::printString(std::string buf) {
			return "";
		}
		bool Control::iterate(EvalState& e, size_t loc) {
			switch (type) {
				case Type::FOR:						// for
				{
					Table t = e.pop<Table>(loc - 2);
					auto kv = t->next(e.pop(loc - 2));

					// continue loop
					if (kv != t->end()) {
						e.settop(loc - 2);											// Remove trash from stack (value of last iteration)

						e.push(t);													// Keep table for next iteration
						e.push(kv->first);											// Push key on stack
						e.push(kv->second.val);										// Push value on stack

					// exit loop
					} else
						return false;												// exit loop

					// Assign variables (same as in eval)
					auto var = vars->rbegin();

					while (var != vars->rend())
						(*var++)->set(e, false, false);								// Assign value then key to trackers

					// Remember last key (if not assigned)
					if (vars->size() == 2)
						(*vars->begin())->eval(e);
				}

					// Generator abstraction (needs work)
					/*
					// Save arguments to generator

					e.push(generator);
					e.call();

					if (???) {
						???
						return false;
					}

					// Assign variables
					*/

					return true;

					break;
				case Type::WHILE:					// while
				{
					bool val = (bool)expr->eval(e);
					if (val) e.settop(loc);
					return val;
				}
				case Type::DO_WHILE:				// do-while
					if (!next) {
						bool val = (bool)expr->eval(e);
						if (val) e.settop(loc);
						return val;
					}
				default:
					next = !next;
					return !next;
			}

			// Settop is handled by Block::eval
		}

		// Block methods
		Block::Block(const ParseData& in) : ASTNode{ in }, save_scope{ false }, table{ false }, excep_if_empty{ true } {}
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

			// Special stack handling of for loops
			x += ((control->type == Control::FOR) * 2);

			while (control->iterate(e, x)) {
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
		TryCatch::TryCatch(const ParseData& in) : ASTNode{ in } {}
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
			auto& code_block = try_code ? catch_code : try_code;
			if (code_block) throw error::operands_error{ "Attempt to add more than two blocks to TryCatch node" };

			code_block.swap(std::dynamic_pointer_cast<Block>(c));
			if (!code_block) throw error::invalid_ast_construction{ "Attempt to construct TryCatch node with a non-Block node" };
		}
		bool TryCatch::isFull() {
			return try_code && catch_code;
		}

		// If methods
		If::If(const ParseData& in) : ASTNode{ in }, statements{} {}
		EvalState& If::eval(EvalState& e) {
			auto b = std::begin(statements);
			auto siz = e.size();

			// Find the selected block
			while (b != std::end(statements)) {
				b->first->eval(e);

				if (e.pop<bool>()) break;

				++b;
			}

			e.settop(siz);

			// Evaluate the selected block (or none if fall through
			if (b != std::end(statements))
				b->second->eval(e);
			else
				e.push(false);

			return e;
		}
		std::string If::toString() { return ""; }
		std::string If::printString(std::string buf) { return ""; }
		void If::addBlock(ExprType& expr, BlockType& block) {
			statements.emplace_back(std::make_pair(expr, block));
		}
		bool If::isFull() {
			return !accepting;
		}
		void If::setFull() {
			accepting = false;
		}

		std::string ASTNode::node_type = "ASTNode";
		std::string Debug::node_type = "Debug";
		std::string Literal::node_type = "Literal";
		std::string Value::node_type = "Value";
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