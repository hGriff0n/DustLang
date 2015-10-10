#include "EvalState.h"
#include "Exceptions\dust.h"

namespace dust {

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;

		swap(idx, -1);
		callMethod(ts.getName(type));				// Call the converter (if execution reaches here, the converter exists)
		swap(idx, -1);
	}

	// Convert the element to var.type_id if possible because var is statically typed
		// Is only called if var.type_id != ts.Nil and at(idx).type_id is not a child of var.type_id
	void EvalState::staticTyping(impl::Variable& var, bool is_const) {
		if (!ts.convertible(var.type_id, at().type_id))	throw error::converter_not_found{ "No converter from from the assigned value to the variable's static type" };

		try_decRef(var.val);
		callMethod(ts.getName(var.type_id));
		try_incRef(var.val = pop());
		var.is_const = is_const;
	}

	// Create and set a new Variable
	void EvalState::newVar(const std::string& name, bool is_const, bool is_typed) {
		//auto& var = global.getVar(name);
		auto& var = curr_scp->getVar(name);
		var = impl::Variable{ pop(), ts.NIL, is_const };
		if (is_typed) var.type_id = var.val.type_id;
	}

	// Constructor
	EvalState::EvalState() : ts{}, gc{}, CallStack{ gc }, global{}, curr_scp{ nullptr } {
		curr_scp = nullptr;
	}

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

		if (dis_t == ts.NIL) throw error::dispatch_error{ "Method " + fn + " is not defined for objects of type " + ts.getName(l) + " and " + ts.getName(r) };

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
		auto dis_t = ts.findDef(at().type_id, fn);
		if (dis_t == ts.NIL) throw error::dispatch_error{ "Method " + fn + " is not defined for objects of type " + ts.getName(at().type_id) };
		
		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}

	// Assign the top value on the stack to the given variable with the given flags
	void EvalState::setVar(const std::string& name, bool is_const, bool is_typed) {
		//if (!global.has(name)) return newVar(name, is_const, is_typed);
		//auto& var = global.getVar(name);
		if (!curr_scp->has(name)) return newVar(name, is_const, is_typed);
		auto& var = curr_scp->getVar(name);

		if (var.is_const) throw error::illegal_operation{ "Attempt to reassign a constant variable" };

		if (var.type_id != ts.NIL && !ts.isChildType(at().type_id, var.type_id))
			return staticTyping(var, is_const);

		try_decRef(var.val);
		var.val = pop();
		try_incRef(var.val);

		var.is_const = is_const;
		if (is_typed) var.type_id = var.val.type_id;
	}

	// Push the variable onto the stack (0 if it doesn't exist)
	void EvalState::getVar(const std::string& name) {
		//if (!global.has(name)) return push(0);
		//push(global.getVal(name));
		if (!curr_scp->has(name)) return push(0);
		push(curr_scp->getVal(name));
	}

	void EvalState::markConst(const std::string& name) {
		//if (global.has(name))
			//global.getVar(name).is_const = !global.getVar(name).is_const;
		if (curr_scp->has(name))
			curr_scp->getVar(name).is_const = !curr_scp->getVar(name).is_const;
	}
	void EvalState::markTyped(const std::string& name, size_t typ) {
		//if (!global.has(name)) return;
		//auto& var = global.getVar(name);
		
		if (!curr_scp->has(name)) return;
		auto& var = curr_scp->getVar(name);

		if (typ != ts.NIL) {
			//if (!ts.convertible(global.getVar(name).type_id, typ))
			if (!ts.convertible(curr_scp->getVar(name).type_id, typ))
				throw error::converter_not_found{ "No converter from the current value to the given type" };

			push(var.val);
			callMethod(ts.getName(typ));
			try_decRef(var.val);
			var.val = pop();
		}

		var.val.type_id = typ;
	}
	bool EvalState::isConst(const std::string& name) {
		//return global.getVar(name).is_const;
		return curr_scp->getVar(name).is_const;
	}
	bool EvalState::isTyped(const std::string& name) {
		//return global.getVar(name).type_id != ts.NIL;
		return curr_scp->getVar(name).type_id != ts.NIL;
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
		String.addOp("_op=", [](EvalState& e) { e.push(e.pop_ref(true) == e.pop_ref(true)); return 1; });


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