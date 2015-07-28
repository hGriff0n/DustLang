#include "state.h"

EvalState& EvalState::reg_func(std::string name, const DustFunc& rule) {
	calc_rules[name] = rule;
	return *this;
}

int EvalState::call(std::string name) {
	int n = calc_rules[name](*this);
	return n;			// int is a holdover from old code (I may use it to transfer return values from functions)
}

void EvalState::set(std::string var, int val) {
	globals[var] = val;
}

int EvalState::get(std::string var) {
	return globals[var];
}