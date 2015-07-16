#include "state.h"

EvalState& EvalState::reg_func(std::string name, const std::function<int(int, int)>& rule) {
	calc_rules[name] = rule;
	return *this;
}

int EvalState::call(std::string name) {
	return calc_rules[name](stack.pop(), stack.pop());
}