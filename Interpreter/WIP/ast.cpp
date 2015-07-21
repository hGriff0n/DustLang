#include "ast.h"
#include "state.h"

#include <iostream>

EvalState& evaluate(std::shared_ptr<ASTNode>& node, EvalState& state) {
	for (auto& n : *node)
		evaluate(n, state);		// Children work first (Especially good in a stack based execution environment) (What about variables ???)
								// I should move this work into the eval functions

	// What about error handling ??? (Solve this later)
	return node->eval(state);
}

std::string _typename(ASTNode& node) {
	switch (node.token_type()) {
		case TokenType::Comment:			// 0
			return "Comment";
		case TokenType::Literal:			// 1
			return "Literal";
		case TokenType::Operator:			// 2
			return "Operator";
		case TokenType::Variable:			// 3
			return "Variable";
		case TokenType::Expr:				// 98
			return "Expression";
		case TokenType::Debug:				// 99
			return "Debug";
		case TokenType::Assignment:
			return "Assignment";
		default:
			return "Unrecognized Token";
	}
}

std::string _type(Literal& node) {
	switch (node.value_type()) {
		case ValType::INT:					// 1
			return "INT";
		case ValType::FLOAT:				// 2
			return "FLOAT";
		default:
			return "Unrecognized Literal Type";
	}
}

void clear(stack<std::shared_ptr<ASTNode>>& s) {
	while (!s.empty())
		s.pop();
}

std::string Literal::to_string() { return _type(*this) + " " + val; }
std::string Variable::to_string() { return name; }
std::string Assignment::to_string() { return op;  }

EvalState& ASTNode::eval(EvalState& state) {
	/*
	for (auto n : *this) 
		n->eval(state);
	*/

	return state;
}

EvalState& Literal::eval(EvalState& state) {
	state.push(std::stoi(val));
	return state;
}

EvalState& UnOp::eval(EvalState& state) {
	//ASTNode::eval(state).call(this->oper);
	state.call(this->oper);					// Need to work on the calling convention
	return state;
}

EvalState& BinOp::eval(EvalState& state) {
	//ASTNode::eval(state).call(this->oper);
	state.call(this->oper);
	return state;
}

EvalState& Variable::eval(EvalState& state) {
	state.push(state.get(this->name));		// Think about adjusting when I change the stack from being ints only (simplifies assignment code)
	return state;
}

EvalState& Assignment::eval(EvalState& state) {
	state.pop();				// Because of how evaluation is currently performed

	auto variable = var();
	//val()->eval(state);

	if (op != ":") {			// If compound assignment
		variable->eval(state);
		state.call("_op" + op.substr(1));
	}

	state.set(variable->to_string(), state.top());

	return state;
}