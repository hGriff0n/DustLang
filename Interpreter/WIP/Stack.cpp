#include "Stack.h"
#include "Value.h"

#define MIN_STACK_SIZE 20

namespace dust {
	namespace impl {

		Stack::Stack() {
			reserve(MIN_STACK_SIZE);
		}

		int Stack::normalize(int& idx) {
			return idx = idx < 0 ? idx + s.size() : idx;
		}

		bool Stack::invalidIndex(int idx) {
			return idx < 0 || idx > s.size();
		}
		
		void Stack::push(Value& v) {
			s.push_back(v);
		}

		void Stack::before(Value& v, int bef) {
			s.insert(s.begin() + normalize(bef), v);
		}

		void Stack::after(Value& v, int aft) {
			s.insert(s.begin() + normalize(aft) + 1, v);
		}

		Value Stack::pop(int idx) {
			if (invalidIndex(normalize(idx))) throw std::out_of_range{ "Invalid index to CallStack" };

			auto ret = s[idx];
			s.erase(s.begin() + idx);				// Apparently erase shuffles the memory to be contiguous
			return ret;
		}

		void Stack::swap(int idx1, int idx2) {
			if (invalidIndex(normalize(idx1))) throw std::out_of_range{ "Invalid index to CallStack" };
			if (invalidIndex(normalize(idx2))) throw std::out_of_range{ "Invalid index to CallStack" };

			std::swap(s[idx1], s[idx2]);
		}

		Value& Stack::at(int idx) {
			if (invalidIndex(normalize(idx))) throw std::out_of_range{ "Invalid index to CallStack" };

			return s[idx];
		}

		size_t Stack::size() {
			return s.size();
		}

		void Stack::reserve(size_t space) {
			s.reserve(space);
		}

		void Stack::settop(int idx) {
			s.resize(normalize(idx));
		}

		bool Stack::empty() {
			return s.empty();
		}
	}
}