#include "ast.h"
#include "state.h"

#include <iostream>

EvalState& evaluate(ASTNode::node_ptr& node, EvalState& state) {
	// What about error handling ??? (Solve this later)
	return node->eval(state);
}

std::string _typename(TokenType token) {
	switch (token) {
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
		case TokenType::Assignment:			// 100 (tmp)
			return "Assignment";
		case TokenType::List:				// 101 (tmp)
			return "List";
		default:
			return "Unrecognized Token";
	}
}

std::string _typename(ASTNode& node) {
	return _typename(node.token_type());
}

std::string _type(Literal& node) {
	// Or I could have ValType : std::string
	switch (node.value_type()) {
		case ValType::BOOL:					// 0
			return "BOOL";
		case ValType::INT:					// 1
			return "INT";
		case ValType::FLOAT:				// 2
			return "FLOAT";
		case ValType::STRING:				// 3
			return "STRING";
		case ValType::FUNCTION:				// 4
			return "FUNCTION";
		case ValType::TABLE:				// 5
			return "TABLE";
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
std::string Assignment::to_string() { return op; }
std::string List::to_string() { return "<" + _typename(vals) + ">"; }

EvalState& ASTNode::eval(EvalState& state) {
	// This evaluates right->left
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


// Need to clean up variable declarations (easier if I remove some comments)
EvalState& Assignment::eval(EvalState& state) {
	auto r_var = var()->begin(), l_var = var()->end();			// could convert to use std::make_pair
	auto r_val = val()->begin(), l_val = val()->end();
	int var_size = l_var - r_var, val_size = l_val - r_val;

	// Problems with this code in relation to multiple function returns and the splat operator
		// I don't know how many values will be returned at parse-time
			// Can a function return change the number of variables to be assigned (Though this is a massive side-effect
			// Should this run-time difference in variables and values change the number of expressions evaluated
		// Splat modifies the number of values that a variable "accepts" (I could modify the grammar for this)
		// Doesn't check that any assignments are "okay"
		// You could still perform optimizations on the assumption that the function returns 0-1 items (memoize the iterators)
			// Possibly keep a running tally of "used" variables and values
				// "request" a new value iff there is still a variable to take it (update based on how many were variables pushed on the stack)
				// don't pair the request and assignment loops

	// More values than variables (Readjust the val iterator (Can I store the new iterator???))
	while (val_size > var_size) {
		++r_val; --val_size;
	}

	// Evaluate expr_list (left->right) (1st on bottom)
	while (r_val != l_val) {
		--l_val; (*l_val)->eval(state);
	}

	// Didn't I give a seperate semantics if there are more variables than values in a compound assignment (namely the last value is carried over)???

	// More variables than values (Push nil{0} on the stack)
	while(var_size > val_size) {
		state.push(0); --var_size;
	}

	// Perform the assignments (right->left, compound if necessary)
	while (l_var != r_var) {
		if (compound) (*r_var)->eval(state).call("_op" + op);
		state.set((*r_var)->to_string(), state.pop()); ++r_var;
	}

	// The value of the last variable is the assignment's value (or is it?)
	return (*var()->begin())->eval(state);
}

// List data is stored right->left
EvalState& List::eval(EvalState& state) {
	//*/
	auto n = end();
	while (n != begin())
		(*--n)->eval(state);

	/*/

	for (auto n : *this)
		n->eval(state);
		//std::cout << n->to_string() << " - ";
	//*/

	//std::cout << "s: " << size() << std::endl;
	//state.push(0);

	return state;
}