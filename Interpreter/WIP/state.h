#pragma once

#include <map>
#include <string>
#include <functional>
#include "stack.h"

// Run-time state

//class Table;

//class Environ;	// ???
class EvalState {
	private:
		// If I move this outside/public I can instead pass this to the function

		typedef std::function<int(EvalState&)> func_type;
		std::map<std::string, func_type> calc_rules;			// I can extend this to be the "global" environment
		//std::vector<Table> globals; int curr_global;
		stack<int> ss;

	protected:
	public:
		EvalState() {};

		EvalState& reg_func(std::string, const func_type&);

		EvalState& push(int v) {
			ss.push(v);
			return *this;
		}
		int pop() {
			return ss.pop();
		}
		std::vector<int> getStack() {
			return ss.get();
		}
		
		// I don't like this interface
		int call(std::string);
		//int call(std::string, int);
		//EvalState& call(std::string, int, int);
};