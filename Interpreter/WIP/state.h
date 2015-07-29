#pragma once

#include <map>
#include <string>
#include <functional>

#include "stack.h"
#include "types.h"
//#include <iostream>

// Run-time state

//class Table;
//class DustValue;
//class Environ;	// ???

class EvalState;
typedef std::function<int(EvalState&)> DustFunc;

// Consider decoupling the call stack from evaluation state (It would be a seperate argument to evaluate & eval/a plug-in to EvalState)
class EvalState {
	private:
		std::map<std::string, DustFunc> calc_rules;			// I can extend this to be the "global" environment
		std::map<std::string, DustObj> globals;
		//std::vector<Table> globals; int curr_global;
		stack<DustObj> _call;

	protected:
	public:
		EvalState() {};

		EvalState& reg_func(std::string, const DustFunc&);

		// Generic stack
		template <typename T>
		EvalState& push(T v) {
			_call.push(makeObj(v));
			return *this;
		}
		DustObj pop() {
			return _call.pop();
		}
		DustObj top() {
			return _call.top();
		}

		EvalState& swap();
		
		// I don't like this interface
		int call(std::string);
		//int call(std::string, int);
		//EvalState& call(std::string, int, int);

		// Temporary interface for variables
		template <typename T>
		void set(std::string var, T val) {
			globals[var] = makeObj(val);
		}
		DustObj get(std::string);
};

void addOperators(EvalState&);