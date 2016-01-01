#include "EvalState.h"
#include "Exceptions\dust.h"

#include <unordered_set>

namespace dust {

	// Definitions expected by Function::push
	namespace impl {
		GC& getGC(EvalState& e) {
			return e.getGC();
		}

		void push(EvalState& e, impl::Value v) {
			e.push(v);
		}
	}

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;
		swap(idx, -1);								// Move the value to the stack top

		// Special conversion for Tables (needs to be simplified)
		if (type == type::Traits<Table>::id) {
			newScope();
			push(1);
			swap();
			setScoped();
			pushScope(2);

		} else
			callMethod(ts.getName(type));			// Call the converter (if execution reaches here, the converter exists)

		swap(idx, -1);								// Restore the stack positions
	}

	/*	THIS NEEDS A BETTER EXPLANATION
	 * Returns a parent scope where var is defined, var being a variable name, starting with the current evaluation scope
	 * If lvl > 1, repeat this process starting with the intermediate scope, the result of findScope(var, 1, not_null)
	 * If not_null, return the global scope if the search "overruns" the available scopes (var is defined in 'n' scopes, n < lvl)
	 */
	Table EvalState::findScope(const impl::Value& var, int lvl, bool not_null) {
		Table scp = curr_scp;

		while (scp && lvl-- && (scp = scp->findDef(var)))
			if (lvl) scp = scp->getPar();

		// Return the global table if scp is nullptr and the not_null field is set
		return (!scp && not_null) ? &global : scp;
	}

	// Constructor
	EvalState::EvalState() : ts{}, gc{}, CallStack{ gc }, global{}, curr_scp{ nullptr } {}

	// Preliminary implementations of function calling that is divided based on function type

	// Free functions
	EvalState& EvalState::call(const std::string& fn) {
		if (fn == "type")
			push((int)(pop().type_id));

		else if (fn == "typename")
			push(ts.getName(pop().type_id));

		return *this;
	}
	
	// Operators
		// Expects stack = ..., {rhs}, {lhs}
	EvalState& EvalState::callOp(const std::string& fn) {
		if (fn.at(0) == '_' && fn.at(2) == 'u') return callMethod(fn);
		if (fn.at(0) != '_' || fn.at(2) != 'p') throw error::bad_api_call{ "Attempt to call EvalState::callOp on a non-operator" };

		auto r = at(-2).type_id, l = at().type_id;
		auto com_t = ts.com(l, r, fn);						// find common type for the two arguments
		auto dis_t = ts.findDef(com_t, fn);					// find dispatch type (where fn is defined)

		if (dis_t == ts.NO_DEF) throw error::dispatch_error{ fn, ts.getName(l), ts.getName(r) };
		
		// Determine whether com selected a converter and perform conversions
		if (((com_t == l) ^ (com_t == r)) && (com_t == type::Traits<Table>::id || ts.convertible(l, r))) {
			forceType(-1, com_t);					// Forces the value at the given index to have the given type by calling a converter if necessary
			forceType(-2, com_t);
		}

		// rets might be used in some future stack manipulation operations, but it's uncertain what they would be
		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}
	
	// Methods (Currently used for calling constructors)
	EvalState& EvalState::callMethod(const std::string& fn) {
		auto dis_t = ts.findDef(at().type_id, fn);
		if (dis_t == ts.NO_DEF) throw error::dispatch_error{ fn, ts.getName(at().type_id) };
		
		auto rets = ts.get(dis_t).ops[fn](*this);

		return *this;
	}

	// Adds key and value checking
	void EvalState::setVar(Table t, const impl::Value& key, bool is_const, bool is_typed) {
		if (!t->okayKey(key))
			throw error::illegal_operation{ "Attempt to index a table with an invalid key" };

		if (!t->okayValue(at()))
			throw error::illegal_operation{ "Attempt to store a value inside a table that doesn't accept it" };

		setVar(t->getVar(key), is_const, is_typed);
	}

	// Expects stack = ..., {val}
	void EvalState::setVar(impl::Variable& var, bool is_const, bool is_typed) {
		if (var.is_const) throw error::illegal_operation{ "Attempt to reassign a constant variable" };
		// add check for table ???

		// if the variable held a value (static typing is only a concern in this case)
		if (var.val.type_id != type::Traits<Nil>::id) {
			// if the var is statically typed and the new value is not a child type
			if (var.type_id != type::Traits<Nil>::id && !ts.isChildType(at().type_id, var.type_id)) {

				// Not sure about the logic here
				if (!ts.convertible(var.type_id, at().type_id))	throw error::dispatch_error{ "No converter from from the assigned value to the variable's static type" };
				callMethod(ts.getName(var.type_id));
			}

			try_decRef(var.val);
		}

		// Set the variable data
		var.val = pop();															// `pop` (no templates) doesn't decrement references
		try_incRef(var.val);
		var.is_const = is_const;
		if (is_typed) var.type_id = var.val.type_id;
	}

	void EvalState::getVar(Table tbl, const impl::Value& var) {
		if (!tbl->okayKey(var))
			throw error::illegal_operation{ "Attempt to index a table with an invalid key" };

		tbl->hasKey(var) ? push(tbl->getVal(var)) : pushNil();
	}
	
	void EvalState::setScoped(const impl::Value& name, int _lvl, bool is_const, bool is_typed) {
		try_incRef(name);
		setVar(findScope(name, _lvl, true), name, is_const, is_typed);
	}

	// Expects stack = ..., {var}, {val}
	void EvalState::setScoped(int lvl, bool is_const, bool is_typed) {
		setScoped(pop(-2), lvl, is_const, is_typed);
	}

	// Expects stack = ..., {table}, {var}, {val}
	// Leaves stack = ..., {table}
	void EvalState::set() {
		if (!is<Table>(-3)) throw error::illegal_operation{ "Attempt to initialize a non-table field" };

		auto tbl = pop<Table>(-3);
		try_incRef(at(-2));

		setVar(tbl, pop(-2), false, false);
		push(tbl);
	}

	// Deprecated as it doesn't follow the same "semantics" as set()
	void EvalState::set(const impl::Value& var) {
		throw error::bad_api_call{ "EvalState::set(const impl::Value&) is currently deprecated" };

		if (!is<Table>()) throw error::illegal_operation{ "Attempt to initialize a non-stored variable" };

		setVar(pop<Table>()->getVar(var), false, false);
	}

	void EvalState::getScoped(const impl::Value& name, int _lvl) {
		getVar(findScope(name, _lvl + 1, true), name);
	}

	// Expects stack = ..., {var}
	void EvalState::getScoped(int lvl) {
		getScoped(pop(), lvl);
	}

	// Expects stack = ..., {table}, {var}
	void EvalState::get() {
		get(pop());
	}

	// Expects stack = ..., {table}
	void EvalState::get(const impl::Value& var) {
		if (is<Nil>()) return;										// TODO Change when implementing type methods ???
		if (!is<Table>()) return pop(), pushNil();

		getVar(pop<Table>(), var);
	}

	// Not currently used in any capacity
	void EvalState::markConst(const impl::Value& name) {
		auto scp = findScope(name, 0);

		if (scp) scp->getVar(name).is_const = !scp->getVar(name).is_const;
	}

	// Not currently used in any capacity
	void EvalState::markTyped(const impl::Value& name, size_t typ) {
		auto scp = findScope(name, 0);
		if (!scp) return;		// throw error::base{ "Attempt to static type a nil value" };
		
		auto& var = scp->getVar(name);

		// If the typing change may require type conversion (ie. typ not Nil and !(type(var) <= typ))
		if (typ != type::Traits<Nil>::id && !ts.isChildType(var.type_id, typ)) {
			if (!ts.convertible(var.type_id, typ)) throw error::converter_not_found{ ts.getName(var.type_id), ts.getName(typ) };

			push(var.val);
			callMethod(ts.getName(typ));
			try_decRef(var.val);
			var.val = pop();										// `pop` (no templates) doesn't decrement references
		}

		var.val.type_id = typ;
	}

	// Not currently used in any capacity
	bool EvalState::isConst(const impl::Value& name) {
		auto scp = findScope(name, 0);

		return scp && scp->getVar(name).is_const;
	}

	// Not currently used in any capacity
	bool EvalState::isTyped(const impl::Value& name) {
		auto scp = findScope(name, 0);

		return scp && scp->getVar(name).type_id != type::Traits<Nil>::id;
	}

	void EvalState::newScope() {
		curr_scp = curr_scp ? new impl::Table{ curr_scp } : &global;
	}

	void EvalState::endScope() {
		auto* sav = curr_scp->getPar();

		if (sav) {
			for (auto pair : *curr_scp) {
				try_decRef(pair.first);
				try_decRef(pair.second.val);
			}

			delete curr_scp;					// If the current scope wasn't the global scope
		}

		curr_scp = sav;
	}

	void EvalState::pushScope(int nxt) {
		Table sav = curr_scp;
		curr_scp = curr_scp->getPar();

		sav->setNext(nxt);
		push(sav);
	}

	type::TypeSystem& EvalState::getTS() {
		return ts;
	}

	impl::GC& EvalState::getGC() {
		return gc;
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
		type::Traits<dust::Function>::id = Function;
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

		Int.addOp("String", [](EvalState& e) { e.push((std::string)e); return 1; });

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
		auto Function = ts.getType("Function");

		//! -
		//^ * / + - % < = > <= != >=

		// Define _op<=, _op>=, and _op!= in relation to _op<, _op>, _ou!, and _ou= for all types
		Object.addOp("_op<=", [](EvalState& e) {
			e.copy(-2);				// Make copies of the arguments
			e.copy(-2);
			e.callOp("_op<");		// Call the _op< for the types

			if (!(bool)e)			// Short-circuit evaluation. (pops from the stack)
				e.callOp("_op=");	// Call the _op= for the types. QUERY What if A._op< and B._op= are defined but A._op= is not defined ?

			else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});
		
		Object.addOp("_op>=", [](EvalState& e) {
			e.copy(-2);				// Make copies of the arguments
			e.copy(-2);
			e.callOp("_op>");		// Call the _op> for the types

			if (!(bool)e)			// Short-circuit evaluation. (pops from the stack)
				e.callOp("_op=");	// Call the _op= for the types. QUERY See defintion for Object._op<=

			else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});
		
		Object.addOp("_op!=", [](EvalState& e) {
			e.callOp("_op=");			// Throws if _op= is not defined by the type
			e.callOp("_ou!");			// Throws if _op= doesn't return a boolean
			return 1;
		});

		// Define _op^ and _op/ (exponentation and division) for Ints and Floats
		Number.addOp("_op^", [](EvalState& e) {
			auto base = (double)e;
			e.push(pow(base, (double)e));
			return 1;
		});
		
		Number.addOp("_op/", [](EvalState& e) { e.push((double)e / (double)e); return 1; });

		// Define math operators for Ints
		Int.addOp("_op+", [](EvalState& e) { e.push((int)e + (int)e); return 1; });
		Int.addOp("_op-", [](EvalState& e) { e.push((int)e - (int)e); return 1; });
		Int.addOp("_op*", [](EvalState& e) { e.push((int)e * (int)e); return 1; });
		Int.addOp("_op%", [](EvalState& e) { e.push((int)e % (int)e); return 1; });
		Int.addOp("_op<", [](EvalState& e) { e.push((int)e < (int)e); return 1; });		// 
		Int.addOp("_op=", [](EvalState& e) { e.push((int)e == (int)e); return 1; });
		Int.addOp("_op>", [](EvalState& e) { e.push((int)e >(int)e); return 1; });
		Int.addOp("_ou-", [](EvalState& e) { e.push(-(int)e); return 1; });

		// Define math operators for Floats
		Float.addOp("_op+", [](EvalState& e) { e.push((double)e + (double)e); return 1; });
		Float.addOp("_op-", [](EvalState& e) { e.push((double)e - (double)e); return 1; });
		Float.addOp("_op*", [](EvalState& e) { e.push((double)e * (double)e); return 1; });
		Float.addOp("_op<", [](EvalState& e) { e.push((double)e < (double)e); return 1; });		// 
		Float.addOp("_op=", [](EvalState& e) { e.push((double)e == (double)e); return 1; });
		Float.addOp("_op>", [](EvalState& e) { e.push((double)e >(double)e); return 1; });
		Float.addOp("_ou-", [](EvalState& e) { e.push(-(double)e); return 1; });

		// Define concatentation (+) and equality (=) operators for Strings
		String.addOp("_op+", [](EvalState& e) { e.push((std::string)e + e.pop<std::string>(-2)); return 1; });			// Why is Int._op- correct then???
		String.addOp("_op=", [](EvalState& e) { e.push(e.pop().val.i == e.pop().val.i); return 1; });

		// Define _op= and _ou! for Bools
		Bool.addOp("_op=", [](EvalState& e) { e.push((bool)e == (bool)e); return 1; });
		Bool.addOp("_ou!", [](EvalState& e) { e.push(!(bool)e);  return 1; });

		// Table functions
			// Currently these functions "erase" non-integer key values
				// This means that they migrate the values existence into the new table
				// However the value is assigned based on the current open integer key

		// Append element(s) to table (+)
		Table.addOp("_op+", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();			// Create a new table
			int nxt = 1;

			// Push all elements from the left table into the new table
			for (auto l_elem : *lt) {
				e.push(nxt++);
				e.push(l_elem.second.val);
				e.setScoped();
			}

			// Same for the right table
			for (auto r_elem : *rt) {
				e.push(nxt++);
				e.push(r_elem.second.val);
				e.setScoped();
			}

			e.pushScope(nxt);
			return 1;
		});

		// Remove element(s) from table (-)
		Table.addOp("_op-", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();			// Create a new table
			int nxt = 1;

			// Push all elements from the left table into the new table
				// Provided they aren't found in the right table
			for (auto l_elem : *lt)
				if (!rt->contains(l_elem.second.val)) {
					e.push(nxt++);
					e.push(l_elem.second.val);
					e.setScoped();
				}

			e.pushScope(nxt);
			return 1;
		});
		
		// Union (Append and remove duplicates) (*)
		Table.addOp("_op*", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			// Push all elemnts from both tables into a set
			std::unordered_set<impl::Value> nt{};// { lt->begin(), lt->end() };
			for (auto l_elem : *lt)
				nt.insert(l_elem.second.val);
			for (auto r_elem : *rt)
				nt.insert(r_elem.second.val);

			e.newScope();		// Create a table
			int nxt = 1;

			// Push all elements from the set into the new table
			for (auto elem : nt) {
				e.push(nxt++);
				e.push(elem);
				e.setScoped();
			}

			e.pushScope(nxt);
			return 0;
		});
		
		// Intersection (Elements in both tables) (^)
		Table.addOp("_op^", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();		// Create a table
			int nxt = 1;

			// Push all elements from the left table into the new table
				// Provided they aren found in the right table
			for (auto l_elem : *lt)
				if (rt->contains(l_elem.second.val)) {
					e.push(nxt++);
					e.push(l_elem.second.val);
					e.setScoped();
				}

			e.pushScope(nxt);
			return 1;
		});
		
		// Member-wise comparison (=)
		Table.addOp("_op=", [](EvalState& e) {
			if (e.at().val.i == e.at(-2).val.i) {		// Short-cut if the two tables are the same reference
				e.pop();
				e.pop();
				e.push(true);

				return 1;
			}
			
			dust::Table rt = e.pop<dust::Table>(), lt = e.pop<dust::Table>();

			if (lt->size() != rt->size()) {			// Short-cut if the two tables have different sizes
				e.push(false);
				return 1;
			}

			auto lt_iter = lt->begin(), lt_end = lt->end();

			// For each element in {l, r}, check if they are equal
			for (auto rt_elem : *rt) {
				e.push(lt_iter->second.val);
				e.push(rt_elem.second.val);
				e.callOp("_op=");

				if (!e.at().val.i)
					return 1;		// Short-circuit if an element is not found in r

				++lt_iter;
				if (lt_iter != lt_end)
					e.pop();
			}

			return 1;
		});
	

		// Function functions
	}
}