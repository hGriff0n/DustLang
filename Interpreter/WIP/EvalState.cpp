#include "EvalState.h"
#include "Init.h"

#include <iostream>

namespace dust {

	void initState(EvalState& e) {
		initTypeSystem(e.ts);
		initConversions(e.ts);
		initOperations(e.ts);
	}

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;
		
		swap(idx, -1);
		callMethod(ts.getName(type));				// Call the converter (if execution reaches here, the converter exists)
		swap(idx, -1);
	}


	// Convert the element to var.type_id if possible because var is statically typed
		// Is only called if var.type_id != ts.Nil and at(idx).type_id is not a child of var.type_id
	void EvalState::staticTyping(impl::Variable& var, bool isConst) {
		if (!ts.convertible(var.type_id, at().type_id)) throw std::string{ "No converter from the assigned value to the variable's static type" };

		callMethod(ts.getName(var.type_id));
		var.val = pop();
		var.is_const = isConst;
	}

	// Create and set a new Variable
	void EvalState::newVar(std::string name, bool isConst, bool isTyped) {
		auto& var = vars[name] = impl::Variable{ pop(), ts.NIL, isConst };

		if (isTyped) var.type_id = var.val.type_id;
	}

	// Assign the top value on the stack to the given variable with the given flags
	void EvalState::setVar(std::string name, bool isConst, bool isTyped) {
		if (vars.count(name) == 0) return newVar(name, isConst, isTyped);					// If the variable doesn't already exist
		auto& var = vars[name];

		// If the variable is marked "constant"
		if (var.is_const) throw std::string{ "Attempt to reassign a constant variable" };

		// If the variable is statically typed
		if (var.type_id != ts.NIL && !ts.isChildType(at().type_id, var.type_id))
			return staticTyping(var, isConst);

		// Remove the variables current value and give it the new one
		if (var.val.type_id == type::Traits<std::string>::id) gc.decRef(var.val.val.i);
		var.val = pop();
		if (var.val.type_id == type::Traits<std::string>::id) gc.incRef(var.val.val.i);

		// Ensure the isConst and type_id flags are current
		var.is_const = isConst;
		if (isTyped) var.type_id = var.val.type_id;
	}

	// Push the variable onto the stack (0 if it doesn't exist)
	void EvalState::getVar(std::string var) {
		if (vars.count(var) == 0) return push(0);			// Temporary as nil is not implemented
		push(vars[var].val);
	}

	void EvalState::mark_constant(std::string var) {
		if (vars.count(var) == 0) return;					// Temporary. Haven't determined what should happen in these circumstances
		vars[var].is_const = !vars[var].is_const;
	}

	void EvalState::set_typing(std::string var, size_t typ) {
		if (vars.count(var) == 0) return;					// Temporary. Haven't determined what should happen in these circumstances

		if (typ != ts.NIL) {								// If the variable is being statically typed
			if (!ts.convertible(vars[var].type_id, typ)) throw std::string{ "No converter from the current value to the given type" };

			getVar(var);
			callMethod(ts.getName(typ));
			if (vars[var].val.type_id == type::Traits<std::string>::id) gc.decRef(vars[var].val.val.i);
			vars[var].val = pop();
		}

		vars[var].type_id = typ;
	}

	bool EvalState::isConst(std::string var) {
		return vars[var].is_const;
	}

	bool EvalState::isStatic(std::string var) {
		return vars[var].type_id != ts.NIL;
	}


	// Free functions
	EvalState& EvalState::call(std::string fn) {
		if (fn == "type")
			push((int)(pop().type_id));
		else if (fn == "typename")
			push(ts.getName(pop().type_id));

		return *this;
	}

	// Operators
	EvalState& EvalState::callOp(std::string fn) {
		if (fn.at(0) == '_' && fn.at(2) == 'u') return callMethod(fn);
		if (fn.at(0) != '_' || fn.at(2) != 'p') throw std::string{ "Bad API Call: Attempt to callOp on a non-operator" };

		auto l = at(-2).type_id, r = at().type_id;
		auto com_t = ts.com(l, r, fn);					// common type
		auto dis_t = ts.findDef(com_t, fn);				// dispatch type (where fn is defined)

		// This error isn't very explanatory though
		if (dis_t == ts.NIL) throw std::string{ "Dispatch error: Method " + fn + " is not defined for objects of type " + ts.getName(dis_t) };

		// Determine whether com selected a converter and perform conversions
		if ((com_t == l ^ com_t == r) && ts.convertible(l, r)) {
			forceType(-1, com_t);					// The name is a placeholder (Maybe move to be a TypeSystem method ???)
			forceType(-2, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary
		}

		auto rets = ts.get(dis_t).ops[fn](*this);

		//std::cout << fn << ": " << rets << std::endl;
		return *this;
	}

	// Methods
	EvalState& EvalState::callMethod(std::string fn) {
		auto dis_t = ts.findDef(at().type_id, fn);

		if (dis_t == ts.NIL) throw std::string{ "Dispatch error: Method " + fn + " is not defined for objects of type " + ts.getName(dis_t) };

		auto rets = ts.get(dis_t).ops[fn](*this);

		//std::cout << fn << ": " << rets << std::endl;
		return *this;
	}

}