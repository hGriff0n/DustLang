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

	Table EvalState::getScope() {
		return curr_scp ? curr_scp : &global;
	}

	void EvalState::forceType(int idx, size_t type) {
		if (at(idx).type_id == type) return;
		swap(idx, -1);								// Move the value to the stack top

		// Special conversion for Tables (needs to be simplified)
		if (type == type::Traits<Table>::id) {
			newScope();
			push(1);								// Set scp.1 = pop
			swap();
			set(SCOPE);
			push(2);
			pushScope();

		} else {
			push(ts.get(at().type_id).fields->getVal(type::Traits<std::string>::make(ts.getName(type), gc)));		// Call the converter (if execution reaches here, the converter exists)
			call(1);
		}

		swap(idx, -1);								// Restore the stack positions
	}

	void EvalState::try_supplement(impl::Value& val, size_t type_id) {
		auto method = type::Traits<std::string>::get(at(), gc);

		if (method == "new") {
			push([usernew{ type::Traits<Function>::get(val, gc) }, id{ type_id }](EvalState& e) {
				// Create the object instance structure
				Table typ = type::Traits<Table>::get(e.getTS().get(id).ref, e.getGC());
				Table instance = new impl::Table{ typ };
				e.copyInstance(instance, typ);

				// Setup OOP structure and syntax
				e.push(instance);
				e.at().type_id = id;
				e.at().object = true;
				e.copy();
				e.set(EvalState::SELF);
				auto ret = e.pop();					// Get a reference to the object for returning

				// Allow user code to modify the created object
				usernew(e);

				// TODO: Figure out a way to handle multiple returns from usernew
					// I can't know the number of arguments either
				e.pop();							// I'm assuming one return value for now
				e.push(ret);

				return 1;
			});

			val = pop();

		} else if (method == "drop") {
			push([userdrop{ type::Traits<Function>::get(val, gc) }, id{ type_id }](EvalState& e) {
				e.enableObjectSyntax().get(EvalState::SELF);

				// Run the user function
				userdrop(e);

				// Setup code for language drop
				e.get(EvalState::SELF);
				auto tbl = e.pop();
				Table instance = type::Traits<Table>::get(tbl, e.getGC());

				// destroy the table
				// How to do that ???

				return 0;
			});

			val = pop();
		}
	}

	/*	THIS NEEDS A BETTER EXPLANATION
	 * Returns a parent scope where var is defined, var being a variable name, starting with the current evaluation scope
	 *   lvl > 1  -> repeat this process starting with the intermediate scope, the result of findScope(var, 1, not_null)
	 */
	Table EvalState::findDef(Table tbl, const impl::Value& key, int lookup) {
		while (tbl && lookup-- > 0 && (tbl = tbl->findDef(key)))
			if (lookup) tbl = tbl->getPar();

		return tbl;
	}

	void EvalState::getTable(Table tbl, const impl::Value& key) {
		if (!tbl) return pushNil();
			// throw error::null_exception{ "tbl is null in getTable" };

		if (!tbl->okayKey(key))
			throw error::illegal_operation{ "Attempt to index a table with an invalid key" };

		tbl->hasKey(key) ? push(tbl->getVal(key)) : pushNil();
	}

	void EvalState::setTable(Table tbl, const impl::Value& key, const impl::Value& val, bool instance) {
		if (!tbl) throw error::null_exception{ "tbl is null in setTable" };

		// TODO: Implement Table::okayKey
		if (!tbl->okayKey(key))
			throw error::illegal_operation{ "Attempt to index a table with an invalid key" };

		// TODO: Implement Table::okayValue
		if (!tbl->okayValue(val))
			throw error::illegal_operation{ "Attempt to store an invalid value in a table" };

		auto& var = tbl->getVar(key);

		var.is_member = instance;
		try_decRef(var.val);
		try_incRef(var.val = val);
	}

	// Constructor
	EvalState::EvalState() : ts{}, gc{}, CallStack{ gc }, self{}, global{}, curr_scp{ nullptr } {}

	void EvalState::call(int num_args) {
		// stack: ..., {arg0}, {fn}			The first argument is on the top

		if (num_args >= size()) throw error::bad_api_call{ "Attempt to call function with more arguments than values on the stack" };

		// Note: loc may equal -1 if num_args == size() - 1
		size_t loc = Stack::size() - num_args - 1;								// Index of the value before the argument list (the function being called)

		// Ensure there is a callable object at the expected location
		if (!is<Function>()) {
			// I can set self in here (especially when I add in metamethods)
			//push("_op()");
			//get(-2);

			throw error::dispatch_error{ "Attempt to call a non-function" };
		}

		// Enter the function
		newScope();
		size_t old_limit = setMinSize(loc);										// Limit the stack size for the child process (handles too few arguments)
		int num_ret = 1;

		// Perform the call
		try {
			num_ret = pop<Function>()(*this);									// loc now points to the last argument

			// Ensure self doesn't pollute outside of the function call
			try_decRef(self);
			self.type_id = type::Traits<Nil>::id;

			// I'm guessing -1 means "I got this" (the function handles the stack)
			if (num_ret >= 0) {

				// Ensure return values are at the correct position on the stack
				loc += num_ret++;												// Stack size with all return values left
				while (loc != Stack::size())									// Remove leftover values (handles too many arguments)
					pop(-num_ret);												// The return values are on the stack's top
			}

		} catch (...) {
			// Reset the stack and leave the function
			while (!empty()) pop();

			setMinSize(old_limit);
			endScope();

			throw;
		}

		// Leave the function
		setMinSize(old_limit);
		endScope();

		// stack: ..., {ret0}, ...				The last return value is on the top
	}

	void EvalState::callOp(std::string op) {
		// stack: ..., {rhs}, {lhs}

		if (op.at(0) != '_' && op.at(1) != 'o') throw error::bad_api_call{ "Attempt to call EvalState::callOp on a non-operator" };
		auto fn = type::Traits<std::string>::make(op, gc);

		// Handle unary operators
		if (op.at(2) == 'u') {
			auto dis_t = ts.findDef(at().type_id, fn);

			if (dis_t == ts.NO_DEF) throw error::dispatch_error{ op, ts.getName(at().type_id) };

			push(ts.get(dis_t).fields->getVal(fn));
			call(1);

		// Handle binary operators
		} else if (op.at(2) == 'p' && op.at(3) != '(') {
			auto r = at(-2).type_id, l = at().type_id;
			auto com_t = ts.com(l, r, fn);					// Find the common type of the two argments
			auto dis_t = ts.findDef(com_t, fn);				// Find the dispatch type of the function

			if (dis_t == ts.NO_DEF) throw error::dispatch_error{ op, ts.getName(l), ts.getName(r) };

			// Determine whether com selected a converter and perform the conversions
				// Should I forceType if dis_t is a parent of r and l
			if (((com_t == l) ^ (com_t == r)) && (com_t == type::Traits<Table>::id || ts.convertible(l, r))) {
				forceType(-1, com_t);
				forceType(-2, com_t);
			}

			push(ts.get(dis_t).fields->getVal(fn));
			call(2);

		// Handle _op()
		} else if (op.at(2) == 'p' && op.at(3) == '(') {
			// TODO: Implement

		} else
			throw error::bad_api_call{ "Attemp to call EvalState::callOp on a non-operator" };

		// stack: ..., op(lhs, rhs)
	}

	void EvalState::get(int idx, int lookup) {
		auto self_key = type::Traits<std::string>::make("self", gc);
		Table tbl = nullptr;

		switch (idx) {
			case SELF:												// SCOPE.self
				return getTable(getScope(), self_key);

			case SCOPE:

				// We're not forcing lookup, so first look in the current scope
				if (!lookup) {
					auto scp = getScope();

					if (scp->hasKey(at()))
						tbl = scp;

					// SCOPE.self.x
					else if (scp->hasKey(self_key)) {
						tbl = type::Traits<Table>::get(scp->getVal(self_key), gc);

						if (!tbl->hasKey(at())) tbl = nullptr;
					}
				}

				// March up the scope tree until I find a definition
				if (!tbl) tbl = findDef(getScope(), at(), lookup + 1);

				break;
			default:
				// {idx}.field
				if (is<Table>(idx))
					tbl = pop<Table>(idx);

				// {type}.method
				else {
					// Ensure self gets set if necessary
					if (resolving_function) {
						copy(idx);
						set(SELF);
					}

					tbl = ts.get(at(idx).type_id).fields;

					//Prefer accessing members over statics
						// Note: If I remove this check, then all members are private (even from type methods, so not a good solution) 
					if (at(idx).object) {
						Table tmp = type::Traits<Table>::get(at(idx), gc);
						
						// If the instance has the member, take it
						if (tmp->hasKey(at())) tbl = tmp;
					}
					
					pop(idx);
				}

				break;
		}

		getTable(tbl ? tbl : &global, pop());

		// stack: ..., {value}
	}

	void EvalState::set(int idx, bool instance, int lookup) {
		Table tbl = nullptr;
		auto value = pop();

		switch (idx) {
			case SELF:
				try_decRef(self);
				return try_incRef(self = value);

			case SCOPE:
				tbl = findDef(getScope(), at(), lookup);
				break;

			default:
				if (idx < 0) idx += 1;

				// Table handling
				if (is<Table>(idx)) {
					tbl = pop<Table>(idx);

					// Type table specific handling
					if (tbl->hasKey(type::Traits<std::string>::make("__type", gc))) {
						auto typ = (size_t)tbl->getVal(type::Traits<std::string>::make("__typid", gc)).val.i;
						try_supplement(value, typ);
					}

				// OOP handling
				} else if (at(idx).object) {
					tbl = pop<Table>(idx);

					if (!tbl->hasKey(at())) {
						pop();
						throw error::dust_error{ "Attempt to set static field of an object" };
					}

				} else {
					throw error::dust_error{ "Attempt to set a field of a non-table value" };
				}

				break;
		}

		setTable(tbl, pop(), value, instance);

		// stack: ..., {value}
	}

	EvalState& EvalState::enableObjectSyntax() {
		// Set SELF to stack top if it isn't set
		if (self.type_id == type::Traits<Nil>::id)
			try_incRef(self = pop());

		// Set SCOPE.self to SELF
		push("self");
		push(self);
		set(SCOPE);

		// Delete SELF
		try_decRef(self);
		self.type_id = type::Traits<Nil>::id;

		return *this;
	}

	void EvalState::setResolvingFunctionName() {
		resolving_function = !resolving_function;
	}

	/*
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
	//*/

	void EvalState::newScope() {
		curr_scp = curr_scp ? new impl::Table{ curr_scp } : &global;
	}

	void EvalState::endScope() {
		Table sav = nullptr;

		// Delete curr_scp iff it's not the global scope
		if (curr_scp != &global) {
			for (auto pair : *curr_scp) {
				try_decRef(pair.first);
				try_decRef(pair.second.val);
			}

			sav = curr_scp->getPar();
			delete curr_scp;					// If the current scope wasn't the global scope
		}

		curr_scp = sav;
	}

	void EvalState::pushScope() {
		// stack: ..., {nxt}

		Table sav = curr_scp;
		curr_scp = sav->getPar();

		sav->setNext(pop<int>());
		push(sav);

		// stack: ..., {table}
	}

	type::TypeSystem& EvalState::getTS() {
		return ts;
	}

	impl::GC& EvalState::getGC() {
		return gc;
	}

	void EvalState::completeDef(type::TypeSystem::TypeVisitor& typ) {
		using namespace type;
		using std::string;
		
		auto table = pop();
		auto tbl = Traits<Table>::get(table, gc);


		/*
		 * Provide default implementations for language functions
		 */

		// New
		static impl::Value new_fn;
		push([id{ (size_t)typ }, type{ tbl }](EvalState& e) {
			// Initialize type table (Just use a table for now)
			Table obj = new impl::Table{ type };

			// Copy over type table entries
			e.copyInstance(obj, type);
			e.push(obj);

			// Ensure the type information is correct
			e.at().type_id = id;
			e.at().object = true;

			return 1;
		});
		setTable(tbl, Traits<string>::make("new", gc), pop(), false);

		// Drop
		push([](EvalState& e) {
			Table instance = e.pop<Table>();

			// Decrement all references
			return 0;
		});
		setTable(tbl, Traits<string>::make("drop", gc), pop(), false);

		// Copy
		push([](EvalState& e) {
			e.enableObjectSyntax().get(EvalState::SELF);

			size_t id = e.at().type_id;
			Table orig = e.pop<Table>();
			Table copy = new impl::Table{ orig->getPar() };

			// Copy the instance over
			e.copyInstance(copy, orig);
			e.push(copy);
			e.at().type_id = id;
			e.at().object = true;

			return 1;
		});
		setTable(tbl, Traits<string>::make("copy", gc), pop(), false);


		/*
		 * Set auto-defined fields
		 */
		setTable(tbl, Traits<string>::make("__type", gc), table, false);
		setTable(tbl, Traits<string>::make("__typid", gc), Traits<size_t>::make(typ, gc), false);
		setTable(tbl, Traits<string>::make("class", gc), Traits<string>::make(ts.getName(typ), gc), false);


		/*
		 * Associate the table to the relevant type
		 */
		try_incRef(const_cast<Type&>(ts.get(typ)).ref = table);												// Ensure the type table isn't collected
		ts.setMethods(typ, tbl);
	}

	void EvalState::assignRef(type::Type& typ) {
		copy();
		try_incRef(typ.ref = pop());
	}

	void EvalState::copyInstance(Table t, Table f) {
		// Copy over instance variables
		for (auto entry : *f)
			if (entry.second.is_member)
				setTable(t, entry.first, entry.second.val, true);

		// Copy other state
	}


	// Note: I'm going to be redoing these really soon

	void initState(EvalState& e) {
		initTypeSystem(e.ts);
		initConversions(e);
		initOperations(e);
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

	void initConversions(EvalState& e) {
		auto& ts = e.getTS();
		auto Int = ts.getType("Int");
		auto Float = ts.getType("Float");
		auto String = ts.getType("String");

		// Initialize Conversions
		e.addMember(Int, "String", [](EvalState& e) {
			e.push((std::string)e);
			return 1;
		});
		e.addMember(Int, "Float", [](EvalState& e) {
			e.push((double)e);
			return 1;
		});

		e.addMember(Float, "String", [](EvalState& e) {
			e.push((std::string)e);
			return 1;
		});
		e.addMember(Float, "Int", [](EvalState& e) {
			e.push((int)e);
			return 1;
		});

		e.addMember(String, "Int", [](EvalState& e) {
			e.push((int)e);
			return 1;
		});
		e.addMember(String, "Float", [](EvalState& e) {
			e.push((float)e);
			return 1;
		});

	}

	void initOperations(EvalState& e) {
		auto& ts = e.getTS();

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
		e.addMember(Object, "_op<=", [](EvalState& e) {
			e.copy(-2);				// Make copies of the arguments
			e.copy(-2);
			e.callOp("_op<");		// Call the _op< function

			if (!(bool)e) {			// Short-circuit evaluation. (pops from the stack)
				e.callOp("_op=");	// Call the _op= for the types. QUERY What if A._op< and B._op= are defined but A._op= is not defined ?

			} else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});

		e.addMember(Object, "_op>=", [](EvalState& e) {
			e.copy(-2);				// Make copies of the arguments
			e.copy(-2);
			e.callOp("_op>");		// Call the _op> for the types

			if (!(bool)e) {			// Short-circuit evaluation. (pops from the stack)
				e.callOp("_op=");	// Call the _op= for the types. QUERY See defintion for Object._op<=

			} else {
				e.pop();
				e.pop();
				e.push(true);
			}

			return 1;
		});

		e.addMember(Object, "_op!=", [](EvalState& e) {
			e.callOp("_op=");				// Throws if _op= is not defined by the type
			e.callOp("_ou!");				// Throws if _op= doesn't return a boolean
			return 1;
		});

		// Define _op^ and _op/ (exponentation and division) for Ints and Floats
		e.addMember(Number, "_op^", [](EvalState& e) {
			auto base = (double)e;
			e.push(pow(base, (double)e));
			return 1;
		});
		e.addMember(Number, "_op/", [](EvalState& e) {
			e.push((double)e / (double)e);
			return 1;
		});

		// Define math operators for Ints
		e.addMember(Int, "_op+", [](EvalState& e) {
			e.push((int)e + (int)e);
			return 1;
		});
		e.addMember(Int, "_op-", [](EvalState& e) {
			e.push((int)e - (int)e);
			return 1;
		});
		e.addMember(Int, "_op*", [](EvalState& e) {
			e.push((int)e * (int)e);
			return 1;
		});
		e.addMember(Int, "_op%", [](EvalState& e) {
			e.push((int)e % (int)e);
			return 1;
		});
		e.addMember(Int, "_op<", [](EvalState& e) {
			e.push((int)e < (int)e);
			return 1;
		});
		e.addMember(Int, "_op=", [](EvalState& e) {
			e.push((int)e == (int)e);
			return 1;
		});
		e.addMember(Int, "_op>", [](EvalState& e) {
			e.push((int)e >(int)e);
			return 1;
		});
		e.addMember(Int, "_ou-", [](EvalState& e) {
			e.push(-(int)e);
			return 1;
		});

		// Define math operators for Floats
		e.addMember(Float, "_op+", [](EvalState& e) {
			e.push((double)e + (double)e);
			return 1;
		});
		e.addMember(Float, "_op-", [](EvalState& e) {
			e.push((double)e - (double)e);
			return 1;
		});
		e.addMember(Float, "_op*", [](EvalState& e) {
			e.push((double)e * (double)e);
			return 1;
		});
		e.addMember(Float, "_op<", [](EvalState& e) {
			e.push((double)e < (double)e);
			return 1;
		});		// 
		e.addMember(Float, "_op=", [](EvalState& e) {
			e.push((double)e == (double)e);
			return 1;
		});
		e.addMember(Float, "_op>", [](EvalState& e) {
			e.push((double)e >(double)e);
		return 1;
		});
		e.addMember(Float, "_ou-", [](EvalState& e) {
			e.push(-(double)e);
			return 1;
		});

		// Define concatentation (+) and equality (=) operators for Strings
		e.addMember(String, "_op+", [](EvalState& e) {
			e.push((std::string)e + e.pop<std::string>(-2));
			return 1;
		});
		e.addMember(String, "_op=", [](EvalState& e) {
			e.push(e.pop().val.i == e.pop().val.i);
			return 1;
		});

		// Define _op= and _ou! for Bools
		e.addMember(Bool, "_op=", [](EvalState& e) {
			e.push((bool)e == (bool)e);
			return 1;
		});
		e.addMember(Bool, "_ou!", [](EvalState& e) {
			e.push(!(bool)e);
			return 1;
		});

		// Table functions
			// Currently these functions "erase" non-integer key values
				// This means that they migrate the values existence into the new table
				// However the value is assigned based on the current open integer key

		// Append element(s) to table (+)
		e.addMember(Table, "_op+", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();			// Create a new table
			int nxt = 1;

			// Push all elements from the left table into the new table
			for (auto l_elem : *lt) {
				e.push(nxt++);
				e.push(l_elem.second.val);
				e.set(EvalState::SCOPE);
			}

			// Same for the right table
			for (auto r_elem : *rt) {
				e.push(nxt++);
				e.push(r_elem.second.val);
				e.set(EvalState::SCOPE);
			}

			e.push(nxt);
			e.pushScope();
			return 1;
		});

		// Remove element(s) from table (-)
		e.addMember(Table, "_op-", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();			// Create a new table
			int nxt = 1;

			// Push all elements from the left table into the new table
				// Provided they aren't found in the right table
			for (auto l_elem : *lt)
				if (!rt->contains(l_elem.second.val)) {
					e.push(nxt++);
					e.push(l_elem.second.val);
					e.set(EvalState::SCOPE);
				}

			e.push(nxt);
			e.pushScope();
			return 1;
		});

		// Union (Append and remove duplicates) (*)
		e.addMember(Table, "_op*", [](EvalState& e) {
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
				e.set(EvalState::SCOPE);
			}

			e.push(nxt);
			e.pushScope();
			return 1;
		});

		// Intersection (Elements in both tables) (^)
		e.addMember(Table, "_op^", [](EvalState& e) {
			dust::Table lt = e.pop<dust::Table>(), rt = e.pop<dust::Table>();

			e.newScope();		// Create a table
			int nxt = 1;

			// Push all elements from the left table into the new table
				// Provided they aren found in the right table
			for (auto l_elem : *lt)
				if (rt->contains(l_elem.second.val)) {
					e.push(nxt++);
					e.push(l_elem.second.val);
					e.set(EvalState::SCOPE);
				}

			e.push(nxt);
			e.pushScope();
			return 1;
		});

		// Member-wise comparison (=)
		e.addMember(Table, "_op=", [](EvalState& e) {
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


		// Free functions
		e.push("typeof");						// Reflection entry point: typeof({obj}) returns a reflection object
		e.push([](EvalState& e) {
			e.pushNil();
			return 1;
		});
		e.set(EvalState::SCOPE);

		e.push("typename");
		e.push([](EvalState& e) {
			e.push(e.getTS().getName(e.pop().type_id));
			return 1;
		});
		e.set(EvalState::SCOPE);

		e.push("print");
		e.push([](EvalState& e) {
			while (!e.empty())
				std::cout << e.pop<std::string>();
			
			std::cout << std::endl;
			return 0;
		});
		e.set(EvalState::SCOPE);
		
	}
}