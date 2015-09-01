#include "EvalState.h"
#include "Init.h"

#include <iostream>

namespace dust {

	void initState(EvalState& e) {
		initTypeSystem(e.ts);
		initConversions(e.ts);
	}

	void EvalState::call(std::string fn) {
		if (fn.at(0) == '_' && fn.at(2) == 'p') {
			std::cout << "Operator" << std::endl;
		}		// function is an operator
	}

}