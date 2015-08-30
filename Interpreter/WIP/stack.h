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

				void push(Value&);
				void before(Value&, int);
				void after(Value&, int);
				Value pop(int = -1);
				void swap(int = -1, int = -2);
				Value& at(int = -1);
				size_t size();
				bool empty();
		};

	}
}
