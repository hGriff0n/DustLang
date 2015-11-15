#pragma once

#include "TypeTraits.h"
#include "Value.h"
#include "Stack.h"
#include "DualGC.h"

#include "Exceptions\runtime.h"

namespace dust {
	// Can I move this specialization to EvalState.h?  NO
	template<> impl::Value type::Traits<std::string>::make(std::string s, impl::GC& gc) {
		return{ gc.getStrings().loadRef(s), type::Traits<std::string>::id };
	}

	template<> impl::Value type::Traits<Table>::make(Table t, impl::GC& gc) {
		return{ gc.getTables().loadRef(t), type::Traits<Table>::id };
	}

	namespace impl {

		/*
		 * Specialization of Stack<impl::Value> that handles interactions with the Garbage Collector
		 * Provides functions to pop and push values of any type (converted to impl::Value for storage)
		 */
		class CallStack : public Stack<impl::Value> {
			private:
				impl::GC& gc;

			protected:
				// Handle reference incrementing/decrementing based on type
				void try_incRef(impl::Value&);
				void try_decRef(impl::Value&);

			public:
				CallStack(impl::GC&);

				// Push values onto the stack
				template <typename T>
				void push(T val) {
					Stack::push(type::Traits<T>::make(val, gc));
				}

				// Special Overloads
				void push(const char* val);
				void push(impl::Value val);	// impl::Value& ???
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

					try_decRef(v);

					return type::Traits<T>::get(v, gc);
				}
				impl::Value pop(int idx = -1);


				// Checks the type of the value at the given index
				template<typename T>
				bool is(int idx = -1) {
					return at(idx).type_id == type::Traits<T>::id;
				}

				// Stack management

				// Copies the value at the given index
				void copy(int idx = -1);

				// Replaces the value at the given index with the top
				// Stack size decreases by 1
				void replace(int idx = -1);

				// Resizes the stack to the given size
				void settop(int siz);
		};
	}
}