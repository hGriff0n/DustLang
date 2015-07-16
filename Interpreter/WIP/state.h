#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>

//class Table;

// Run-time state
class EvalState {
	private:
		template <typename T>
		struct _stack {
			std::vector<T> s;
			int top = -1;

			void push(T obj) {
				s.emplace_back(obj);
				++top;
			}

			T pop() {
				if (top >= 0) {
					T ret = s[top--];
					s.pop_back();
					return ret;
				}

				return T{};
			}
		};

		std::map<std::string, std::function<int(int, int)>> calc_rules;			// I can extend this to be the "global" environment
		//std::vector<Table> globals; int curr_global;
		_stack<int> stack;

	protected:
	public:
		EvalState() {};

		EvalState& reg_func(std::string, const std::function<int(int, int)>&);

		EvalState& push(int v) {
			stack.push(v);
			return *this;
		}
		int pop() {
			return stack.pop();
		}
		std::vector<int> getStack() {
			return stack.s;
		}
		
		int call(std::string);		// I don't like this interface
};