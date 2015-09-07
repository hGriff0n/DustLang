#pragma once

#include "TypeTraits.h"
#include "Value.h"
#include "Stack.h"
#include "GC.h"

// Can I move this specialization to EvalState.h ??? NO
template<> dust::impl::Value dust::type::Traits<std::string>::make(std::string s, dust::impl::GC& gc) {
	return{ gc.loadRef(s), dust::type::Traits<std::string>::id };
}

namespace dust {
	namespace impl {

		class CallStack : public Stack<impl::Value> {
			private:
				impl::GC& gc;

			protected:
				void try_incRef(impl::Value&);
				void try_decRef(impl::Value&);

			public:
				CallStack(impl::GC&);

				// Push values onto the stack
				template <typename T>
				void push(T val) {
					Stack::push(type::Traits<T>::make(val, gc));
				}
				
				void push(const char* val);
				
				void push(impl::Value val);	// impl::Value& ???


				// Pop values from the stack
				template <typename T>
				T pop(int idx = -1) {
					auto v = pop(idx);

					try_decRef(v);

					return type::Traits<T>::get(v, gc);
				}

				impl::Value pop(int idx = -1);

				template <typename T>
				void pop(T& val, int idx = -1) {
					val = pop<T>(idx);
				}


				// Stack management

				// Copies the value at the given index
				void copy(int idx = -1);

				// Replaces the value at the given index with the top
				void replace(int idx = -1);

				// Resizes the stack to the given size
				void settop(int siz);


				// Other Functions
				// For quicker String operations (particularly equality testing)
					// Should this convert the object if it is not a string
				size_t pop_ref(bool decRef = false);
				// A push_ref method is not really useful (the implementation is also slightly convoluted)

				// Overload cast operator to perform a pop
				template <typename T>
				explicit operator T() {
					return pop<T>(-1);
				}

				// Checks the type of the value at the given index
				template<typename T>
				bool is(int idx = -1) {
					return at(idx).type_id == type::Traits<T>::id;
				}

		};
	}

}