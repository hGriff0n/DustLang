#include "state.h"

EvalState& EvalState::reg_func(std::string name, const EvalState::func_type& rule) {
	calc_rules[name] = rule;
	return *this;
}

int EvalState::call(std::string name) {
	int n = calc_rules[name](*this);
	return n;			// int is a holdover from old code
}