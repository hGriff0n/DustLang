#include "EvalState.h"
#include "Exceptions\dust.h"

#include <unordered_set>

namespace dust {

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;
		swap(idx, -1);								// Ensure the value is at the top of the stack

		if (type == type::Traits<Table>::id) {		// Special handling for tables
			newScope();
			push(1);
			setScoped();
			pushScope(2);
		} else {
			callMethod(ts.getName(type));			// Otherwise call the converter (if execution reaches here, the converter exists)
		}

		swap(idx, -1);								// Restore the stack positions
	}

	Table EvalState::findScope(const impl::Value& var, int lvl, bool not_null) {
		Table scp = curr_scp;

		while (scp && lvl-- && (scp = scp->findDef(var)))
			if (lvl) scp = scp->getPar();

		return (!scp && not_null) ? &global : scp;
	}

	// Constructor
	EvalState::EvalState() : ts{}, gc{}, CallStack{ gc }, global{}, curr_scp{ nullptr } {}

	// Free functions
	EvalState& EvalState::call(const std::string& fn) {
		if (fn == "type")
			push((int)(pop().type_id));
		else if (fn == "typename")
			push(ts.getName(pop().type_id));

		return *this;
	}
	// Operators
	EvalState& EvalState::callOp(const std::string& fn) {
		if (fn.at(0) == '_' && fn.at(2) == 'u') return callMethod(fn);
		if (fn.at(0) != '_' || fn.at(2) != 'p') throw error::bad_api_call{ "Attempt to callOp on a non-operator" };

		auto r = at(-2).type_id, l = at().type_id;
		auto com_t = ts.com(l, r, fn);						// find common type
		auto dis_t = ts.findDef(com_t, fn);					// find dispatch type (where fn is defined)

		if (dis_t == ts.NO_DEF) throw error::dispatch_error{ "Method " + fn + " is not defined for operands of type " + ts.getName(l) + " and " + ts.getName(r) };
		
		// Determine whether com selected a converter and perform conversions
		if ((com_t == l ^ com_t == r) && (com_t == type::Traits<Table>::id || ts.convertible(l, r))) {
			forceType(-1, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary
			forceType(-2, com_t);
		}

		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}
	// Methods
	EvalState& EvalState::callMethod(const std::string& fn) {
		auto dis_t = ts.findDef(at().type_id, fn);
		if (dis_t == ts.NO_DEF) throw error::dispatch_error{ "Method " + fn + " is not defined for objects of type " + ts.getName(at().type_id) };
		
		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}

	// Handles variable assignment interaction with constant and typing
	void EvalState::setVar(impl::Variable& var, bool is_const, bool is_typed) {
		if (var.is_const) throw error::illegal_operation{ "Attempt to reassign a constant variable" };

		// if the variable held a value (static typing is only possible here)
		if (var.val.type_id != type::Traits<Nil>::id) {

			// if the var is statically typed and the new value is not a child type
			if (var.type_id != type::Traits<Nil>::id && !ts.isChildType(at().type_id, var.type_id)) {
				if (!ts.convertible(var.type_id, at().type_id))	throw error::converter_not_found{ "No converter from from the assigned value to the variable's static type" };
				callMethod(ts.getName(var.type_id));
			}

			try_decRef(var.val);
		}

		// Set the variable data
		var.val = pop();															// `pop` (no templates) doesn't decrement references
		var.is_const = is_const;
		if (is_typed) var.type_id = var.val.type_id;
	}
	
	// Assign the top value on the stack to the given variable with the given flags
	void EvalState::setScoped(const impl::Value& name, int _lvl, bool is_const, bool is_typed) {
		push(findScope(name, _lvl, true));
		set(name);
	}

	// Push the variable onto the stack (nil if it doesn't exist)
	void EvalState::getScoped(const impl::Value& name, int _lvl) {
		push(findScope(name, _lvl + 1, true));
		get(name);
	}

	void EvalState::getScoped(int lvl) {
		getScoped(pop(), lvl);
	}

	void EvalState::setScoped(int lvl, bool is_const, bool is_typed) {
		setScoped(pop(), lvl, is_const, is_typed);
	}

	// Workspace
	void EvalState::get() {
		get(pop());
	}

	void EvalState::get(const impl::Value& var) {
		if (is<Nil>()) return;
		if (!is<Table>()) return pop(), pushNil();

		Table t = pop<Table>();
		t->has(var) ? push(t->getVal(var)) : pushNil();
	}

	void EvalState::set() {
		set(pop());
	}

	void EvalState::set(const impl::Value& var) {
		if (!is<Table>()) throw error::dust_error{ "Attempt to initialize a non-stored variable" };
		
		setVar(pop<Table>()->getVar(var), false, false);
	}

	void EvalState::markConst(const impl::Value& name) {
		auto scp = findScope(name, 0);

		if (scp) scp->getVar(name).is_const = !scp->getVar(name).is_const;
		//bool& ic = scp->getVar(name).is_const;
		//ic = !ic;
	}
	void EvalState::markTyped(const impl::Value& name, size_t typ) {
		auto scp = findScope(name, 0);
		if (!scp) return;		// throw error::base{ "Attempt to static type a nil value" };
		
		auto& var = scp->getVar(name);

		// If the typing change may require type conversion (ie. typ not Nil and !(type(var) <= typ))
		if (typ != type::Traits<Nil>::id && !ts.isChildType(var.type_id, typ)) {
			if (!ts.convertible(var.type_id, typ)) throw error::converter_not_found{ "No converter from the current value to the given type" };

			push(var.val);
			callMethod(ts.getName(typ));
			try_decRef(var.val);
			var.val = pop();										// `pop` (no templates) doesn't decrement references
		}

		var.val.type_id = typ;
	}
	bool EvalState::isConst(const impl::Value& name) {
		auto scp = findScope(name, 0);

		return scp && scp->getVar(name).is_const;
	}
	bool EvalState::isTyped(const impl::Value& name) {
		auto scp = findScope(name, 0);

		return scp && scp->getVar(name).type_id != type::Traits<Nil>::id;
	}

	void EvalState::newScope() {
		curr_scp = curr_scp ? new table_type{ curr_scp } : &global;
	}

	void EvalState::endScope() {
		auto* sav = curr_scp->getPar();
		if (sav) delete curr_scp;					// If the current scope wasn't the global scope
		curr_scp = sav;
	}

	void EvalState::pushScope(int nxt) {
		Table sav = curr_scp;
		curr_scp = sav->getPar();

		sav->setNext(nxt);
		push(sav);
	}

	type::TypeSystem& EvalState::getTS() {
		return ts;
	}


	void initState(EvalState& e) {
		initTypeSystem(e.ts);
		initConversions(e.ts);
		initOperations(e.ts);
	}

	void initTypeSystem(type::TypeSystem& ts) {
		auto Object = ts.getType("Object");
		auto Number = ts.newType("Number");
		auto Int = ts.newType("Int", Number);
		auto Float = ts.newType("Float", Number);
		auto String = ts.newType("String");
		auto Bool = ts.newType("Bool");
		auto Table = ts.newType("Table");
		auto Function = ts.newType("Function");

		// Initialize type::Traits id's
		type::Traits<int>::id = Int;
		type::Traits<double>::id = Float;
		type::Traits<std::string>::id = String;
		type::Traits<bool>::id = Bool;
		type::Traits<dust::Table>::id = Table;
	}

	void initConversions(type::TypeSystem& ts) {
		auto Int = ts.getType("Int");
		auto Float = ts.getType("Float");
		auto String = ts.getType("String");

		// Initialize Conversions	
		Int.addOp("String", [](EvalState& e) { e.push((std::string)e); return 1; });
		Int.addOp("Float", [](EvalState& e) { e.push((double)e); return 1; });

		Float.addOp("String", [](EvalState& e) { e.push((std::string)e); return 1; });
		Float.addOp("Int", [](EvalState& e) { e.push((int)e); return 1; });

		String.addOp("Int", [](EvalState& e) { e.push((int)e); return 1; });
		String.addOp("Float", [](EvalState& e) { e.push((float)e); return 1; });

	}

	void initOperations(type::TypeSystem& ts) {
		auto Nil = ts.getType("Nil");
		auto Object = ts.getType("Object");
		auto Number = ts.getType("Number");
		auto Int = ts.getType("Int");
		auto Float = ts.getType("Float");
		auto String = ts.getType("String");
		auto Bool = ts.getType("Bool");
		auto Table = ts.getType("Table");

		//! -
		//^ * / + - % < = > <= != >=

		Object.addOp("_op<=", [](EvalState& e) {
			e.copy(-2);
			e.copy(-2);
			e.callOp("_op<");

			if (!(bool)e)
				e.callOp("_op=");
			else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});
		Object.addOp("_op>=", [](EvalState& e) {
			e.copy(-2);
			e.copy(-2);
			e.callOp("_op>");

			if (!(bool)e)
				e.callOp("_op=");
			else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});
		Object.addOp("_op!=", [](EvalState& e) {
			e.callOp("_op=");
			e.callOp("_ou!");
			return 1;
		});
		/*
		Object.addOp("_op=", [](EvalState& e) {
			e.push((bool)e == (bool)e); return 1;
		});
		Object.addOp("_ou!", [](EvalState& e) {
			e.push(!(bool)e); return 1;
		});
		*/

		Number.addOp("_op^", [](EvalState& e) {
			auto base = (double)e;
			e.push(pow(base, (double)e));
			return 1;
		});
		Number.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });

		Int.addOp("_op+", [](EvalState& e) { e.push((int)e + (int)e); return 1; });
		Int.addOp("_op-", [](EvalState& e) { e.push((int)e - (int)e); return 1; });
		Int.addOp("_op*", [](EvalState& e) { e.push((int)e * (int)e); return 1; });
		Int.addOp("_op%", [](EvalState& e) { e.push((int)e % (int)e); return 1; });
		Int.addOp("_op<", [](EvalState& e) { e.push((int)e < (int)e); return 1; });		// 
		Int.addOp("_op=", [](EvalState& e) { e.push((int)e == (int)e); return 1; });
		Int.addOp("_op>", [](EvalState& e) { e.push((int)e >(int)e); return 1; });
		Int.addOp("_ou-", [](EvalState& e) { e.push(-(int)e); return 1; });


		String.addOp("_op+", [](EvalState& e) { e.push((std::string)e + e.pop<std::string>(-2)); return 1; });			// Why is Int._op- correct then???
		String.addOp("_op=", [](EvalState& e) { e.push(e.pop().val.i == e.pop().val.i); return 1; });


		Float.addOp("_op+", [](EvalState& e) { e.push((double)e + (double)e); return 1; });
		Float.addOp("_op-", [](EvalState& e) { e.push((double)e - (double)e); return 1; });
		Float.addOp("_op*", [](EvalState& e) { e.push((double)e * (double)e); return 1; });
		Float.addOp("_op<", [](EvalState& e) { e.push((double)e < (double)e); return 1; });		// 
		Float.addOp("_op=", [](EvalState& e) { e.push((double)e == (double)e); return 1; });
		Float.addOp("_op>", [](EvalState& e) { e.push((double)e >(double)e); return 1; });
		Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });

		Bool.addOp("_op=", [](EvalState& e) { e.push((bool)e == (bool)e); return 1; });
		Bool.addOp("_ou!", [](EvalState& e) { e.push(!(bool)e);  return 1; });

		// Table functions
			// Currently these functions "erase" non-integer key values
				// This means that they migrate the values existence into the new table
				// However the value is assigned based on the current open integer key

		// Append element(s) to table
		Table.addOp("_op+", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();
			e.newScope();
			int nxt = 1;

			// Rework once set has a nice behavior
			for (auto l_elem : *lt) {
				e.push(l_elem.second.val);
				e.push(nxt++);
				e.setScoped();
			}

			for (auto r_elem : *rt) {
				e.push(r_elem.second.val);
				e.push(nxt++);
				e.setScoped();
			}

			e.pushScope(nxt);
			return 1;
		});
		// Remove element(s) from table
		Table.addOp("_op-", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();
			int nxt = 1;

			for (auto l_elem : *lt)
				if (!rt->contains(l_elem.second.val)) {
					e.push(l_elem.second.val);
					e.push(nxt++);
					e.setScoped();
				}

			e.pushScope(nxt);
			return 1;
		});
		// Union (Append and remove duplicates)
		Table.addOp("_op*", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			std::unordered_set<impl::Value> nt{};

			for (auto l_elem : *lt)
				nt.insert(l_elem.second.val);

			for (auto r_elem : *rt)
				nt.insert(r_elem.second.val);

			e.newScope();
			int nxt = 1;

			for (auto elem : nt) {
				e.push(elem);
				e.push(nxt++);
				e.setScoped();
			}

			e.pushScope(nxt);
			return 0;
		});
		// Intersection (Elements in both tables)
		Table.addOp("_op^", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();
			int nxt = 1;

			for (auto l_elem : *lt)
				if (rt->contains(l_elem.second.val)) {
					e.push(l_elem.second.val);
					e.push(nxt++);
					e.setScoped();
				}

			e.pushScope(nxt);
			return 1;
		});
		// Comparison
		Table.addOp("_op=", [](EvalState& e) {
			if (e.at().val.i == e.at(-2).val.i) {
				e.pop();
				e.pop();
				e.push(true);

			} else {
				dust::Table rt = e.pop<dust::Table>(), lt = e.pop<dust::Table>();

				if (lt->size() == rt->size()) {
					auto lt_iter = lt->begin();

					for (auto rt_elem : *rt) {
						e.push(lt_iter->second.val);
						e.push(rt_elem.second.val);
						e.callOp("_op=");

						if (!e.at().val.i) return 1;

						e.pop();
						++lt_iter;
					}

					e.push(true);
				} else
					e.push(false);
			}

			return 1;
		});
	}
}