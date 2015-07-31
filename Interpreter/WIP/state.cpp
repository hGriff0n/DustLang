#include "state.h"

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
	using string = std::string;

	state.reg_func("_op+", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (lub(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l + (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l + (double)r); return 1;
			case ValType::STRING:
				//s.push(add_string(l, r)); return 1;			// I can "possibly" do very precise manipulations
				s.push((string)l + (string)r); return 1;
				// add string
			default:
				return 0;
		}
	});											// Relies on arguments being evaluated right->left

	state.reg_func("_op*", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (lub(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l * (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l * (double)r); return 1;
			//case ValType::STRING:
			default:
				return 0;
		}
	});

	state.reg_func("_op-", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
			case ValType::INT:
			case ValType::BOOL:
				s.push((int)l == (int)r); return 1;
			case ValType::FLOAT:
				s.push((double)l == (double)r); return 1;
			case ValType::STRING:
				s.push((string)l == (string)r); return 1;
			default:
				return 0;
		}
	});

	state.reg_func("_op<", [](EvalState& s) {
		auto l = s.pop(), r = s.pop();

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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

		switch (lub(l, r)) {
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


#include <unordered_map>

struct str_record {
	int n;
	std::string s;
};

static const int MAX_STRS = 100;
static str_record* storage = new str_record[MAX_STRS];
static str_record* open = storage;
static std::unordered_map<std::string, str_record*> registry{};

// Improvement: Garbage collection
	// Add a stack to next_record (to fill holes)
// Improvement: Dynamic Memory
// Improvement: Reduce storage duration of temporary values (I don't even use remove yet)
	// Or find a way to eliminate the storage of temps

//*/
str_record* next_record() {
	if (str_size() == MAX_STRS) throw "Reached Max Number of Strings";
	return open++;
}

str_record* store(std::string s) {
	str_record* ret = nullptr;

	if (registry.count(s) == 0) {			// New (or collected) string
		registry[s] = next_record();
		ret = registry[s];
		ret->n = 0;
		ret->s = s;
	} else
		ret = registry[s];

	++(ret->n);
	return ret;
}

std::string recall(str_record* r) {
	return r->s;
}

void remove(str_record* r) {
	--(r->n);
	r = nullptr;				// this does not delete what's at r
}

int str_size() {
	return open - storage;
}

int num_of(std::string s) {
	return registry[s]->n;
}

void all_strings() {
	for (auto it : registry)
		std::cout << it.first << " :: " << it.second->n << std::endl;
	// even temporary strings have it.second->n == 1
}