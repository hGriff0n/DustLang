#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

#include "Table.h"
typedef dust::impl::Table table_type;


namespace dust {
	namespace type {
		// Traits conversion specializations (Could I move these into TypeTraits.h ???)
		template<> int Traits<int>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<double>::id)
					return v.val.d;

				else if (v.type_id == Traits<int>::id)
					return v.val.i;

				else if (v.type_id == Traits<std::string>::id)
					return std::stoi(gc.getStrings().deref(v.val.i));
			} catch (...) {}

			throw error::conversion_error{ "Traits<int>::get", "Int" };
		}

		template<> double Traits<double>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<double>::id)
					return v.val.d;

				else if (v.type_id == Traits<int>::id)
					return v.val.i;

				else if (v.type_id == Traits<std::string>::id) {
					return std::stod(gc.getStrings().deref(v.val.i));
				}
			} catch (...) {}

			throw error::conversion_error{ "Traits<double>::get", "Float" };
		}

		template<> std::string Traits<std::string>::get(const impl::Value& v, impl::GC& gc) {
			if (v.type_id == Traits<std::string>::id)
				return gc.getStrings().deref(v.val.i);

			else if (v.type_id == Traits<bool>::id)
				return v.val.i ? "true" : "false";

			else if (v.type_id == Traits<int>::id)
				return std::to_string(v.val.i);

			else if (v.type_id == Traits<double>::id)
				return std::to_string(v.val.d);

			else if (v.type_id == Traits<Table>::id) {
				std::string t = "[";
				bool notFirst = false;

				for (auto pair : *(gc.getTables().deref(v.val.i))) {
					t += ((notFirst ? ", " : " ") +
						(pair.first.type_id == Traits<int>::id ? "" : Traits<std::string>::get(pair.first, gc) + ": ") +
						Traits<std::string>::get(pair.second.val, gc));
					notFirst = true;
				}

				return t + " ]";
			}

			throw error::conversion_error{ "Traits<string>::get", "String" };
		}

		template<> bool Traits<bool>::get(const impl::Value& v, impl::GC& gc) {
			if (v.type_id == Traits<bool>::id)
				return v.val.i;

			else if (v.type_id == Traits<Nil>::id)
				return false;

			else if (v.type_id == Traits<Table>::id) {
				return gc.getTables().deref(v.val.i)->size() != 0;
			}

			return true;
		}

		template<> Table Traits<Table>::get(const impl::Value& v, impl::GC& gc) {
			if (v.type_id == Traits<Table>::id)
				return gc.getTables().deref(v.val.i);

			return nullptr;				// return nil;
		}
	}

	namespace test {
		template <class Stream>
		class Tester;
	}

	/*
	 * The (current) entry point into dust evaluation.
	 * Provides a basic API to interact with dust systems
	 * - Passed to ASTNode::eval to perform evaluation
	 */
	class EvalState : public impl::CallStack {
		private:
			Table curr_scp;
			table_type global;
			type::TypeSystem ts;

			//impl::RuntimeStorage<std::string> strings;
			//impl::RuntimeStorage<impl::Table> tables;
			//impl::RuntimeStorage<void*> user_data;
			impl::GC gc;

		protected:
			// Convert the object at the given index to the given type
			void forceType(int, size_t);

			// Scope lookup for the given variable
			Table findScope(const impl::Value&, int, bool = false);

			// Set var.val = stack top
			void setVar(impl::Variable& var, bool is_const, bool is_typed);
			void setVar(dust::Table t, const impl::Value& key, bool is_const, bool is_typed);

			// Push tbl[var] on the stack (or nil)
			void getVar(Table tbl, const impl::Value& var);

		public:
			EvalState();

			// Call functions	(Temporary methods until I determine how functions and tables will be implemented)
			EvalState& call(const std::string& fn);
			EvalState& callOp(const std::string& fn);
			EvalState& callMethod(const std::string& fn);

			// EvalState doesn't know about shared_ptr or ASTNode
			//EvalState& eval(std::shared_ptr<parse::ASTNode>&);

			// Set/Get Variables
				// Need to clean up this interface a bit more (when I consolidate get/set)
			// Assign to the evaluation scope
			void setScoped(int lvl = 0, bool is_const = false, bool is_typed = false);
			void setScoped(const impl::Value& name, int lvl = 0, bool is_const = false, bool is_typed = false);

			// Assign to a table on the call stack
			void set();
			void set(const impl::Value& name);

			// Index the evaluation scope
			void getScoped(int lvl = 0);
			void getScoped(const impl::Value& var, int lvl = 0);

			// Index a table on the call stack
			void get();
			void get(const impl::Value& var);
			
			// Variable flags (setters & getters)
			void markConst(const impl::Value& name);
			void markTyped(const impl::Value& name, size_t typ);
			bool isConst(const impl::Value& name);
			bool isTyped(const impl::Value& name);

			// Scope Interaction

			// Start a new scope with the current scope as parent
			void newScope();
			// Delete current scope (Cleans up memory. Doesn't handle reference decrementing)
			void endScope();
			// Push scope on the stack (used in building tables)
			void pushScope(int nxt = 1);

			// Get the current type system
			type::TypeSystem& getTS();

			// Pass the top element on the stack to the stream
				// Handles non-printable values and string-special printing
			template <class Stream>
			Stream& stream(Stream& s) {
				if (is<std::string>())
					return s << "\"" << pop<std::string>() << "\"";

				else if (is<Nil>())
					return s << (pop(), "nil");

				return s << pop<std::string>();
			}

			friend void initState(EvalState&);
			template <class Stream> friend class test::Tester;
	};


}