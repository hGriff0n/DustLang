#pragma once

#include "CallStack.h"
#include "TypeSystem.h"
#include "Scope.h"

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
			//Stack<Scope> curr_scope;
			impl::Scope global;
			type::TypeSystem ts;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

			// Variables
			void staticTyping(impl::Variable&, bool);
			void newVar(const std::string&, bool, bool);

		public:
			EvalState();

			// Call functions
			EvalState& call(const std::string& fn);
			EvalState& callOp(const std::string& fn);				// Temporary methods until I determine how functions and tables will be implemented
			EvalState& callMethod(const std::string& fn);

			// EvalState doesn't know about shared_ptr or ASTNode
			//EvalState& eval(std::shared_ptr<parse::ASTNode>&);

			// Set/Get Variables
			void setVar(const std::string& name, bool isConst = false, bool isTyped = false);
			void getVar(const std::string& var);

			// Variable flags
			// Sets var to const if not const and vice versa
			void markConst(const std::string& name);
			void markTyped(const std::string& name, size_t typ);
			bool isConst(const std::string& name);
			bool isTyped(const std::string& name);

			friend void initState(EvalState&);
			template <class Stream> friend class test::Tester;
	};


}