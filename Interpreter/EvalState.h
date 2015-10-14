#pragma once

#include "CallStack.h"
#include "TypeSystem.h"
#include "Table.h"

namespace dust {
	void initTypeSystem(dust::type::TypeSystem&);
	void initConversions(dust::type::TypeSystem&);
	void initOperations(dust::type::TypeSystem&);

	namespace type {
		// Traits conversion specializations (Could I move these into TypeTraits.h ???)
		template<> int Traits<int>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<double>::id)
					return v.val.d;

				else if (v.type_id == Traits<int>::id)
					return v.val.i;

				else if (v.type_id == Traits<std::string>::id)
					return std::stoi(gc.deref(v.val.i));
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
					return std::stod(gc.deref(v.val.i));
				}
			} catch (...) {}

			throw error::conversion_error{ "Not convertible to Float" };
		}

		template<> std::string Traits<std::string>::get(const impl::Value& v, impl::GC& gc) {
			if (v.type_id == Traits<std::string>::id)
				return gc.deref(v.val.i);

			else if (v.type_id == Traits<bool>::id)
				return v.val.i ? "true" : "false";

			else if (v.type_id == Traits<int>::id)
				return std::to_string(v.val.i);

			else if (v.type_id == Traits<double>::id)
				return std::to_string(v.val.d);

			throw error::conversion_error{ "Not convertible to String" };
		}

		template<> bool type::Traits<bool>::get(const impl::Value& v, impl::GC& gc) {
			if (v.type_id == Traits<bool>::id)
				return v.val.i;

			return true;
		}
	}

	namespace test {
		template <class Stream>
		class Tester;
	}

	class EvalState : public impl::CallStack {
		private:
			impl::Table* curr_scp;
			impl::Table global;
			type::TypeSystem ts;

			//impl::RuntimeStorage<str_record> strings;
			//impl::RuntimeStorage<impl::Table> tables;
			//impl::RuntimeStorage<void> user_data;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

			impl::Table* findScope(const std::string&, int, bool = false);
			impl::Table* findScope(const std::function<bool(impl::Table*)>&, int, bool = false);
			impl::Table* findScope(impl::Table*, const std::function<bool(impl::Table*)>&);
			int forcedLevel(const std::string&);

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
			void set(const std::string& name, bool is_const = false, bool is_typed = false);
			void get(const std::string& var);
			void set(bool is_const = false, bool is_typed = false);
			void get();

			// Might remove these interfaces (no need)
			void setGlobal(const std::string& name, bool isConst = false, bool isTyped = false);
			void getGlobal(const std::string& name);

			// Variable flags
			// Sets var to const if not const and vice versa
			void markConst(const std::string& name);
			void markTyped(const std::string& name, size_t typ);
			bool isConst(const std::string& name);
			bool isTyped(const std::string& name);

			// Scope Interaction
			void newScope();				// Start a new scope with the current scope as parent
			void endScope();				// Delete current scope (Cleans up memory)
			void pushScope();				// Store scope in memory and push on the stack (tables, functions, etc.)

			friend void initState(EvalState&);
			template <class Stream> friend class test::Tester;
	};


}