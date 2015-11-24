#pragma once

#include <vector>
#include "Exceptions\logic.h"

namespace dust {

	/*
	 * Wrapper of std::vector that provides a random-access stack interface
	 */
	template <typename Value>
	class Stack {
		private:
			static const size_t MIN_STACK_SIZE = 20;
			std::vector<Value> s;

		protected:
			int normalize(int& idx) {
				return idx = idx < 0 ? idx + s.size() : idx;
			}

			bool invalidIndex(int idx) {
				return idx < 0 || idx >(int)s.size();
			}

			virtual void before(Value& v, int bef) {
				s.insert(s.begin() + normalize(bef), v);
			}

			virtual void after(Value& v, int bef) {
				s.insert(s.begin() + normalize(bef) + 1, v);
			}

		public:
			Stack() {
				reserve(MIN_STACK_SIZE);
			}

			virtual void push(Value& v) {
				s.push_back(v);
			}

			virtual Value pop(int idx = -1) {
				if (invalidIndex(normalize(idx))) throw error::out_of_bounds{ "Stack::pop", idx, size() };

				auto ret = s[idx];
				s.erase(s.begin() + idx);
				return ret;
			}

			// References the value at the given index
			virtual Value& at(int idx = -1) {
				if (invalidIndex(normalize(idx))) throw error::out_of_bounds{ "Stack::at", idx, size() };

				return s[idx];
			}

			// Inserts the top at the given index
			virtual void insert(int idx = -1) {
				before(pop(), idx);
			}

			// Swaps the values at the given index
			void swap(int idx1 = -1, int idx2 = -2) {
				if (invalidIndex(normalize(idx1)))
					throw error::out_of_bounds{ "Stack::swap::idx1", idx1, size() };

				if (invalidIndex(normalize(idx2)))
					throw error::out_of_bounds{ "Stack::swap::idx2", idx2, size() };

				std::swap(s[idx1], s[idx2]);
			}

			// stl interface methods
			bool empty() {
				return s.empty();
			}

			void clear() {
				s.clear();
			}

			size_t size() {
				return s.size();
			}

			void reserve(size_t space) {
				s.reserve(space);
			}

			auto begin() {
				return s.rbegin();
			}

			auto end() {
				return s.rend();
			}
	};

}
