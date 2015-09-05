#pragma once

#include "Value.h"
#include <vector>

namespace dust {
	namespace impl {
		class Stack {
			private:
				std::vector<Value> s;

			protected:
				int normalize(int& idx);
				bool invalidIndex(int idx);

				virtual void before(Value&, int);
				virtual void after(Value&, int);

				std::vector<Value>& stack();

			public:
				Stack();

				virtual void push(Value&);
				virtual Value pop(int = -1);

				// References the value at the given index
				virtual Value& at(int = -1);

				// Inserts the top at the given index
				virtual void insert(int = -1);

				// Swaps the values at the given index
				void swap(int = -1, int = -2);

				// Checks the type of the value at the given index
				template<typename T> bool is(int idx = -1) {
					return at(idx).type_id == TypeTraits<T>::id;
				}

				bool empty();
				void clear();
				size_t size();
				void reserve(size_t);
		};

	}
}
