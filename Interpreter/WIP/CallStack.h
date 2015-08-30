#pragma once

#include "TypeTraits.h"
#include "Stack.h"
#include "GC.h"

namespace dust {

	class CallStack : public impl::Stack {
		private:
			impl::GC& gc;

		protected:
		public:
			CallStack(impl::GC& g) : gc{ g } {}

			// Push values onto the stack
			template <typename T>
			void push(T val) {
				Stack::push(TypeTraits<T>::make(val, gc));
			}

			void push(const char* val) {
				push<std::string>(val);
			}

			void push(impl::Value val) {
				if (val.type_id == TypeTraits<std::string>::id) gc.incRef(val.val.i);

				Stack::push(val);
			}


			// Pop values from the stack
			template <typename T>
			T pop(int idx = -1) {
				auto v = Stack::pop(idx);

				if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

				return TypeTraits<T>::get(v, gc);
			}

			impl::Value pop(int idx = -1) {
				return Stack::pop(idx);
			}

			template <typename T>
			void pop(T& val, int idx = -1) {
				val = pop<T>(idx);
			}


			// Stack management (lua functions)
			// Copies the value at the given index
			void copy(int idx = -1) {
				push(Stack::at(idx));			// Relies on CallStack::push behavior
			}
			
			// Replaces the value at the given index with the top
			void replace(int idx = -1) {
				auto& v = Stack::at(idx);

				if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

				v = pop();
			}


			// Others
			// For quicker String operations
				// Should this convert the object if it is not a string
			size_t pop_ref(int idx = -1) {
				if (Stack::at(idx).type_id != TypeTraits<std::string>::id) throw std::logic_error{ "Object at index is not a String" };

				return pop(idx).val.i;
			}

			// Shorter pop
			template <typename T>
			explicit operator T() {
				return pop<T>(-1);
			}

	};

}