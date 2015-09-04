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

	// Free functions
	EvalState& EvalState::call(std::string fn) {
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

		std::cout << fn << ": " << rets << std::endl;
		return *this;
	}

	// Methods
	EvalState& EvalState::callMethod(std::string fn) {
		auto dis_t = ts.findDef(at().type_id, fn);

		if (dis_t == ts.NIL) throw std::string{ "Dispatch error: Method " + fn + " is not defined for objects of type " + ts.getName(dis_t) };

		auto rets = ts.get(dis_t).ops[fn](*this);

		std::cout << fn << ": " << rets << std::endl;
		return *this;
	}

}