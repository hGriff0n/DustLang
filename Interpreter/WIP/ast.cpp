#include "ast.h"
#include "state.h"

#include <iostream>

EvalState& evaluate(std::shared_ptr<ASTNode>& node, EvalState& state) {
	for (auto& n : *node)
		evaluate(n, state);		// Children work first (Especially good in a stack based execution environment)

	// do_stuff
	// For BinOp
	// state.call(tree->to_string(), 2);			// Call the function (of the node) with the top two items from the stack (requires having a stack in state)
	// node->eval(state);						// What about error handling ??? (Solve this later)

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
		case TokenType::Expr:				// 99
			return "Expression";
		default:
			return "Unrecognized Token";
	}
}

std::string _type(Literal& node) {
	switch (node.value_type()) {
		case ValType::INT:
			return "INT";
		case ValType::FLOAT:
			return "FLOAT";
		default:
			return "Unrecognized Literal Type";
	}
}

void clear(std::stack<std::shared_ptr<ASTNode>>& s) {
	while (!s.empty())
		s.pop();
}

std::string Literal::to_string() { return _type(*this) + " " + val; }


EvalState& ASTNode::eval(EvalState& state) {
	return state;
}

EvalState& Literal::eval(EvalState& state) {
	state.push(std::stoi(val));
	return state;
}

EvalState& UnOp::eval(EvalState& state) {
	if (this->oper == "_ou!")
		state.push(!state.pop());
	else
		state.push(-state.pop());

	//state.call(this->oper);					// Need to work on the calling convention
	return state;
}

EvalState& BinOp::eval(EvalState& state) {
	state.push(state.call(this->oper));
	//state.call(this->oper);
	return state;
}