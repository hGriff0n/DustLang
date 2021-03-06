#pragma once

#include "TypeTraits.h"
#include "Value.h"
#include "Stack.h"
#include "DualGC.h"
#include "Optional.h"
#include "../Utility/has_interface.h"

#include "Exceptions\runtime.h"

namespace dust {
	class EvalState;				// This forward declaration is for Traits specialization
	class Optional;

	// type::Traits::make specializations for default types
	namespace type {
		// Can I move this specialization to EvalState.h?  NO (I don't think so)
		template<> impl::Value Traits<std::string>::make(const std::string& s, impl::GC& gc) {
			return{ gc.getStrings().loadRef(s), Traits<std::string>::id };
		}

		template<> impl::Value Traits<Table>::make(const Table& t, impl::GC& gc) {
			return{ gc.getTables().loadRef(t), Traits<Table>::id };
		}

		template<> impl::Value Traits<Function>::make(const Function& f, impl::GC& gc) {
			return{ gc.getFunctions().loadRef(f), Traits<Function>::id };
		}

		template<> impl::Value Traits<std::shared_ptr<parse::ASTNode>>::make(const std::shared_ptr<parse::ASTNode>& f, impl::GC& gc) {
			return Traits<Function>::make(f, gc);
		}

		// Allow e.push to accept lambdas/std::functions/free functions
		template <typename T>
		struct Traits<T, std::enable_if_t<shl::has_interface<T, int(EvalState&)>::value>> {
			static size_t id;		// this will always be -1

			static impl::Value make(const T& v, impl::GC& gc) {
				return Traits<Function>::make(NativeFn{ v }, gc);
			}

			static T get(const impl::Value& v, impl::GC& gc) {
				throw error::illegal_template("Traits<T>::get is not implemented for type ", typeid(T));
			}
		};
	}
	
	namespace impl {

		/*
		 * Specialization of Stack<impl::Value> that handles interactions with the Garbage Collector
		 * Provides functions to pop and push values of any type (converted to impl::Value for storage)
		 */
		class CallStack : public Stack<impl::Value> {
			private:
				impl::GC& gc;
				size_t min_size = 0;

			protected:
				// Handle reference incrementing/decrementing based on type
				void try_incRef(const impl::Value&);
				void try_decRef(const impl::Value&);

				// Set the minimum number of elements the stack must have. Returns the old minimum
				size_t setMinSize(size_t new_min);

				// Translate a pseudo-index into a real index
				int normalize(int& idx);

				// Check whether the passed index is valid for the current stack
				bool invalidIndex(int idx);

				// TODO: What is this for
				std::vector<impl::Value>::iterator rbegin();

			public:
				CallStack(impl::GC&);

				/*
				 * Stack Interaction
				 */

				// Push values onto the stack
				template <typename T>
				void push(T val) {
					push(type::Traits<T>::make(val, gc));
				}

				// Special Overloads
				void push(const char* val);
				void push(const impl::Value& val);
				void push(const Optional& opt);
				void pushNil();

				// Overload cast operator to perform a pop
				template <typename T>
				explicit operator T() {
					return pop<T>(-1);
				}
				
				// Pop values from the stack
				template <typename T>
				T pop(int idx = -1) {
					auto v = pop(idx);

					return type::Traits<T>::get(v, gc);
				}
				impl::Value pop(int idx = -1);


				// Checks the type of the value at the given index
				template<typename T>
				bool is(int idx = -1) {
					return at(idx).type_id == type::Traits<T>::id;
				}


				/*
				 * Stack management
				 */

				// Copies the value at the given index
				void copy(int idx = -1);

				// Replaces the value at the given index with the top. Shrinks the stack
				void replace(int idx = -1);

				// Resizes the stack to the given size
				void settop(int siz);

				// Overload of empty to account for the variable minimum size
				bool empty();

				// Overload of size to possibly account for the variable minimum size
				size_t size();
		};
	}
}