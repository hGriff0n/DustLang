#pragma once

#include <vector>

namespace dust {
	namespace impl {
		struct Value;

		class Stack {
			private:
				std::vector<Value> s;

			protected:
				int normalize(int& idx);
				bool invalidIndex(int idx);

			public:
				Stack();

				virtual void push(Value&);

				virtual void before(Value&, int);
				virtual void after(Value&, int);
				void swap(int = -1, int = -2);

				virtual Value pop(int = -1);
				virtual Value& at(int = -1);

				size_t size();
				void reserve(size_t);
				void settop(int);
				bool empty();
		};

	}
}
