#include "state.h"
#include <iostream>

EvalState& EvalState::reg_func(std::string name, const DustFunc& rule) {
	calc_rules[name] = rule;
	return *this;
}

int EvalState::call(std::string name) {
	int n = calc_rules[name](*this);
	return n;			// int is a holdover from old code (I may use it to transfer return values from functions)
}

DustObj EvalState::get(std::string var) {
	return globals[var];
}

EvalState& EvalState::swap() {
	_call.swap();
	return *this;
}

// Still need to modify to account for metamethods (future)
void addOperators(EvalState& state) {
	state.reg_func("_op+", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l + (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l + (double)r); return 1;
			default:
				return 0;
		}
	});											// Relies on arguments being evaluated right->left

	state.reg_func("_op*", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l * (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l * (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op-", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l - (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l - (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op/", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
			case ValType::FLOAT:
				s.push((double)l / (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op^", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
			case ValType::FLOAT:
				s.push(pow((double)l, (double)r)); return 1;
			default:
				return 0;
		}
	});

	// Not an official operator
	state.reg_func("_op%", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
			case ValType::FLOAT:
				s.push((int)l % (int)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op=", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l == (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l == (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op<", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l < (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l < (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op>", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l > (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l > (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op<=", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l <= (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l <= (double)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op>=", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (commonType(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l >= (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l >= (double)r); return 1;
			default:
				return 0;
		}
		//*/
	});

	state.reg_func("_op!=", [](EvalState& s) {
		s.call("_op=");
		s.call("_ou!");
		return 1;
	});

	state.reg_func("_ou-", [](EvalState& s) {
		auto l = s.pop();

		switch (l.type) {
			case ValType::INT:
			case ValType::BOOL:
				s.push(-(int)l); return 1;
			case ValType::FLOAT:
				s.push(-(double)l);; return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_ou!", [](EvalState& s) {
		auto l = s.pop();

		switch (l.type) {
			case ValType::INT:
			case ValType::BOOL:
			case ValType::FLOAT:
				s.push(!(int)l); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("print", [](EvalState& s) {
		/*
		while (!s.empty())
			std::cout << s.pop() << std::endl;
		*/

		std::cout << s.pop() << std::endl;
		return 0;
	});
}