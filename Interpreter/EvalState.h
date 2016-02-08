#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {
	namespace type {

		// Traits conversion specializations (Could I move these into TypeTraits.h ???)
		template<> int Traits<int>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<double>::id)
					return (int)v.val.d;

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
			try {
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
			} catch (...) {}

			throw error::conversion_error{ "Traits<string>::get", "String" };
		}

		template<> bool Traits<bool>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<bool>::id)
					return (bool)v.val.i;

				else if (v.type_id == Traits<Table>::id) {
					return gc.getTables().deref(v.val.i)->size() != 0;
				}

			} catch (...) {
				return false;
			}

			return v.type_id != Traits<Nil>::id;
		}

		template<> Table Traits<Table>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<Table>::id)
					return gc.getTables().deref(v.val.i);
			} catch (...) {}

			throw error::conversion_error{ "Traits<Table>::get", "Table" };
			//return nullptr;	// Nil
		}

		template<> Function Traits<Function>::get(const impl::Value& v, impl::GC& gc) {
			try {
				if (v.type_id == Traits<Function>::id)
					return gc.getFunctions().deref(v.val.i);

			} catch (...) {}

			throw error::conversion_error{ "Traits<Function>::get", "Function" };
		}

		template <> struct Traits<size_t> : Traits<int> {};

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
		public:
			static const int SELF = INT_MAX, SCOPE = INT_MAX - 1;

		private:
			// Scoping
			impl::Table global, *curr_scp;

			type::TypeSystem ts;
			impl::GC gc;

			impl::Value self;
			bool resolving_function = false;

		protected:
			// Force the value at {idx} to have type {typ}
			void forceType(int idx, size_t typ);

			// Find the nth definition of key in [tbl..global]
			static Table findDef(Table tbl, const impl::Value& key, int lookup);

			// Put tbl[key] on the stack
			void getTable(Table tbl, const impl::Value& key);

			// Set tbl[key] = val
			void setTable(Table tbl, const impl::Value& key, const impl::Value& val);

		public:
			EvalState();

			// Get variable at {-1} in {idx} (unless idx = SELF)
			void get(int idx, int lookup = 0);

			// Set variable at {-2} to value at {-1} in {idx} (unless idx = SELF)
			void set(int idx, int lookup = 0);

			// Call the function at {idx - 1} passing {idx} arguments
			void call(int num_args);

			// Call the operator (with type resolution)
			void callOp(std::string op);

			// Ensure OOP structure exists/doesn't exist
			EvalState& enableObjectSyntax();

			// Allow self to be set by get
			void setResolvingFunctionName();


			// Variable flags (setters & getters)
			//void markConst(const impl::Value& name);
			//void markTyped(const impl::Value& name, size_t typ);
			//bool isConst(const impl::Value& name);
			//bool isTyped(const impl::Value& name);


			// Scope Interaction
			void newScope();				// Start a new scope with the current scope as parent
			void endScope();				// Delete current scope (Cleans up memory)
			void pushScope();				// Push scope on the stack (used in building tables)

			// Get the current type system
			type::TypeSystem& getTS();
			impl::GC& getGC();

			template <typename T>
			type::TypeSystem::TypeVisitor& addMember(type::TypeSystem::TypeVisitor& t, std::string op, T val) {
				auto v = type::Traits<T>::make(val, gc);
				auto fn = type::Traits<std::string>::make(op, gc);

				try_incRef(v);
				try_incRef(fn);

				return t.addOp(fn, v, op);
			}

			// Pass the top element on the stack to the stream
				// Handles non-printable values and string-special printing
			template <class Stream>
			Stream& stream(Stream& s) {
				if (empty())
					return s << "No values on the stack";

				if (is<std::string>())
					return s << "\"" << pop<std::string>() << "\"";

				else if (is<Nil>())
					return s << (pop(), "nil");

				else if (is<Function>())
					return s << (pop(), "function");

				return s << pop<std::string>();
			}

			friend void initState(EvalState&);
			template <class Stream> friend class test::Tester;
	};

	void initConversions(EvalState& e);
	void initOperations(EvalState& e);

}