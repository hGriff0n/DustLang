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

		std::string trim(std::string s) {
			// Include <cctype>
			//auto front = std::find_if_not(s.begin(), s.end(), std::isspace);
			//auto back = std::find_if_not(s.rbegin(), s.rend(), strd::isspace).base();
			//return back <= front ? "" : std::string{front, back};
			return s;
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
			if (id == type::Traits<std::string>::id) return "String \"" + val + "\"";

			return (id == type::Traits<int>::id ? "Int " :
				id == type::Traits<double>::id ? "Float " :
				id == type::Traits<bool>::id ? "Bool " : "Nil ") + val;
		}

		// Operator methods
		Operator::Operator(const ParseData& in, std::string o) : ASTNode{ in }, l{ nullptr }, r{ nullptr }, op{ o } {}
		EvalState& Operator::eval(EvalState& e) {
			l->eval(e);

			if (r) {
				r->eval(e);
				e.swap();						// Operators expect stack = ..., {rhs}, {lhs}
			}

			e.callOp(op);
			return e;
		}
		std::string Operator::toString() { return op; }
		std::string Operator::printString(std::string buf) {
			return buf + "+- " + node_type + " " + op + "\n" + l->printString(buf + " ") + (r ? r->printString(buf + " ") : "");
		}
		void Operator::addChild(std::shared_ptr<ASTNode>& c) {
			if (!l) l = c;
			else if (!r) r = c;
			else throw error::unimplemented_operation{ "Dust does not currently support ternary operators using the Operator node" };
		}

		// VarName methods
		VarName::VarName(const ParseData& in, std::string var) : ASTNode{ in } {
			fields.emplace_back(makeNode<Literal>(in, var, type::Traits<std::string>::id));
		}
		VarName::VarName(std::shared_ptr<ASTNode>&& var) : ASTNode{ var->p } {
			if (!isNode<VarName>(var)) get_first = false;

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
		void VarName::setLevel(const std::string& dots) {
			lvl = dots.size();
		}
		EvalState& VarName::eval(EvalState& e) {
			// Get the variable on to the stack
			auto field = std::begin(fields);
			(*field)->eval(e);

			if (!sub_var) {
				if (get_first) e.get(EvalState::SCOPE, lvl);

				while (++field != std::end(fields))
					(*field)->eval(e).get(-2);
			}

			return e;
		}
		EvalState& VarName::set(EvalState& e, bool is_const, bool is_static) {
			auto field = std::begin(fields), end = std::end(fields) - 1;
			(*field)->eval(e);
			
			// No indexing
			if (field == end) {
				e.swap();
				e.set(EvalState::SCOPE, true, lvl);

			// Handle indexing
			} else {
				bool instance = fields.size() != 2;
				if (!isNode<TypeName>(*field)) {
					e.get(EvalState::SCOPE, lvl);
					instance = true;
				}

				while (++field != end)
					(*field)->eval(e).get(-2);
				//if (e.is<Nil>()) throw error::dust_error{ "Attempt to assign to a Nil value" };

				e.swap();
				(*end)->eval(e).swap();
				e.set(-3, instance);
			}

			return e;
		}

		// TypeName methods
		TypeName::TypeName(const ParseData& in, std::string n) : ASTNode{ in }, name{ n } {}
		EvalState& TypeName::eval(EvalState& e) {
			auto& type = e.getTS().get(name);

			// If the type hasn't been reference-trapped yet (so never pushed on the stack)
			if (type.ref.type_id == type::Traits<Nil>::id) {
				e.push(type.fields);
				e.assignRef(const_cast<type::Type&>(type));

			} else {
				e.push(type.ref);
			}

			return e;
		}
		std::string TypeName::toString() { return name; }
		std::string TypeName::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n";
		}

		// TypeCast methods
		TypeCast::TypeCast(const ParseData& in) : ASTNode{ in } {}
		EvalState& TypeCast::eval(EvalState& e) {
			expr->eval(e);

			// Get the converter on the stack
			auto key = type::Traits<std::string>::make(convert->toString(), e.getGC());
			e.push(e.getTS().get(e.at().type_id).fields->getVal(key));
			e.call(1);

			return e;
		}
		void TypeCast::addChild(std::shared_ptr<ASTNode>& c) {
			if (!convert && std::dynamic_pointer_cast<TypeName>(c))
				convert = std::dynamic_pointer_cast<TypeName>(c);

			else if (!expr)
				expr = c;

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

			// Associate the type with its provided method list and complete type definition
			definition->eval(e);
			e.completeDef(nType);

			return e;
		}
		std::string NewType::toString() { return name + " extends " + inherit; }
		void NewType::addChild(std::shared_ptr<ASTNode>& c) {
			if (std::dynamic_pointer_cast<TypeName>(c))
				(name == "" ? name : inherit) = c->toString();

			else if (std::dynamic_pointer_cast<Block>(c))
				definition = std::dynamic_pointer_cast<Block>(c);

			else
				throw error::invalid_ast_construction{ "Attempt to construct NewType node without a definition (Block) node" };
		}

		// TypeCheck methods
		TypeCheck::TypeCheck(const ParseData& in) : ASTNode{ in } {}
		EvalState& TypeCheck::eval(EvalState& e) {
			auto x = e.size();
			auto res = l->eval(e).pop();						// Possible issue with multiple returns (might pick up the last return, hopefully picks up the first)

			// In-case l is a multiple return function, clean the stack
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
				l = c;

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

			// TODO: Add in splat semantics

			auto l_var = vars->begin(), r_var = vars->end();
			auto l_val = vals->begin(), r_val = vals->end();
			size_t old = e.size(), exp = e.size() + vars->size();

			//auto n = num<Splat>(vars);		// Still needs to handle splat assignment
			//if (n > 1) throw error::
			//if (n == 1) exp = -1;				// Cause the evaluate loop to evaluate all expressions

			// Evaluate expression list (left -> right)
			while (l_val != r_val && e.size() < exp)
				(*l_val++)->eval(e);

			// More variables than values. Push nil
				// Might change to copy() depending on compound assignment semantics (extend the last value to match)
			while (e.size() < exp) e.pushNil();

			// More values than variables (due to multiple returns). Pop extras
			while (e.size() > exp) e.pop();

			// Reverse the stack to enable left->right evaluation of assignments
				// Doesn't handle multiple returns
				// Do I really need this (The actual assignments can be right->left)
			e.reverse(vars->size());

			// Perform assignments. Compound if necessary
			while (r_var != l_var) {
				if (op.size()) (*l_var)->eval(e).callOp(op);

				(*l_var++)->set(e, set_const, set_static);
			}

			// Can't memoize the last value due to splat/nil semantics
			return (*vars->rbegin())->eval(e);				//return last_var->eval(e);
		}
		std::string Assign::toString() { return op; }
		std::string Assign::printString(std::string buf) {
			return buf + "+- " + node_type + " " + toString() + "\n" + vars->printString(buf + " ") + vals->printString(buf + " ");
		}
		void Assign::addChild(std::shared_ptr<ASTNode>& c) {
			if (!vars && std::dynamic_pointer_cast<var_type>(c))
				vars = std::dynamic_pointer_cast<var_type>(c);

			else if (!vals && std::dynamic_pointer_cast<val_type>(c))
				vals = std::dynamic_pointer_cast<val_type>(c);

			else
				throw error::operands_error{ "Assignment is a binary operation" };
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
			if (!l) l = c;
			else if (!r) r = c;
			else throw error::operands_error{ "Attempt to add more than three operands to BinaryKeyword node" };
		}

		// Control methods
		Control::Control(const ParseData& in, Type typ) : ASTNode{ in }, type{ typ }, next{ true } {}
		EvalState& Control::eval(EvalState& e) {
			switch (type) {
				case TRY_CATCH:
					std::dynamic_pointer_cast<VarName>(expr)->set(e, true, false);
					break;

				case FOR:
					if (!expr->eval(e).is<Table>())
						throw error::bad_node_eval{ "Attempt to iterate over a non-table" };

					// Generator Abstraction (needs work)

					/*
					// Get generator function into local state
					if (!has_generator) {

						// Try for an implied generator
						if (!e.is<Function>()) {
							e.copy();
							e.push("_iterator");
							e.get();

							// Should this be a dust_error ???
							if (e.is<Nil>())
								throw error::bad_node_eval{ "Attempt to iterate over a non-iterable object" };
						}

						generator = e.pop();
						has_generator = true;
					}
					*/

				case WHILE:
					e.pushNil();								// Initial loop value

				case DO_WHILE:
				default:
					next = true;
					break;										// Removing this line causes C2059: syntax error: '}'
			}

			return e;
		}
		void Control::addChild(std::shared_ptr<ASTNode>& c) {
			if (expr) {
				if (type == FOR && isNode<List<VarName>>(c))
					vars = std::dynamic_pointer_cast<List<VarName>>(c);
				else
					throw error::operands_error{ "Attempt to construct control node with more than two expresions" };
			} else
				expr = c;
		}
		std::string Control::toString() {
			return (type == DO_WHILE ? "repeat" :
				type == FOR ? "for" :
				type == TRY_CATCH ? "try-catch" :
				type == WHILE ? "while" : "");
		}
		std::string Control::printString(std::string buf) {
			std::string ret = buf + "+- " + node_type + " " + toString();

			if (type == FOR) {
				ret += " " + vars->printString("");
			}

			return ret + "\n" + expr->printString(buf + " ");
		}
		bool Control::iterate(EvalState& e, size_t loc) {
			switch (type) {
				case FOR:
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


					// Generator abstraction (needs work)
					/*
					e.copy(loc - 2);
					e.copy(loc - 1);
					e.push(generator);
					e.call();

					if (e.is<Nil>()) {
						e.pop(loc - 2);
						e.pop(loc - 2);
						e.pop();

						return false;
					}
					*/

					// Alternate implementation (need to modify to work on non-table state)
					/*
					Table t = e.pop<Table>(loc - 2);
					auto k = e.pop(loc - 2);

					auto siz = e.size();
					generator(e, t, k);

					if (e.is<Nil>()) {
						e.pop();
						return false;
					}

					// Assign variables

					// Remember last state
					if (vars->size() == 2) {
						e.settop(loc - 2);
						e.push(t);
						(*vars->begin())->eval(e);
					} else {
						auto key = e.pop();
						e.settop(loc - 2);
						e.push(t);
						e.push(key);
					}
					*/

					// Assign variables
					auto var = vars->rbegin();

					while (var != vars->rend())
						(*var++)->set(e, false, false);								// Assign value then key to trackers

					// Remember last key (if not assigned)
					if (vars->size() == 2)
						(*vars->begin())->eval(e);

					e.settop(loc);
					return true;
				}
				break;
				case WHILE:
				{
					bool val = (bool)expr->eval(e);
					if (val) e.settop(loc);
					return val;
				}
				case DO_WHILE:
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
			// Perform evaluation setup
			if (expr.empty() && !table) {
				if (excep_if_empty)
					throw error::bad_node_eval{ "Attempt to evaluate an empty block" };

				e.pushNil();
				return e;
			}

			size_t x = e.size(), next = 1;

			e.newScope();
			control->eval(e);								// Setup the control state

			// Special stack handling
			if (control->type == Control::FUNCTION) x = 0;
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
						e.set(EvalState::SCOPE);			// WARNING! Might give "wrong" answers
					}
				}
			}

			if (save_scope) {
				e.settop(x);
				e.push(next);
				e.pushScope();

			} else
				e.endScope();

			return e;
		}
		std::string Block::toString() {
			return " " + (table ? "[]" : control->toString());
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
				control = std::dynamic_pointer_cast<Control>(c);

				if (control->type != Control::NONE) excep_if_empty = false;

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
			if (!std::dynamic_pointer_cast<Block>(c)) throw error::invalid_ast_construction{ "Attempt to construct TryCatch node with a non-Block node" };

			code_block = std::dynamic_pointer_cast<Block>(c);
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
				e.pushNil();

			return e;
		}
		std::string If::toString() { return ""; }
		std::string If::printString(std::string buf) {
			std::string stub = buf + "+- ";
			buf += " ";

			auto b = std::begin(statements);
			std::string ret = stub + "If\n" + b->first->printString(buf) + b->second->printString(buf);

			while (++b != std::end(statements))
				ret += stub + "Else-If\n" + b->first->printString(buf) + b->second->printString(buf);

			return ret;

		}
		void If::addBlock(ExprType& expr, BlockType& block) {
			statements.emplace_back(std::make_pair(expr, block));
		}
		bool If::isFull() {
			return !accepting;
		}
		void If::setFull() {
			accepting = false;
		}

		// FunctionCall methods
		FunctionCall::FunctionCall(const ParseData& in) : ASTNode{ in } {}
		EvalState& FunctionCall::eval(EvalState& e) {
			// Get arguments on the stack
			auto top = e.size();
			for (auto arg : *args) arg->eval(e);

			// Determine the number of arguments
			auto num_args = e.size() - top;

			// Rotate so that the first (left) argument is on the top
			e.reverse(num_args);

			// Get the function onto the stack
			e.setResolvingFunctionName();
			fn->eval(e);
			e.setResolvingFunctionName();

			// Call the function
			e.call(num_args);
			return e;
		}
		void FunctionCall::addChild(std::shared_ptr<ASTNode>& c) {
			if (isNode<VarName>(c)) {
				if (!fn)
					fn = std::dynamic_pointer_cast<VarName>(c);
				else
					throw error::base{ "Attempt to assign multiple functions" };

			} else if (isNode<List<ASTNode>>(c)) {
				if (!args)
					args = std::dynamic_pointer_cast<List<ASTNode>>(c);
				else
					throw error::base{ "Attempt to assign multiple argument lists" };

			} else
				throw error::base{ "Attempt to assign a unaccepted node type" };
		}
		std::string FunctionCall::toString() { return fn->toString(); }
		std::string FunctionCall::printString(std::string buf) {
			return buf + "+- " + node_type + "\n" + fn->printString(buf + " ") + buf + " +- Arguments:\n" + args->printString(buf + "  ");
		}

		// Argument methods
		Argument::Argument(std::shared_ptr<VarName>& v, bool is_self) : ASTNode{ v->p }, var{ v }, self { is_self } {}
		EvalState& Argument::eval(EvalState& e) {
			if (!self)							// self gets handled by client code
				var->set(e, false, false);

			return e;
		}
		std::string Argument::toString() {
			return var->toString();
		}
		std::string Argument::printString(std::string buf) {
			return var->printString(buf);
		}

		// FunctionDef methods
			// However I implement dust functions, the entry point must take care to completely allocate the arguments (and clean the stack to empty) before execution
		FunctionDef::FunctionDef(const ParseData& in) : Control{ in, Control::FUNCTION } {}
		EvalState& FunctionDef::eval(EvalState& e) {
			if (args->size() > 0) {
				auto arg = args->begin();

				if ((*arg)->self)
					e.enableObjectSyntax();

				while (arg != args->end()) {
					if (e.empty()) e.pushNil();
					(*arg++)->eval(e);
				}
			}

			while (!e.empty()) e.pop();
			// and other necessary stuff

			return e;
		}
		void FunctionDef::addChild(std::shared_ptr<ASTNode>& c) {
			if (isNode<List<Argument>>(c))
				args = std::dynamic_pointer_cast<List<Argument>>(c);
			else
				throw error::missing_node_x{ "FunctionDef", "List<Argument>" };
		}
		std::string FunctionDef::toString() {
			return "Function";
		}
		std::string FunctionDef::printString(std::string buf) {
			return "";
		}
		bool FunctionDef::iterate(EvalState& e, size_t loc) {
			// might want to add some special stuff for recursion ???

			return Control::iterate(e, loc);
		}

		// FunctionLiteral methods
		FunctionLiteral::FunctionLiteral(std::shared_ptr<ASTNode>& node) : ASTNode{ node->p }, fn{ node } {}
		EvalState& FunctionLiteral::eval(EvalState& e) {
			e.push(fn);
			return e;
		}
		std::string FunctionLiteral::toString() {
			return "";
		}
		std::string FunctionLiteral::printString(std::string buf) {
			return fn->printString(buf);
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
		std::string If::node_type = "If";
		std::string FunctionCall::node_type = "Function Call";
		std::string Argument::node_type = "Argument";
		std::string FunctionLiteral::node_type = "Function Literal";
		std::string FunctionDef::node_type = "Function Definition";
	}
}