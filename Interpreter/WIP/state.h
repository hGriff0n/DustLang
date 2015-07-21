#pragma once

#include <map>
#include <string>
#include <functional>
#include "stack.h"

// Run-time state

//class Table;
//class DustValue;
//class Environ;	// ???

class EvalState;
typedef std::function<int(EvalState&)> DustFunc;

class EvalState {
	private:
		std::map<std::string, DustFunc> calc_rules;			// I can extend this to be the "global" environment
		std::map<std::string, int> globals;
		//std::vector<Table> globals; int curr_global;
		stack<int> ss;
		//stack<DustValue> ss;

	protected:
	public:
		EvalState() {};

		EvalState& reg_func(std::string, const DustFunc&);

		EvalState& push(int v) {
			ss.push(v);
			return *this;
		}
		int pop() {
			return ss.pop();
		}
		int top() {
			return ss.top();
		}
		std::vector<int> getStack() {
			return ss.get();
		}
		
		// I don't like this interface
		int call(std::string);
		//int call(std::string, int);
		//EvalState& call(std::string, int, int);

		// Temporary interface for variables
		void set(std::string, int);
		int get(std::string);
};