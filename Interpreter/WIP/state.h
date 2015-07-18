#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>
// Run-time state

//class Table;

//class Environ;	// ???
class EvalState {
	private:
		// If I move this outside/public I can instead pass this to the function
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

		typedef std::function<int(EvalState&)> func_type;
		std::map<std::string, func_type> calc_rules;			// I can extend this to be the "global" environment
		//std::vector<Table> globals; int curr_global;
		_stack<int> stack;

	protected:
	public:
		EvalState() {};

		EvalState& reg_func(std::string, const func_type&);

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
		
		// I don't like this interface
		int call(std::string);
		//int call(std::string, int);
		//EvalState& call(std::string, int, int);
};