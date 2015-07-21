#include "ast.h"
#include "state.h"

#include <iostream>

EvalState& evaluate(ASTNode::node_ptr& node, EvalState& state) {
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

void clear(stack<ASTNode::node_ptr>& s) {
	while (!s.empty())
		s.pop();
}

std::string Literal::to_string() { return _type(*this) + " " + val; }
std::string Variable::to_string() { return name; }
std::string Assignment::to_string() { return op;  }

EvalState& ASTNode::eval(EvalState& state) {
	// Is *this organized in right->left or left->right ???
	for (auto n : *this) 
		n->eval(state);

	return state;
}

EvalState& Literal::eval(EvalState& state) {
	state.push(std::stoi(val));
	return state;
}

EvalState& UnOp::eval(EvalState& state) {
	ASTNode::eval(state).call(this->oper);			// Work on calling convention??
	return state;
}

EvalState& BinOp::eval(EvalState& state) {
	//rhs()->eval(state).push(lhs()->eval(state).pop());	// For some reason this works

	// Change when I change the syntax/semantics for DustFuncs
		// Currently expect: {last_arg}, ..., {first_arg}
		// Might change to: {first_arg}, ..., {last_arg}	// In this case the swap is unnecessary
		// Also change how parsing lays out the data (optimized for the current expectation?)
	rhs()->eval(lhs()->eval(state)).swap().call(oper);		// Call lhs, then rhs, then swap their positions on the stack, then call (operators expect stack = ..., {rhs}, {lhs})

	//{...}.getStack().swap();								// Change EvalState::getStack to return stack&
	//state.call(oper);										// Unless I pop and push intermediate arguments (how maintainable is this though)
	return state;
}

EvalState& Variable::eval(EvalState& state) {
	state.push(state.get(this->name));				// Can I rework this to push the name on the stack (but then how do I get the value?)
	return state;
}

EvalState& Assignment::eval(EvalState& state) {
	auto variable = var();							// Get reference to the variable
	val()->eval(state);								// Push the expression value onto the stack

	if (op != ":") {								// If the assignment is compound
		variable->eval(state);						// Push the variable's value onto the stack
		// val()->eval(variable->eval(state));		// If the syntax is changed as shown in BinOp
		state.call("_op" + op.substr(1));			// Call the linked operator
	} // else
	// val()->eval(state);

	state.set(variable->to_string(), state.top());	// Perform assignment (assuming to_string() == Variable::name)

	return state;
}