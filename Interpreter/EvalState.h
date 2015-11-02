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

			throw error::conversion_error{ "Not convertible to Int" };
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

			throw error::conversion_error{ "Not convertible to Float" };
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

				t += " impl::Values not yet \"printable\"";
				for (auto pair : *(gc.getTables().deref(v.val.i))) {
					//t += ((notFirst ? ", ": " ") + pair.first + ": " + Traits<std::string>::get(pair.second.val, gc));
					//notFirst = true;
				}

				return t + " ]";
			}

			throw error::conversion_error{ "Not convertible to String" };
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

			/*
			else
			*/
			return nullptr;
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

			//impl::RuntimeStorage<str_record> strings;
			//impl::RuntimeStorage<impl::Table> tables;
			//impl::RuntimeStorage<void> user_data;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

			Table findScope(const impl::Value&, int, bool = false);

			void setVar(impl::Variable&, bool, bool);

		public:
			EvalState();

			// Call functions
			EvalState& call(const std::string& fn);
			EvalState& callOp(const std::string& fn);				// Temporary methods until I determine how functions and tables will be implemented
			EvalState& callMethod(const std::string& fn);

			// EvalState doesn't know about shared_ptr or ASTNode
			//EvalState& eval(std::shared_ptr<parse::ASTNode>&);

			// Set/Get Variables
			void setScoped(const impl::Value& name, int lvl = 0, bool is_const = false, bool is_typed = false);
			void setScoped(int lvl = 0, bool is_const = false, bool is_typed = false);
			void getScoped(const impl::Value& var, int lvl = 0);
			void getScoped(int lvl = 0);

			void get();
			void get(const impl::Value& var);
			void set();
			void set(const impl::Value& name);
			
			// Variable flags (setters & getters)
			void markConst(const impl::Value& name);
			void markTyped(const impl::Value& name, size_t typ);
			bool isConst(const impl::Value& name);
			bool isTyped(const impl::Value& name);

			// Scope Interaction
			void newScope();				// Start a new scope with the current scope as parent
			void endScope();				// Delete current scope (Cleans up memory)
			void pushScope();				// Store scope in memory and push on the stack (tables, functions, etc.)

			// TypeSystem Interaction
			type::TypeSystem& getTS();

			// Output
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