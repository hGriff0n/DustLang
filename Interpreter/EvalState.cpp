#include "EvalState.h"
#include "Exceptions\dust.h"

namespace dust {

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;

		swap(idx, -1);								// Ensure the value is at the top of the stack
		callMethod(ts.getName(type));				// Call the converter (if execution reaches here, the converter exists)
		swap(idx, -1);								// Restore the stack positions
	}

	impl::Table* EvalState::findScope(const std::string& var, int off, bool not_null) {
		if (!curr_scp) return &global;

		return findScope([&](impl::Table* s) { return s->has(var); }, off, not_null);
	}

	impl::Table* EvalState::findScope(impl::Table* scp, const std::function<bool(impl::Table*)>& pred) {
		while (!pred(scp) && (scp = scp->getPar()));
		return scp;
	}
	
	impl::Table* EvalState::findScope(const std::function<bool(impl::Table*)>& pred, int lvl, bool not_null) {
		impl::Table* scp = curr_scp;

		while (scp && lvl-- && (scp = findScope(scp, pred)))
			if (lvl) scp = scp->getPar();

		return (!scp && not_null) ? &global : scp;			// Wouldn't need this code if the above is taken
	}

	int EvalState::forcedLevel(const std::string& var) {
		return var.find_first_not_of('.');
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

		r = r == type::Traits<Nil>::id ? type::Traits<bool>::id : r;						// Allow Bool functions to be called on nil (temporary measure)
		l = l == type::Traits<Nil>::id ? type::Traits<bool>::id : l;

		auto com_t = ts.com(l, r, fn);						// find common type
		auto dis_t = ts.findDef(com_t, fn);					// find dispatch type (where fn is defined)

		if (dis_t == type::Traits<Nil>::id) throw error::dispatch_error{ "Method " + fn + " is not defined for operands of type " + ts.getName(l) + " and " + ts.getName(r) };

		// Determine whether com selected a converter and perform conversions
		if ((com_t == l ^ com_t == r) && ts.convertible(l, r)) {
			forceType(-1, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary
			forceType(-2, com_t);
		}

		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}
	// Methods
	EvalState& EvalState::callMethod(const std::string& fn) {
		auto curr_t = at().type_id;
		if (curr_t == type::Traits<Nil>::id) curr_t = type::Traits<bool>::id;				// Allow Bool functions to be called on nil (temporary measure)

		auto dis_t = ts.findDef(curr_t, fn);
		if (dis_t == type::Traits<Nil>::id) throw error::dispatch_error{ "Method " + fn + " is not defined for objects of type " + ts.getName(at().type_id) };
		
		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}

	// Handles variable assignment interaction with constant and typing
	void EvalState::setVar(impl::Variable& var, bool is_const, bool is_typed) {
		if (var.val.type_id != type::Traits<Nil>::id) {
			if (var.is_const) throw error::illegal_operation{ "Attempt to reassign a constant variable" };
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
	void EvalState::set(const std::string& name, bool is_const, bool is_typed) {
		int lvl = forcedLevel(name);
		setVar(findScope(name.substr(lvl), lvl, true)->getVar(name), is_const, is_typed);
	}

	// Push the variable onto the stack (nil if it doesn't exist)
	void EvalState::get(const std::string& name) {
		int lvl = forcedLevel(name);						// Strip leveling from the string
		std::string var = name.substr(lvl);

		auto scp = findScope(var, lvl + 1, lvl);
		scp ? push(scp->getVal(var)) : pushNil();
	}

	void EvalState::get() {
		get(pop<std::string>());
	}
	void EvalState::set(bool is_const, bool is_typed) {
		set(pop<std::string>(), is_const, is_typed);
	}
	void EvalState::setGlobal(const std::string& name, bool is_const, bool is_typed) {
		setVar(global.getVar(name.substr(forcedLevel(name))), is_const, is_typed);
	}
	void EvalState::getGlobal(const std::string& name) {
		auto var = name.substr(forcedLevel(name));
		global.has(var) ? push(global.getVal(var)) : pushNil();
	}

	void EvalState::markConst(const std::string& name) {
		
		auto scp = findScope(name, 0);
		if (scp) scp->getVar(name).is_const = !scp->getVar(name).is_const;
			//bool& ic = scp->getVar(name).is_const;
			//ic = !ic;
	}
	void EvalState::markTyped(const std::string& name, size_t typ) {
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
	bool EvalState::isConst(const std::string& name) {
		// if (name[0] = '.') throw "isConst cannot not be called on non-local variables";
		auto scp = findScope(name, 0);
		return scp && scp->getVar(name).is_const;
	}
	bool EvalState::isTyped(const std::string& name) {
		auto scp = findScope(name, 0);
		return scp && scp->getVar(name).type_id != type::Traits<Nil>::id;
	}

	void EvalState::newScope() {
		curr_scp = curr_scp ? new impl::Table{ curr_scp } : &global;
	}

	void EvalState::endScope() {
		auto* sav = curr_scp->getPar();
		if (sav) delete curr_scp;					// If the current scope wasn't the global scope
		curr_scp = sav;
	}

	void EvalState::pushScope() {
		// store scope
		// push scope id onto stack
		//curr_scp = curr_scp->getPar();
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
		auto Object = ts.getType("Object");
		auto Int = ts.getType("Int");
		auto Float = ts.getType("Float");
		auto String = ts.getType("String");
		auto Bool = ts.getType("Bool");

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

		Int.addOp("_op+", [](EvalState& e) { e.push((int)e + (int)e); return 1; });
		Int.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });		// Could be moved to Number
		Int.addOp("_op-", [](EvalState& e) { e.push((int)e - (int)e); return 1; });
		Int.addOp("_op*", [](EvalState& e) { e.push((int)e * (int)e); return 1; });
		Int.addOp("_op^", [](EvalState& e) {
			auto base = (double)e;
			e.push(pow(base, (double)e));
			return 1;
		});											// Could be moved to Number
		Int.addOp("_op%", [](EvalState& e) { e.push((int)e % (int)e); return 1; });
		Int.addOp("_op<", [](EvalState& e) { e.push((int)e < (int)e); return 1; });		// 
		Int.addOp("_op=", [](EvalState& e) { e.push((int)e == (int)e); return 1; });
		Int.addOp("_op>", [](EvalState& e) { e.push((int)e >(int)e); return 1; });
		Int.addOp("_ou-", [](EvalState& e) { e.push(-(int)e); return 1; });


		String.addOp("_op+", [](EvalState& e) { e.push((std::string)e + e.pop<std::string>(-2)); return 1; });			// Why is Int._op- correct then???
		String.addOp("_op=", [](EvalState& e) { e.push(e.pop().val.i == e.pop().val.i); return 1; });


		Float.addOp("_op+", [](EvalState& e) { e.push((double)e + (double)e); return 1; });
		Float.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });
		Float.addOp("_op-", [](EvalState& e) { e.push((double)e - (double)e); return 1; });
		Float.addOp("_op*", [](EvalState& e) { e.push((double)e * (double)e); return 1; });
		Float.addOp("_op^", [](EvalState& e) {
			auto base = (double)e;
			e.push(pow(base, (double)e));
			return 1;
		});
		Float.addOp("_op<", [](EvalState& e) { e.push((double)e < (double)e); return 1; });		// 
		Float.addOp("_op=", [](EvalState& e) { e.push((double)e == (double)e); return 1; });
		Float.addOp("_op>", [](EvalState& e) { e.push((double)e >(double)e); return 1; });
		Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });
		Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });

		Bool.addOp("_op=", [](EvalState& e) { e.push((bool)e == (bool)e); return 1; });
		Bool.addOp("_ou!", [](EvalState& e) { e.push(!(bool)e);  return 1; });
	}
}