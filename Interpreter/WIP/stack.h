#pragma once

#include <vector>

template <typename T>
class stack {
	private:
		std::vector<T> s;
		//int top = -1;

	public:
		void push(T obj) {
			s.emplace_back(obj);					//++top;
		}

		T pop() {
			T ret = s.back();					//T ret = s[top--];
			s.pop_back();
			return ret;
		}

		T top() {
			return s.back();
		}

		bool empty() {
			return s.empty();
		}

		int size() {
			return s.size();
		}

		std::vector<T> get() { return s; }
};