#pragma once

#include "TypeTraits.h"
#include "Stack.h"
#include "GC.h"

// Specialization of TypeTraits<std::string>::make as I now have access to dust::impl::GC
template<> dust::impl::Value TypeTraits<std::string>::make(std::string s, dust::impl::GC& gc) {
	return{ gc.loadRef(s), TypeTraits<std::string>::id };
}

namespace dust {

	class CallStack : public impl::Stack {
		private:
			impl::GC& gc;

		protected:
		public:
			CallStack(impl::GC&);

			// Push values onto the stack
			template <typename T>
			void push(T val) {
				Stack::push(TypeTraits<T>::make(val, gc));				// Something is wrong with TypeTraits<std::string>::make (I'm able to "solve" it by "forward declaring" the string specialization, C4506)
			}
			void push(const char*);
			void push(impl::Value);	// impl::Value& ???


			// Pop values from the stack
			template <typename T>
			T pop(int idx = -1) {
				auto v = Stack::pop(idx);

				if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

				return TypeTraits<T>::get(v, gc);
			}

			impl::Value pop(int = -1);

			template <typename T>
			void pop(T& val, int idx = -1) {
				val = pop<T>(idx);
			}


			// Stack management (lua functions)
			// Copies the value at the given index
			void copy(int = -1);
			
			// Replaces the value at the given index with the top
			void replace(int = -1);


			// Other Functions
			// For quicker String operations (particularly equality testing)
				// Should this convert the object if it is not a string
			size_t pop_ref(bool = false);

			// Shorter pop
			template <typename T>
			explicit operator T() {
				return pop<T>(-1);
			}

	};

}