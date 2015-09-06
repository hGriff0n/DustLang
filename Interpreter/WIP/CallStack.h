#pragma once

#include "TypeTraits.h"
#include "Value.h"
#include "Stack.h"
#include "GC.h"

// Specialization of type::Traits<std::string>::make as I now have access to dust::impl::GC
template<> dust::impl::Value dust::type::Traits<std::string>::make(std::string s, dust::impl::GC& gc) {
	return{ gc.loadRef(s), dust::type::Traits<std::string>::id };
}

namespace dust {
	namespace impl {

		class CallStack : public Stack<impl::Value> {
			private:
				impl::GC& gc;

			protected:
			public:
				CallStack(impl::GC&);

				// Push values onto the stack
				template <typename T>
				void push(T val) {
					Stack::push(type::Traits<T>::make(val, gc));				// Something is wrong with type::Traits<std::string>::make (I'm able to "solve" it by "forward declaring" the string specialization, C4506)
				}
				void push(const char* val);
				void push(impl::Value val);	// impl::Value& ???


				// Pop values from the stack
				template <typename T>
				T pop(int idx = -1) {
					auto v = Stack::pop(idx);

					if (v.type_id == type::Traits<std::string>::id) gc.decRef(v.val.i);

					return type::Traits<T>::get(v, gc);
				}

				impl::Value pop(int idx = -1);

				template <typename T>
				void pop(T& val, int idx = -1) {
					val = pop<T>(idx);
				}


				// Stack management (lua functions)
				// Copies the value at the given index
				void copy(int idx = -1);

				// Replaces the value at the given index with the top
				void replace(int idx = -1);

				// 
				void settop(int idx);


				// Other Functions
				// For quicker String operations (particularly equality testing)
					// Should this convert the object if it is not a string
				size_t pop_ref(bool decRef = false);

				// Shorter pop
				template <typename T>
				explicit operator T() {
					return pop<T>(-1);
				}

				// Checks the type of the value at the given index
				template<typename T> bool is(int idx = -1) {
					return at(idx).type_id == type::Traits<T>::id;
				}

		};
	}

}